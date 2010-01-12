/*
    AddressBook sub-plugin for FARMail
    Copyright (C) 2002-2004 FARMail Group
    Copyright (C) 1999,2000 Serge Alexandrov

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
#include "plugin.hpp"
#include "registry.hpp"
#define uchar unsigned char
#define cuchar const unsigned char

static const char S1[] = "%s%s%s";
static const char S2[] = "\\";

static HKEY CreateRegKey2(HKEY hRoot,const char *root, const char *Key)
{
  HKEY hKey;
  DWORD Disposition;
  char FullKeyName[512];
  FSF.sprintf(FullKeyName,S1,root,*Key ? S2:NULLSTR,Key);
  RegCreateKeyEx(hRoot,FullKeyName,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition);
  return(hKey);
}

static HKEY OpenRegKey2(HKEY hRoot, const char *root, const char *Key)
{
  HKEY hKey;
  char FullKeyName[512];
  FSF.sprintf(FullKeyName,S1,root,*Key ? S2:NULLSTR,Key);
  if (RegOpenKeyEx(hRoot,FullKeyName,0,KEY_READ,&hKey)!=ERROR_SUCCESS)
    return(NULL);
  return(hKey);
}

void SetRegKey2(HKEY hRoot, const char *root, const char *Key,const char *ValueName,const char *ValueData,DWORD type)
{
  HKEY hKey=CreateRegKey2(hRoot,root, Key);
  RegSetValueEx(hKey,ValueName,0,type,(cuchar *)ValueData,lstrlen(ValueData)+1);
  RegCloseKey(hKey);
}

int GetRegKey2(HKEY hRoot,const char * root, const char *Key,const char *ValueName,char *ValueData,const char *Default,DWORD DataSize)
{
  HKEY hKey=OpenRegKey2(hRoot,root, Key);
  DWORD Type;
  int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(uchar *)ValueData,&DataSize);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    lstrcpy(ValueData,Default);
    return(FALSE);
  }
  return(TRUE);
}
