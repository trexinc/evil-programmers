/*
    Active-Help plugin for FAR Manager
    Copyright (C) 2002 Alex Yaroslavsky

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef __AHP_HPP__
#define __AHP_HPP__
#include "plugin.hpp"

enum
{
  F_INPUTKEYWORD = 1,
  F_NOKEYWORDSEARCH = 2,
  F_NOSKIPPATH = 4,
};

typedef void (WINAPI *PLUGINGETMESSAGE)(const char *file_name,int index,char *message);
typedef int (WINAPI *PLUGINSHOWHELPMENU)(const char *ModuleName, int PluginNumber, int X, int Y, int MaxHeight, unsigned int Flags, const char *Title, const char *Bottom, const char *HelpTopic, const int *BreakKeys, int *BreakCode, const struct FarMenuItem *Item, int ItemsNumber);
typedef BOOL (WINAPI *PLUGINENCODE)(char *str, int len, const char *encoding);
typedef BOOL (WINAPI *PLUGINDECODE)(char *str, int len, const char *encoding);

struct AHPluginStartupInfo
{
  int StructSize;
  char ModuleName[MAX_PATH];
  char MessageName[MAX_PATH];
  const char *RootKey;
  PLUGINGETMESSAGE GetMsg;
  FARWINDOWPROC ShowHelpDlgProc;
  PLUGINSHOWHELPMENU ShowHelpMenu;
  PLUGINENCODE Encode;
  PLUGINDECODE Decode;
};

struct TypeInfo
{
  unsigned long StructSize;
  unsigned long HFlags;
  char TypeString[128];
  char Mask[128];
  char Encoding[128];
};

struct GetInfoOutData
{
  unsigned long StructSize;
  unsigned long Flags;
  const struct TypeInfo *TypesInfo;
  int TypesNumber;
  char ConfigString[128];
};

struct HelpOutData
{
  unsigned long StructSize;
  char Error[128];
};

struct HelpInData
{
  unsigned long StructSize;
  int TypeNumber;
  unsigned long Flags;
  int CallType;
  const char *FileName;
  const char *Keyword;
  const char *Encoding;
};

#ifdef __cplusplus
extern "C" {
#endif
  int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct AHPluginStartupInfo *AHInfo);
  void WINAPI _export Exit(void);
  int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData);
#ifdef __cplusplus
}
#endif

#define AHMSG_GETINFO               1
#define AHMSG_SHOWHELP              2
#define AHMSG_CONFIG                4

#endif
