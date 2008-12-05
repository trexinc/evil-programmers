/*
    test.cpp
    Copyright (C) 2004-2008 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <tchar.h>
#include "ListBoxEx.hpp"

#ifndef UNICODE
#define EXP_NAME(p) _export p
#else
#define EXP_NAME(p) _export p ## W
#endif

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

const TCHAR* GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
};

struct InitDialogItem
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  int Selected;
  unsigned int Flags;
  int DefaultButton;
  const TCHAR* Data;
};

void InitDialogItems(InitDialogItem *Init,FarDialogItem *Item,int ItemsNumber)
{
  for(int i=0;i<ItemsNumber;i++)
  {
    Item[i].Type=Init[i].Type;
    Item[i].X1=Init[i].X1;
    Item[i].Y1=Init[i].Y1;
    Item[i].X2=Init[i].X2;
    Item[i].Y2=Init[i].Y2;
    Item[i].Focus=Init[i].Focus;
    Item[i].Selected=Init[i].Selected;
    Item[i].Flags=Init[i].Flags;
    Item[i].DefaultButton=Init[i].DefaultButton;
#ifdef UNICODE
    Item[i].MaxLen=0;
#endif
    if((unsigned)Init[i].Data<2000)
#ifdef UNICODE
      Item[i].PtrData=GetMsg((unsigned int)(DWORD_PTR)Init[i].Data);
#else
      strcpy(Item[i].Data,GetMsg((unsigned int)(DWORD_PTR)Init[i].Data));
#endif
    else
#ifdef UNICODE
      Item[i].PtrData=Init[i].Data;
#else
      strcpy(Item[i].Data,Init[i].Data);
#endif
  }
}

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>(int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;
}

void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=0;
  Info->DiskMenuStringsNumber=0;
  static const TCHAR* PluginMenuStrings[1];
  PluginMenuStrings[0]=_T("Test listboxex");
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  Info->PluginConfigStringsNumber=0;
}

static long WINAPI MainDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_INIT,1,0);
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("aaa"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("1 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("2 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("3 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("4 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("5 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("6 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("7 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("8 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("9 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("0 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("1 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("2 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("3 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("4 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("5 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("6 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("7 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("8 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("9 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("0 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("1 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("2 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("3 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("4 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("5 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("6 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(long)_T("7 bbb"));
      {
        ListBoxExSetColor color={2,LISTBOXEX_COLORS_ITEM,0,0x7C};
        for(int i=2;i<5;i++)
        {
          color.ColorIndex=i;
          Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETCOLOR,1,(long)&color);
        }
        color.Color=0x7A;
        for(int i=7;i<10;i++)
        {
          color.ColorIndex=i;
          Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETCOLOR,1,(long)&color);
        }
        color.Color=0x79;
        for(int i=12;i<15;i++)
        {
          color.ColorIndex=i;
          Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETCOLOR,1,(long)&color);
        }
        color.Color=0x09;
        color.TypeIndex=LISTBOXEX_COLORS_SELECTED;
        for(int i=12;i<15;i++)
        {
          color.ColorIndex=i;
          Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETCOLOR,1,(long)&color);
        }
        ListBoxExSetHotkey hotkey={16,'a'};
        Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETHOTKEY,1,(long)&hotkey);
      }
      break;
    case DN_KEY:
      if(Param2==KEY_DEL)
      {
        FarListDelete list={3,4};
        Info.SendDlgMessage(hDlg,DM_LISTBOXEX_DELETE,1,(long)&list);
      }
      else if(Param2==KEY_F2)
      {
        ListBoxExSetFlags flags={Info.SendDlgMessage(hDlg,DM_LISTBOXEX_GETCURPOS,1,0L),LIFEX_DISABLE};
        Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETFLAGS,1,(long)&flags);
      }
      else if(Param2==KEY_F3)
      {
        ListBoxExSetFlags flags={Info.SendDlgMessage(hDlg,DM_LISTBOXEX_GETCURPOS,1,0L)-1,0};
        Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETFLAGS,1,(long)&flags);
      }
      else if(Param2==KEY_INS)
      {
        Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_TOGGLE,1,Info.SendDlgMessage(hDlg,DM_LISTBOXEX_GETCURPOS,1,0L));
      }
      else if(Param2==KEY_SHIFTDEL)
      {
        ListBoxExSetFlags flags={Info.SendDlgMessage(hDlg,DM_LISTBOXEX_GETCURPOS,1,0L),LIFEX_HIDDEN};
        Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETFLAGS,1,(long)&flags);
      }
      else if(Param2==KEY_CTRLUP)
      {
        FarListPos pos={Info.SendDlgMessage(hDlg,DM_LISTBOXEX_GETCURPOS,1,0L),-1};
        Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_MOVE_UP,1,pos.SelectPos);
        pos.SelectPos--;
        Info.SendDlgMessage(hDlg,DM_LISTBOXEX_SETCURPOS,1,(long)&pos);
      }
      else if(Param2==KEY_CTRLDOWN)
      {
        FarListPos pos={Info.SendDlgMessage(hDlg,DM_LISTBOXEX_GETCURPOS,1,0L),-1};
        Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_MOVE_DOWN,1,pos.SelectPos);
        pos.SelectPos++;
        Info.SendDlgMessage(hDlg,DM_LISTBOXEX_SETCURPOS,1,(long)&pos);
      }
      break;
    case DN_CLOSE:
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_FREE,1,0);
      break;
    case DM_LISTBOXEX_ISLBE:
      if(Param1==1) return TRUE;
      return FALSE;
  }
  return ListBoxExDialogProc(hDlg,Msg,Param1,Param2);
}

HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom,int Item)
{
  InitDialogItem InitItems[]=
  {
    //type,x1,y1,x2,y2,focus,selected,flags,default,data
    {DI_DOUBLEBOX,3,1,67,18,0,0,0,0,_T("Very Cool Dialog")},
    {DI_USERCONTROL,4,2,66,17,0,0,0,0,_T("")},
    {DI_BUTTON,3,19,0,0,0,0,0,1,_T("Ok")},
    {DI_BUTTON,12,19,0,0,0,0,0,0,_T("Cancel")},
  };
  FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  CHAR_INFO *VirtualBuffer=(CHAR_INFO *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(DialogItems[1].X2-DialogItems[1].X1+1)*(DialogItems[1].Y2-DialogItems[1].Y1+1)*sizeof(CHAR_INFO));
  DialogItems[1].VBuf=VirtualBuffer;
#ifdef UNICODE
  HANDLE hDlg=Info.DialogInit
#else
  Info.DialogEx
#endif
  (Info.ModuleNumber,-1,-1,71,21,NULL,DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]),0,0,MainDialogProc,0);
#ifdef UNICODE
  if(hDlg!=INVALID_HANDLE_VALUE)
  {
    Info.DialogRun(hDlg);
    Info.DialogFree(hDlg);
  }
#endif
  if(VirtualBuffer) HeapFree(GetProcessHeap(),0,VirtualBuffer);
  return(INVALID_HANDLE_VALUE);
}
