/*
    Logger plugin for DialogManager
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
#include "../../plugin.hpp"
#include "../../dm_module.hpp"

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
  mStartLog,
  mStopLog,
  mLogFilePath,
  mLogPlace,
  mDialog,
  mPanels,
  mConfig,
  mSave,
  mCancel
};

PluginStartupInfo FarInfo;
FARSTANDARDFUNCTIONS FSF;
DialogPluginStartupInfo DialogInfo;
char PluginRootKey[100];
HANDLE LogMutex=NULL;
HANDLE fh;
int Log;
char LogPath[MAX_PATH];
BOOL ShowInDialog=TRUE;
BOOL ShowInPanels=TRUE;
BOOL ShowInConfig=TRUE;

static HANDLE OpenLog(char *FileName)
{
  HANDLE fh=CreateFile(FileName,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,0,NULL);
  return fh;
}

static void WriteToLog(HANDLE fh, const char *line)
{
  if(fh!=INVALID_HANDLE_VALUE)
  {
    DWORD t; LONG dist=0;
    WaitForSingleObject(LogMutex,INFINITE);
    if((SetFilePointer(fh,0,&dist,FILE_END)!=0xFFFFFFFF)||(GetLastError()==NO_ERROR)) WriteFile(fh,line,strlen(line),&t,NULL);
    ReleaseMutex(LogMutex);
  }
}

static void CloseLog(HANDLE *fh)
{
  if(fh!=INVALID_HANDLE_VALUE)
  {
    CloseHandle(*fh);
    *fh=INVALID_HANDLE_VALUE;
  }
}

static void PrintDialogMessages(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  const char *Messages1[]=
  {
    "DM_FIRST",
    "DM_CLOSE",
    "DM_ENABLE",
    "DM_ENABLEREDRAW",
    "DM_GETDLGDATA",
    "DM_GETDLGITEM",
    "DM_GETDLGRECT",
    "DM_GETTEXT",
    "DM_GETTEXTLENGTH",
    "DM_KEY",
    "DM_MOVEDIALOG",
    "DM_SETDLGDATA",
    "DM_SETDLGITEM",
    "DM_SETFOCUS",
    "DM_REDRAW",
    "DM_SETTEXT",
    "DM_SETMAXTEXTLENGTH",
    "DM_SHOWDIALOG",
    "DM_GETFOCUS",
    "DM_GETCURSORPOS",
    "DM_SETCURSORPOS",
    "DM_GETTEXTPTR",
    "DM_SETTEXTPTR",
    "DM_SHOWITEM",
    "DM_ADDHISTORY",
    "DM_GETCHECK",
    "DM_SETCHECK",
    "DM_SET3STATE",
    "DM_LISTSORT",
    "DM_LISTGETITEM",
    "DM_LISTGETCURPOS",
    "DM_LISTSETCURPOS",
    "DM_LISTDELETE",
    "DM_LISTADD",
    "DM_LISTADDSTR",
    "DM_LISTUPDATE",
    "DM_LISTINSERT",
    "DM_LISTFINDSTRING",
    "DM_LISTINFO",
    "DM_LISTGETDATA",
    "DM_LISTSETDATA",
    "DM_LISTSETTITLES",
    "DM_LISTGETTITLES",
    "DM_RESIZEDIALOG",
    "DM_SETITEMPOSITION",
    "DM_GETDROPDOWNOPENED",
    "DM_SETDROPDOWNOPENED",
    "DM_SETHISTORY",
    "DM_GETITEMPOSITION",
    "DM_SETMOUSEEVENTNOTIFY",
    "DM_EDITUNCHANGEDFLAG",
    "DM_GETITEMDATA",
    "DM_SETITEMDATA",
    "DM_LISTSET",
    "DM_LISTSETMOUSEREACTION",
    "DM_GETCURSORSIZE",
    "DM_SETCURSORSIZE",
    "DM_LISTGETDATASIZE",
    "DM_GETSELECTION",
    "DM_SETSELECTION",
  };
  const char *Messages2[]=
  {
    "DN_FIRST",
    "DN_BTNCLICK",
    "DN_CTLCOLORDIALOG",
    "DN_CTLCOLORDLGITEM",
    "DN_CTLCOLORDLGLIST",
    "DN_DRAWDIALOG",
    "DN_DRAWDLGITEM",
    "DN_EDITCHANGE",
    "DN_ENTERIDLE",
    "DN_GOTFOCUS",
    "DN_HELP",
    "DN_HOTKEY",
    "DN_INITDIALOG",
    "DN_KILLFOCUS",
    "DN_LISTCHANGE",
    "DN_MOUSECLICK",
    "DN_DRAGGED",
    "DN_RESIZECONSOLE",
    "DN_MOUSEEVENT",
    "DN_DRAWDIALOGDONE",
  };
  const char *MessageName="Unknown";
  if(Msg>=0&&Msg<(int)(sizeof(Messages1)/sizeof(Messages1[0]))) MessageName=Messages1[Msg];
  if(Msg>=0x1000&&Msg<(int)(0x1000+sizeof(Messages2)/sizeof(Messages2[0]))) MessageName=Messages2[Msg-0x1000];
  {
    char log[1024];
    WindowInfo wi;
    wi.Pos=-1;
    FarInfo.AdvControl(FarInfo.ModuleNumber,ACTL_GETWINDOWINFO,(void *)&wi);
    FSF.sprintf(log,"%s <0x%08lx:0x%08lx> - %p 0x%04x - [%s] 0x%x 0x%lx\n",wi.Name,GetCurrentProcessId(),GetCurrentThreadId(),hDlg,Msg,MessageName,Param1,Param2);
    WriteToLog(fh,log);
  }
}

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
  FSF.sprintf(PluginRootKey,"%s\\Logger",::DialogInfo.RootKey);
  fh=INVALID_HANDLE_VALUE;
  GetMsg(mLogFilePath,LogPath);
  HKEY hKey;
  if(RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
  {
    DWORD Type,DataSize=sizeof(LogPath);
    RegQueryValueEx(hKey,"LogPath",0,&Type,(BYTE *)LogPath,&DataSize);
    DataSize=sizeof(ShowInDialog);
    RegQueryValueEx(hKey,"ShowInDialog",0,&Type,(BYTE *)&ShowInDialog,&DataSize);
    DataSize=sizeof(ShowInPanels);
    RegQueryValueEx(hKey,"ShowInPanels",0,&Type,(BYTE *)&ShowInPanels,&DataSize);
    DataSize=sizeof(ShowInConfig);
    RegQueryValueEx(hKey,"ShowInConfig",0,&Type,(BYTE *)&ShowInConfig,&DataSize);
    DataSize=sizeof(Log);
    RegQueryValueEx(hKey,"Log",0,&Type,(BYTE *)&Log,&DataSize);
    RegCloseKey(hKey);
  }
  LogMutex=CreateMutex(NULL,FALSE,"FAR_DialogM_Logger_ShareMutex");
  if(Log) fh=OpenLog(LogPath);
  return 0;
}

void WINAPI _export Exit(void)
{
  CloseLog(&fh);
  CloseHandle(LogMutex);
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
        strcpy(data->HotkeyID,"zg+ay_logger");
        data->Flags=FMMSG_FILTER;
        if(ShowInDialog) data->Flags|=FMMSG_MENU;
        if(ShowInPanels) data->Flags|=FMMSG_PANEL;
        if(ShowInConfig) data->Flags|=FMMSG_CONFIG;
      }
      return TRUE;
    case FMMSG_PANEL:
    case FMMSG_MENU:
      {
        HKEY hKey;
        if(RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
        {
          DWORD Type,DataSize=sizeof(Log);
          RegQueryValueEx(hKey,"Log",0,&Type,(BYTE *)&Log,&DataSize);
          RegCloseKey(hKey);
        }
        char temp[512], temp2[512];
        const char *msg[2];
        msg[0]=GetMsg(mName,temp);
        msg[1]=GetMsg(mStartLog+(Log?1:0),temp2);
        if(FarInfo.Message(FarInfo.ModuleNumber,FMSG_MB_YESNO,NULL,msg,2,0)==0)
        {
          Log=!Log;

          HKEY hKey; DWORD Disposition;
          if(RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition)==ERROR_SUCCESS)
          {
            RegSetValueEx(hKey,"Log",0,REG_DWORD,(BYTE *)&Log,sizeof(Log));
            RegCloseKey(hKey);
          }
          if(Log)
          {
            fh=OpenLog(LogPath);
          }
          else
          {
            CloseLog(&fh);
          }
        }
      }
      return TRUE;
    case FMMSG_FILTER:
      {
        if(Log)
        {
          FilterInData *dlg=(FilterInData *)InData;
          PrintDialogMessages(dlg->hDlg,dlg->Msg,dlg->Param1,dlg->Param2);
        }
      }
      return TRUE;
    case FMMSG_CONFIG:
      {
        FarDialogItem DialogItems[9]; int idx=0;
        memset(DialogItems,0,sizeof(DialogItems));
        DialogItems[idx].Type=DI_DOUBLEBOX; DialogItems[idx].X1=3; DialogItems[idx].X2=72; DialogItems[idx].Y1=1; DialogItems[idx].Y2=10;
        GetMsg(mName,DialogItems[idx].Data); idx++;
        DialogItems[idx].Type=DI_TEXT; DialogItems[idx].X1=5; DialogItems[idx].Y1=2; GetMsg(mLogPlace,DialogItems[idx].Data); idx++;
        DialogItems[idx].Type=DI_EDIT; DialogItems[idx].X1=5; DialogItems[idx].Y1=3; DialogItems[idx].X2=70;
        DialogItems[idx].Flags=DIF_HISTORY; DialogItems[idx].History="DialogM\\LoggerPath"; strcpy(DialogItems[idx].Data,LogPath); idx++;
        DialogItems[idx].Type=DI_CHECKBOX; DialogItems[idx].X1=5; DialogItems[idx].Y1=5; GetMsg(mDialog,DialogItems[idx].Data); DialogItems[idx].Selected=ShowInDialog; idx++;
        DialogItems[idx].Type=DI_CHECKBOX; DialogItems[idx].X1=5; DialogItems[idx].Y1=6; GetMsg(mPanels,DialogItems[idx].Data); DialogItems[idx].Selected=ShowInPanels; idx++;
        DialogItems[idx].Type=DI_CHECKBOX; DialogItems[idx].X1=5; DialogItems[idx].Y1=7; GetMsg(mConfig,DialogItems[idx].Data); DialogItems[idx].Selected=ShowInConfig; idx++;
        DialogItems[idx].Type=DI_TEXT; DialogItems[idx].X1=5; DialogItems[idx].Y1=8; DialogItems[idx].Flags=DIF_BOXCOLOR|DIF_SEPARATOR; idx++;
        DialogItems[idx].Type=DI_BUTTON; DialogItems[idx].Y1=9; DialogItems[idx].Flags=DIF_CENTERGROUP;
        DialogItems[idx].DefaultButton=TRUE; GetMsg(mSave,DialogItems[idx].Data); idx++;
        DialogItems[idx].Type=DI_BUTTON; DialogItems[idx].Y1=9; DialogItems[idx].Flags=DIF_CENTERGROUP;
        GetMsg(mCancel,DialogItems[idx].Data);
        int DlgCode=FarInfo.DialogEx(FarInfo.ModuleNumber,-1,-1,76,12,NULL,DialogItems,sizeofa(DialogItems),0,0,ConfigDialogProc,0);
        if(DlgCode==(idx-1))
        {
          strcpy(LogPath,DialogItems[2].Data);
          ShowInDialog=DialogItems[3].Selected;
          ShowInPanels=DialogItems[4].Selected;
          ShowInConfig=DialogItems[5].Selected;
          HKEY hKey; DWORD Disposition;
          if(RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition)==ERROR_SUCCESS)
          {
            RegSetValueEx(hKey,"LogPath",0,REG_SZ,(BYTE *)LogPath,strlen(LogPath)+1);
            RegSetValueEx(hKey,"ShowInDialog",0,REG_DWORD,(BYTE *)&ShowInDialog,sizeof(ShowInDialog));
            RegSetValueEx(hKey,"ShowInPanels",0,REG_DWORD,(BYTE *)&ShowInPanels,sizeof(ShowInPanels));
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
