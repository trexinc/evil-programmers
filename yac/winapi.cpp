#include "winapi.hpp"

string& ReplaceSlashToBSlash(string& strStr)
{
	wchar_t* lpwszStr=strStr.GetBuffer();
	while(*lpwszStr)
	{
		if(*lpwszStr==L'/')
		{
			*lpwszStr=L'\\';
		}
		lpwszStr++;
	}
	strStr.ReleaseBuffer(strStr.GetLength());
	return strStr;
}

bool PathPrefix(const wchar_t* Path)
{
/*
	\\?\
	\\.\
	\??\
*/
	return Path && Path[0] == L'\\' && (Path[1] == L'\\' || Path[1] == L'?') && (Path[2] == L'?' || Path[2] == L'.') && Path[3] == L'\\';
}

bool IsLocalPath(const wchar_t* Path)
{
	return Path && *Path && Path[1]==L':';
}

NTPath::NTPath(const wchar_t* Src)
{
	if(Src&&*Src)
	{
		Str=Src;
		ReplaceSlashToBSlash(Str);
		if(!PathPrefix(Src))
		{
			int Size=FSF.ConvertPath(CPM_FULL,Str,NULL,0);
			if(Size)
			{
				wchar_t* FullPath=new wchar_t[Size];
				if(FullPath)
				{
					FSF.ConvertPath(CPM_FULL,Str,FullPath,Size);
					Str=FullPath;
					delete[] FullPath;
				}
			}
			if(!PathPrefix(Str))
			{
				if(IsLocalPath(Str))
				{
					Str=string(L"\\\\?\\")+Str;
				}
				else
				{
					Str=string(L"\\\\?\\UNC\\")+&Str[2];
				}
			}
		}
	}
}

HANDLE apiCreateFile(const wchar_t* lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	if(dwCreationDisposition==OPEN_EXISTING)
	{
		dwFlagsAndAttributes|=FILE_FLAG_POSIX_SEMANTICS;
	}
	dwFlagsAndAttributes|=FILE_FLAG_BACKUP_SEMANTICS;
	string strName(NTPath(lpFileName).Str);
	HANDLE hFile=CreateFile(strName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	DWORD Error=GetLastError();
	if(hFile==INVALID_HANDLE_VALUE && (Error==ERROR_FILE_NOT_FOUND||Error==ERROR_PATH_NOT_FOUND))
	{
		dwFlagsAndAttributes&=~FILE_FLAG_POSIX_SEMANTICS;
		hFile=CreateFile(strName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	}
	return hFile;
}

HANDLE apiFindFirstFileEx(const wchar_t* lpFileName,FINDEX_INFO_LEVELS fInfoLevelId,void* lpFindFileData,FINDEX_SEARCH_OPS fSearchOp,void* lpSearchFilter,DWORD dwAdditionalFlags)
{
	return FindFirstFileExW(NTPath(lpFileName),fInfoLevelId,lpFindFileData,fSearchOp,lpSearchFilter,dwAdditionalFlags);
}

DWORD apiGetFileAttributes(const wchar_t* lpFileName)
{
	return GetFileAttributesW(NTPath(lpFileName));
}
