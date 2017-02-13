#pragma once

struct imported_functions
{
private:
	using ATTACHCONSOLE = BOOL(WINAPI*)(DWORD);
	using ISWOW64PROCESS = BOOL(WINAPI*)(HANDLE, PBOOL);

	ATTACHCONSOLE pAttachConsole;
	ISWOW64PROCESS pIsWow64Process;

public:
	void Load();
	BOOL AttachConsole(DWORD dwProcessId) const;
	BOOL IsWow64Process(HANDLE hProcess, PBOOL Wow64Process) const;
};

extern imported_functions ifn;
