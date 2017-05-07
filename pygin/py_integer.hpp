#pragma once

#include "py_object.hpp"

namespace py
{
	class integer: public object
	{
	public:
		TRIVIALLY_MOVABLE(integer);

		explicit integer(int Value);
		explicit integer(unsigned int Value);
		explicit integer(long long Value);
		explicit integer(unsigned long long Value);
		explicit integer(const object& Object);

		size_t to_size_t() const;
		int to_int() const;
		unsigned int to_unsigned_int() const;
		long long to_long_long() const;
		unsigned long long to_unsigned_long_long() const;
	};

	integer as_integer(PyObject* Object);
	integer as_integer(const object& Object);

	inline namespace literals
	{
		inline auto operator ""_py(unsigned long long Value) noexcept
		{
			return integer(Value);
		}
	}
}
