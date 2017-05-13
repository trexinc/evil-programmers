#include "headers.hpp"

#include "py_integer.hpp"

#include "python.hpp"

namespace py
{
	integer::integer(int Value):
		object(invoke(PyLong_FromLong, Value))
	{
	}

	integer::integer(unsigned int Value):
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

	integer::integer(cast_guard, const object& Object):
		object(Object)
	{
	}

	size_t integer::to_size_t() const
	{
		return invoke(PyLong_AsSize_t, get());
	}

	int integer::to_int() const
	{
		return invoke(PyLong_AsLong, get());
	}

	unsigned int integer::to_unsigned_int() const
	{
		return invoke(PyLong_AsUnsignedLong, get());
	}

	long long integer::to_long_long() const
	{
		return invoke(PyLong_AsLongLong, get());
	}

	unsigned long long integer::to_unsigned_long_long() const
	{
		return invoke(PyLong_AsUnsignedLongLong, get());
	}
}
