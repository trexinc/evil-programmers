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
char PluginRootKey[80];
HMODULE hEsc;
int (WINAPI *GetEditorSettings)(int EditorID, const char *szName, void *Param);

struct Options
{
  int ShowRightBorder;
  int ShowEOL;
  int ShowTabs;
  int ShowCross;
  int OtherColoringPlugins;
  int ColorOfRightBorder;
  int ColorOfEOLNormal;
  int ColorOfEOLCR;
  int ColorOfEOLLF;
  int ColorOfEOLNULL;
  int ColorOfEOLEOF;
  int ColorOfTabs;
  int ColorOfCrossVertical;
  int ColorOfCrossHorizontal;
  //int HotkeyOfCross;
} Opt;

enum ENUMRightBorder
{
  RB_OFF,
  RB_ON,
  RB_ONONLYIFAUTOWRAP,

  RB_LAST,
  RB_MAX = RB_LAST-1,
};

enum ENUMEOL
{
  EOL_OFF,
  EOL_ON,
  EOL_MARKALL,
  EOL_MARKEOF,

  EOL_LAST,
  EOL_MAX = EOL_LAST-1,
};

enum ENUMTabs
{
  TAB_OFF,
  TAB_ON,
  TAB_WHOLE,

  TAB_LAST,
  TAB_MAX = TAB_LAST-1,
};

enum ENUMCross
{
  CROSS_OFF,
  CROSS_ON,
  CROSS_VERTICAL,
  CROSS_HORIZONTAL,
  //CROSS_HOTKEY,

  CROSS_LAST,
  CROSS_MAX = CROSS_LAST-1,
};

void *memcpy(void * dst, const void * src, size_t count)
{
  void * ret = dst;

  while (count--)
  {
    *(char *)dst = *(char *)src;
    dst = (char *)dst + 1;
    src = (char *)src + 1;
  }
  return(ret);
}

const char *GetMsg(int MsgId)
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
    if (PInit->Data>=0)
      FSF.sprintf(PItem->Data,"%s",GetMsg(PInit->Data));
    else
      *(PItem->Data)=0;
  }
}

void ReadRegistry()
{
  GetRegKey("ShowRightBorder",&Opt.ShowRightBorder,1);
  Opt.ShowRightBorder%=RB_LAST;
  GetRegKey("ShowEOL",&Opt.ShowEOL,1);
  Opt.ShowEOL%=EOL_LAST;
  GetRegKey("ShowTabs",&Opt.ShowTabs,1);
  Opt.ShowTabs%=TAB_LAST;
  GetRegKey("ShowCross",&Opt.ShowCross,1);
  Opt.ShowCross%=CROSS_LAST;
  GetRegKey("OtherColoringPlugins",&Opt.OtherColoringPlugins,1);
  GetRegKey("ColorOfRightBorder",&Opt.ColorOfRightBorder,0|5<<4);
  GetRegKey("ColorOfEOLNormal",&Opt.ColorOfEOLNormal,0|8<<4);
  GetRegKey("ColorOfEOLCR",&Opt.ColorOfEOLCR,0|8<<4);
  GetRegKey("ColorOfEOLLF",&Opt.ColorOfEOLLF,0|7<<4);
  GetRegKey("ColorOfEOLNULL",&Opt.ColorOfEOLNULL,0|12<<4);
  GetRegKey("ColorOfEOLEOF",&Opt.ColorOfEOLEOF,0|12<<4);
  GetRegKey("ColorOfTabs",&Opt.ColorOfTabs,0|15<<4);
  GetRegKey("ColorOfCrossVertical",&Opt.ColorOfCrossVertical,0|7<<4);
  GetRegKey("ColorOfCrossHorizontal",&Opt.ColorOfCrossHorizontal,0|7<<4);
  //GetRegKey("HotkeyOfCross",&Opt.HotkeyOfCross,);
}

void WriteRegistry()
{
  SetRegKey("ShowRightBorder",Opt.ShowRightBorder);
  SetRegKey("ShowEOL",Opt.ShowEOL);
  SetRegKey("ShowTabs",Opt.ShowTabs);
  SetRegKey("ShowCross",Opt.ShowCross);
  SetRegKey("OtherColoringPlugins",Opt.OtherColoringPlugins);
  SetRegKey("ColorOfRightBorder",Opt.ColorOfRightBorder);
  SetRegKey("ColorOfEOLNormal",Opt.ColorOfEOLNormal);
  SetRegKey("ColorOfEOLCR",Opt.ColorOfEOLCR);
  SetRegKey("ColorOfEOLLF",Opt.ColorOfEOLLF);
  SetRegKey("ColorOfEOLNULL",Opt.ColorOfEOLNULL);
  SetRegKey("ColorOfEOLEOF",Opt.ColorOfEOLEOF);
  SetRegKey("ColorOfTabs",Opt.ColorOfTabs);
  SetRegKey("ColorOfCrossVertical",Opt.ColorOfCrossVertical);
  SetRegKey("ColorOfCrossHorizontal",Opt.ColorOfCrossHorizontal);
}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *psi)
{
  Info=*psi;
  FSF=*psi->FSF;
  Info.FSF=&FSF;
  FSF.sprintf(PluginRootKey,"%s\\Visualizer",Info.RootKey);
  ReadRegistry();
  GetEditorSettings=NULL;
  hEsc=GetModuleHandle("esc.dll");
  if (hEsc)
    GetEditorSettings=(int (WINAPI*)(int, const char*, void*))GetProcAddress(hEsc,"GetEditorSettings");
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
    DLG_EOLEOFCOLOR,
    DLG_TABCOLOR,
    DLG_CROSSVERTICALCOLOR,
    DLG_CROSSHORIZONTALCOLOR,
    DLG_SEP1,

    DLG_OK,
    DLG_CANCEL,
  };

  static const struct InitDialogItem PreDialogItems[] =
  {
    DI_DOUBLEBOX   ,3  ,1  ,40 ,13 ,0               ,MColorTitle,

    DI_BUTTON      ,0  ,2  ,0  ,0  ,DIF_CENTERGROUP ,MRightBorderColor,
    DI_BUTTON      ,0  ,3  ,0  ,0  ,DIF_CENTERGROUP ,MEOLNormalColor,
    DI_BUTTON      ,0  ,4  ,0  ,0  ,DIF_CENTERGROUP ,MEOLCRColor,
    DI_BUTTON      ,0  ,5  ,0  ,0  ,DIF_CENTERGROUP ,MEOLLFColor,
    DI_BUTTON      ,0  ,6  ,0  ,0  ,DIF_CENTERGROUP ,MEOLNULLColor,
    DI_BUTTON      ,0  ,7  ,0  ,0  ,DIF_CENTERGROUP ,MEOLEOFColor,
    DI_BUTTON      ,0  ,8  ,0  ,0  ,DIF_CENTERGROUP ,MTabsColor,
    DI_BUTTON      ,0  ,9  ,0  ,0  ,DIF_CENTERGROUP ,MCrossVerticalColor,
    DI_BUTTON      ,0  ,10 ,0  ,0  ,DIF_CENTERGROUP ,MCrossHorizontalColor,
    DI_TEXT        ,-1 ,11 ,0  ,0  ,DIF_SEPARATOR   ,-1,

    DI_BUTTON      ,0  ,12 ,0  ,0  ,DIF_CENTERGROUP ,MOk,
    DI_BUTTON      ,0  ,12 ,0  ,0  ,DIF_CENTERGROUP ,MCancel,
  };
  struct FarDialogItem DialogItems[sizeofa(PreDialogItems)];

  InitDialogItems(PreDialogItems,DialogItems,sizeofa(PreDialogItems));

  DialogItems[DLG_OK].DefaultButton = 1;

  struct Options TmpOpt;
  memcpy(&TmpOpt,Opt,sizeof(TmpOpt));

  int ExitCode;

  while (1)
  {
    ExitCode = Info.Dialog(Info.ModuleNumber,-1,-1,44,15,NULL,DialogItems,sizeofa(DialogItems));

    if (ExitCode==-1 || ExitCode==DLG_OK || ExitCode==DLG_CANCEL)
      break;

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
      case DLG_EOLEOFCOLOR:
        SetColor(&TmpOpt.ColorOfEOLEOF);
        break;
      case DLG_TABCOLOR:
        SetColor(&TmpOpt.ColorOfTabs);
        break;
      case DLG_CROSSVERTICALCOLOR:
        SetColor(&TmpOpt.ColorOfCrossVertical);
        break;
      case DLG_CROSSHORIZONTALCOLOR:
        SetColor(&TmpOpt.ColorOfCrossHorizontal);
    }
  }

  if (ExitCode == DLG_OK)
  {
    memcpy(Opt,&TmpOpt,sizeof(TmpOpt));
  }
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  enum ENUMDlg
  {
    DLG_TITLE,

    DLG_RBOFF,
    DLG_RBON,
    DLG_RBONONLYIFAUTOWRAP,
    DLG_SEP1,

    DLG_EOLOFF,
    DLG_EOLON,
    DLG_EOLMARKALL,
    DLG_EOLMARKEOF,
    DLG_SEP2,

    DLG_TABOFF,
    DLG_TABON,
    DLG_TABWHOLE,
    DLG_SEP3,

    DLG_CROSSOFF,
    DLG_CROSSON,
    DLG_CROSSVERTICAL,
    DLG_CROSSHORIZONTAL,
    DLG_SEP4,

    DLG_OTHERCOLORINGPLUGINS,
    DLG_SEP5,

    DLG_OK,
    DLG_CANCEL,
    DLG_SETCOLOR,
  };

  static const struct InitDialogItem PreDialogItems[] =
  {
    DI_DOUBLEBOX   ,3  ,1  ,66 ,23 ,0         ,MTitle,

    DI_RADIOBUTTON ,5  ,2  ,0  ,0  ,DIF_GROUP ,MRightBorderOff,
    DI_RADIOBUTTON ,5  ,3  ,0  ,0  ,0         ,MRightBorderOn,
    DI_RADIOBUTTON ,5  ,4  ,0  ,0  ,0         ,MRightBorderOnOnlyIfAutoWrap,
    DI_TEXT        ,-1 ,5  ,0  ,0  ,DIF_SEPARATOR   ,-1,

    DI_RADIOBUTTON ,5  ,6  ,0  ,0  ,DIF_GROUP ,MEOLOff,
    DI_RADIOBUTTON ,5  ,7  ,0  ,0  ,0         ,MEOLOn,
    DI_RADIOBUTTON ,5  ,8  ,0  ,0  ,0         ,MEOLMarkAll,
    DI_RADIOBUTTON ,5  ,9  ,0  ,0  ,0         ,MEOLMarkEOF,
    DI_TEXT        ,-1 ,10 ,0  ,0  ,DIF_SEPARATOR   ,-1,

    DI_RADIOBUTTON ,5  ,11 ,0  ,0  ,DIF_GROUP ,MTabsOff,
    DI_RADIOBUTTON ,5  ,12 ,0  ,0  ,0         ,MTabsOn,
    DI_RADIOBUTTON ,5  ,13 ,0  ,0  ,0         ,MTabsWhole,
    DI_TEXT        ,-1 ,14 ,0  ,0  ,DIF_SEPARATOR   ,-1,

    DI_RADIOBUTTON ,5  ,15 ,0  ,0  ,DIF_GROUP ,MCrossOff,
    DI_RADIOBUTTON ,5  ,16 ,0  ,0  ,0         ,MCrossOn,
    DI_RADIOBUTTON ,5  ,17 ,0  ,0  ,0         ,MCrossVertical,
    DI_RADIOBUTTON ,5  ,18 ,0  ,0  ,0         ,MCrossHorizontal,
    DI_TEXT        ,-1 ,19 ,0  ,0  ,DIF_SEPARATOR   ,-1,

    DI_CHECKBOX    ,5  ,20 ,0  ,0  ,0         ,MOtherColoringPlugins,
    DI_TEXT        ,-1 ,21 ,0  ,0  ,DIF_SEPARATOR   ,-1,

    DI_BUTTON      ,0  ,22 ,0  ,0  ,DIF_CENTERGROUP ,MOk,
    DI_BUTTON      ,0  ,22 ,0  ,0  ,DIF_CENTERGROUP ,MCancel,
    DI_BUTTON      ,0  ,22 ,0  ,0  ,DIF_CENTERGROUP ,MSetColor,
  };
  struct FarDialogItem DialogItems[sizeofa(PreDialogItems)];

  InitDialogItems(PreDialogItems,DialogItems,sizeofa(PreDialogItems));

  ReadRegistry();

  DialogItems[DLG_RBOFF+Opt.ShowRightBorder].Focus = 1;
  DialogItems[DLG_RBOFF+Opt.ShowRightBorder].Selected = 1;
  DialogItems[DLG_EOLOFF+Opt.ShowEOL].Selected = 1;
  DialogItems[DLG_TABOFF+Opt.ShowTabs].Selected = 1;
  DialogItems[DLG_CROSSOFF+Opt.ShowCross].Selected = 1;

  DialogItems[DLG_OTHERCOLORINGPLUGINS].Selected = Opt.OtherColoringPlugins;

  DialogItems[DLG_OK].DefaultButton = 1;

  struct Options TmpOpt;
  memcpy(&TmpOpt,&Opt,sizeof(TmpOpt));

  int ExitCode;

  while (1)
  {
    ExitCode = Info.Dialog(Info.ModuleNumber,-1,-1,70,25,NULL,DialogItems,sizeofa(DialogItems));

    if (ExitCode==-1 || ExitCode==DLG_OK || ExitCode==DLG_CANCEL)
      break;

    if (ExitCode == DLG_SETCOLOR)
      ConfigColor(&TmpOpt);
  }

  if (ExitCode == DLG_OK)
  {
    memcpy(&Opt,&TmpOpt,sizeof(TmpOpt));

    for (int i=DLG_RBOFF; i<=DLG_RBOFF+RB_MAX; i++)
    {
      if (DialogItems[i].Selected)
        Opt.ShowRightBorder=i-DLG_RBOFF;
    }

    for (int i=DLG_EOLOFF; i<=DLG_EOLOFF+EOL_MAX; i++)
    {
      if (DialogItems[i].Selected)
        Opt.ShowEOL=i-DLG_EOLOFF;
    }

    for (int i=DLG_TABOFF; i<=DLG_TABOFF+TAB_MAX; i++)
    {
      if (DialogItems[i].Selected)
        Opt.ShowTabs=i-DLG_TABOFF;
    }

    for (int i=DLG_CROSSOFF; i<=DLG_CROSSOFF+CROSS_MAX; i++)
    {
      if (DialogItems[i].Selected)
        Opt.ShowCross=i-DLG_CROSSOFF;
    }

    Opt.OtherColoringPlugins = DialogItems[DLG_OTHERCOLORINGPLUGINS].Selected;

    WriteRegistry();

    Info.EditorControl(ECTL_REDRAW,NULL);
  }

  return INVALID_HANDLE_VALUE;
}


void WINAPI _export GetPluginInfo(struct PluginInfo *pi)
{
  static const char *MenuStrings[1];

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
  if (ShowRightBorder && RightBorder && (AutoWrap || ShowRightBorder!=RB_ONONLYIFAUTOWRAP))
  {
    ecp.SrcPos = RightBorder;
    Info.EditorControl(ECTL_TABTOREAL,(void *)&ecp);
    ec.StartPos = ec.EndPos = ecp.DestPos;
    ec.Color = Opt.ColorOfRightBorder;
    Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
  }
}

void VisualizeEndOfLine(int ShowEOL, int StringLength,const char *StringEOL, int CurLine, int TotalLines)
{
  static struct EditorColor ec = {-1, 0, 0, 0, 0};
  if (ShowEOL)
  {
    ec.StartPos = ec.EndPos = StringLength;
    if (ShowEOL==EOL_ON || (ShowEOL==EOL_MARKEOF && CurLine!=TotalLines-1))
    {
      ec.Color = Opt.ColorOfEOLNormal;
      Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
    }
    else if (*StringEOL)
    {
      if (*StringEOL=='\r')
        ec.Color = Opt.ColorOfEOLCR;
      else
        ec.Color = Opt.ColorOfEOLLF;
      Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
      if (StringEOL[1])
      {
        ec.StartPos++;
        ec.EndPos++;
        if (StringEOL[1]=='\r')
          ec.Color = Opt.ColorOfEOLCR;
        else
          ec.Color = Opt.ColorOfEOLLF;
        Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
      }
    }
    else
    {
      ec.Color = (CurLine!=TotalLines-1)?Opt.ColorOfEOLNULL:Opt.ColorOfEOLEOF;
      Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
    }
  }
}

void VisualizeTabs(int ShowTabs, const char *StringText, int StringLength)
{
  static struct EditorColor ec = {-1, 0, 0, 0, 0};
  if (ShowTabs)
  {
    ec.Color = Opt.ColorOfTabs|(ShowTabs==TAB_WHOLE?0:ECF_TAB1);
    for (int i=0; i<StringLength; i++)
    {
      if (StringText[i] == 9)
      {
        ec.StartPos = ec.EndPos = i;
        Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
      }
    }
  }
}

void VisualizeCross(int ShowCross, int CurLine, int Line, int Column, int LeftPos, int WindowSizeX)
{
  static struct EditorConvertPos ecp = {-1, 0, 0};
  static struct EditorColor ec = {-1, 0, 0, 0, 0};
  if (ShowCross)
  {
    if (ShowCross==CROSS_ON || ShowCross==CROSS_VERTICAL)
    {
      ecp.SrcPos = Column;
      Info.EditorControl(ECTL_TABTOREAL,(void *)&ecp);
      ec.StartPos = ec.EndPos = ecp.DestPos;
      ec.Color = Opt.ColorOfCrossVertical;
      Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
    }
    if (CurLine == Line && (ShowCross==CROSS_ON || ShowCross==CROSS_HORIZONTAL))
    {
      ec.Color = Opt.ColorOfCrossHorizontal;
      ec.StartPos = 0;
      ec.EndPos = (LeftPos+WindowSizeX)*2;
      Info.EditorControl(ECTL_ADDCOLOR,(void *)&ec);
    }
  }
}

int WINAPI _export ProcessEditorEvent(int Event, void *Param)
{
  int RightBorder=0;
  int AutoWrap=0;
  int VisualizerOn=Opt.ShowRightBorder|Opt.ShowEOL|Opt.ShowTabs|Opt.ShowCross;
  struct EditorInfo ei;
  static struct EditorSetPosition esp = {-1, -1, -1, -1, -1, -1};
  static struct EditorGetString egs = {-1, NULL, NULL, 0, 0, 0};
  static struct EditorColor ec = {-1, 0, -1, 100000, 0};


  if (Event==EE_REDRAW && VisualizerOn)
  {
    if (!Opt.OtherColoringPlugins && Param!=EEREDRAW_ALL)
    {
      Info.EditorControl(ECTL_REDRAW,NULL);
      return 0;
    }

    Info.EditorControl(ECTL_GETINFO,(void *)&ei);

    if (GetEditorSettings)
    {
      GetEditorSettings(ei.EditorID,"wrap",(void *)&RightBorder);
      GetEditorSettings(ei.EditorID,"autowrap",(void *)&AutoWrap);

      if (RightBorder<=0 || RightBorder>512)
        RightBorder = 0;
    }

    if (Param == EEREDRAW_LINE)
    {
      VisualizeRightBorder(Opt.ShowRightBorder,RightBorder,AutoWrap);

      Info.EditorControl(ECTL_GETSTRING,(void *)&egs);

      VisualizeEndOfLine(Opt.ShowEOL,egs.StringLength,egs.StringEOL,ei.CurLine,ei.TotalLines);

      VisualizeTabs(Opt.ShowTabs,egs.StringText,egs.StringLength);

      VisualizeCross(Opt.ShowCross,ei.CurLine,ei.CurLine,ei.CurTabPos,ei.LeftPos,ei.WindowSizeX);
    }
    else
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

        VisualizeRightBorder(Opt.ShowRightBorder,RightBorder,AutoWrap);

        Info.EditorControl(ECTL_GETSTRING,(void *)&egs);

        VisualizeEndOfLine(Opt.ShowEOL,egs.StringLength,egs.StringEOL,i,ei.TotalLines);

        VisualizeTabs(Opt.ShowTabs,egs.StringText,egs.StringLength);

        VisualizeCross(Opt.ShowCross,i,ei.CurLine,ei.CurTabPos,ei.LeftPos,ei.WindowSizeX);
      }

      //restore position
      esp.CurLine = ei.CurLine;
      esp.CurTabPos = ei.CurTabPos;
      Info.EditorControl(ECTL_SETPOSITION,(void *)&esp);
    }
  }
  return 0;
}
