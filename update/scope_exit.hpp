#pragma once

#include <functional>

// poor man's SCOPE_EXIT
// TODO: take from Far

class scope_exit_t
{
public:
	NONCOPYABLE(scope_exit_t);

	explicit scope_exit_t(std::function<void()> Callable):
		m_Callable(std::move(Callable))
	{
	}

	~scope_exit_t()
	{
		if (m_Callable)
			m_Callable();
	}

private:
	std::function<void()> m_Callable;
};
