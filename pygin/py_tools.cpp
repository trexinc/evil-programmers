#include "headers.hpp"

#include "py_tools.hpp"

#include "python.hpp"

namespace py
{
	void initialize()
	{
		Py_Initialize();
	}

	void finalize()
	{
		Py_Finalize();
	}

	bool callable_check(const object& Object)
	{
		return PyCallable_Check(Object.get()) != 0;
	}
}

