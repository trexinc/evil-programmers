#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "memory.h"
#include "bcCommon.h"
#include "bcsvc.h"
#include "bcsvc_interface.h"

static HANDLE InfoSynchro;
static struct InfoRec *Info=NULL;
static DWORD InfoCount=0;
static DWORD MaxError=20;
static DWORD ThreadCount=4;
static HANDLE StartEvent=NULL;
static BOOL AllowNetwork=0;
static int WorkPriority=THREAD_PRIORITY_IDLE;
static int HearPriority=THREAD_PRIORITY_HIGHEST;
static LONG QueueSize=0;
static HANDLE QueueSemaphore=NULL;

void InitInfo(void)
{
  HKEY hKey; DWORD Type,DataSize;
  if((RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"SYSTEM\\CurrentControlSet\\Services\\"SVC_NAMEW"\\Parameters",0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
  {
    DataSize=sizeof(MaxError);
    RegQueryValueExW(hKey,L"MaxError",0,&Type,(LPBYTE)&MaxError,&DataSize);
    DataSize=sizeof(ThreadCount);
    RegQueryValueExW(hKey,L"ThreadCount",0,&Type,(LPBYTE)&ThreadCount,&DataSize);
    if((ThreadCount>16)||(ThreadCount<1)) ThreadCount=4;
    DataSize=sizeof(AllowNetwork);
    RegQueryValueExW(hKey,L"AllowNetwork",0,&Type,(LPBYTE)&AllowNetwork,&DataSize);
    DataSize=sizeof(WorkPriority);
    RegQueryValueExW(hKey,L"WorkPriority",0,&Type,(LPBYTE)&WorkPriority,&DataSize);
    DataSize=sizeof(HearPriority);
    RegQueryValueExW(hKey,L"HearPriority",0,&Type,(LPBYTE)&HearPriority,&DataSize);
    DataSize=sizeof(QueueSize);
    RegQueryValueExW(hKey,L"QueueSize",0,&Type,(LPBYTE)&QueueSize,&DataSize);
    RegCloseKey(hKey);
  }
  StartEvent=CreateEventW(NULL,TRUE,FALSE,NULL);
  InfoSynchro=CreateMutex(NULL,FALSE,NULL);
  if(QueueSize) QueueSemaphore=CreateSemaphore(NULL,QueueSize,QueueSize,NULL);
}

void FreeInfo(void)
{
  ResetStartEvent();
  SetAllStopInfo();
  WaitForInfo();
  if(InfoSynchro) CloseHandle(InfoSynchro);
  if(Info) free(Info);
  Info=NULL; InfoCount=0;
  if(QueueSemaphore)
  {
    CloseHandle(QueueSemaphore);
    QueueSemaphore=NULL;
  }
}

void ResetStartEvent(void)
{
  if(StartEvent)
  {
    SetEvent(StartEvent);
    CloseHandle(StartEvent);
    StartEvent=NULL;
  }
}

void WaitStartEvent(BOOL aService)
{
  if(StartEvent)
    while(WaitForSingleObject(StartEvent,3000)!=WAIT_OBJECT_0)
    {
      if(aService) ReportStatusToSCMgr(SERVICE_START_PENDING,NO_ERROR,3000);
    }
}

void AddInfo(DWORD type,const wchar_t *Src,const wchar_t *SrcDir,const wchar_t *DestDir,BOOL InfoEx,struct AdditionalData *add)
{
  DWORD ThreadId=GetCurrentThreadId();
  struct InfoRec *NewInfo;
  if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
  {
    NewInfo=(struct InfoRec *)realloc(Info,sizeof(struct InfoRec)*(InfoCount+1));
    if(NewInfo)
    {
      Info=NewInfo;
      Info[InfoCount].info.ThreadId=ThreadId;
      Info[InfoCount].info.type=type;
      wcscpy(Info[InfoCount].info.Src,Src);
      wcscpy(Info[InfoCount].info.SrcDir,SrcDir);
      wcscpy(Info[InfoCount].info.DestDir,DestDir);
      { //remove file name from DestDir
        wchar_t *fileptr=wcsrchr(Info[InfoCount].info.DestDir,L'\\');
        if(fileptr) fileptr[1]=0;
      }
      Info[InfoCount].info.percent=0;
      Info[InfoCount].info.InfoEx=InfoEx;
      Info[InfoCount].info.Ask=FALSE;
      wcscpy(Info[InfoCount].Src,L"");
      wcscpy(Info[InfoCount].Dest,L"");
      Info[InfoCount].stop=FALSE;
      Info[InfoCount].info.pause=FALSE;
      Info[InfoCount].TotalSize=0;
      Info[InfoCount].CurrentSize=0;
      Info[InfoCount].CurrentSizeAdd=0;
      Info[InfoCount].Errors=0;
      Info[InfoCount].PauseTime=0;
      Info[InfoCount].SizeType=0;
      Info[InfoCount].Flags=0;
      Info[InfoCount].AskID.LowPart=0;
      Info[InfoCount].AskID.HighPart=0;
      Info[InfoCount].info.wait=TRUE;
      if(add)
      {
        Info[InfoCount].CPS=add->CPS;
      }
      else
      {
        Info[InfoCount].CPS=0;
      }
      InfoCount++;
      JobNotify(BCSVC_START_JOB,Src);
    }
    else
    {
      free(Info);
      Info=NULL;
      InfoCount=0;
    }
    ReleaseMutex(InfoSynchro);
  }
}

void UnwaitInfo(void)
{
  DWORD ThreadId=GetCurrentThreadId(); unsigned int i; BOOL Stop=FALSE;
  if(QueueSemaphore)
  {
    while(!Stop)
    {
      if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
      {
        for(i=0;i<InfoCount;i++)
        {
          if(Info[i].info.ThreadId==ThreadId)
          {
            Stop=Info[i].stop;
            Info[i].PauseTime+=2000000ULL;
            break;
          }
        }
        ReleaseMutex(InfoSynchro);
      }
      if(!Stop)
      {
        if(WaitForSingleObject(QueueSemaphore,200)==WAIT_OBJECT_0) break;
      }
    }
  }
  if(!Stop)
    if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
    {
      for(i=0;i<InfoCount;i++)
        if(Info[i].info.ThreadId==ThreadId)
          Info[i].info.wait=FALSE;
      ReleaseMutex(InfoSynchro);
    }
}

void DelInfo(void)
{
  unsigned int i; DWORD ThreadId=GetCurrentThreadId();
  struct InfoRec *NewInfo;
  if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
  {
    for(i=0;i<InfoCount;i++)
    {
      if(Info[i].info.ThreadId==ThreadId)
      {
        if(QueueSemaphore&&!Info[i].info.wait) ReleaseSemaphore(QueueSemaphore,1,NULL);
        JobNotify(BCSVC_STOP_JOB,Info[i].info.Src);
        memmove(Info+i,Info+i+1,sizeof(struct InfoRec)*(InfoCount-i-1));
        NewInfo=(struct InfoRec *)realloc(Info,sizeof(struct InfoRec)*(InfoCount-1));
        if(NewInfo||(InfoCount==1))
        {
          Info=NewInfo;
          InfoCount--;
        }
        else
        {
          free(Info);
          Info=NULL;
          InfoCount=0;
        }
        break;
      }
    }
    ReleaseMutex(InfoSynchro);
  }
}

DWORD GetAllInfo(struct SmallInfoRec **GetInfo)
{
  unsigned int i; DWORD Result=0;
  if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
  {
    if(InfoCount)
    {
      *GetInfo=(struct SmallInfoRec *)malloc(sizeof(struct SmallInfoRec)*InfoCount);
      if(*GetInfo)
      {
        Result=InfoCount;
        for(i=0;i<InfoCount;i++)
          (*GetInfo)[i]=Info[i].info;
      }
    }
    ReleaseMutex(InfoSynchro);
  }
  return Result;
}

void GetInfo(DWORD ThreadId,struct InfoRec *GetInfo)
{
  unsigned int i;
  if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
  {
    for(i=0;i<InfoCount;i++)
    {
      if(Info[i].info.ThreadId==ThreadId)
      {
        *GetInfo=Info[i];
        break;
      }
    }
    ReleaseMutex(InfoSynchro);
  }
}

static void PulseSemaphore(void)
{
  if(QueueSemaphore)
  {
    unsigned int i; DWORD ThreadId=GetCurrentThreadId();
    ReleaseSemaphore(QueueSemaphore,1,NULL);
    for(i=0;i<InfoCount;i++)
      if(Info[i].info.ThreadId==ThreadId)
        Info[i].info.wait=TRUE;
    UnwaitInfo();
  }
}

static void CheckPause(BOOL *Stop)
{
  if(!*Stop)
  {
    unsigned int i; DWORD ThreadId=GetCurrentThreadId(); BOOL wait=TRUE;
    PulseSemaphore();
    while(wait)
    {
      wait=FALSE;
      if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
      {
        for(i=0;i<InfoCount;i++)
        {
          if(Info[i].info.ThreadId==ThreadId)
          {
            *Stop=Info[i].stop;
            wait=Info[i].info.pause&(!Info[i].stop);
            Info[i].PauseTime+=2000000ULL;
            break;
          }
        }
        ReleaseMutex(InfoSynchro);
      }
      Sleep(200);
    }
  }
}

BOOL UpdateInfo(const wchar_t *newSrc,const wchar_t *newDest)
{
  unsigned int i; DWORD ThreadId=GetCurrentThreadId(); BOOL Result=FALSE,Pause=FALSE;
  if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
  {
    for(i=0;i<InfoCount;i++)
    {
      if(Info[i].info.ThreadId==ThreadId)
      {
        Result=Info[i].stop;
        Pause=Info[i].info.pause;
        if(!Result)
        {
          wcscpy(Info[i].Src,newSrc);
          wcscpy(Info[i].Dest,newDest);
        }
        break;
      }
    }
    ReleaseMutex(InfoSynchro);
  }
  if(Pause) CheckPause(&Result);
  return Result;
}

void SetStopInfo(DWORD ThreadId)
{
  unsigned int i;
  if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
  {
    for(i=0;i<InfoCount;i++)
    {
      if(Info[i].info.ThreadId==ThreadId)
      {
        Info[i].stop=TRUE;
        break;
      }
    }
    ReleaseMutex(InfoSynchro);
  }
}

void SetPauseInfo(DWORD ThreadId)
{
  unsigned int i;
  if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
  {
    for(i=0;i<InfoCount;i++)
    {
      if(Info[i].info.ThreadId==ThreadId)
      {
        Info[i].info.pause=!Info[i].info.pause;
        break;
      }
    }
    ReleaseMutex(InfoSynchro);
  }
}

BOOL SetStartInfo(unsigned long long size,unsigned int type)
{
  unsigned int i; DWORD ThreadId=GetCurrentThreadId(); BOOL Result=FALSE,Pause=FALSE;
  if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
  {
    for(i=0;i<InfoCount;i++)
    {
      if(Info[i].info.ThreadId==ThreadId)
      {
        GetLocalTime(&Info[i].StartTime);
        Info[i].PauseTime=0;
        Info[i].TotalSize=size;
        Info[i].SizeType=type;
        Result=Info[i].stop;
        Pause=Info[i].info.pause;
        break;
      }
    }
    ReleaseMutex(InfoSynchro);
  }
  if(Pause) CheckPause(&Result);
  return Result;
}

BOOL IdleInfo(void)
{
  unsigned int i; DWORD ThreadId=GetCurrentThreadId(); BOOL Result=FALSE,Pause=FALSE;
  if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
  {
    for(i=0;i<InfoCount;i++)
    {
      if(Info[i].info.ThreadId==ThreadId)
      {
        Result=Info[i].stop;
        Pause=Info[i].info.pause;
        break;
      }
    }
    ReleaseMutex(InfoSynchro);
  }
  if(Pause) CheckPause(&Result);
  return Result;
}

BOOL UpdatePosInfo(unsigned long long add,unsigned long long subadd)
{
  unsigned int i; DWORD ThreadId=GetCurrentThreadId(),SleepTime=0; BOOL Result=FALSE,Pause=FALSE; double percent;
  if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
  {
    for(i=0;i<InfoCount;i++)
    {
      if(Info[i].info.ThreadId==ThreadId)
      {
        if(Info[i].SizeType)
          Info[i].CurrentSize++;
        else
        {
          Info[i].CurrentSize+=add;
          Info[i].CurrentSizeAdd=subadd;
        }
        if(Info[i].TotalSize>0)
        {
          if(Info[i].TotalSize>=(Info[i].CurrentSize+Info[i].CurrentSizeAdd))
          {
            percent=(Info[i].CurrentSize+Info[i].CurrentSizeAdd);
            percent=percent/Info[i].TotalSize*100;
            Info[i].info.percent=percent;
          }
          else
            Info[i].info.percent=100;
        }
        Result=Info[i].stop;
        Pause=Info[i].info.pause;
        if(!Result&&!Pause&&Info[i].CPS&&Info[i].info.type==INFOTYPE_COPY) //CPS regulation
        {
          SYSTEMTIME CurrTime; FILETIME start,current;
          unsigned long long llstart,llcurrent,lldiff;
          GetLocalTime(&CurrTime);
          SystemTimeToFileTime(&Info[i].StartTime,&start);
          SystemTimeToFileTime(&CurrTime,&current);
          memcpy(&llstart,&start,sizeof(llstart));
          memcpy(&llcurrent,&current,sizeof(llcurrent));
          if(llcurrent<(llstart+Info[i].PauseTime)) llcurrent=llstart+Info[i].PauseTime;
          lldiff=llcurrent-llstart-Info[i].PauseTime;
          if(lldiff>2000000ULL)
          {
            double fAdd=(Info[i].CurrentSize+Info[i].CurrentSizeAdd)*1000.0/Info[i].CPS-lldiff/10000.0;
            if(fAdd>100.0) SleepTime=floor(fAdd);
          }
        }
        break;
      }
    }
    ReleaseMutex(InfoSynchro);
  }
  if(SleepTime)
  {
    DWORD CurrSleepTime;
    while(SleepTime)
    {
      CurrSleepTime=(SleepTime>200)?200:SleepTime;
      SleepTime-=CurrSleepTime;
      Sleep(CurrSleepTime);
      if(SleepTime)
      {
        if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
        {
          for(i=0;i<InfoCount;i++)
          {
            if(Info[i].info.ThreadId==ThreadId)
            {
              Result=Info[i].stop;
              Pause=Info[i].info.pause;
            }
          }
          ReleaseMutex(InfoSynchro);
        }
      }
      if(Result||Pause) break;
    }
  }
  if(Pause) CheckPause(&Result);
  return Result;
}

DWORD CheckErrorInfo(void)
{
  unsigned int i; DWORD ThreadId=GetCurrentThreadId(); DWORD Result=0;
  if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
  {
    for(i=0;i<InfoCount;i++)
    {
      if(Info[i].info.ThreadId==ThreadId)
      {
        Info[i].Errors++;
        if(Info[i].Errors==MaxError) Result=1;
        if(Info[i].Errors>MaxError) Result=2;
        break;
      }
    }
    ReleaseMutex(InfoSynchro);
  }
  return Result;
}

void SetAllStopInfo(void)
{
  unsigned int i;
  if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
  {
    for(i=0;i<InfoCount;i++)
      Info[i].stop=TRUE;
    ReleaseMutex(InfoSynchro);
  }
}

void WaitForInfo(void)
{
  BOOL wait=TRUE;
  while(wait)
  {
    if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
    {
      wait=InfoCount;
      ReleaseMutex(InfoSynchro);
    }
    Sleep(200);
  }
}

DWORD AskUserInfo(DWORD Type,DWORD Error)
{
  unsigned int i; DWORD ThreadId=GetCurrentThreadId(); DWORD Result=ASKTYPE_CANCEL; BOOL Ask=FALSE;
  if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
  {
    for(i=0;i<InfoCount;i++)
    {
      if(Info[i].info.ThreadId==ThreadId)
      {
        Info[i].info.Ask=Type; Ask=TRUE;
        Info[i].Flags=0;
        Info[i].Error=Error;
        AllocateLocallyUniqueId(&Info[i].AskID);
        break;
      }
    }
    ReleaseMutex(InfoSynchro);
  }
  while(Ask)
  {
    Ask=FALSE;
    Sleep(200);
    if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
    {
      for(i=0;i<InfoCount;i++)
      {
        if(Info[i].info.ThreadId==ThreadId)
        {
          Ask=Info[i].info.Ask;
          if(!Ask) Result=Info[i].Flags;
          Info[i].PauseTime+=2000000ULL;
          if(Info[i].stop) Ask=FALSE;
          break;
        }
      }
      ReleaseMutex(InfoSynchro);
    }
  }
  return Result;
}

void SetAskInfo(DWORD ThreadId,DWORD Flags,LUID AskID)
{
  unsigned int i;
  if(WaitForSingleObject(InfoSynchro,INFINITE)!=WAIT_FAILED)
  {
    for(i=0;i<InfoCount;i++)
    {
      if(Info[i].info.ThreadId==ThreadId)
      {
        if(Info[i].info.Ask&&!memcmp(&Info[i].AskID,&AskID,sizeof(AskID)))
        {
          Info[i].Flags=Flags;
          Info[i].info.Ask=FALSE;
          Info[i].AskID.LowPart=0;
          Info[i].AskID.HighPart=0;
        }
        break;
      }
    }
    ReleaseMutex(InfoSynchro);
  }
}

DWORD GetMaxError(void)
{
  return MaxError;
}

DWORD GetThreadCount(void)
{
  return ThreadCount;
}

BOOL GetAllowNetwork(void)
{
  return AllowNetwork;
}

int GetWorkPriority(void)
{
  return WorkPriority;
}

int GetHearPriority(void)
{
  return HearPriority;
}
