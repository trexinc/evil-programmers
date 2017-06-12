#pragma once

#include "py_object.hpp"

namespace py
{
	class integer: public object
	{
	public:
		static auto type_name() { return "int"; }

		explicit integer(int Value);
		explicit integer(unsigned int Value);
		explicit integer(long long Value);
		explicit integer(unsigned long long Value);
		integer(cast_guard, const object& Object);

		size_t to_size_t() const;
		int to_int() const;
		unsigned int to_unsigned_int() const;
		long long to_long_long() const;
		unsigned long long to_unsigned_long_long() const;
	};

	inline namespace literals
	{
		inline auto operator ""_py(unsigned long long Value) noexcept
		{
			return integer(Value);
		}
	}
}
