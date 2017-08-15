#pragma once

#include "py.type.hpp"

namespace py
{
	class bytes: public object
	{
	public:
		static const type& get_type();

		bytes(const void* Value, size_t Size);
		bytes(cast_guard, const object& Object);
	};
}
