/*
py.common.cpp

*/
/*
Copyright 2017 Alex Alabuzhev
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"

#include "py.common.hpp"

#include "py.import.hpp"
#include "py.list.hpp"
#include "py.object.hpp"
#include "py.string.hpp"

#include "python.hpp"

#ifdef _DEBUG
extern std::atomic_size_t g_ObjectsCount;
#endif

namespace py
{
	void initialize()
	{
		Py_Initialize();
	}

	void finalize()
	{
#ifdef _DEBUG
		assert(!g_ObjectsCount);
#endif
		Py_Finalize();
	}

	bool callable_check(const object& Object)
	{
		return invoke(PyCallable_Check, Object.get()) != 0;
	}

	void raise_exception_if_needed()
	{
		if (!PyErr_Occurred())
			return;

		PyObject* TypePtr, * ValuePtr, * TracebackPtr;
		PyErr_Fetch(&TypePtr, &ValuePtr, &TracebackPtr);
		PyErr_NormalizeException(&TypePtr, &ValuePtr, &TracebackPtr);

		object Type(TypePtr), Value(ValuePtr), Traceback(TracebackPtr);

		const auto TracebackModule = import::import("traceback"_py);

		std::string Message;

		{
			const auto List = cast<list>(TracebackModule["format_exception_only"](Type, Value));
			for (size_t i = 0, size = List.size(); i != size; ++i)
			{
				Message += cast<string>(List[i]);
			}
		}

		if (Traceback)
		{
			Message += "\1\nTraceback (most recent call last):\n"s;

			const auto List = cast<list>(TracebackModule["format_tb"](Traceback));
			for (size_t i = 0, size = List.size(); i != size; ++i)
			{
				Message += cast<string>(List[i]);
			}
		}

		throw exception(Message);
	}
}

