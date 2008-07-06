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

int GetRegKey(const char *ValueName,char *ValueData,const char *Default,DWORD DataSize)
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
