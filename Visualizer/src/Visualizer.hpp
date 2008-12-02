/*
    Visualizer plugin for FAR Manager
    Copyright (C) 2006 Alex Yaroslavsky

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
#include "plugin.hpp"
#include <CRT/crt.hpp>

extern struct PluginStartupInfo Info;
extern FARSTANDARDFUNCTIONS FSF;

enum
{
  MTitle,

  MOnOffSwitch,
  MOtherColoringPlugins,

  MRightBorder,
  MRightBorderOn,
  MRightBorderOnOnlyIfAutoWrap,

  MEOL,
  MEOLOn,
  MEOLMarkAll,
  MEOLMarkAllWithSymbols,

  MEOF,

  MTabs,
  MTabsOn,
  MTabsWhole,
  MTabsTwoColor,
  MTabsShowSymbol,

  MCross,
  MCrossOn,
  MCrossVertical,
  MCrossHorizontal,
  MCrossSmall,
  MCrossOnTop,

  MCursor,

  MLineNumbers,

  MScrollbar,
  MScrollbarOn,
  MScrollbarVariable,
  MScrollbarFillbar,
  MScrollbarNoBack,
  MScrollbarVariableNoBack,
  MScrollbarFillbarNoBack,

  MRightBorderColor,
  MEOLNormalColor,
  MEOLCRColor,
  MEOLLFColor,
  MEOLNULLColor,
  MEOLEOFColor,
  MTabsColor,
  MTabs2Color,
  MCrossVerticalColor,
  MCrossHorizontalColor,
  MCursorColor,
  MScrollbarColor,
  MScrollbarPositionMarkerColor,

  MColorTitle,
  MColorForeground,
  MColorBackground,
  MColorTest1,
  MColorTest2,
  MColorTest3,

  MOk,
  MCancel,
  MSetColor,
};

struct InitDialogItem
{
  int Type;
  int X1, Y1, X2, Y2;
  unsigned int Flags;
  int Data;
};

#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

const TCHAR *GetMsg(int MsgId);
void InitDialogItems(const struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber);
bool SelectColor(int *fg,int *bg);

#ifndef UNICODE
#define GetCheck(i) DialogItems[i].Selected
#define GetListPos(i) DialogItems[i].ListPos
#else
#define GetCheck(i) (int)Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
#define GetListPos(i) ((int)Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,i,0))
#endif
