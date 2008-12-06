#ifndef __syslog_cpp
#define __syslog_cpp

#include "syslog.hpp"

#ifdef _DEBUG
#include <windows.h>

char SysLogName[MAX_PATH*2];

// выводит строку типа "DD.MM.YYYY HH:MM:SS text"
void SysLog(char *fmt, ...)
{
  const static char SYSLOGEABLED[]="SYSLOGEABLED";
  static char temp[4096];

  static SYSTEMTIME st;
  GetLocalTime(&st);
  wsprintf(temp, "%02d.%02d.%04d %02d:%02d:%02d ",
           st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute, st.wSecond);
  static char *msg=temp+strlen(temp);

  va_list argptr;
  va_start(argptr, fmt);
  wvsprintf(msg, fmt, argptr);
  va_end(argptr);

  lstrcat(msg, "\r\n");
  HANDLE f =
    CreateFile(SysLogName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS,
               FILE_ATTRIBUTE_NORMAL, NULL);

  if (f != INVALID_HANDLE_VALUE)
    {
      DWORD dwBytesToWrite = strlen(temp), dwBytesWritten = 0;
      DWORD dwPos = SetFilePointer(f, 0, NULL, FILE_END);

      LockFile(f, dwPos, 0, dwPos + dwBytesToWrite, 0);
      WriteFile(f, temp, dwBytesToWrite, &dwBytesWritten, NULL);
      UnlockFile(f, dwPos, 0, dwPos + dwBytesToWrite, 0);
    }
  CloseHandle(f);
}

#endif

#endif // __syslog_cpp
