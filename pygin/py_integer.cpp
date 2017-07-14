#include "headers.hpp"

#include "py_integer.hpp"

#include "py_common.hpp"

#include "types_cache.hpp"

#include "python.hpp"

namespace py
{
	const type& integer::get_type()
	{
		return types_cache::get_type(types::integer, []()
		{
			return type(cast_guard{}, integer(0));
		});
	}

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

	integer::operator short() const
	{
		return static_cast<short>(invoke(PyLong_AsLong, get()));
	}

	integer::operator unsigned short() const
	{
		return static_cast<unsigned short>(invoke(PyLong_AsUnsignedLong, get()));
	}

	integer::operator int() const
	{
		return invoke(PyLong_AsLong, get());
	}

	integer::operator unsigned int() const
	{
		return invoke(PyLong_AsUnsignedLong, get());
	}

	integer::operator long() const
	{
		return invoke(PyLong_AsLong, get());
	}

	integer::operator unsigned long() const
	{
		return invoke(PyLong_AsUnsignedLong, get());
	}

	integer::operator long long() const
	{
		return invoke(PyLong_AsLongLong, get());
	}

	integer::operator unsigned long long() const
	{
		return invoke(PyLong_AsUnsignedLongLong, get());
	}
}
