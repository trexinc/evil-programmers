#pragma once

#include "py_object.hpp"

namespace py
{
	class integer: public object
	{
	public:
		static auto type_name() { return "int"; }

		integer(int Value);
		integer(unsigned int Value);
		integer(long long Value);
		integer(unsigned long long Value);
		integer(cast_guard, const object& Object);

		operator int() const;
		operator unsigned int() const;
		operator long long() const;
		operator unsigned long long() const;
	};

	inline namespace literals
	{
		inline auto operator ""_py(unsigned long long Value) noexcept
		{
			return integer(Value);
		}
	}
}
