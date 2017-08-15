#include "headers.hpp"

#include "py.function.hpp"

#include "py.import.hpp"

#include "types_cache.hpp"

//#include "python.hpp"

namespace py
{
	const type& function::get_type()
	{
		return types_cache::get_type(types::function, []
		{
			return type(import::import("pygin").get_attribute("load_plugin"));
		});
	}

	function::function(cast_guard, const object& Object):
		object(Object)
	{
	}

	function::function(const object& Object, const char* Name):
		function(cast<function>(Object[Name]))
	{
	}
}
