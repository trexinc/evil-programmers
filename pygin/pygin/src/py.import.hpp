#pragma once

#include "py.module.hpp"
#include "py.string.hpp"

namespace py
{
	namespace import
	{
		module import(const string& Name);
		module reload(const module& Module);
	}
}
