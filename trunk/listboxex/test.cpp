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
#include <initguid.h>

// {843882E6-3774-4f99-A1E3-65CD195867A0}
DEFINE_GUID(MainGuid, 0x843882e6, 0x3774, 0x4f99, 0xa1, 0xe3, 0x65, 0xcd, 0x19, 0x58, 0x67, 0xa0);
// {AFCDCD78-241C-472c-9D0B-F0CDCFDAE52D}
DEFINE_GUID(DialogGuid, 0xafcdcd78, 0x241c, 0x472c, 0x9d, 0xb, 0xf0, 0xcd, 0xcf, 0xda, 0xe5, 0x2d);

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

const TCHAR* GetMsg(int MsgId)
{
  return(Info.GetMsg(&MainGuid,MsgId));
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
    Item[i].Type=(FARDIALOGITEMTYPES)Init[i].Type;
    Item[i].X1=Init[i].X1;
    Item[i].Y1=Init[i].Y1;
    Item[i].X2=Init[i].X2;
    Item[i].Y2=Init[i].Y2;
    Item[i].Selected=Init[i].Selected;
    Item[i].Flags=Init[i].Flags;
    Item[i].MaxLength=0;
    if((UINT_PTR)Init[i].Data<2000)
      Item[i].Data=GetMsg((unsigned int)(DWORD_PTR)Init[i].Data);
    else
      Item[i].Data=Init[i].Data;
    Item[i].Mask=NULL;
    Item[i].History=NULL;
    Item[i].UserData=0;
  }
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=MAKEFARVERSION(1,0,0,1,VS_RELEASE);
  Info->Guid=MainGuid;
  Info->Title=L"ListBoxEx Demo";
  Info->Description=L"ListBoxEx demo plugin";
  Info->Author=L"Vadim Yegorov";
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>(int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=0;
  Info->DiskMenu.Count=0;
  static const TCHAR* PluginMenuStrings[1];
  PluginMenuStrings[0]=_T("Test listboxex");
  Info->PluginMenu.Guids=&MainGuid;
  Info->PluginMenu.Strings=PluginMenuStrings;
  Info->PluginMenu.Count=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  Info->PluginConfig.Count=0;
}

static INT_PTR WINAPI MainDialogProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_INIT,1,0);
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("aaa"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("1 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("2 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("3 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("4 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("5 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("6 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("7 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("8 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("9 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("0 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("1 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("2 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("3 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("4 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("5 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("6 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("7 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("8 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("9 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("0 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("1 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("2 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("3 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("4 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("5 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("6 bbb"));
      Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,1,(void*)_T("7 bbb"));
      {
        ListBoxExSetColor color={2,LISTBOXEX_COLORS_ITEM,0,{{FCF_FG_4BIT|FCF_BG_4BIT,0xC,0x7,NULL},0,false}};
        for(int i=2;i<5;i++)
        {
          color.ColorIndex=i;
          Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETCOLOR,1,&color);
        }
        color.Color.Color.ForegroundColor=0xA;
        color.Color.Color.BackgroundColor=0x7;
        for(int i=7;i<10;i++)
        {
          color.ColorIndex=i;
          Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETCOLOR,1,&color);
        }
        color.Color.Color.ForegroundColor=0x9;
        color.Color.Color.BackgroundColor=0x7;
        for(int i=12;i<15;i++)
        {
          color.ColorIndex=i;
          Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETCOLOR,1,&color);
        }
        color.Color.Color.ForegroundColor=0x9;
        color.Color.Color.BackgroundColor=0x0;
        color.TypeIndex=LISTBOXEX_COLORS_SELECTED;
        for(int i=12;i<15;i++)
        {
          color.ColorIndex=i;
          Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETCOLOR,1,&color);
        }
        ListBoxExSetHotkey hotkey={16,'a'};
        Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETHOTKEY,1,&hotkey);
      }
      break;
    case DN_CONTROLINPUT:
      {
        const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
        if(record->EventType==KEY_EVENT&&record->Event.KeyEvent.bKeyDown)
        {
          WORD vk=record->Event.KeyEvent.wVirtualKeyCode;
          if(IsNone(record))
          {
            switch(vk)
            {
              case VK_DELETE:
                {
                  FarListDelete list={sizeof(FarListDelete),3,4};
                  Info.SendDlgMessage(hDlg,DM_LISTBOXEX_DELETE,1,&list);
                }
                break;
              case VK_F2:
                {
                  ListBoxExSetFlags flags={Info.SendDlgMessage(hDlg,DM_LISTBOXEX_GETCURPOS,1,0L),LIFEX_DISABLE};
                  Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETFLAGS,1,&flags);
                }
                break;
              case VK_F3:
                {
                  ListBoxExSetFlags flags={Info.SendDlgMessage(hDlg,DM_LISTBOXEX_GETCURPOS,1,0L)-1,0};
                  Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETFLAGS,1,&flags);
                }
                break;
              case VK_INSERT:
                Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_TOGGLE,1,(void*)Info.SendDlgMessage(hDlg,DM_LISTBOXEX_GETCURPOS,1,0L));
                break;
            }
          }
          else if(IsShift(record))
          {
            switch(vk)
            {
              case VK_DELETE:
                {
                  ListBoxExSetFlags flags={Info.SendDlgMessage(hDlg,DM_LISTBOXEX_GETCURPOS,1,0L),LIFEX_HIDDEN};
                  Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETFLAGS,1,&flags);
                }
                break;
            }
          }
          else if(IsCtrl(record))
          {
            switch(vk)
            {
              case VK_UP:
                {
                  FarListPos pos={sizeof(FarListPos),Info.SendDlgMessage(hDlg,DM_LISTBOXEX_GETCURPOS,1,0L),-1};
                  Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_MOVE_UP,1,(void*)(INT_PTR)pos.SelectPos);
                  pos.SelectPos--;
                  Info.SendDlgMessage(hDlg,DM_LISTBOXEX_SETCURPOS,1,&pos);
                }
                break;
              case VK_DOWN:
                {
                  FarListPos pos={sizeof(FarListPos),Info.SendDlgMessage(hDlg,DM_LISTBOXEX_GETCURPOS,1,0L),-1};
                  Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_MOVE_DOWN,1,(void*)(INT_PTR)pos.SelectPos);
                  pos.SelectPos++;
                  Info.SendDlgMessage(hDlg,DM_LISTBOXEX_SETCURPOS,1,&pos);
                }
                break;
            }
          }
        }
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

HANDLE WINAPI OpenW(const struct OpenInfo*)
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
  FAR_CHAR_INFO *VirtualBuffer=(FAR_CHAR_INFO *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(DialogItems[1].X2-DialogItems[1].X1+1)*(DialogItems[1].Y2-DialogItems[1].Y1+1)*sizeof(FAR_CHAR_INFO));
  DialogItems[1].VBuf=VirtualBuffer;
  HANDLE hDlg=Info.DialogInit(&MainGuid,&DialogGuid,-1,-1,71,21,NULL,DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]),0,0,MainDialogProc,0);
  if(hDlg!=INVALID_HANDLE_VALUE)
  {
    Info.DialogRun(hDlg);
    Info.DialogFree(hDlg);
  }
  if(VirtualBuffer) HeapFree(GetProcessHeap(),0,VirtualBuffer);
  return(INVALID_HANDLE_VALUE);
}
