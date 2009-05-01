#include "Headers.hpp"

#include "Imports.hpp"

ImportedFunctions ifn;

VOID ImportedFunctions::Load()
{
	HMODULE hKernel32=GetModuleHandle(TEXT("kernel32.dll"));
	if(hKernel32)
	{
		pAttachConsole=reinterpret_cast<ATTACHCONSOLE>(GetProcAddress(hKernel32,"AttachConsole"));
		pGetConsoleProcessList=reinterpret_cast<GETCONSOLEPROCESSLIST>(GetProcAddress(hKernel32,"GetConsoleProcessList"));
	}
}

BOOL ImportedFunctions::AttachConsole(DWORD dwProcessId)
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

DWORD ImportedFunctions::GetConsoleProcessList(LPDWORD lpdwProcessList,DWORD dwProcessCount)
{
	DWORD Ret=0;
	if(pGetConsoleProcessList)
	{
		Ret=pGetConsoleProcessList(lpdwProcessList,dwProcessCount);
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	}
	return Ret;
}
