#pragma once

/*
system.hpp

*/
/*
Copyright 2018 Alex Alabuzhev
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

namespace detail
{
	struct module_closer
	{
		void operator()(HMODULE Module) const
		{
			FreeLibrary(Module);
		}
	};
}

using module_ptr = std::unique_ptr<std::remove_pointer_t<HMODULE>, detail::module_closer>;

namespace detail
{
	struct local_deleter
	{
		void operator()(HLOCAL Memory) const
		{
			LocalFree(Memory);
		}
	};
}

template<typename T>
using local_ptr = std::unique_ptr<T, detail::local_deleter>;

namespace detail
{
	struct nop_deleter
	{
		void operator()(void*) const
		{
		}
	};
}

template<class T>
using movalbe_ptr = std::unique_ptr<T, detail::nop_deleter>;

template<typename T>
class ptr_setter_t
{
public:
	ptr_setter_t(const ptr_setter_t&) = delete;
	ptr_setter_t& operator=(const ptr_setter_t&) = delete;
	ptr_setter_t(ptr_setter_t&&) = default;
	ptr_setter_t& operator=(ptr_setter_t&&) = default;

	ptr_setter_t(T& Ptr): m_Ptr(&Ptr)
	{
	}

	~ptr_setter_t()
	{
		if (m_Ptr)
			m_Ptr->reset(m_RawPtr);
	}

	auto operator&()
	{
		return &m_RawPtr;
	}

private:
	movalbe_ptr<T> m_Ptr;
	typename T::pointer m_RawPtr{};
};

template<typename T>
auto ptr_setter(T& Ptr)
{
	return ptr_setter_t<T>(Ptr);
}

std::wstring GetLastErrorMessage(DWORD LastError);
