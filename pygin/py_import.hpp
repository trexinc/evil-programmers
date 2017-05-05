#pragma once

#include "py_object.hpp"

namespace py
{
	namespace import
	{
		object import(const object& Name);
		object reload_module(const object& Name);
	}
}