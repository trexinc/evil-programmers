#pragma once

/*
error_handling.hpp

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

#include "py.common.hpp"

void set_error_context(const wchar_t* Summary, const std::string& Description);
bool get_error_context(ErrorInfo* Info);
void reset_error_context();

namespace detail
{
	class exception_impl
	{
	public:
		exception_impl(const std::string& Message, const char* Function, const char* File, int Line);
		const std::string& get_message() const noexcept;
		const std::string& get_full_message() const noexcept;

	private:
		std::string m_Message;
		std::string m_FullMessage;
	};
}

class pygin_exception: public detail::exception_impl, public std::runtime_error
{
public:
	pygin_exception(const std::string& Message, const char* Function, const char* File, int Line);
};

class silent_exception
{};

#define MAKE_EXCEPTION(ExceptionType, ...) ExceptionType(__VA_ARGS__, __FUNCTION__, __FILE__, __LINE__)
#define MAKE_PYGIN_EXCEPTION(...) MAKE_EXCEPTION(pygin_exception, __VA_ARGS__)


template<typename callable, typename fallback>
auto try_call(const callable& Callable, const fallback& Fallback) noexcept
{
	try
	{
		reset_error_context();
		return Callable();
	}
	catch (const py::exception& e)
	{
		set_error_context(L"Pygin: Python exception", e.what());
	}
	catch (const pygin_exception& e)
	{
		set_error_context(L"Pygin: pygin_exception", e.what());
	}
	catch (const std::exception& e)
	{
		set_error_context(L"Pygin: std::exception", e.what());
	}
	catch (const silent_exception&)
	{
	}
	catch (...)
	{
		set_error_context(L"Pygin: unknown exception", "Unknown exception");
	}
	return Fallback();
}
