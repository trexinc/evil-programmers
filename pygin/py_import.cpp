#include "headers.hpp"

#include "py_import.hpp"

#include "py_common.hpp"

#include "python.hpp"

namespace py
{
	namespace import
	{
		module import(const string& Name)
		{
			return cast<module>(invoke(PyImport_Import, Name.get()));
		}

		module reload(const module& Module)
		{
			return cast<module>(invoke(PyImport_ReloadModule, Module.get()));
		}
	}
}
