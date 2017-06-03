#pragma once

#include "py_object.hpp"

namespace py
{
	class uuid: public object
	{
	public:
		MOVABLE(uuid);
		static auto type_name() { return "UUID"; }

		explicit uuid(const UUID& Uuid);
		explicit uuid(cast_guard, const object& Object);

		UUID to_uuid() const;
	};
}
