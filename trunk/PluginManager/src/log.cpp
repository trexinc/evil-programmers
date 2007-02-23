/*
    PluginManager plugin for FAR Manager
    Copyright (C) 2003-2005 Alex Yaroslavsky

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
//#define __AAAA__
#ifdef __AAAA__

HANDLE OpenLog(char *FileName)
{
  HANDLE fh=CreateFile(FileName,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,0,NULL);
  return fh;
}

void WriteLog(HANDLE fh, const char *line)
{
  if(fh!=INVALID_HANDLE_VALUE)
  {
    DWORD t; LONG dist=0;
    //WaitForSingleObject(LogMutex,INFINITE);
    if((SetFilePointer(fh,0,&dist,FILE_END)!=0xFFFFFFFF)||(GetLastError()==NO_ERROR)) WriteFile(fh,line,lstrlen(line),&t,NULL);
    //ReleaseMutex(LogMutex);
  }
}

void CloseLog(HANDLE *fh)
{
  if(fh!=INVALID_HANDLE_VALUE)
  {
    CloseHandle(*fh);
    *fh=INVALID_HANDLE_VALUE;
  }
}

#else

#define OpenLog(a) INVALID_HANDLE_VALUE;
#define CloseLog(a)
#define WriteLog(a,b)

#endif
