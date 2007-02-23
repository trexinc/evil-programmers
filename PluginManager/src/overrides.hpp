/*
    PluginManager plugin for FAR Manager
    Copyright (C) 2003-2005 Alex Yaroslavsky

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
#ifndef __OVERRIDES_HPP__
#define __OVERRIDES_HPP__

int WINAPI MDialog(int PluginNumber,int X1,int Y1,int X2,int Y2,const char *HelpTopic,struct FarDialogItem *Item,int ItemsNumber);
int WINAPI MMenu(int PluginNumber,int X,int Y,int MaxHeight,DWORD Flags,const char *Title,const char *Bottom,const char *HelpTopic,const int *BreakKeys,int *BreakCode,const struct FarMenuItem *Item,int ItemsNumber);
int WINAPI MMessage(int PluginNumber,DWORD Flags,const char *HelpTopic,const char * const *Items,int ItemsNumber,int ButtonsNumber);
int WINAPI MGetPluginDirList(int PluginNumber,HANDLE hPlugin,const char *Dir,struct PluginPanelItem **pPanelItem,int *pItemsNumber);
int WINAPI MAdvControl(int ModuleNumber,int Command,void *Param);
int WINAPI MDialogEx(int PluginNumber,int X1,int Y1,int X2,int Y2,const char *HelpTopic,struct FarDialogItem *Item,int ItemsNumber,DWORD Reserved,DWORD Flags,FARWINDOWPROC DlgProc,long Param);
int WINAPI MViewerControl(int Command,void *Param);
int WINAPI MControl(HANDLE hPlugin,int Command,void *Param);
const char *WINAPI MGetMsg(int PluginNumber,int MsgId);

#endif
