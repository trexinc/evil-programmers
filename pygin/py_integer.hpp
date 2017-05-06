#pragma once

#include "py_object.hpp"

namespace py
{
	class integer: public object
	{
	public:
		TRIVIALLY_MOVABLE(integer);

		explicit integer(int Value);
		explicit integer(unsigned long long Value);
	};

	inline auto operator ""_py(unsigned long long Value) noexcept
	{
		return integer(Value);
	}
}
