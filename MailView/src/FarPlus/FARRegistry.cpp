/* $Header: /cvsroot/farplus/FARPlus/FARRegistry.cpp,v 1.2 2002/09/03 06:32:11 yole Exp $
   FAR+Plus: registry access class implementation
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
   This file is heavily based on sources by Eugene Roshal
*/

#include "FARRegistry.h"

bool FarRegistry::SetRegKey (const char *Key, const char *ValueName, const char *ValueData, HKEY hRoot)
{
    HKEY hKey=CreateRegKey(hRoot,Key);
  if (!hKey)
    return false;
    RegSetValueEx (hKey, ValueName, 0, REG_SZ, (BYTE *) ValueData, lstrlen(ValueData)+1);
    RegCloseKey(hKey);
  return true;
}


bool FarRegistry::SetRegKey (const char *Key, const char *ValueName, DWORD ValueData, HKEY hRoot)
{
    HKEY hKey=CreateRegKey(hRoot,Key);
  if (!hKey)
    return false;
    RegSetValueEx (hKey, ValueName, 0, REG_DWORD, (BYTE *)&ValueData, sizeof(ValueData));
    RegCloseKey(hKey);
  return true;
}


int FarRegistry::GetRegKey (const char *Key, const char *ValueName, char *ValueData,
                    const char *Default, DWORD DataSize, HKEY hRoot)
{
    HKEY hKey=OpenRegKey(hRoot,Key,true);
    DWORD Type;
    int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type, (BYTE *) ValueData,&DataSize);
    RegCloseKey(hKey);
    if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
    {
        lstrcpy(ValueData,Default);
        return(FALSE);
    }
    return(TRUE);
}

FarString FarRegistry::GetRegStr (const char *Key, const char *ValueName,
                  const char *Default, HKEY hRoot/* =HKEY_CURRENT_USER */)
{
  HKEY hKey=OpenRegKey (hRoot, Key, true);
  if (!hKey)
    return Default;
  DWORD Type;
  char Data [256];
  DWORD dataSize = sizeof (Data);

  FarString retStr;
  int retVal = RegQueryValueEx (hKey, ValueName, NULL, &Type, (BYTE *) Data, &dataSize);
  if (retVal == ERROR_MORE_DATA)
  {
    retStr.SetLength (dataSize);
    RegQueryValueEx (hKey, ValueName, NULL, &Type, (BYTE *) retStr.GetBuffer(),
      &dataSize);
  }
  else if (retVal == ERROR_SUCCESS)
  {
    if (Type == REG_SZ)
      retStr = Data;
    else
      retStr.SetText (Data, dataSize);
  }
  else
    retStr = Default;

  RegCloseKey (hKey);
  return retStr;
}

bool FarRegistry::KeyExists (const char *key, const char *valueName, HKEY hRoot)
{
  HKEY hKey = OpenRegKey (hRoot, key, true);
  if (!hKey)
    return false;

  DWORD Type;
  BYTE Data [256];
  DWORD dataSize = sizeof (Data);
  int retVal = RegQueryValueEx (hKey, valueName, NULL, &Type, Data, &dataSize);
  RegCloseKey (hKey);
  return (retVal == ERROR_SUCCESS || retVal == ERROR_MORE_DATA);
}


int FarRegistry::GetRegKey (const char *Key, const char *ValueName, int &ValueData,
                    DWORD Default, HKEY hRoot)
{
    HKEY hKey=OpenRegKey(hRoot,Key, true);
  if (hKey == NULL)
  {
    ValueData = Default;
    return FALSE;
  }
    DWORD Type,Size=sizeof(ValueData);
    int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(BYTE *)&ValueData,&Size);
    RegCloseKey(hKey);
    if (ExitCode!=ERROR_SUCCESS)
    {
        ValueData=Default;
        return(FALSE);
    }
    return(TRUE);
}

int FarRegistry::GetRegKey (const char *Key, const char *ValueName,DWORD Default,HKEY hRoot)
{
    int ValueData;
    GetRegKey(Key,ValueName,ValueData,Default,hRoot);
    return ValueData;
}

int FarRegistry::DeleteRegKey (const char *RootKey, const char *KeyName, HKEY hRoot)
{
    HKEY hKey=OpenRegKey (hRoot, RootKey, false);
    int ExitCode=RegDeleteKey (hKey, KeyName);
    RegCloseKey (hKey);
    return ExitCode;
}

FarRegistry::KeyIterator *FarRegistry::EnumKeys (const char *Key,
                        HKEY hRoot /* = HKEY_CURRENT_USER */)
{
  return new KeyIterator (OpenRegKey (hRoot, Key, true));
}

FarRegistry::ValueIterator *FarRegistry::EnumValues(const char *Key,
                           HKEY hRoot /* = HKEY_CURRENT_USER */)
{
  return new ValueIterator (OpenRegKey (hRoot, Key, true));
}

FarRegistry::BaseIterator::~BaseIterator()
{
  if (fEnumKey)
    RegCloseKey (fEnumKey);
}

bool FarRegistry::KeyIterator::NextKey (FarString &keyName)
{
  char Buf [256];
  FILETIME ftLastWrite;
  unsigned long bufSize = sizeof (Buf)-1;
  if (RegEnumKeyEx (fEnumKey, fEnumIndex++, Buf, &bufSize, NULL,
    NULL, NULL, &ftLastWrite) != ERROR_SUCCESS)
  {
    return false;
  }

  keyName = Buf;
  return true;
}

bool FarRegistry::ValueIterator::NextValue (FarString &valueName, FarString &valueData)
{
  char valueNameBuf [256];
  DWORD valueNameSize = sizeof (valueNameBuf)-1;
  DWORD valueType;
  unsigned char valueDataBuf [256];
  DWORD valueDataSize = sizeof (valueDataBuf)-1;

  LONG retVal = RegEnumValue (fEnumKey, fEnumIndex++, valueNameBuf, &valueNameSize, NULL,
    &valueType, valueDataBuf, &valueDataSize);
    if (retVal != ERROR_SUCCESS && retVal != ERROR_MORE_DATA)
    return false;

  valueName = valueNameBuf;
    valueData = reinterpret_cast<char *> (valueDataBuf);
  return true;
}

HKEY FarRegistry::CreateRegKey (HKEY hRoot, const char *Key)
{
    HKEY hKey;
    DWORD Disposition;
    FarString FullKeyName = fRootKey;
  if (Key)
  {
    FullKeyName += "\\";
    FullKeyName += Key;
  }
    RegCreateKeyEx(hRoot,FullKeyName,0,NULL,0,KEY_WRITE,NULL,
                 &hKey,&Disposition);
    return(hKey);
}


HKEY FarRegistry::OpenRegKey (HKEY hRoot, const char *Key, bool readOnly)
{
  HKEY hKey;
  FarString FullKeyName = fRootKey;
  if (Key)
  {
    FullKeyName += "\\";
    FullKeyName += Key;
  }
  REGSAM desiredAccess = (readOnly) ? KEY_READ : KEY_ALL_ACCESS;
  if (RegOpenKeyEx(hRoot,FullKeyName,0,desiredAccess,&hKey)!=ERROR_SUCCESS)
        return(NULL);
    return(hKey);
}
