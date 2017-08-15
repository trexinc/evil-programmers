#pragma once

#include "py.type.hpp"

namespace py
{
	class uuid: public object
	{
	public:
		static const type& get_type();

		uuid(const UUID& Uuid);
		uuid(cast_guard, const object& Object);

		operator UUID() const;
	};

	template<>
	inline UUID cast(const object& Object)
	{
		return cast<uuid>(Object);
	}
}
