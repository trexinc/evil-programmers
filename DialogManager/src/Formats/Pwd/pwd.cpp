/*
    Pwd plugin for DialogManager
    Copyright (C) 2003 Vadim Yegorov
    Copyright (C) 2004 Vadim Yegorov and Alex Yaroslavsky

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
#include <stdio.h>
#include <stdlib.h>
#include "../../plugin.hpp"
#include "../../dm_module.hpp"
#include "memory.hpp"

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

enum
{
  mName,
  mPwd,
  mOk,
};

PluginStartupInfo FarInfo;
FARSTANDARDFUNCTIONS FSF;
DialogPluginStartupInfo DialogInfo;
char PluginRootKey[100];

char msg1[128];
char msg2[128];

char *GetMsg(int MsgNum,char *Str)
{
  DialogInfo.GetMsg(DialogInfo.MessageName,MsgNum,Str);
  return Str;
}

struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1, Y1, X2, Y2;
  unsigned int Flags;
  signed char Data;
};

void InitDialogItems(const struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber)
{
  int i;
  struct FarDialogItem *PItem = Item;
  const struct InitDialogItem *PInit = Init;

  for (i=0; i<ItemsNumber; i++, PItem++, PInit++)
  {
    PItem->Type = PInit->Type;
    PItem->X1 = PInit->X1;
    PItem->Y1 = PInit->Y1;
    PItem->X2 = PInit->X2;
    PItem->Y2 = PInit->Y2;
    PItem->Focus = 0;
    PItem->Selected = 0;
    PItem->Flags = PInit->Flags;
    PItem->DefaultButton = 0;
    if(PInit->Data!=-1)
      GetMsg(PInit->Data,PItem->Data);
    else
      *(PItem->Data)=0;
  }
}

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct DialogPluginStartupInfo *DialogInfo)
{
  ::FarInfo=*FarInfo;
  FSF=*FarInfo->FSF;
  ::DialogInfo=*DialogInfo;
  FSF.sprintf(PluginRootKey,"%s\\Pwd",::DialogInfo.RootKey);
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
        strcpy(data->HotkeyID,"zg_pwd");
        data->Flags=FMMSG_MENU;
      }
      return TRUE;
    case FMMSG_MENU:
      {
        MenuInData *dlg=(MenuInData *)InData;
        FarDialogItem DialogItem;
        FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,dlg->ItemID,(long)&DialogItem);
        if(DialogItem.Type==DI_PSWEDIT)
        {
          long length=FarInfo.SendDlgMessage(dlg->hDlg,DM_GETTEXTLENGTH,dlg->ItemID,0)+1;
          char *buffer=(char *)malloc(length);
          if(buffer)
          {
            static const struct InitDialogItem PreDialogItems[] =
            {
              DI_DOUBLEBOX   ,3  ,1  ,43 ,5  ,0                  ,mName,
              DI_TEXT        ,5  ,2  ,0  ,0  ,0                  ,mPwd,
              DI_EDIT        ,5  ,3  ,41 ,0  ,DIF_READONLY       ,-1,
              DI_BUTTON      ,0  ,4  ,0  ,0  ,DIF_CENTERGROUP    ,mOk,
            };
            struct FarDialogItem DialogItems[sizeof(PreDialogItems)/sizeof(PreDialogItems[0])];

            InitDialogItems(PreDialogItems,DialogItems,sizeof(PreDialogItems)/sizeof(PreDialogItems[0]));
            DialogItems[3].DefaultButton = 1;
            FarInfo.SendDlgMessage(dlg->hDlg,DM_GETTEXTPTR,dlg->ItemID,(long)buffer);
            if (length>512)
            {
              DialogItems[2].Ptr.PtrLength=length;
              DialogItems[2].Ptr.PtrData=buffer;
              DialogItems[2].Flags|=DIF_VAREDIT;
            }
            else
            {
              strcpy(DialogItems[2].Data,buffer);
            }
            FarInfo.Dialog(FarInfo.ModuleNumber,-1,-1,47,7,NULL,(struct FarDialogItem *)&DialogItems,sizeof(PreDialogItems)/sizeof(PreDialogItems[0]));
            free(buffer);
          }
        }

      }
      return TRUE;
  }
  return FALSE;
}
