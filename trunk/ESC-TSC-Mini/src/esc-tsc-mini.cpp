/*
    ESC-TSC-Mini plugin for FAR Manager
    Copyright (C) 2001-2004 Alex Yaroslavsky

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
#include "plugin.hpp"
#include "farkeys.hpp"
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
int (WINAPI *SetEditorOption)(int EditorID, const char *szName, void *Param);
int (WINAPI *GetEditorSettings)(int EditorID, const char *szName, void *Param);

typedef struct InitDialogItem
{
  int Type;
  int X1, Y1, X2, Y2;
  int Focus;
  int Selected;
  unsigned int Flags;
  int DefaultButton;
  char *Data;
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
    PItem->Focus=PInit->Focus;
    PItem->Selected=PInit->Selected;
    PItem->Flags=PInit->Flags;
    PItem->DefaultButton=PInit->DefaultButton;
    if ((unsigned int)PInit->Data<2000)
      FSF.sprintf(PItem->Data,"%s",GetMsg((unsigned int)PInit->Data));
    else
      FSF.sprintf(PItem->Data,"%s",PInit->Data);
  }
}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *psi)
{
  Info=*psi;
  FSF=*psi->FSF;
  Info.FSF=&FSF;
}

long WINAPI MyDialog(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  struct FarDialogItem DlgEdit;
  struct FarDialogItemData DlgData;
  static BOOL StatusBar;
  static struct EditorInfo ei;

  switch (Msg)
  {
    case DN_INITDIALOG:
      Info.EditorControl(ECTL_GETINFO,(void *)&ei);
      Info.SendDlgMessage(hDlg,DM_SETFOCUS,1,0);
      StatusBar = FALSE;
      return TRUE;

    case DN_MOUSECLICK:
      if (StatusBar)
      {
        DlgData.PtrLength=28;
        DlgData.PtrData=(char *)GetMsg(MWaiting);
        Info.SendDlgMessage(hDlg,DM_SETTEXT,8,(long)&DlgData);
        StatusBar = FALSE;
      }
      return FALSE;

    case DN_KEY:
      if (StatusBar)
      {
        DlgData.PtrLength=28;
        DlgData.PtrData=(char *)GetMsg(MWaiting);
        Info.SendDlgMessage(hDlg,DM_SETTEXT,8,(long)&DlgData);
        StatusBar = FALSE;
      }
      if (Param1 == 3 || Param1 == 4 || Param2 != KEY_ENTER)
        return FALSE;
      Param1 = 3;

    case DN_BTNCLICK:
      switch (Param1)
      {
        case 3:
        {
          Info.SendDlgMessage(hDlg,DM_GETDLGITEM,2,(long)&DlgEdit);
          int i = 0; //no etry == zero
          FSF.sscanf(DlgEdit.Data,"%d",&i);
          FSF.sprintf(DlgEdit.Data,"%d",i);
          Info.SendDlgMessage(hDlg,DM_SETDLGITEM,2,(long)&DlgEdit);
          Info.SendDlgMessage(hDlg,DM_GETDLGITEM,1,(long)&DlgEdit);
          if (SetEditorOption(ei.EditorID,DlgEdit.Data,(void *)&i))
          {
            Info.SendDlgMessage(hDlg,DM_ADDHISTORY,1,(long)DlgEdit.Data);
            FSF.sprintf(DlgEdit.Data,"%d",i);
            Info.SendDlgMessage(hDlg,DM_ADDHISTORY,2,(long)DlgEdit.Data);
            DlgData.PtrLength=28;
            DlgData.PtrData=(char *)GetMsg(MUpdateSuccessfull);
            Info.SendDlgMessage(hDlg,DM_SETTEXT,8,(long)&DlgData);
            StatusBar = TRUE;
          }
          else
          {
            DlgData.PtrLength=28;
            DlgData.PtrData=(char *)GetMsg(MUpdateFailed);
            Info.SendDlgMessage(hDlg,DM_SETTEXT,8,(long)&DlgData);
            StatusBar = TRUE;
          }
          break;
        }
        case 4:
        {
          int i;
          Info.SendDlgMessage(hDlg,DM_GETDLGITEM,1,(long)&DlgEdit);
          if (GetEditorSettings(ei.EditorID,DlgEdit.Data,(void *)&i))
          {
            Info.SendDlgMessage(hDlg,DM_ADDHISTORY,1,(long)DlgEdit.Data);
            Info.SendDlgMessage(hDlg,DM_GETDLGITEM,2,(long)&DlgEdit);
            FSF.sprintf(DlgEdit.Data,"%d",i);
            Info.SendDlgMessage(hDlg,DM_SETDLGITEM,2,(long)&DlgEdit);
            Info.SendDlgMessage(hDlg,DM_SHOWITEM,2,1);
            DlgData.PtrLength=28;
            DlgData.PtrData=(char *)GetMsg(MCheckSuccessfull);
            Info.SendDlgMessage(hDlg,DM_SETTEXT,8,(long)&DlgData);
            StatusBar = TRUE;
          }
          else
          {
            DlgData.PtrLength=28;
            DlgData.PtrData=(char *)GetMsg(MCheckFailed);
            Info.SendDlgMessage(hDlg,DM_SETTEXT,8,(long)&DlgData);
            StatusBar = TRUE;
          }
          break;
        }
      }
      Info.SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,1,1);
      Info.SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,2,1);
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

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{

  static const char ParamsHistory[] = "ESC-TSC-Params";
  static const char SettingsHistory[] = "ESC-TSC-Values";
  static const struct InitDialogItem PreDialogItems[] =
  {
    DI_DOUBLEBOX  ,3  ,1  ,32 ,9  ,0    ,0                      ,0                                                   ,0 ,(char *)MTitle,
    DI_EDIT       ,15 ,2  ,30 ,0  ,TRUE ,(DWORD)ParamsHistory   ,DIF_HISTORY|DIF_USELASTHISTORY|DIF_MANUALADDHISTORY ,0 ,"",
    DI_EDIT       ,15 ,4  ,30 ,0  ,0    ,(DWORD)SettingsHistory ,DIF_HISTORY|DIF_USELASTHISTORY|DIF_MANUALADDHISTORY ,0 ,"",
    DI_BUTTON     ,8  ,6  ,0  ,0  ,0    ,0                      ,DIF_BTNNOCLOSE|DIF_CENTERGROUP                      ,1 ,(char *)MUpdate,
    DI_BUTTON     ,8  ,6  ,0  ,0  ,0    ,0                      ,DIF_BTNNOCLOSE|DIF_CENTERGROUP                      ,0 ,(char *)MCheck,
    DI_TEXT       ,4  ,2  ,0  ,0  ,0    ,0                      ,0                                                   ,0 ,(char *)MParam,
    DI_TEXT       ,4  ,4  ,0  ,0  ,0    ,0                      ,0                                                   ,0 ,(char *)MValue,
    DI_TEXT       ,4  ,7  ,0  ,0  ,0    ,0                      ,0                                                   ,0 ,(char *)MStatus,
    DI_TEXT       ,4  ,8  ,0  ,0  ,0    ,0                      ,0                                                   ,0 ,(char *)MWaiting,
  };
  struct FarDialogItem DialogItems[sizeof(PreDialogItems)/sizeof(PreDialogItems[0])];

  if (FirstRun)
  {
    if (!hEsc)
    {
      hEsc=GetModuleHandle("esc.dll");
    }
    if (!GetEditorSettings && hEsc)
    {
      GetEditorSettings=(int (WINAPI*)(int, const char*, void*))GetProcAddress(hEsc,"GetEditorSettings");
    }
    if (!SetEditorOption && hEsc)
    {
      SetEditorOption=(int (WINAPI*)(int, const char*, void*))GetProcAddress(hEsc,"SetEditorOption");
    }
    if (!SetEditorOption || !GetEditorSettings)
    {
      const char *WMsg[2];
      WMsg[0] = GetMsg(MTitle);
      WMsg[1] = GetMsg(MNoESC);
      Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_MB_OK,NULL,WMsg,2,0);
      return (INVALID_HANDLE_VALUE);
    }
    FirstRun = FALSE;
  }

  InitDialogItems(PreDialogItems,DialogItems,sizeof(PreDialogItems)/sizeof(PreDialogItems[0]));
  Info.DialogEx(Info.ModuleNumber,-1,-1,36,11,NULL,DialogItems,sizeof(PreDialogItems)/sizeof(PreDialogItems[0]),0,0,MyDialog,0);

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
