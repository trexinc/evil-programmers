/*
    bcplugdialogproc.cpp
    Copyright (C) 2000-2009 zg

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
#include "dm_macro.hpp"

LONG_PTR WINAPI CopyDialogProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  CopyDialogData *dialog_data=(CopyDialogData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  FarDialogItem DialogItemSrc,DialogItemDst;
  FarDialogItemData Caption1,Caption2,Caption3;
  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_SETTEXTLENGTH,COPYDLG_ETO,MAX_PATH-1);
    case DN_BTNCLICK:
      Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,COPYDLG_SKIP,(LONG_PTR)&DialogItemSrc);
      Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,COPYDLG_RODST,(LONG_PTR)&DialogItemDst);
      if(DialogItemSrc.Selected)
      {
        if(!(DialogItemDst.Flags&DIF_DISABLE))
          dialog_data->DstRO=DialogItemDst.Selected;
        DialogItemDst.Selected=FALSE;
        DialogItemDst.Flags|=DIF_DISABLE;
      }
      else
      {
        if(DialogItemDst.Flags&DIF_DISABLE)
          DialogItemDst.Selected=dialog_data->DstRO;
        DialogItemDst.Flags&=~DIF_DISABLE;
      }
      Info.SendDlgMessage(hDlg,DM_SETDLGITEMSHORT,COPYDLG_RODST,(LONG_PTR)&DialogItemDst);

      Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,COPYDLG_REMOVE,(LONG_PTR)&DialogItemSrc);
      Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,COPYDLG_ROSRC,(LONG_PTR)&DialogItemDst);
      if(DialogItemSrc.Selected)
      {
        if(DialogItemDst.Flags&DIF_DISABLE)
          DialogItemDst.Selected=dialog_data->SrcRO;
        DialogItemDst.Flags&=~DIF_DISABLE;
      }
      else
      {
        if(!(DialogItemDst.Flags&DIF_DISABLE))
          dialog_data->SrcRO=DialogItemDst.Selected;
        DialogItemDst.Selected=FALSE;
        DialogItemDst.Flags|=DIF_DISABLE;
      }
      Info.SendDlgMessage(hDlg,DM_SETDLGITEMSHORT,COPYDLG_ROSRC,(LONG_PTR)&DialogItemDst);
      if(DialogItemSrc.Selected)
      {
        Caption1.PtrData=(TCHAR *)GetMsg(mCpyDlgMoveTitle);
        Caption1.PtrLength=(int)_tcslen(Caption1.PtrData);
        Caption2.PtrData=dialog_data->MoveMessage;
        Caption2.PtrLength=(int)_tcslen(Caption2.PtrData);
        Caption3.PtrData=(TCHAR *)GetMsg(mCpyDlgOkMove);
        Caption3.PtrLength=(int)_tcslen(Caption3.PtrData);
      }
      else
      {
        Caption1.PtrData=(TCHAR *)GetMsg(mCpyDlgCopyTitle);
        Caption1.PtrLength=(int)_tcslen(Caption1.PtrData);
        Caption2.PtrData=dialog_data->CopyMessage;
        Caption2.PtrLength=(int)_tcslen(Caption2.PtrData);
        Caption3.PtrData=(TCHAR *)GetMsg(mCpyDlgOkCopy);
        Caption3.PtrLength=(int)_tcslen(Caption3.PtrData);
      }
      Info.SendDlgMessage(hDlg,DM_SETTEXT,COPYDLG_BORDER,(LONG_PTR)&Caption1);
      Info.SendDlgMessage(hDlg,DM_SETTEXT,COPYDLG_LTO,(LONG_PTR)&Caption2);
      Info.SendDlgMessage(hDlg,DM_SETTEXT,COPYDLG_OK,(LONG_PTR)&Caption3);
      break;
    case DN_ENTERIDLE:
      if(dialog_data->Macro) dialog_data->Macro=FALSE;
      break;
    case DN_KEY:
      if(!dialog_data->Macro)
      {
        Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,COPYDLG_REMOVE,(LONG_PTR)&DialogItemSrc);
        if(run_macro(DialogItemSrc.Selected?MACRO_MOVE:MACRO_COPY,(DWORD)Param2))
        {
          dialog_data->Macro=TRUE;
          return TRUE;
        }
      }
      break;
    case DN_MACRO_DISABLE:
      return TRUE;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

LONG_PTR WINAPI DelDialogProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  CommonDialogData *dialog_data=(CommonDialogData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_ENTERIDLE:
      if(dialog_data->Macro) dialog_data->Macro=FALSE;
      break;
    case DN_KEY:
      if(!dialog_data->Macro&&run_macro(MACRO_DELETE,(DWORD)Param2))
      {
        dialog_data->Macro=TRUE;
        return TRUE;
      }
      break;
    case DN_MACRO_DISABLE:
      return TRUE;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

LONG_PTR WINAPI AttrDialogProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  CommonDialogData *dialog_data=(CommonDialogData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_ENTERIDLE:
      if(dialog_data->Macro) dialog_data->Macro=FALSE;
      break;
    case DN_KEY:
      if(!dialog_data->Macro&&run_macro(MACRO_ATTR,(DWORD)Param2))
      {
        dialog_data->Macro=TRUE;
        return TRUE;
      }
      break;
    case DN_MACRO_DISABLE:
      return TRUE;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

LONG_PTR WINAPI WipeDialogProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  CommonDialogData *dialog_data=(CommonDialogData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_ENTERIDLE:
      if(dialog_data->Macro) dialog_data->Macro=FALSE;
      break;
    case DN_KEY:
      if(!dialog_data->Macro&&run_macro(MACRO_WIPE,(DWORD)Param2))
      {
        dialog_data->Macro=TRUE;
        return TRUE;
      }
      break;
    case DN_MACRO_DISABLE:
      return TRUE;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

LONG_PTR WINAPI Config1DialogProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

LONG_PTR WINAPI PwdDialogProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_SETTEXTLENGTH,1,255);
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}
