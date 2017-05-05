#pragma once

#include "py_object.hpp"

namespace py
{
	void initialize();
	void finalize();

	bool callable_check(const object& Object);
}
