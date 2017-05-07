#include "headers.hpp"

#include "py_integer.hpp"

#include "python.hpp"

namespace py
{
	integer::integer(int Value):
		object(PyLong_FromLong(Value))
	{
	}

	integer::integer(unsigned int Value):
		object(PyLong_FromUnsignedLong(Value))
	{
	}

	integer::integer(long long Value):
		object(PyLong_FromLongLong(Value))
	{
	}

	integer::integer(unsigned long long Value):
		object(PyLong_FromUnsignedLongLong(Value))
	{
	}

	integer::integer(const object& Object):
		object(Object)
	{
	}

	size_t integer::to_size_t() const
	{
		return PyLong_AsSize_t(get());
	}

	int integer::to_int() const
	{
		return PyLong_AsLong(get());
	}

	unsigned int integer::to_unsigned_int() const
	{
		return PyLong_AsUnsignedLong(get());
	}

	long long integer::to_long_long() const
	{
		return PyLong_AsLongLong(get());
	}

	unsigned long long integer::to_unsigned_long_long() const
	{
		return PyLong_AsUnsignedLongLong(get());
	}

	integer as_integer(PyObject* Object)
	{
		return integer(object::from_borrowed(Object));
	}

	integer as_integer(const object& Object)
	{
		return integer(Object);
	}
}
