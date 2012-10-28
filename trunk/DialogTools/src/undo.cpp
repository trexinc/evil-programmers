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
#include "guid.hpp"
#include "avl.hpp"

struct UndoItem
{
  TCHAR *data;
  COORD pos;
  long unchanged;
  UndoItem *next;
};

class UndoData
{
  private:
    UndoItem* Top;
    UndoItem* Stack;
    HANDLE Dialog;
    intptr_t Item;
  private:
    UndoData();
    UndoData(const UndoData& value);
    UndoData& operator=(const UndoData& value);
    void Restore(UndoItem* Data);
  public:
    UndoData(HANDLE value,intptr_t item);
    ~UndoData();
    void Add(HANDLE value,intptr_t item);
    void Pop(void);
    void SetTop(void) {Top=Stack;}
    void Undo(void);
    void Redo(void);
  friend class UndoTree;
};

UndoData::UndoData(HANDLE value,intptr_t item): Top(NULL),Stack(NULL),Dialog(value),Item(item)
{
}

UndoData::~UndoData()
{
  UndoItem* curr_stack;
  while(Stack)
  {
    curr_stack=Stack;
    Stack=Stack->next;
    if(curr_stack->data) HeapFree(GetProcessHeap(),0,curr_stack->data);
    HeapFree(GetProcessHeap(),0,curr_stack);
  }
}

void UndoData::Add(HANDLE value,intptr_t item)
{
  UndoItem* NewStack=(UndoItem*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(UndoItem));
  if(NewStack)
  {
    size_t length=Info.SendDlgMessage(value,DM_GETTEXT,item,0);
    NewStack->data=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(length+1)*sizeof(TCHAR));
    if(NewStack->data)
    {
      FarDialogItemData getdata={sizeof(FarDialogItemData),length,NewStack->data};
      Info.SendDlgMessage(value,DM_GETTEXT,item,&getdata);
      NewStack->unchanged=Info.SendDlgMessage(value,DM_EDITUNCHANGEDFLAG,item,(void*)-1);
      Info.SendDlgMessage(value,DM_GETCURSORPOS,item,&NewStack->pos);
      NewStack->next=Stack;
      Stack=NewStack;
    }
    else
    {
      HeapFree(GetProcessHeap(),0,NewStack);
    }
  }
}

void UndoData::Pop(void)
{
  if(Stack)
  {
    UndoItem* curr_stack;
    while(Stack&&Stack->next!=Top)
    {
      curr_stack=Stack;
      Stack=Stack->next;
      if(curr_stack->data) HeapFree(GetProcessHeap(),0,curr_stack->data);
      HeapFree(GetProcessHeap(),0,curr_stack);
    }
  }
}

static int inside_undo=0;
void UndoData::Restore(UndoItem* Data)
{
  inside_undo++;
  Info.SendDlgMessage(Dialog,DM_SETTEXTPTR,Item,Data->data);
  Info.SendDlgMessage(Dialog,DM_SETCURSORPOS,Item,&Data->pos);
  Info.SendDlgMessage(Dialog,DM_EDITUNCHANGEDFLAG,Item,(void*)(intptr_t)Data->unchanged);
  inside_undo--;
}

void UndoData::Undo(void)
{
  if(Top)
  {
    Restore(Top);
    Top=Top->next;
  }
}

void UndoData::Redo(void)
{
  if(Stack->next!=Top)
  {
    UndoItem* NewTop=Stack;
    while(NewTop->next&&NewTop->next->next!=Top) NewTop=NewTop->next;
    if(NewTop->next)
    {
      Restore(NewTop);
      Top=NewTop->next;
    }
  }
}

class UndoTree: public avl_tree<UndoData>
{
  public:
    UndoTree();
    ~UndoTree();
    long compare(avl_node<UndoData> *first,UndoData *second);
    UndoData *query(HANDLE value,intptr_t item);
    void Undo(HANDLE value,intptr_t item);
    void Redo(HANDLE value,intptr_t item);
};

UndoTree::UndoTree()
{
  clear();
}

UndoTree::~UndoTree()
{
  clear();
}

long UndoTree::compare(avl_node<UndoData> *first,UndoData *second)
{
  long result=reinterpret_cast<intptr_t>(second->Dialog)-reinterpret_cast<intptr_t>(first->data->Dialog);
  if(!result) result=second->Item-first->data->Item;
  return result;
}

UndoData *UndoTree::query(HANDLE value,intptr_t item)
{
  UndoData Get(value,item);
  return avl_tree<UndoData>::query(&Get);
}

void UndoTree::Undo(HANDLE value,intptr_t item)
{
  UndoData* data=query(value,item);
  if(data) data->Undo();
}

void UndoTree::Redo(HANDLE value,intptr_t item)
{
  UndoData* data=query(value,item);
  if(data) data->Redo();
}

static UndoTree* GUndo=NULL;

void InitUndo(void)
{
  GUndo=new UndoTree;
}

void FinishUndo(void)
{
  delete GUndo;
}

void DoUndo(HANDLE aDlg)
{
  intptr_t itemID=Info.SendDlgMessage(aDlg,DM_GETFOCUS,0,0);
  FarMenuItem MenuItems[]={{MIF_SELECTED,_T(""),{0},0,{0,0}},{0,_T(""),{0},0,{0,0}}};
  INIT_MENU_TEXT(0,GetMsg(mUndo)); INIT_MENU_TEXT(1,GetMsg(mRedo));
  int MenuCode=Info.Menu(&MainGuid,&UndoMenuGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,GetMsg(mNameUndo),NULL,NULL,NULL,NULL,MenuItems,ArraySize(MenuItems));
  if(MenuCode>=0)
  {
    FarDialogItem DialogItem;
    Info.SendDlgMessage(aDlg,DM_GETDLGITEMSHORT,itemID,&DialogItem);
    if(DialogItem.Type==DI_EDIT)
    {
      switch(MenuCode)
      {
        case 0:
          GUndo->Undo(aDlg,itemID);
          break;
        case 1:
          GUndo->Redo(aDlg,itemID);
          break;
      }
    }
  }
}

void FilterUndoInit(HANDLE aDlg,intptr_t aMsg,intptr_t aParam1,void* aParam2)
{
  if(inside_undo) return;
  if(DN_EDITCHANGE==aMsg||DN_GOTFOCUS==aMsg)
  {
    FarDialogItem DialogItem;
    Info.SendDlgMessage(aDlg,DM_GETDLGITEMSHORT,aParam1,&DialogItem);
    if(DialogItem.Type==DI_EDIT)
    {
      bool forceadd=true;
      UndoData* data=GUndo->query(aDlg,aParam1);
      if(!data)
      {
        data=new UndoData(aDlg,aParam1);
        data=GUndo->insert(data);
      }
      else
      {
        if(DN_GOTFOCUS==aMsg) forceadd=false;
        else data->Pop();
      }
      if(forceadd)
      {
        data->SetTop();
        data->Add(aDlg,aParam1);
      }
    }
  }
}

void FilterUndoEnd(HANDLE aDlg,intptr_t aMsg,intptr_t aParam1,void* aParam2,intptr_t Result)
{
  if(DN_CLOSE==aMsg&&Result)
  {
    FarDialogItem dialog_item;
    for(size_t ii=0;Info.SendDlgMessage(aDlg,DM_GETDLGITEMSHORT,ii,&dialog_item);++ii)
    {
      UndoData del(aDlg,ii);
      GUndo->remove(&del);
    }
  }
}
