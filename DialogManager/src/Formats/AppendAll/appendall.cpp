/*
    AppendAll plugin for DialogManager
    Copyright (C) 2005 Alex Yaroslavsky

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
#include "appendall.hpp"

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void)hDll;
  (void)dwReason;
  (void)lpReserved;
  return TRUE;
}

PluginStartupInfo FarInfo;
FARSTANDARDFUNCTIONS FSF;
DialogPluginStartupInfo DialogInfo;
static bool AppendAll=false;
static char tmp[50];

static char *GetMsg(int MsgNum,char *Str)
{
  DialogInfo.GetMsg(DialogInfo.MessageName,MsgNum,Str);
  return Str;
}

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct DialogPluginStartupInfo *DialogInfo)
{
  ::FarInfo=*FarInfo;
  FSF=*FarInfo->FSF;
  ::DialogInfo=*DialogInfo;
  //FSF.sprintf(PluginRootKey,"%s\\AppendAll",::DialogInfo.RootKey);
  return 0;
}

void WINAPI _export Exit(void)
{
}

int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData)
{
  (void)OutData;
  switch(Msg)
  {
    case FMMSG_GETINFO:
      {
        GetInfoOutData *data=(GetInfoOutData *)OutData;
        GetMsg(mName,data->MenuString);
        strcpy(data->HotkeyID,"ay_appendall");
        data->Flags=FMMSG_MENU|FMMSG_FILTER;
      }
      return TRUE;

    case FMMSG_MENU:
      {
        MenuInData *dlg=(MenuInData *)InData;
        FarDialogItem Item;
        if (FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,0,(long)&Item))
        {
          if (!strcmp(Item.Data,GetMsg(mWarningTitle,tmp)))
          {
            if (FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,11,(long)&Item))
            {
              if (strstr(Item.Data,GetMsg(mAppendButton,tmp)))
              {
                AppendAll=true;
                FarInfo.SendDlgMessage(dlg->hDlg,DM_CLOSE,11,0);
              }
            }
          }
        }
      }
      return TRUE;

    case FMMSG_FILTER:
      {
        FilterInData *dlg=(FilterInData *)InData;
        if(dlg->Msg==DN_INITDIALOG && AppendAll)
        {
          FarDialogItem Item;
          if (FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,0,(long)&Item))
          {
            if (!strcmp(Item.Data,GetMsg(mCopyTitle,tmp)) || !strcmp(Item.Data,GetMsg(mMoveTitle,tmp)))
            {
              if (FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,12,(long)&Item))
              {
                if (strstr(Item.Data,GetMsg(mTreeButton,tmp)))
                  AppendAll=false; //reset when new copy/move starts
              }
            }
            else if (!strcmp(Item.Data,GetMsg(mWarningTitle,tmp)))
            {
              if (FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,11,(long)&Item))
              {
                if (strstr(Item.Data,GetMsg(mAppendButton,tmp)))
                {
                  FarInfo.SendDlgMessage(dlg->hDlg,DM_CLOSE,11,0);
                }
              }
            }
          }
        }
      }
      return TRUE;
  }
  return FALSE;
}
