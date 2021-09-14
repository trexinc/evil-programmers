#pragma once

/*
py.integer.hpp

*/
/*
Copyright 2017 Alex Alabuzhev
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "py.type.hpp"

namespace py
{
	class integer: public object
	{
	public:
		static const type& get_type();

		integer(int Value);
		integer(unsigned int Value);
		integer(long Value);
		integer(unsigned long Value);
		integer(long long Value);
		integer(unsigned long long Value);
		explicit integer(void* Value);
		integer(cast_guard, const object& Object);

		[[nodiscard]] operator short() const;
		[[nodiscard]] operator unsigned short() const;
		[[nodiscard]] operator int() const;
		[[nodiscard]] operator unsigned int() const;
		[[nodiscard]] operator long() const;
		[[nodiscard]] operator unsigned long() const;
		[[nodiscard]] operator long long() const;
		[[nodiscard]] operator unsigned long long() const;

		[[nodiscard]] explicit operator void*() const;
	};

	inline namespace literals
	{
		[[nodiscard]]
		inline auto operator ""_py(unsigned long long Value) noexcept
		{
			return integer(Value);
		}
	}

	template<>
	[[nodiscard]]
	inline short cast(const object& Object)
	{
		return cast<integer>(Object);
	}

	template<>
	[[nodiscard]]
	inline unsigned short cast(const object& Object)
	{
		return cast<integer>(Object);
	}

	template<>
	[[nodiscard]]
	inline int cast(const object& Object)
	{
		return cast<integer>(Object);
	}

	template<>
	[[nodiscard]]
	inline unsigned int cast(const object& Object)
	{
		return cast<integer>(Object);
	}

	template<>
	[[nodiscard]]
	inline long cast(const object& Object)
	{
		return cast<integer>(Object);
	}

	template<>
	[[nodiscard]]
	inline unsigned long cast(const object& Object)
	{
		return cast<integer>(Object);
	}

	template<>
	[[nodiscard]]
	inline long long cast(const object& Object)
	{
		return cast<integer>(Object);
	}

	template<>
	[[nodiscard]]
	inline unsigned long long cast(const object& Object)
	{
		return cast<integer>(Object);
	}

	template<>
	[[nodiscard]]
	inline void* cast(const object& Object)
	{
		return static_cast<void*>(cast<integer>(Object));
	}

	template<typename type>
	struct cast_impl<type, false>
	{
		static auto impl(const object& Object)
		{
			static_assert(std::is_enum<type>::value, "Only enums are supported");
			return static_cast<type>(cast<std::underlying_type_t<type>>(Object));
		}
	};

	template <typename type>
	struct translate<type,typename std::enable_if_t<std::is_integral_v<type> && !std::is_same_v<type, bool>>>
	{
		static [[nodiscard]] object from(type Value)
		{
			return integer(Value);
		}
	};

	template <typename type>
	struct translate<type, typename std::enable_if_t<std::is_enum_v<type>>>
	{
		static [[nodiscard]] object from(type Value)
		{
			return translate<std::underlying_type_t<type>>::from(Value);
		}
	};
}
