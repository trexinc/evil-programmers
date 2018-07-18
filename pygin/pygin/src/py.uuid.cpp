/*
py.uuid.cpp

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

#include "py.uuid.hpp"

#include "py.import.hpp"
#include "py.string.hpp"
#include "py.type.hpp"

#include "py.common.hpp"

#include "types_cache.hpp"

#include "python.hpp"

using namespace py::literals;

namespace py
{
	const type& uuid::get_type()
	{
		return types_cache::get_type(types::uuid, []
		{
			return type(uuid(UUID{}));
		});
	}

	uuid::uuid(const UUID& Uuid):
		object(import::import("uuid"_py).get_attribute("UUID")(none(), none(), object(invoke(PyBytes_FromStringAndSize, reinterpret_cast<const char*>(&Uuid), sizeof(Uuid)))))
	{
	}

	uuid::uuid(cast_guard, const object& Object):
		object(Object)
	{
	}

	uuid::operator UUID() const
	{
		const auto Bytes = (*this)["bytes_le"];
		UUID Result;
		std::memcpy(&Result, invoke(PyBytes_AsString, Bytes.get()), sizeof(Result));
		return Result;
	}

	object from(const UUID& Value)
	{
		return uuid(Value);
	}
}
