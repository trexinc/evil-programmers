#include "headers.hpp"

#include "py_uuid.hpp"

#include "python.hpp"

namespace py
{
	uuid::uuid(cast_guard, const object& Object):
		object(Object)
	{
	}

	UUID uuid::to_uuid() const
	{
		const auto Bytes = get_attribute("bytes_le");
		UUID Result;
		std::memcpy(&Result, invoke(PyBytes_AsString, Bytes.get()), sizeof(Result));
		return Result;
	}
}
