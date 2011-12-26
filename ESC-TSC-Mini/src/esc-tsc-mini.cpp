/*
    ESC-TSC-Mini plugin for FAR Manager
    Copyright (C) 2001 Alex Yaroslavsky

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
#include <CRT/crt.hpp>
#include "plugin.hpp"
#include "esc-tsc-lang.hpp"

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
HMODULE hEsc = NULL;
BOOL FirstRun = TRUE;
int (WINAPI *SetEditorOption)(int EditorID, const wchar_t *szName, void *Param);
int (WINAPI *GetEditorSettings)(int EditorID, const wchar_t *szName, void *Param);

struct InitDialogItem
{
  FARDIALOGITEMTYPES Type;
  int X1, Y1, X2, Y2;
  const wchar_t *History;
  FARDIALOGITEMFLAGS Flags;
  int Data;
};

#define ControlKeyAllMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED|RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED)
#define IsNone(rec) (((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyAllMask)==0)

// {A718B580-E55F-44BB-908F-5143C8278361}
static const GUID MainGuid =
{ 0xa718b580, 0xe55f, 0x44bb, { 0x90, 0x8f, 0x51, 0x43, 0xc8, 0x27, 0x83, 0x61 } };

// {06D01BFF-698D-4B16-83DA-D41AABB5180B}
static const GUID MenuGuid =
{ 0x6d01bff, 0x698d, 0x4b16, { 0x83, 0xda, 0xd4, 0x1a, 0xab, 0xb5, 0x18, 0xb } };

// {6C75EEC0-A95D-445F-B2D9-6443AC2B8480}
static const GUID DialogGuid =
{ 0x6c75eec0, 0xa95d, 0x445f, { 0xb2, 0xd9, 0x64, 0x43, 0xac, 0x2b, 0x84, 0x80 } };

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
    PItem->UserData=0;
    PItem->History=PInit->History;
    PItem->Mask=NULL;
    if (PInit->Data>=0)
    {
      PItem->Data = GetMsg(PInit->Data);
    }
    else
    {
      PItem->Data = L"";
    }
  }
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=MAKEFARVERSION(1,3,0,0,VS_RELEASE);
  Info->Guid=MainGuid;
  Info->Title=L"ESC-TSC-Mini";
  Info->Description=L"ESC's Temporary Settings Changer - Minimalistic";
  Info->Author=L"Alex Yaroslavsky";
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *psi)
{
  Info=*psi;
  FSF=*psi->FSF;
  Info.FSF=&FSF;
}

INT_PTR WINAPI MyDialog(HANDLE hDlg,int Msg,int Param1,void *Param2)
{
  struct FarDialogItemData DlgData;
  static BOOL StatusBar;
  static struct EditorInfo ei;
  wchar_t temp[52];
  static const wchar_t *KnownCommands[] = {
    L"addsymbol",
    L"autowrap",
    L"charcodebase",
    L"justify",
    L"killemptylines",
    L"killspace",
    L"lines",
    L"p_end",
    L"p_quote",
    L"smarthome",
    L"smarttab",
    L"wrap"
  };

  switch (Msg)
  {
    case DN_INITDIALOG:
      Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
      Info.SendDlgMessage(hDlg,DM_SETFOCUS,1,0);
      Info.SendDlgMessage(hDlg,DM_SETMAXTEXTLENGTH,1,(void *)50);
      Info.SendDlgMessage(hDlg,DM_SETMAXTEXTLENGTH,2,(void *)50);
      for (int i=(int)ARRAYSIZE(KnownCommands)-1; i>=0; i--)
        Info.SendDlgMessage(hDlg,DM_ADDHISTORY,1,(void *)KnownCommands[i]);
      StatusBar = FALSE;
      return TRUE;

    case DN_CONTROLINPUT:
    {
      if (StatusBar)
      {
        DlgData.PtrLength=28;
        DlgData.PtrData=(wchar_t *)GetMsg(MWaiting);
        Info.SendDlgMessage(hDlg,DM_SETTEXT,8,&DlgData);
        StatusBar = FALSE;
      }
      const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
      if (record->EventType==MOUSE_EVENT)
      {
        return FALSE;
      }
      else if (record->EventType==KEY_EVENT&&record->Event.KeyEvent.bKeyDown)
      {
        if (Param1 == 3 || Param1 == 4 || !(record->Event.KeyEvent.wVirtualKeyCode == VK_RETURN && IsNone(record)))
          return FALSE;
        Param1 = 3;
      }
      else
      {
        break;
      }
    }

    case DN_BTNCLICK:
      switch (Param1)
      {
        case 3:
        {
          Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,2,temp);
          int i = 0; //no etry == zero
          FSF.sscanf(temp,L"%d",&i);
          FSF.sprintf(temp,L"%d",i);
          Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,2,temp);
          Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,1,temp);
          if (SetEditorOption(ei.EditorID,temp,&i))
          {
            Info.SendDlgMessage(hDlg,DM_ADDHISTORY,1,temp);
            FSF.sprintf(temp,L"%d",i);
            Info.SendDlgMessage(hDlg,DM_ADDHISTORY,2,temp);
            DlgData.PtrLength=28;
            DlgData.PtrData=(wchar_t *)GetMsg(MUpdateSuccessfull);
            Info.SendDlgMessage(hDlg,DM_SETTEXT,8,&DlgData);
            StatusBar = TRUE;
          }
          else
          {
            DlgData.PtrLength=28;
            DlgData.PtrData=(wchar_t *)GetMsg(MUpdateFailed);
            Info.SendDlgMessage(hDlg,DM_SETTEXT,8,&DlgData);
            StatusBar = TRUE;
          }
          break;
        }
        case 4:
        {
          int i;
          Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,1,temp);
          if (GetEditorSettings(ei.EditorID,temp,&i))
          {
            Info.SendDlgMessage(hDlg,DM_ADDHISTORY,1,temp);
            FSF.sprintf(temp,L"%d",i);
            Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,2,temp);
            Info.SendDlgMessage(hDlg,DM_SHOWITEM,2,(void *)1);
            DlgData.PtrLength=28;
            DlgData.PtrData=(wchar_t *)GetMsg(MCheckSuccessfull);
            Info.SendDlgMessage(hDlg,DM_SETTEXT,8,&DlgData);
            StatusBar = TRUE;
          }
          else
          {
            DlgData.PtrLength=28;
            DlgData.PtrData=(wchar_t *)GetMsg(MCheckFailed);
            Info.SendDlgMessage(hDlg,DM_SETTEXT,8,&DlgData);
            StatusBar = TRUE;
          }
          break;
        }
      }
      Info.SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,1,(void *)1);
      Info.SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,2,(void *)1);
      Info.SendDlgMessage(hDlg,DM_SETFOCUS,1,0);
      return TRUE;

    case DN_HOTKEY:
      switch (Param1)
      {
        case 5:
          Info.SendDlgMessage(hDlg,DM_SETFOCUS,1,0);
          break;
        case 6:
          Info.SendDlgMessage(hDlg,DM_SETFOCUS,2,0);
          break;
        default:
          return TRUE;
      }
      return FALSE;
  }

  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
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

HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{

  static const wchar_t ParamsHistory[] = L"ESC-TSC-Params";
  static const wchar_t SettingsHistory[] = L"ESC-TSC-Values";
  static const struct InitDialogItem PreDialogItems[] =
  {
    {DI_DOUBLEBOX  ,3  ,1  ,32 ,9  ,0                      ,0                                                             , MTitle},
    {DI_EDIT       ,15 ,2  ,30 ,0  ,ParamsHistory          ,DIF_HISTORY|DIF_USELASTHISTORY|DIF_MANUALADDHISTORY|DIF_FOCUS , -1},
    {DI_EDIT       ,15 ,4  ,30 ,0  ,SettingsHistory        ,DIF_HISTORY|DIF_USELASTHISTORY|DIF_MANUALADDHISTORY           , -1},
    {DI_BUTTON     ,8  ,6  ,0  ,0  ,0                      ,DIF_BTNNOCLOSE|DIF_CENTERGROUP|DIF_DEFAULTBUTTON              , MUpdate},
    {DI_BUTTON     ,8  ,6  ,0  ,0  ,0                      ,DIF_BTNNOCLOSE|DIF_CENTERGROUP                                , MCheck},
    {DI_TEXT       ,4  ,2  ,0  ,0  ,0                      ,0                                                             , MParam},
    {DI_TEXT       ,4  ,4  ,0  ,0  ,0                      ,0                                                             , MValue},
    {DI_TEXT       ,4  ,7  ,0  ,0  ,0                      ,0                                                             , MStatus},
    {DI_TEXT       ,4  ,8  ,0  ,0  ,0                      ,0                                                             , MWaiting},
  };
  struct FarDialogItem DialogItems[ARRAYSIZE(PreDialogItems)];

  if (FirstRun)
  {
    if (!hEsc)
    {
      hEsc=GetModuleHandle(L"esc.dll");
    }
    if (!GetEditorSettings && hEsc)
    {
      GetEditorSettings=(int (WINAPI*)(int, const wchar_t*, void*))GetProcAddress(hEsc,"GetEditorSettingsW");
    }
    if (!SetEditorOption && hEsc)
    {
      SetEditorOption=(int (WINAPI*)(int, const wchar_t*, void*))GetProcAddress(hEsc,"SetEditorOptionW");
    }
    if (!SetEditorOption || !GetEditorSettings)
    {
      const wchar_t *WMsg[2];
      WMsg[0] = GetMsg(MTitle);
      WMsg[1] = GetMsg(MNoESC);
      Info.Message(&MainGuid,NULL,FMSG_WARNING|FMSG_MB_OK,NULL,WMsg,2,0);
      return INVALID_HANDLE_VALUE;
    }
    FirstRun = FALSE;
  }

  InitDialogItems(PreDialogItems,DialogItems,ARRAYSIZE(PreDialogItems));

  HANDLE hDlg = Info.DialogInit(&MainGuid,&DialogGuid,-1,-1,36,11,NULL,DialogItems,ARRAYSIZE(PreDialogItems),0,0,MyDialog,0);
  if (hDlg != INVALID_HANDLE_VALUE)
  {
    Info.DialogRun(hDlg);
    Info.DialogFree(hDlg);
  }

  return INVALID_HANDLE_VALUE;
}
