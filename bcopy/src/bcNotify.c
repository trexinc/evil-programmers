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
