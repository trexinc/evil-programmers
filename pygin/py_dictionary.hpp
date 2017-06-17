#pragma once

#include "py_object.hpp"

namespace py
{
	class dictionary: public object, public proxy_owner<dictionary, object>
	{
	public:
		using proxy_owner<dictionary, object>::operator[];
		static auto type_name() { return "dict"; }

		dictionary();
		dictionary(cast_guard, const object& Object);

		void set_at(const object& Key, const object& Value);
		object get_at(const object& Key) const;
	};
}
