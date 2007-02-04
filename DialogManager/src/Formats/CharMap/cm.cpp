/*
    CharMap plugin for DialogManager
    Copyright (C) 2003-2004 Vadim Yegorov and Alex Yaroslavsky

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
#include "../../farkeys.hpp"
#include "cm.hpp"

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

typedef HANDLE (WINAPI *OpenPluginProc)(int OpenFrom,int Item);

OpenPluginProc OpenPluginCharMap=NULL;

char *GetMsg(int MsgNum,char *Str)
{
  DialogInfo.GetMsg(DialogInfo.MessageName,MsgNum,Str);
  return Str;
}

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct DialogPluginStartupInfo *DialogInfo)
{
  ::FarInfo=*FarInfo;
  FSF=*FarInfo->FSF;
  ::DialogInfo=*DialogInfo;
  FSF.sprintf(PluginRootKey,"%s\\CharMap",::DialogInfo.RootKey);
  if(!OpenPluginCharMap)
  {
    HMODULE m=GetModuleHandle("CharacterMap.dll");
    OpenPluginCharMap=m?((OpenPluginProc)GetProcAddress(m,"OpenPlugin")):NULL;
  }
  return 0;
}

void WINAPI _export Exit(void)
{
  OpenPluginCharMap=NULL;
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
        strcpy(data->HotkeyID,"zg+ay_charactermap");
        data->Flags=FMMSG_MENU;
      }
      return TRUE;
    case FMMSG_MENU:
      {
        if(OpenPluginCharMap)
        {
          MenuInData *dlg=(MenuInData *)InData;
          FarDialogItem DialogItem;
          FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,dlg->ItemID,(long)&DialogItem);
          if(DialogItem.Type==DI_EDIT||DialogItem.Type==DI_PSWEDIT)
          {
            long length=FarInfo.SendDlgMessage(dlg->hDlg,DM_GETTEXTLENGTH,dlg->ItemID,0)+1;
            unsigned char *buffer=(unsigned char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,length);
            if(buffer)
            {
              COORD Pos; FarInfo.SendDlgMessage(dlg->hDlg,DM_GETCURSORPOS,dlg->ItemID,(long)&Pos);
              FarInfo.SendDlgMessage(dlg->hDlg,DM_GETTEXTPTR,dlg->ItemID,(long)buffer);
              unsigned char out_buffer[512];
              out_buffer[0]=buffer[Pos.X];
              long out_length=(long)OpenPluginCharMap(255,(int)out_buffer);
              if(out_length)
              {
                unsigned char *result_buffer=(unsigned char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,length+out_length);
                if(result_buffer)
                {
                  memcpy(result_buffer,buffer,Pos.X);
                  memcpy(result_buffer+Pos.X,out_buffer,out_length);
                  memcpy(result_buffer+Pos.X+out_length,buffer+Pos.X,length-Pos.X);
                  FarInfo.SendDlgMessage(dlg->hDlg,DM_SETTEXTPTR,dlg->ItemID,(long)result_buffer);
                  FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,dlg->ItemID,(long)&DialogItem);
                  FarInfo.SendDlgMessage(dlg->hDlg,DN_EDITCHANGE,dlg->ItemID,(long)&DialogItem);
                  Pos.X+=out_length;
                  FarInfo.SendDlgMessage(dlg->hDlg,DM_SETCURSORPOS,dlg->ItemID,(long)&Pos);
                  HeapFree(GetProcessHeap(),0,result_buffer);
                }
              }
              HeapFree(GetProcessHeap(),0,buffer);
            }
          }
        }
      }
      return TRUE;
  }
  return FALSE;
}
