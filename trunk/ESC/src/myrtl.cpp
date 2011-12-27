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
#ifndef __myrtl_cpp
#define __myrtl_cpp

#include <CRT/crt.hpp>
#include <windows.h>
#include "myrtl.hpp"

FILE *wfopen(const wchar_t *filename, const wchar_t *mode)
{
   if(!mode || !filename || !*filename)
      return NULL;
   //_D(SysLog("fopen: filename[%s], mode[%s] strpbrk%c=NULL",
   //   filename, mode, strpbrk(mode, "rwa")?'!':'='));
   if(wcspbrk(mode, L"rwa")==NULL)
      return NULL;
   DWORD dwDesiredAccess=0, dwCreationDistribution=0;
   BOOL R, W, A;
   R=W=A=FALSE;
/*
r   Open for reading only.
w   Create for writing. If a file by that name already exists, it will be
    overwritten.
a   Append; open for writing at end-of-file or create for writing if the file
    does not exist.
r+  Open an existing file for update (reading and writing).
w+  Create a new file for update (reading and writing). If a file by that name
    already exists, it will be overwritten.
a+  Open for append; open (or create if the file does not exist) for update at
    the end of the file.
*/
   if(wcschr(mode, L'r'))
   {
     //_D(SysLog("r - GENERIC_READ"));
     dwDesiredAccess|=GENERIC_READ;
     dwCreationDistribution=OPEN_EXISTING;
     R=TRUE;
   }
   if(wcschr(mode, L'w'))
   {
     //_D(SysLog("w - GENERIC_WRITE"));
     dwDesiredAccess|=GENERIC_WRITE;
     dwCreationDistribution=CREATE_ALWAYS;
     W=TRUE;
   }
   if(wcschr(mode, L'a'))
   {
     //_D(SysLog("a - GENERIC_WRITE"));
     dwDesiredAccess|=GENERIC_WRITE;
     dwCreationDistribution=OPEN_ALWAYS;
     A=TRUE;
   }
   if(wcschr(mode, L'+'))
   {
     //_D(SysLog("+++"));
     dwDesiredAccess|=GENERIC_WRITE;
     dwCreationDistribution=CREATE_ALWAYS;
     if(R) dwDesiredAccess|=GENERIC_WRITE;
     if(W) dwDesiredAccess|=GENERIC_READ;
   }

   HANDLE hFile=CreateFile(filename, dwDesiredAccess, FILE_SHARE_READ, NULL,
                           dwCreationDistribution, 0, NULL);
   //_D(SysLog("hFile%c=INVALID_HANDLE_VALUE", hFile==INVALID_HANDLE_VALUE?'=':'!'));
   if(hFile==INVALID_HANDLE_VALUE) return NULL;
   if(A) fseek((FILE *)hFile, 0, SEEK_END);
   return (FILE *)hFile;
};

int fseek(FILE *stream, long offset, int whence)
{
   return SetFilePointer(stream, offset, NULL, whence)==0xFFFFFFFF;
}

long int ftell(FILE *stream)
{
   return SetFilePointer(stream, 0, NULL, FILE_CURRENT);
}

int fclose(FILE *stream)
{
   return (stream!=NULL && stream!=INVALID_HANDLE_VALUE)?
          CloseHandle(stream):TRUE;
}

size_t fread(void *ptr, size_t size, size_t n, FILE *stream)
{
   DWORD NumberOfBytesToRead=(DWORD)(size*n), NumberOfBytesRead;
   if(!ptr || !size || !n) return 0;
   BOOL r=ReadFile(stream, ptr, NumberOfBytesToRead, &NumberOfBytesRead, NULL);
   return (!r || NumberOfBytesRead!=NumberOfBytesToRead)?0:n;
}

size_t fwrite(const void *ptr, size_t size, size_t n, FILE *stream)
{
   DWORD NumberOfBytesToWrite=(DWORD)(n*size), NumberOfBytesWritten;
   if(!ptr || !size || !n) return 0;
   BOOL w=WriteFile(stream, ptr, NumberOfBytesToWrite, &NumberOfBytesWritten,
                    NULL);
   return (!w || NumberOfBytesWritten!=NumberOfBytesToWrite)?0:n;
}

#endif //__myrtl_cpp
