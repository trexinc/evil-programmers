/*
    Undo plugin for DialogManager
    Copyright (C) 2004 Vadim Yegorov

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
#include "undo.hpp"
#include <stdio.h>

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}

PluginStartupInfo FarInfo;
FARSTANDARDFUNCTIONS FSF;
DialogPluginStartupInfo DialogInfo;
char PluginRootKey[100];

static UndoItem *Top=NULL,*Stack=NULL;
static HANDLE CurrDialog=INVALID_HANDLE_VALUE;
static int CurrItem=-1;

static char *GetMsg(int MsgNum,char *Str)
{
  DialogInfo.GetMsg(DialogInfo.MessageName,MsgNum,Str);
  return Str;
}

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct DialogPluginStartupInfo *DialogInfo)
{
  ::FarInfo=*FarInfo;
  FSF=*FarInfo->FSF;
  ::DialogInfo=*DialogInfo;
  FSF.sprintf(PluginRootKey,"%s\\Undo",::DialogInfo.RootKey);
  return 0;
}

static void free_stack(void)
{
  UndoItem *curr_stack;
  while(Stack)
  {
    curr_stack=Stack;
    Stack=Stack->next;
    if(curr_stack->data) HeapFree(GetProcessHeap(),0,curr_stack->data);
    HeapFree(GetProcessHeap(),0,curr_stack);
  }
  Top=NULL;
  CurrDialog=INVALID_HANDLE_VALUE;
  CurrItem=-1;
}

static UndoItem *add_to_stack(UndoItem *OldStack,HANDLE hDlg,int item)
{
  UndoItem *NewStack=(UndoItem *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(UndoItem));
  if(NewStack)
  {
    long length=FarInfo.SendDlgMessage(hDlg,DM_GETTEXTLENGTH,item,0);
    NewStack->data=(char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,length+1);
    if(NewStack->data)
    {
      FarInfo.SendDlgMessage(hDlg,DM_GETTEXTPTR,item,(long)NewStack->data);
      NewStack->unchanged=FarInfo.SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,item,-1);
      FarInfo.SendDlgMessage(hDlg,DM_GETCURSORPOS,item,(long)&NewStack->pos);
      NewStack->next=OldStack;
      CurrDialog=hDlg;
      CurrItem=item;
    }
    else
    {
      HeapFree(GetProcessHeap(),0,NewStack);
      NewStack=OldStack;
    }
  } else NewStack=OldStack;
  return NewStack;
}

static void pop_stack(void)
{
  if(Stack)
  {
    UndoItem *curr_stack;
    while(Stack&&Stack->next!=Top)
    {
      curr_stack=Stack;
      Stack=Stack->next;
      if(curr_stack->data) HeapFree(GetProcessHeap(),0,curr_stack->data);
      HeapFree(GetProcessHeap(),0,curr_stack);
    }
  }
}

static void restore_state(UndoItem *Data,HANDLE hDlg,int item)
{
  FarInfo.SendDlgMessage(hDlg,DM_SETTEXTPTR,item,(long)Data->data);
  FarInfo.SendDlgMessage(hDlg,DM_SETCURSORPOS,item,(long)&Data->pos);
  FarInfo.SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,item,Data->unchanged);
}

void WINAPI _export Exit(void)
{
  free_stack();
}

int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData)
{
  (void)OutData;
  switch(Msg)
  {
    case FMMSG_GETINFO:
      {
        GetInfoOutData *data=(GetInfoOutData *)OutData;
        GetMsg(mName,data->MenuString);
        strcpy(data->HotkeyID,"zg_undo");
        data->Flags=FMMSG_FILTER|FMMSG_MENU;
      }
      return TRUE;
    case FMMSG_MENU:
      {
        MenuInData *dlg=(MenuInData *)InData;
        FarMenuItem MenuItems[]={{"",1,0,0},{"",0,0,0}};
        GetMsg(mUndo,MenuItems[0].Text); GetMsg(mRedo,MenuItems[1].Text);
        char msg1[128]; GetMsg(mName,msg1);
        int MenuCode=FarInfo.Menu(FarInfo.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,msg1,NULL,NULL,NULL,NULL,MenuItems,sizeof(MenuItems)/sizeof(MenuItems[0]));
        if(MenuCode>=0)
        {
          FarDialogItem DialogItem;
          FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,dlg->ItemID,(long)&DialogItem);
          if(DialogItem.Type==DI_EDIT&&Stack)
          {
            if(MenuCode==0&&Top)
            {
              restore_state(Top,dlg->hDlg,dlg->ItemID);
              Top=Top->next;
            }
            else if(MenuCode==1&&Stack->next!=Top)
            {
              UndoItem *NewTop=Stack;
              while(NewTop->next&&NewTop->next->next!=Top) NewTop=NewTop->next;
              if(NewTop->next)
              {
                restore_state(NewTop,dlg->hDlg,dlg->ItemID);
                Top=NewTop->next;
              }
            }
          }
        }
      }
      return TRUE;
    case FMMSG_FILTER:
      {
        FilterInData *dlg=(FilterInData *)InData;
        if(dlg->Msg==DN_GOTFOCUS||dlg->Msg==DN_INITDIALOG||(dlg->Msg==DN_EDITCHANGE&&!Stack))
        {
          free_stack();
          FarDialogItem DialogItem;
          FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,dlg->Param1,(long)&DialogItem);
          if(DialogItem.Type==DI_EDIT)
          {
            Top=Stack;
            Stack=add_to_stack(Stack,dlg->hDlg,dlg->Param1);
          }
        }
        else if(dlg->Msg==DN_KILLFOCUS)
        {
          free_stack();
        }
        else if(dlg->Msg==DN_EDITCHANGE&&Stack)
        {
          if(CurrDialog!=dlg->hDlg||CurrItem!=dlg->Param1) free_stack();
          pop_stack();
          Top=Stack;
          Stack=add_to_stack(Stack,dlg->hDlg,dlg->Param1);
        }
      }
      return TRUE;
  }
  return FALSE;
}
