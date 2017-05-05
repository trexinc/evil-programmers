#pragma once

#include "py_object.hpp"

namespace py
{
	class integer: public object
	{
	public:
		explicit integer(int Value);
	};
}
