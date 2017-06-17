#pragma once

#include "py_common.hpp"

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
	catch (...)
	{
		set_error_context(L"Pygin: unknown exception", "Unknown exception");
	}
	return Fallback();
}
