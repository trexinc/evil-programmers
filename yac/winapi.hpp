#pragma once
#include "headers.hpp"
#include "unicodestring.hpp"

class NTPath
{
public:
	string Str;
	NTPath(const wchar_t* Src);

	operator const wchar_t*() const
	{
		return Str;
	}
};

HANDLE apiCreateFile(const wchar_t* lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
HANDLE apiFindFirstFileEx(const wchar_t* lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, void* lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, void* lpSearchFilter, DWORD dwAdditionalFlags);
DWORD apiGetFileAttributes(const wchar_t* lpFileName);
