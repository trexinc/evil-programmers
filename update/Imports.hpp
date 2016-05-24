#pragma once

#include "headers.hpp"

typedef BOOL (WINAPI *ATTACHCONSOLE)(DWORD dwProcessId);
typedef BOOL (WINAPI *ISWOW64PROCESS)(HANDLE hProcess,PBOOL Wow64Process);

struct ImportedFunctions
{
private:
	ATTACHCONSOLE pAttachConsole;
	ISWOW64PROCESS pIsWow64Process;

public:
	void Load();
	BOOL AttachConsole(DWORD dwProcessId) const;
	BOOL IsWow64Process(HANDLE hProcess,PBOOL Wow64Process) const;
};

extern ImportedFunctions ifn;
