#pragma once

#include "py_type.hpp"

namespace py
{
	class integer: public object
	{
	public:
		static const type& get_type();

		integer(int Value);
		integer(unsigned int Value);
		integer(long long Value);
		integer(unsigned long long Value);
		integer(cast_guard, const object& Object);

		operator short() const;
		operator unsigned short() const;
		operator int() const;
		operator unsigned int() const;
		operator long() const;
		operator unsigned long() const;
		operator long long() const;
		operator unsigned long long() const;
	};

	inline namespace literals
	{
		inline auto operator ""_py(unsigned long long Value) noexcept
		{
			return integer(Value);
		}
	}

	template<>
	inline short cast(const object& Object)
	{
		return cast<integer>(Object);
	}

	template<>
	inline unsigned short cast(const object& Object)
	{
		return cast<integer>(Object);
	}

	template<>
	inline int cast(const object& Object)
	{
		return cast<integer>(Object);
	}

	template<>
	inline unsigned int cast(const object& Object)
	{
		return cast<integer>(Object);
	}

	template<>
	inline long cast(const object& Object)
	{
		return cast<integer>(Object);
	}

	template<>
	inline unsigned long cast(const object& Object)
	{
		return cast<integer>(Object);
	}

	template<>
	inline long long cast(const object& Object)
	{
		return cast<integer>(Object);
	}

	template<>
	inline unsigned long long cast(const object& Object)
	{
		return cast<integer>(Object);
	}
}
