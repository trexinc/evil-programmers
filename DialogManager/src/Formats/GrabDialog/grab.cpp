/*
    GrabDialog plugin for DialogManager
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
#include "grab_func.hpp"

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
  mGrabFilePath,
  mGrabPlace,
  mFullMode,
  mConfig,
  mSave,
  mCancel
};

PluginStartupInfo FarInfo;
FARSTANDARDFUNCTIONS FSF;
DialogPluginStartupInfo DialogInfo;
char PluginRootKey[100];
char GrabPath[MAX_PATH];
BOOL FullMode=FALSE;
BOOL ShowInConfig=TRUE;
HANDLE GrabMutex=NULL;

char msg1[128];
char msg2[128];

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
  FSF.sprintf(PluginRootKey,"%s\\Grab",::DialogInfo.RootKey);
  GetMsg(mGrabFilePath,GrabPath);
  HKEY hKey;
  if(RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
  {
    DWORD Type,DataSize=sizeof(GrabPath);
    RegQueryValueEx(hKey,"GrabPath",0,&Type,(BYTE *)GrabPath,&DataSize);
    DataSize=sizeof(FullMode);
    RegQueryValueEx(hKey,"FullMode",0,&Type,(BYTE *)&FullMode,&DataSize);
    DataSize=sizeof(ShowInConfig);
    RegQueryValueEx(hKey,"ShowInConfig",0,&Type,(BYTE *)&ShowInConfig,&DataSize);
    RegCloseKey(hKey);
  }
  GrabMutex=CreateMutex(NULL,FALSE,"FAR_DialogM_GrabDialog_ShareMutex");
  return 0;
}

void WINAPI _export Exit(void)
{
  CloseHandle(GrabMutex);
}

static long WINAPI ConfigDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  switch(Msg)
  {
    case DN_INITDIALOG:
      FarInfo.SendDlgMessage(hDlg,DM_SETTEXTLENGTH,2,MAX_PATH-1);
      break;
  }
  return FarInfo.DefDlgProc(hDlg,Msg,Param1,Param2);
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
        strcpy(data->HotkeyID,"zg+ay_grab");
        data->Flags=FMMSG_MENU;
        if(ShowInConfig) data->Flags|=FMMSG_CONFIG;
      }
      return TRUE;
    case FMMSG_MENU:
      {
        MenuInData *dlg=(MenuInData *)InData;
        HANDLE log=CreateFile(GrabPath,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,0,NULL);
        if(log!=INVALID_HANDLE_VALUE)
        {
          if(FullMode)
            FullGrab(log,dlg);
          else
            SimpleGrab(log,dlg);
          CloseHandle(log);
        }
      }
      return TRUE;
    case FMMSG_CONFIG:
      {
        FarDialogItem DialogItems[8]; int idx=0;
        memset(DialogItems,0,sizeof(DialogItems));
        DialogItems[idx].Type=DI_DOUBLEBOX; DialogItems[idx].X1=3; DialogItems[idx].X2=72; DialogItems[idx].Y1=1; DialogItems[idx].Y2=9;
        GetMsg(mName,DialogItems[idx].Data.Data); idx++;
        DialogItems[idx].Type=DI_TEXT; DialogItems[idx].X1=5; DialogItems[idx].Y1=2; GetMsg(mGrabPlace,DialogItems[idx].Data.Data); idx++;
        DialogItems[idx].Type=DI_EDIT; DialogItems[idx].X1=5; DialogItems[idx].Y1=3; DialogItems[idx].X2=70;
        DialogItems[idx].Flags=DIF_HISTORY; DialogItems[idx].Param.History="DialogM\\GrabPath"; strcpy(DialogItems[idx].Data.Data,GrabPath); idx++;
        DialogItems[idx].Type=DI_CHECKBOX; DialogItems[idx].X1=5; DialogItems[idx].Y1=5; GetMsg(mFullMode,DialogItems[idx].Data.Data); DialogItems[idx].Param.Selected=FullMode; idx++;
        DialogItems[idx].Type=DI_CHECKBOX; DialogItems[idx].X1=5; DialogItems[idx].Y1=6; GetMsg(mConfig,DialogItems[idx].Data.Data); DialogItems[idx].Param.Selected=ShowInConfig; idx++;
        DialogItems[idx].Type=DI_TEXT; DialogItems[idx].X1=5; DialogItems[idx].Y1=7; DialogItems[idx].Flags=DIF_BOXCOLOR|DIF_SEPARATOR; idx++;
        DialogItems[idx].Type=DI_BUTTON; DialogItems[idx].Y1=8; DialogItems[idx].Flags=DIF_CENTERGROUP;
        DialogItems[idx].DefaultButton=TRUE; GetMsg(mSave,DialogItems[idx].Data.Data); idx++;
        DialogItems[idx].Type=DI_BUTTON; DialogItems[idx].Y1=8; DialogItems[idx].Flags=DIF_CENTERGROUP;
        GetMsg(mCancel,DialogItems[idx].Data.Data);
        int DlgCode=FarInfo.DialogEx(FarInfo.ModuleNumber,-1,-1,76,11,NULL,DialogItems,sizeofa(DialogItems),0,0,ConfigDialogProc,0);
        if(DlgCode==(idx-1))
        {
          strcpy(GrabPath,DialogItems[2].Data.Data);
          FullMode=DialogItems[3].Param.Selected;
          ShowInConfig=DialogItems[4].Param.Selected;
          HKEY hKey; DWORD Disposition;
          if(RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition)==ERROR_SUCCESS)
          {
            RegSetValueEx(hKey,"GrabPath",0,REG_SZ,(BYTE *)GrabPath,strlen(GrabPath)+1);
            RegSetValueEx(hKey,"FullMode",0,REG_DWORD,(BYTE *)&FullMode,sizeof(FullMode));
            RegSetValueEx(hKey,"ShowInConfig",0,REG_DWORD,(BYTE *)&ShowInConfig,sizeof(ShowInConfig));
            RegCloseKey(hKey);
          }
          return TRUE;
        }
      }
      return FALSE;
  }
  return FALSE;
}
