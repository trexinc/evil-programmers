#include "headers.hpp"

#include "py_common.hpp"

#include "py_object.hpp"

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
		return invoke(PyCallable_Check, Object.get()) != 0;
	}
}

