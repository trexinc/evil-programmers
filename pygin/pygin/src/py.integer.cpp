#include "headers.hpp"

#include "py.integer.hpp"

#include "py.common.hpp"

#include "types_cache.hpp"

#include "python.hpp"

namespace py
{
	const type& integer::get_type()
	{
		return types_cache::get_type(types::integer, []
		{
			return type(integer(0));
		});
	}

	integer::integer(int Value):
		integer(static_cast<long>(Value))
	{
	}

	integer::integer(unsigned int Value):
		integer(static_cast<unsigned long>(Value))
	{
	}

	integer::integer(long Value):
		object(invoke(PyLong_FromLong, Value))
	{
	}

	integer::integer(unsigned long Value):
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

	integer::integer(void* Value):
		object(invoke(PyLong_FromVoidPtr, Value))
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

	integer::operator void*() const
	{
		return invoke(PyLong_AsVoidPtr, get());
	}
}
