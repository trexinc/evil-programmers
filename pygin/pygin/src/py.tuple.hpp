#pragma once

#include "py.type.hpp"

namespace py
{
	class tuple: public object, public proxy_owner<tuple, size_t>
	{
	public:
		using proxy_owner<tuple, size_t>::operator[];

		static const type& get_type();

		explicit tuple(size_t Size = 0);
		tuple(const std::initializer_list<object>& Args);
		tuple(cast_guard, const object& Object);

		void set_at(size_t Position, const object& Object);
		object get_at(size_t Position) const;

		size_t size() const;
	};
}
