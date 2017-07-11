#pragma once

#include "py_type.hpp"

using PyTypeObject = struct _typeobject;

namespace py
{
	class object;

	class function: public object
	{
	public:
		static const type& get_type();

		function() = default;
		function(const object& Object, const char* Name);
		function(cast_guard, const object& Object);
	};
}
