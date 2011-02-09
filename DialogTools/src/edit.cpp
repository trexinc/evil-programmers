/*
    edit.cpp
    Copyright (C) 2009 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "dt.hpp"
#include "guid.hpp"
#include <stdio.h>

void DoEdit(HANDLE aDlg)
{
  LONG_PTR itemID=Info.SendDlgMessage(aDlg,DM_GETFOCUS,0,0);
  FarDialogItem DialogItem;
  Info.SendDlgMessage(aDlg,DM_GETDLGITEMSHORT,itemID,(LONG_PTR)&DialogItem);
  if(DialogItem.Type==DI_EDIT)
  {
    long length=Info.SendDlgMessage(aDlg,DM_GETTEXTLENGTH,itemID,0)+1;
    TCHAR* buffer=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,length*sizeof(TCHAR));
    if(buffer)
    {
      Info.SendDlgMessage(aDlg,DM_GETTEXTPTR,itemID,(LONG_PTR)buffer);
      TCHAR filename[MAX_PATH];
      if(FSF.MkTemp(filename,ArraySize(filename),_T("DTE")))
      {
        HANDLE file=CreateFile(filename,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
        if(file!=INVALID_HANDLE_VALUE)
        {
          DWORD written;
          short marker=0xfeff;
          WriteFile(file,&marker,sizeof(marker),&written,NULL);
          WriteFile(file,buffer,(length-1)*sizeof(TCHAR),&written,NULL);
          CloseHandle(file);
          Info.Editor(filename,_T(""),0,0,-1,-1,EF_DISABLEHISTORY,1,1,CP_AUTODETECT);
          file=CreateFile(filename,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
          if(file!=INVALID_HANDLE_VALUE)
          {
            DWORD size=SetFilePointer(file,0,NULL,FILE_END)/sizeof(TCHAR);
            SetFilePointer(file,sizeof(marker),NULL,FILE_BEGIN);
            TCHAR* outBuffer=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(size+1)*sizeof(TCHAR));
            if(outBuffer)
            {
              DWORD read;
              ReadFile(file,outBuffer,size*sizeof(TCHAR),&read,NULL);
              outBuffer[size]=0;
              for(size_t ii=0;ii<size;++ii)
              {
                if(outBuffer[ii]==13&&outBuffer[ii+1]==10)
                {
                  wmemcpy(outBuffer+ii,outBuffer+ii+1,size-ii);
                  outBuffer[ii]=0x20;
                  size--;
                }
                else if(outBuffer[ii]==13)
                {
                  outBuffer[ii]=0x20;
                }
                else if(outBuffer[ii]==10)
                {
                  outBuffer[ii]=0x20;
                }
              }
              Info.SendDlgMessage(aDlg,DM_SETTEXTPTR,itemID,(LONG_PTR)outBuffer);
              HeapFree(GetProcessHeap(),0,outBuffer);
            }
            CloseHandle(file);
          }
          DeleteFile(filename);
        }
      }
      HeapFree(GetProcessHeap(),0,buffer);
    }
  }
  Info.AdvControl(&MainGuid,ACTL_REDRAWALL,NULL);
}
