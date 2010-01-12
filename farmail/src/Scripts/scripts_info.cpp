/*
    Scripts sub-plugin for FARMail
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
#include "scripts.hpp"

int GetScriptInfo(char *fd, char *fn, int i)
{
  char Value[50];

  FSF.sprintf(Value,"File%d",i);
  if (!GetRegKey2(HKEY_CURRENT_USER,PluginRootKey, NULLSTR,Value,fn,NULLSTR,512))
    return FALSE;
  FSF.sprintf(Value,"Desc%d",i);
  if (!GetRegKey2(HKEY_CURRENT_USER,PluginRootKey, NULLSTR,Value,fd,NULLSTR,512))
    return FALSE;
  return TRUE;
}

void SetScriptInfo(char *fd, char *fn, int i)
{
  char Value[50];

  FSF.sprintf(Value,"Desc%d",i);
  SetRegKey2(HKEY_CURRENT_USER,PluginRootKey, NULLSTR,Value,fd);
  FSF.sprintf(Value,"File%d",i);
  SetRegKey2(HKEY_CURRENT_USER,PluginRootKey, NULLSTR,Value,fn);
}

void DelScriptInfo(int i)
{
  char Value[50];

  FSF.sprintf(Value,"Desc%d",i);
  DelRegValue2(HKEY_CURRENT_USER,PluginRootKey, NULLSTR,Value);
  FSF.sprintf(Value,"File%d",i);
  DelRegValue2(HKEY_CURRENT_USER,PluginRootKey, NULLSTR,Value);
}
