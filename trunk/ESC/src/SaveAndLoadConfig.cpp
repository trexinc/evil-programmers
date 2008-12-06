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
#ifndef __SaveAndLoadConfig
#define __SaveAndLoadConfig

#include <windows.h>
#include "myrtl.hpp"
#include "SaveAndLoadConfig.hpp"
#include "options.hpp"
#include "plugin.hpp"
#include "syslog.hpp"

extern OPTIONS Opt;
extern FarStandardFunctions FSF;
extern wchar_t PluginRootKey[NM];
const wchar_t *REGStrSSS=L"%s%s%s", *REGStrOptions=L"Options";
wchar_t REGFullKeyName[512];

enum
{
  O_TurnOnPluginModule          = 0x00000001,
  O_ReloadSettingsAutomatically = 0x00000002,
  O_ShowFileMaskInMenu          = 0x00000004,
};


HKEY CreateRegKey(HKEY hRoot, const wchar_t *RootKey, const wchar_t *Key);
HKEY OpenRegKey(HKEY hRoot, const wchar_t *RootKey, const wchar_t *Key);

void SetRegKey(HKEY hRoot, const wchar_t *RootKey, const wchar_t *Key,
               const wchar_t *ValueName, DWORD ValueData)
{
  HKEY hKey = CreateRegKey(hRoot, RootKey, Key);

  RegSetValueEx(hKey, ValueName, 0, REG_DWORD, (BYTE *) & ValueData,
                sizeof(ValueData));
  RegCloseKey(hKey);
}


void SetRegKey(HKEY hRoot, const wchar_t *RootKey, const wchar_t *Key,
               const wchar_t *ValueName, const wchar_t *ValueData)
{
  HKEY hKey = CreateRegKey(hRoot, RootKey, Key);

  RegSetValueEx(hKey, ValueName, 0, REG_SZ, (CONST BYTE *) ValueData,
                (wstrlen(ValueData) + 1)*sizeof(wchar_t));
  RegCloseKey(hKey);
}

void SetRegKey(HKEY hRoot, const wchar_t *RootKey, const wchar_t *Key,
               const wchar_t *ValueName, const BYTE *ValueData, DWORD ValueSize)
{
  HKEY hKey=CreateRegKey(hRoot, RootKey, Key);
  RegSetValueEx(hKey, ValueName,0, REG_BINARY, ValueData, ValueSize);
  RegCloseKey(hKey);
}

int GetRegKey(HKEY hRoot, const wchar_t *RootKey, const wchar_t *Key,
              const wchar_t *ValueName, BYTE *ValueData,
              const BYTE *Default, DWORD DataSize)
{
  int ExitCode=ERROR_MORE_DATA;
  HKEY hKey=OpenRegKey(hRoot, RootKey, Key);
  if(hKey)
  {
    DWORD Type, Required=DataSize;
    ExitCode=RegQueryValueEx(hKey,ValueName, 0, &Type, ValueData, &Required);
    if(ExitCode == ERROR_MORE_DATA) // если размер не подходящие...
    {
      BYTE *TempBuffer=(BYTE*)malloc(Required+1); // ...то выделим сколько надо
      if(TempBuffer) // Если с памятью все нормально...
      {
        if((ExitCode=RegQueryValueEx(hKey, ValueName, 0, &Type,
           TempBuffer,&Required)) == ERROR_SUCCESS)
           memcpy(ValueData, TempBuffer, DataSize);  // скопируем сколько надо.
        free(TempBuffer);
      }
    }
    RegCloseKey(hKey);
  }
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    if (Default!=NULL)
      memcpy(ValueData, Default, DataSize);
    else
      memset(ValueData, 0, DataSize);

    return(FALSE);
  }

  return(DataSize);
}

int GetRegKey(HKEY hRoot, const wchar_t *RootKey, const wchar_t *Key,
              const wchar_t *ValueName, int &ValueData,
              DWORD Default)
{
  HKEY hKey = OpenRegKey(hRoot, RootKey, Key);
  DWORD Type, Size = sizeof(ValueData);
  int ExitCode =
    RegQueryValueEx(hKey, ValueName, 0, &Type, (BYTE *) & ValueData, &Size);
  RegCloseKey(hKey);
  if (hKey == NULL || ExitCode != ERROR_SUCCESS)
    {
      ValueData = Default;
      return (FALSE);
    }
  return (TRUE);
}


int GetRegKey(HKEY hRoot, const wchar_t *RootKey, const wchar_t *Key,
              const wchar_t *ValueName, DWORD Default)
{
  int ValueData;

  GetRegKey(hRoot, RootKey, Key, ValueName, ValueData, Default);
  return (ValueData);
}

/*
int GetRegKey(HKEY hRoot, const wchar_t *RootKey, const wchar_t *Key,
              const wchar_t *ValueName, wchar_t *ValueData,
              const wchar_t *Default, DWORD DataSize)
{
  HKEY hKey = OpenRegKey(hRoot, RootKey, Key);
  DWORD Type;
  int ExitCode =
    RegQueryValueEx(hKey, ValueName, 0, &Type, (LPBYTE) ValueData, &DataSize);
  RegCloseKey(hKey);
  if (hKey == NULL || ExitCode != ERROR_SUCCESS)
    {
      wstrncpy(ValueData, Default, DataSize-1);
      return (FALSE);
    }
  return (TRUE);
}
*/

HKEY CreateRegKey(HKEY hRoot, const wchar_t *RootKey, const wchar_t *Key)
{
  HKEY hKey;
  DWORD Disposition;
  FSF.sprintf(REGFullKeyName, REGStrSSS, RootKey, *Key ? L"\\" : L"", Key);
  RegCreateKeyEx(hRoot, REGFullKeyName, 0, NULL, 0, KEY_WRITE, NULL,
                 &hKey, &Disposition);
  return (hKey);
}


HKEY OpenRegKey(HKEY hRoot, const wchar_t *RootKey, const wchar_t *Key)
{
  HKEY hKey;
  FSF.sprintf(REGFullKeyName, REGStrSSS, RootKey, *Key ? L"\\" : L"", Key);
  if (RegOpenKeyEx(hRoot, REGFullKeyName, 0, KEY_QUERY_VALUE, &hKey) !=
      ERROR_SUCCESS)
    return (NULL);
  return (hKey);
}

void DeleteRegKey(HKEY hRoot, const wchar_t *RootKey, const wchar_t *Key)
{
  FSF.sprintf(REGFullKeyName, REGStrSSS, RootKey, *Key ? L"\\" : L"", Key);
  RegDeleteKey(hRoot, REGFullKeyName);
}


void LoadGlobalConfig() // грузится здесь, а сохраняется в настройках
{
  DWORD Options=GetRegKey(HKEY_CURRENT_USER, PluginRootKey, L"",
                          REGStrOptions,
                          O_TurnOnPluginModule|O_ReloadSettingsAutomatically|
                          O_ShowFileMaskInMenu);
  Opt.TurnOnPluginModule =(Options&O_TurnOnPluginModule)?TRUE:FALSE;
  Opt.ReloadSettingsAutomatically=(Options&O_ReloadSettingsAutomatically)?TRUE:FALSE;
  Opt.ShowFileMaskInMenu=(Options&O_ShowFileMaskInMenu)?TRUE:FALSE;
}

void SaveGlobalConfig()
{
  DWORD Options=0;
  if(Opt.TurnOnPluginModule)
     Options|=O_TurnOnPluginModule;
  if(Opt.ReloadSettingsAutomatically)
     Options|=O_ReloadSettingsAutomatically;
  if(Opt.ShowFileMaskInMenu)
     Options|=O_ShowFileMaskInMenu;
  SetRegKey(HKEY_CURRENT_USER, PluginRootKey, L"", REGStrOptions, Options);
}

#endif //__SaveAndLoadConfig
