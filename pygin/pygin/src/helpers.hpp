#pragma once

/*
helpers.hpp

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

#include "py.list.hpp"
#include "py.string.hpp"

namespace helpers
{
	namespace list
	{
		template<typename type, typename converter>
		[[nodiscard]] static auto from_array(const type* Data, size_t Size, const converter& Converter)
		{
			py::list Result;
			std::transform(Data, Data + Size, std::back_inserter(Result), Converter);
			return Result;
		}
	}

	class py_string_storage
	{
	public:
		py_string_storage() = default;
		NONCOPYABLE(py_string_storage)

		~py_string_storage(){ this->clear(); }

		void reserve(size_t size) { m_Strings.reserve(size); }

		void clear();

		size_t size() const { return m_Strings.size(); }
		wchar_t const* const* data() const { return m_Strings.data(); }

		wchar_t const* push_back(py::string const&);
		wchar_t const* push_back_allow_none(py::object const&);
	private:
		std::vector<wchar_t*> m_Strings;
	};

	wchar_t const* push_back_if_not_none(py::object const&, py_string_storage&);

}
