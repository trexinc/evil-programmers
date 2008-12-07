/*
    [ESC] Editor's settings changer plugin for FAR Manager
    Copyright (C) 2001 Ivan Sintyurin
    Copyright (C) 2008 Alex Yaroslavsky

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef __syslog_cpp
#define __syslog_cpp

#include "syslog.hpp"
#include "myrtl.hpp"

#ifdef _DEBUG
#include <windows.h>

wchar_t SysLogName[MAX_PATH*2];

// выводит строку типа "DD.MM.YYYY HH:MM:SS text"
void SysLog(const wchar_t *fmt, ...)
{
  const static wchar_t SYSLOGENABLED[]=L"SYSLOGENABLED";
  static wchar_t temp[4096];

  static SYSTEMTIME st;
  GetLocalTime(&st);
  wsprintf(temp, L"%02d.%02d.%04d %02d:%02d:%02d ",
           st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute, st.wSecond);
  static wchar_t *msg=temp+wstrlen(temp);

  va_list argptr;
  va_start(argptr, fmt);
  wvsprintf(msg, fmt, argptr);
  va_end(argptr);

  wstrcat(msg, L"\r\n");
  HANDLE f =
    CreateFile(SysLogName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS,
               FILE_ATTRIBUTE_NORMAL, NULL);

  if (f != INVALID_HANDLE_VALUE)
    {
      DWORD dwBytesToWrite = wstrlen(temp)*sizeof(wchar_t), dwBytesWritten = 0;
      DWORD dwPos = SetFilePointer(f, 0, NULL, FILE_END);

      LockFile(f, dwPos, 0, dwPos + dwBytesToWrite, 0);
      WriteFile(f, temp, dwBytesToWrite, &dwBytesWritten, NULL);
      UnlockFile(f, dwPos, 0, dwPos + dwBytesToWrite, 0);
    }
  CloseHandle(f);
}

#endif

#endif // __syslog_cpp
