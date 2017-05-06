#include "headers.hpp"

#include "py_integer.hpp"

#include "python.hpp"

namespace py
{
	integer::integer(int Value):
		object(PyLong_FromLong(Value))
	{
	}

	integer::integer(unsigned long long Value):
		object(PyLong_FromUnsignedLongLong(Value))
	{
	}
}
