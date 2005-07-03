/*
    Filter sub-plugin for FARMail
    Copyright (C) 2002-2004 FARMail Group

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
#include <windows.h>

bool ReadLine(HANDLE file,char *buffer,size_t len)
{
  DWORD CurrPos=SetFilePointer(file,0,NULL,FILE_CURRENT),transferred;
  bool res=true;
  if(ReadFile(file,buffer,len-1,&transferred,NULL)&&transferred)
  {
    buffer[len-1]=0;
    DWORD len=0; char *ptr=buffer;
    for(size_t i=0;i<transferred;i++,len++,ptr++)
    {
      if(buffer[i]=='\n'||buffer[i]=='\r')
      {
        if((buffer[i+1]=='\n'||buffer[i+1]=='\r')&&buffer[i]!=buffer[i+1])
          len++;
        break;
      }
    }
    *ptr=0;
    SetFilePointer(file,CurrPos+len+1,NULL,FILE_BEGIN);
  }
  else
  {
    buffer[0]=0;
    res=false;
  }
  return res;
}
