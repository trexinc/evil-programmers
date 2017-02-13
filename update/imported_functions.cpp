#include "Headers.hpp"
#pragma hdrstop

#include "imported_functions.hpp"

imported_functions ifn;

void imported_functions::Load()
{
	if (const auto hKernel32 = GetModuleHandle(L"kernel32.dll"))
	{
		pAttachConsole = reinterpret_cast<ATTACHCONSOLE>(GetProcAddress(hKernel32, "AttachConsole"));
		pIsWow64Process = reinterpret_cast<ISWOW64PROCESS>(GetProcAddress(hKernel32, "IsWow64Process"));
	}
}

BOOL imported_functions::AttachConsole(DWORD dwProcessId) const
{
	if (pAttachConsole)
	{
		return pAttachConsole(dwProcessId);
	}

	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

BOOL imported_functions::IsWow64Process(HANDLE hProcess, PBOOL Wow64Process) const
{
	if (pIsWow64Process)
	{
		return pIsWow64Process(hProcess, Wow64Process);
	}

	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}
