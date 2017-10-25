/*
py.module.cpp

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

#include "py.module.hpp"

#include "py.dictionary.hpp"
#include "py.string.hpp"
#include "py.sys.hpp"

#include "py.common.hpp"

#include "types_cache.hpp"

#include "python.hpp"

namespace py
{
	const type& module::get_type()
	{
		return types_cache::get_type(types::module, []
		{
			const auto ModulesDict = cast<dictionary>(sys::get_object("modules"));
			using namespace literals;
			return type(ModulesDict.get_at("sys"_py));
		});
	}

	module::module(cast_guard, const object& Object):
		object(Object)
	{
	}

	void module::add_object(const char* Name, const object& Object)
	{
		DONT_STEAL_REFERENCE(Object.get());
		py::invoke(PyModule_AddObject, get(), Name, Object.get());
	}

	void module::add_functions(PyMethodDef* Methods)
	{
		// Not available with Py_LIMITED_API
		// py::invoke(PyModule_AddFunctions, get(), Methods);

		const auto Name = get_attribute("__name__");
		for(auto Method = Methods; Method->ml_name; ++Method)
		{
			object Function(py::invoke(PyCFunction_NewEx, Method, get(), Name.get()));
			set_attribute(Method->ml_name, Function);
		}
	}
}
