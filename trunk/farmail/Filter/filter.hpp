/*
    Filter sub-plugin for FARMail
    Copyright (C) 2002-2004 FARMail Group
    Copyright (C) 1999,2000 Serge Alexandrov

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
#ifndef __FILTER_HPP__
#define __FILTER_HPP__
#include "plugin.hpp"
#include "../FARMail/fmp.hpp"

#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

extern struct PluginStartupInfo FarInfo;
extern FARSTANDARDFUNCTIONS FSF;
extern struct MailPluginStartupInfo FarMailInfo;
extern char DefFiltersDir[];
extern const char NULLSTR[];

extern int RunFilter(HANDLE hPlugin,int index);
extern char *GetMsg(int MsgNum,char *Str);
extern bool ReadLine(HANDLE file,char *buffer,size_t len);
extern void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item, int ItemsNumber);

struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  unsigned int Selected;
  unsigned int Flags;
  unsigned char DefaultButton;
  const char *Data;
};

enum
{
  mName,
  mOk,
  mCancel,
  mError,
  mNoMem,
  mFilter1,
  mFilter2,
  mFilter3,
  mFilter4,
  mFilter5,
  mUseFile,
  mNoLists,
  mFiltersList,
  mConfig_Title,
  mConfig_DefFiltersDir,
};

#endif
