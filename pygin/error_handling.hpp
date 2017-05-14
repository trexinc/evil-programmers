#pragma once

#include "py_err.hpp"

void set_error_context(const wchar_t* Summary, const std::string& Description);
bool get_error_context(ErrorInfo* Info);
void reset_error_context();

template<typename callable, typename fallback>
auto try_call(const callable& Callable, const fallback& Fallback) noexcept
{
	try
	{
		reset_error_context();
		return Callable();
	}
	catch (const py::err::exception& e)
	{
		set_error_context(L"Pygin: Python exception", e.what());
		return Fallback();
	}
	catch (const std::exception& e)
	{
		set_error_context(L"Pygin: std::exception", e.what());
		return Fallback();
	}
	catch (...)
	{
		return Fallback();
	}
}
