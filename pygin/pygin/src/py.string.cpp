/*
py.string.cpp

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

#include "py.string.hpp"

#include "py.common.hpp"

#include "types_cache.hpp"

#include "python.hpp"

namespace py
{
	const type& string::get_type()
	{
		return types_cache::get_type(types::string, []
		{
			return type(string(static_cast<const char*>(nullptr), 0));
		});
	}

	string::string(const char* Str, size_t Size):
		object(invoke(PyUnicode_FromStringAndSize, Str, Size))
	{
	}

	string::string(const wchar_t* Str, size_t Size):
		object(invoke(PyUnicode_FromWideChar, Str, Size))
	{
	}

	string::string(const char* Str):
		string(Str, strlen(Str))
	{
	}

	string::string(const wchar_t* Str):
		string(Str, wcslen(Str))
	{
	}

	string::string(const std::string& Str):
		string(Str.data(), Str.size())
	{
	}

	string::string(const std::wstring& Str):
		string(Str.data(), Str.size())
	{
	}

	string::string(cast_guard, const object& Object):
		object(Object)
	{
	}

	bool string::operator==(const string& Other) const
	{
		return invoke(PyUnicode_Compare, get(), Other.get()) == 0;
	}

	string::operator std::string() const
	{
		const auto Utf8Str = object(invoke(PyUnicode_AsUTF8String, get()));
		const auto Data = invoke(PyBytes_AsString, Utf8Str.get());

		const auto Size = static_cast<size_t>(invoke(PyBytes_Size, Utf8Str.get()));

		return { Data, Size };
	}

	string::operator std::wstring() const
	{
		Py_ssize_t Size;
		const auto Data = invoke(PyUnicode_AsWideCharString, get(), &Size);

		const std::wstring Result(Data, static_cast<size_t>(Size));

		invoke(PyMem_Free, Data);

		return Result;
	}
}
