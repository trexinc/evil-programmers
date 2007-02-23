/*
    PluginManager plugin for FAR Manager
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
#include "plugin_viewer.hpp"
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
FARAPIVIEWERCONTROL ViewerControl;

enum
{
  MTitle,
  MConfig1,
  MConfig2,
  MBottom,
  MOpenPanels1,
  MOpenPanels2,
  MsgOK,
};

#include "common.cpp"

HANDLE log=INVALID_HANDLE_VALUE;
#include "log.cpp"

#include "module.cpp"
#include "fs.cpp"
#include "pm.cpp"

PluginManager *pm=NULL;

#include "overrides.cpp"

void WINAPI SetStartupInfo(const struct PluginStartupInfo *psi)
{
  log = OpenLog("G:\\Program Files\\Far\\pm\\PluginManager.log");
  WriteLog(log,"SetStartupInfo\n");
  Info = *psi;
  FSF = *psi->FSF;
  Info.FSF = &FSF;
  ViewerControl = (FARAPIVIEWERCONTROL) Info.Reserved[1];
  //FSF.sprintf(PluginRootKey,"%s\\PluginManager",Info.RootKey);

  { //init plugins
    char plug_dir[MAX_PATH];
    lstrcpy(plug_dir, Info.ModuleName);
    *(FSF.PointToName(plug_dir)-1) = 0;
    *(FSF.PointToName(plug_dir)) = 0;
    char dir[1024];
    GetCurrentDirectory(sizeof(dir),dir);
    lstrcat(plug_dir, "Plugins\\");
    pm = new PluginManager(plug_dir,psi);
    SetCurrentDirectory(dir);
  }
  if (pm)
    pm->SetStartupInfo();
}

void WINAPI GetPluginInfo(struct PluginInfo *pi)
{
  WriteLog(log,"GetPluginInfo\n");
  if (!pm)
    return;
  pm->GetPluginInfo(pi);
}

int WINAPI ProcessEditorInput(const INPUT_RECORD *Rec)
{
  return pm->ProcessEditorInput(Rec);
}

int WINAPI ProcessEditorEvent(int Event,void *Param)
{
  return pm->ProcessEditorEvent(Event,Param);
}

int WINAPI ProcessViewerEvent(int Event,void *Param)
{
  return pm->ProcessViewerEvent(Event,Param);
}

int WINAPI Configure(int ItemNumber)
{
  WriteLog(log,"Config\n");
  if (ItemNumber==0)
    return pm->Configure();
  return pm->Manager();
}

void WINAPI ExitFAR()
{
  WriteLog(log,"ExitFAR\n");
  CloseLog(&log);
  if (pm)
    delete pm;
}

HANDLE WINAPI OpenPlugin(int OpenFrom,int Item)
{
  WriteLog(log,"OpenPlugin\n");
  return pm->OpenPlugin(OpenFrom,Item);
}

HANDLE WINAPI OpenFilePlugin(char *Name,const unsigned char *Data,int DataSize)
{
  WriteLog(log,"OpenFilePlugin\n");
  return pm->OpenFilePlugin(Name,Data,DataSize);
}

void WINAPI ClosePlugin(HANDLE hPlugin)
{
  WriteLog(log,"ClosePlugin\n");
  pm->ClosePlugin(hPlugin);
}

void WINAPI GetOpenPluginInfo(HANDLE hPlugin, struct OpenPluginInfo *Info)
{
  WriteLog(log,"GetOpenPluginInfo\n");
  CFileSystem *fs = (CFileSystem *) hPlugin;
  return fs->GetOpenPluginInfo(Info);
}

int WINAPI GetFindData(HANDLE hPlugin, struct PluginPanelItem **pPanelItem,  int *pItemsNumber, int OpMode)
{
  WriteLog(log,"GetFindData\n");
  CFileSystem *fs = (CFileSystem *) hPlugin;
  return fs->GetFindData(pPanelItem,pItemsNumber,OpMode);
}

void WINAPI FreeFindData(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber)
{
  WriteLog(log,"FreeFindData\n");
  CFileSystem *fs = (CFileSystem *) hPlugin;
  fs->FreeFindData(PanelItem,ItemsNumber);
}

int WINAPI GetVirtualFindData(HANDLE hPlugin, struct PluginPanelItem **pPanelItem,  int *pItemsNumber, const char *Path)
{
  WriteLog(log,"GetVirtualFindData\n");
  CFileSystem *fs = (CFileSystem *) hPlugin;
  return fs->GetVirtualFindData(pPanelItem,pItemsNumber,Path);
}

void WINAPI FreeVirtualFindData(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber)
{
  WriteLog(log,"FreeVirtualFindData\n");
  CFileSystem *fs = (CFileSystem *) hPlugin;
  fs->FreeVirtualFindData(PanelItem,ItemsNumber);
}

int WINAPI Compare(HANDLE hPlugin,const struct PluginPanelItem *Item1,const struct PluginPanelItem *Item2,unsigned int Mode)
{
  CFileSystem *fs = (CFileSystem *) hPlugin;
  return fs->Compare(Item1,Item2,Mode);
}

int WINAPI ProcessKey(HANDLE hPlugin, int Key, unsigned int ControlState)
{
  CFileSystem *fs = (CFileSystem *) hPlugin;
  return fs->ProcessKey(Key,ControlState);
}

int WINAPI ProcessEvent(HANDLE hPlugin, int Event, void *Param)
{
  CFileSystem *fs = (CFileSystem *) hPlugin;
  return fs->ProcessEvent(Event,Param);
}

int WINAPI ProcessHostFile(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  WriteLog(log,"ProcessHostFile\n");
  CFileSystem *fs = (CFileSystem *) hPlugin;
  return fs->ProcessHostFile(PanelItem,ItemsNumber,OpMode);
}

int WINAPI GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode)
{
  WriteLog(log,"GetFiles\n");
  CFileSystem *fs = (CFileSystem *) hPlugin;
  return fs->GetFiles(PanelItem,ItemsNumber,Move,DestPath,OpMode);
}

int WINAPI PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode)
{
  WriteLog(log,"PutFiles\n");
  CFileSystem *fs = (CFileSystem *) hPlugin;
  return fs->PutFiles(PanelItem,ItemsNumber,Move,OpMode);
}

int WINAPI DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  WriteLog(log,"DeleteFiles\n");
  CFileSystem *fs = (CFileSystem *) hPlugin;
  return fs->DeleteFiles(PanelItem,ItemsNumber,OpMode);
}

int WINAPI SetDirectory(HANDLE hPlugin, const char *Dir, int OpMode)
{
  WriteLog(log,"SetDirectory\n");
  CFileSystem *fs = (CFileSystem *) hPlugin;
  return fs->SetDirectory(Dir,OpMode);
}

int WINAPI MakeDirectory(HANDLE hPlugin,char *Name,int OpMode)
{
  WriteLog(log,"MakeDirectory\n");
  CFileSystem *fs = (CFileSystem *) hPlugin;
  return fs->MakeDirectory(Name,OpMode);
}

int WINAPI SetFindList(HANDLE hPlugin,const struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  WriteLog(log,"SetFindList\n");
  CFileSystem *fs = (CFileSystem *) hPlugin;
  return fs->SetFindList(PanelItem,ItemsNumber);
}
