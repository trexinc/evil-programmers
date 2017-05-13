#pragma once

void notify_user(const std::string& ExceptionText);

template<typename callable, typename fallback>
auto try_call(const callable& Callable, const fallback& Fallback) noexcept
{
	try
	{
		return Callable();
	}
	catch (const std::exception& e)
	{
		notify_user(e.what());
		return Fallback();
	}
	catch (...)
	{
		return Fallback();
	}
}
