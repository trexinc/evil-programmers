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
#include <windows.h>
#include <string.h>
#include "fsplugin.h"

int __stdcall ProgressProc(int PluginNr,char* SourceName,char* TargetName,int PercentDone)
{
  return 0;
}

void __stdcall LogProc(int PluginNr,int MsgType,char* LogString)
{
}

BOOL __stdcall RequestProc(int PluginNr,int RequestType,char* CustomTitle,char* CustomText,char* ReturnedText,int maxlen)
{
  return FALSE;
}

typedef int __stdcall FsExecuteFile_t(HWND MainWin,char* RemoteName,char* Verb);
FsExecuteFile_t *FsExecuteFileMy;

LRESULT WINAPI WndProc(HWND hwnd,UINT msg,UINT wParam,LONG lParam)
{
  switch (msg)
  {
    case WM_CREATE:
      FsExecuteFileMy(NULL,"\\Settings","open");
    break;
    case WM_DESTROY:
      PostQuitMessage(0);
    break;
    default:
      return(DefWindowProc(hwnd, msg, wParam, lParam));
  }
  return(0);
}

int WINAPI WinMain(HINSTANCE hI, HINSTANCE hPI, LPSTR lpCmdLine, int nC)
{
  typedef int __stdcall FsInit_t(int PluginNr,tProgressProc pProgressProc, tLogProc pLogProc,tRequestProc pRequestProc);
  typedef HANDLE __stdcall FsFindFirst_t(char* Path,WIN32_FIND_DATA *FindData);
  typedef BOOL __stdcall FsFindNext_t(HANDLE Hdl,WIN32_FIND_DATA *FindData);
  typedef int __stdcall FsFindClose_t(HANDLE Hdl);
  typedef void __stdcall FsGetDefRootName_t(char* DefRootName,int maxlen);
  typedef void __stdcall FsSetDefaultParams_t(FsDefaultParamStruct* dps);

  FsInit_t *FsInit;
  FsFindFirst_t *FsFindFirst;
  FsFindNext_t *FsFindNext;
  FsFindClose_t *FsFindClose;
  FsGetDefRootName_t *FsGetDefRootName;
  FsSetDefaultParams_t *FsSetDefaultParams;
  HMODULE wfx = LoadLibrary(lpCmdLine);

  FsInit = (FsInit_t *) GetProcAddress(wfx, "FsInit");
  FsFindFirst = (FsFindFirst_t *) GetProcAddress(wfx, "FsFindFirst");
  FsFindNext = (FsFindNext_t *) GetProcAddress(wfx, "FsFindNext");
  FsFindClose = (FsFindClose_t *) GetProcAddress(wfx, "FsFindClose");
  FsExecuteFileMy = (FsExecuteFile_t *) GetProcAddress(wfx, "FsExecuteFile");
  FsGetDefRootName = (FsGetDefRootName_t *) GetProcAddress(wfx, "FsGetDefRootName");
  FsSetDefaultParams = (FsSetDefaultParams_t *) GetProcAddress(wfx, "FsSetDefaultParams");

  if (FsGetDefRootName)
  {
    char root[MAX_PATH];
    FsGetDefRootName(root, MAX_PATH);
  }

  FsInit(1,ProgressProc,LogProc,RequestProc);

  if (FsSetDefaultParams)
  {
    FsDefaultParamStruct dps;
    dps.size = sizeof(dps);
    dps.PluginInterfaceVersionLow = 30;
    dps.PluginInterfaceVersionHi = 1;
    strcpy(dps.DefaultIniName, "G:\\Program Files\\Far\\Plugins\\wfx2far\\wfx\\wfx.ini");
    FsSetDefaultParams(&dps);
  }

  WIN32_FIND_DATA fd;
  char path[1024];
  strcpy(path,"\\");
  HANDLE fh = FsFindFirst(path,&fd);
  if (fh != INVALID_HANDLE_VALUE)
  {
    BOOL fn;
    do
    {
      fn = FsFindNext(fh, &fd);
    } while (fn);
    FsFindClose(fh);
  }
  WNDCLASS winclass;
  winclass.style = CS_HREDRAW|CS_VREDRAW;
  winclass.lpfnWndProc = WndProc;
  winclass.lpszClassName = "wfx2far";
  winclass.lpszMenuName = NULL;
  winclass.hIcon = NULL;
  winclass.hCursor = NULL;
  winclass.hbrBackground = NULL;
  winclass.hInstance = hI;
  winclass.cbClsExtra = 0;
  winclass.cbWndExtra = 0;
  RegisterClass(&winclass);
  CreateWindow("wfx2far",
       "wfx2far",
       WS_VISIBLE,
       0, 0,
       10, 10,
       NULL, NULL,
       hI, NULL);

  MSG msg;
  while(true)
  {
    if(PeekMessage(&msg, NULL, 0,0, PM_REMOVE))
    {
      if(msg.message == WM_QUIT)
        break;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  FreeLibrary(wfx);
  return 0;
}