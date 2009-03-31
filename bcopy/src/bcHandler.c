/*
    bcHandler.c
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
#include <stdio.h>
#include "memory.h"
#include "bcCommon.h"
#include "bcsvc.h"

static HANDLE hServerStopEvent[LISTEN_THREADS_COUNT]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static HANDLE threads[LISTEN_THREADS_COUNT];
static PSECURITY_DESCRIPTOR pipe_sd=NULL;
static DWORD WINAPI ServiceStartThread(LPVOID lpvThreadParm);
static BOOL IsSystem=FALSE;

#include "bcToken.c"

void ServiceStop(void)
{
  DWORD i;
  for(i=0;i<GetThreadCount();i++)
    if(hServerStopEvent[i])
      SetEvent(hServerStopEvent[i]);
  FreeInfo();
  WaitForMultipleObjects(GetThreadCount(),threads,TRUE,INFINITE);
  for(i=0;i<GetThreadCount();i++)
    CloseHandle(threads[i]);
  FreeNotify();
  free(pipe_sd); pipe_sd=NULL;
}

void ServiceStart(BOOL aService)
{
  DWORD ThreadID,i;
  wchar_t filename[MAX_PATH],access_filename[MAX_PATH];
  InitInfo();
  InitNotify();
  {
    HANDLE token; PTOKEN_USER token_user=NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuthSystem={SECURITY_NT_AUTHORITY}; PSID pSystemSid=NULL;
    if(AllocateAndInitializeSid(&SIDAuthSystem,1,SECURITY_LOCAL_SYSTEM_RID,0,0,0,0,0,0,0,&pSystemSid))
    {
      if(OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token))
      {
        token_user=(PTOKEN_USER)DefaultTokenInformation(token,TokenUser);
        if(token_user)
        {
          if((token_user->User.Sid)&&(pSystemSid)&&(IsValidSid(token_user->User.Sid))&&(IsValidSid(pSystemSid))&&(EqualSid(token_user->User.Sid,pSystemSid)))
            IsSystem=TRUE;
          free(token_user);
        }
        CloseHandle(token);
      }
      FreeSid(pSystemSid);
    }
  }
  //get security from file.
  EnablePrivilege(L"SeSecurityPrivilege");
  if(!pipe_sd&&GetModuleFileNameW(NULL,filename,sizeofa(filename)))
  {
    wchar_t *filename_ptr;
    DWORD res=GetFullPathNameW(filename,sizeofa(access_filename),access_filename,&filename_ptr);
    if(res&&(res<sizeofa(access_filename))&&filename_ptr)
    {
      DWORD needed;
      wcscpy(filename_ptr,ACCESS_NAMEW);
      if(!GetFileSecurityW(access_filename,DACL_SECURITY_INFORMATION|SACL_SECURITY_INFORMATION,NULL,0,&needed))
        if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
        {
          pipe_sd=(PSECURITY_DESCRIPTOR)malloc(needed);
          if(pipe_sd)
          {
            if(!GetFileSecurityW(access_filename,DACL_SECURITY_INFORMATION|SACL_SECURITY_INFORMATION,pipe_sd,needed,&needed))
            {
              free(pipe_sd);
              pipe_sd=NULL;
            }
          }
        }
    }
  }
  //create default security
  if(!pipe_sd)
  {
    PSID pAccessSid=NULL;
    PSID pSystemSid=NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuthLocal={SECURITY_LOCAL_SID_AUTHORITY};
    SID_IDENTIFIER_AUTHORITY SIDAuthEveryone={SECURITY_WORLD_SID_AUTHORITY};
    SID_IDENTIFIER_AUTHORITY SIDAuthSystem={SECURITY_NT_AUTHORITY};
    DWORD sd_size=SECURITY_DESCRIPTOR_MIN_LENGTH+sizeof(ACL);
    PACL pAcl=NULL;

    if(GetAllowNetwork()?AllocateAndInitializeSid(&SIDAuthEveryone,1,SECURITY_WORLD_RID,0,0,0,0,0,0,0,&pAccessSid):AllocateAndInitializeSid(&SIDAuthLocal,1,SECURITY_LOCAL_RID,0,0,0,0,0,0,0,&pAccessSid))
    {
      if(AllocateAndInitializeSid(&SIDAuthSystem,1,SECURITY_LOCAL_SYSTEM_RID,0,0,0,0,0,0,0,&pSystemSid))
      {
        sd_size+=2*(sizeof(ACCESS_ALLOWED_ACE)-sizeof(DWORD))+GetLengthSid(pAccessSid)+GetLengthSid(pSystemSid);
        pipe_sd=(PSECURITY_DESCRIPTOR)malloc(sd_size);
        if(pipe_sd)
        {
          pAcl=(PACL)(((char *)pipe_sd)+SECURITY_DESCRIPTOR_MIN_LENGTH);
          if(!(InitializeAcl(pAcl,sd_size-SECURITY_DESCRIPTOR_MIN_LENGTH,ACL_REVISION)&&AddAccessAllowedAce(pAcl,ACL_REVISION,FILE_ALL_ACCESS,pAccessSid)&&AddAccessAllowedAce(pAcl,ACL_REVISION,FILE_ALL_ACCESS,pSystemSid)&&InitializeSecurityDescriptor(pipe_sd,SECURITY_DESCRIPTOR_REVISION)&&SetSecurityDescriptorDacl(pipe_sd,TRUE,pAcl,FALSE)))
          {
            free(pipe_sd);
            pipe_sd=NULL;
          }
        }
        FreeSid(pSystemSid);
      }
      FreeSid(pAccessSid);
    }
  }
  for(i=0;i<GetThreadCount();i++)
  {
    threads[i]=CreateThread(NULL,0,ServiceStartThread,(void *)(DWORD_PTR)i,CREATE_SUSPENDED,&ThreadID);
    if(threads[i])
    {
      SetThreadPriority(threads[i],GetHearPriority());
      ResumeThread(threads[i]);
    }
  }
  WaitStartEvent(aService);
  if(aService) ReportStatusToSCMgr(SERVICE_RUNNING,NO_ERROR,0);
}

#define TRANSFER_INIT \
memset(&os,0,sizeof(OVERLAPPED)); \
os.hEvent=hEvents[1]; \
ResetEvent(hEvents[1]);
#define TRANSFER_DATAU(TYPE,DATA,SIZE) \
TRANSFER_INIT \
if(!TYPE(hPipe,DATA,SIZE,&transferred,&os)) \
{ \
  if(GetLastError()==ERROR_IO_PENDING) \
  { \
    if(WaitForMultipleObjects(2,hEvents,FALSE,INFINITE)!=WAIT_OBJECT_0+1) \
      break; \
    if(!GetOverlappedResult(hPipe,&os,&transferred,FALSE)) continue; \
  } \
  else continue; \
}
#define TRANSFER_DATA(TYPE,DATA,SIZE) \
TRANSFER_DATAU(TYPE,DATA,SIZE) \
if(SIZE!=transferred) continue;

static DWORD WINAPI ServiceStartThread(LPVOID lpvThreadParm)
{
  HANDLE hPipe=INVALID_HANDLE_VALUE;
  HANDLE hEvents[2]={NULL,NULL};
  OVERLAPPED os;
  SECURITY_ATTRIBUTES sa;
  DWORD transferred;
  DWORD dCode;
  DWORD dFlags;
  DWORD dCount;
  struct FileRec *files=NULL;
  struct FileRecs *sfiles=NULL;
  void *add_data=NULL;
  struct SmallInfoRec *alldata=NULL;
  struct StrRec *rec=NULL;
  HANDLE hThread;
  DWORD ThreadID;
  BOOL connected=FALSE;
  DWORD index=(DWORD)(DWORD_PTR)lpvThreadParm;

  // Service initialization
  hServerStopEvent[index]=CreateEventW(NULL,TRUE,FALSE,NULL);
  if(!hServerStopEvent[index])
    goto cleanup;
  hEvents[0]=hServerStopEvent[index];
  // create the event object object use in overlapped i/o
  hEvents[1]=CreateEventW(NULL,TRUE,FALSE,NULL);
  if(!hEvents[1])
    goto cleanup;
  sa.nLength=sizeof(sa);
  sa.lpSecurityDescriptor=pipe_sd;
  sa.bInheritHandle=FALSE;
  // open our named pipe...
  hPipe=CreateNamedPipeW(PIPE_NAMEW,FILE_FLAG_OVERLAPPED|PIPE_ACCESS_DUPLEX,PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT,GetThreadCount(),0,0,1000,&sa);
  if(hPipe==INVALID_HANDLE_VALUE)
    goto cleanup;
  // End of initialization
  if(!index) ResetStartEvent();
  // Service is now running, perform work until shutdown
  for(;;connected?(DisconnectNamedPipe(hPipe),connected=FALSE):0)
  {
    //check stack
    if(!HeapValidate(GetProcessHeap(),0,NULL)) { LogSys(L"HeapValidate",NULL,NULL); break; }
    //clean up
    if(files) { free(files); files=NULL; }
    if(sfiles) { free(sfiles); sfiles=NULL; }
    if(add_data) { free(add_data); add_data=NULL; }
    if(rec) { free(rec); rec=NULL; }
    if(alldata) { free(alldata); alldata=NULL; }
    TRANSFER_INIT
    if(!ConnectNamedPipe(hPipe,&os))
    {
      DWORD Error=GetLastError();
      if(Error==ERROR_IO_PENDING)
      {
        if(WaitForMultipleObjects(2,hEvents,FALSE,INFINITE)!=WAIT_OBJECT_0+1)
          break;
        if(!GetOverlappedResult(hPipe,&os,&transferred,FALSE)) Error=GetLastError();
        else Error=ERROR_SUCCESS;
      }
      if(Error!=ERROR_PIPE_CONNECTED&&Error!=ERROR_SUCCESS)
      {
        if(WaitForSingleObject(hEvents[0],200)==WAIT_OBJECT_0) break;
        continue;
      }
    }
    connected=TRUE;
    // grab whatever's coming through the pipe...
    TRANSFER_DATA(ReadFile,&dCode,sizeof(dCode))
    TRANSFER_DATA(ReadFile,&dFlags,sizeof(dFlags))
    //check network
    if(dCode==OPERATION_COPY)
    {
      if(dFlags&COPYFLAG_ADDITIONAL_DATA)
      {
        DWORD dAddSize;
        TRANSFER_DATA(ReadFile,&dAddSize,sizeof(dAddSize))
        add_data=malloc(dAddSize);
        if(add_data)
        {
          TRANSFER_DATA(ReadFile,add_data,dAddSize)
        }
        else continue;
      }
      TRANSFER_DATA(ReadFile,&dCount,sizeof(dCount))
      if(dCount>2)
      {
        files=(struct FileRec *)malloc(sizeof(struct FileRec)*dCount);
        if(files)
        {
          TRANSFER_DATA(ReadFile,files,sizeof(struct FileRec)*dCount)
          sfiles=(struct FileRecs *)malloc(sizeof(struct FileRecs));
          if(sfiles)
          {
            HANDLE wait_event=CreateEventW(NULL,TRUE,FALSE,NULL);
            sfiles->event=wait_event;
            sfiles->flags=dFlags;
            sfiles->count=dCount;
            sfiles->files=files;
            sfiles->add=add_data;
            hThread=CreateThread(NULL,0,CopyThread,sfiles,CREATE_SUSPENDED,&ThreadID);
            if(hThread)
            {
              SetThreadPriority(hThread,GetWorkPriority());
              if(IsSystem) PipeToThread(hPipe,hThread);
              ResumeThread(hThread);
              if(wait_event)
              {
                struct InfoRec RetData;
                WaitForSingleObject(wait_event,INFINITE);
                CloseHandle(wait_event);
                RetData.info.ThreadId=ThreadID;
                GetInfo(ThreadID,&RetData);
                TRANSFER_DATA(WriteFile,&RetData.info,sizeof(RetData.info))
              }
              CloseHandle(hThread);
              files=NULL;
              sfiles=NULL;
              add_data=NULL;
            } else CloseHandle(wait_event);
          }
        }
      }
    }
    else if(dCode==OPERATION_INFO)
    {
      if(dFlags&INFOFLAG_ALL)
      {
        DWORD size,rec_size=sizeof(struct SmallInfoRec);
        size=GetAllInfo(&alldata);
        // send it back out...
        TRANSFER_DATA(WriteFile,&size,sizeof(size))
        TRANSFER_DATA(WriteFile,&rec_size,sizeof(rec_size))
        if(size)
          TRANSFER_DATA(WriteFile,alldata,sizeof(struct SmallInfoRec)*size)
      }
      else if(dFlags&(INFOFLAG_BYHANDLE|INFOFLAG_STOP|INFOFLAG_PAUSE))
      {
        DWORD ThreadId;
        struct InfoRec data;
        TRANSFER_DATA(ReadFile,&ThreadId,sizeof(ThreadId))
        if(dFlags&INFOFLAG_STOP)
          SetStopInfo(ThreadId);
        else if(dFlags&INFOFLAG_BYHANDLE)
        {
          DWORD rec_size=sizeof(data);
          data.info.type=INFOTYPE_INVALID;
          GetInfo(ThreadId,&data);
          // send it back out...
          TRANSFER_DATA(WriteFile,&rec_size,sizeof(rec_size))
          TRANSFER_DATA(WriteFile,&data,sizeof(data))
        }
        else if(dFlags&INFOFLAG_PAUSE)
          SetPauseInfo(ThreadId);
      }
    }
    else if((dCode==OPERATION_PWD)&&(dFlags&(PWDFLAG_CLEAR|PWDFLAG_SET)))
    {
      DWORD result_error=ERROR_SUCCESS;
      LogSys(L"Obsolete",NULL,NULL);
      TRANSFER_DATA(WriteFile,&result_error,sizeof(result_error))
    }
    else if(dCode==OPERATION_ANSWER)
    {
      DWORD ThreadId; LUID AskID;
      TRANSFER_DATA(ReadFile,&ThreadId,sizeof(ThreadId))
      TRANSFER_DATA(ReadFile,&AskID,sizeof(AskID))
      SetAskInfo(ThreadId,dFlags,AskID);
    }
    else if(dCode==OPERATION_ASCII)
    {
      char paramsA[1024];
      wchar_t paramsW[1024];
      wchar_t **argv; int argc; unsigned int i,j;
      DWORD ExcludeMask=~(COPYFLAG_OVERWRITE|COPYFLAG_APPEND|COPYFLAG_SKIP|COPYFLAG_REFRESH);
      memset(paramsA,0,sizeof(paramsA));
      *(int *)paramsA=dFlags;

      TRANSFER_DATAU(ReadFile,paramsA+sizeof(dFlags),sizeof(paramsA)-sizeof(dFlags))
      for(i=0;i<strlen(paramsA);i++)
        if(paramsA[i]<' ')
          paramsA[i]=' ';
      MultiByteToWideChar(CP_OEMCP,0,paramsA,-1,paramsW,sizeof(paramsW)/sizeof(paramsW[0]));
      rec=(struct StrRec *)malloc(sizeof(struct StrRec));
      if(rec)
      {
        memset(rec,0,sizeof(struct StrRec));
        rec->flags|=COPYFLAG_SKIP;
        argv=CommandLineToArgvW(paramsW,&argc);
        if(argv)
        {
          for(i=0;i<(unsigned int)argc;i++)
          {
            if(argv[i][0]==L'/')
            {
              for(j=1;j<wcslen(argv[i]);j++)
              {
                switch(argv[i][j])
                {
                  case STRFLAG_STATISTIC:
                    rec->flags|=COPYFLAG_STATISTIC;
                    break;
                  case STRFLAG_ACCESS:
                    rec->flags|=COPYFLAG_ACCESS;
                    break;
                  case STRFLAG_MOVE:
                    rec->flags|=COPYFLAG_MOVE;
                    break;
                  case STRFLAG_FAIL:
                    rec->flags|=COPYFLAG_FAIL;
                    break;
                  case STRFLAG_ROSRC:
                    rec->flags|=COPYFLAG_ROSRC;
                    break;
                  case STRFLAG_RODST:
                    rec->flags|=COPYFLAG_RODST;
                    break;
                  case STRFLAG_OVERWRITE:
                    rec->flags&=ExcludeMask;
                    rec->flags|=COPYFLAG_OVERWRITE;
                    break;
                  case STRFLAG_APPEND:
                    rec->flags&=ExcludeMask;
                    rec->flags|=COPYFLAG_APPEND;
                    break;
                  case STRFLAG_REFRESH:
                    rec->flags&=ExcludeMask;
                    rec->flags|=COPYFLAG_REFRESH;
                    break;
                }
              }
            }
            else
            {
              if(!(rec->from[0]))
                wcsncpy(rec->from,argv[i],MAX_PATH-1);
              else if(!(rec->to[0]))
                wcsncpy(rec->to,argv[i],MAX_PATH-1);
            }
          }
        }
        if(!(rec->to[0]))
        {
          rec->flags&=~COPYFLAG_MOVE;
          rec->flags|=COPYFLAG_DELETE;
        }
        if(rec->from[0])
        {
          hThread=CreateThread(NULL,0,PrepareThread,rec,CREATE_SUSPENDED,&ThreadID);
          SetThreadPriority(hThread,GetWorkPriority());
          if(IsSystem) PipeToThread(hPipe,hThread);
          ResumeThread(hThread);
          CloseHandle(hThread);
          rec=NULL;
        }
      }
    }
  }
  if(connected) {DisconnectNamedPipe(hPipe); connected=FALSE;}
  //clean up
  if(files) { free(files); files=NULL; }
  if(sfiles) { free(sfiles); sfiles=NULL; }
  if(alldata) { free(alldata); alldata=NULL; }
  if(rec) { free(rec); rec=NULL; }
cleanup:
  if(hPipe!=INVALID_HANDLE_VALUE)
    CloseHandle(hPipe);
  if(hServerStopEvent[index])
    CloseHandle(hServerStopEvent[index]);
  if(hEvents[1]) // overlapped i/o event
    CloseHandle(hEvents[1]);
  if(files)
    free(files);
  return 0;
}
#undef TRANSFER_DATA
#undef TRANSFER_DATAU
#undef TRANSFER_INIT
