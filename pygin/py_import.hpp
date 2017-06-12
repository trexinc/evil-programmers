#pragma once

#include "py_module.hpp"

namespace py
{
	namespace import
	{
		module import(const object& Name);
		module reload_module(const object& Name);
	}
}
