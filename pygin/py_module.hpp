#pragma once

#include "py_object.hpp"

struct PyMethodDef;

namespace py
{
	class module: public object
	{
	public:
		static auto type_name() { return "module"; }

		module() = default;
		module(cast_guard, const object& Object);

		void add_object(const char* Name, const object& Object);
		void add_functions(const PyMethodDef* Methods);
	};
}
