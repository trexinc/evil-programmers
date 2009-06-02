#ifndef __ENV_HPP__
#define __ENV_HPP__

#define ENV_NAME_MAXLEN 1024
#define ENV_VALUE_MAXLEN 1024

typedef HINSTANCE (WINAPI *PROC_LOADLIBRARY) (LPCTSTR lpLibFileName);
typedef FARPROC (WINAPI *PROC_GETPROCADDRESS) (HMODULE hModule, LPCSTR lpProcName);
typedef DWORD (WINAPI *PROC_GETENVIRONMENTVARIABLE) (LPCTSTR lpName, LPTSTR lpBuffer, DWORD nSize);
typedef BOOL (WINAPI *PROC_SETENVIRONMENTVARIABLE) (LPCTSTR lpName, LPCTSTR lpValue);
typedef VOID (WINAPI *PROC_EXITTHREAD) (DWORD dwExitCode);
typedef BOOL (WINAPI *PROC_WRITEPROCESSMEMORY) (HANDLE hProcess, LPVOID lpBaseAddress, LPVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesWritten);
typedef DWORD (WINAPI *PROC_GETLASTERROR) (VOID);
typedef HANDLE (WINAPI *PROC_OPENPROCESS)(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
typedef LPVOID (WINAPI *PROC_VIRTUALALLOCEX)(HANDLE hProcess, LPVOID lpAddress, DWORD dwSize, DWORD flAllocationType, DWORD flProtect);
typedef DWORD (WINAPI *PROC_GETLASTERROR)(VOID);

typedef enum tagENVWHATDO
{
  ENV_SET,
  ENV_GET
} ENVWHATDO;

typedef struct tagREMOTEINFO
{
    PROC_LOADLIBRARY LoadLibrary;
    PROC_GETPROCADDRESS GetProcAddress;
    TCHAR Kernel32[16];
    CHAR GetEnvironmentVariable[32];
    CHAR SetEnvironmentVariable[32];
    CHAR ExitThread[16];
    CHAR WriteProcessMemory[32];
    CHAR OpenProcess[32];
    CHAR GetLastError[32];

    TCHAR Name[ENV_NAME_MAXLEN];
    TCHAR Value[ENV_VALUE_MAXLEN];
    ENVWHATDO WhatDo;
    LPVOID VarValue;
    DWORD ProcessID;
} REMOTEINFO, *PREMOTEINFO;

typedef struct tagENVVARIABLE
{
  TCHAR Name[ENV_NAME_MAXLEN];
  TCHAR Value[ENV_VALUE_MAXLEN];
  ENVWHATDO WhatDo;
  LPVOID VarValue;
  DWORD ProcessID;
} ENVVARIABLE, *PENVVARIABLE;

DWORD SetProcessEnv(PPROCESSENTRY32 ProcessEntry, PENVVARIABLE EnvVariable);

#endif /* __ENV_HPP__ */