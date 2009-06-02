#include "stdafx.h"
#include "setenv.hpp"
#include "cmd.hpp"
#include "find.hpp"
#include "env.hpp"
#include "match.hpp"
#include "common.hpp"

ENVVARIABLE EnvVariable;
LPTSTR OutputText = NULL;

INT SoughtCount = 0;
SoughtProcessItem *SoughtList = NULL;

static VOID DynStrAdd(LPTSTR & str, LPCTSTR value);
static BOOL SetOutputStr(PPROCESSENTRY32 Process, PENVVARIABLE pEnv, DWORD ExitCode, BOOL bVerbose);
static VOID Usage(BOOL fUnicode);

static VOID DynStrAdd(LPTSTR & str, LPCTSTR value)
{
  CONST DWORD step = 1024;

  DWORD len = (lstrlen(str) + lstrlen(value) + 1) * sizeof(TCHAR);

  DWORD size = memsize(str);
  if (len > size)
    str = (PTCHAR)realloc(str, size + (len / step + 1) * step);

  if (str)
    lstrcat(str, value);
  else
    lstrcpy(str, value);
}

static BOOL SetOutputStr(PPROCESSENTRY32 Process, PENVVARIABLE pEnv, DWORD ExitCode, BOOL bVerbose)
{
  BOOL Result = FALSE;
  TCHAR VerboseItemStr[1024];
  TCHAR ValueBuf[sizeof(ENVVARIABLE().Value)];
  TCHAR ItemBuf[sizeof(ENVVARIABLE().Name) +
                sizeof(ENVVARIABLE().Value) +
                sizeof(_TEXT("=\r\n")) +
                sizeof(TCHAR)];

  if (bVerbose)
  {
    LPTSTR lpMsgBuf = GetErrorMessage(ExitCode);
    DWORD len = lstrlen(lpMsgBuf);

    PCHAR OemMsgBuf = (PCHAR)malloc(len);
    if (OemMsgBuf)
    {
      CharToOemBuff(lpMsgBuf, OemMsgBuf, len);
      MultiByteToWideChar(CP_OEMCP, 0, OemMsgBuf, -1, (LPWSTR)lpMsgBuf, len);

      TCHAR FormatStr[256];
      lstrcpy(FormatStr, _TEXT("%s ENV for (PID=0x%.8lX, ImageName=\"%s\") -> %s"));
      if (ExitCode != 0)
        lstrcat(FormatStr, _TEXT("\r\n%d: %s"));
      lstrcat(FormatStr, _TEXT("\r\n"));

      wsprintf(VerboseItemStr, FormatStr,
        (pEnv->WhatDo == ENV_SET ? _TEXT("Set") : _TEXT("Get")),
        Process->th32ProcessID, Process->szExeFile,
        (ExitCode ? _TEXT("failed") : _TEXT("ok")),
        ExitCode,
        (LPTSTR)lpMsgBuf
      );

      DynStrAdd(OutputText, VerboseItemStr);
      LocalFree(lpMsgBuf);
      free(OemMsgBuf);
      Result = TRUE;
    }
  }

  if (!ExitCode && pEnv->WhatDo == ENV_GET)
  {
    DWORD BytesRead;
    ReadProcessMemory(GetCurrentProcess(), pEnv->VarValue, ValueBuf, sizeof(ENVVARIABLE().Value), &BytesRead);
    wsprintf(ItemBuf, _TEXT("%s=%s\r\n"), pEnv->Name, ValueBuf);
    DynStrAdd(OutputText, ItemBuf);
    Result = TRUE;
  }
  return Result;
}

static VOID Usage(BOOL fUnicode)
{
  CHAR buf[] =
    "SETENV variable[=string] [/PID processid ...] [/IM imagename ...]\r\n"
    "                         [/P [depth]] [/V] [/U] [/W]\r\n"
    "\r\n"
    "Description:\r\n"
    "  Displays, sets, or removes environment variables for any processes\r\n"
    "\r\n"
    "Parameters list:\r\n"
    "  variable   Specifies the environment-variable name.\r\n"
    "  string     Specifies a series of characters to assign to the variable.\r\n"
    "  processid  Specifies the process ID of the process.\r\n"
    "  imagename  Specifies the image name of the process.\r\n"
    "  /P depth   Specifies processing only chains of parental processes\r\n"
    "             with optional limitation of depth.\r\n"
    "  /V         Specifies that verbose information be displayed in the output.\r\n"
    "  /U         Specifies that output to a pipe or file to be Unicode.\r\n"
    "  /W         Specifies to wait for pressing any key after execution.\r\n"
    "\r\n"
    "NOTE:\r\n"
    "  Wildcards '*', '?' can be used to specify imagename and processid.\r\n"
    "\r\n"
    "Examples:\r\n"
    "  SETENV TMP=c:\\temp /IM Far.exe /PID 716 2???\r\n"
    "  SETENV \"path=%path%;d:\\tools\" /IM *\r\n";

  MyWriteConsoleA(&buf[0], fUnicode, TRUE);
}

static VOID AddSoughtItem(BOOL UseMask, SoughtProcessType Type, TCHAR Name[MAX_PATH])
{
  SoughtList = (SoughtProcessItem*)realloc(SoughtList, ++SoughtCount * sizeof(SoughtProcessItem));
  if (SoughtList)
  {
    SoughtList[SoughtCount - 1].UseMask = UseMask;
    SoughtList[SoughtCount - 1].Type = Type;
    lstrcpyn(SoughtList[SoughtCount - 1].Name, Name, sizeof(SoughtProcessItem().Name)/sizeof(TCHAR));
  }
}

MyMain()
{
  ProcessCmdLine();

  INT p = 0;

  BOOL DoNothing = TRUE;

  BOOL FindInParents = FALSE;
  INT ParentsMaxLevel = -1;

  INT AllProcessesCount;
  PROCESSENTRY32 *AllProcessesList;

  INT ProcessesCount;
  PROCESSENTRY32 *ProcessesList;

  BOOL DoUnicodeOutput = CheckParam(_TEXT("U")) != 0;
  BOOL DoVerbose = CheckParam(_TEXT("V")) != 0;
  BOOL DoWait = CheckParam(_TEXT("W")) != 0;

  if (myargc > 2 && !IsParamPrefix(*myargv[1]))
  {
    // По имени процесса
    p = CheckParam(_TEXT("IM"));
    if (p)
    {
      while (++p != myargc && !IsParamPrefix(*myargv[p]))
        AddSoughtItem(TRUE, SEARCH_BY_IMAGENAME, myargv[p]);
    }

    // По идентификатору процесса
    p = CheckParam(_TEXT("PID"));
    if (p)
    {
      while (++p != myargc && !IsParamPrefix(*myargv[p]))
        AddSoughtItem(TRUE, SEARCH_BY_PROCESSID, myargv[p]);
    }
  }

  if (SoughtCount)
  {
    INT EqualPos = MyStrChrPos(myargv[1], TEXT('='));

    if (EqualPos)
    {
      EnvVariable.WhatDo = ENV_SET;
      lstrcpyn(EnvVariable.Name, myargv[1], EqualPos + 1);
      lstrcpyn(EnvVariable.Value, &myargv[1][EqualPos + 1], sizeof(EnvVariable.Value)/sizeof(TCHAR));
    }
    else
    {
       EnvVariable.WhatDo = ENV_GET;
       lstrcpyn(EnvVariable.Name, myargv[1], sizeof(EnvVariable.Name)/sizeof(TCHAR));
       EnvVariable.Value[0] = TEXT('\0');
    }

    // Поиск только в цепочке родительских процессов
    p = CheckParam(_TEXT("P"));
    FindInParents = (p != 0);
    ParentsMaxLevel = 32767;
    if (p && p < myargc - 1)
      Val(myargv[p + 1], &ParentsMaxLevel);

    AllProcessesCount = GetAllProcessesList(NULL);
    AllProcessesList = (PROCESSENTRY32*)malloc(AllProcessesCount * sizeof(PROCESSENTRY32));
    GetAllProcessesList(AllProcessesList);

    if (FindInParents)
    {
      ProcessesCount = 0;
      ProcessesList = NULL;
      DWORD CurrProcess = GetCurrentProcessId();
      while (CurrProcess && ProcessesCount <= ParentsMaxLevel)
      {
        for (INT i = 0; i < AllProcessesCount; i++)
        {
          if (CurrProcess == AllProcessesList[i].th32ProcessID)
          {
            ProcessesList = (PROCESSENTRY32*)realloc(ProcessesList, (ProcessesCount + 1) * sizeof(PROCESSENTRY32));
            ProcessesList[ProcessesCount] = AllProcessesList[i];
            ProcessesCount++;
            break;
          }
        }
        CurrProcess = GetParentProcessID(CurrProcess);
      }
    }
    else
    {
      ProcessesList = AllProcessesList;
      ProcessesCount = AllProcessesCount;
    }

    if (ProcessesList)
    {
      EnvVariable.ProcessID = GetCurrentProcessId();
      EnvVariable.VarValue = VirtualAllocEx(GetCurrentProcess(), NULL, sizeof(ENVVARIABLE().Value), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
      if (EnvVariable.VarValue)
      {
        for (INT i = 0; i < ProcessesCount; i++)
        {
          for (INT j = 0; j < SoughtCount; j++)
          {
            TCHAR str[MAX_PATH];

            switch (SoughtList[j].Type)
            {
            case SEARCH_BY_IMAGENAME:
              wsprintf(str, _TEXT("%s"), ProcessesList[i].szExeFile);
              break;
            case SEARCH_BY_PROCESSID:
              wsprintf(str, _TEXT("%d"), ProcessesList[i].th32ProcessID);
              break;
            }

            if ( !SoughtList[j].Name ||
                 (!SoughtList[j].UseMask && lstrcmpi(str, SoughtList[j].Name) == 0) ||
                 (SoughtList[j].UseMask && Match(SoughtList[j].Name, str)))
            {
              DWORD ExitCode = SetProcessEnv(&ProcessesList[i], &EnvVariable);
              if (SetOutputStr(&ProcessesList[i], &EnvVariable, ExitCode, DoVerbose))
                DoNothing = FALSE;
            }
          }
        }
        VirtualFreeEx(GetCurrentProcess(), EnvVariable.VarValue, 0, MEM_RELEASE);
      }
      if (DoVerbose && DoNothing)
      {
        CHAR buf[] = "Nothing to do...\n";
        MyWriteConsoleA(buf, DoUnicodeOutput, TRUE);
      }
    }
    if (OutputText)
      MyWriteConsole(OutputText, DoUnicodeOutput, TRUE);

  }
  else
  {
    Usage(DoUnicodeOutput);
  }

  if (DoWait)
  {
    INPUT_RECORD InputRecord;
    DWORD NumberOfEventsRead;
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);

    FlushConsoleInputBuffer(hInput);
    do
    {
      ReadConsoleInput(hInput, &InputRecord, 1, &NumberOfEventsRead);
      Sleep(10);
    }
    while (InputRecord.EventType != KEY_EVENT);
  }

  return TRUE;
}
