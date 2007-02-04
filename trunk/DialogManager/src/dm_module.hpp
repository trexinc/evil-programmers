/*
    DialogManager plugin for FAR Manager
    Copyright (C) 2003 Vadim Yegorov
    Copyright (C) 2004 Vadim Yegorov and Alex Yaroslavsky

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
#ifndef __FMT_HPP__
#define __FMT_HPP__

#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

typedef void (WINAPI *PLUGINGETMESSAGE)(const char *file_name,int index,char *message);

struct DialogPluginStartupInfo
{
  int StructSize;
  char ModuleName[MAX_PATH];
  char MessageName[MAX_PATH];
  const char *RootKey;
  PLUGINGETMESSAGE GetMsg;
};

struct GetInfoOutData
{
  unsigned long StructSize;
  unsigned long Flags;
  char MenuString[64];
  char HotkeyID[64];
};

struct MenuInData
{
  unsigned long StructSize;
  HANDLE hDlg;
  int ItemID;
};

struct FilterInData
{
  unsigned long StructSize;
  HANDLE hDlg;
  int Msg;
  int Param1;
  long Param2;
  long *Result;
};

#define FMMSG_GETINFO               1
#define FMMSG_MENU                  2
#define FMMSG_CONFIG                4
#define FMMSG_FILTER                8
#define FMMSG_CLEANUP              16
#define FMMSG_PANEL                32

#ifdef __cplusplus
extern "C" {
#endif
  int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct DialogPluginStartupInfo *DialogInfo);
  void WINAPI _export Exit(void);
  int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData);
#ifdef __cplusplus
}
#endif

#endif
