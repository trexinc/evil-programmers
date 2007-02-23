/*
    WFX2FAR plugin for FAR Manager
    Copyright (C) 2003-2005 Alex Yaroslavsky

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
#include "plugin.hpp"
#include "fsplugin.h"
#include "memory.cpp"
#include "crt.cpp"

#if defined(__GNUC__)
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
#endif

struct PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
HWND FarHWND;

#include "common.cpp"

enum
{
  MTitle,
  MDeleteAsk,
  MMakeDir,
  MPumpWarn,
  MsgRT_Other,
  MsgRT_UserName,
  MsgRT_Password,
  MsgRT_Account,
  MsgRT_UserNameFirewall,
  MsgRT_PasswordFirewall,
  MsgRT_TargetDir,
  MsgRT_URL,
  MsgOK,
};

#include "wfx.cpp"
#include "fs.cpp"
#include "pm.cpp"

static PluginManager *pm=NULL;

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *psi)
{
  Info = *psi;
  FSF = *psi->FSF;
  Info.FSF = &FSF;
  //FSF.sprintf(PluginRootKey,"%s\\wfx2far",Info.RootKey);

  { //init plugins
    char plug_dir[MAX_PATH];
    lstrcpy(plug_dir, Info.ModuleName);
    *(FSF.PointToName(plug_dir)) = 0;
    char dir[1024];
    GetCurrentDirectory(sizeof(dir),dir);
    lstrcat(plug_dir, "wfx\\");
    pm = new PluginManager(plug_dir);
    SetCurrentDirectory(dir);
  }
  FarHWND = (HWND) Info.AdvControl(Info.ModuleNumber,ACTL_GETFARHWND,0);
}

void WINAPI _export GetPluginInfo(struct PluginInfo *pi)
{
  if (!pm) return;
  pi->StructSize = sizeof(struct PluginInfo);
  static const char *MenuStrings[1];
  MenuStrings[0] = GetMsg(MTitle);
  pi->PluginMenuStrings = MenuStrings;
  pi->PluginMenuStringsNumber = 1;
  pi->PluginConfigStrings = MenuStrings;
  pi->PluginConfigStringsNumber = 1;
}

void WINAPI ExitFAR()
{
  if (pm) delete pm;
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  (void) Item;
  (void) OpenFrom;
  SetFileApisToANSI();
  HANDLE ret = (HANDLE) pm->SelectFS(false);
  SetFileApisToOEM();
  if (ret == NULL)
    ret = INVALID_HANDLE_VALUE;
  return ret;
}

void WINAPI ClosePlugin(HANDLE hPlugin)
{
  SetFileApisToANSI();
  ((FileSystem *) hPlugin)->ClosePlugin();
  SetFileApisToOEM();
}

void WINAPI GetOpenPluginInfo(HANDLE hPlugin, struct OpenPluginInfo *Info)
{
  SetFileApisToANSI();
  ((FileSystem *) hPlugin)->GetOpenPluginInfo(Info);
  SetFileApisToOEM();
  static char Dir[1024];
  static char Format[1024];
  static char Title[1024*2+1];
  CharToOem(Info->CurDir,Dir);
  Info->CurDir=Dir;
  CharToOem(Info->Format,Format);
  Info->Format=Format;
  CharToOem(Info->PanelTitle,Title);
  Info->PanelTitle=Title;
}

int WINAPI SetDirectory(HANDLE hPlugin, const char *FarDir, int OpMode)
{
  FileSystem *fs = (FileSystem *) hPlugin;
  static char Dir[1024];
  OemToChar(FarDir,Dir);
  SetFileApisToANSI();
  int ret=fs->SetDirectory(Dir,OpMode);
  SetFileApisToOEM();
  return ret;
}

int WINAPI GetFindData(HANDLE hPlugin, struct PluginPanelItem **pPanelItem,  int *pItemsNumber, int OpMode)
{
  FileSystem *fs = (FileSystem *) hPlugin;
  SetFileApisToANSI();
  fs->StatusInfo(FS_STATUS_START,FS_STATUS_OP_LIST);
  int ret=fs->GetFindData(pPanelItem,pItemsNumber,OpMode);
  fs->StatusInfo(FS_STATUS_END,FS_STATUS_OP_LIST);
  SetFileApisToOEM();
  if (ret)
  {
    for (int i=0; i<*pItemsNumber; i++)
    {
      CharToOem((*pPanelItem)[i].FindData.cFileName,(*pPanelItem)[i].FindData.cFileName);
      CharToOem((*pPanelItem)[i].FindData.cAlternateFileName,(*pPanelItem)[i].FindData.cAlternateFileName);
    }
  }
  return ret;
}

void WINAPI FreeFindData(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber)
{
  SetFileApisToANSI();
  ((FileSystem *) hPlugin)->FreeFindData(PanelItem,ItemsNumber);
  SetFileApisToOEM();
}

int WINAPI ProcessKey(HANDLE hPlugin, int Key, unsigned int ControlState)
{
  FileSystem *fs = (FileSystem *) hPlugin;
  SetFileApisToANSI();
  int ret=fs->ProcessKey(Key,ControlState);
  SetFileApisToOEM();
  return ret;
}

int WINAPI GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode)
{
  FileSystem *fs = (FileSystem *) hPlugin;
  static char Dest[1024];
  OemToChar(DestPath,Dest);
  for (int i=0; i<ItemsNumber; i++)
  {
    OemToChar(PanelItem[i].FindData.cFileName,PanelItem[i].FindData.cFileName);
    //OemToChar(PanelItem[i].FindData.cAlternateFileName,PanelItem[i].FindData.cAlternateFileName);
  }
  SetFileApisToANSI();
  fs->StatusInfo(FS_STATUS_START,ItemsNumber==1?FS_STATUS_OP_GET_SINGLE:FS_STATUS_OP_GET_MULTI);
  int ret=fs->GetFiles(PanelItem,ItemsNumber,Move,Dest,OpMode);
  fs->StatusInfo(FS_STATUS_END,ItemsNumber==1?FS_STATUS_OP_GET_SINGLE:FS_STATUS_OP_GET_MULTI);
  SetFileApisToOEM();
  return ret;
}

int WINAPI PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode)
{
  FileSystem *fs = (FileSystem *) hPlugin;
  for (int i=0; i<ItemsNumber; i++)
  {
    OemToChar(PanelItem[i].FindData.cFileName,PanelItem[i].FindData.cFileName);
    //OemToChar(PanelItem[i].FindData.cAlternateFileName,PanelItem[i].FindData.cAlternateFileName);
  }
  SetFileApisToANSI();
  fs->StatusInfo(FS_STATUS_START,ItemsNumber==1?FS_STATUS_OP_PUT_SINGLE:FS_STATUS_OP_PUT_MULTI);
  int ret=fs->PutFiles(PanelItem,ItemsNumber,Move,OpMode);
  fs->StatusInfo(FS_STATUS_END,ItemsNumber==1?FS_STATUS_OP_PUT_SINGLE:FS_STATUS_OP_PUT_MULTI);
  SetFileApisToOEM();
  return ret;
}

int WINAPI DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  FileSystem *fs = (FileSystem *) hPlugin;
  const char *msg[2];
  char Title[1024];
  CharToOem(fs->Title(),Title);
  msg[0]=Title;
  msg[1]=GetMsg(MDeleteAsk);
  if (Info.Message(Info.ModuleNumber,FMSG_MB_YESNO,NULL,msg,2,0))
    return FALSE;
  for (int i=0; i<ItemsNumber; i++)
  {
    OemToChar(PanelItem[i].FindData.cFileName,PanelItem[i].FindData.cFileName);
    OemToChar(PanelItem[i].FindData.cAlternateFileName,PanelItem[i].FindData.cAlternateFileName);
  }
  SetFileApisToANSI();
  fs->StatusInfo(FS_STATUS_START,FS_STATUS_OP_DELETE);
  int ret=fs->DeleteFiles(PanelItem,ItemsNumber,OpMode);
  fs->StatusInfo(FS_STATUS_END,FS_STATUS_OP_DELETE);
  SetFileApisToOEM();
  return ret;
}

int WINAPI MakeDirectory(HANDLE hPlugin,char *Name,int OpMode)
{
  (void) Name;
  FileSystem *fs = (FileSystem *) hPlugin;
  char Title[1024];
  CharToOem(fs->Title(),Title);
  static const char History[]="wfx2far\\makedir";
  InitDialogItem InitItems[]=
  {
    // type,x1,y1,x2,y2,focus,selected,flags,default,data
    {DI_DOUBLEBOX,3,1,57,4,0,0,0,0,Title},
    {DI_TEXT,5,2,0,0,0,0,0,0,(char *)MMakeDir},
    {DI_EDIT,5,3,55,0,1,(DWORD)History,DIF_HISTORY,0,""},
    {DI_BUTTON,3,5,0,0,0,0,0,1,(char *)MsgOK},
  };
  FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));

  if (Info.Dialog(Info.ModuleNumber,-1,-1,60,6,NULL,DialogItems,sizeofa(DialogItems))!=3)
    return (-1);
  char Dir[1024];
  OemToChar(DialogItems[2].Data,Dir);
  SetFileApisToANSI();
  fs->StatusInfo(FS_STATUS_START,FS_STATUS_OP_MKDIR);
  int ret=fs->MakeDirectory(Dir,OpMode)?1:0;
  fs->StatusInfo(FS_STATUS_END,FS_STATUS_OP_MKDIR);
  SetFileApisToOEM();
  return ret;
}

int WINAPI Configure(int ItemNumber)
{
  (void) ItemNumber;
  SetFileApisToANSI();
  FileSystem *fs=NULL;
  do
  {
    if (fs)
    {
      fs->Config();
      delete fs;
    }
    fs=pm->SelectFS(true);
  } while (fs);
  SetFileApisToOEM();
  return TRUE;
}
