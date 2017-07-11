#include "headers.hpp"

#include "py_module.hpp"

#include "py_dictionary.hpp"
#include "py_string.hpp"
#include "py_sys.hpp"

#include "py_common.hpp"

#include "types_cache.hpp"

#include "python.hpp"

namespace py
{
	const type& module::get_type()
	{
		return types_cache::get_type(types::module, []()
		{
			const auto ModulesDict = cast<dictionary>(sys::get_object("modules"));
			using namespace literals;
			return type(cast_guard{}, ModulesDict.get_at("sys"_py));
		});
	}

	module::module(cast_guard, const object& Object):
		object(Object)
	{
	}

	void module::add_object(const char* Name, const object& Object)
	{
		DONT_STEAL_REFERENCE(Object.get());
		py::invoke(PyModule_AddObject, get(), Name, Object.get());
	}

	void module::add_functions(const PyMethodDef* Methods)
	{
		py::invoke(PyModule_AddFunctions, get(), const_cast<PyMethodDef*>(Methods));
	}
}
