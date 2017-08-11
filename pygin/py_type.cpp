#include "headers.hpp"

#include "py_type.hpp"

#include "py_common.hpp"

#include "types_cache.hpp"

#include "python.hpp"

namespace py
{
	const type& type::get_type()
	{
		return types_cache::get_type(types::type, []
		{
			return type(from_borrowed(Py_None));
		});
	}

	type::type(const object& Object):
		object(PyType_Check(Object.get())? Object : object(invoke(PyObject_Type, Object.get())))
	{
	}
}
