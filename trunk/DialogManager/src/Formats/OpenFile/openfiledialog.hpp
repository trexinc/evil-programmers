/*
    OpenFile plugin for DialogManager
    Copyright (C) 2003-2005 Vadim Yegorov and Alex Yaroslavsky

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
#ifndef __OPENFILEDIALOG_HPP__
#define __OPENFILEDIALOG_HPP__

enum
{
  mName,
  mBottomTitle,
};

extern PluginStartupInfo FarInfo;
extern FARSTANDARDFUNCTIONS FSF;
extern DialogPluginStartupInfo DialogInfo;

char *GetMsg(int MsgNum,char *Str);
int GetRegKey(HKEY hRoot,const char * root, const char *Key,const char *ValueName,char *ValueData,const char *Default,DWORD DataSize);

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct DialogPluginStartupInfo *DialogInfo);
void WINAPI _export Exit(void);
int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData);

#endif /* __OPENFILEDIALOG_HPP__ */
