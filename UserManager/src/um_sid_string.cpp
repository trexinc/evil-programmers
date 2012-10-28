/*
    um_sid_string.cpp
    Copyright (C) 2001-2007 zg

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "far_helper.h"
#include "umplugin.h"
#include "memory.h"
#include <winternl.h>

TCHAR *get_sid_string(PSID sid)
{
  TCHAR *res=NULL;
  UNICODE_STRING sid_str;
  memset(&sid_str,0,sizeof(sid_str));
  if(!RtlConvertSidToUnicodeString(&sid_str,sid,TRUE))
  {
    size_t length=sid_str.Length/sizeof(wchar_t);
    res=(TCHAR *)malloc((length+1)*sizeof(TCHAR));
    if(res)
    {
      _tcsncpy(res,sid_str.Buffer,length);
      res[length]=0;
    }
    RtlFreeUnicodeString(&sid_str);
  }
  return res;
}
