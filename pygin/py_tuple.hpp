#pragma once

#include "py_object.hpp"

namespace py
{
	class tuple: public object, public proxy_owner<tuple, size_t>
	{
	public:
		using proxy_owner<tuple, size_t>::operator[];
		static auto type_name() { return "tuple"; }

		explicit tuple(size_t Size = 0);
		tuple(const std::initializer_list<object>& Args);
		template<typename... args>
		tuple(const args&... Args): tuple({ Args... }) {}
		tuple(cast_guard, const object& Object);

		void set_at(size_t Position, const object& Object);
		object get_at(size_t Position) const;
	};
}
