#include "stdafx.h"
#include "common.hpp"

LPTSTR MyStrChr(LPCTSTR string, TCHAR ch)
{
  while (*string && *string != (TCHAR)ch)
    string++;

  if (*string == (TCHAR)ch)
    return((LPTSTR)string);

  return(NULL);
}

INT MyStrChrPos(LPCTSTR string, TCHAR ch)
{
  INT pos = 0;
  while (*string && *string != (TCHAR)ch)
  {
    string++;
    pos++;
  }

  if (*string == (TCHAR)ch)
    return(pos);

  return(0);
}

BOOL Val(LPCTSTR lpSrc, PINT Result)
{
#define ISDIGIT(c)  ((c) >= TEXT('0') && (c) <= TEXT('9'))

  BOOL bRes = FALSE;

  INT n = 0;
  BOOL bNeg = FALSE;

  if (*lpSrc == TEXT('-'))
  {
    bNeg = TRUE;
    lpSrc++;
  }

  while (ISDIGIT(*lpSrc))
  {
    bRes = TRUE;
    n *= 10;
    n += *lpSrc - TEXT('0');
    lpSrc++;
  }

  if (bRes)
  {
    *Result = (bNeg ? -n : n);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

VOID MyWriteConsoleA(LPCSTR ConBuf, BOOL fUnicode, BOOL fOutOrErr)
{
#ifdef UNICODE
  INT len = MultiByteToWideChar(CP_OEMCP, 0, ConBuf, -1, NULL, 0);
  LPWSTR UnicodeBuf = (LPWSTR)malloc(len * sizeof(WCHAR));
  if (UnicodeBuf)
  {
    len = MultiByteToWideChar(CP_OEMCP, 0, ConBuf, -1, UnicodeBuf, len);
    MyWriteConsole(UnicodeBuf, fUnicode, fOutOrErr);
    free(UnicodeBuf);
  }
#else
  MyWriteConsole(ConBuf, fUnicode, fOutOrErr);
#endif
}

VOID MyWriteConsole(LPCTSTR ConBuf, BOOL fUnicode, BOOL fOutOrErr)
{
  HANDLE hOut;
  DWORD NumberOfCharsWritten;
  INT len = lstrlen(ConBuf);

  if (fOutOrErr == 1)
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  else
    hOut = GetStdHandle(STD_ERROR_HANDLE);

  if (fUnicode)
  {
    WriteFile(hOut, ConBuf, len * sizeof(WCHAR), &NumberOfCharsWritten, NULL);
  }
  else
  {
    DWORD hType = GetFileType(hOut);
    hType &= ~FILE_TYPE_REMOTE;

    if (hType == FILE_TYPE_CHAR)
    {
      WriteConsole(hOut, ConBuf, len, &NumberOfCharsWritten, NULL);
    }
    else
    {
#ifdef UNICODE
      PCHAR AnsiBuf = (PCHAR)malloc(len);
      if (AnsiBuf)
      {
        len = WideCharToMultiByte(CP_OEMCP, 0, ConBuf, len, AnsiBuf, len, NULL, NULL);
        WriteFile(hOut, AnsiBuf, len, &NumberOfCharsWritten, NULL);
        free(AnsiBuf);
      }
#else
      WriteFile(hOut, ConBuf, len, &NumberOfCharsWritten, NULL);
#endif
    }
  }
}

LPTSTR GetErrorMessage(DWORD MessageId)
{
  LPTSTR Message, s;

  Message = NULL;
  FormatMessage(
    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
    NULL, MessageId,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR)&Message, 0,
    NULL);

  if (Message == NULL)
  {
    Message = (LPTSTR)LocalAlloc(0, 128);
    wsprintf(Message, _TEXT("Unable to get message for %08x"), MessageId);
  }
  else
  {
    s = MyStrChr(Message, TEXT('\r'));
    if (!s)
      s = MyStrChr(Message, TEXT('\n'));

    if (s)
      *s = TEXT('\0');
  }
  return Message;
}

DWORD GetParentProcessID(DWORD dwPID)
{
  NTSTATUS ntStatus;
  DWORD dwParentPID = 0xffffffff;

  HANDLE  hProcess;
  PROCESS_BASIC_INFORMATION pbi;
  ULONG ulRetLen;

  typedef NTSTATUS (__stdcall *FPTR_NtQueryInformationProcess)
    (HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

  // create entry point for 'NtQueryInformationProcess()'
  FPTR_NtQueryInformationProcess NtQueryInformationProcess =
    (FPTR_NtQueryInformationProcess ) GetProcAddress (GetModuleHandleA ( "ntdll"), "NtQueryInformationProcess");

  // get process handle
  hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPID);

  // could fail due to invalid PID or insufficiant privileges
  if (!hProcess)
    return (0);

  //  gather information
  ntStatus = NtQueryInformationProcess(hProcess,
                                       ProcessBasicInformation,
                                       (void*) &pbi,
                                       sizeof(PROCESS_BASIC_INFORMATION),
                                       &ulRetLen
                                      );

  //  copy PID on success
  if (!ntStatus)
    dwParentPID = pbi.InheritedFromUniqueProcessId;

  CloseHandle (hProcess);

  return (dwParentPID);
}
