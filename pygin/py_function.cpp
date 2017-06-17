#include "headers.hpp"

#include "py_function.hpp"

//#include "python.hpp"

namespace py
{
	function::function(cast_guard, const object& Object):
		object(Object)
	{
	}

	function::function(const object& Object, const char* Name):
		function(cast<function>(Object[Name]))
	{
	}
}
