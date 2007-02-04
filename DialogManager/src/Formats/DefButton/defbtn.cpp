/*
    DefButton plugin for DialogManager
    Copyright (C) 2004 Vadim Yegorov
    Copyright (C) 2005 Vadim Yegorov & Alex Yaroslavsky

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
#include "../../plugin.hpp"
#include "../../dm_module.hpp"

enum
{
  mName,
};

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

PluginStartupInfo FarInfo;
FARSTANDARDFUNCTIONS FSF;
DialogPluginStartupInfo DialogInfo;
char PluginRootKey[100];

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
  FSF.sprintf(PluginRootKey,"%s\\DefButton",::DialogInfo.RootKey);
  return 0;
}

void WINAPI _export Exit(void)
{
}

#define DIF_DEFBTN_HACK 0x08000000UL

int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData)
{
  (void)OutData;
  switch(Msg)
  {
    case FMMSG_GETINFO:
      {
        GetInfoOutData *data=(GetInfoOutData *)OutData;
        GetMsg(mName,data->MenuString);
        strcpy(data->HotkeyID,"zg_defbtn");
        data->Flags=FMMSG_FILTER;
      }
      return TRUE;
    case FMMSG_FILTER:
      {
        FilterInData *dlg=(FilterInData *)InData;
        if(dlg->Msg==DN_DRAWDLGITEM)
        {
          FarDialogItem *DialogItem=(FarDialogItem *)dlg->Param2;
          if(DialogItem->DefaultButton&&DialogItem->Type==DI_BUTTON)
          {
            long length=strlen(DialogItem->Data);
            if(DialogItem->Data[0]=='['&&DialogItem->Data[length-1]==']')
            {
              DialogItem->Data[0]='{';
              DialogItem->Data[length-1]='}';
              DialogItem->Flags|=DIF_NOBRACKETS|DIF_DEFBTN_HACK;
            }
          }
        }
        else if(dlg->Msg==DN_CTLCOLORDIALOG)
        {
          FarDialogItem DialogItem;
          for(int i=0; FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,i,(long)&DialogItem); i++)
          {
            if(DialogItem.DefaultButton&&DialogItem.Type==DI_BUTTON&&(DialogItem.Flags&DIF_DEFBTN_HACK)&&DialogItem.Data[0]!='{')
            {

              char buffer[512];
              strcpy(buffer,DialogItem.Data);
              strcpy(DialogItem.Data,"{ ");
              strcat(DialogItem.Data,buffer);
              strcat(DialogItem.Data," }");
              FarInfo.SendDlgMessage(dlg->hDlg,DM_SETDLGITEM,i,(long)&DialogItem);
              break;
            }
          }
        }
      }
      return TRUE;
  }
  return FALSE;
}
