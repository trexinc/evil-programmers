#pragma once

#include "py_object.hpp"

namespace py
{
	class boolean: public object
	{
	public:
		MOVABLE(boolean);
		static auto type_name() { return "bool"; }

		explicit boolean(bool Value);
		explicit boolean(cast_guard, const object& Object);

		bool to_bool() const;
	};
}
