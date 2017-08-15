#include "headers.hpp"

#include "py.bytes.hpp"

#include "py.common.hpp"

#include "types_cache.hpp"

#include "python.hpp"

namespace py
{
	const type& bytes::get_type()
	{
		return types_cache::get_type(types::bytes, []
		{
			return type(bytes("", 0));
		});
	}

	bytes::bytes(const void* Value, size_t Size):
		object(invoke(PyBytes_FromStringAndSize, static_cast<const char*>(Value), Size))
	{
	}

	bytes::bytes(cast_guard, const object& Object):
		object(Object)
	{
	}
}
