#pragma once

#include "py.type.hpp"

struct PyMethodDef;

namespace py
{
	class module: public object
	{
	public:
		static const type& get_type();

		module() = default;
		module(cast_guard, const object& Object);

		void add_object(const char* Name, const object& Object);
		void add_functions(PyMethodDef* Methods);
	};
}
