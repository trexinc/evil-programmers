#ifndef __REGISTRY_HPP__
#define __REGISTRY_HPP__

#include <windows.h>

extern char PluginRootKey[];
void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData,char *Default,DWORD DataSize);
HKEY CreateRegKey(HKEY hRoot,const char *Key);
HKEY OpenRegKey(HKEY hRoot,const char *Key);

#endif /* __REGISTRY_HPP__ */
