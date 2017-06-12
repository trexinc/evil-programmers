#include "headers.hpp"

#include "py_module.hpp"
#include "py_common.hpp"

#include "python.hpp"

namespace py
{
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
