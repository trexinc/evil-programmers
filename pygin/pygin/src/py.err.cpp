#include "headers.hpp"

#include "py.err.hpp"

#include "py.common.hpp"

#include "python.hpp"

namespace py
{
	namespace err
	{
		object occurred()
		{
			return object::from_borrowed(PyErr_Occurred());
		}

		void clear()
		{
			PyErr_Clear();
		}

		void print()
		{
			if (PyErr_Occurred())
				PyErr_Print();
		}

		exception::exception(const char* Name):
			type(object(invoke(PyErr_NewException, Name, nullptr, nullptr)))
		{
		}
	}
}
