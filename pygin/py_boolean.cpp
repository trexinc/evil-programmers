#include "headers.hpp"

#include "py_boolean.hpp"

#include "types_cache.hpp"

#include "python.hpp"

namespace py
{
	const type& boolean::get_type()
	{
		return types_cache::get_type(types::boolean, []()
		{
			return type(cast_guard{}, boolean(false));
		});
	}

	boolean::boolean(bool Value):
		object(Value? Py_True : Py_False)
	{
	}

	boolean::boolean(cast_guard, const object& Object):
		object(Object)
	{
	}

	boolean::operator bool() const
	{
		return get() == Py_True;
	}
}
