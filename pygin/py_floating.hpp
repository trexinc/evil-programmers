#pragma once

#include "py_type.hpp"

namespace py
{
	class floating: public object
	{
	public:
		static const type& get_type();

		floating(double Value);
		floating(cast_guard, const object& Object);

		operator double() const;
	};

	inline namespace literals
	{
		inline auto operator ""_py(long double Value) noexcept
		{
			return floating(Value);
		}
	}

	template<>
	inline double cast(const object& Object)
	{
		return cast<floating>(Object);
	}
}
