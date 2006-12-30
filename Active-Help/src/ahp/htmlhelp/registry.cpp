/*
    HtmlHelp sub-plugin for Active-Help plugin for FAR Manager
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
#include "registry.hpp"

static HKEY hKey;
static HKEY hRoot = HKEY_CURRENT_USER;

static HKEY CreateRegKey()
{
  DWORD Disposition;
  RegCreateKeyEx(hRoot,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition);
  return(hKey);
}

static HKEY OpenRegKey()
{
  if (RegOpenKeyEx(hRoot,PluginRootKey,0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
    return(NULL);
  return(hKey);
}

void SetRegKey(const char *ValueName,DWORD ValueData)
{
  hKey=CreateRegKey();
  RegSetValueEx(hKey,ValueName,0,REG_DWORD,(BYTE *)&ValueData,sizeof(ValueData));
  RegCloseKey(hKey);
}

BOOL GetRegKey(const char *ValueName,int *ValueData,DWORD Default)
{
  hKey=OpenRegKey();
  DWORD Type,Size=sizeof(*ValueData);
  int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(BYTE *)ValueData,&Size);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    *ValueData=Default;
    return(FALSE);
  }
  return(TRUE);
}

int GetRegKey(const char *ValueName,char *ValueData,char *Default,DWORD DataSize)
{
  DWORD Type;
  int ExitCode;
  hKey=OpenRegKey();
  ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(LPBYTE)ValueData,&DataSize);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    lstrcpy(ValueData,Default);
    return(FALSE);
  }
  return(TRUE);
}

void SetRegKey(const char *ValueName,char *ValueData)
{
  hKey=CreateRegKey();
  RegSetValueEx(hKey,ValueName,0,REG_SZ,(CONST BYTE *)ValueData,lstrlen(ValueData)+1);
  RegCloseKey(hKey);
}
