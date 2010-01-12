/*
    FARMail plugin for FAR Manager
    Copyright (C) 2002-2005 FARMail Group
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
#include "farmail.hpp"

const char S3[] = "%s%s%s";

HKEY CreateRegKey2(HKEY hRoot,const char *root, const char *Key)
{
  HKEY hKey;
  DWORD Disposition;
  char FullKeyName[512];
  FSF.sprintf(FullKeyName,S3,root,*Key ? BACKSLASH:NULLSTR,Key);
  RegCreateKeyEx(hRoot,FullKeyName,0,NULL,0,KEY_WRITE,NULL,
                 &hKey,&Disposition);
  return(hKey);
}

HKEY OpenRegKey2(HKEY hRoot, const char*root, const char *Key)
{
  HKEY hKey;
  char FullKeyName[512];
  FSF.sprintf(FullKeyName,S3,root,*Key ? BACKSLASH:NULLSTR,Key);
  if (RegOpenKeyEx(hRoot,FullKeyName,0,KEY_READ,&hKey)!=ERROR_SUCCESS)
    return(NULL);
  return(hKey);
}

void SetRegKey2(HKEY hRoot, const char *root, const char *Key,const char *ValueName,const char *ValueData,DWORD type)
{
  HKEY hKey=CreateRegKey2(hRoot,root, Key);
  RegSetValueEx(hKey,ValueName,0,type,(uchar*)ValueData,lstrlen(ValueData)+1);
  RegCloseKey(hKey);
}


void SetRegKey2(HKEY hRoot,const char *root, const char *Key,const char *ValueName,DWORD ValueData)
{
  HKEY hKey=CreateRegKey2(hRoot,root, Key);
  RegSetValueEx(hKey,ValueName,0,REG_DWORD,(BYTE *)&ValueData,sizeof(ValueData));
  RegCloseKey(hKey);
}

void DeleteRegKey2(HKEY hRoot,const char *root, const char *Key)
{
  char FullKeyName[512];
  FSF.sprintf(FullKeyName,S3,root,*Key ? BACKSLASH:NULLSTR,Key);
  RegDeleteKey(hRoot,FullKeyName);
}


int GetRegKey2(HKEY hRoot,const char * root, const char *Key,const char *ValueName,char *ValueData,const char *Default,DWORD DataSize)
{
  HKEY hKey=OpenRegKey2(hRoot,root, Key);
  DWORD Type;
  int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(uchar*)ValueData,&DataSize);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    lstrcpy(ValueData,Default);
    return(FALSE);
  }
  return(TRUE);
}


int GetRegKey2(HKEY hRoot,const char *root,const char *Key,const char *ValueName,int *ValueData,DWORD Default)
{
  HKEY hKey=OpenRegKey2(hRoot,root, Key);
  DWORD Type,Size=sizeof(int);
  int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(BYTE *)ValueData,&Size);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    *ValueData=Default;
    return(FALSE);
  }
  return(TRUE);
}

char *RegQueryStringValueEx (
    HKEY hKey,
    const char *lpValueName,
    char *lpCurrentValue /* = NULL */
    )
{
  DWORD dwSize = 0;

  char *lpResultValue;

  if ( (RegQueryValueEx (
      hKey,
      lpValueName,
      NULL,
      NULL,
      NULL,
      &dwSize
      ) == ERROR_SUCCESS) )
  {
    z_free (lpCurrentValue);
    lpResultValue = (char*)z_malloc (dwSize+1);

    RegQueryValueEx (
        hKey,
        lpValueName,
        NULL,
        NULL,
        (PBYTE)lpResultValue,
        &dwSize
        );

    return lpResultValue;
  }

  return lpCurrentValue;
}
