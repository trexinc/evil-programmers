/*
py.list.cpp

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

#include "py.list.hpp"

#include "py.common.hpp"

#include "types_cache.hpp"

#include "python.hpp"

namespace py
{
	const type& list::get_type()
	{
		return types_cache::get_type(types::list, []
		{
			return type(list(0));
		});
	}

	list::list(size_t Size):
		object(invoke(PyList_New, Size))
	{
	}

	list::list(cast_guard, const object& Object):
		object(Object)
	{
	}

	void list::set_at(size_t Index, const object& Value)
	{
		invoke(PyList_SetItem, get(), Index, Value.get_no_steal());
	}

	object list::get_at(size_t Index) const
	{
		return from_borrowed(invoke(PyList_GetItem, get(), Index));
	}

	size_t list::size() const
	{
		return invoke(PyList_Size, get());
	}

	void list::push_back(const object& Object)
	{
		invoke(PyList_Append, get(), Object.get());
	}

	void list::insert(const object& Object, size_t index)
	{
		invoke(PyList_Insert, get(), index, Object.get());
	}
}
