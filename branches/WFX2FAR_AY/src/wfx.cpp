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
typedef int __stdcall FsInit_t(int PluginNr,tProgressProc pProgressProc, tLogProc pLogProc,tRequestProc pRequestProc);
typedef HANDLE __stdcall FsFindFirst_t(const char* Path,WIN32_FIND_DATA *FindData);
typedef BOOL __stdcall FsFindNext_t(HANDLE Hdl,WIN32_FIND_DATA *FindData);
typedef int __stdcall FsFindClose_t(HANDLE Hdl);
typedef int __stdcall FsExecuteFile_t(HWND MainWin,char* RemoteName,char* Verb);
typedef void __stdcall FsGetDefRootName_t(char* DefRootName,int maxlen);
typedef void __stdcall FsSetDefaultParams_t(FsDefaultParamStruct* dps);
typedef int __stdcall FsGetFile_t(char* RemoteName,char* LocalName,int CopyFlags,RemoteInfoStruct* ri);
typedef void __stdcall FsStatusInfo_t(char* RemoteDir,int InfoStartEnd,int InfoOperation);
typedef BOOL __stdcall FsDisconnect_t(char* DisconnectRoot);
typedef int __stdcall FsPutFile_t(char* LocalName,char* RemoteName,int CopyFlags);
typedef int __stdcall FsRenMovFile_t(char* OldName,char* NewName,BOOL Move,BOOL OverWrite,RemoteInfoStruct* ri);
typedef BOOL __stdcall FsDeleteFile_t(char* RemoteName);
typedef BOOL __stdcall FsRemoveDir_t(char* RemoteName);
typedef BOOL __stdcall FsMkDir_t(char* Path);

class WFX
{
private:
  HMODULE hModule;
  char PluginName[1024];
  char RootName[1024];
  bool loaded;
  WFX(const char *filename);

public:
  static WFX *Create(const char *filename);
  ~WFX() {if (hModule) FreeLibrary(hModule);}
  const char *Title() {return ((*RootName)?RootName:PluginName);}
  FsInit_t *FsInit;
  FsFindFirst_t *FsFindFirst;
  FsFindNext_t *FsFindNext;
  FsFindClose_t *FsFindClose;
  FsExecuteFile_t *FsExecuteFile;
  FsGetDefRootName_t *FsGetDefRootName;
  FsSetDefaultParams_t *FsSetDefaultParams;
  FsGetFile_t *FsGetFile;
  FsStatusInfo_t *FsStatusInfo;
  FsDisconnect_t *FsDisconnect;
  FsPutFile_t *FsPutFile;
  FsRenMovFile_t *FsRenMovFile;
  FsDeleteFile_t *FsDeleteFile;
  FsRemoveDir_t *FsRemoveDir;
  FsMkDir_t *FsMkDir;
};

WFX::WFX(const char *filename)
{
  loaded=false;

  hModule = LoadLibrary(filename);
  if (!hModule)
    return;

  FsInit = (FsInit_t *) GetProcAddress(hModule, "FsInit");
  FsFindFirst = (FsFindFirst_t *) GetProcAddress(hModule, "FsFindFirst");
  FsFindNext = (FsFindNext_t *) GetProcAddress(hModule, "FsFindNext");
  FsFindClose = (FsFindClose_t *) GetProcAddress(hModule, "FsFindClose");
  FsExecuteFile = (FsExecuteFile_t *) GetProcAddress(hModule, "FsExecuteFile");
  FsGetDefRootName = (FsGetDefRootName_t *) GetProcAddress(hModule, "FsGetDefRootName");
  FsSetDefaultParams = (FsSetDefaultParams_t *) GetProcAddress(hModule, "FsSetDefaultParams");
  FsGetFile = (FsGetFile_t *) GetProcAddress(hModule, "FsGetFile");
  FsPutFile = (FsPutFile_t *) GetProcAddress(hModule, "FsPutFile");
  FsStatusInfo = (FsStatusInfo_t *) GetProcAddress(hModule, "FsStatusInfo");
  FsMkDir = (FsMkDir_t *) GetProcAddress(hModule, "FsMkDir");
  FsRemoveDir = (FsRemoveDir_t *) GetProcAddress(hModule, "FsRemoveDir");
  FsDeleteFile = (FsDeleteFile_t *) GetProcAddress(hModule, "FsDeleteFile");
  FsRenMovFile = (FsRenMovFile_t *) GetProcAddress(hModule, "FsRenMovFile");
  FsDisconnect = (FsDisconnect_t *) GetProcAddress(hModule, "FsDisconnect");

  if (!FsInit || !FsFindFirst || !FsFindNext || !FsFindClose)
    return;

  lstrcpy(PluginName,FSF.PointToName(filename));

  *RootName = 0;
  if (FsGetDefRootName)
  {
    FsGetDefRootName(RootName, sizeof(RootName));
  }

  loaded=true;
}

WFX *WFX::Create(const char *filename)
{
  WFX *wfx = new WFX(filename);
  if (wfx)
  {
    if (!wfx->loaded)
    {
      delete wfx;
      wfx=NULL;
    }
  }
  return wfx;
}
