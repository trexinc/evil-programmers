#include "Headers.hpp"

#include "Imports.hpp"

ImportedFunctions ifn;

void ImportedFunctions::Load()
{
	if (const auto hKernel32 = GetModuleHandle(L"kernel32.dll"))
	{
		pAttachConsole = reinterpret_cast<ATTACHCONSOLE>(GetProcAddress(hKernel32,"AttachConsole"));
		pIsWow64Process = reinterpret_cast<ISWOW64PROCESS>(GetProcAddress(hKernel32,"IsWow64Process"));
	}
}

BOOL ImportedFunctions::AttachConsole(DWORD dwProcessId) const
{
	BOOL Ret=FALSE;
	if(pAttachConsole)
	{
		Ret=pAttachConsole(dwProcessId);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	}
	return Ret;
}

BOOL ImportedFunctions::IsWow64Process(HANDLE hProcess,PBOOL Wow64Process) const
{
	BOOL Ret=0;
	if(pIsWow64Process)
	{
		Ret=pIsWow64Process(hProcess,Wow64Process);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	}
	return Ret;
}
