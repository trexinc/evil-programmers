#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../../plugin.hpp"
#include "../../farkeys.hpp"
#include "memory.h"
#include "bcplugin.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
char PluginRootKey[80];
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

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>(int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
  if(Info->StructSize>FAR165_INFO_SIZE)
  {
    IsOldFAR=FALSE;
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;

    strcpy(PluginRootKey,Info->RootKey);
    strcat(PluginRootKey,"\\BCopy");

    HKEY hKey;
    DWORD Type,DataSize=0;
    if((RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
    {
      //tech options
      DataSize=sizeof(PlgOpt.AutoStart);
      RegQueryValueEx(hKey,"TechAutoStart",0,&Type,(LPBYTE)&PlgOpt.AutoStart,&DataSize);
      DataSize=sizeof(PlgOpt.ShowMenu);
      RegQueryValueEx(hKey,"TechShowMenu",0,&Type,(LPBYTE)&PlgOpt.ShowMenu,&DataSize);
      DataSize=sizeof(PlgOpt.InfoMenu);
      RegQueryValueEx(hKey,"TechInfoMenu",0,&Type,(LPBYTE)&PlgOpt.InfoMenu,&DataSize);
      DataSize=sizeof(PlgOpt.ExpandVars);
      RegQueryValueEx(hKey,"TechExpandVars",0,&Type,(LPBYTE)&PlgOpt.ExpandVars,&DataSize);
      DataSize=sizeof(PlgOpt.CheckPassive);
      RegQueryValueEx(hKey,"TechCheckPassive",0,&Type,(LPBYTE)&PlgOpt.CheckPassive,&DataSize);
      DataSize=sizeof(PlgOpt.FormatSize);
      RegQueryValueEx(hKey,"TechFormatSize",0,&Type,(LPBYTE)&PlgOpt.FormatSize,&DataSize);
      DataSize=sizeof(PlgOpt.AutoShowInfo);
      RegQueryValueEx(hKey,"TechAutoShowInfo",0,&Type,(LPBYTE)&PlgOpt.AutoShowInfo,&DataSize);
      DataSize=sizeof(PlgOpt.ConfirmAbort);
      RegQueryValueEx(hKey,"TechConfirmAbort",0,&Type,(LPBYTE)&PlgOpt.ConfirmAbort,&DataSize);
      DataSize=sizeof(PlgOpt.CurrentTime);
      RegQueryValueEx(hKey,"TechCurrentTime",0,&Type,(LPBYTE)&PlgOpt.CurrentTime,&DataSize);
      DataSize=sizeof(PlgOpt.ErrorColor);
      RegQueryValueEx(hKey,"TechErrorColor",0,&Type,(LPBYTE)&PlgOpt.ErrorColor,&DataSize);
      DataSize=sizeof(PlgOpt.AutoRefresh);
      RegQueryValueEx(hKey,"TechAutoRefresh",0,&Type,(LPBYTE)&PlgOpt.AutoRefresh,&DataSize);
      DataSize=sizeof(PlgOpt.ForceEject);
      RegQueryValueEx(hKey,"TechForceEject",0,&Type,(LPBYTE)&PlgOpt.ForceEject,&DataSize);
      DataSize=sizeof(PlgOpt.IgnoreButton);
      RegQueryValueEx(hKey,"TechIgnoreButton",0,&Type,(LPBYTE)&PlgOpt.IgnoreButton,&DataSize);
      DataSize=sizeof(PlgOpt.ResolveDestination);
      RegQueryValueEx(hKey,"TechResolveDestination",0,&Type,(LPBYTE)&PlgOpt.ResolveDestination,&DataSize);
      DataSize=sizeof(PlgOpt.ShowUnicode);
      RegQueryValueEx(hKey,"TechShowUnicode",0,&Type,(LPBYTE)&PlgOpt.ShowUnicode,&DataSize);
      DataSize=sizeof(PlgOpt.RefreshInterval);
      RegQueryValueEx(hKey,"TechRefreshInterval",0,&Type,(LPBYTE)&PlgOpt.RefreshInterval,&DataSize);
      DataSize=sizeof(PlgOpt.Preload);
      RegQueryValueEx(hKey,"TechPreload",0,&Type,(LPBYTE)&PlgOpt.Preload,&DataSize);
      //copy options
      DataSize=sizeof(Opt.CopyType);
      RegQueryValueEx(hKey,"CopyType",0,&Type,(LPBYTE)&Opt.CopyType,&DataSize);
      DataSize=sizeof(Opt.CopyHistory);
      RegQueryValueEx(hKey,"CopyHistory",0,&Type,(LPBYTE)&Opt.CopyHistory,&DataSize);
      DataSize=sizeof(Opt.CopyROSrc);
      RegQueryValueEx(hKey,"CopyROSrc",0,&Type,(LPBYTE)&Opt.CopyROSrc,&DataSize);
      DataSize=sizeof(Opt.CopyRODest);
      RegQueryValueEx(hKey,"CopyRODest",0,&Type,(LPBYTE)&Opt.CopyRODest,&DataSize);
      DataSize=sizeof(Opt.CopyAbort);
      RegQueryValueEx(hKey,"CopyAbort",0,&Type,(LPBYTE)&Opt.CopyAbort,&DataSize);
      DataSize=sizeof(Opt.CopyFullInfo);
      RegQueryValueEx(hKey,"CopyFullInfo",0,&Type,(LPBYTE)&Opt.CopyFullInfo,&DataSize);
      DataSize=sizeof(Opt.CopyAccess);
      RegQueryValueEx(hKey,"CopyAccess",0,&Type,(LPBYTE)&Opt.CopyAccess,&DataSize);
      DataSize=sizeof(Opt.CopyLink);
      RegQueryValueEx(hKey,"CopyLink",0,&Type,(LPBYTE)&Opt.CopyLink,&DataSize);
      //delete options
      DataSize=sizeof(Opt.DeleteRO);
      RegQueryValueEx(hKey,"DeleteRO",0,&Type,(LPBYTE)&Opt.DeleteRO,&DataSize);
      DataSize=sizeof(Opt.DeleteAbort);
      RegQueryValueEx(hKey,"DeleteAbort",0,&Type,(LPBYTE)&Opt.DeleteAbort,&DataSize);
      DataSize=sizeof(Opt.WipeAbort);
      RegQueryValueEx(hKey,"WipeAbort",0,&Type,(LPBYTE)&Opt.WipeAbort,&DataSize);
      RegCloseKey(hKey);
    }
    if((Opt.CopyType>4)||(Opt.CopyType<0))
      Opt.CopyType=2;
    load_macros(PluginRootKey); //FIXME: remove after far release
    load_macros_2(PluginRootKey);
  }
}

void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
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
    static const char *PluginMenuStrings[1];
    PluginMenuStrings[0]=GetMsg(mName);
    Info->PluginMenuStrings=PluginMenuStrings;
    Info->PluginMenuStringsNumber=sizeofa(PluginMenuStrings);
    Info->PluginConfigStrings=PluginMenuStrings;
    Info->PluginConfigStringsNumber=0;
    if(PlgOpt.ShowMenu&SHOW_IN_CONFIG)
      Info->PluginConfigStringsNumber=sizeofa(PluginMenuStrings);
  }
}

int WINAPI _export GetMinFarVersion(void)
{
  return MAKEFARVERSION(1,70,1527);
}

static void SendToPipe(DWORD *send,DWORD sendsize,SmallInfoRec *RetData)
{
  HANDLE hPipe=CreateFile(PIPE_NAME,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
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
  HANDLE hPipe=CreateFile(PIPE_NAME,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
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

      SCManagerHandle=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
      if(SCManagerHandle>0)
      {
        ServiceControlHandle=OpenService(SCManagerHandle,SVC_NAME,SERVICE_ALL_ACCESS);
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

static const char *GetRealName(const FAR_FIND_DATA *src)
{
  WIN32_FIND_DATAA find,find_ok; HANDLE hFind; BOOL Res;
  hFind=FindFirstFileA(src->cFileName,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    memcpy(&find_ok,&find,sizeof(find));
    Res=FindNextFileA(hFind,&find);
    FindClose(hFind);
    if((!Res)&&(!_stricmp(src->cAlternateFileName,find_ok.cAlternateFileName)))
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

static bool GetWideNameDirect(const char *Root,const char *src,wchar_t *dest)
{
  char FileNameA[MAX_PATH]; wchar_t FileNameW[MAX_PATH]; WIN32_FIND_DATAW find;
  strcpy(FileNameA,Root);
  strcat(FileNameA,src);
  MultiByteToWideChar(CP_OEMCP,0,FileNameA,-1,FileNameW,MAX_PATH);
  HANDLE hFind=FindFirstFileW(FileNameW,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
    wcscpy(dest,find.cFileName);
  } else return false;
  return true;
}

static bool GetWideName(const char *Root,const FAR_FIND_DATA *src,wchar_t *dest)
{
  const char *RealFileName=GetRealName(src);
  if(!RealFileName) return false;
  return GetWideNameDirect(Root,RealFileName,dest);
}

static BOOL CheckPaths(wchar_t *Src,wchar_t *Dest,BOOL Trim)
{
  if(Trim)
    return !_wcsnicmp(Src,Dest,wcslen(Src));
  else
    return !_wcsicmp(Src,Dest);
}

static bool CheckSystemFile(char *name)
{
  if(name[strlen(name)-1]=='\\') return false;
  HANDLE handle=CreateFileA(name,0,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  if(handle!=INVALID_HANDLE_VALUE)
  {
    CloseHandle(handle);
    char path[MAX_PATH];
    strcpy(path,name);
    strcat(path,"\\");
    if(GetDriveType(path)<2) return true;
  }
  return false;
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  (void)Item;
  int MenuCode=-1;
  FarMenuItem MenuItems[mMenuInfo-mMenuCopy+1];
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
    strcpy(MenuItems[i].Text,GetMsg(Msgs[i]));
    if(MenuItems[i].Text[0]=='-') MenuItems[i].Separator=TRUE;
  };

  // First item is selected
  MenuItems[0].Selected=TRUE;
  // Show menu
  MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_WRAPMODE,GetMsg(mName),NULL,"Contents",NULL,NULL,MenuItems,MenuItemsSize);
  if((OpenFrom!=OPEN_PLUGINSMENU)&&(MenuCode>=0))
    MenuCode+=mMenuName-mMenuCopy;
  //prepare source dir
  PanelInfo PInfo;
  if(!Info.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&PInfo))
  {
    ShowError(mErrorList,false);
    return INVALID_HANDLE_VALUE;
  }

  if((MenuCode<(mMenuSep2-mMenuCopy))&&(MenuCode>=0))
  {
    bool NoDots=true;
    if(PInfo.SelectedItemsNumber==1&&!strcmp(PInfo.SelectedItems[0].FindData.cFileName,"..")) NoDots=false;
    if(NoDots&&(PInfo.SelectedItemsNumber>0||MenuCode==(mMenuName-mMenuCopy))&&(!PInfo.Plugin||(PInfo.Flags&PFLAGS_REALNAMES)))
    {
      bool bcopy_flags[BCOPY_FLAG_COUNT];
      for(int i=0;i<BCOPY_FLAG_COUNT;i++) bcopy_flags[i]=false;
      char SrcA[MAX_PATH]; wchar_t SrcW[MAX_PATH];
      if(!PInfo.Plugin)
      {
        strcpy(SrcA,PInfo.CurDir);
        if(MenuCode!=5) UNCPath(SrcA); //FIXME: named const
        FSF.AddEndSlash(SrcA);
        MultiByteToWideChar(CP_OEMCP,0,SrcA,-1,SrcW,MAX_PATH);
      }
      else
      {
        SrcA[0]=0;
        SrcW[0]=0;
      }

      DWORD *send=NULL;
      DWORD sendsize=sizeof(DWORD)*3+sizeof(FileRec)*(PInfo.SelectedItemsNumber+2);
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
          PanelInfo PAnotherInfo;
          Info.Control(INVALID_HANDLE_VALUE,FCTL_GETANOTHERPANELSHORTINFO,&PAnotherInfo);
          if(PlgOpt.CheckPassive&&PAnotherInfo.Plugin)
            break;

          char DestA[MAX_PATH]; wchar_t DestW[MAX_PATH];
          if(PAnotherInfo.Plugin)
            strcpy(DestA,"");
          else
          {
            strcpy(DestA,PAnotherInfo.CurDir);
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
          char *CopyHistoryName="Copy";
          static char *BCopyCopyHistoryName="BCopyCopy";
          static struct InitDialogItem InitDlg[]={
          /* 0*/  {DI_DOUBLEBOX,3,1,72,19,0,0,0,0,(char *)mCpyDlgCopyTitle},
          /* 1*/  {DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
          /* 2*/  {DI_EDIT,5,3,70,0,1,(DWORD_PTR)BCopyCopyHistoryName,DIF_HISTORY,0,""},
          /* 3*/  {DI_TEXT,5,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
          /* 4*/  {DI_RADIOBUTTON,5,5,0,0,0,0,DIF_GROUP,0,(char *)mCpyDlgOpt1},
          /* 5*/  {DI_RADIOBUTTON,5,6,0,0,0,0,0,0,(char *)mCpyDlgOpt2},
          /* 6*/  {DI_RADIOBUTTON,5,7,0,0,0,0,0,0,(char *)mCpyDlgOpt3},
          /* 7*/  {DI_RADIOBUTTON,5,8,0,0,0,0,0,0,(char *)mCpyDlgOpt4},
          /* 8*/  {DI_RADIOBUTTON,5,9,0,0,0,0,0,0,(char *)mCpyDlgAsk},
          /* 9*/  {DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)mCpyDlgChk1},
          /*10*/  {DI_CHECKBOX,5,12,0,0,0,0,0,0,(char *)mCpyDlgChkAccess},
          /*11*/  {DI_CHECKBOX,5,13,0,0,0,0,DIF_DISABLE,0,(char *)mCpyDlgChk2},
          /*12*/  {DI_CHECKBOX,5,14,0,0,0,0,DIF_DISABLE,0,(char *)mCpyDlgChk3},
          /*13*/  {DI_CHECKBOX,5,15,0,0,0,0,DIF_3STATE,0,(char *)mCpyDlgChk4},
          /*14*/  {DI_CHECKBOX,5,16,0,0,0,0,DIF_3STATE,0,(char *)mCpyDlgChkLink},
          /*15*/  {DI_TEXT,5,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
          /*16*/  {DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,1,(char *)mCpyDlgOkCopy},
          /*17*/  {DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)mCpyDlgCancel},
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
          if(PInfo.SelectedItemsNumber>1)
          {
            sprintf(dialog_data.CopyMessage,GetMsg(mCpyDlgCopyToN+NumberType(PInfo.SelectedItemsNumber)),PInfo.SelectedItemsNumber);
            sprintf(dialog_data.MoveMessage,GetMsg(mCpyDlgMoveToN+NumberType(PInfo.SelectedItemsNumber)),PInfo.SelectedItemsNumber);
          }
          else
          {
            NormalizeName(InitDlg[0].X2-InitDlg[0].X1-3,mCpyDlgCopyTo1,PInfo.SelectedItems[0].FindData.cFileName,dialog_data.CopyMessage);
            NormalizeName(InitDlg[0].X2-InitDlg[0].X1-3,mCpyDlgMoveTo1,PInfo.SelectedItems[0].FindData.cFileName,dialog_data.MoveMessage);
          }
          strcpy(DialogItems[COPYDLG_LTO].Data,dialog_data.CopyMessage);
          strcpy(DialogItems[COPYDLG_ETO].Data,DestA);
          int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,76,21,"Copy",DialogItems,sizeofa(InitDlg),0,0,CopyDialogProc,(LONG_PTR)&dialog_data);
          if(DlgCode==COPYDLG_OK)
          {
            FSF.Unquote(DialogItems[COPYDLG_ETO].Data);
            if(PlgOpt.ExpandVars)
            {
              char exp[MAX_PATH]; DWORD res;
              res=FSF.ExpandEnvironmentStr(DialogItems[COPYDLG_ETO].Data,exp,sizeof(exp));
              strcpy(DialogItems[COPYDLG_ETO].Data,exp);
            }
            char *filename;
            filename=strrchr(DialogItems[COPYDLG_ETO].Data,'\\');
            if(!filename)
              filename=DialogItems[COPYDLG_ETO].Data;
            if(!(strcmp(filename,".")&&strcmp(filename,"..")))
              strcat(DialogItems[COPYDLG_ETO].Data,"\\");
            DWORD full_res=GetFullPathName(DialogItems[COPYDLG_ETO].Data,sizeof(DestA),DestA,&filename);
            if(!full_res||full_res>=sizeof(DestA))
            {
              char err3[512];
              sprintf(err3,GetMsg(mCpyErr3),DialogItems[COPYDLG_ETO].Data);
              const char *MsgItems[]={GetMsg(mError),err3,GetMsg(mOk)};
              Info.Message(Info.ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,sizeofa(MsgItems),1);
              break;
            }
            UNCPath(DestA);
            //check for such names as \\ZG\TEMP
            if(!CheckSystemFile(DestA))
            {
              char Volume[MAX_PATH];
              FSF.GetPathRoot(DestA,Volume);
              if(strlen(DestA)<strlen(Volume))
                FSF.AddEndSlash(DestA);
            }
            if(DestA[strlen(DestA)-1]!='\\')
            {
              WIN32_FIND_DATAA find;
              HANDLE test_handle;
              if((test_handle=FindFirstFileA(DestA,&find))!=INVALID_HANDLE_VALUE)
              {
                FindClose(test_handle);
                if(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
                  strcat(DestA,"\\");
              }
              else if((!CheckSystemFile(DestA))&&(PInfo.SelectedItemsNumber==1)&&(!(PInfo.SelectedItems[0].Flags&PPIF_SELECTED))&&(PInfo.SelectedItems[0].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
              {
                bcopy_flags[BCOPY_REN_COPY]=true;
                strcat(DestA,"\\");
              }
            }
            MultiByteToWideChar(CP_OEMCP,0,DestA,-1,DestW,MAX_PATH);
            if(CheckPaths(SrcW,DestW,FALSE))
              goto copyexit_err1;
            send[0]=OPERATION_COPY;
            /*fill flags*/
            send[1]=0;
            if(DialogItems[COPYDLG_OVERWRITE].Selected)
              send[1]|=COPYFLAG_OVERWRITE;
            if(DialogItems[COPYDLG_APPEND].Selected)
              send[1]|=COPYFLAG_APPEND;
            if(DialogItems[COPYDLG_SKIP].Selected)
              send[1]|=COPYFLAG_SKIP;
            if(DialogItems[COPYDLG_REFRESH].Selected)
              send[1]|=COPYFLAG_REFRESH;
            if(DialogItems[COPYDLG_ASK].Selected)
              send[1]|=COPYFLAG_ASK;
            if(DialogItems[COPYDLG_REMOVE].Selected)
            {
              send[1]|=COPYFLAG_MOVE;
              bcopy_flags[BCOPY_MOVE]=true;
              //check same drive
              char VolumeSrc[MAX_PATH],VolumeDest[MAX_PATH];
              FSF.GetPathRoot(SrcA,VolumeSrc);
              FSF.GetPathRoot(DestA,VolumeDest);
              if(!_stricmp(VolumeSrc,VolumeDest))
                bcopy_flags[BCOPY_NOREAL_COPY]=true;
            }
            if(DialogItems[COPYDLG_ACCESS].Selected)
              send[1]|=COPYFLAG_ACCESS;
            if(DialogItems[COPYDLG_ROSRC].Selected)
              send[1]|=COPYFLAG_ROSRC;
            if(DialogItems[COPYDLG_RODST].Selected)
              send[1]|=COPYFLAG_RODST;
            switch(DialogItems[COPYDLG_ABORT].Selected)
            {
              case BSTATE_CHECKED:
                send[1]|=COPYFLAG_FAIL;
                break;
              case BSTATE_3STATE:
                send[1]|=COPYFLAG_FAIL_ASK|COPYFLAG_DONTLOGERRORS;
                break;
            }
            switch(DialogItems[COPYDLG_LINK].Selected)
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
              char Volume[MAX_PATH],VolumeName[MAX_PATH],VolumeFS[MAX_PATH];
              DWORD VolumeSerialNumber,MaximumComponentLength,FileSystemFlags;
              FSF.GetPathRoot(SrcA,Volume);
              strcpy(VolumeFS,"    ");
              GetVolumeInformation(Volume,VolumeName,sizeof(VolumeName),&VolumeSerialNumber,&MaximumComponentLength,&FileSystemFlags,VolumeFS,sizeof(VolumeFS));
              if((GetDriveType(Volume)==DRIVE_CDROM)||(!_stricmp(VolumeFS,"CDFS")))
              {
                send[1]|=COPYFLAG_ATTR;
                send[1]|=COPYFLAG_CLEAR_READONLY;
              }
            }
            send[2]=PInfo.SelectedItemsNumber+2;
            //create destination dirs
            {
              if(bcopy_flags[BCOPY_REN_COPY]&&bcopy_flags[BCOPY_NOREAL_COPY]&&bcopy_flags[BCOPY_MOVE])
              { //don't create renamed dir
                int last_char_index=(int)strlen(DestA)-1;
                if(DestA[last_char_index]=='\\')
                  DestA[last_char_index]=0;
              }
              char DirA[MAX_PATH],*DirEnd;
              if((strlen(DestA)>2)&&(!CheckSystemFile(DestA)))
              {
                BOOL CreateFlag=FALSE; char CreateDest[MAX_PATH];
                DirEnd=DestA+3;
                do
                {
                  DirEnd=strchr(DirEnd,'\\');
                  if(DirEnd)
                  {
                    strncpy(DirA,DestA,DirEnd-DestA); DirA[DirEnd-DestA]=0;
                    DirEnd++;
                    CreateDirectoryA(DirA,NULL);
                    CreateFlag=TRUE;
                  }
                } while(DirEnd);
                if(CreateFlag)
                {
                  strcpy(CreateDest,DirA);
                  strcat(CreateDest,"\\*");
                  WIN32_FIND_DATAA find;
                  SetLastError(0);
                  HANDLE hFind=FindFirstFileA(CreateDest,&find);
                  DWORD err=GetLastError();
                  if(hFind!=INVALID_HANDLE_VALUE)
                    FindClose(hFind);
                  if(err==ERROR_PATH_NOT_FOUND)
                  {
                    char err2[512];
                    sprintf(err2,GetMsg(mCpyErr2),DirA);
                    const char *MsgItems[]={GetMsg(mError),err2,GetMsg(mOk)};
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
          /* 0*/  {DI_DOUBLEBOX,3,1,62,8,0,0,0,0,(char *)mDelDlgTitle},
          /* 1*/  {DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
          /* 2*/  {DI_TEXT,5,3,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
          /* 3*/  {DI_CHECKBOX,5,4,0,0,0,0,0,0,(char *)mDelDlgChk1},
          /* 4*/  {DI_CHECKBOX,5,5,0,0,0,0,DIF_3STATE,0,(char *)mDelDlgChk2},
          /* 5*/  {DI_TEXT,5,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
          /* 6*/  {DI_BUTTON,0,7,0,0,1,0,DIF_CENTERGROUP,1,(char *)mDelDlgOk},
          /* 7*/  {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(char *)mDelDlgCancel}
          };
          struct FarDialogItem DialogItems[sizeofa(InitDlg)];
          InitDialogItems(InitDlg,DialogItems,sizeofa(InitDlg));
          DialogItems[3].Selected=Opt.DeleteRO;
          DialogItems[4].Selected=Opt.DeleteAbort;
          if(PInfo.SelectedItemsNumber>1)
            sprintf(DialogItems[1].Data,GetMsg(mDelDlgDeleteN+NumberType(PInfo.SelectedItemsNumber)),PInfo.SelectedItemsNumber);
          else
            NormalizeName(InitDlg[0].X2-InitDlg[0].X1-3,mDelDlgDelete1,PInfo.SelectedItems[0].FindData.cFileName,DialogItems[1].Data);
          CommonDialogData dialog_data={TRUE};
          int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,66,10,"Delete",DialogItems,sizeofa(InitDlg),0,0,DelDialogProc,(LONG_PTR)&dialog_data);
          if(DlgCode==6)
          {
            send[0]=OPERATION_COPY;
            /*fill flags*/
            send[1]=COPYFLAG_DELETE|COPYFLAG_OVERWRITE;
            if(DialogItems[3].Selected)
              send[1]|=COPYFLAG_ROSRC;
            switch(DialogItems[4].Selected)
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
            send[2]=PInfo.SelectedItemsNumber+2;
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
          /* 0*/  {DI_DOUBLEBOX,3,1,41,12,0,0,0,0,(char *)mAttrDlgTitle},
          /* 1*/  {DI_TEXT,0,3,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
          /* 2*/  {DI_TEXT,0,2,0,0,0,0,0,0,(char *)mAttrDlgTitle2},
          /* 3*/  {DI_TEXT,5,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
          /* 4*/  {DI_CHECKBOX,5,5,0,0,1,2,DIF_3STATE,0,(char *)mAttrDlgReadOnly},
          /* 5*/  {DI_CHECKBOX,5,6,0,0,0,2,DIF_3STATE,0,(char *)mAttrDlgArchive},
          /* 6*/  {DI_CHECKBOX,5,7,0,0,0,2,DIF_3STATE,0,(char *)mAttrDlgHidden},
          /* 7*/  {DI_CHECKBOX,5,8,0,0,0,2,DIF_3STATE,0,(char *)mAttrDlgSystem},
          /* 8*/  {DI_CHECKBOX,5,9,0,0,0,2,DIF_3STATE|DIF_DISABLE,0,(char *)mAttrDlgCompressed},
          /* 9*/  {DI_TEXT,5,10,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
          /*10*/  {DI_BUTTON,0,11,0,0,1,0,DIF_CENTERGROUP,1,(char *)mAttrDlgSet},
          /*11*/  {DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,0,(char *)mAttrDlgCancel}
          };
          struct FarDialogItem DialogItems[sizeofa(InitDlg)];
          InitDialogItems(InitDlg,DialogItems,sizeofa(InitDlg));
          if(PInfo.SelectedItemsNumber>1)
            sprintf(DialogItems[1].Data,GetMsg(mAttrDlgChangeN+NumberType(PInfo.SelectedItemsNumber)),PInfo.SelectedItemsNumber);
          else
            NormalizeName(InitDlg[0].X2-InitDlg[0].X1-3,mAttrDlgChange1,PInfo.SelectedItems[0].FindData.cFileName,DialogItems[1].Data);
          DialogItems[1].X1=4+(37-(int)strlen(DialogItems[1].Data))/2;
          DialogItems[2].X1=4+(37-(int)strlen(DialogItems[2].Data))/2;
          { // clear read-only attribute, when copying from CD
            char Volume[MAX_PATH],VolumeName[MAX_PATH],VolumeFS[MAX_PATH];
            DWORD VolumeSerialNumber,MaximumComponentLength,FileSystemFlags;
            FSF.GetPathRoot(SrcA,Volume);
            if(GetVolumeInformation(Volume,VolumeName,sizeof(VolumeName),&VolumeSerialNumber,&MaximumComponentLength,&FileSystemFlags,VolumeFS,sizeof(VolumeFS)))
              if(FileSystemFlags&FS_FILE_COMPRESSION)
                DialogItems[8].Flags&=~DIF_DISABLE;
          }
          CommonDialogData dialog_data={TRUE};
          int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,45,14,"Attributes",DialogItems,sizeofa(InitDlg),0,0,AttrDialogProc,(LONG_PTR)&dialog_data);
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
              send[1]|=flags[i][DialogItems[4+i].Selected];
            if(send[1]!=(COPYFLAG_ATTRMAIN|COPYFLAG_ATTR|COPYFLAG_OVERWRITE))
            {
              if(Opt.CopyFullInfo) //!!!
                send[1]|=COPYFLAG_STATISTIC;
              send[2]=PInfo.SelectedItemsNumber+2;
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
          /* 0*/  {DI_DOUBLEBOX,3,1,62,7,0,0,0,0,(char *)mWpeDlgTitle},
          /* 1*/  {DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
          /* 2*/  {DI_TEXT,5,3,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
          /* 3*/  {DI_CHECKBOX,5,4,0,0,0,0,DIF_3STATE,0,(char *)mWpeDlgChk},
          /* 4*/  {DI_TEXT,5,5,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
          /* 5*/  {DI_BUTTON,0,6,0,0,1,0,DIF_CENTERGROUP,1,(char *)mWpeDlgOk},
          /* 6*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP,0,(char *)mWpeDlgCancel}
          };
          struct FarDialogItem DialogItems[sizeofa(InitDlg)];
          InitDialogItems(InitDlg,DialogItems,sizeofa(InitDlg));
          DialogItems[3].Selected=Opt.WipeAbort;
          if(PInfo.SelectedItemsNumber>1)
            sprintf(DialogItems[1].Data,GetMsg(mWpeDlgWipeN+NumberType(PInfo.SelectedItemsNumber)),PInfo.SelectedItemsNumber);
          else
            NormalizeName(InitDlg[0].X2-InitDlg[0].X1-3,mWpeDlgWipe1,PInfo.SelectedItems[0].FindData.cFileName,DialogItems[1].Data);
          CommonDialogData dialog_data={TRUE};
          int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,66,9,"Delete",DialogItems,sizeofa(InitDlg),0,0,WipeDialogProc,(LONG_PTR)&dialog_data);
          if(DlgCode==5)
          {
            send[0]=OPERATION_COPY;
            /*fill flags*/
            send[1]=COPYFLAG_DELETE|COPYFLAG_WIPE|COPYFLAG_OVERWRITE;
            switch(DialogItems[3].Selected)
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
            send[2]=PInfo.SelectedItemsNumber+2;
            #define CHECK_DEST
            #define DEST_W L""
            #define ON_ERROR ;
            #include "bcopy_find.cpp"
          }
          break;
        }
        case 5:
          if(PInfo.CurrentItem>=0)
          {
            wchar_t WideName[MAX_PATH],WideFullName[MAX_PATH];
            if(GetWideName(SrcA,&PInfo.PanelItems[PInfo.CurrentItem].FindData,WideName))
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
          if((PInfo.CurrentItem>=0)&&!(PInfo.PanelItems[PInfo.CurrentItem].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
          {
            char SrcLA[MAX_PATH];
            char *ShortFileName=PInfo.PanelItems[PInfo.CurrentItem].FindData.cAlternateFileName;
            strcpy(SrcLA,SrcA);
            if(!ShortFileName[0]||PInfo.Plugin)
              ShortFileName=PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName;
            strcat(SrcA,ShortFileName);
            strcat(SrcLA,PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName);
            if(MenuCode==7)
              Info.Viewer(SrcA,SrcLA,0,0,-1,-1,VF_NONMODAL|VF_IMMEDIATERETURN|VF_ENABLE_F6);
            else
              Info.Editor(SrcA,SrcLA,0,0,-1,-1,EF_NONMODAL|EF_IMMEDIATERETURN|EF_ENABLE_F6,1,1);
          }
          break;
        }
        case 9:
          {
            wchar_t WideName[MAX_PATH];
            if(OpenFrom==OPEN_PLUGINSMENU&&PInfo.ItemsNumber>0)
            {
              if(PInfo.CurrentItem>=0)
              {
                if(PInfo.Plugin)
                {
                  if(GetWideNameDirect(SrcA,PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName,WideName))
                    ShowName(WideName);
                }
                else
                {
                  if(GetWideName(SrcA,&PInfo.PanelItems[PInfo.CurrentItem].FindData,WideName))
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
                if(GetWideNameDirect("",winfo.Name,WideName))
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
        Configure(0);
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

int WINAPI _export Configure(int ItemNumber)
{
  switch(ItemNumber)
  {
    case 0:
      return(Config());
  }
  return(FALSE);
}

void WINAPI _export ExitFAR()
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
