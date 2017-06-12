#pragma once
#include "py_object.hpp"

using PyTypeObject = struct _typeobject;

namespace py
{
	class object;

	class type: public object
	{
	public:
		static auto type_name() { return "type"; }

		type() = default;
		type(const object& Object, const char* Name);
		type(cast_guard, const object& Object);

		bool is_same(const type& Object) const;
	};
}
