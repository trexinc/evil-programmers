/*
    Visualizer plugin for Far Manager
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
#include <PluginSettings.hpp>
#include "version.hpp"

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

// {8719879F-BB66-4FBE-ACC2-F877C2E581F1}
static const GUID MainGuid =
{ 0x8719879f, 0xbb66, 0x4fbe, { 0xac, 0xc2, 0xf8, 0x77, 0xc2, 0xe5, 0x81, 0xf1 } };

// {2DA3F5E0-055E-4B35-8FBD-419E050B85DE}
static const GUID MenuGuid =
{ 0x2da3f5e0, 0x55e, 0x4b35, { 0x8f, 0xbd, 0x41, 0x9e, 0x5, 0xb, 0x85, 0xde } };

// {04FE7452-0FFC-4258-8E61-097030798AF3}
static const GUID ConfigDialogGuid =
{ 0x4fe7452, 0xffc, 0x4258, { 0x8e, 0x61, 0x9, 0x70, 0x30, 0x79, 0x8a, 0xf3 } };

// {8212CF0C-6C2B-4AD9-89B7-3B3818577CBC}
static const GUID ConfigCologDialogGuid =
{ 0x8212cf0c, 0x6c2b, 0x4ad9, { 0x89, 0xb7, 0x3b, 0x38, 0x18, 0x57, 0x7c, 0xbc } };


struct PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
HMODULE hEsc;
int (WINAPI *GetEditorSettings)(int EditorID, const wchar_t *szName, void *Param);

struct Options
{
  int OnOffSwitch;
  int ShowRightBorderOn;
  int ShowEOLOn;
  int ShowEOFOn;
  int ShowTabsOn;
  int ShowCrossOn;
  int ShowCursorOn;
  int ShowLineNumbersOn;
  int ShowRightBorder;
  int ShowEOL;
  //int ShowEOF;
  int ShowTabs;
  int ShowCross;
  //int ShowCursor;
  //int ShowLineNumbers;
  int ShowCrossOnTop;
  int ShowTabSymbol;
  int TabSymbol;
  int ShowBookmarks;
  int ShowStackBookmarks;
  FarColor ColorOfRightBorder;
  FarColor ColorOfEOLNormal;
  FarColor ColorOfEOLCR;
  FarColor ColorOfEOLLF;
  FarColor ColorOfEOLNULL;
  FarColor ColorOfEOF;
  FarColor ColorOfTabs;
  FarColor ColorOfTabs2;
  FarColor ColorOfCrossVertical;
  FarColor ColorOfCrossHorizontal;
  FarColor ColorOfCursor;
  FarColor ColorOfBookmarks;
  FarColor ColorOfStackBookmarks;
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

const wchar_t *GetMsg(int MsgId)
{
  return Info.GetMsg(&MainGuid,MsgId);
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
    PItem->Reserved=0;
    PItem->Flags=PInit->Flags;
    PItem->MaxLength=0;
    if (PInit->Data>=0)
    {
      PItem->Data = GetMsg(PInit->Data);
    }
    else
    {
      PItem->Data = L"";
    }
    PItem->History=NULL;
    PItem->Mask=NULL;
    PItem->UserData=0;
  }
}

void ConvertColor(FarColor *Color, int Bg)
{
	Color->Flags = FCF_FG_4BIT|FCF_BG_4BIT;
	Color->ForegroundColor = 0;
	Color->BackgroundColor = Bg;
	Color->Reserved = 0;
}

void ReadSettings()
{
  PluginSettings settings(MainGuid, Info.SettingsControl);

  Opt.OnOffSwitch = settings.Get(0,L"OnOffSwitch",1);

  Opt.ShowRightBorderOn = settings.Get(0,L"ShowRightBorderOn",1);
  Opt.ShowEOLOn = settings.Get(0,L"ShowEOLOn",1);
  Opt.ShowEOFOn = settings.Get(0,L"ShowEOFOn",1);
  Opt.ShowTabsOn = settings.Get(0,L"ShowTabsOn",1);
  Opt.ShowCrossOn = settings.Get(0,L"ShowCrossOn",1);
  Opt.ShowCursorOn = settings.Get(0,L"ShowCursorOn",1);
  Opt.ShowLineNumbersOn = settings.Get(0,L"ShowLineNumbersOn",1);

  Opt.ShowRightBorder = settings.Get(0,L"ShowRightBorder",RB_ON);
  Opt.ShowRightBorder%=RB_MAX;
  Opt.ShowEOL = settings.Get(0,L"ShowEOL",EOL_ON);
  Opt.ShowEOL%=EOL_MAX;
  //Opt.ShowEOF = settings.Get(0,L"ShowEOF",EOF_ON);
  //Opt.ShowEOF%=EOF_MAX;
  Opt.ShowTabs = settings.Get(0,L"ShowTabs",TAB_ON);
  Opt.ShowTabs%=TAB_MAX;
  Opt.ShowCross = settings.Get(0,L"ShowCross",CROSS_ON);
  Opt.ShowCross%=CROSS_MAX;
  //Opt.ShowCursor = settings.Get(0,L"ShowCursor",CURSOR_ON);
  //Opt.ShowCursor%=CURSOR_MAX;
  //Opt.ShowLineNumbers = settings.Get(0,L"ShowLineNumbers",LINENUMBERS_ON);
  //Opt.ShowLineNumbers%=LINENUMBERS_MAX;

  Opt.ShowCrossOnTop = settings.Get(0,L"ShowCrossOnTop",0);
  Opt.ShowTabSymbol = settings.Get(0,L"ShowTabSymbol",0);
  Opt.TabSymbol = settings.Get(0,L"TabSymbol",DEFTABSYMBOL);
  Opt.ShowBookmarks = settings.Get(0,L"ShowBookmarks",1);
  Opt.ShowStackBookmarks = settings.Get(0,L"ShowStackBookmarks",1);

  if (settings.Get(0,L"ColorOfRightBorder",&Opt.ColorOfRightBorder,sizeof(Opt.ColorOfRightBorder)) != sizeof(Opt.ColorOfRightBorder))
  {
    ConvertColor(&Opt.ColorOfRightBorder,5);
  }
  if (settings.Get(0,L"ColorOfEOLNormal",&Opt.ColorOfEOLNormal,sizeof(Opt.ColorOfEOLNormal)) != sizeof(Opt.ColorOfEOLNormal))
  {
    ConvertColor(&Opt.ColorOfEOLNormal,8);
  }
  if (settings.Get(0,L"ColorOfEOLCR",&Opt.ColorOfEOLCR,sizeof(Opt.ColorOfEOLCR)) != sizeof(Opt.ColorOfEOLCR))
  {
    ConvertColor(&Opt.ColorOfEOLCR,8);
  }
  if (settings.Get(0,L"ColorOfEOLLF",&Opt.ColorOfEOLLF,sizeof(Opt.ColorOfEOLLF)) != sizeof(Opt.ColorOfEOLLF))
  {
    ConvertColor(&Opt.ColorOfEOLLF,7);
  }
  if (settings.Get(0,L"ColorOfEOLNULL",&Opt.ColorOfEOLNULL,sizeof(Opt.ColorOfEOLNULL)) != sizeof(Opt.ColorOfEOLNULL))
  {
    ConvertColor(&Opt.ColorOfEOLNULL,12);
  }
  if (settings.Get(0,L"ColorOfEOF",&Opt.ColorOfEOF,sizeof(Opt.ColorOfEOF)) != sizeof(Opt.ColorOfEOF))
  {
    ConvertColor(&Opt.ColorOfEOF,12);
  }
  if (settings.Get(0,L"ColorOfTabs",&Opt.ColorOfTabs,sizeof(Opt.ColorOfTabs)) != sizeof(Opt.ColorOfTabs))
  {
    ConvertColor(&Opt.ColorOfTabs,15);
  }
  if (settings.Get(0,L"ColorOfTabs2",&Opt.ColorOfTabs2,sizeof(Opt.ColorOfTabs2)) != sizeof(Opt.ColorOfTabs2))
  {
    ConvertColor(&Opt.ColorOfTabs2,7);
  }
  if (settings.Get(0,L"ColorOfCrossVertical",&Opt.ColorOfCrossVertical,sizeof(Opt.ColorOfCrossVertical)) != sizeof(Opt.ColorOfCrossVertical))
  {
    ConvertColor(&Opt.ColorOfCrossVertical,7);
  }
  if (settings.Get(0,L"ColorOfCrossHorizontal",&Opt.ColorOfCrossHorizontal,sizeof(Opt.ColorOfCrossHorizontal)) != sizeof(Opt.ColorOfCrossHorizontal))
  {
    ConvertColor(&Opt.ColorOfCrossHorizontal,7);
  }
  if (settings.Get(0,L"ColorOfCursor",&Opt.ColorOfCursor,sizeof(Opt.ColorOfCursor)) != sizeof(Opt.ColorOfCursor))
  {
    ConvertColor(&Opt.ColorOfCursor,12);
  }
  if (settings.Get(0,L"ColorOfBookmarks",&Opt.ColorOfBookmarks,sizeof(Opt.ColorOfBookmarks)) != sizeof(Opt.ColorOfBookmarks))
  {
    ConvertColor(&Opt.ColorOfBookmarks,2);
  }
  if (settings.Get(0,L"ColorOfStackBookmarks",&Opt.ColorOfStackBookmarks,sizeof(Opt.ColorOfStackBookmarks)) != sizeof(Opt.ColorOfStackBookmarks))
  {
    ConvertColor(&Opt.ColorOfStackBookmarks,6);
  }
  //Opt.HotkeyOfCross = settings.Get(0,L"HotkeyOfCross",);
}

void WriteSettings()
{
  PluginSettings settings(MainGuid, Info.SettingsControl);

  settings.Set(0,L"OnOffSwitch",Opt.OnOffSwitch);

  settings.Set(0,L"ShowRightBorderOn",Opt.ShowRightBorderOn);
  settings.Set(0,L"ShowEOLOn",Opt.ShowEOLOn);
  settings.Set(0,L"ShowEOFOn",Opt.ShowEOFOn);
  settings.Set(0,L"ShowTabsOn",Opt.ShowTabsOn);
  settings.Set(0,L"ShowCrossOn",Opt.ShowCrossOn);
  settings.Set(0,L"ShowCursorOn",Opt.ShowCursorOn);
  settings.Set(0,L"ShowLineNumbersOn",Opt.ShowLineNumbersOn);

  settings.Set(0,L"ShowRightBorder",Opt.ShowRightBorder);
  settings.Set(0,L"ShowEOL",Opt.ShowEOL);
  //settings.Set(0,L"ShowEOF",Opt.ShowEOF);
  settings.Set(0,L"ShowTabs",Opt.ShowTabs);
  settings.Set(0,L"ShowCross",Opt.ShowCross);
  //settings.Set(0,L"ShowCursor",Opt.ShowCursor);
  //settings.Set(0,L"ShowLineNumbers",Opt.ShowLineNumbers);

  settings.Set(0,L"ShowCrossOnTop",Opt.ShowCrossOnTop);
  settings.Set(0,L"ShowTabSymbol",Opt.ShowTabSymbol);
  settings.Set(0,L"TabSymbol",Opt.TabSymbol);
  settings.Set(0,L"ShowBookmarks",Opt.ShowBookmarks);
  settings.Set(0,L"ShowStackBookmarks",Opt.ShowStackBookmarks);

  settings.Set(0,L"ColorOfRightBorder",&Opt.ColorOfRightBorder,sizeof(Opt.ColorOfRightBorder));
  settings.Set(0,L"ColorOfEOLNormal",&Opt.ColorOfEOLNormal,sizeof(Opt.ColorOfEOLNormal));
  settings.Set(0,L"ColorOfEOLCR",&Opt.ColorOfEOLCR,sizeof(Opt.ColorOfEOLCR));
  settings.Set(0,L"ColorOfEOLLF",&Opt.ColorOfEOLLF,sizeof(Opt.ColorOfEOLLF));
  settings.Set(0,L"ColorOfEOLNULL",&Opt.ColorOfEOLNULL,sizeof(Opt.ColorOfEOLNULL));
  settings.Set(0,L"ColorOfEOF",&Opt.ColorOfEOF,sizeof(Opt.ColorOfEOF));
  settings.Set(0,L"ColorOfTabs",&Opt.ColorOfTabs,sizeof(Opt.ColorOfTabs));
  settings.Set(0,L"ColorOfTabs2",&Opt.ColorOfTabs2,sizeof(Opt.ColorOfTabs2));
  settings.Set(0,L"ColorOfCrossVertical",&Opt.ColorOfCrossVertical,sizeof(Opt.ColorOfCrossVertical));
  settings.Set(0,L"ColorOfCrossHorizontal",&Opt.ColorOfCrossHorizontal,sizeof(Opt.ColorOfCrossHorizontal));
  settings.Set(0,L"ColorOfCursor",&Opt.ColorOfCursor,sizeof(Opt.ColorOfCursor));
  settings.Set(0,L"ColorOfBookmarks",&Opt.ColorOfBookmarks,sizeof(Opt.ColorOfBookmarks));
  settings.Set(0,L"ColorOfStackBookmarks",&Opt.ColorOfStackBookmarks,sizeof(Opt.ColorOfStackBookmarks));
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=PLUGIN_VERSION;
  Info->Guid=MainGuid;
  Info->Title=PLUGIN_NAME;
  Info->Description=PLUGIN_DESC;
  Info->Author=PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *psi)
{
  Info=*psi;
  FSF=*psi->FSF;
  Info.FSF=&FSF;
  ReadSettings();
  GetEditorSettings=NULL;
  hEsc=GetModuleHandle(L"esc.dll");
  if (hEsc)
  {
    GetEditorSettings=(int (WINAPI*)(int, const wchar_t*, void*))GetProcAddress(hEsc,"GetEditorSettingsW");
  }
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
    DLG_BOOKMARKSCOLOR,
    DLG_STACKBOOKMARKSCOLOR,

    DLG_SEP1,

    DLG_OK,
    DLG_CANCEL,
  };

  static const struct InitDialogItem PreDialogItems[] =
  {
    {DI_DOUBLEBOX   ,3  ,1  ,40 ,17 ,0               ,MColorTitle},

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
    {DI_BUTTON      ,0  ,13 ,0  ,0  ,DIF_CENTERGROUP ,MBookmarksColor},
    {DI_BUTTON      ,0  ,14 ,0  ,0  ,DIF_CENTERGROUP ,MStackBookmarksColor},

    {DI_TEXT        ,-1 ,15 ,0  ,0  ,DIF_SEPARATOR   ,-1},

    {DI_BUTTON      ,0  ,16 ,0  ,0  ,DIF_CENTERGROUP ,MOk},
    {DI_BUTTON      ,0  ,16 ,0  ,0  ,DIF_CENTERGROUP ,MCancel},
  };
  struct FarDialogItem DialogItems[ARRAYSIZE(PreDialogItems)];

  InitDialogItems(PreDialogItems,DialogItems,ARRAYSIZE(PreDialogItems));

  DialogItems[DLG_OK].Flags |= DIF_DEFAULTBUTTON;

  struct Options TmpOpt;
  memcpy(&TmpOpt,Opt,sizeof(TmpOpt));

  int ExitCode=-1;

  while (1)
  {
    HANDLE hDlg = Info.DialogInit(&MainGuid,&ConfigCologDialogGuid,-1,-1,44,19,NULL,DialogItems,ARRAYSIZE(DialogItems),0,0,NULL,0);
    if (hDlg == INVALID_HANDLE_VALUE)
      break;

    ExitCode = Info.DialogRun(hDlg);

    if (ExitCode==-1 || ExitCode==DLG_OK || ExitCode==DLG_CANCEL)
    {
      Info.DialogFree(hDlg);
      break;
    }

    switch (ExitCode)
    {
      case DLG_RBCOLOR:
        Info.ColorDialog(&MainGuid,CDF_NONE,&TmpOpt.ColorOfRightBorder);
        break;
      case DLG_EOLNORMALCOLOR:
        Info.ColorDialog(&MainGuid,CDF_NONE,&TmpOpt.ColorOfEOLNormal);
        break;
      case DLG_EOLCRCOLOR:
        Info.ColorDialog(&MainGuid,CDF_NONE,&TmpOpt.ColorOfEOLCR);
        break;
      case DLG_EOLLFCOLOR:
        Info.ColorDialog(&MainGuid,CDF_NONE,&TmpOpt.ColorOfEOLLF);
        break;
      case DLG_EOLNULLCOLOR:
        Info.ColorDialog(&MainGuid,CDF_NONE,&TmpOpt.ColorOfEOLNULL);
        break;
      case DLG_EOFCOLOR:
        Info.ColorDialog(&MainGuid,CDF_NONE,&TmpOpt.ColorOfEOF);
        break;
      case DLG_TABCOLOR:
        Info.ColorDialog(&MainGuid,CDF_NONE,&TmpOpt.ColorOfTabs);
        break;
      case DLG_TAB2COLOR:
        Info.ColorDialog(&MainGuid,CDF_NONE,&TmpOpt.ColorOfTabs2);
        break;
      case DLG_CROSSVERTICALCOLOR:
        Info.ColorDialog(&MainGuid,CDF_NONE,&TmpOpt.ColorOfCrossVertical);
        break;
      case DLG_CROSSHORIZONTALCOLOR:
        Info.ColorDialog(&MainGuid,CDF_NONE,&TmpOpt.ColorOfCrossHorizontal);
        break;
      case DLG_CURSORCOLOR:
        Info.ColorDialog(&MainGuid,CDF_NONE,&TmpOpt.ColorOfCursor);
        break;
      case DLG_BOOKMARKSCOLOR:
        Info.ColorDialog(&MainGuid,CDF_NONE,&TmpOpt.ColorOfBookmarks);
        break;
      case DLG_STACKBOOKMARKSCOLOR:
        Info.ColorDialog(&MainGuid,CDF_NONE,&TmpOpt.ColorOfStackBookmarks);
        break;
    }

    Info.DialogFree(hDlg);
  }

  if (ExitCode == DLG_OK)
  {
    memcpy(Opt,&TmpOpt,sizeof(TmpOpt));
  }
}

void InitList(struct FarList *List, int Lng, size_t Sel)
{
  memset(List->Items,0,sizeof(struct FarListItem)*List->ItemsNumber);
  for (size_t i=0; i<List->ItemsNumber; i++)
  {
    if (Sel == i)
      List->Items[i].Flags = LIF_SELECTED;
    List->Items[i].Text = GetMsg(Lng+(int)i);
  }
}

HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
  enum ENUMDlg
  {
    DLG_TITLE,

    DLG_ONOFFSWITCH,

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

    DLG_BOOKMARKSCHK,
    DLG_STACKBOOKMARKSCHK,

    DLG_SEP2,

    DLG_OK,
    DLG_CANCEL,
    DLG_SETCOLOR,
  };

  static const struct InitDialogItem PreDialogItems[] =
  {
    {DI_DOUBLEBOX   ,3  ,1  ,62 ,21 ,0          ,MTitle},

    {DI_CHECKBOX    ,5  ,2  ,0  ,0  ,0          ,MOnOffSwitch},

    {DI_TEXT        ,-1 ,3  ,0  ,0  ,DIF_SEPARATOR   ,-1},

    {DI_CHECKBOX    ,5  ,4  ,0  ,0  ,0          ,MRightBorder},
    {DI_COMBOBOX    ,8  ,5  ,60 ,0  ,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND, -1},

    {DI_CHECKBOX    ,5  ,6  ,0  ,0  ,0          ,MEOL},
    {DI_COMBOBOX    ,8  ,7  ,60 ,0  ,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND, -1},

    {DI_CHECKBOX    ,5  ,8  ,0  ,0  ,0          ,MEOF},
    //{DI_COMBOBOX    ,8  ,9 ,60 ,0  ,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND, -1},

    {DI_CHECKBOX    ,5  ,9 ,0  ,0  ,0          ,MTabs},
    {DI_COMBOBOX    ,8  ,10 ,60 ,0  ,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND, -1},
    {DI_CHECKBOX    ,8  ,11 ,0  ,0  ,0          ,MTabsShowSymbol},
    {DI_FIXEDIT     ,56 ,11 ,60 ,0  ,DIF_MASKEDIT, -1}, //          ,(int)_T("HHHH"),     DIF_MASKEDIT,   0},

    {DI_CHECKBOX    ,5  ,12 ,0  ,0  ,0          ,MCross},
    {DI_COMBOBOX    ,8  ,13 ,60 ,0  ,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND, -1},
    {DI_CHECKBOX    ,8  ,14 ,0  ,0  ,0          ,MCrossOnTop},

    {DI_CHECKBOX    ,5  ,15 ,0  ,0  ,0          ,MCursor},
    //{DI_COMBOBOX    ,8  ,16 ,60 ,0  ,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND, -1},

    {DI_CHECKBOX    ,5  ,16 ,0  ,0  ,0          ,MLineNumbers},
    //{DI_COMBOBOX    ,8  ,17 ,60 ,0  ,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND, -1},

    {DI_CHECKBOX    ,5  ,17 ,0  ,0  ,0          ,MBookmarks},
    {DI_CHECKBOX    ,5  ,18 ,0  ,0  ,0          ,MStackBookmarks},

    {DI_TEXT        ,-1 ,19 ,0  ,0  ,DIF_SEPARATOR   ,-1},

    {DI_BUTTON      ,0  ,20 ,0  ,0  ,DIF_CENTERGROUP ,MOk},
    {DI_BUTTON      ,0  ,20 ,0  ,0  ,DIF_CENTERGROUP ,MCancel},
    {DI_BUTTON      ,0  ,20 ,0  ,0  ,DIF_CENTERGROUP ,MSetColor},
  };
  struct FarDialogItem DialogItems[ARRAYSIZE(PreDialogItems)];

  InitDialogItems(PreDialogItems,DialogItems,ARRAYSIZE(PreDialogItems));

  ReadSettings();

  DialogItems[DLG_ONOFFSWITCH].Flags |= DIF_FOCUS;
  DialogItems[DLG_ONOFFSWITCH].Selected = Opt.OnOffSwitch;

  DialogItems[DLG_RBCHK].Selected          = Opt.ShowRightBorderOn;
  DialogItems[DLG_EOLCHK].Selected         = Opt.ShowEOLOn;
  DialogItems[DLG_EOFCHK].Selected         = Opt.ShowEOFOn;
  DialogItems[DLG_TABCHK].Selected         = Opt.ShowTabsOn;
  DialogItems[DLG_CROSSCHK].Selected       = Opt.ShowCrossOn;
  DialogItems[DLG_CURSORCHK].Selected      = Opt.ShowCursorOn;
  DialogItems[DLG_LINENUMBERSCHK].Selected = Opt.ShowLineNumbersOn;

  DialogItems[DLG_CROSSONTOPCHK].Selected    = Opt.ShowCrossOnTop;
  DialogItems[DLG_TABSHOWSYMBOLCHK].Selected = Opt.ShowTabSymbol;
  DialogItems[DLG_TABSHOWSYMBOLNUM].Mask     = L"xHHHH";
  wchar_t TabSymbolNum[5];
  FSF.sprintf(TabSymbolNum, L"%04X", (DWORD)(0xFFFF & Opt.TabSymbol));
  DialogItems[DLG_TABSHOWSYMBOLNUM].Data = TabSymbolNum;

  DialogItems[DLG_BOOKMARKSCHK].Selected      = Opt.ShowBookmarks;
  DialogItems[DLG_STACKBOOKMARKSCHK].Selected = Opt.ShowStackBookmarks;

  struct FarListItem RBListItems[RB_MAX];
  struct FarList RBList = {ARRAYSIZE(RBListItems),RBListItems};

  struct FarListItem EOLListItems[EOL_MAX];
  struct FarList EOLList = {ARRAYSIZE(EOLListItems),EOLListItems};

  struct FarListItem TabListItems[TAB_MAX];
  struct FarList TabList = {ARRAYSIZE(TabListItems),TabListItems};

  struct FarListItem CrossListItems[CROSS_MAX];
  struct FarList CrossList = {ARRAYSIZE(CrossListItems),CrossListItems};

  DialogItems[DLG_OK].Flags |= DIF_DEFAULTBUTTON;

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

    HANDLE hDlg = Info.DialogInit(&MainGuid,&ConfigDialogGuid,-1,-1,66,23,NULL,DialogItems,ARRAYSIZE(DialogItems),0,0,NULL,0);
    if (hDlg == INVALID_HANDLE_VALUE)
      break;

    ExitCode = Info.DialogRun(hDlg);

    if (ExitCode==-1 || ExitCode==DLG_CANCEL)
    {
      Info.DialogFree(hDlg);
      break;
    }

    if (ExitCode == DLG_SETCOLOR)
      ConfigColor(&TmpOpt);

    TmpOpt.ShowRightBorder=GetListPos(DLG_RBCMB);
    TmpOpt.ShowEOL=GetListPos(DLG_EOLCMB);
    TmpOpt.ShowTabs=GetListPos(DLG_TABCMB);
    TmpOpt.ShowCross=GetListPos(DLG_CROSSCMB);

    if (ExitCode==DLG_OK)
    {
      memcpy(&Opt,&TmpOpt,sizeof(Opt));

      Opt.OnOffSwitch = GetCheck(DLG_ONOFFSWITCH);

      Opt.ShowRightBorderOn=GetCheck(DLG_RBCHK);

      Opt.ShowEOLOn=GetCheck(DLG_EOLCHK);

      Opt.ShowEOFOn=GetCheck(DLG_EOFCHK);

      Opt.ShowTabsOn=GetCheck(DLG_TABCHK);
      Opt.ShowTabSymbol=GetCheck(DLG_TABSHOWSYMBOLCHK);
      const wchar_t *pszHex = GetDataPtr(DLG_TABSHOWSYMBOLNUM);
      wchar_t *pszEnd;
      Opt.TabSymbol = _tcstol(pszHex+1,&pszEnd,16);
      if (Opt.TabSymbol < 32) Opt.TabSymbol = DEFTABSYMBOL;

      Opt.ShowCrossOn=GetCheck(DLG_CROSSCHK);
      Opt.ShowCrossOnTop=GetCheck(DLG_CROSSONTOPCHK);

      Opt.ShowCursorOn=GetCheck(DLG_CURSORCHK);

      Opt.ShowLineNumbersOn=GetCheck(DLG_LINENUMBERSCHK);

      Opt.ShowBookmarks=GetCheck(DLG_BOOKMARKSCHK);

      Opt.ShowStackBookmarks=GetCheck(DLG_STACKBOOKMARKSCHK);

      Info.DialogFree(hDlg);

      WriteSettings();

      Info.EditorControl(-1,ECTL_REDRAW,0,NULL);

      break;
    }

    Info.DialogFree(hDlg);
  }

  return INVALID_HANDLE_VALUE;
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
  static const wchar_t *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MTitle);
  Info->PluginMenu.Guids=&MenuGuid;
  Info->PluginMenu.Strings=PluginMenuStrings;
  Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
}

#define __DEF_EDITORCOLOR__ {sizeof(EditorColor), -1, 0, 0, 0, (unsigned)-1, 0, {0}, MainGuid}
void VisualizeRightBorder(int ShowRightBorder, int RightBorder, int AutoWrap)
{
  static struct EditorConvertPos ecp = {-1, 0, 0};
  static struct EditorColor ec = __DEF_EDITORCOLOR__;
  if (RightBorder && (AutoWrap || ShowRightBorder!=RB_ONONLYIFAUTOWRAP))
  {
    ecp.SrcPos = RightBorder;
    Info.EditorControl(-1,ECTL_TABTOREAL,0,&ecp);
    ec.StartPos = ec.EndPos = ecp.DestPos;
    ec.Color = Opt.ColorOfRightBorder;
    Info.EditorControl(-1,ECTL_ADDCOLOR,0,&ec);
  }
}

void VisualizeEndOfLine(int ShowEOL, int StringLength,const wchar_t *StringEOL, int CurLine, int TotalLines, int Line, int LeftPos)
{
  static struct EditorConvertPos ecp = {-1, 0, 0};
  static struct EditorColor ec = __DEF_EDITORCOLOR__;
  COORD c;
  DWORD w;
  ec.StartPos = ec.EndPos = StringLength;
  ecp.SrcPos = StringLength-1;
  if (ecp.SrcPos >= 0)
  {
    Info.EditorControl(-1,ECTL_REALTOTAB,0,&ecp);
    ecp.DestPos++;
  }
  else
    ecp.DestPos = 0;
  int nXShift = 0, nYShift = 0;
  SMALL_RECT rcFar = {0};
  if (Info.AdvControl(&MainGuid, ACTL_GETFARRECT, 0, &rcFar))
  {
    nXShift = rcFar.Left; nYShift = rcFar.Top;
  }
  c.Y = Line + nYShift;
  c.X = ecp.DestPos - LeftPos + nXShift;
  if (ShowEOL==EOL_ON || ShowEOL==EOL_MARKWITHSYMBOL)
  {
    ec.Color = Opt.ColorOfEOLNormal;
    Info.EditorControl(-1,ECTL_ADDCOLOR,0,&ec);
    if (ShowEOL==EOL_MARKWITHSYMBOL)
      WriteConsoleOutputCharacterW(GetStdHandle(STD_OUTPUT_HANDLE),L"\x2193",1,c,&w);
  }
  else if (*StringEOL)
  {
    if (*StringEOL==_T('\r'))
    {
      ec.Color = Opt.ColorOfEOLCR;
      Info.EditorControl(-1,ECTL_ADDCOLOR,0,&ec);
      if (ShowEOL==EOL_MARKALLWITHSYMBOLS)
        WriteConsoleOutputCharacterW(GetStdHandle(STD_OUTPUT_HANDLE),L"\x266A",1,c,&w);
    }
    else
    {
      ec.Color = Opt.ColorOfEOLLF;
      Info.EditorControl(-1,ECTL_ADDCOLOR,0,&ec);
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
        Info.EditorControl(-1,ECTL_ADDCOLOR,0,&ec);
        if (ShowEOL==EOL_MARKALLWITHSYMBOLS)
          WriteConsoleOutputCharacterW(GetStdHandle(STD_OUTPUT_HANDLE),L"\x266A",1,c,&w);
      }
      else
      {
        ec.Color = Opt.ColorOfEOLLF;
        Info.EditorControl(-1,ECTL_ADDCOLOR,0,&ec);
        if (ShowEOL==EOL_MARKALLWITHSYMBOLS)
          WriteConsoleOutputCharacterW(GetStdHandle(STD_OUTPUT_HANDLE),L"\x25D9",1,c,&w);
      }
    }
  }
  else
  {
    ec.Color = Opt.ColorOfEOLNULL;
    Info.EditorControl(-1,ECTL_ADDCOLOR,0,&ec);
  }
}

void VisualizeEndOfFile(int StringLength, int CurLine, int TotalLines)
{
  static struct EditorColor ec = __DEF_EDITORCOLOR__;
  if (CurLine==TotalLines-1)
  {
    ec.StartPos = ec.EndPos = StringLength;
    ec.Color = Opt.ColorOfEOF;
    Info.EditorControl(-1,ECTL_ADDCOLOR,0,&ec);
  }
}

void VisualizeTabs(int ShowTabs, int ShowTabSymbol, wchar_t TabSymbol, const wchar_t *StringText, int StringLength, int Line, int LeftPos)
{
  static struct EditorConvertPos ecp = {-1, 0, 0};
  static struct EditorColor ec = __DEF_EDITORCOLOR__;
  static struct EditorColor ec2 = __DEF_EDITORCOLOR__;
  COORD c;
  DWORD w;
  ec.Color = Opt.ColorOfTabs;
  if (ShowTabs!=TAB_WHOLE && ShowTabs!=TAB_TWOCOLORS)
    ec.Flags = ECF_TABMARKFIRST;
  else
    ec.Flags = 0;
  ec2.Color = Opt.ColorOfTabs2;
  ec2.Flags = ECF_TABMARKFIRST;
  int nXShift = 0, nYShift = 0;
  SMALL_RECT rcFar = {0};
  if (Info.AdvControl(&MainGuid, ACTL_GETFARRECT, 0, &rcFar))
  {
    nXShift = rcFar.Left; nYShift = rcFar.Top;
  }
  for (int i=0; i<StringLength; i++)
  {
    if (StringText[i] == L'\t')
    {
      ec.StartPos = ec.EndPos = ec2.StartPos = ec2.EndPos = i;
      Info.EditorControl(-1,ECTL_ADDCOLOR,0,&ec);
      if (ShowTabs==TAB_TWOCOLORS)
        Info.EditorControl(-1,ECTL_ADDCOLOR,0,&ec2);
      if (ShowTabSymbol)
      {
        ecp.SrcPos = i;
        Info.EditorControl(-1,ECTL_REALTOTAB,0,&ecp);
        c.X = ecp.DestPos - LeftPos + nXShift;
        c.Y = Line + nYShift;
        WriteConsoleOutputCharacterW(GetStdHandle(STD_OUTPUT_HANDLE),&TabSymbol,1,c,&w);
      }
    }
  }
}

void VisualizeCross(int ShowCross, int CurLine, int Line, int Column, int LeftPos, int WindowSizeX)
{
  static struct EditorConvertPos ecp = {-1, 0, 0};
  static struct EditorColor ec = __DEF_EDITORCOLOR__;
  if (ShowCross==CROSS_ON || ShowCross==CROSS_VERTICAL || ShowCross==CROSS_SMALL)
  {
    if (ShowCross!=CROSS_SMALL || (Line < CurLine+4 && Line > CurLine-4))
    {
      ecp.SrcPos = Column;
      Info.EditorControl(-1,ECTL_TABTOREAL,0,&ecp);
      ec.StartPos = ec.EndPos = ecp.DestPos;
      ec.Color = Opt.ColorOfCrossVertical;
      ec.Flags = ECF_TABMARKCURRENT;
      Info.EditorControl(-1,ECTL_ADDCOLOR,0,&ec);
    }
  }
  if (CurLine == Line && (ShowCross==CROSS_ON || ShowCross==CROSS_HORIZONTAL || ShowCross==CROSS_SMALL))
  {
    ec.Color = Opt.ColorOfCrossHorizontal;
    if (ShowCross==CROSS_SMALL)
    {
      ecp.SrcPos = Column-4>=0 ? Column-4 : 0;
      Info.EditorControl(-1,ECTL_TABTOREAL,0,&ecp);
      ec.StartPos = ecp.DestPos;

      ecp.SrcPos = Column+4;
      Info.EditorControl(-1,ECTL_TABTOREAL,0,&ecp);
      ec.EndPos = ecp.DestPos;
    }
    else
    {
      ec.StartPos = 0;
      ec.EndPos = (LeftPos+WindowSizeX)*2;
    }
    Info.EditorControl(-1,ECTL_ADDCOLOR,0,&ec);
  }
}

void VisualizeCursor(int CurLine, int Line, int Column)
{
  static struct EditorConvertPos ecp = {-1, 0, 0};
  static struct EditorColor ec = __DEF_EDITORCOLOR__;
  if (CurLine==Line)
  {
    ecp.SrcPos = Column;
    Info.EditorControl(-1,ECTL_TABTOREAL,0,&ecp);
    ec.StartPos = ec.EndPos = ecp.DestPos;
    ec.Color = Opt.ColorOfCursor;
    ec.Flags = ECF_TABMARKCURRENT;
    Info.EditorControl(-1,ECTL_ADDCOLOR,0,&ec);
  }
}

void VisualizeBookmarks(bool bStack)
{
  int iBookmarkCount=0;
  if (bStack)
  {
    iBookmarkCount=(int)Info.EditorControl(-1,ECTL_GETSTACKBOOKMARKS,0,0);
  }
  else
  {
    EditorInfo ei;
    Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
    iBookmarkCount=ei.BookMarkCount;
  }
  if (iBookmarkCount)
  {
    EditorBookMarks bm = {new int[iBookmarkCount],new int[iBookmarkCount],0,0,{0}};
    if (Info.EditorControl(-1,bStack?ECTL_GETSTACKBOOKMARKS:ECTL_GETBOOKMARKS,0,&bm))
    {
      for (int i=0;i<iBookmarkCount;i++)
      {
        static EditorColor ec = __DEF_EDITORCOLOR__;
        if (bm.Line[i] != -1)
        {
          ec.StringNumber = bm.Line[i];
          ec.StartPos = ec.EndPos = bm.Cursor[i];
          ec.Color = bStack?Opt.ColorOfStackBookmarks:Opt.ColorOfBookmarks;
          Info.EditorControl(-1,ECTL_ADDCOLOR,0,&ec);
        }
      }
    }
    delete[] bm.Line;
    delete[] bm.Cursor;
  }
}

int WINAPI ProcessEditorEventW(const struct ProcessEditorEventInfo *EInfo)
{
  int RightBorder=0;
  int AutoWrap=0;
  int VisualizerOn=Opt.OnOffSwitch&&(Opt.ShowRightBorderOn||Opt.ShowEOLOn||Opt.ShowEOFOn||Opt.ShowTabsOn||Opt.ShowCrossOn||Opt.ShowCursorOn||Opt.ShowLineNumbersOn);
  struct EditorInfo ei;
  static struct EditorSetPosition esp = {-1, -1, -1, -1, -1, -1};
  static struct EditorGetString egs = {-1, 0, NULL, NULL, 0, 0};
  static struct EditorDeleteColor edc = {sizeof(EditorDeleteColor), MainGuid, -1, -1};

  if (EInfo->Event==EE_REDRAW && VisualizerOn)
  {
    if (EInfo->Param!=EEREDRAW_ALL)
    {
      Info.EditorControl(-1,ECTL_REDRAW,0,NULL);
      return 0;
    }

    Info.EditorControl(-1,ECTL_GETINFO,0,&ei);

    if (GetEditorSettings)
    {
      GetEditorSettings(ei.EditorID,L"wrap",(void *)&RightBorder);
      GetEditorSettings(ei.EditorID,L"autowrap",(void *)&AutoWrap);

      if (RightBorder<=0 || RightBorder>512)
        RightBorder = 0;
    }

    /*
    if (EInfo->Param == EEREDRAW_LINE)
    {
      Info.EditorControl(-1,ECTL_GETSTRING,0,&egs);

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

      if (Opt.ShowCursorOn)
        VisualizeCursor(ei.CurLine,ei.CurLine,ei.CurTabPos);
    }
    else
    */
    {
      int nXShift = 0, nYShift = 0;
      SMALL_RECT rcFar = {0};
      if (Info.AdvControl(&MainGuid, ACTL_GETFARRECT, 0, &rcFar))
      {
        nXShift = rcFar.Left; nYShift = rcFar.Top;
      }
      int limit = Min(ei.TopScreenLine+ei.WindowSizeY,ei.TotalLines);
      for (int i=ei.TopScreenLine; i<limit; i++)
      {
        //move to next line (this is needed to speed up the editor)
        esp.CurLine = i;
        Info.EditorControl(-1,ECTL_SETPOSITION,0,&esp);

        //clean up
        Info.EditorControl(-1,ECTL_DELCOLOR,0,&edc);

        Info.EditorControl(-1,ECTL_GETSTRING,0,&egs);

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

        if (Opt.ShowBookmarks)
          VisualizeBookmarks(false);

        if (Opt.ShowStackBookmarks)
          VisualizeBookmarks(true);

        if (Opt.ShowCrossOn && Opt.ShowCrossOnTop)
          VisualizeCross(Opt.ShowCross,i,ei.CurLine,ei.CurTabPos,ei.LeftPos,ei.WindowSizeX);

        if (Opt.ShowCursorOn)
          VisualizeCursor(i,ei.CurLine,ei.CurTabPos);

        if (Opt.ShowLineNumbersOn)
        {
          COORD c;
          DWORD w;
          wchar_t tmp[50];
          FSF.sprintf(tmp,L"%d",ei.TotalLines);
          FSF.sprintf(tmp,L":%*.*d",lstrlen(tmp),lstrlen(tmp),i+1);
          c.X = ei.WindowSizeX-lstrlen(tmp)+nXShift;
          c.Y = i-ei.TopScreenLine+1+nYShift;
          WriteConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE),tmp,lstrlen(tmp),c,&w);
        }
      }

      //restore position
      esp.CurLine = ei.CurLine;
      esp.CurTabPos = ei.CurTabPos;
      Info.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
    }
  }
  return 0;
}

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
void __cxa_pure_virtual(void);
#ifdef __cplusplus
};
#endif

void __cxa_pure_virtual(void)
{
}
#endif
