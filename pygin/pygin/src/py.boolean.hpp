#pragma once

#include "py.type.hpp"

namespace py
{
	class boolean: public object
	{
	public:
		static const type& get_type();

		boolean(bool Value);
		boolean(cast_guard, const object& Object);

		operator bool() const;
	};

	template<>
	inline bool cast(const object& Object)
	{
		return cast<boolean>(Object);
	}

}
