/*
    DialogManager plugin for FAR Manager
    Copyright (C) 2003 Vadim Yegorov
    Copyright (C) 2004-2005 Vadim Yegorov and Alex Yaroslavsky

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
#ifndef __DIALOGM_HPP__
#define __DIALOGM_HPP__

enum {
  mName,
  mHotkeyDialogTitle,
  mHotkeyDialogLabel,
  mEditor,
  mConfig,
  mHotkey,
};

#define FAR165_INFO_SIZE 336

extern PluginStartupInfo Info;
extern FARSTANDARDFUNCTIONS FSF;
extern char PluginRootKey[100];
extern char PluginHotkeyKey[100];
extern char PluginPluginsRootKey[100];

//plugins

#include "dm_module.hpp"

typedef int (WINAPI *PLUGINSTART)(const struct PluginStartupInfo *FarInfo,const struct DialogPluginStartupInfo *DialogInfo);
typedef void (WINAPI *PLUGINEXIT)(void);
typedef int (WINAPI *PLUGMESSAGE)(unsigned long Msg,void *InData,void *OutData);

struct PluginItem
{
  HMODULE hModule;
  unsigned long Flags;
  char MenuString[64];
  char HotkeyID[64];
  char Hotkey;
  PLUGINSTART pStart;
  PLUGINEXIT pExit;
  PLUGMESSAGE pMessage;
};

extern void LoadPlugs(void);
extern void UnloadPlugs(void);
extern void SendBroadcastMessage(unsigned long Msg,void *InData,void *OutData);
extern int ShowMenu(unsigned long flag,void *InData,void *OutData);
extern int PluginCount(unsigned long flag);
extern void LoadPluginsData(void);

#endif
