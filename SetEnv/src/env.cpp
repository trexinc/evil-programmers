#include "stdafx.h"
#include "env.hpp"

/* Процедура внедряемая в процесс */
static VOID WINAPI RemoteThread(PVOID RemoteInfo);
static LPVOID InjectMemory(HANDLE Process, LPVOID Memory, DWORD Size);

/* Процедура внедряемая в процесс */
static VOID WINAPI RemoteThread(PVOID RemoteInfo)
{
  REMOTEINFO info = *(PREMOTEINFO)RemoteInfo;
  HINSTANCE hKernel = info.LoadLibrary(info.Kernel32);

  PROC_GETENVIRONMENTVARIABLE _GetEnvironmentVariable =
    (PROC_GETENVIRONMENTVARIABLE)info.GetProcAddress(hKernel, info.GetEnvironmentVariable);
  PROC_SETENVIRONMENTVARIABLE _SetEnvironmentVariable =
    (PROC_SETENVIRONMENTVARIABLE)info.GetProcAddress(hKernel, info.SetEnvironmentVariable);
  PROC_EXITTHREAD _ExitThread =
    (PROC_EXITTHREAD)info.GetProcAddress(hKernel, info.ExitThread);
  PROC_WRITEPROCESSMEMORY _WriteProcessMemory =
    (PROC_WRITEPROCESSMEMORY)info.GetProcAddress(hKernel, info.WriteProcessMemory);
  PROC_OPENPROCESS _OpenProcess =
    (PROC_OPENPROCESS)info.GetProcAddress(hKernel, info.OpenProcess);
  PROC_GETLASTERROR _GetLastError =
    (PROC_GETLASTERROR)info.GetProcAddress(hKernel, info.GetLastError);

  DWORD res = FALSE;
  HANDLE hProcess;
  DWORD BytesWritten;

  switch (info.WhatDo)
  {
    case ENV_SET:
      _SetEnvironmentVariable(info.Name, info.Value);
      break;
    case ENV_GET:
      TCHAR buf[256];
      res = _GetEnvironmentVariable(info.Name, buf, sizeof(buf) / sizeof(TCHAR));
      if (res)
      {
        hProcess = (HANDLE)_OpenProcess(
          PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
          FALSE, info.ProcessID);
        if (hProcess)
          res = _WriteProcessMemory(hProcess, info.VarValue, buf, (res + 1) * sizeof(TCHAR), &BytesWritten);
      }
      break;
  }
  res = _GetLastError();
  _ExitThread(res);
};

static LPVOID InjectMemory(HANDLE hProcess, LPVOID Memory, DWORD Size)
{
  DWORD BytesWritten;
  LPVOID pMem;

  pMem = VirtualAllocEx(hProcess, NULL, Size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if (pMem)
    WriteProcessMemory(hProcess, pMem, Memory, Size, &BytesWritten);

  return pMem;
}

DWORD SetProcessEnv(PPROCESSENTRY32 ProcessEntry, PENVVARIABLE EnvVariable)
{
  REMOTEINFO RemoteInfo;
  LPVOID pInfo = NULL;
  LPVOID CodeAdr = NULL;
  DWORD TID;
  HANDLE hProcess;
  DWORD ExitCode = 0;

  hProcess = (HANDLE)OpenProcess(
    PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
    PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
    FALSE, ProcessEntry->th32ProcessID);
  if (!hProcess)
    goto anyerror;

#ifdef UNICODE
#define LoadLibraryName "LoadLibraryW"
#define GetEnvironmentVariableName "GetEnvironmentVariableW"
#define SetEnvironmentVariableName "SetEnvironmentVariableW"
#else
#define LoadLibraryName "LoadLibraryA"
#define GetEnvironmentVariableName "GetEnvironmentVariableA"
#define SetEnvironmentVariableName "SetEnvironmentVariableA"
#endif

  //Заполняем структуру передаваемую внедряемому коду
  lstrcpy(RemoteInfo.Kernel32, _TEXT("kernel32.dll"));
  lstrcpyA(RemoteInfo.GetEnvironmentVariable, GetEnvironmentVariableName);
  lstrcpyA(RemoteInfo.SetEnvironmentVariable, SetEnvironmentVariableName);
  lstrcpyA(RemoteInfo.ExitThread, "ExitThread");
  lstrcpyA(RemoteInfo.WriteProcessMemory, "WriteProcessMemory");
  lstrcpyA(RemoteInfo.OpenProcess, "OpenProcess");
  lstrcpyA(RemoteInfo.GetLastError, "GetLastError");
  lstrcpyn(RemoteInfo.Name, EnvVariable->Name, sizeof(REMOTEINFO().Name));
  lstrcpyn(RemoteInfo.Value, EnvVariable->Value, sizeof(REMOTEINFO().Value));
  RemoteInfo.WhatDo = EnvVariable->WhatDo;
  RemoteInfo.VarValue = EnvVariable->VarValue;
  RemoteInfo.ProcessID = EnvVariable->ProcessID;

  //получаем адреса используемых API
  RemoteInfo.LoadLibrary = (PROC_LOADLIBRARY)GetProcAddress(
    GetModuleHandle(_TEXT("kernel32.dll")), LoadLibraryName);
  if (!RemoteInfo.LoadLibrary)
    goto anyerror;

  RemoteInfo.GetProcAddress = (PROC_GETPROCADDRESS)GetProcAddress(
    GetModuleHandle(_TEXT("kernel32.dll")), "GetProcAddress");
  if (!RemoteInfo.GetProcAddress)
    goto anyerror;

  //копируем в процесс структуру с данными
  pInfo = InjectMemory(hProcess, &RemoteInfo, sizeof(REMOTEINFO));

  //копируем в процесс внедряемый код
  CodeAdr = InjectMemory(hProcess, &RemoteThread, 0x1000);


  //запускаем внедренный код
  if (pInfo && CodeAdr)
  {
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)CodeAdr, pInfo, 0, &TID);

    if (hThread)
    {
      DWORD Res = WaitForSingleObject(hThread, INFINITE);

      if (Res == WAIT_OBJECT_0)
      {
        if (GetExitCodeThread(hThread, &ExitCode))
        {
          VirtualFreeEx(hProcess, pInfo, 0, MEM_RELEASE);
          VirtualFreeEx(hProcess, CodeAdr, 0, MEM_RELEASE);

          return (ExitCode);
        }
      }
    }
  }
anyerror:
  return (GetLastError());
}

