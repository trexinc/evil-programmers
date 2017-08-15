#include "headers.hpp"

#include "py.sys.hpp"

#include "py.common.hpp"

#include "python.hpp"

namespace py
{
	namespace sys
	{
		object get_object(const char* Name)
		{
			return object::from_borrowed(invoke(PySys_GetObject, Name));
		}
	}
}