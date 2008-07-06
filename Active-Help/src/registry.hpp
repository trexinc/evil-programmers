#ifndef __REGISTRY_HPP__
#define __REGISTRY_HPP__
#include <windows.h>

int GetRegKey(const char *ValueName,char *ValueData,const char *Default,DWORD DataSize);
void SetRegKey(const char *ValueName,char *ValueData);
void DelRegValue(char* ValueName);
void SetRegKey(const char *ValueName,DWORD ValueData);
BOOL GetRegKey(const char *ValueName,int *ValueData,DWORD Default);
extern char PluginRootKey[];

#endif
