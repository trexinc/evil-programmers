#ifndef __options_hpp
#define __options_hpp

#include "strcon.hpp"

struct OPTIONS
{
  int TurnOnPluginModule, ReloadSettingsAutomatically, ShowFileMaskInMenu;
};

struct ESCFileInfo
{
  strcon   Name;
  FILETIME Time;
  DWORD    SizeLow, SizeHigh;
  ESCFileInfo():SizeLow(0), SizeHigh(0)
  {
    memset(&Time,0,sizeof(Time));
  }
  ESCFileInfo(const char *name):Name(name), SizeLow(0), SizeHigh(0)
  {
    memset(&Time,0,sizeof(Time));
  }
};

BOOL WINAPI ESCFICompLT(const ESCFileInfo &a,const ESCFileInfo &b);
BOOL WINAPI ESCFICompEQ(const ESCFileInfo &a,const ESCFileInfo &b);

#endif // __options_hpp
