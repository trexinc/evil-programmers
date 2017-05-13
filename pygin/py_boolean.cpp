#include "headers.hpp"

#include "py_boolean.hpp"

#include "python.hpp"

namespace py
{
	boolean::boolean(bool Value):
		object(Value? Py_True : Py_False)
	{
	}

	boolean::boolean(cast_guard, const object& Object):
		object(Object)
	{
	}

	bool boolean::to_bool() const
	{
		return get() == Py_True;
	}
}
