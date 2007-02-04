/*
    ExtViewerSearch plugin for DialogManager
    Copyright (C) 2003-2004 Vadim Yegorov

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
#include <stdio.h>

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

const int original_width=76;
const int original_height=12;
const int original_count=13;

HANDLE curr_dlg=NULL;

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
};

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
  FSF.sprintf(PluginRootKey,"%s\\ExtVS",::DialogInfo.RootKey);
  return 0;
}

void WINAPI _export Exit(void)
{
}

static void UpdateHex(HANDLE hDlg)
{
  char in[512],out[512];
  unsigned long len;
  FarInfo.SendDlgMessage(hDlg,DM_GETTEXTPTR,2,(long)in);
  len=strlen(in);
  if(len>22) len=22;
  for(unsigned long i=0;i<len;i++)
    FSF.sprintf(out+i*3,"%02X ",(unsigned char)in[i]);
  FarInfo.SendDlgMessage(hDlg,DM_SETTEXTPTR,3,(long)out);
}

static void UpdateText(HANDLE hDlg)
{
  char in[512],out[512];
  unsigned long len=22;
  FarInfo.SendDlgMessage(hDlg,DM_GETTEXTPTR,3,(long)in);
  for(unsigned long i=0;i<len;i++)
  {
    unsigned long value;
    if(*(in+i*3)==' ')
    {
      len=i;
      break;
    }
    FSF.sscanf(in+i*3,"%02X ",&value);
    out[i]=value;
  }
  out[len]=0;
  FarInfo.SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(long)out);
}

static void Update(HANDLE hDlg)
{
  if(FarInfo.SendDlgMessage(hDlg,DM_GETCHECK,5,0)==BSTATE_CHECKED)
    UpdateHex(hDlg);
  else
    UpdateText(hDlg);
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
        strcpy(data->HotkeyID,"zg_extvs");
        data->Flags=FMMSG_FILTER;
      }
      return TRUE;
    case FMMSG_FILTER:
      {
        FilterInData *dlg=(FilterInData *)InData;
        if(dlg->Msg==DN_DRAWDIALOGDONE)
        {
          char caption[512];
          FarDialogItemData caption_data={sizeof(caption),caption};
          FarInfo.SendDlgMessage(dlg->hDlg,DM_GETTEXT,0,(long)&caption_data);
          if(!strcmp(caption,"Search"))
          {
            SMALL_RECT dialog_rect;
            FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGRECT,0,(long)&dialog_rect);
            int width=dialog_rect.Right-dialog_rect.Left+1,height=dialog_rect.Bottom-dialog_rect.Top+1;
            if(width==original_width&&height==original_height)
            {
              COORD new_size={width,height+2},new_pos={-1,-1};
              FarInfo.SendDlgMessage(dlg->hDlg,DM_RESIZEDIALOG,0,(long)&new_size);
              FarInfo.SendDlgMessage(dlg->hDlg,DM_MOVEDIALOG,0,(long)&new_pos);
              SMALL_RECT item_rect;
              for(int i=0;i<original_count;i++)
              {
                if(i==1) continue; if(i==2) continue;
                FarInfo.SendDlgMessage(dlg->hDlg,DM_GETITEMPOSITION,i,(long)&item_rect);
                if(i) item_rect.Top+=2;
                item_rect.Bottom+=2;
                FarInfo.SendDlgMessage(dlg->hDlg,DM_SETITEMPOSITION,i,(long)&item_rect);
              }
              FarInfo.SendDlgMessage(dlg->hDlg,DM_SHOWITEM,3,1);
              Update(dlg->hDlg);
              FarInfo.SendDlgMessage(dlg->hDlg,DM_REDRAW,0,0);
              curr_dlg=dlg->hDlg;
            }
          }
        }
        else if(curr_dlg==dlg->hDlg)
        {
          switch(dlg->Msg)
          {
            case DN_EDITCHANGE:
              if(dlg->Param1==2)
                UpdateHex(dlg->hDlg);
              else if(dlg->Param1==3)
                UpdateText(dlg->hDlg);
              break;
            case DN_DRAWDIALOG:
              FarInfo.SendDlgMessage(dlg->hDlg,DM_SHOWITEM,2,1);
              FarInfo.SendDlgMessage(dlg->hDlg,DM_SHOWITEM,3,1);
              break;
          }
        }
      }
      return TRUE;
  }
  return FALSE;
}
