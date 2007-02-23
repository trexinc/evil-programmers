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
#include <stdio.h>
#include <string.h>
#include "fsplugin.h"

int __stdcall ProgressProc(int PluginNr,char* SourceName,char* TargetName,int PercentDone)
{
  return 0;
}

void __stdcall LogProc(int PluginNr,int MsgType,char* LogString)
{
  printf("Log: %s\n",LogString);
}

BOOL __stdcall RequestProc(int PluginNr,int RequestType,char* CustomTitle,char* CustomText,char* ReturnedText,int maxlen)
{
  printf("Title: %s\nText: %s\nReturned: %s\n",CustomTitle,CustomText,ReturnedText);
  gets(ReturnedText);
  return strncmp(ReturnedText,"cancel",6);
}

void main(int , char *argv[])
{
  typedef int __stdcall FsInit_t(int PluginNr,tProgressProc pProgressProc, tLogProc pLogProc,tRequestProc pRequestProc);
  typedef HANDLE __stdcall FsFindFirst_t(char* Path,WIN32_FIND_DATA *FindData);
  typedef BOOL __stdcall FsFindNext_t(HANDLE Hdl,WIN32_FIND_DATA *FindData);
  typedef int __stdcall FsFindClose_t(HANDLE Hdl);
  typedef int __stdcall FsExecuteFile_t(HWND MainWin,char* RemoteName,char* Verb);
  typedef void __stdcall FsGetDefRootName_t(char* DefRootName,int maxlen);
  typedef void __stdcall FsSetDefaultParams_t(FsDefaultParamStruct* dps);

  FsInit_t *FsInit;
  FsFindFirst_t *FsFindFirst;
  FsFindNext_t *FsFindNext;
  FsFindClose_t *FsFindClose;
  FsExecuteFile_t *FsExecuteFile;
  FsGetDefRootName_t *FsGetDefRootName;
  FsSetDefaultParams_t *FsSetDefaultParams;

  HMODULE wfx = LoadLibrary(argv[1]);

  FsInit = (FsInit_t *) GetProcAddress(wfx, "FsInit");
  FsFindFirst = (FsFindFirst_t *) GetProcAddress(wfx, "FsFindFirst");
  FsFindNext = (FsFindNext_t *) GetProcAddress(wfx, "FsFindNext");
  FsFindClose = (FsFindClose_t *) GetProcAddress(wfx, "FsFindClose");
  FsExecuteFile = (FsExecuteFile_t *) GetProcAddress(wfx, "FsExecuteFile");
  FsGetDefRootName = (FsGetDefRootName_t *) GetProcAddress(wfx, "FsGetDefRootName");
  FsSetDefaultParams = (FsSetDefaultParams_t *) GetProcAddress(wfx, "FsSetDefaultParams");

  if (FsGetDefRootName)
  {
    char root[MAX_PATH];
    FsGetDefRootName(root, MAX_PATH);
    printf("Plugin: %s\n", root);
  }

  FsInit(1,ProgressProc,LogProc,RequestProc);

  if (FsSetDefaultParams)
  {
    FsDefaultParamStruct dps;
    dps.size = sizeof(dps);
    dps.PluginInterfaceVersionLow = 30;
    dps.PluginInterfaceVersionHi = 1;
    char ini[MAX_PATH];
    strcpy(ini,argv[0]);
    strcpy(ini+strlen(ini)-3,"ini");
    strcpy(dps.DefaultIniName, ini);
    FsSetDefaultParams(&dps);
  }

  WIN32_FIND_DATA fd;
  char path[1024];
  strcpy(path,"\\");

  do
  {
    printf("Dir: %s\n",path);
    HANDLE fh = FsFindFirst(path,&fd);
    if (fh != INVALID_HANDLE_VALUE)
    {
      BOOL fn;
      do
      {
        printf("File: %s%s\n",fd.cFileName,fd.dwFileAttributes&0x80000000/*FILE_ATTRIBUTE_DIRECTORY*/?"\\":"");
        fn = FsFindNext(fh, &fd);
      } while (fn);
      FsFindClose(fh);
    }
    char command[1024];
    printf("command: ");
    gets(command);
    if (!strncmp(command,"cd ",3))
    {
      strcpy(path,command+3);
    }
    else if (!strncmp(command,"ad ",3))
    {
      strcat(path,command+3);
    }
    else if (!strncmp(command,"exec ",5))
    {
      if (FsExecuteFile)
      {
        if (FsExecuteFile(GetDesktopWindow(),command+5,"open")==FS_EXEC_SYMLINK)
        {
          strcpy(path,command+5);
        }
      }
    }
    else if (!strncmp(command,"exit",4))
    {
      break;
    }
  } while (1);

  FreeLibrary(wfx);
}