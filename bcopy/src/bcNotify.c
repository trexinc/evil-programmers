/*
    bcNotify.c
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

#include <windows.h>
#include "bcsvc_interface.h"
#include "bcCommon.h"
#include "bcsvc.h"

static HMODULE hNotifyDll=NULL;
typedef int (WINAPI *PLUGINNOTIFY)(unsigned long Msg,void *Data);
static PLUGINNOTIFY pNotify=NULL;

void InitNotify(void)
{
  wchar_t filename[MAX_PATH],notifier_filename[MAX_PATH];
  if(GetModuleFileNameW(NULL,filename,sizeofa(filename)))
  {
    wchar_t *filename_ptr;
    DWORD res=GetFullPathNameW(filename,sizeofa(notifier_filename),notifier_filename,&filename_ptr);
    if(res&&(res<sizeofa(notifier_filename))&&filename_ptr)
    {
      wcscpy(filename_ptr,NOTIFY_NAMEW);
      hNotifyDll=LoadLibraryW(notifier_filename);
      if(hNotifyDll!=NULL) pNotify=(PLUGINNOTIFY)GetProcAddress(hNotifyDll,"Notify");
      ServiceNotify(BCSVC_START);
    }
  }
}

void FreeNotify(void)
{
  if(hNotifyDll)
  {
    ServiceNotify(BCSVC_STOP);
    pNotify=NULL;
    FreeLibrary(hNotifyDll);
    hNotifyDll=NULL;
  }
}

void ServiceNotify(unsigned long Code)
{
  if(pNotify) pNotify(Code,NULL);
}

void JobNotify(unsigned long Code,const wchar_t *JobName)
{
  if(pNotify)
  {
    struct BCSvcJob job;
    job.StructSize=sizeof(struct BCSvcJob);
    wcscpy(job.JobName,JobName);
    pNotify(Code,&job);
  }
}
