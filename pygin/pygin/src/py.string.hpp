#pragma once

/*
py.string.hpp

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
	class string: public object
	{
	public:
		static const type& get_type();

		string(const char* Str, size_t Size);
		string(const wchar_t* Str, size_t Size);
		string(const char* Str);
		string(const wchar_t* Str);
		string(const std::string& Str);
		string(const std::wstring& Str);
		string(cast_guard, const object& Object);

		bool operator==(const string& Other) const;

		operator std::string() const;
		operator std::wstring() const;
	};

	inline namespace literals
	{
		inline auto operator ""_py(const char* Data, size_t Size) noexcept
		{
			return string(Data, Size);
		}

		inline auto operator ""_py(const wchar_t* Data, size_t Size) noexcept
		{
			return string(Data, Size);
		}
	}

	template<>
	inline std::wstring cast(const object& Object) 
	{
		return cast<string>(Object);
	}

	template<>
	inline std::string cast(const object& Object)
	{
		return cast<string>(Object);
	}

	object from(const char* Value);
	object from(const wchar_t* Value);
	object from(const std::string& Value);
	object from(const std::wstring& Value);
}
