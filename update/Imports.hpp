#pragma once

#include "headers.hpp"

typedef BOOL (WINAPI *ATTACHCONSOLE)(DWORD dwProcessId);
typedef DWORD (APIENTRY *GETCONSOLEPROCESSLIST)(LPDWORD lpdwProcessList,DWORD dwProcessCount);

struct ImportedFunctions
{
private:
	ATTACHCONSOLE pAttachConsole;
	GETCONSOLEPROCESSLIST pGetConsoleProcessList;
public:
	VOID Load();
	BOOL AttachConsole(DWORD dwProcessId);
	DWORD GetConsoleProcessList(LPDWORD lpdwProcessList,DWORD dwProcessCount);
};

extern ImportedFunctions ifn;
