#ifndef __REGISTRY_HPP__
#define __REGISTRY_HPP__
#include <windows.h>

int GetRegKey(const char *ValueName,char *ValueData,const char *Default,DWORD DataSize);
void SetRegKey(const char *ValueName,char *ValueData);
extern char PluginRootKey[];

#endif
