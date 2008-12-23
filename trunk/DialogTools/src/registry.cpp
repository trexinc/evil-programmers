#include "dt.hpp"

static HKEY CreateRegKey(HKEY hRoot,const TCHAR *Key)
{
  HKEY hKey;
  DWORD Disposition;
  TCHAR FullKeyName[512];
  _tcscpy(FullKeyName,PluginRootKey);
  if (*Key)
  {
    _tcscat(FullKeyName,_T("\\"));
    _tcscat(FullKeyName,Key);
  }
  RegCreateKeyEx(hRoot,FullKeyName,0,NULL,0,KEY_WRITE,NULL,
                 &hKey,&Disposition);
  return(hKey);
}


static HKEY OpenRegKey(HKEY hRoot,const TCHAR *Key)
{
  HKEY hKey;
  TCHAR FullKeyName[512];
  _tcscpy(FullKeyName,PluginRootKey);
  if (*Key)
  {
    _tcscat(FullKeyName,_T("\\"));
    _tcscat(FullKeyName,Key);
  }
  if (RegOpenKeyEx(hRoot,FullKeyName,0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
    return(NULL);
  return(hKey);
}

void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,DWORD ValueData)
{
  HKEY hKey=CreateRegKey(hRoot,Key);
  RegSetValueEx(hKey,ValueName,0,REG_DWORD,(BYTE *)&ValueData,sizeof(ValueData));
  RegCloseKey(hKey);
}

static int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,int &ValueData,DWORD Default)
{
  HKEY hKey=OpenRegKey(hRoot,Key);
  DWORD Type,Size=sizeof(ValueData);
  int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(BYTE *)&ValueData,&Size);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    ValueData=Default;
    return(FALSE);
  }
  return(TRUE);
}


int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,DWORD Default)
{
  int ValueData;
  GetRegKey(hRoot,Key,ValueName,ValueData,Default);
  return(ValueData);
}
