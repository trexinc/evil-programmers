#pragma once

#include "py_object.hpp"

using PyTypeObject = struct _typeobject;

namespace py
{
	class object;

	class type: public object
	{
	public:
		static const type& get_type();

		type() = default;
		explicit type(const object& Object);
	};
}
