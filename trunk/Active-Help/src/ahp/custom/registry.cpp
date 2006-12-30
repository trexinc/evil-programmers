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
