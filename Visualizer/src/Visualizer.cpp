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
#include "Visualizer.hpp"
#include "registry.hpp"

#if defined(__GNUC__)
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
#endif

struct PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
TCHAR PluginRootKey[80];
HMODULE hEsc;
int (WINAPI *GetEditorSettings)(int EditorID, const TCHAR *szName, void *Param);

struct Options
{
  int OnOffSwitch;
  int OtherColoringPlugins;
  int ShowRightBorderOn;
  int ShowEOLOn;
  int ShowEOFOn;
  int ShowTabsOn;
  int ShowCrossOn;
  int ShowCursorOn;
  int ShowLineNumbersOn;
  int ShowScrollbarOn;
  int ShowRightBorder;
  int ShowEOL;
  //int ShowEOF;
  int ShowTabs;
  int ShowCross;
  //int ShowCursor;
  //int ShowLineNumbers;
  int ShowScrollbar;
  int ShowCrossOnTop;
  int ShowTabSymbol;
  int TabSymbol;
  int ShowBookmarks;
  int ShowStackBookmarks;
  int ColorOfRightBorder;
  int ColorOfEOLNormal;
  int ColorOfEOLCR;
  int ColorOfEOLLF;
  int ColorOfEOLNULL;
  int ColorOfEOF;
  int ColorOfTabs;
  int ColorOfTabs2;
  int ColorOfCrossVertical;
  int ColorOfCrossHorizontal;
  int ColorOfCursor;
  int ColorOfScrollbar;
  int ColorOfScrollbarPositionMarker;
  int ColorOfBookmarks;
  int ColorOfStackBookmarks;
  //int HotkeyOfCross;
} Opt;

enum ENUMRightBorder
{
  RB_ON,
  RB_ONONLYIFAUTOWRAP,

  RB_MAX,
};

enum ENUMEOL
{
  EOL_ON,
  EOL_MARKALL,
  EOL_MARKALLWITHSYMBOLS,
  EOL_MARKWITHSYMBOL,

  EOL_MAX,
};

/*
enum ENUMEOF
{
  EOF_ON,

  EOF_MAX,
};
*/

enum ENUMTabs
{
  TAB_ON,
  TAB_WHOLE,
  TAB_TWOCOLORS,

  TAB_MAX,
};

enum ENUMCross
{
  CROSS_ON,
  CROSS_VERTICAL,
  CROSS_HORIZONTAL,
  CROSS_SMALL,
  //CROSS_HOTKEY,

  CROSS_MAX,
};

/*
enum ENUMCursor
{
  CURSOR_ON,

  CURSOR_MAX,
};
*/

/*
enum ENUMLineNumbers
{
  LINENUMBERS_ON,

  LINENUMBERS_MAX,
};
*/

enum ENUMScrollbar
{
  SCROLLBAR_ON,
  SCROLLBAR_VARIABLE,
  SCROLLBAR_FILLBAR,
  SCROLLBAR_NOBACK,
  SCROLLBAR_VARIABLENOBACK,
  SCROLLBAR_FILLBARNOBACK,

  SCROLLBAR_MAX,
};

const TCHAR *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

void InitDialogItems(const struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber)
{
  struct FarDialogItem *PItem=Item;
  const struct InitDialogItem *PInit=Init;
  for (int I=0;I<ItemsNumber;I++,PItem++,PInit++)
  {
    PItem->Type=PInit->Type;
    PItem->X1=PInit->X1;
    PItem->Y1=PInit->Y1;
    PItem->X2=PInit->X2;
    PItem->Y2=PInit->Y2;
    PItem->Focus=0;
    PItem->Selected=0;
    PItem->Flags=PInit->Flags;
    PItem->DefaultButton=0;
#ifdef UNICODE
    PItem->MaxLen=0;
#endif
    if (PInit->Data>=0)
    {
#ifndef UNICODE
      FSF.sprintf(PItem->Data,"%s",GetMsg(PInit->Data));
#else
      PItem->PtrData = GetMsg(PInit->Data);
#endif
    }
    else
    {
#ifndef UNICODE
      *(PItem->Data)=0;
#else
      PItem->PtrData = L"";
#endif
    }
  }
}

void ReadRegistry()
{
  GetRegKey(_T("OnOffSwitch"),&Opt.OnOffSwitch,1);
  GetRegKey(_T("OtherColoringPlugins"),&Opt.OtherColoringPlugins,1);

  GetRegKey(_T("ShowRightBorderOn"),&Opt.ShowRightBorderOn,1);
  GetRegKey(_T("ShowEOLOn"),&Opt.ShowEOLOn,1);
  GetRegKey(_T("ShowEOFOn"),&Opt.ShowEOFOn,1);
  GetRegKey(_T("ShowTabsOn"),&Opt.ShowTabsOn,1);
  GetRegKey(_T("ShowCrossOn"),&Opt.ShowCrossOn,1);
  GetRegKey(_T("ShowCursorOn"),&Opt.ShowCursorOn,1);
  GetRegKey(_T("ShowLineNumbersOn"),&Opt.ShowLineNumbersOn,1);
  GetRegKey(_T("ShowScrollbarOn"),&Opt.ShowScrollbarOn,1);

  GetRegKey(_T("ShowRightBorder"),&Opt.ShowRightBorder,RB_ON);
  Opt.ShowRightBorder%=RB_MAX;
  GetRegKey(_T("ShowEOL"),&Opt.ShowEOL,EOL_ON);
  Opt.ShowEOL%=EOL_MAX;
  //GetRegKey(_T("ShowEOF"),&Opt.ShowEOF,EOF_ON);
  //Opt.ShowEOF%=EOF_MAX;
  GetRegKey(_T("ShowTabs"),&Opt.ShowTabs,TAB_ON);
  Opt.ShowTabs%=TAB_MAX;
  GetRegKey(_T("ShowCross"),&Opt.ShowCross,CROSS_ON);
  Opt.ShowCross%=CROSS_MAX;
  //GetRegKey(_T("ShowCursor"),&Opt.ShowCursor,CURSOR_ON);
  //Opt.ShowCursor%=CURSOR_MAX;
  //GetRegKey(_T("ShowLineNumbers"),&Opt.ShowLineNumbers,LINENUMBERS_ON);
  //Opt.ShowLineNumbers%=LINENUMBERS_MAX;
  GetRegKey(_T("ShowScrollbar"),&Opt.ShowScrollbar,SCROLLBAR_ON);
  Opt.ShowScrollbar%=SCROLLBAR_MAX;

  GetRegKey(_T("ShowCrossOnTop"),&Opt.ShowCrossOnTop,0);
  GetRegKey(_T("ShowTabSymbol"),&Opt.ShowTabSymbol,0);
  GetRegKey(_T("TabSymbol"),&Opt.TabSymbol,DEFTABSYMBOL);
  GetRegKey(_T("ShowBookmarks"),&Opt.ShowBookmarks,1);
  GetRegKey(_T("ShowStackBookmarks"),&Opt.ShowStackBookmarks,1);

  GetRegKey(_T("ColorOfRightBorder"),&Opt.ColorOfRightBorder,0|5<<4);
  GetRegKey(_T("ColorOfEOLNormal"),&Opt.ColorOfEOLNormal,0|8<<4);
  GetRegKey(_T("ColorOfEOLCR"),&Opt.ColorOfEOLCR,0|8<<4);
  GetRegKey(_T("ColorOfEOLLF"),&Opt.ColorOfEOLLF,0|7<<4);
  GetRegKey(_T("ColorOfEOLNULL"),&Opt.ColorOfEOLNULL,0|12<<4);
  GetRegKey(_T("ColorOfEOF"),&Opt.ColorOfEOF,0|12<<4);
  GetRegKey(_T("ColorOfTabs"),&Opt.ColorOfTabs,0|15<<4);
  GetRegKey(_T("ColorOfTabs2"),&Opt.ColorOfTabs2,0|7<<4);
  GetRegKey(_T("ColorOfCrossVertical"),&Opt.ColorOfCrossVertical,0|7<<4);
  GetRegKey(_T("ColorOfCrossHorizontal"),&Opt.ColorOfCrossHorizontal,0|7<<4);
  GetRegKey(_T("ColorOfCursor"),&Opt.ColorOfCursor,0|12<<4);
  GetRegKey(_T("ColorOfScrollbar"),&Opt.ColorOfScrollbar,0|11<<4);
  GetRegKey(_T("ColorOfScrollbarPositionMarker"),&Opt.ColorOfScrollbarPositionMarker,0|9<<4);
  GetRegKey(_T("ColorOfBookmarks"),&Opt.ColorOfBookmarks,0|2<<4);
  GetRegKey(_T("ColorOfStackBookmarks"),&Opt.ColorOfStackBookmarks,0|6<<4);
  //GetRegKey(_T("HotkeyOfCross"),&Opt.HotkeyOfCross,);
}

void WriteRegistry()
{
  SetRegKey(_T("OnOffSwitch"),Opt.OnOffSwitch);
  SetRegKey(_T("OtherColoringPlugins"),Opt.OtherColoringPlugins);

  SetRegKey(_T("ShowRightBorderOn"),Opt.ShowRightBorderOn);
  SetRegKey(_T("ShowEOLOn"),Opt.ShowEOLOn);
  SetRegKey(_T("ShowEOFOn"),Opt.ShowEOFOn);
  SetRegKey(_T("ShowTabsOn"),Opt.ShowTabsOn);
  SetRegKey(_T("ShowCrossOn"),Opt.ShowCrossOn);
  SetRegKey(_T("ShowCursorOn"),Opt.ShowCursorOn);
  SetRegKey(_T("ShowLineNumbersOn"),Opt.ShowLineNumbersOn);
  SetRegKey(_T("ShowScrollbarOn"),Opt.ShowScrollbarOn);

  SetRegKey(_T("ShowRightBorder"),Opt.ShowRightBorder);
  SetRegKey(_T("ShowEOL"),Opt.ShowEOL);
  //SetRegKey(_T("ShowEOF"),Opt.ShowEOF);
  SetRegKey(_T("ShowTabs"),Opt.ShowTabs);
  SetRegKey(_T("ShowCross"),Opt.ShowCross);
  //SetRegKey(_T("ShowCursor"),Opt.ShowCursor);
  //SetRegKey(_T("ShowLineNumbers"),Opt.ShowLineNumbers);
  SetRegKey(_T("ShowScrollbar"),Opt.ShowScrollbar);

  SetRegKey(_T("ShowCrossOnTop"),Opt.ShowCrossOnTop);
  SetRegKey(_T("ShowTabSymbol"),Opt.ShowTabSymbol);
  SetRegKey(_T("TabSymbol"),Opt.TabSymbol);
  SetRegKey(_T("ShowBookmarks"),Opt.ShowBookmarks);
  SetRegKey(_T("ShowStackBookmarks"),Opt.ShowStackBookmarks);

  SetRegKey(_T("ColorOfRightBorder"),Opt.ColorOfRightBorder);
  SetRegKey(_T("ColorOfEOLNormal"),Opt.ColorOfEOLNormal);
  SetRegKey(_T("ColorOfEOLCR"),Opt.ColorOfEOLCR);
  SetRegKey(_T("ColorOfEOLLF"),Opt.ColorOfEOLLF);
  SetRegKey(_T("ColorOfEOLNULL"),Opt.ColorOfEOLNULL);
  SetRegKey(_T("ColorOfEOF"),Opt.ColorOfEOF);
  SetRegKey(_T("ColorOfTabs"),Opt.ColorOfTabs);
  SetRegKey(_T("ColorOfTabs2"),Opt.ColorOfTabs2);
  SetRegKey(_T("ColorOfCrossVertical"),Opt.ColorOfCrossVertical);
  SetRegKey(_T("ColorOfCrossHorizontal"),Opt.ColorOfCrossHorizontal);
  SetRegKey(_T("ColorOfCursor"),Opt.ColorOfCursor);
  SetRegKey(_T("ColorOfScrollbar"),Opt.ColorOfScrollbar);
  SetRegKey(_T("ColorOfScrollbarPositionMarker"),Opt.ColorOfScrollbarPositionMarker);
  SetRegKey(_T("ColorOfBookmarks"),Opt.ColorOfBookmarks);
  SetRegKey(_T("ColorOfStackBookmarks"),Opt.ColorOfStackBookmarks);
}

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *psi)
{
  Info=*psi;
  FSF=*psi->FSF;
  Info.FSF=&FSF;
  FSF.sprintf(PluginRootKey,_T("%s\\Visualizer"),Info.RootKey);
  ReadRegistry();
  GetEditorSettings=NULL;
  hEsc=GetModuleHandle(_T("esc.dll"));
  if (hEsc)
  {
#ifdef UNICODE
    GetEditorSettings=(int (WINAPI*)(int, const wchar_t*, void*))GetProcAddress(hEsc,"GetEditorSettingsW");
#else
    GetEditorSettings=(int (WINAPI*)(int, const char*, void*))GetProcAddress(hEsc,"GetEditorSettings");
#endif
  }
}

void SetColor(int *Color)
{
  int fg = (*Color)&0x0F;
  int bg = ((*Color)&0xF0)>>4;

  if (SelectColor(&fg,&bg))
    *Color = fg|(bg<<4);
}

void ConfigColor(struct Options *Opt)
{
  enum ENUMDlg
  {
    DLG_TITLE,

    DLG_RBCOLOR,
    DLG_EOLNORMALCOLOR,
    DLG_EOLCRCOLOR,
    DLG_EOLLFCOLOR,
    DLG_EOLNULLCOLOR,
    DLG_EOFCOLOR,
    DLG_TABCOLOR,
    DLG_TAB2COLOR,
    DLG_CROSSVERTICALCOLOR,
    DLG_CROSSHORIZONTALCOLOR,
    DLG_CURSORCOLOR,
    DLG_SCROLLBARCOLOR,
    DLG_SCROLLBARMARKERCOLOR,
    DLG_BOOKMARKSCOLOR,
    DLG_STACKBOOKMARKSCOLOR,

    DLG_SEP1,

    DLG_OK,
    DLG_CANCEL,
  };

  static const struct InitDialogItem PreDialogItems[] =
  {
    {DI_DOUBLEBOX   ,3  ,1  ,40 ,19 ,0               ,MColorTitle},

    {DI_BUTTON      ,0  ,2  ,0  ,0  ,DIF_CENTERGROUP ,MRightBorderColor},
    {DI_BUTTON      ,0  ,3  ,0  ,0  ,DIF_CENTERGROUP ,MEOLNormalColor},
    {DI_BUTTON      ,0  ,4  ,0  ,0  ,DIF_CENTERGROUP ,MEOLCRColor},
    {DI_BUTTON      ,0  ,5  ,0  ,0  ,DIF_CENTERGROUP ,MEOLLFColor},
    {DI_BUTTON      ,0  ,6  ,0  ,0  ,DIF_CENTERGROUP ,MEOLNULLColor},
    {DI_BUTTON      ,0  ,7  ,0  ,0  ,DIF_CENTERGROUP ,MEOLEOFColor},
    {DI_BUTTON      ,0  ,8  ,0  ,0  ,DIF_CENTERGROUP ,MTabsColor},
    {DI_BUTTON      ,0  ,9  ,0  ,0  ,DIF_CENTERGROUP ,MTabs2Color},
    {DI_BUTTON      ,0  ,10 ,0  ,0  ,DIF_CENTERGROUP ,MCrossVerticalColor},
    {DI_BUTTON      ,0  ,11 ,0  ,0  ,DIF_CENTERGROUP ,MCrossHorizontalColor},
    {DI_BUTTON      ,0  ,12 ,0  ,0  ,DIF_CENTERGROUP ,MCursorColor},
    {DI_BUTTON      ,0  ,13 ,0  ,0  ,DIF_CENTERGROUP ,MScrollbarColor},
    {DI_BUTTON      ,0  ,14 ,0  ,0  ,DIF_CENTERGROUP ,MScrollbarPositionMarkerColor},
    {DI_BUTTON      ,0  ,15 ,0  ,0  ,DIF_CENTERGROUP ,MBookmarksColor},
    {DI_BUTTON      ,0  ,16 ,0  ,0  ,DIF_CENTERGROUP ,MStackBookmarksColor},

    {DI_TEXT        ,-1 ,17 ,0  ,0  ,DIF_SEPARATOR   ,-1},

    {DI_BUTTON      ,0  ,18 ,0  ,0  ,DIF_CENTERGROUP ,MOk},
    {DI_BUTTON      ,0  ,18 ,0  ,0  ,DIF_CENTERGROUP ,MCancel},
  };
  struct FarDialogItem DialogItems[sizeofa(PreDialogItems)];

  InitDialogItems(PreDialogItems,DialogItems,sizeofa(PreDialogItems));

  DialogItems[DLG_OK].DefaultButton = 1;

  struct Options TmpOpt;
  memcpy(&TmpOpt,Opt,sizeof(TmpOpt));

  int ExitCode=-1;

  while (1)
  {
#ifndef UNICODE
    ExitCode = Info.Dialog(Info.ModuleNumber,-1,-1,44,21,NULL,DialogItems,sizeofa(DialogItems));
#else
    HANDLE hDlg = Info.DialogInit(Info.ModuleNumber,-1,-1,44,21,NULL,DialogItems,sizeofa(DialogItems),0,0,NULL,NULL);
    if (hDlg == INVALID_HANDLE_VALUE)
      break;

    ExitCode = Info.DialogRun(hDlg);
#endif

    if (ExitCode==-1 || ExitCode==DLG_OK || ExitCode==DLG_CANCEL)
    {
#ifdef UNICODE
      Info.DialogFree(hDlg);
#endif
      break;
    }

    switch (ExitCode)
    {
      case DLG_RBCOLOR:
        SetColor(&TmpOpt.ColorOfRightBorder);
        break;
      case DLG_EOLNORMALCOLOR:
        SetColor(&TmpOpt.ColorOfEOLNormal);
        break;
      case DLG_EOLCRCOLOR:
        SetColor(&TmpOpt.ColorOfEOLCR);
        break;
      case DLG_EOLLFCOLOR:
        SetColor(&TmpOpt.ColorOfEOLLF);
        break;
      case DLG_EOLNULLCOLOR:
        SetColor(&TmpOpt.ColorOfEOLNULL);
        break;
      case DLG_EOFCOLOR:
        SetColor(&TmpOpt.ColorOfEOF);
        break;
      case DLG_TABCOLOR:
        SetColor(&TmpOpt.ColorOfTabs);
        break;
      case DLG_TAB2COLOR:
        SetColor(&TmpOpt.ColorOfTabs2);
        break;
      case DLG_CROSSVERTICALCOLOR:
        SetColor(&TmpOpt.ColorOfCrossVertical);
        break;
      case DLG_CROSSHORIZONTALCOLOR:
        SetColor(&TmpOpt.ColorOfCrossHorizontal);
        break;
      case DLG_CURSORCOLOR:
        SetColor(&TmpOpt.ColorOfCursor);
        break;
      case DLG_SCROLLBARCOLOR:
        SetColor(&TmpOpt.ColorOfScrollbar);
        break;
      case DLG_SCROLLBARMARKERCOLOR:
        SetColor(&TmpOpt.ColorOfScrollbarPositionMarker);
        break;
      case DLG_BOOKMARKSCOLOR:
        SetColor(&TmpOpt.ColorOfBookmarks);
        break;
      case DLG_STACKBOOKMARKSCOLOR:
        SetColor(&TmpOpt.ColorOfStackBookmarks);
        break;
    }

#ifdef UNICODE
    Info.DialogFree(hDlg);
#endif
  }

  if (ExitCode == DLG_OK)
  {
    memcpy(Opt,&TmpOpt,sizeof(TmpOpt));
  }
}

void InitList(struct FarList *List, int Lng, int Sel)
{
  memset(List->Items,0,sizeof(struct FarListItem)*List->ItemsNumber);
  for (int i=0; i<List->ItemsNumber; i++)
  {
    if (Sel == i)
      List->Items[i].Flags = LIF_SELECTED;
#ifndef UNICODE
    FSF.sprintf(List->Items[i].Text,"%s",GetMsg(Lng+i));
#else
    List->Items[i].Text = GetMsg(Lng+i);
#endif
  }
}

HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item)
{
  enum ENUMDlg
  {
    DLG_TITLE,

    DLG_ONOFFSWITCH,
    DLG_OTHERCOLORINGPLUGINS,

    DLG_SEP1,

    DLG_RBCHK,
    DLG_RBCMB,

    DLG_EOLCHK,
    DLG_EOLCMB,

    DLG_EOFCHK,
    //DLG_EOFCMB,

    DLG_TABCHK,
    DLG_TABCMB,
    DLG_TABSHOWSYMBOLCHK,
    DLG_TABSHOWSYMBOLNUM,

    DLG_CROSSCHK,
    DLG_CROSSCMB,
    DLG_CROSSONTOPCHK,

    DLG_CURSORCHK,
    //DLG_CURSORCMB,

    DLG_LINENUMBERSCHK,
    //DLG_LINENUMBERSCMB,

    DLG_SCROLLBARCHK,
    DLG_SCROLLBARCMB,

    DLG_BOOKMARKSCHK,
    DLG_STACKBOOKMARKSCHK,

    DLG_SEP2,

    DLG_OK,
    DLG_CANCEL,
    DLG_SETCOLOR,
  };

  static const struct InitDialogItem PreDialogItems[] =
  {
    {DI_DOUBLEBOX   ,3  ,1  ,62 ,24 ,0          ,MTitle},

    {DI_CHECKBOX    ,5  ,2  ,0  ,0  ,0          ,MOnOffSwitch},
    {DI_CHECKBOX    ,5  ,3  ,0  ,0  ,0          ,MOtherColoringPlugins},

    {DI_TEXT        ,-1 ,4  ,0  ,0  ,DIF_SEPARATOR   ,-1},

    {DI_CHECKBOX    ,5  ,5  ,0  ,0  ,0          ,MRightBorder},
    {DI_COMBOBOX    ,8  ,6  ,60 ,0  ,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND, -1},

    {DI_CHECKBOX    ,5  ,7  ,0  ,0  ,0          ,MEOL},
    {DI_COMBOBOX    ,8  ,8  ,60 ,0  ,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND, -1},

    {DI_CHECKBOX    ,5  ,9  ,0  ,0  ,0          ,MEOF},
    //{DI_COMBOBOX    ,8  ,10 ,60 ,0  ,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND, -1},

    {DI_CHECKBOX    ,5  ,10 ,0  ,0  ,0          ,MTabs},
    {DI_COMBOBOX    ,8  ,11 ,60 ,0  ,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND, -1},
    {DI_CHECKBOX    ,8  ,12 ,0  ,0  ,0          ,MTabsShowSymbol},
    {DI_FIXEDIT     ,56 ,12 ,60 ,0  ,DIF_MASKEDIT, -1}, //          ,(int)_T("HHHH"),     DIF_MASKEDIT,   0},

    {DI_CHECKBOX    ,5  ,13 ,0  ,0  ,0          ,MCross},
    {DI_COMBOBOX    ,8  ,14 ,60 ,0  ,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND, -1},
    {DI_CHECKBOX    ,8  ,15 ,0  ,0  ,0          ,MCrossOnTop},

    {DI_CHECKBOX    ,5  ,16 ,0  ,0  ,0          ,MCursor},
    //{DI_COMBOBOX    ,8  ,17 ,60 ,0  ,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND, -1},

    {DI_CHECKBOX    ,5  ,17 ,0  ,0  ,0          ,MLineNumbers},
    //{DI_COMBOBOX    ,8  ,18 ,60 ,0  ,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND, -1},

    {DI_CHECKBOX    ,5  ,18 ,0  ,0  ,0          ,MScrollbar},
    {DI_COMBOBOX    ,8  ,19 ,60 ,0  ,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND, -1},

    {DI_CHECKBOX    ,5  ,20 ,0  ,0  ,0          ,MBookmarks},
    {DI_CHECKBOX    ,5  ,21 ,0  ,0  ,0          ,MStackBookmarks},

    {DI_TEXT        ,-1 ,22 ,0  ,0  ,DIF_SEPARATOR   ,-1},

    {DI_BUTTON      ,0  ,23 ,0  ,0  ,DIF_CENTERGROUP ,MOk},
    {DI_BUTTON      ,0  ,23 ,0  ,0  ,DIF_CENTERGROUP ,MCancel},
    {DI_BUTTON      ,0  ,23 ,0  ,0  ,DIF_CENTERGROUP ,MSetColor},
  };
  struct FarDialogItem DialogItems[sizeofa(PreDialogItems)];

  InitDialogItems(PreDialogItems,DialogItems,sizeofa(PreDialogItems));

  ReadRegistry();

  DialogItems[DLG_ONOFFSWITCH].Focus = 1;
  DialogItems[DLG_ONOFFSWITCH].Selected = Opt.OnOffSwitch;
  DialogItems[DLG_OTHERCOLORINGPLUGINS].Selected = Opt.OtherColoringPlugins;

  DialogItems[DLG_RBCHK].Selected          = Opt.ShowRightBorderOn;
  DialogItems[DLG_EOLCHK].Selected         = Opt.ShowEOLOn;
  DialogItems[DLG_EOFCHK].Selected         = Opt.ShowEOFOn;
  DialogItems[DLG_TABCHK].Selected         = Opt.ShowTabsOn;
  DialogItems[DLG_CROSSCHK].Selected       = Opt.ShowCrossOn;
  DialogItems[DLG_CURSORCHK].Selected      = Opt.ShowCursorOn;
  DialogItems[DLG_LINENUMBERSCHK].Selected = Opt.ShowLineNumbersOn;
  DialogItems[DLG_SCROLLBARCHK].Selected   = Opt.ShowScrollbarOn;

  DialogItems[DLG_CROSSONTOPCHK].Selected    = Opt.ShowCrossOnTop;
  DialogItems[DLG_TABSHOWSYMBOLCHK].Selected = Opt.ShowTabSymbol;
  DialogItems[DLG_TABSHOWSYMBOLNUM].Mask     = _T("xHHHH");
  TCHAR TabSymbolNum[5];
  FSF.sprintf(TabSymbolNum, _T("%04X"), (DWORD)(0xFFFF & Opt.TabSymbol));
  SetText(DLG_TABSHOWSYMBOLNUM, TabSymbolNum);

  DialogItems[DLG_BOOKMARKSCHK].Selected      = Opt.ShowBookmarks;
  DialogItems[DLG_STACKBOOKMARKSCHK].Selected = Opt.ShowStackBookmarks;

  struct FarListItem RBListItems[RB_MAX];
  struct FarList RBList = {sizeofa(RBListItems),RBListItems};

  struct FarListItem EOLListItems[EOL_MAX];
  struct FarList EOLList = {sizeofa(EOLListItems),EOLListItems};

  struct FarListItem TabListItems[TAB_MAX];
  struct FarList TabList = {sizeofa(TabListItems),TabListItems};

  struct FarListItem CrossListItems[CROSS_MAX];
  struct FarList CrossList = {sizeofa(CrossListItems),CrossListItems};

  struct FarListItem ScrollbarListItems[SCROLLBAR_MAX];
  struct FarList ScrollbarList = {sizeofa(ScrollbarListItems),ScrollbarListItems};

  DialogItems[DLG_OK].DefaultButton = 1;

  struct Options TmpOpt;
  memcpy(&TmpOpt,&Opt,sizeof(TmpOpt));

  int ExitCode;

  while (1)
  {
    InitList(&RBList,MRightBorderOn,TmpOpt.ShowRightBorder);
    DialogItems[DLG_RBCMB].ListItems=&RBList;

    InitList(&EOLList,MEOLOn,TmpOpt.ShowEOL);
    DialogItems[DLG_EOLCMB].ListItems=&EOLList;

    InitList(&TabList,MTabsOn,TmpOpt.ShowTabs);
    DialogItems[DLG_TABCMB].ListItems=&TabList;

    InitList(&CrossList,MCrossOn,TmpOpt.ShowCross);
    DialogItems[DLG_CROSSCMB].ListItems=&CrossList;

    InitList(&ScrollbarList,MScrollbarOn,TmpOpt.ShowScrollbar);
    DialogItems[DLG_SCROLLBARCMB].ListItems=&ScrollbarList;

#ifndef UNICODE
    ExitCode = Info.Dialog(Info.ModuleNumber,-1,-1,66,26,NULL,DialogItems,sizeofa(DialogItems));
#else
    HANDLE hDlg = Info.DialogInit(Info.ModuleNumber,-1,-1,66,26,NULL,DialogItems,sizeofa(DialogItems),0,0,NULL,NULL);
    if (hDlg == INVALID_HANDLE_VALUE)
      break;

    ExitCode = Info.DialogRun(hDlg);
#endif

    if (ExitCode==-1 || ExitCode==DLG_CANCEL)
    {
#ifdef UNICODE
      Info.DialogFree(hDlg);
#endif
      break;
    }

    if (ExitCode == DLG_SETCOLOR)
      ConfigColor(&TmpOpt);

    TmpOpt.ShowRightBorder=GetListPos(DLG_RBCMB);
    TmpOpt.ShowEOL=GetListPos(DLG_EOLCMB);
    TmpOpt.ShowTabs=GetListPos(DLG_TABCMB);
    TmpOpt.ShowCross=GetListPos(DLG_CROSSCMB);
    TmpOpt.ShowScrollbar=GetListPos(DLG_SCROLLBARCMB);

    if (ExitCode==DLG_OK)
    {
      memcpy(&Opt,&TmpOpt,sizeof(Opt));

      Opt.OnOffSwitch = GetCheck(DLG_ONOFFSWITCH);
      Opt.OtherColoringPlugins = GetCheck(DLG_OTHERCOLORINGPLUGINS);

      Opt.ShowRightBorderOn=GetCheck(DLG_RBCHK);

      Opt.ShowEOLOn=GetCheck(DLG_EOLCHK);

      Opt.ShowEOFOn=GetCheck(DLG_EOFCHK);

      Opt.ShowTabsOn=GetCheck(DLG_TABCHK);
      Opt.ShowTabSymbol=GetCheck(DLG_TABSHOWSYMBOLCHK);
      const TCHAR *pszHex = GetDataPtr(DLG_TABSHOWSYMBOLNUM);
      TCHAR *pszEnd;
      Opt.TabSymbol = _tcstol(pszHex+1,&pszEnd,16);
      if (Opt.TabSymbol < 32) Opt.TabSymbol = DEFTABSYMBOL;

      Opt.ShowCrossOn=GetCheck(DLG_CROSSCHK);
      Opt.ShowCrossOnTop=GetCheck(DLG_CROSSONTOPCHK);

      Opt.ShowCursorOn=GetCheck(DLG_CURSORCHK);

      Opt.ShowLineNumbersOn=GetCheck(DLG_LINENUMBERSCHK);

      Opt.ShowScrollbarOn=GetCheck(DLG_SCROLLBARCHK);

      Opt.ShowBookmarks=GetCheck(DLG_BOOKMARKSCHK);

      Opt.ShowStackBookmarks=GetCheck(DLG_STACKBOOKMARKSCHK);

#ifdef UNICODE
      Info.DialogFree(hDlg);
#endif

      WriteRegistry();

      Info.EditorControl(ECTL_REDRAW,NULL);

      break;
    }

#ifdef UNICODE
    Info.DialogFree(hDlg);
#endif
  }

  return INVALID_HANDLE_VALUE;
}


void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *pi)
{
  static const TCHAR *MenuStrings[1];

  pi->StructSize=sizeof(struct PluginInfo);
  pi->Flags=PF_EDITOR|PF_DISABLEPANELS;
  MenuStrings[0]=GetMsg(MTitle);
  pi->PluginMenuStrings=MenuStrings;
  pi->PluginMenuStringsNumber=1;
}

void VisualizeRightBorder(int ShowRightBorder, int RightBorder, int AutoWrap)
{
  static struct EditorConvertPos ecp = {-1, 0, 0};
  static struct EditorColor ec = {-1, 0, 0, 0, 0};
  if (RightBorder && (AutoWrap || ShowRightBorder!=RB_ONONLYIFAUTOWRAP))
  {
    ecp.SrcPos = RightBorder;
    Info.EditorControl(ECTL_TABTOREAL,(void *)&ecp);
    ec.StartPos = ec.EndPos = ecp.DestPos;
    ec.Color = Opt.ColorOfRightBorder;
    Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
  }
}

void VisualizeEndOfLine(int ShowEOL, int StringLength,const TCHAR *StringEOL, int CurLine, int TotalLines, int Line, int LeftPos)
{
  static struct EditorConvertPos ecp = {-1, 0, 0};
  static struct EditorColor ec = {-1, 0, 0, 0, 0};
  COORD c;
  DWORD w;
  ec.StartPos = ec.EndPos = StringLength;
  ecp.SrcPos = StringLength-1;
  if (ecp.SrcPos >= 0)
  {
    Info.EditorControl(ECTL_REALTOTAB,(void *)&ecp);
    ecp.DestPos++;
  }
  else
    ecp.DestPos = 0;
  c.Y = Line;
  c.X = ecp.DestPos - LeftPos;
  if (ShowEOL==EOL_ON || ShowEOL==EOL_MARKWITHSYMBOL)
  {
    ec.Color = Opt.ColorOfEOLNormal;
    Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
    if (ShowEOL==EOL_MARKWITHSYMBOL)
      WriteConsoleOutputCharacterW(GetStdHandle(STD_OUTPUT_HANDLE),L"\x2193",1,c,&w);
  }
  else if (*StringEOL)
  {
    if (*StringEOL==_T('\r'))
    {
      ec.Color = Opt.ColorOfEOLCR;
      Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
      if (ShowEOL==EOL_MARKALLWITHSYMBOLS)
        WriteConsoleOutputCharacterW(GetStdHandle(STD_OUTPUT_HANDLE),L"\x266A",1,c,&w);
    }
    else
    {
      ec.Color = Opt.ColorOfEOLLF;
      Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
      if (ShowEOL==EOL_MARKALLWITHSYMBOLS)
        WriteConsoleOutputCharacterW(GetStdHandle(STD_OUTPUT_HANDLE),L"\x25D9",1,c,&w);
    }
    if (StringEOL[1])
    {
      ec.StartPos++;
      ec.EndPos++;
      c.X++;
      if (StringEOL[1]==_T('\r'))
      {
        ec.Color = Opt.ColorOfEOLCR;
        Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
        if (ShowEOL==EOL_MARKALLWITHSYMBOLS)
          WriteConsoleOutputCharacterW(GetStdHandle(STD_OUTPUT_HANDLE),L"\x266A",1,c,&w);
      }
      else
      {
        ec.Color = Opt.ColorOfEOLLF;
        Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
        if (ShowEOL==EOL_MARKALLWITHSYMBOLS)
          WriteConsoleOutputCharacterW(GetStdHandle(STD_OUTPUT_HANDLE),L"\x25D9",1,c,&w);
      }
    }
  }
  else
  {
    ec.Color = Opt.ColorOfEOLNULL;
    Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
  }
}

void VisualizeEndOfFile(int StringLength, int CurLine, int TotalLines)
{
  static struct EditorColor ec = {-1, 0, 0, 0, 0};
  if (CurLine==TotalLines-1)
  {
    ec.StartPos = ec.EndPos = StringLength;
    ec.Color = Opt.ColorOfEOF;
    Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
  }
}

void VisualizeTabs(int ShowTabs, int ShowTabSymbol, wchar_t TabSymbol, const TCHAR *StringText, int StringLength, int Line, int LeftPos)
{
  static struct EditorConvertPos ecp = {-1, 0, 0};
  static struct EditorColor ec = {-1, 0, 0, 0, 0};
  static struct EditorColor ec2 = {-1, 0, 0, 0, 0};
  COORD c;
  DWORD w;
  ec.Color = Opt.ColorOfTabs;
  if (ShowTabs!=TAB_WHOLE && ShowTabs!=TAB_TWOCOLORS)
    ec.Color|=ECF_TAB1;
  ec2.Color = Opt.ColorOfTabs2|ECF_TAB1;
  for (int i=0; i<StringLength; i++)
  {
    if (StringText[i] == _T('\t'))
    {
      ec.StartPos = ec.EndPos = ec2.StartPos = ec2.EndPos = i;
      Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
      if (ShowTabs==TAB_TWOCOLORS)
        Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec2);
      if (ShowTabSymbol)
      {
        ecp.SrcPos = i;
        Info.EditorControl(ECTL_REALTOTAB,(void *)&ecp);
        c.X = ecp.DestPos - LeftPos;
        c.Y = Line;
        WriteConsoleOutputCharacterW(GetStdHandle(STD_OUTPUT_HANDLE),&TabSymbol,1,c,&w);
      }
    }
  }
}

void VisualizeCross(int ShowCross, int CurLine, int Line, int Column, int LeftPos, int WindowSizeX)
{
  static struct EditorConvertPos ecp = {-1, 0, 0};
  static struct EditorColor ec = {-1, 0, 0, 0, 0};
  if (ShowCross==CROSS_ON || ShowCross==CROSS_VERTICAL || ShowCross==CROSS_SMALL)
  {
    if (ShowCross!=CROSS_SMALL || (Line < CurLine+4 && Line > CurLine-4))
    {
      ecp.SrcPos = Column;
      Info.EditorControl(ECTL_TABTOREAL,(void *)&ecp);
      ec.StartPos = ec.EndPos = ecp.DestPos;
      ec.Color = Opt.ColorOfCrossVertical;
      Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
    }
  }
  if (CurLine == Line && (ShowCross==CROSS_ON || ShowCross==CROSS_HORIZONTAL || ShowCross==CROSS_SMALL))
  {
    ec.Color = Opt.ColorOfCrossHorizontal;
    if (ShowCross==CROSS_SMALL)
    {
      ecp.SrcPos = Column-4>=0 ? Column-4 : 0;
      Info.EditorControl(ECTL_TABTOREAL,(void *)&ecp);
      ec.StartPos = ecp.DestPos;

      ecp.SrcPos = Column+4;
      Info.EditorControl(ECTL_TABTOREAL,(void *)&ecp);
      ec.EndPos = ecp.DestPos;
    }
    else
    {
      ec.StartPos = 0;
      ec.EndPos = (LeftPos+WindowSizeX)*2;
    }
    Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
  }
}

void VisualizeCursor(int CurLine, int Line, int Column)
{
  static struct EditorConvertPos ecp = {-1, 0, 0};
  static struct EditorColor ec = {-1, 0, 0, 0, 0};
  if (CurLine==Line)
  {
    ecp.SrcPos = Column;
    Info.EditorControl(ECTL_TABTOREAL,(void *)&ecp);
    ec.StartPos = ec.EndPos = ecp.DestPos;
    ec.Color = Opt.ColorOfCursor|ECF_TAB1;
    Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
  }
}

void VisualizeScrollbar(int ShowScrollbar, int MarkerPosition, int MarkerPosition2, int MarkerSize, int Line, int LeftPos, int WindowSizeX)
{
  static struct EditorConvertPos ecp = {-1, 0, 0};
  static struct EditorColor ec = {-1, 0, 0, 0, 0};
  ecp.SrcPos = LeftPos+WindowSizeX-1;
  Info.EditorControl(ECTL_TABTOREAL,(void *)&ecp);
  ec.StartPos = ec.EndPos = ecp.DestPos;
  if (ShowScrollbar==SCROLLBAR_ON || ShowScrollbar==SCROLLBAR_VARIABLE || ShowScrollbar==SCROLLBAR_FILLBAR)
  {
    ec.Color = Opt.ColorOfScrollbar;
    Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
  }
  ec.Color = Opt.ColorOfScrollbarPositionMarker;
  if (ShowScrollbar==SCROLLBAR_FILLBAR || ShowScrollbar==SCROLLBAR_FILLBARNOBACK)
  {
    if (Line <= MarkerPosition)
      Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
  }
  else if (ShowScrollbar==SCROLLBAR_VARIABLE || ShowScrollbar==SCROLLBAR_VARIABLENOBACK)
  {
    if (Line >= MarkerPosition2 && Line < MarkerPosition2+MarkerSize)
      Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
  }
  else if (MarkerPosition == Line)
  {
    Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
  }
}

void VisualizeBookmarks(bool bStack)
{
  int iBookmarkCount=0;
  if (bStack)
  {
    iBookmarkCount=Info.EditorControl(ECTL_GETSTACKBOOKMARKS,0);
  }
  else
  {
    EditorInfo ei;
    Info.EditorControl(ECTL_GETINFO,&ei);
    iBookmarkCount=ei.BookMarkCount;
  }
  if (iBookmarkCount)
  {
    EditorBookMarks bm = {new long[iBookmarkCount],new long[iBookmarkCount],0,0};
    if (Info.EditorControl(bStack?ECTL_GETSTACKBOOKMARKS:ECTL_GETBOOKMARKS,&bm))
    {
      for (int i=0;i<iBookmarkCount;i++)
      {
        EditorColor ec={bm.Line[i],0,bm.Cursor[i],bm.Cursor[i],bStack?Opt.ColorOfStackBookmarks:Opt.ColorOfBookmarks};
        Info.EditorControl(ECTL_ADDCOLOR,&ec);
      }
    }
    delete[] bm.Line;
    delete[] bm.Cursor;
  }
}

int WINAPI EXP_NAME(ProcessEditorEvent)(int Event, void *Param)
{
  int RightBorder=0;
  int AutoWrap=0;
  int VisualizerOn=Opt.OnOffSwitch&&(Opt.ShowRightBorderOn||Opt.ShowEOLOn||Opt.ShowEOFOn||Opt.ShowTabsOn||Opt.ShowCrossOn||Opt.ShowCursorOn||Opt.ShowLineNumbersOn||Opt.ShowScrollbarOn);
  struct EditorInfo ei;
  static struct EditorSetPosition esp = {-1, -1, -1, -1, -1, -1};
  static struct EditorGetString egs = {-1, NULL, NULL, 0, 0, 0};
  static struct EditorColor ec = {-1, 0, -1, 100000, 0};
  int MarkerPosition;
  int MarkerPosition2;
  int MarkerSize;

  if (Event==EE_REDRAW && VisualizerOn)
  {
    if (/*!Opt.OtherColoringPlugins && */Param!=EEREDRAW_ALL)
    {
      Info.EditorControl(ECTL_REDRAW,NULL);
      return 0;
    }

    Info.EditorControl(ECTL_GETINFO,(void *)&ei);

    if (ei.TopScreenLine+ei.WindowSizeY-1<=ei.TotalLines)
      MarkerPosition = (int)((float)ei.CurLine/(float)ei.TotalLines*(float)ei.WindowSizeY)+ei.TopScreenLine;
    else
      MarkerPosition = (int)((float)ei.CurLine/(float)ei.TotalLines*(float)(ei.TotalLines-ei.TopScreenLine+1))+ei.TopScreenLine;

    if (ei.WindowSizeY>=ei.TotalLines)
    {
      MarkerSize = ei.TotalLines;
      MarkerPosition2 = 0;
    }
    else
    {
      MarkerSize = (int)((float)ei.WindowSizeY/(float)ei.TotalLines*(float)ei.WindowSizeY);
      if (MarkerSize > ei.WindowSizeY)
        MarkerSize = ei.WindowSizeY;
      if (MarkerSize >= ei.WindowSizeY-1 && ei.TotalLines > ei.WindowSizeY)
        MarkerSize=ei.WindowSizeY-2;
      if (MarkerSize<=0)
        MarkerSize=1;

      MarkerPosition2 = MarkerPosition-MarkerSize/2;
      if (MarkerPosition2<0)
        MarkerPosition2 = 0;
      if (MarkerPosition2 < ei.TopScreenLine)
        MarkerPosition2 = ei.TopScreenLine;
      if (MarkerPosition2 == ei.TopScreenLine && ei.TopScreenLine != 0)
        MarkerPosition2 = ei.TopScreenLine+1;
      if (MarkerPosition2+MarkerSize>=ei.TopScreenLine+ei.WindowSizeY)
      {
        MarkerPosition2 = ei.TopScreenLine+ei.WindowSizeY - MarkerSize;
        if (ei.TopScreenLine+ei.WindowSizeY<ei.TotalLines)
          MarkerPosition2--;
      }
    }

    if (GetEditorSettings)
    {
      GetEditorSettings(ei.EditorID,_T("wrap"),(void *)&RightBorder);
      GetEditorSettings(ei.EditorID,_T("autowrap"),(void *)&AutoWrap);

      if (RightBorder<=0 || RightBorder>512)
        RightBorder = 0;
    }

    /*
    if (Param == EEREDRAW_LINE)
    {
      Info.EditorControl(ECTL_GETSTRING,(void *)&egs);

      if (Opt.ShowCrossOn && !Opt.ShowCrossOnTop)
        VisualizeCross(Opt.ShowCross,ei.CurLine,ei.CurLine,ei.CurTabPos,ei.LeftPos,ei.WindowSizeX);

      if (Opt.ShowRightBorderOn)
        VisualizeRightBorder(Opt.ShowRightBorder,RightBorder,AutoWrap);

      if (Opt.ShowEOLOn)
        VisualizeEndOfLine(Opt.ShowEOL,egs.StringLength,egs.StringEOL,ei.CurLine,ei.TotalLines);

      if (Opt.ShowEOFOn)
        VisualizeEndOfFile(egs.StringLength,ei.CurLine,ei.TotalLines);

      if (Opt.ShowTabsOn)
        VisualizeTabs(Opt.ShowTabs,egs.StringText,egs.StringLength);

      if (Opt.ShowCrossOn && Opt.ShowCrossOnTop)
        VisualizeCross(Opt.ShowCross,ei.CurLine,ei.CurLine,ei.CurTabPos,ei.LeftPos,ei.WindowSizeX);

      if (Opt.ShowScrollbarOn)
        VisualizeScrollbar(MarkerPosition,ei.CurLine,ei.LeftPos,ei.WindowSizeX);

      if (Opt.ShowCursorOn)
        VisualizeCursor(ei.CurLine,ei.CurLine,ei.CurTabPos);
    }
    else
    */
    {
      int limit = min(ei.TopScreenLine+ei.WindowSizeY,ei.TotalLines);
      for (int i=ei.TopScreenLine; i<limit; i++)
      {
        //move to next line (this is needed to speed up the editor)
        esp.CurLine = i;
        Info.EditorControl(ECTL_SETPOSITION,(void *)&esp);

        //clean up if needed
        if (!Opt.OtherColoringPlugins)
          Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);

        Info.EditorControl(ECTL_GETSTRING,(void *)&egs);

        if (Opt.ShowCrossOn && !Opt.ShowCrossOnTop)
          VisualizeCross(Opt.ShowCross,i,ei.CurLine,ei.CurTabPos,ei.LeftPos,ei.WindowSizeX);

        if (Opt.ShowRightBorderOn)
          VisualizeRightBorder(Opt.ShowRightBorder,RightBorder,AutoWrap);

        if (Opt.ShowEOLOn)
          VisualizeEndOfLine(Opt.ShowEOL,egs.StringLength,egs.StringEOL,i,ei.TotalLines,i-ei.TopScreenLine+1,ei.LeftPos);

        if (Opt.ShowEOFOn)
          VisualizeEndOfFile(egs.StringLength,i,ei.TotalLines);

        if (Opt.ShowTabsOn)
          VisualizeTabs(Opt.ShowTabs,Opt.ShowTabSymbol,(wchar_t)Opt.TabSymbol,egs.StringText,egs.StringLength,i-ei.TopScreenLine+1,ei.LeftPos);

        if (Opt.ShowCrossOn && Opt.ShowCrossOnTop)
          VisualizeCross(Opt.ShowCross,i,ei.CurLine,ei.CurTabPos,ei.LeftPos,ei.WindowSizeX);

        if (Opt.ShowScrollbarOn)
          VisualizeScrollbar(Opt.ShowScrollbar,MarkerPosition,MarkerPosition2,MarkerSize,i,ei.LeftPos,ei.WindowSizeX);

        if (Opt.ShowCursorOn)
          VisualizeCursor(i,ei.CurLine,ei.CurTabPos);

        if (Opt.ShowLineNumbersOn)
        {
          COORD c;
          DWORD w;
          TCHAR tmp[50];
          FSF.sprintf(tmp,_T("%d"),ei.TotalLines);
          FSF.sprintf(tmp,_T(":%*.*d"),lstrlen(tmp),lstrlen(tmp),i+1);
          c.X = ei.WindowSizeX-lstrlen(tmp);
          c.Y = i-ei.TopScreenLine+1;
          WriteConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE),tmp,lstrlen(tmp),c,&w);
        }

        if (Opt.ShowBookmarks)
          VisualizeBookmarks(false);

        if (Opt.ShowStackBookmarks)
          VisualizeBookmarks(true);
      }

      //restore position
      esp.CurLine = ei.CurLine;
      esp.CurTabPos = ei.CurTabPos;
      Info.EditorControl(ECTL_SETPOSITION,(void *)&esp);
    }
  }
  return 0;
}
