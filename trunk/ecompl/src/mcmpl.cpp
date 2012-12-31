/*
    mcmpl.cpp
    Copyright (C) 2000-2001 Igor Lyubimov
    Copyright (C) 2002-2008 zg

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

#include "plugin.hpp"
#include "farcolor.hpp"
#include "mcmpl.hpp"
#include "EditCmpl.hpp"
#include "language.hpp"
#include "../../listboxex/ListBoxEx.hpp"
#include "guid.hpp"
#include <initguid.h>
// {0FCE7966-950A-499f-9FA5-32FE0A981342}
DEFINE_GUID(MCmplGuid, 0xfce7966, 0x950a, 0x499f, 0x9f, 0xa5, 0x32, 0xfe, 0xa, 0x98, 0x13, 0x42);

TMenuCompletion::TMenuCompletion(): TCompletion()
{
  SingleVariantInMenu=FALSE;
  NotFoundSound=TRUE;
  SortListCount=10;
  _tcscpy(ShortCuts,_T("0123456789abcdefghijklmnopqrstuvwxyz"));
  ShortCutsLen=_tcslen(ShortCuts);
  AcceptChars[0]=0;
  _tcscpy(ConfigHelpTopic,_T("Config"));
  GetOptions();
}

TMenuCompletion::~TMenuCompletion()
{
}

bool TMenuCompletion::CompleteWord(void)
{
  bool result=false;
  if(GetPreWord()&&WordsToFindCnt)
  {
    if(DoSearch())
    {
      bool Insert=true;
      string NewWord=WordList.get_top()->get_data()->get_data();
      if(PartialCompletion&&(WordList.get_partial().length()>Word.length()))
      {
        NewWord=WordList.get_partial();
      }
      else if(SingleVariantInMenu||WordList.count()>1)
      {
        Insert=ShowMenu(NewWord);
      }
      if(Insert)
      {
        PutWord(NewWord);
        SetCurPos(WordPos+NewWord.length());
        result=true;
      }
    }
    else if(NotFoundSound) MessageBeep(MB_ICONASTERISK);
  }
  Cleanup();
  return result;
}

struct MenuAdaptor
{
  int ref;
  const string *data;
};

struct ListMenuData
{
  int count;
  MenuAdaptor *MenuData;
  //TCHAR *Top;
  //TCHAR *Bottom;
  TCHAR *ShortCuts;
  int ShortCutsLen;
  TCHAR *AcceptChars;
  unsigned LastHotkey;
  long ClosedKey;
  long CursorPos;
};

static void ForEach(void *data,int &pos,avl_word_data &node)
{
  MenuAdaptor *menudata=(MenuAdaptor *)data;
  menudata[pos].ref=node.get_ref();
  menudata[pos].data=&node.get_data();
  pos++;
}

int WINAPI fcmp(const void *first,const void *second,void *)
{
  return (((const MenuAdaptor *)second)->ref)-(((const MenuAdaptor *)first)->ref);
}

static intptr_t WINAPI ListMenuProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
  ListMenuData *DlgParams=(ListMenuData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_INITDIALOG:
      {
        Info.SendDlgMessage(hDlg,DM_LISTBOXEX_INIT,2,0);
        Info.SendDlgMessage(hDlg,DM_LISTBOXEX_SETFLAGS,2,(void*)LBFEX_WRAPMODE);
        {
          int ColorIndex[LISTBOXEX_COLOR_COUNT]={COL_MENUTEXT,COL_MENUTEXT,COL_MENUHIGHLIGHT,COL_MENUSELECTEDTEXT,COL_MENUSELECTEDHIGHLIGHT,COL_MENUDISABLEDTEXT};
          FarColor NewColors[LISTBOXEX_COLOR_COUNT];
          for(unsigned long i=0;i<sizeofa(ColorIndex);i++)
            Info.AdvControl(&MainGuid,ACTL_GETCOLOR,ColorIndex[i],NewColors+i);
          ListBoxExColors Colors={LISTBOXEX_COLOR_COUNT,NewColors};
          Info.SendDlgMessage(hDlg,DM_LISTBOXEX_SETCOLORS,2,&Colors);
        }
        for(int i=0;i<DlgParams->count;i++)
        {
          UTCHAR buffer[3];
          FSF.sprintf((TCHAR*)buffer,_T("%c "),(i<DlgParams->ShortCutsLen)?DlgParams->ShortCuts[i]:' ');
          string item=*DlgParams->MenuData[i].data,prefix(buffer);
          item=prefix+item;
          Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ADDSTR,2,(void*)((const UTCHAR*)item));
          if(i<DlgParams->ShortCutsLen)
          {
            ListBoxExSetColor color={i,LISTBOXEX_COLORS_ITEM,0,{{0},LISTBOXEX_COLOR_HOTKEY,true}};
            Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETCOLOR,2,&color);
            color.Color.Index=LISTBOXEX_COLOR_SELECTEDHOTKEY;
            color.TypeIndex=LISTBOXEX_COLORS_SELECTED;
            Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETCOLOR,2,&color);
            ListBoxExSetHotkey hotkey={i,DlgParams->ShortCuts[i]};
            Info.SendDlgMessage(hDlg,DM_LISTBOXEX_ITEM_SETHOTKEY,2,&hotkey);
          }
        }
      }
      break;
    case DN_CTLCOLORDIALOG:
      Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_MENUTEXT,Param2);
      break;
    case DN_CTLCOLORDLGITEM:
      switch(Param1)
      {
        case 0:
        case 1:
          {
            FarDialogItemColors* colors=(FarDialogItemColors*)Param2;
            if(colors->ColorsCount>=3)
            {
              Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_MENUTITLE,colors->Colors);
              Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_MENUTITLE,colors->Colors+1);
              Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_MENUBOX,colors->Colors+2);
            }
          }
          break;
      }
      break;
    case DN_CONTROLINPUT:
      {
        const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
        if(record->EventType==KEY_EVENT&&record->Event.KeyEvent.bKeyDown)
        {
          unsigned long key=record->Event.KeyEvent.uChar.UnicodeChar;
          if(key&&_tcschr(DlgParams->AcceptChars,key))
          {
            DlgParams->ClosedKey=key;
            Info.SendDlgMessage(hDlg,DM_CLOSE,-1,NULL);
          }
        }
      }
      break;
    case DN_CLOSE:
      DlgParams->CursorPos=Info.SendDlgMessage(hDlg,DM_LISTBOXEX_GETCURPOS,2,NULL);
      break;
    case DM_LISTBOXEX_ISLBE:
      if(Param1==2) return TRUE;
      return FALSE;
    case DN_LISTBOXEX_HOTKEY:
      Info.SendDlgMessage(hDlg,DM_CLOSE,2,NULL);
      break;
  }
  return ListBoxExDialogProc(hDlg,Msg,Param1,Param2);
}


#define MENU_OVERHEAD_WIDTH 6 //6 => 2 рамка, 2 тень, 2 место для чекмарка
#define MENU_OVERHEAD_HEIGHT 3 //6 => 2 рамка, 1 тень

bool TMenuCompletion::ShowMenu(string &Selected)
{
  int result=false;
  FarDialogItem DialogItems[3];
  memset(DialogItems,0,sizeof(DialogItems));
  DialogItems[0].Type=DI_DOUBLEBOX; DialogItems[1].Type=DI_TEXT; DialogItems[2].Type=DI_USERCONTROL;
  MenuAdaptor *menudata=(MenuAdaptor *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,WordList.count()*sizeof(MenuAdaptor));
  if(menudata)
  {
    int pos=0;
    WordList.iterate((void *)menudata,pos,ForEach);
    if(WordList.count()<=SortListCount) FSF.qsort(menudata,WordList.count(),sizeof(*menudata),fcmp,NULL);

    TCHAR BottomMsg[256];
    FSF.sprintf(BottomMsg,GetMsg(MHave),WordList.count());

    EditorInfo ei={sizeof(ei)};
    Info.EditorControl(-1,ECTL_GETINFO,0,&ei);

    size_t MenuWidth=MAX(WordList.get_max_len()+2,_tcslen(GetMsg(MChooseWord))+1);
    MenuWidth=MAX(MenuWidth,_tcslen(BottomMsg));
    size_t MenuHeight=1;
    int CoorX=ei.CurTabPos-ei.LeftPos;
    int CoorY=ei.CurLine-ei.TopScreenLine;
    int MenuX=MAX(0,CoorX+1-((signed)(Word.length()))-(signed)MenuWidth-MENU_OVERHEAD_WIDTH);
    MenuX=(ei.WindowSizeX-CoorX)>(CoorX+2-((signed)(Word.length())))?CoorX+1:MenuX; //меню права или слева от слова?
    int MenuY=0;
    if((ei.WindowSizeY-CoorY-1)>CoorY+1) //меню сверху или снизу?
    { //снизу
      MenuY=CoorY+2;
      MenuHeight=ei.WindowSizeY-MenuY+1-MENU_OVERHEAD_HEIGHT;
      if((signed)MenuHeight>WordList.count()) MenuHeight=WordList.count();
    }
    else
    { //сверху
      MenuY=CoorY-WordList.count()-1;
      if(MenuY<1) MenuY=1;
      MenuHeight=CoorY-MenuY-1;
    }

    //fix menu width
    if((MenuX+MenuWidth+MENU_OVERHEAD_WIDTH)>(unsigned int)ei.WindowSizeX)
      MenuWidth=ei.WindowSizeX-MenuX-MENU_OVERHEAD_WIDTH;

    DialogItems[0].X1=0; DialogItems[0].X2=MenuWidth+3; DialogItems[0].Y1=0; DialogItems[0].Y2=MenuHeight+1;
    INIT_DLG_DATA(DialogItems[0],GetMsg(MChooseWord));
    DialogItems[1].X1=(MenuWidth+4-_tcslen(BottomMsg))/2; DialogItems[1].Y1=DialogItems[0].Y2;
    INIT_DLG_DATA(DialogItems[1],BottomMsg);
    DialogItems[2].X1=DialogItems[0].X1+1; DialogItems[2].Y1=DialogItems[0].Y1+1; DialogItems[2].X2=DialogItems[0].X2-1; DialogItems[2].Y2=DialogItems[0].Y2-1;
    FAR_CHAR_INFO* VirtualBuffer=(FAR_CHAR_INFO*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(DialogItems[2].X2-DialogItems[2].X1+1)*(DialogItems[2].Y2-DialogItems[2].Y1+1)*sizeof(FAR_CHAR_INFO));
    if(VirtualBuffer)
    {
      DialogItems[2].VBuf=VirtualBuffer;
      ListMenuData params={WordList.count(),menudata,/*(TCHAR *)GetMsg(MChooseWord),BottomMsg,*/ShortCuts,ShortCutsLen,AcceptChars,0,0,-1};
      CFarDialog dialog;
      intptr_t DlgCode=dialog.Execute(MainGuid,MCmplGuid,MenuX,MenuY,MenuX+MenuWidth+3,MenuY+MenuHeight+1,_T("List"),DialogItems,sizeofa(DialogItems),0,0,ListMenuProc,&params);
      if(DlgCode==2)
      {
        int MenuCode=params.CursorPos;
        if(MenuCode>-1)
        {
          result=true;
          Selected=*menudata[MenuCode].data;
          if(params.ClosedKey) Selected+=params.ClosedKey;
        }
      }
      HeapFree(GetProcessHeap(),0,VirtualBuffer);
    }
    HeapFree(GetProcessHeap(),0,menudata);
  }
  return result;
}

void TMenuCompletion::GetOptions(void)
{
  TCompletion::GetOptions();
  FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
  if(Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
  {
    intptr_t root=Root(settings.Handle);
    SingleVariantInMenu=GetValue(settings.Handle,root,_T("SingleVariantInMenu"),SingleVariantInMenu);
    NotFoundSound=GetValue(settings.Handle,root,_T("NotFoundSound"),NotFoundSound);
    SortListCount=GetValue(settings.Handle,root,_T("SortListCount"),SortListCount);
    AsteriskSymbol=GetValue(settings.Handle,root,_T("AsteriskSymbol"),AsteriskSymbol);
    GetValue(settings.Handle,root,_T("AcceptChars"),AcceptChars,ArraySize(AcceptChars));
    GetValue(settings.Handle,root,_T("ShortCuts"),ShortCuts,ArraySize(ShortCuts));

    Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
  }
}

void TMenuCompletion::SetOptions(void)
{
  TCompletion::SetOptions();
  FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
  if(Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
  {
    intptr_t root=Root(settings.Handle);
    SetValue(settings.Handle,root,_T("SingleVariantInMenu"),SingleVariantInMenu);
    SetValue(settings.Handle,root,_T("NotFoundSound"),NotFoundSound);
    SetValue(settings.Handle,root,_T("SortListCount"),SortListCount);
    SetValue(settings.Handle,root,_T("AsteriskSymbol"),AsteriskSymbol);
    SetValue(settings.Handle,root,_T("AcceptChars"),AcceptChars);
    SetValue(settings.Handle,root,_T("ShortCuts"),ShortCuts);

    Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
  }
}

int TMenuCompletion::GetItemCount(void)
{
  return MCMPL_DIALOG_ITEMS;
}

int TMenuCompletion::DialogWidth(void)
{
  return 42;
}

int TMenuCompletion::DialogHeight(void)
{
  return 23;
}

INT_PTR TMenuCompletion::DialogProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
  if(Msg==DN_INITDIALOG)
  {
    Info.SendDlgMessage(hDlg,DM_SETMAXTEXTLENGTH,IMenuAcceptChars,(void*)(sizeof(AcceptChars)-1));
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void TMenuCompletion::InitItems(FarDialogItem *DialogItems)
{
  TCompletion::InitItems(DialogItems);
  int Msgs[]=
  {
    MSingleVariantInMenu,
    MNotFoundSound,
    MSortListCount,MSortListCount,
    MAsteriskSymbol,MAsteriskSymbol,
    MAcceptChars,MAcceptChars,
  };
  int DialogElements[][4]=
  {
    {DI_CHECKBOX,  3, 14,  0  }, // SingleVariantInMenu
    {DI_CHECKBOX,  3, 15,  0  }, // NotFoundSound
    {DI_TEXT,      6, 16,  0  }, //
    {DI_FIXEDIT,   3, 16,  4  }, // SortListCount
    {DI_TEXT,      6, 17,  0  }, //
    {DI_FIXEDIT,   3, 17,  3  }, // AsteriskSymbol
    {DI_TEXT,      6, 19,  0  }, //
    {DI_EDIT,      3, 19,  4  }, // AcceptChars
  };

  for(unsigned int i=0;i<(sizeof(Msgs)/sizeof(Msgs[0]));i++)
  {
    DialogItems[i+CMPL_DIALOG_ITEMS].Type=(FARDIALOGITEMTYPES)DialogElements[i][0];
    DialogItems[i+CMPL_DIALOG_ITEMS].X1=DialogElements[i][1];
    DialogItems[i+CMPL_DIALOG_ITEMS].Y1=DialogElements[i][2];
    DialogItems[i+CMPL_DIALOG_ITEMS].X2=DialogElements[i][3];
    DialogItems[i+CMPL_DIALOG_ITEMS].Y2=0;
    DialogItems[i+CMPL_DIALOG_ITEMS].Selected=0;
    DialogItems[i+CMPL_DIALOG_ITEMS].Flags=0;
    INIT_DLG_DATA(DialogItems[i+CMPL_DIALOG_ITEMS],GetMsg(Msgs[i])); // Надписи на эл-тах диалога
  }

  DialogItems[ISingleVariantInMenu].Selected=SingleVariantInMenu;
  DialogItems[INotFoundSound].Selected=NotFoundSound;

  // Что будет в строках ввода
  DLG_DATA_ITOA(DialogItems[ISortListCount],SortListCount);
  AsteriskSymbolText[0]=AsteriskSymbol;
  AsteriskSymbolText[1]=0;
  DialogItems[IAsteriskSymbol].Data=AsteriskSymbolText;
  INIT_DLG_DATA(DialogItems[IMenuAcceptChars],AcceptChars);

  DialogItems[IMenuAcceptChars].X2=DialogWidth()-_tcslen(GetMsg(MAcceptChars))-4;
  DialogItems[IMenuAcceptCharsLabel].X1=DialogItems[IMenuAcceptChars].X2+2;
  INIT_DLG_DATA(DialogItems[ICfg],GetMsg(MMenuCfg)); // Заголовок
}

void TMenuCompletion::StoreItems(CFarDialog& Dialog)
{
  TCompletion::StoreItems(Dialog);
  SingleVariantInMenu=Dialog.Check(ISingleVariantInMenu);
  NotFoundSound=Dialog.Check(INotFoundSound);
  AsteriskSymbol=Dialog.Str(IAsteriskSymbol)[0];
  SortListCount=FSF.atoi(Dialog.Str(ISortListCount));
  _tcscpy(AcceptChars,Dialog.Str(IMenuAcceptChars));
}
