#pragma once

#include "headers.hpp"

class critical_section
{
public:
	critical_section() { InitializeCriticalSection(&m_Lock); }
	~critical_section() { DeleteCriticalSection(&m_Lock); }

	critical_section(const critical_section&) = delete;
	critical_section(critical_section&&) = default;

	critical_section& operator=(const critical_section&) = delete;
	critical_section& operator=(critical_section&&) = default;

	void lock() { EnterCriticalSection(&m_Lock); }
	void unlock() { LeaveCriticalSection(&m_Lock); }

private:
	CRITICAL_SECTION m_Lock;
};
