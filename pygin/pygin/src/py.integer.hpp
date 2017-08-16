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

		operator short() const;
		operator unsigned short() const;
		operator int() const;
		operator unsigned int() const;
		operator long() const;
		operator unsigned long() const;
		operator long long() const;
		operator unsigned long long() const;

		explicit operator void*() const;
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

	template<>
	inline void* cast(const object& Object)
	{
		return static_cast<void*>(cast<integer>(Object));
	}

	template<typename Type>
	struct cast_impl<Type, false>
	{
		static auto impl(const object& Object)
		{
			static_assert(std::is_enum<Type>::value, "Only enums are supported");
			return static_cast<Type>(cast<std::underlying_type_t<Type>>(Object));
		}
	};
}
