#include "headers.hpp"

#include "py_import.hpp"
#include "py_object.hpp"
#include "python.hpp"

namespace py
{
	namespace import
	{
		object import(const object& Name)
		{
			return object(PyImport_Import(Name.get()));
		}

		object reload_module(const object& Name)
		{
			return object(PyImport_ReloadModule(Name.get()));
		}
	}
}
