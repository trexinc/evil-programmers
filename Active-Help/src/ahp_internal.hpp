/*
    Active-Help plugin for FAR Manager
    Copyright (C) 2002-2005 Alex Yaroslavsky

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
#ifndef __AHP_INTERNAL_HPP__
#define __AHP_INTERNAL_HPP__
#include "plugin.hpp"
#include "ahp.hpp"

void GetLanguage(char *lang, DWORD size);

typedef int (WINAPI *PLUGINSTART)(const struct PluginStartupInfo *FarInfo,const struct AHPluginStartupInfo *AHInfo);
typedef void (WINAPI *PLUGINEXIT)(void);
typedef int (WINAPI *PLUGMESSAGE)(unsigned long Msg,void *InData,void *OutData);

struct PluginItem
{
  HMODULE hModule;
  unsigned long Flags;
  const struct TypeInfo *TypesInfo;
  int TypesNumber;
  char ConfigString[128];
  PLUGINSTART pStart;
  PLUGINEXIT pExit;
  PLUGMESSAGE pMessage;
};

class PluginManager
{
  private:
    PluginItem *PluginsData;
    char *Language;
    int PluginsCount;
    static int WINAPI FindPlugin(const WIN32_FIND_DATA *fdata,const char *filename,PluginManager *self);
    void AddPlugin(const char *filename);
    void SendBroadcastMessage(unsigned long Msg,void *InData,void *OutData);
    int SendMessage(int Index,unsigned long Msg,void *InData,void *OutData);
    bool UpdateLanguage(void);
    void LoadPluginsData(void);
    struct FarListItem *TypesListItems;
    int ItemsNumber;
  public:
    PluginManager(char *plug_dir);
    ~PluginManager();
    void ShowConfigMenu(void);
    int ShowHelp(const char *Type, const char *FileName, const char *Keyword, unsigned long Flags, int CallType, const char *Encoding, char *Error);
    int PluginsTotal(void);
    bool DetectType(const char *Str, int Action, const struct TypeInfo ***data, int *DetectedTypes);
    struct FarList TypesList;
};

extern char PluginPluginsRootKey[100];

extern void WINAPI api_get_message(const char *file_name,int index,char *message);
extern long WINAPI api_show_help_dlg_proc(HANDLE hDlg, int Msg,int Param1,long Param2);
extern int WINAPI api_show_help_menu(const char *ModuleName, int PluginNumber, int X, int Y, int MaxHeight, unsigned int Flags, const char *Title, const char *Bottom, const char *HelpTopic, const int *BreakKeys, int *BreakCode, const struct FarMenuItem *Item, int ItemsNumber);
extern BOOL WINAPI api_encode(char *str, int len, const char *encoding);
extern BOOL WINAPI api_decode(char *str, int len, const char *encoding);

#endif
