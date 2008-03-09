#include <windows.h>
#include <stdio.h>
#include "./bootstrap/bcversion.h"
#include "bcCommon.h"
#include "bcsvc.h"
#include "memory.h"

static SERVICE_STATUS_HANDLE ServiceStatusHandle;
static SERVICE_STATUS ServiceStatus;

static unsigned long __small_printf(char *format,...)
{
  va_list vl; HANDLE screen;
  char buff[1024]; unsigned long transferred=0;
  {
    va_start(vl,format);
    vsprintf(buff,format,vl);
    va_end(vl);
  }
  screen=CreateFileW(L"CONOUT$",GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
  if(screen!=INVALID_HANDLE_VALUE)
  {
    WriteFile(screen,buff,strlen(buff),&transferred,NULL);
    CloseHandle(screen);
  }
  return transferred;
}

BOOL AddEventSupport(void)
{
  HKEY hk;
  DWORD dwData;
  wchar_t filename[MAX_PATH];
  if(!GetModuleFileNameW(NULL,filename,sizeofa(filename)))
    return FALSE;
  /*
   * Add your source name as a subkey under the Application
   * key in the EventLog service portion of the registry.
   */
  if(RegCreateKeyW(HKEY_LOCAL_MACHINE,L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\"SVC_DISPLAYNAMEW,&hk))
    return FALSE;
  /* Add the Event ID message-file name to the subkey. */
  if(RegSetValueExW(hk,L"EventMessageFile",0,REG_EXPAND_SZ,(LPBYTE)filename,(wcslen(filename)+1)*sizeof(wchar_t)))
    return FALSE;
  /* Set the supported types flags. */
  dwData=EVENTLOG_ERROR_TYPE|EVENTLOG_WARNING_TYPE|EVENTLOG_INFORMATION_TYPE;
  if(RegSetValueExW(hk,L"TypesSupported",0,REG_DWORD,(LPBYTE) &dwData,sizeof(DWORD)))
    return FALSE;
  RegCloseKey(hk);
  return TRUE;
}

BOOL ReportStatusToSCMgr(DWORD State,DWORD ExitCode,DWORD Wait)
{
  BOOL ret=TRUE;
  if(State==SERVICE_START_PENDING)
    ServiceStatus.dwControlsAccepted=0;
  else
    ServiceStatus.dwControlsAccepted=SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;
  ServiceStatus.dwCurrentState=State;
  ServiceStatus.dwWin32ExitCode=ExitCode;
  ServiceStatus.dwWaitHint=Wait;
  if((State==SERVICE_RUNNING)||(State==SERVICE_STOPPED))
    ServiceStatus.dwCheckPoint=0;
  else
    ServiceStatus.dwCheckPoint++;
  if(!(ret=SetServiceStatus(ServiceStatusHandle,&ServiceStatus)))
    LogSys(L"SetServiceStatus",NULL,NULL);
  return ret;
}

void WINAPI Handler(DWORD fdwControl)
{
  switch(fdwControl)
  {
    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
      ReportStatusToSCMgr(SERVICE_STOP_PENDING,NO_ERROR,30000);
      ServiceStop();
      ServiceStatus.dwCurrentState=SERVICE_STOPPED;
      break;
  }
  ReportStatusToSCMgr(ServiceStatus.dwCurrentState,NO_ERROR,0);
}

void WINAPI ServiceMain(DWORD dwArgc,LPWSTR *lpszArgv)
{
  (void)dwArgc;
  (void)lpszArgv;
  SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);
  ServiceStatusHandle=RegisterServiceCtrlHandlerW(SVC_NAMEW,Handler);
  if(!ServiceStatusHandle)
  {
    ReportStatusToSCMgr(SERVICE_STOPPED,GetLastError(),0);
    return;
  }
  ServiceStatus.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
  ServiceStatus.dwServiceSpecificExitCode=0;
  ServiceStatus.dwCheckPoint=1;
  if(!ReportStatusToSCMgr(SERVICE_START_PENDING,NO_ERROR,3000))
  {
    ReportStatusToSCMgr(SERVICE_STOPPED,GetLastError(),0);
    return;
  }
  ServiceStart(TRUE);
}


void InstallService(int Type)
{
  wchar_t path[MAX_PATH];
  wchar_t dependencies[2]={0,0};
  SC_HANDLE ServiceControlHandle;
  SC_HANDLE SCManagerHandle;

  if(!AddEventSupport())
    __small_printf("Can't create Event Log support value in registry.\n");

  if(!GetModuleFileNameW(0,path,sizeofa(path)))
  {
    __small_printf("Unable to install "SVC_NAME", GetModuleFileName Fail.\n");
    return;
  }

  SCManagerHandle=OpenSCManagerW(NULL,NULL,SC_MANAGER_ALL_ACCESS);
  if(SCManagerHandle)
  {
    ServiceControlHandle=CreateServiceW(SCManagerHandle,SVC_NAMEW,
      SVC_DISPLAYNAMEW,SERVICE_ALL_ACCESS,SERVICE_WIN32_OWN_PROCESS,
      Type,SERVICE_ERROR_NORMAL,path,NULL,NULL,dependencies,NULL,NULL);
    if(ServiceControlHandle)
    {
      __small_printf("Install Ok.\n");
      CloseServiceHandle(ServiceControlHandle);
    }
    else
      __small_printf("Unable to install "SVC_NAME", CreateService Fail.\n");
    CloseServiceHandle(SCManagerHandle);
  }
  else
  {
    __small_printf("Unable to install "SVC_NAME", OpenSCManager Fail.\n");
  }
}

void UnInstallService(void)
{
  SC_HANDLE ServiceControlHandle;
  SC_HANDLE SCManagerHandle;

  SCManagerHandle=OpenSCManagerW(NULL,NULL,SC_MANAGER_ALL_ACCESS);
  if(SCManagerHandle)
  {
    ServiceControlHandle=OpenServiceW(SCManagerHandle,SVC_NAMEW,SERVICE_ALL_ACCESS);
    if(ServiceControlHandle)
    {
      if(ControlService(ServiceControlHandle,SERVICE_CONTROL_STOP,&ServiceStatus))
      {
        __small_printf("Stopping Service ");
        Sleep(1000);
        while(QueryServiceStatus(ServiceControlHandle,&ServiceStatus))
        {
          if(ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING)
          {
            __small_printf(".");
            Sleep(1000);
          }
          else
            break;
        }
        __small_printf("\n");

        if(ServiceStatus.dwCurrentState==SERVICE_STOPPED)
          __small_printf("Service Stop Now\n");
        else
        {
          CloseServiceHandle(ServiceControlHandle);
          CloseServiceHandle(SCManagerHandle);
          __small_printf("Service Stop Fail\n");
          return;
        }
      }
      if(DeleteService(ServiceControlHandle))
        __small_printf("Service Uninstall Ok.\n");
      else
        __small_printf("DeleteService fail (%ld).\n",GetLastError());
      CloseServiceHandle(ServiceControlHandle);
    }
    else
      __small_printf("OpenService fail (%ld).\n",GetLastError());
    CloseServiceHandle(SCManagerHandle);
  }
  else
    __small_printf("OpenSCManager fail (%ld).\n",GetLastError());
}

int mainCRTStartup()
{
  wchar_t **argv,*prog; int argc;
  SERVICE_TABLE_ENTRYW ste[] = {{SVC_NAMEW,ServiceMain},{NULL,NULL}};
  argv=CommandLineToArgvW(GetCommandLineW(),&argc);
  if(!argv) argc=0;
  if(argc<=1) free(argv);
  if(argc>1)
  {
    if(!_wcsicmp(argv[1],L"/?"))
    {
      prog=wcsrchr(argv[0],L'\\');
      if(!prog) prog=argv[0]; else prog++;
      __small_printf(
        VERSION
        "\n   usage: %ls [flags]\n"
        "   \n"
        "   /I install the service for manual startup\n"
        "   /A install the service for automatic startup\n"
        "   /D install the service to be installed as disabled\n"
        "   /R uninstall the service\n"
        "   /? help\n"
        "   \n",
        prog
      );
    }
    else if(!_wcsicmp(argv[1],L"/I"))
    {
      InstallService(SERVICE_DEMAND_START);
    }
    else if(!_wcsicmp(argv[1],L"/A"))
    {
      InstallService(SERVICE_AUTO_START);
    }
    else if(!_wcsicmp(argv[1],L"/D"))
    {
      InstallService(SERVICE_DISABLED);
    }
    else if(!_wcsicmp(argv[1],L"/R"))
    {
      UnInstallService();
    }
    else if(!_wcsicmp(argv[1],L"/P"))
    {
      ServiceStart(FALSE);
    }
    free(argv);
  }
  else
  {
    StartServiceCtrlDispatcherW(ste);
  }
  return 0;
}
