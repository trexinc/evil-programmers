#pragma once

class imported_functions
{
public:
	NONCOPYABLE(imported_functions);
	imported_functions() = default;

	void Load();
	BOOL AttachConsole(DWORD dwProcessId) const;
	BOOL IsWow64Process(HANDLE hProcess, PBOOL Wow64Process) const;

private:
	using ATTACHCONSOLE = BOOL(WINAPI*)(DWORD);
	using ISWOW64PROCESS = BOOL(WINAPI*)(HANDLE, PBOOL);

	ATTACHCONSOLE pAttachConsole;
	ISWOW64PROCESS pIsWow64Process;
};

extern imported_functions ifn;
