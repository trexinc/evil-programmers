#pragma once

#include "py_module.hpp"
#include "py_string.hpp"

namespace py
{
	namespace import
	{
		module import(const string& Name);
		module reload(const module& Module);
	}
}
