#include "headers.hpp"

#include "py_err.hpp"
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
			PyErr_Print();
		}
	}
}
