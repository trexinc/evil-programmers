#pragma once

#include <functional>

// poor man's SCOPE_EXIT
// TODO: take from Far

class scope_exit_t
{
public:
	explicit scope_exit_t(const std::function<void()>& Callable):
		m_Callable(Callable)
	{
	}

	scope_exit_t(const scope_exit_t&) = delete;
	scope_exit_t& operator=(const scope_exit_t&) = delete;

	~scope_exit_t()
	{
		if (m_Callable)
			m_Callable();
	}

private:
	std::function<void()> m_Callable;
};
