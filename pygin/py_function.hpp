#pragma once

#include "py_object.hpp"

using PyTypeObject = struct _typeobject;

namespace py
{
	class object;

	class function: public object
	{
	public:
		static auto type_name() { return "function"; }

		function() = default;
		function(const object& Object, const char* Name);
		function(cast_guard, const object& Object);
	};
}
