#include <stdio.h>
#include "dm_macro.hpp"

long WINAPI CopyDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  CopyDialogData *dialog_data=(CopyDialogData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  FarDialogItem DialogItemSrc,DialogItemDst;
  FarDialogItemData Caption1,Caption2,Caption3;
  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_SETTEXTLENGTH,COPYDLG_ETO,MAX_PATH-1);
    case DN_BTNCLICK:
      Info.SendDlgMessage(hDlg,DM_GETDLGITEM,COPYDLG_SKIP,(long)&DialogItemSrc);
      Info.SendDlgMessage(hDlg,DM_GETDLGITEM,COPYDLG_RODST,(long)&DialogItemDst);
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
      Info.SendDlgMessage(hDlg,DM_SETDLGITEM,COPYDLG_RODST,(long)&DialogItemDst);

      Info.SendDlgMessage(hDlg,DM_GETDLGITEM,COPYDLG_REMOVE,(long)&DialogItemSrc);
      Info.SendDlgMessage(hDlg,DM_GETDLGITEM,COPYDLG_ROSRC,(long)&DialogItemDst);
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
      Info.SendDlgMessage(hDlg,DM_SETDLGITEM,COPYDLG_ROSRC,(long)&DialogItemDst);
      if(DialogItemSrc.Selected)
      {
        Caption1.PtrData=(char *)GetMsg(mCpyDlgMoveTitle);
        Caption1.PtrLength=strlen(Caption1.PtrData);
        Caption2.PtrData=dialog_data->MoveMessage;
        Caption2.PtrLength=strlen(Caption2.PtrData);
        Caption3.PtrData=(char *)GetMsg(mCpyDlgOkMove);
        Caption3.PtrLength=strlen(Caption3.PtrData);
      }
      else
      {
        Caption1.PtrData=(char *)GetMsg(mCpyDlgCopyTitle);
        Caption1.PtrLength=strlen(Caption1.PtrData);
        Caption2.PtrData=dialog_data->CopyMessage;
        Caption2.PtrLength=strlen(Caption2.PtrData);
        Caption3.PtrData=(char *)GetMsg(mCpyDlgOkCopy);
        Caption3.PtrLength=strlen(Caption3.PtrData);
      }
      Info.SendDlgMessage(hDlg,DM_SETTEXT,COPYDLG_BORDER,(long)&Caption1);
      Info.SendDlgMessage(hDlg,DM_SETTEXT,COPYDLG_LTO,(long)&Caption2);
      Info.SendDlgMessage(hDlg,DM_SETTEXT,COPYDLG_OK,(long)&Caption3);
      break;
    case DN_ENTERIDLE:
      if(dialog_data->Macro) dialog_data->Macro=FALSE;
      break;
    case DN_KEY:
      if(!dialog_data->Macro)
      {
        Info.SendDlgMessage(hDlg,DM_GETDLGITEM,COPYDLG_REMOVE,(long)&DialogItemSrc);
        if(run_macro(DialogItemSrc.Selected?MACRO_MOVE:MACRO_COPY,Param2))
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

long WINAPI DelDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  CommonDialogData *dialog_data=(CommonDialogData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_ENTERIDLE:
      if(dialog_data->Macro) dialog_data->Macro=FALSE;
      break;
    case DN_KEY:
      if(!dialog_data->Macro&&run_macro(MACRO_DELETE,Param2))
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

long WINAPI AttrDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  CommonDialogData *dialog_data=(CommonDialogData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_ENTERIDLE:
      if(dialog_data->Macro) dialog_data->Macro=FALSE;
      break;
    case DN_KEY:
      if(!dialog_data->Macro&&run_macro(MACRO_ATTR,Param2))
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

long WINAPI WipeDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  CommonDialogData *dialog_data=(CommonDialogData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_ENTERIDLE:
      if(dialog_data->Macro) dialog_data->Macro=FALSE;
      break;
    case DN_KEY:
      if(!dialog_data->Macro&&run_macro(MACRO_WIPE,Param2))
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

long WINAPI Config1DialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

long WINAPI PwdDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_SETTEXTLENGTH,1,255);
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}
