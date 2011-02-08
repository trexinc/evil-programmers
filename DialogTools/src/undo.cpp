/*
    undo.cpp
    Copyright (C) 2004 Vadim Yegorov
    Copyright (C) 2008 zg

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

#include "dt.hpp"

struct UndoItem
{
  TCHAR *data;
  COORD pos;
  long unchanged;
  UndoItem *next;
};

static UndoItem *Top=NULL,*Stack=NULL;
static HANDLE CurrDialog=INVALID_HANDLE_VALUE;
static int CurrItem=-1;
static int inside_undo=0;

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
    long length=Info.SendDlgMessage(hDlg,DM_GETTEXTLENGTH,item,0);
    NewStack->data=(TCHAR *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(length+1)*sizeof(TCHAR));
    if(NewStack->data)
    {
      Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,item,(long)NewStack->data);
      NewStack->unchanged=Info.SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,item,-1);
      Info.SendDlgMessage(hDlg,DM_GETCURSORPOS,item,(LONG_PTR)&NewStack->pos);
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
  inside_undo++;
  Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,item,(long)Data->data);
  Info.SendDlgMessage(hDlg,DM_SETCURSORPOS,item,(LONG_PTR)&Data->pos);
  Info.SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,item,Data->unchanged);
  inside_undo--;
}

void FinishUndo(void)
{
  free_stack();
}

void DoUndo(HANDLE aDlg)
{
  LONG_PTR itemID=Info.SendDlgMessage(aDlg,DM_GETFOCUS,0,0);
  FarMenuItem MenuItems[]={{MIF_SELECTED,_T(""),0,0,0},{0,_T(""),0,0,0}};
  INIT_MENU_TEXT(0,GetMsg(mUndo)); INIT_MENU_TEXT(1,GetMsg(mRedo));
  int MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,GetMsg(mNameUndo),NULL,NULL,NULL,NULL,MenuItems,ArraySize(MenuItems));
  if(MenuCode>=0)
  {
    FarDialogItem DialogItem;
    Info.SendDlgMessage(aDlg,DM_GETDLGITEMSHORT,itemID,(LONG_PTR)&DialogItem);
    if(DialogItem.Type==DI_EDIT&&Stack)
    {
      if(MenuCode==0&&Top)
      {
        restore_state(Top,aDlg,itemID);
        Top=Top->next;
      }
      else if(MenuCode==1&&Stack->next!=Top)
      {
        UndoItem *NewTop=Stack;
        while(NewTop->next&&NewTop->next->next!=Top) NewTop=NewTop->next;
        if(NewTop->next)
        {
          restore_state(NewTop,aDlg,itemID);
          Top=NewTop->next;
        }
      }
    }
  }
}

void FilterUndo(HANDLE aDlg,int aMsg,int aParam1,LONG_PTR aParam2)
{
  if(inside_undo) return;
  if(aMsg==DN_GOTFOCUS||aMsg==DN_INITDIALOG||(aMsg==DN_EDITCHANGE&&!Stack))
  {
    free_stack();
    FarDialogItem DialogItem;
    Info.SendDlgMessage(aDlg,DM_GETDLGITEMSHORT,aParam1,(LONG_PTR)&DialogItem);
    if(DialogItem.Type==DI_EDIT)
    {
      Top=Stack;
      Stack=add_to_stack(Stack,aDlg,aParam1);
    }
  }
  else if(aMsg==DN_KILLFOCUS)
  {
    free_stack();
  }
  else if(aMsg==DN_EDITCHANGE&&Stack)
  {
    if(CurrDialog!=aDlg||CurrItem!=aParam1) free_stack();
    pop_stack();
    Top=Stack;
    Stack=add_to_stack(Stack,aDlg,aParam1);
  }
}
