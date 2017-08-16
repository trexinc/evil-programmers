/*
py.integer.cpp

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

#include "py.integer.hpp"

#include "py.common.hpp"

#include "types_cache.hpp"

#include "python.hpp"

namespace py
{
	const type& integer::get_type()
	{
		return types_cache::get_type(types::integer, []
		{
			return type(integer(0));
		});
	}

	integer::integer(int Value):
		integer(static_cast<long>(Value))
	{
	}

	integer::integer(unsigned int Value):
		integer(static_cast<unsigned long>(Value))
	{
	}

	integer::integer(long Value):
		object(invoke(PyLong_FromLong, Value))
	{
	}

	integer::integer(unsigned long Value):
		object(invoke(PyLong_FromUnsignedLong, Value))
	{
	}

	integer::integer(long long Value):
		object(invoke(PyLong_FromLongLong, Value))
	{
	}

	integer::integer(unsigned long long Value):
		object(invoke(PyLong_FromUnsignedLongLong, Value))
	{
	}

	integer::integer(void* Value):
		object(invoke(PyLong_FromVoidPtr, Value))
	{
	}

	integer::integer(cast_guard, const object& Object):
		object(Object)
	{
	}

	integer::operator short() const
	{
		return static_cast<short>(invoke(PyLong_AsLong, get()));
	}

	integer::operator unsigned short() const
	{
		return static_cast<unsigned short>(invoke(PyLong_AsUnsignedLong, get()));
	}

	integer::operator int() const
	{
		return invoke(PyLong_AsLong, get());
	}

	integer::operator unsigned int() const
	{
		return invoke(PyLong_AsUnsignedLong, get());
	}

	integer::operator long() const
	{
		return invoke(PyLong_AsLong, get());
	}

	integer::operator unsigned long() const
	{
		return invoke(PyLong_AsUnsignedLong, get());
	}

	integer::operator long long() const
	{
		return invoke(PyLong_AsLongLong, get());
	}

	integer::operator unsigned long long() const
	{
		return invoke(PyLong_AsUnsignedLongLong, get());
	}

	integer::operator void*() const
	{
		return invoke(PyLong_AsVoidPtr, get());
	}
}
