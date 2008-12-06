#ifndef __SaveAndLoadConfig
#define __SaveAndLoadConfig

#include <windows.h>
#include "SaveAndLoadConfig.hpp"
#include "options.hpp"
#include "plugin.hpp"
#include "syslog.hpp"

extern OPTIONS Opt;
extern FarStandardFunctions FSF;
extern char PluginRootKey[NM];
const char *REGStrSSS="%s%s%s", *REGStrOptions="Options";
char REGFullKeyName[512];

enum
{
  O_TurnOnPluginModule          = 0x00000001,
  O_ReloadSettingsAutomatically = 0x00000002,
  O_ShowFileMaskInMenu          = 0x00000004,
};


HKEY CreateRegKey(HKEY hRoot, const char *RootKey, const char *Key);
HKEY OpenRegKey(HKEY hRoot, const char *RootKey, const char *Key);

void SetRegKey(HKEY hRoot, const char *RootKey, const char *Key,
               const char *ValueName, DWORD ValueData)
{
  HKEY hKey = CreateRegKey(hRoot, RootKey, Key);

  RegSetValueEx(hKey, ValueName, 0, REG_DWORD, (BYTE *) & ValueData,
                sizeof(ValueData));
  RegCloseKey(hKey);
}


void SetRegKey(HKEY hRoot, const char *RootKey, const char *Key,
               const char *ValueName, const char *ValueData)
{
  HKEY hKey = CreateRegKey(hRoot, RootKey, Key);

  RegSetValueEx(hKey, ValueName, 0, REG_SZ, (CONST BYTE *) ValueData,
                strlen(ValueData) + 1);
  RegCloseKey(hKey);
}

void SetRegKey(HKEY hRoot, const char *RootKey, const char *Key,
               const char *ValueName, const BYTE *ValueData, DWORD ValueSize)
{
  HKEY hKey=CreateRegKey(hRoot, RootKey, Key);
  RegSetValueEx(hKey, ValueName,0, REG_BINARY, ValueData, ValueSize);
  RegCloseKey(hKey);
}

int GetRegKey(HKEY hRoot, const char *RootKey, const char *Key,
              const char *ValueName, BYTE *ValueData,
              const BYTE *Default, DWORD DataSize)
{
  int ExitCode;
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

int GetRegKey(HKEY hRoot, const char *RootKey, const char *Key,
              const char *ValueName, int &ValueData,
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


int GetRegKey(HKEY hRoot, const char *RootKey, const char *Key,
              const char *ValueName, DWORD Default)
{
  int ValueData;

  GetRegKey(hRoot, RootKey, Key, ValueName, ValueData, Default);
  return (ValueData);
}


int GetRegKey(HKEY hRoot, const char *RootKey, const char *Key,
              const char *ValueName, char *ValueData,
              const char *Default, DWORD DataSize)
{
  HKEY hKey = OpenRegKey(hRoot, RootKey, Key);
  DWORD Type;
  int ExitCode =
    RegQueryValueEx(hKey, ValueName, 0, &Type, (LPBYTE) ValueData, &DataSize);
  RegCloseKey(hKey);
  if (hKey == NULL || ExitCode != ERROR_SUCCESS)
    {
      strncpy(ValueData, Default, DataSize-1);
      return (FALSE);
    }
  return (TRUE);
}

HKEY CreateRegKey(HKEY hRoot, const char *RootKey, const char *Key)
{
  HKEY hKey;
  DWORD Disposition;
  FSF.sprintf(REGFullKeyName, REGStrSSS, RootKey, *Key ? "\\" : "", Key);
  RegCreateKeyEx(hRoot, REGFullKeyName, 0, NULL, 0, KEY_WRITE, NULL,
                 &hKey, &Disposition);
  return (hKey);
}


HKEY OpenRegKey(HKEY hRoot, const char *RootKey, const char *Key)
{
  HKEY hKey;
  FSF.sprintf(REGFullKeyName, REGStrSSS, RootKey, *Key ? "\\" : "", Key);
  if (RegOpenKeyEx(hRoot, REGFullKeyName, 0, KEY_QUERY_VALUE, &hKey) !=
      ERROR_SUCCESS)
    return (NULL);
  return (hKey);
}

void DeleteRegKey(HKEY hRoot, const char *RootKey, const char *Key)
{
  FSF.sprintf(REGFullKeyName, REGStrSSS, RootKey, *Key ? "\\" : "", Key);
  RegDeleteKey(hRoot, REGFullKeyName);
}


void LoadGlobalConfig() // грузится здесь, а сохраняется в настройках
{
  DWORD Options=GetRegKey(HKEY_CURRENT_USER, PluginRootKey, "",
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
  SetRegKey(HKEY_CURRENT_USER, PluginRootKey, "", REGStrOptions, Options);
}

#endif //__SaveAndLoadConfig
