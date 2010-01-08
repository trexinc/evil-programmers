/*
  Copyright (C) 2001 by Nikolay Redko
  Copyright (C) 2010 by Alex Yaroslavsky

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

#include <stdio.h>
#include <windows.h>
#include "makeit.h"

#define BUFSIZE 4096

static HANDLE hChildStdoutRd, hChildStdoutWr;

BOOL CreateChildProcess(const char*);
int ReadFromPipe(void);
extern void AddStrings(void*,DWORD);

int
pipesrv(const char* command)
{
    SECURITY_ATTRIBUTES attr;
    attr.nLength = sizeof(SECURITY_ATTRIBUTES);
    attr.bInheritHandle = TRUE;
    attr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &attr, 0))
        return MPipeError;

    if (!CreateChildProcess(command))
    {
        CloseHandle(hChildStdoutWr);
        CloseHandle(hChildStdoutRd);
        return MProcessError;
    }

    CloseHandle(hChildStdoutWr);

    ReadFromPipe();

    CloseHandle(hChildStdoutRd);

    return 0;
}

BOOL CreateChildProcess(const char*command)
{
    PROCESS_INFORMATION pi = {0};
    STARTUPINFO si = {0};

    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = NULL;
    si.lpReserved2 = NULL;
    si.cbReserved2 = 0;
    si.lpDesktop = NULL;
    si.lpTitle = NULL;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdError = hChildStdoutWr;
    si.hStdOutput = hChildStdoutWr;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    BOOL ret = CreateProcess(NULL,const_cast<char*>(command),NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);

    if (ret)
    {
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    }

    return ret;
}

int ReadFromPipe(void)
{
    DWORD dwRead;
    char buf[BUFSIZE];
    for (;;)
    {
        if (!ReadFile(hChildStdoutRd, buf, BUFSIZE, &dwRead, NULL) || !dwRead)
            break;
        AddStrings(buf,dwRead);
    }
    return 0;
}
