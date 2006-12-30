#ifndef __REGISTRY_HPP__
#define __REGISTRY_HPP__
#include <windows.h>

extern char PluginRootKey[];
void SetRegKey(const char *ValueName,DWORD ValueData);
BOOL GetRegKey(const char *ValueName,int *ValueData,DWORD Default);

#endif
