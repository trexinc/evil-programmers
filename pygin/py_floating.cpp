#include "headers.hpp"

#include "py_floating.hpp"

#include "py_common.hpp"

#include "types_cache.hpp"

#include "python.hpp"

namespace py
{
	const type& floating::get_type()
	{
		return types_cache::get_type(types::floating, []
		{
			return type(floating(0));
		});
	}

	floating::floating(double Value):
		object(invoke(PyFloat_FromDouble, Value))
	{
	}

	floating::floating(cast_guard, const object& Object):
		object(Object)
	{
	}

	floating::operator double() const
	{
		return invoke(PyLong_AsDouble, get());
	}
}
