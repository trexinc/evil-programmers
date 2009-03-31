/*
    bcopy.cpp
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
#include <stdlib.h>
#include <ctype.h>
#include "far_helper.h"
#include "farkeys.hpp"
#include "memory.h"
#include "bcplugin.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
TCHAR PluginRootKey[80];
BOOL IsOldFAR=TRUE;

Options Opt={4,FALSE,TRUE,TRUE,0,TRUE,FALSE,1,TRUE,0,0};
PlugOptions PlgOpt={1,SHOW_IN_VIEWER|SHOW_IN_EDITOR|SHOW_IN_CONFIG|SHOW_IN_DIALOG,0,1,TRUE,TRUE,TRUE,TRUE,FALSE,FALSE,FALSE,0,FALSE,FALSE,FALSE,250,FALSE};

static BOOL CheckPipeEx(void);

enum
{
  COPYDLG_BORDER=0,
  COPYDLG_LTO,
  COPYDLG_ETO,
  COPYDLG_SEP1,
  COPYDLG_OVERWRITE,
  COPYDLG_APPEND,
  COPYDLG_SKIP,
  COPYDLG_REFRESH,
  COPYDLG_ASK,
  COPYDLG_REMOVE,
  COPYDLG_ACCESS,
  COPYDLG_ROSRC,
  COPYDLG_RODST,
  COPYDLG_ABORT,
  COPYDLG_LINK,
  COPYDLG_SEP2,
  COPYDLG_OK,
  COPYDLG_CANCEL
};

#include "bcopy_eject.cpp"
#include "bcplugdialogproc.cpp"
#include "bcconfig.cpp"

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>(int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
#ifndef UNICODE
  if(Info->StructSize>FAR165_INFO_SIZE)
#endif
  {
    IsOldFAR=FALSE;
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;

    _tcscpy(PluginRootKey,Info->RootKey);
    _tcscat(PluginRootKey,_T("\\BCopy"));

    HKEY hKey;
    DWORD Type,DataSize=0;
    if((RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
    {
      //tech options
      DataSize=sizeof(PlgOpt.AutoStart);
      RegQueryValueEx(hKey,_T("TechAutoStart"),0,&Type,(LPBYTE)&PlgOpt.AutoStart,&DataSize);
      DataSize=sizeof(PlgOpt.ShowMenu);
      RegQueryValueEx(hKey,_T("TechShowMenu"),0,&Type,(LPBYTE)&PlgOpt.ShowMenu,&DataSize);
      DataSize=sizeof(PlgOpt.InfoMenu);
      RegQueryValueEx(hKey,_T("TechInfoMenu"),0,&Type,(LPBYTE)&PlgOpt.InfoMenu,&DataSize);
      DataSize=sizeof(PlgOpt.ExpandVars);
      RegQueryValueEx(hKey,_T("TechExpandVars"),0,&Type,(LPBYTE)&PlgOpt.ExpandVars,&DataSize);
      DataSize=sizeof(PlgOpt.CheckPassive);
      RegQueryValueEx(hKey,_T("TechCheckPassive"),0,&Type,(LPBYTE)&PlgOpt.CheckPassive,&DataSize);
      DataSize=sizeof(PlgOpt.FormatSize);
      RegQueryValueEx(hKey,_T("TechFormatSize"),0,&Type,(LPBYTE)&PlgOpt.FormatSize,&DataSize);
      DataSize=sizeof(PlgOpt.AutoShowInfo);
      RegQueryValueEx(hKey,_T("TechAutoShowInfo"),0,&Type,(LPBYTE)&PlgOpt.AutoShowInfo,&DataSize);
      DataSize=sizeof(PlgOpt.ConfirmAbort);
      RegQueryValueEx(hKey,_T("TechConfirmAbort"),0,&Type,(LPBYTE)&PlgOpt.ConfirmAbort,&DataSize);
      DataSize=sizeof(PlgOpt.CurrentTime);
      RegQueryValueEx(hKey,_T("TechCurrentTime"),0,&Type,(LPBYTE)&PlgOpt.CurrentTime,&DataSize);
      DataSize=sizeof(PlgOpt.ErrorColor);
      RegQueryValueEx(hKey,_T("TechErrorColor"),0,&Type,(LPBYTE)&PlgOpt.ErrorColor,&DataSize);
      DataSize=sizeof(PlgOpt.AutoRefresh);
      RegQueryValueEx(hKey,_T("TechAutoRefresh"),0,&Type,(LPBYTE)&PlgOpt.AutoRefresh,&DataSize);
      DataSize=sizeof(PlgOpt.ForceEject);
      RegQueryValueEx(hKey,_T("TechForceEject"),0,&Type,(LPBYTE)&PlgOpt.ForceEject,&DataSize);
      DataSize=sizeof(PlgOpt.IgnoreButton);
      RegQueryValueEx(hKey,_T("TechIgnoreButton"),0,&Type,(LPBYTE)&PlgOpt.IgnoreButton,&DataSize);
      DataSize=sizeof(PlgOpt.ResolveDestination);
      RegQueryValueEx(hKey,_T("TechResolveDestination"),0,&Type,(LPBYTE)&PlgOpt.ResolveDestination,&DataSize);
      DataSize=sizeof(PlgOpt.ShowUnicode);
      RegQueryValueEx(hKey,_T("TechShowUnicode"),0,&Type,(LPBYTE)&PlgOpt.ShowUnicode,&DataSize);
      DataSize=sizeof(PlgOpt.RefreshInterval);
      RegQueryValueEx(hKey,_T("TechRefreshInterval"),0,&Type,(LPBYTE)&PlgOpt.RefreshInterval,&DataSize);
      DataSize=sizeof(PlgOpt.Preload);
      RegQueryValueEx(hKey,_T("TechPreload"),0,&Type,(LPBYTE)&PlgOpt.Preload,&DataSize);
      //copy options
      DataSize=sizeof(Opt.CopyType);
      RegQueryValueEx(hKey,_T("CopyType"),0,&Type,(LPBYTE)&Opt.CopyType,&DataSize);
      DataSize=sizeof(Opt.CopyHistory);
      RegQueryValueEx(hKey,_T("CopyHistory"),0,&Type,(LPBYTE)&Opt.CopyHistory,&DataSize);
      DataSize=sizeof(Opt.CopyROSrc);
      RegQueryValueEx(hKey,_T("CopyROSrc"),0,&Type,(LPBYTE)&Opt.CopyROSrc,&DataSize);
      DataSize=sizeof(Opt.CopyRODest);
      RegQueryValueEx(hKey,_T("CopyRODest"),0,&Type,(LPBYTE)&Opt.CopyRODest,&DataSize);
      DataSize=sizeof(Opt.CopyAbort);
      RegQueryValueEx(hKey,_T("CopyAbort"),0,&Type,(LPBYTE)&Opt.CopyAbort,&DataSize);
      DataSize=sizeof(Opt.CopyFullInfo);
      RegQueryValueEx(hKey,_T("CopyFullInfo"),0,&Type,(LPBYTE)&Opt.CopyFullInfo,&DataSize);
      DataSize=sizeof(Opt.CopyAccess);
      RegQueryValueEx(hKey,_T("CopyAccess"),0,&Type,(LPBYTE)&Opt.CopyAccess,&DataSize);
      DataSize=sizeof(Opt.CopyLink);
      RegQueryValueEx(hKey,_T("CopyLink"),0,&Type,(LPBYTE)&Opt.CopyLink,&DataSize);
      //delete options
      DataSize=sizeof(Opt.DeleteRO);
      RegQueryValueEx(hKey,_T("DeleteRO"),0,&Type,(LPBYTE)&Opt.DeleteRO,&DataSize);
      DataSize=sizeof(Opt.DeleteAbort);
      RegQueryValueEx(hKey,_T("DeleteAbort"),0,&Type,(LPBYTE)&Opt.DeleteAbort,&DataSize);
      DataSize=sizeof(Opt.WipeAbort);
      RegQueryValueEx(hKey,_T("WipeAbort"),0,&Type,(LPBYTE)&Opt.WipeAbort,&DataSize);
      RegCloseKey(hKey);
    }
    if((Opt.CopyType>4)||(Opt.CopyType<0))
      Opt.CopyType=2;
    load_macros(PluginRootKey); //FIXME: remove after far release
    load_macros_2(PluginRootKey);
  }
}

void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
  if(!IsOldFAR)
  {
    Info->StructSize=sizeof(*Info);
    Info->Flags=0;
    if(PlgOpt.ShowMenu&SHOW_IN_VIEWER)
      Info->Flags|=PF_VIEWER;
    if(PlgOpt.ShowMenu&SHOW_IN_EDITOR)
      Info->Flags|=PF_EDITOR;
    if(PlgOpt.ShowMenu&SHOW_IN_DIALOG)
      Info->Flags|=PF_DIALOG;
    if(PlgOpt.Preload)
      Info->Flags|=PF_PRELOAD;
    Info->DiskMenuStringsNumber=0;
    static const TCHAR *PluginMenuStrings[1];
    PluginMenuStrings[0]=GetMsg(mName);
    Info->PluginMenuStrings=PluginMenuStrings;
    Info->PluginMenuStringsNumber=sizeofa(PluginMenuStrings);
    Info->PluginConfigStrings=PluginMenuStrings;
    Info->PluginConfigStringsNumber=0;
    if(PlgOpt.ShowMenu&SHOW_IN_CONFIG)
      Info->PluginConfigStringsNumber=sizeofa(PluginMenuStrings);
  }
}

#ifndef UNICODE
int WINAPI EXP_NAME(GetMinFarVersion)(void)
{
  return MAKEFARVERSION(1,70,1527);
}
#endif

static void SendToPipe(DWORD *send,DWORD sendsize,SmallInfoRec *RetData)
{
  HANDLE hPipe=CreateFile(PIPE_NAMEP,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
  if(hPipe!=INVALID_HANDLE_VALUE)
  {
    DWORD transfered;
    if(WriteFile(hPipe,send,sendsize,&transfered,NULL))
    {
      if(!ReadFile(hPipe,RetData,sizeof(SmallInfoRec),&transfered,NULL))
        ShowError(mErrorReadPipe,true);
    }
    else
      ShowError(mErrorWritePipe,true);
    CloseHandle(hPipe);
  }
}

static BOOL CheckPipe(void)
{
  HANDLE hPipe=CreateFile(PIPE_NAMEP,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
  if(hPipe!=INVALID_HANDLE_VALUE)
  {
    CloseHandle(hPipe);
    return TRUE;
  }
  return FALSE;
}

static BOOL CheckPipeEx(void)
{
  BOOL Result=CheckPipe();
  if(!Result)
  {
    if(PlgOpt.AutoStart)
    {
      SC_HANDLE ServiceControlHandle;
      SC_HANDLE SCManagerHandle;
      SERVICE_STATUS ServiceStatus;

      SCManagerHandle=OpenSCManager(NULL,NULL,STANDARD_RIGHTS_READ);
      if(SCManagerHandle>0)
      {
        ServiceControlHandle=OpenService(SCManagerHandle,SVC_NAMEP,SERVICE_QUERY_STATUS|SERVICE_START);
        if(ServiceControlHandle>0)
        {
          if(StartService(ServiceControlHandle,0,NULL))
          {
            while(QueryServiceStatus(ServiceControlHandle,&ServiceStatus))
            {
              if(ServiceStatus.dwCurrentState==SERVICE_START_PENDING)
                Sleep(200);
              else
                break;
            }
            if(ServiceStatus.dwCurrentState==SERVICE_RUNNING)
              Result=CheckPipe();
          }
          CloseServiceHandle(ServiceControlHandle);
        }
        CloseServiceHandle(SCManagerHandle);
      }
    }
  }
  return Result;
}

static bool GetClearReadOnly(void)
{
  return (Info.AdvControl(Info.ModuleNumber,ACTL_GETSYSTEMSETTINGS,NULL)&FSS_CLEARROATTRIBUTE);
}

static int NumberType(int num)
{
  int Result=0,mod;
  mod=num%100;
  if((mod>20)||(mod<5))
  {
    mod=num%10;
    if((mod<5)&&(mod>0)) Result--;
    if(mod==1) Result--;
  }
  return Result;
}

#ifdef UNICODE
static const TCHAR *GetRealName(const FAR_FIND_DATA *src)
{
  return src->lpwszFileName;
}
#else
static const TCHAR *GetRealName(const FAR_FIND_DATA *src)
{
  WIN32_FIND_DATAA find,find_ok; HANDLE hFind; BOOL Res;
  hFind=FindFirstFileA(src->cFileName,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    memcpy(&find_ok,&find,sizeof(find));
    Res=FindNextFileA(hFind,&find);
    FindClose(hFind);
    if((!Res)&&(!_tcsicmp(src->cAlternateFileName,find_ok.cAlternateFileName)))
    {
      return src->cFileName;
    }
  }
  if(src->cAlternateFileName[0])
  {
    hFind=FindFirstFileA(src->cAlternateFileName,&find);
    if(hFind!=INVALID_HANDLE_VALUE)
    {
      FindClose(hFind);
      return src->cAlternateFileName;
    }
  }
  return NULL;
}

static bool GetWideNameDirect(const TCHAR *Root,const TCHAR *src,wchar_t *dest)
{
  TCHAR FileNameA[MAX_PATH]; wchar_t FileNameW[MAX_PATH]; WIN32_FIND_DATAW find;
  _tcscpy(FileNameA,Root);
  _tcscat(FileNameA,src);
  MultiByteToWideChar(CP_OEMCP,0,FileNameA,-1,FileNameW,MAX_PATH);
  HANDLE hFind=FindFirstFileW(FileNameW,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
    wcscpy(dest,find.cFileName);
  } else return false;
  return true;
}

static bool GetWideName(const TCHAR *Root,const FAR_FIND_DATA *src,wchar_t *dest)
{
  const TCHAR *RealFileName=GetRealName(src);
  if(!RealFileName) return false;
  return GetWideNameDirect(Root,RealFileName,dest);
}
#endif

static BOOL CheckPaths(wchar_t *Src,wchar_t *Dest,BOOL Trim)
{
  if(Trim)
    return !_wcsnicmp(Src,Dest,wcslen(Src));
  else
    return !_wcsicmp(Src,Dest);
}

static bool CheckSystemFile(TCHAR *name)
{
  if(name[_tcslen(name)-1]=='\\') return false;
  HANDLE handle=CreateFile(name,0,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  if(handle!=INVALID_HANDLE_VALUE)
  {
    CloseHandle(handle);
    TCHAR path[MAX_PATH];
    _tcscpy(path,name);
    _tcscat(path,_T("\\"));
    if(GetDriveType(path)<2) return true;
  }
  return false;
}

HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom,int Item)
{
  (void)Item;
  int MenuCode=-1;
  FarMenuItem MenuItems[mMenuInfo-mMenuCopy+1];
#ifdef UNICODE
  TCHAR ItemText[3][128];
#endif
  unsigned int MenuItemsSize=mMenuInfo-mMenuCopy+1;
  memset(MenuItems,0,sizeof(MenuItems));
  int Msgs[]={mMenuCopy,mMenuMove,mMenuDelete,mMenuAttr,mMenuWipe,mMenuRun,mMenuSep1,mMenuView,mMenuEdit,mMenuName,mMenuSep2,mMenuEject,mMenuReject,mMenuRefreshSCSI,mMenuSep3,mMenuConfig,mMenuInfo};
  if(OpenFrom==OPEN_DIALOG)
  {
    ShowInfoMenu();
    return INVALID_HANDLE_VALUE;
  }
  else if(OpenFrom!=OPEN_PLUGINSMENU)
  {
    MenuItemsSize=mMenuInfo-mMenuEdit;
    memcpy(Msgs,Msgs+mMenuName-mMenuCopy,sizeof(Msgs[0])*MenuItemsSize);
  }
  for(unsigned int i=0;i<MenuItemsSize;i++)
  {
    MenuItems[i].Selected=MenuItems[i].Checked=MenuItems[i].Separator=0;
#ifdef UNICODE
    _tcscpy(ItemText[i],GetMsg(Msgs[i]));
    MenuItems[i].Text=ItemText[i];
#else
    _tcscpy(MenuItems[i].Text,GetMsg(Msgs[i]));
#endif
    if(MenuItems[i].Text[0]=='-') MenuItems[i].Separator=TRUE;
  };

  // First item is selected
  MenuItems[0].Selected=TRUE;
  // Show menu
  MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_WRAPMODE,GetMsg(mName),NULL,_T("Contents"),NULL,NULL,MenuItems,MenuItemsSize);
  if((OpenFrom!=OPEN_PLUGINSMENU)&&(MenuCode>=0))
    MenuCode+=mMenuName-mMenuCopy;
  //prepare source dir
  CFarPanel pInfo(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO);
  if(!pInfo.IsOk())
  {
    ShowError(mErrorList,false);
    return INVALID_HANDLE_VALUE;
  }

  if((MenuCode<(mMenuSep2-mMenuCopy))&&(MenuCode>=0))
  {
    bool NoDots=true;
    if(pInfo.SelectedItemsNumber()==1&&!_tcscmp(pInfo.Selected(0).FindData.PANEL_FILENAME,_T(".."))) NoDots=false;
    if(NoDots&&(pInfo.SelectedItemsNumber()>0||MenuCode==(mMenuName-mMenuCopy))&&(!pInfo.Plugin()||(pInfo.Flags()&PFLAGS_REALNAMES)))
    {
      bool bcopy_flags[BCOPY_FLAG_COUNT];
      for(int i=0;i<BCOPY_FLAG_COUNT;i++) bcopy_flags[i]=false;
      TCHAR SrcA[MAX_PATH]; wchar_t SrcW[MAX_PATH];
      if(!pInfo.Plugin())
      {
        _tcscpy(SrcA,pInfo.CurDir());
        if(MenuCode!=5) UNCPath(SrcA); //FIXME: named const
        FSF.AddEndSlash(SrcA);
#ifdef UNICODE
        _tcsncpy(SrcW,SrcA,MAX_PATH-1);
        SrcW[MAX_PATH-1]=0;
#else
        MultiByteToWideChar(CP_OEMCP,0,SrcA,-1,SrcW,MAX_PATH);
#endif
      }
      else
      {
        SrcA[0]=0;
        SrcW[0]=0;
      }

      DWORD *send=NULL;
      DWORD sendsize=sizeof(DWORD)*3+sizeof(FileRec)*(pInfo.SelectedItemsNumber()+2);
      if(MenuCode<5)
      {
        send=(DWORD *)malloc(sendsize);
        if(!send) ShowError(mErrorMemory,false);
      }
      SmallInfoRec RetData;
      switch(MenuCode)
      {
        case 0:
        case 1:
        {
          if(!send) break;
          if(!CheckPipeEx()) break;
          CFarPanel pAnotherInfo(PANEL_PASSIVE,FCTL_GETANOTHERPANELSHORTINFO);
          if(PlgOpt.CheckPassive&&pAnotherInfo.Plugin())
            break;

          TCHAR DestA[MAX_PATH]; wchar_t DestW[MAX_PATH];
          if(pAnotherInfo.Plugin())
            _tcscpy(DestA,_T(""));
          else
          {
            _tcscpy(DestA,pAnotherInfo.CurDir());
            //Display "virtual" path for user
            if(PlgOpt.ResolveDestination) UNCPath(DestA);
            FSF.AddEndSlash(DestA);
          }

          //Show dialog
          /*
            0000000000111111111122222222223333333333444444444455555555556666666666777777
            0123456789012345678901234567890123456789012345678901234567890123456789012345
          00                                                                            00
          01   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ Copy ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»   01
          02   º Copy "bcopy.cpp" to                                                º   02
          03   º \WINNT\Profiles\Administrator\Personal\!Far_plugs\bc\doc\reversi\ |º   03
          04   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   04
          05   º ( ) Overwrite all existing files                                   º   05
          06   º ( ) Append to all existing files                                   º   06
          07   º ( ) Skip all existing files                                        º   07
          08   º ( ) Refresh old files                                              º   08
          09   º                                                                    º   09
          10   º [ ] Remove source files                                            º   10
          11   º [ ] Copy access rights                                             º   11
          12   º [ ] Clear R/O attribute on &source                                 º   12
          13   º [ ] Clear R/O attribute on &destination                            º   13
          14   º [ ] Abort on error                                                 º   14
          15   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   15
          16   º                 [ Copy ]                [ Cancel ]                 º   16
          17   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   17
          18                                                                            18
            0000000000111111111122222222223333333333444444444455555555556666666666777777
            0123456789012345678901234567890123456789012345678901234567890123456789012345
          */
          const TCHAR *CopyHistoryName=_T("Copy");
          static const TCHAR *BCopyCopyHistoryName=_T("BCopyCopy");
          static struct InitDialogItem InitDlg[]={
          /* 0*/  {DI_DOUBLEBOX,3,1,72,19,0,0,0,0,(TCHAR *)mCpyDlgCopyTitle},
          /* 1*/  {DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
          /* 2*/  {DI_EDIT,5,3,70,0,1,(DWORD_PTR)BCopyCopyHistoryName,DIF_HISTORY,0,_T("")},
          /* 3*/  {DI_TEXT,5,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
          /* 4*/  {DI_RADIOBUTTON,5,5,0,0,0,0,DIF_GROUP,0,(TCHAR *)mCpyDlgOpt1},
          /* 5*/  {DI_RADIOBUTTON,5,6,0,0,0,0,0,0,(TCHAR *)mCpyDlgOpt2},
          /* 6*/  {DI_RADIOBUTTON,5,7,0,0,0,0,0,0,(TCHAR *)mCpyDlgOpt3},
          /* 7*/  {DI_RADIOBUTTON,5,8,0,0,0,0,0,0,(TCHAR *)mCpyDlgOpt4},
          /* 8*/  {DI_RADIOBUTTON,5,9,0,0,0,0,0,0,(TCHAR *)mCpyDlgAsk},
          /* 9*/  {DI_CHECKBOX,5,11,0,0,0,0,0,0,(TCHAR *)mCpyDlgChk1},
          /*10*/  {DI_CHECKBOX,5,12,0,0,0,0,0,0,(TCHAR *)mCpyDlgChkAccess},
          /*11*/  {DI_CHECKBOX,5,13,0,0,0,0,DIF_DISABLE,0,(TCHAR *)mCpyDlgChk2},
          /*12*/  {DI_CHECKBOX,5,14,0,0,0,0,DIF_DISABLE,0,(TCHAR *)mCpyDlgChk3},
          /*13*/  {DI_CHECKBOX,5,15,0,0,0,0,DIF_3STATE,0,(TCHAR *)mCpyDlgChk4},
          /*14*/  {DI_CHECKBOX,5,16,0,0,0,0,DIF_3STATE,0,(TCHAR *)mCpyDlgChkLink},
          /*15*/  {DI_TEXT,5,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
          /*16*/  {DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)mCpyDlgOkCopy},
          /*17*/  {DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)mCpyDlgCancel},
          };

          struct FarDialogItem DialogItems[sizeofa(InitDlg)];
          InitDialogItems(InitDlg,DialogItems,sizeofa(InitDlg));
          if(Opt.CopyHistory)
            DialogItems[2].History=CopyHistoryName;
          DialogItems[COPYDLG_OVERWRITE+Opt.CopyType].Selected=1;
          DialogItems[COPYDLG_REMOVE].Selected=MenuCode;
          DialogItems[COPYDLG_ACCESS].Selected=Opt.CopyAccess;
          DialogItems[COPYDLG_ROSRC].Selected=Opt.CopyROSrc;
          DialogItems[COPYDLG_RODST].Selected=Opt.CopyRODest;
          DialogItems[COPYDLG_ABORT].Selected=Opt.CopyAbort;
          DialogItems[COPYDLG_LINK].Selected=Opt.CopyLink;
          CopyDialogData dialog_data;
          dialog_data.SrcRO=Opt.CopyROSrc;
          dialog_data.DstRO=Opt.CopyRODest;
          dialog_data.Macro=TRUE;
          if(pInfo.SelectedItemsNumber()>1)
          {
            _stprintf(dialog_data.CopyMessage,GetMsg(mCpyDlgCopyToN+NumberType(pInfo.SelectedItemsNumber())),pInfo.SelectedItemsNumber());
            _stprintf(dialog_data.MoveMessage,GetMsg(mCpyDlgMoveToN+NumberType(pInfo.SelectedItemsNumber())),pInfo.SelectedItemsNumber());
          }
          else
          {
            NormalizeName(InitDlg[0].X2-InitDlg[0].X1-3,mCpyDlgCopyTo1,pInfo.Selected(0).FindData.PANEL_FILENAME,dialog_data.CopyMessage);
            NormalizeName(InitDlg[0].X2-InitDlg[0].X1-3,mCpyDlgMoveTo1,pInfo.Selected(0).FindData.PANEL_FILENAME,dialog_data.MoveMessage);
          }
          INIT_DLG_DATA(DialogItems[COPYDLG_LTO],dialog_data.CopyMessage);
          INIT_DLG_DATA(DialogItems[COPYDLG_ETO],DestA);
          CFarDialog dialog;
          int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,76,21,_T("Copy"),DialogItems,sizeofa(InitDlg),0,0,CopyDialogProc,(LONG_PTR)&dialog_data);
          if(DlgCode==COPYDLG_OK)
          {
            TCHAR eto[MAX_PATH];
            _tcscpy(eto,dialog.Str(COPYDLG_ETO));
            FSF.Unquote(eto);
            if(PlgOpt.ExpandVars)
            {
              TCHAR exp[MAX_PATH]; DWORD res;
#ifdef UNICODE
              res=ExpandEnvironmentStrings(eto,exp,ArraySize(exp));
#else
              res=FSF.ExpandEnvironmentStr(eto,exp,ArraySize(exp));
#endif
              _tcscpy(eto,exp);
            }
            TCHAR *filename;
            filename=_tcsrchr(eto,'\\');
            if(!filename)
              filename=eto;
            if(!(_tcscmp(filename,_T("."))&&_tcscmp(filename,_T(".."))))
              _tcscat(eto,_T("\\"));
            DWORD full_res=GetFullPathName(eto,ArraySize(DestA),DestA,&filename);
            if(!full_res||full_res>=ArraySize(DestA))
            {
              TCHAR err3[512];
              _stprintf(err3,GetMsg(mCpyErr3),eto);
              const TCHAR *MsgItems[]={GetMsg(mError),err3,GetMsg(mOk)};
              Info.Message(Info.ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,sizeofa(MsgItems),1);
              break;
            }
            UNCPath(DestA);
            //check for such names as \\ZG\TEMP
            if(!CheckSystemFile(DestA))
            {
              TCHAR Volume[MAX_PATH];
              FSF.GetPathRoot(DestA,Volume
#ifdef UNICODE
              ,MAX_PATH
#endif
              );
              if(_tcslen(DestA)<_tcslen(Volume))
                FSF.AddEndSlash(DestA);
            }
            if(DestA[_tcslen(DestA)-1]!='\\')
            {
              WIN32_FIND_DATA find;
              HANDLE test_handle;
              if((test_handle=FindFirstFile(DestA,&find))!=INVALID_HANDLE_VALUE)
              {
                FindClose(test_handle);
                if(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
                  _tcscat(DestA,_T("\\"));
              }
              else if((!CheckSystemFile(DestA))&&(pInfo.SelectedItemsNumber()==1)&&(!(pInfo.Selected(0).Flags&PPIF_SELECTED))&&(pInfo.Selected(0).FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
              {
                bcopy_flags[BCOPY_REN_COPY]=true;
                _tcscat(DestA,_T("\\"));
              }
            }
#ifdef UNICODE
            _tcsncpy(DestW,DestA,MAX_PATH-1);
            DestW[MAX_PATH-1]=0;
#else
            MultiByteToWideChar(CP_OEMCP,0,DestA,-1,DestW,MAX_PATH);
#endif
            if(CheckPaths(SrcW,DestW,FALSE))
              goto copyexit_err1;
            send[0]=OPERATION_COPY;
            /*fill flags*/
            send[1]=0;
            if(dialog.Check(COPYDLG_OVERWRITE))
              send[1]|=COPYFLAG_OVERWRITE;
            if(dialog.Check(COPYDLG_APPEND))
              send[1]|=COPYFLAG_APPEND;
            if(dialog.Check(COPYDLG_SKIP))
              send[1]|=COPYFLAG_SKIP;
            if(dialog.Check(COPYDLG_REFRESH))
              send[1]|=COPYFLAG_REFRESH;
            if(dialog.Check(COPYDLG_ASK))
              send[1]|=COPYFLAG_ASK;
            if(dialog.Check(COPYDLG_REMOVE))
            {
              send[1]|=COPYFLAG_MOVE;
              bcopy_flags[BCOPY_MOVE]=true;
              //check same drive
              TCHAR VolumeSrc[MAX_PATH],VolumeDest[MAX_PATH];
              FSF.GetPathRoot(SrcA,VolumeSrc
#ifdef UNICODE
              ,MAX_PATH
#endif
              );
              FSF.GetPathRoot(DestA,VolumeDest
#ifdef UNICODE
              ,MAX_PATH
#endif
              );
              if(!_tcsicmp(VolumeSrc,VolumeDest))
                bcopy_flags[BCOPY_NOREAL_COPY]=true;
            }
            if(dialog.Check(COPYDLG_ACCESS))
              send[1]|=COPYFLAG_ACCESS;
            if(dialog.Check(COPYDLG_ROSRC))
              send[1]|=COPYFLAG_ROSRC;
            if(dialog.Check(COPYDLG_RODST))
              send[1]|=COPYFLAG_RODST;
            switch(dialog.Check(COPYDLG_ABORT))
            {
              case BSTATE_CHECKED:
                send[1]|=COPYFLAG_FAIL;
                break;
              case BSTATE_3STATE:
                send[1]|=COPYFLAG_FAIL_ASK|COPYFLAG_DONTLOGERRORS;
                break;
            }
            switch(dialog.Check(COPYDLG_LINK))
            {
              case BSTATE_UNCHECKED:
                send[1]=SET_LINK_TYPE(send[1],LINK_TYPE_LINK);
                break;
              case BSTATE_CHECKED:
                send[1]=SET_LINK_TYPE(send[1],LINK_TYPE_COPY);
                break;
              case BSTATE_3STATE:
                send[1]=SET_LINK_TYPE(send[1],LINK_TYPE_ASK);
                break;
            }
            if(Opt.CopyFullInfo&&(!bcopy_flags[BCOPY_NOREAL_COPY]))
              send[1]|=COPYFLAG_STATISTIC;
            if(GetClearReadOnly())
            { // clear read-only attribute, when copying from CD
              TCHAR Volume[MAX_PATH],VolumeName[MAX_PATH],VolumeFS[MAX_PATH];
              DWORD VolumeSerialNumber,MaximumComponentLength,FileSystemFlags;
              FSF.GetPathRoot(SrcA,Volume
#ifdef UNICODE
              ,MAX_PATH
#endif
              );
              _tcscpy(VolumeFS,_T("    "));
              GetVolumeInformation(Volume,VolumeName,ArraySize(VolumeName),&VolumeSerialNumber,&MaximumComponentLength,&FileSystemFlags,VolumeFS,ArraySize(VolumeFS));
              if((GetDriveType(Volume)==DRIVE_CDROM)||(!_tcsicmp(VolumeFS,_T("CDFS"))))
              {
                send[1]|=COPYFLAG_ATTR;
                send[1]|=COPYFLAG_CLEAR_READONLY;
              }
            }
            send[2]=pInfo.SelectedItemsNumber()+2;
            //create destination dirs
            {
              if(bcopy_flags[BCOPY_REN_COPY]&&bcopy_flags[BCOPY_NOREAL_COPY]&&bcopy_flags[BCOPY_MOVE])
              { //don't create renamed dir
                int last_char_index=(int)_tcslen(DestA)-1;
                if(DestA[last_char_index]=='\\')
                  DestA[last_char_index]=0;
              }
              TCHAR DirA[MAX_PATH],*DirEnd;
              if((_tcslen(DestA)>2)&&(!CheckSystemFile(DestA)))
              {
                BOOL CreateFlag=FALSE; TCHAR CreateDest[MAX_PATH];
                DirEnd=DestA+3;
                do
                {
                  DirEnd=_tcschr(DirEnd,'\\');
                  if(DirEnd)
                  {
                    _tcsncpy(DirA,DestA,DirEnd-DestA); DirA[DirEnd-DestA]=0;
                    DirEnd++;
                    CreateDirectory(DirA,NULL);
                    CreateFlag=TRUE;
                  }
                } while(DirEnd);
                if(CreateFlag)
                {
                  _tcscpy(CreateDest,DirA);
                  _tcscat(CreateDest,_T("\\*"));
                  WIN32_FIND_DATA find;
                  SetLastError(0);
                  HANDLE hFind=FindFirstFile(CreateDest,&find);
                  DWORD err=GetLastError();
                  if(hFind!=INVALID_HANDLE_VALUE)
                    FindClose(hFind);
                  if(err==ERROR_PATH_NOT_FOUND)
                  {
                    TCHAR err2[512];
                    _stprintf(err2,GetMsg(mCpyErr2),DirA);
                    const TCHAR *MsgItems[]={GetMsg(mError),err2,GetMsg(mOk)};
                    Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeofa(MsgItems),1);
                    goto copyexit;
                  }
                }
              }
            }
            #define CHECK_DEST if(CheckPaths(TestSrcW,DestW,TRUE)) goto copyexit_err1;
            #define DEST_W DestW
            #define ON_ERROR goto copyexit;
            #include "bcopy_find.cpp"
          }
          break;
copyexit_err1:
          ShowError(mCpyErr1,false);
copyexit:
          break;
        }
        case 2:
        {
          if(!send) break;
          if(!CheckPipeEx()) break;
          //Show dialog
          /*
            000000000011111111112222222222333333333344444444445555555555666666
            012345678901234567890123456789012345678901234567890123456789012345
          00                                                                  00
          01   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ Copy »   01
          02   º Copy "bcopy.cpp" to                                      º   02
          03   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   03
          04   º [ ] Clear R/O attribute                                  º   04
          05   º [ ] Abort on error                                       º   05
          06   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   06
          07   º [ Delete ] [ Cancel ]                                    º   07
          08   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   08
          09                                                                  09
            000000000011111111112222222222333333333344444444445555555555666666
            012345678901234567890123456789012345678901234567890123456789012345
          */
          static struct InitDialogItem InitDlg[]={
          /* 0*/  {DI_DOUBLEBOX,3,1,62,8,0,0,0,0,(TCHAR *)mDelDlgTitle},
          /* 1*/  {DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
          /* 2*/  {DI_TEXT,5,3,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
          /* 3*/  {DI_CHECKBOX,5,4,0,0,0,0,0,0,(TCHAR *)mDelDlgChk1},
          /* 4*/  {DI_CHECKBOX,5,5,0,0,0,0,DIF_3STATE,0,(TCHAR *)mDelDlgChk2},
          /* 5*/  {DI_TEXT,5,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
          /* 6*/  {DI_BUTTON,0,7,0,0,1,0,DIF_CENTERGROUP,1,(TCHAR *)mDelDlgOk},
          /* 7*/  {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)mDelDlgCancel}
          };
          struct FarDialogItem DialogItems[sizeofa(InitDlg)];
          InitDialogItems(InitDlg,DialogItems,sizeofa(InitDlg));
          DialogItems[3].Selected=Opt.DeleteRO;
          DialogItems[4].Selected=Opt.DeleteAbort;
          TCHAR delete_name[512];
          if(pInfo.SelectedItemsNumber()>1)
            _stprintf(delete_name,GetMsg(mDelDlgDeleteN+NumberType(pInfo.SelectedItemsNumber())),pInfo.SelectedItemsNumber());
          else
            NormalizeName(InitDlg[0].X2-InitDlg[0].X1-3,mDelDlgDelete1,pInfo.Selected(0).FindData.PANEL_FILENAME,delete_name);
          INIT_DLG_DATA(DialogItems[1],delete_name);
          CommonDialogData dialog_data={TRUE};
          CFarDialog dialog;
          int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,66,10,_T("Delete"),DialogItems,sizeofa(InitDlg),0,0,DelDialogProc,(LONG_PTR)&dialog_data);
          if(DlgCode==6)
          {
            send[0]=OPERATION_COPY;
            /*fill flags*/
            send[1]=COPYFLAG_DELETE|COPYFLAG_OVERWRITE;
            if(dialog.Check(3))
              send[1]|=COPYFLAG_ROSRC;
            switch(dialog.Check(4))
            {
              case BSTATE_CHECKED:
                send[1]|=COPYFLAG_FAIL;
                break;
              case BSTATE_3STATE:
                send[1]|=COPYFLAG_FAIL_ASK|COPYFLAG_DONTLOGERRORS;
                break;
            }
            if(Opt.CopyFullInfo) //!!!
              send[1]|=COPYFLAG_STATISTIC;
            send[2]=pInfo.SelectedItemsNumber()+2;
            #define CHECK_DEST
            #define DEST_W L""
            #define ON_ERROR ;
            #include "bcopy_find.cpp"
          }
          break;
        }
        case 3:
        {
          if(!send) break;
          if(!CheckPipeEx()) break;
          //Show dialog
          /*
            000000000011111111112222222222333333333344444
            012345678901234567890123456789012345678901234
          00                                             00
          01   ÉÍÍÍÍÍÍÍÍÍÍÍÍ Attributes ÍÍÍÍÍÍÍÍÍÍÍÍÍ»   01
          02   º     Change file attributes for      º   02
          03   º           file_id_diz.m4            º   03
          04   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   04
          05   º [ ] Read only                       º   05
          06   º [x] Archive                         º   06
          07   º [ ] Hidden                          º   07
          08   º [ ] System                          º   08
          09   º [ ] Compressed                      º   09
          10   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   10
          11   º         [ Set ]  [ Cancel ]         º   11
          12   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   12
          13                                             13
            000000000011111111112222222222333333333344444
            012345678901234567890123456789012345678901234
          */
          static struct InitDialogItem InitDlg[]={
          /* 0*/  {DI_DOUBLEBOX,3,1,41,12,0,0,0,0,(TCHAR *)mAttrDlgTitle},
          /* 1*/  {DI_TEXT,0,3,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
          /* 2*/  {DI_TEXT,0,2,0,0,0,0,0,0,(TCHAR *)mAttrDlgTitle2},
          /* 3*/  {DI_TEXT,5,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
          /* 4*/  {DI_CHECKBOX,5,5,0,0,1,2,DIF_3STATE,0,(TCHAR *)mAttrDlgReadOnly},
          /* 5*/  {DI_CHECKBOX,5,6,0,0,0,2,DIF_3STATE,0,(TCHAR *)mAttrDlgArchive},
          /* 6*/  {DI_CHECKBOX,5,7,0,0,0,2,DIF_3STATE,0,(TCHAR *)mAttrDlgHidden},
          /* 7*/  {DI_CHECKBOX,5,8,0,0,0,2,DIF_3STATE,0,(TCHAR *)mAttrDlgSystem},
          /* 8*/  {DI_CHECKBOX,5,9,0,0,0,2,DIF_3STATE|DIF_DISABLE,0,(TCHAR *)mAttrDlgCompressed},
          /* 9*/  {DI_TEXT,5,10,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
          /*10*/  {DI_BUTTON,0,11,0,0,1,0,DIF_CENTERGROUP,1,(TCHAR *)mAttrDlgSet},
          /*11*/  {DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)mAttrDlgCancel}
          };
          struct FarDialogItem DialogItems[sizeofa(InitDlg)];
          InitDialogItems(InitDlg,DialogItems,sizeofa(InitDlg));
          TCHAR attr_name[512];
          if(pInfo.SelectedItemsNumber()>1)
            _stprintf(attr_name,GetMsg(mAttrDlgChangeN+NumberType(pInfo.SelectedItemsNumber())),pInfo.SelectedItemsNumber());
          else
            NormalizeName(InitDlg[0].X2-InitDlg[0].X1-3,mAttrDlgChange1,pInfo.Selected(0).FindData.PANEL_FILENAME,attr_name);
          INIT_DLG_DATA(DialogItems[1],attr_name);
          DialogItems[1].X1=4+(37-(int)_tcslen(DLG_DATA(DialogItems[1])))/2;
          DialogItems[2].X1=4+(37-(int)_tcslen(DLG_DATA(DialogItems[2])))/2;
          { // clear read-only attribute, when copying from CD
            TCHAR Volume[MAX_PATH],VolumeName[MAX_PATH],VolumeFS[MAX_PATH];
            DWORD VolumeSerialNumber,MaximumComponentLength,FileSystemFlags;
            FSF.GetPathRoot(SrcA,Volume
#ifdef UNICODE
            ,MAX_PATH
#endif
            );
            if(GetVolumeInformation(Volume,VolumeName,ArraySize(VolumeName),&VolumeSerialNumber,&MaximumComponentLength,&FileSystemFlags,VolumeFS,ArraySize(VolumeFS)))
              if(FileSystemFlags&FS_FILE_COMPRESSION)
                DialogItems[8].Flags&=~DIF_DISABLE;
          }
          CommonDialogData dialog_data={TRUE};
          CFarDialog dialog;
          int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,45,14,_T("Attributes"),DialogItems,sizeofa(InitDlg),0,0,AttrDialogProc,(LONG_PTR)&dialog_data);
          if(DlgCode==10)
          {
            send[0]=OPERATION_COPY;
            /*fill flags*/
            send[1]=COPYFLAG_ATTRMAIN|COPYFLAG_ATTR|COPYFLAG_OVERWRITE;
            send[1]=SET_LINK_TYPE(send[1],LINK_TYPE_COPY);
            unsigned int flags[5][3]=
            {
              {COPYFLAG_CLEAR_READONLY,COPYFLAG_SET_READONLY,0},
              {COPYFLAG_CLEAR_ARCHIVE,COPYFLAG_SET_ARCHIVE,0},
              {COPYFLAG_CLEAR_HIDDEN,COPYFLAG_SET_HIDDEN,0},
              {COPYFLAG_CLEAR_SYSTEM,COPYFLAG_SET_SYSTEM,0},
              {COPYFLAG_CLEAR_COMPRESSED,COPYFLAG_SET_COMPRESSED,0},
            };
            for(int i=0;i<5;i++)
              send[1]|=flags[i][dialog.Check(4+i)];
            if(send[1]!=(COPYFLAG_ATTRMAIN|COPYFLAG_ATTR|COPYFLAG_OVERWRITE))
            {
              if(Opt.CopyFullInfo) //!!!
                send[1]|=COPYFLAG_STATISTIC;
              send[2]=pInfo.SelectedItemsNumber()+2;
              #define CHECK_DEST
              #define DEST_W L""
              #define ON_ERROR ;
              #include "bcopy_find.cpp"
            }
          }
          break;
        }
        case 4:
        {
          if(!send) break;
          if(!CheckPipeEx()) break;
          //Show dialog
          /*
            000000000011111111112222222222333333333344444444445555555555666666
            012345678901234567890123456789012345678901234567890123456789012345
          00                                                                  00
          01   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ Wipe »   01
          02   º Wipe "bcopy.cpp"                                         º   02
          03   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   03
          04   º [ ] Abort on wipe error                                  º   04
          05   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   05
          06   º [ Wipe ] [ Cancel ]                                      º   06
          07   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   07
          08                                                                  08
            000000000011111111112222222222333333333344444444445555555555666666
            012345678901234567890123456789012345678901234567890123456789012345
          */
          static struct InitDialogItem InitDlg[]={
          /* 0*/  {DI_DOUBLEBOX,3,1,62,7,0,0,0,0,(TCHAR *)mWpeDlgTitle},
          /* 1*/  {DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
          /* 2*/  {DI_TEXT,5,3,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
          /* 3*/  {DI_CHECKBOX,5,4,0,0,0,0,DIF_3STATE,0,(TCHAR *)mWpeDlgChk},
          /* 4*/  {DI_TEXT,5,5,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
          /* 5*/  {DI_BUTTON,0,6,0,0,1,0,DIF_CENTERGROUP,1,(TCHAR *)mWpeDlgOk},
          /* 6*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)mWpeDlgCancel}
          };
          struct FarDialogItem DialogItems[sizeofa(InitDlg)];
          InitDialogItems(InitDlg,DialogItems,sizeofa(InitDlg));
          DialogItems[3].Selected=Opt.WipeAbort;
          TCHAR wipe_name[512];
          if(pInfo.SelectedItemsNumber()>1)
            _stprintf(wipe_name,GetMsg(mWpeDlgWipeN+NumberType(pInfo.SelectedItemsNumber())),pInfo.SelectedItemsNumber());
          else
            NormalizeName(InitDlg[0].X2-InitDlg[0].X1-3,mWpeDlgWipe1,pInfo.Selected(0).FindData.PANEL_FILENAME,wipe_name);
          INIT_DLG_DATA(DialogItems[1],wipe_name);
          CommonDialogData dialog_data={TRUE};
          CFarDialog dialog;
          int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,66,9,_T("Delete"),DialogItems,sizeofa(InitDlg),0,0,WipeDialogProc,(LONG_PTR)&dialog_data);
          if(DlgCode==5)
          {
            send[0]=OPERATION_COPY;
            /*fill flags*/
            send[1]=COPYFLAG_DELETE|COPYFLAG_WIPE|COPYFLAG_OVERWRITE;
            switch(dialog.Check(3))
            {
              case BSTATE_CHECKED:
                send[1]|=COPYFLAG_FAIL;
                break;
              case BSTATE_3STATE:
                send[1]|=COPYFLAG_FAIL_ASK|COPYFLAG_DONTLOGERRORS;
                break;
            }
            if(Opt.CopyFullInfo) //!!!
              send[1]|=COPYFLAG_STATISTIC;
            send[2]=pInfo.SelectedItemsNumber()+2;
            #define CHECK_DEST
            #define DEST_W L""
            #define ON_ERROR ;
            #include "bcopy_find.cpp"
          }
          break;
        }
        case 5:
          if(pInfo.CurrentItem()>=0)
          {
            wchar_t WideName[MAX_PATH],WideFullName[MAX_PATH];
#ifdef UNICODE
            _tcscpy(WideName,pInfo[pInfo.CurrentItem()].FindData.lpwszFileName);
#else
            if(GetWideName(SrcA,&pInfo[pInfo.CurrentItem()].FindData,WideName))
#endif
            {
              wcscpy(WideFullName,SrcW);
              wcscat(WideFullName,WideName);
              SHELLEXECUTEINFOW info;
              memset(&info,0,sizeof(info));
              info.cbSize=sizeof(info);
              info.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_DDEWAIT;
              info.lpFile=WideFullName;
              info.nShow=SW_SHOWNORMAL;
              ShellExecuteExW(&info);
            }
          }
          break;
        case 7:
        case 8:
        {
          if((pInfo.CurrentItem()>=0)&&!(pInfo[pInfo.CurrentItem()].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
          {
            TCHAR SrcLA[MAX_PATH];
#ifndef UNICODE
            TCHAR *ShortFileName=pInfo[pInfo.CurrentItem()].FindData.cAlternateFileName;
#endif
            _tcscpy(SrcLA,SrcA);
#ifdef UNICODE
            _tcscat(SrcA,pInfo[pInfo.CurrentItem()].FindData.PANEL_FILENAME);
#else
            if(!ShortFileName[0]||pInfo.Plugin())
              ShortFileName=pInfo[pInfo.CurrentItem()].FindData.PANEL_FILENAME;
            _tcscat(SrcA,ShortFileName);
#endif
            _tcscat(SrcLA,pInfo[pInfo.CurrentItem()].FindData.PANEL_FILENAME);
            if(MenuCode==7)
              Info.Viewer(SrcA,SrcLA,0,0,-1,-1,VF_NONMODAL|VF_IMMEDIATERETURN|VF_ENABLE_F6
#ifdef UNICODE
              ,CP_AUTODETECT
#endif
              );
            else
              Info.Editor(SrcA,SrcLA,0,0,-1,-1,EF_NONMODAL|EF_IMMEDIATERETURN|EF_ENABLE_F6,1,1
#ifdef UNICODE
              ,CP_AUTODETECT
#endif
              );
          }
          break;
        }
        case 9:
          {
            wchar_t WideName[MAX_PATH];
            if(OpenFrom==OPEN_PLUGINSMENU&&pInfo.ItemsNumber()>0)
            {
              if(pInfo.CurrentItem()>=0)
              {
                if(pInfo.Plugin())
                {
#ifdef UNICODE
                  _tcscpy(WideName,pInfo[pInfo.CurrentItem()].FindData.PANEL_FILENAME);
#else
                  if(GetWideNameDirect(SrcA,pInfo[pInfo.CurrentItem()].FindData.PANEL_FILENAME,WideName))
#endif
                    ShowName(WideName);
                }
                else
                {
#ifdef UNICODE
                  _tcscpy(WideName,pInfo[pInfo.CurrentItem()].FindData.PANEL_FILENAME);
#else
                  if(GetWideName(SrcA,&pInfo[pInfo.CurrentItem()].FindData,WideName))
#endif
                    ShowName(WideName);
                }
              }
            }
            else
            {
              WindowInfo winfo;
              winfo.Pos=-1;
              if(Info.AdvControl(Info.ModuleNumber,ACTL_GETWINDOWINFO,&winfo))
              {
#ifdef UNICODE
                  _tcscpy(WideName,winfo.Name);
#else
                if(GetWideNameDirect(_T(""),winfo.Name,WideName))
#endif
                  ShowName(WideName);
              }
            }
          }
          break;
      }
      free(send);
    }
  }
  else
  {
    switch(MenuCode)
    {
      case 11:
      case 12:
      {
        ShowDiskMenu(MenuCode-11);
        break;
      }
      case 13:
        RefreshSCSI();
        break;
      case 15:
        EXP_NAME_CALL(Configure)(0);
        break;
      case 16:
      {
        ShowInfoMenu();
        break;
      }
    }
  }
  return INVALID_HANDLE_VALUE;
}

int WINAPI EXP_NAME(Configure)(int ItemNumber)
{
  switch(ItemNumber)
  {
    case 0:
      return(Config());
  }
  return(FALSE);
}

void WINAPI EXP_NAME(ExitFAR)()
{
  if(!IsOldFAR)
  {
    free_macros(); //FIXME: remove after far release
    free_macros_2();
  }
}

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
