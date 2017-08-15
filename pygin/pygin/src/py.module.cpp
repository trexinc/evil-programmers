#include "headers.hpp"

#include "py.module.hpp"

#include "py.dictionary.hpp"
#include "py.string.hpp"
#include "py.sys.hpp"

#include "py.common.hpp"

#include "types_cache.hpp"

#include "python.hpp"

namespace py
{
	const type& module::get_type()
	{
		return types_cache::get_type(types::module, []
		{
			const auto ModulesDict = cast<dictionary>(sys::get_object("modules"));
			using namespace literals;
			return type(ModulesDict.get_at("sys"_py));
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

	void module::add_functions(PyMethodDef* Methods)
	{
		// Not available with Py_LIMITED_API
		// py::invoke(PyModule_AddFunctions, get(), Methods);

		const auto Name = get_attribute("__name__");
		for(auto Method = Methods; Method->ml_name; ++Method)
		{
			object Function(py::invoke(PyCFunction_NewEx, Method, get(), Name.get()));
			set_attribute(Method->ml_name, Function);
		}
	}
}
