/*
    bcThreads.c
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
#include <winioctl.h>
#include <stdio.h>
#if defined(_MSC_VER)
#include <cfgmgr32.h>
#endif
#include "memory.h"
#include "bcCommon.h"
#include "bcsvc.h"

static DWORD SetFileAttributesExW(wchar_t *file,DWORD *flags,wchar_t **errname);
static BOOL RemoveDirectoryExW(wchar_t *src,DWORD *flags);
static BOOL SetFilePointerNTEx(HANDLE hFile,LONG lDistanceToMove,LONG lDistanceToMoveHigh,DWORD dwMoveMethod);
static BOOL CreateDirectoryNTExW(const wchar_t *dest);
static BOOL CopyLinkW(const wchar_t *src,const wchar_t *dest);

//ask wrappers fo file functions
#include "bcThreads_wrappers.c"

#define Log(x,y,z) if(!(*flags&COPYFLAG_DONTLOGERRORS)) LogEx(x,y,z)

enum {retError,retOk,retStop};

static unsigned long long MyGetFileSize(wchar_t *file);
static unsigned long long MyGetFileSizeFast(const WIN32_FIND_DATAW find);

DWORD WINAPI CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
                                 LARGE_INTEGER TotalBytesTransferred,
                                 LARGE_INTEGER StreamSize,
                                 LARGE_INTEGER StreamBytesTransferred,
                                 DWORD dwStreamNumber,DWORD dwCallbackReason,
                                 HANDLE hSourceFile,HANDLE hDestinationFile,LPVOID lpData)
{
  DWORD Result=PROGRESS_CONTINUE;
  (void)TotalFileSize; (void)TotalBytesTransferred;
  (void)StreamSize; (void)dwCallbackReason;
  (void)hSourceFile; (void)hDestinationFile; (void)lpData;
  if(dwStreamNumber==1)
    if(UpdatePosInfo(0ULL,(unsigned long long)StreamBytesTransferred.QuadPart))
      Result=PROGRESS_CANCEL;
  return Result;
}

static BOOL CreateDirectoryNTExW(const wchar_t *dest)
{
  if(!CreateDirectoryW(dest,NULL))
  {
    DWORD Error=GetLastError();
    UpdateInfo(dest,L"");
    SetLastError(Error);
    return FALSE;
  }
  return TRUE;
}


static BOOL RemoveDirectoryExW(wchar_t *src,DWORD *flags)
{
  SetFileAttributesW(src,0);
  if(!RemoveDirectoryW(src))
  {
    DWORD Error=GetLastError();
    UpdateInfo(src,L"");
    SetLastError(Error);
    Log(L"RemoveDirectoryW",src,NULL);
    SetLastError(Error);
    return FALSE;
  }
  return TRUE;
}

static BOOL SetFilePointerNTEx(HANDLE hFile,LONG lDistanceToMove,LONG lDistanceToMoveHigh,DWORD dwMoveMethod)
{
  BOOL Result=FALSE;
  if((SetFilePointer(hFile,lDistanceToMove,&lDistanceToMoveHigh,dwMoveMethod)!=0xFFFFFFFFU)||(GetLastError()==NO_ERROR)) Result=TRUE;
  return Result;
}

static BOOL CopyLinkW(const wchar_t *src,const wchar_t *dest)
{
  BOOL Result=FALSE;
  HANDLE In=CreateFileW(src,0,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT,0);
  if(In!=INVALID_HANDLE_VALUE)
  {
    char buffer[2*1024]; DWORD readed,Error=ERROR_SUCCESS;
    if(DeviceIoControl(In,FSCTL_GET_REPARSE_POINT,NULL,0,buffer,sizeof(buffer),&readed,NULL))
    {
      HANDLE Out=CreateFileW(dest,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT,0);
      if(Out!=INVALID_HANDLE_VALUE)
      {
        if(DeviceIoControl(Out,FSCTL_SET_REPARSE_POINT,buffer,readed,NULL,0,&readed,NULL)) Result=TRUE;
        else Error=GetLastError();
        CloseHandle(Out);
      }
    }
    if(!Result&&Error==ERROR_SUCCESS) Error=GetLastError();
    CloseHandle(In);
    SetLastError(Error);
  }
  return Result;
}

static void MyCopySecurityW(wchar_t *src,wchar_t *dest,BOOL dir,const WIN32_FIND_DATAW findin,DWORD *flags)
{
  if(dir)
  {
    HANDLE dest_handle=CreateFileW(dest,FILE_GENERIC_READ|FILE_GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if(dest_handle!=INVALID_HANDLE_VALUE)
    {
      if(findin.dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED)
      {
        unsigned short type=COMPRESSION_FORMAT_DEFAULT; DWORD ret;
        if(!DeviceIoControl(dest_handle,FSCTL_SET_COMPRESSION,&type,sizeof(type),NULL,0,&ret,NULL))
          Log(L"DeviceIoControl",dest,NULL);
      }
      if(!SetFileTime(dest_handle,&findin.ftCreationTime,&findin.ftLastAccessTime,&findin.ftLastWriteTime))
        Log(L"SetFileTime",dest,NULL);
      CloseHandle(dest_handle);
    }
    if(!SetFileAttributesW(dest,findin.dwFileAttributes))
      Log(L"SetFileAttributesW",dest,NULL);
  }
  if((*flags&COPYFLAG_ACCESS)!=0)
  {
    DWORD needed; void *SD;
    if(!GetFileSecurityW(src,DACL_SECURITY_INFORMATION,NULL,0,&needed))
      if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
      {
        SD=malloc(needed);
        if(SD)
        {
          if(GetFileSecurityW(src,DACL_SECURITY_INFORMATION,SD,needed,&needed))
            SetFileSecurityW(dest,DACL_SECURITY_INFORMATION,SD);
          free(SD);
        }
      }
  }
}

#define MY_ATTR     ((*flags&COPYFLAG_ATTR)!=0)
#define MY_ATTRMAIN ((*flags&COPYFLAG_ATTRMAIN)!=0)
#define MY_DELETE   ((*flags&COPYFLAG_DELETE)!=0)
#define MY_MOVE     ((*flags&COPYFLAG_MOVE)!=0)
#define MY_WIPE     ((*flags&COPYFLAG_WIPE)!=0)

#define MY_REMOVE   (MY_MOVE||MY_DELETE)
#define MY_FAIL     ((*flags&COPYFLAG_FAIL)!=0)
#define MY_ROSRC    ((*flags&COPYFLAG_ROSRC)!=0)
#define MY_RODST    ((*flags&COPYFLAG_RODST)!=0)

#define MY_STATISTIC  ((*flags&COPYFLAG_STATISTIC)!=0)
#define MY_COPY_SECURITY \
if((ret==retOk)&&(wcslen(rDestW))&&(rDestW[wcslen(rDestW)-1]==L'\\')) MyCopySecurityW(rSrcW,rDestW,TRUE,find,flags);\
{ \
  if((ret==retOk)&&MY_ATTR) \
  { \
    wchar_t *errname; \
    if(!MySetFileAttributesEx((*rDestW)?rDestW:rSrcW,flags,&errname)) \
      ret=retError; \
  } \
  UpdatePosInfo(0ULL,0ULL); \
}

static DWORD SetFileAttributesExW(wchar_t *file,DWORD *flags,wchar_t **errname)
{
  DWORD Attr,OldAttr,ret=FALSE;
  *errname=L"GetFileAttributesW";
  Attr=GetFileAttributesW(file);
  OldAttr=Attr;
  ret=(Attr!=0xFFFFFFFFU);
  if(ret)
  {
    DWORD curr_attr[]={FILE_ATTRIBUTE_READONLY,FILE_ATTRIBUTE_HIDDEN,FILE_ATTRIBUTE_SYSTEM,FILE_ATTRIBUTE_ARCHIVE};
    DWORD curr_set=COPYFLAG_SET_READONLY,curr_clear=COPYFLAG_CLEAR_READONLY;
    int i;
    for(i=0;i<4;i++,curr_set>>=1,curr_clear>>=1)
    {
      if(*flags&curr_set)
        Attr|=curr_attr[i];
      else if(*flags&curr_clear)
        Attr&=~curr_attr[i];
    }
    if(ret&&((*flags&COPYFLAG_SET_COMPRESSED)||(*flags&COPYFLAG_CLEAR_COMPRESSED))) //process compressed
    {
      HANDLE file_handle;
      if(OldAttr&(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
      {
        OldAttr&=(~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM));
        SetFileAttributesW(file,OldAttr);
      }
      file_handle=CreateFileW(file,FILE_READ_DATA|FILE_WRITE_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
      if(file_handle!=INVALID_HANDLE_VALUE)
      {
        unsigned short type=0xffff; DWORD ret_bytes;
        if(*flags&COPYFLAG_SET_COMPRESSED)
          type=COMPRESSION_FORMAT_DEFAULT;
        else if(*flags&COPYFLAG_CLEAR_COMPRESSED)
          type=COMPRESSION_FORMAT_NONE;
        *errname=L"DeviceIoControl";
        ret=DeviceIoControl(file_handle,FSCTL_SET_COMPRESSION,&type,sizeof(type),NULL,0,&ret_bytes,NULL);
        CloseHandle(file_handle);
      }
    }
    if(ret)
    {
      *errname=L"SetFileAttributesW";
      if(OldAttr!=Attr)
        ret=SetFileAttributesW(file,Attr);
    }
  }
  else if(GetLastError()==ERROR_INVALID_PARAMETER) ret=TRUE;
  return ret;
}

static DWORD CopyCallback(wchar_t *src,wchar_t *dest,DWORD *flags,const WIN32_FIND_DATAW findin)
{
  BOOL bExist=FALSE,bFail=FALSE,bNewer=TRUE;
  WIN32_FIND_DATAW findout; HANDLE hFind;
  BYTE buff[64*1024]; DWORD dReaded,dWritten;
  BOOL bRealROSRC,bRealRODST,bRealHSDST;
  DWORD LocalFlag;
  hFind=FindFirstFileW(dest,&findout);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
    bExist=TRUE;
    if(*flags&COPYFLAG_REFRESH)
      bNewer=(CompareFileTime(&(findin.ftLastWriteTime),&(findout.ftLastWriteTime))==1);
  }
  bRealROSRC=MY_ROSRC&&(findin.dwFileAttributes&FILE_ATTRIBUTE_READONLY)&&(MY_REMOVE);
  bRealRODST=MY_RODST&&(findout.dwFileAttributes&FILE_ATTRIBUTE_READONLY)&&bExist;
  bRealHSDST=(findout.dwFileAttributes&(FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))&&bExist;
  LocalFlag=*flags&(COPYFLAG_OVERWRITE|COPYFLAG_APPEND|COPYFLAG_SKIP|COPYFLAG_REFRESH|COPYFLAG_ASK);
second_start:
  switch(LocalFlag)
  {
    case COPYFLAG_APPEND:
      if(bExist)
      {
        HANDLE hIn=INVALID_HANDLE_VALUE,hOut=INVALID_HANDLE_VALUE;
        unsigned long long subadd=0; DWORD Ok=FALSE;
        if(UpdateInfo(src,dest)) return retStop;
        if(bRealRODST)
          SetFileAttributesW(dest,findout.dwFileAttributes&(~FILE_ATTRIBUTE_READONLY));
        while(!Ok)
        {
          hIn=MyCreateFile(src,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL,flags);
          if(hIn==INVALID_HANDLE_VALUE)
          {
            Log(L"CreateFileW",src,NULL);
            break;
          }
          hOut=MyCreateFile(dest,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL,flags);
          if(hOut==INVALID_HANDLE_VALUE)
          {
            Log(L"CreateFileW",dest,NULL);
            break;
          }
          {
            LARGE_INTEGER write_offset,eof_pos;
            write_offset.LowPart=findout.nFileSizeLow;
            write_offset.HighPart=findout.nFileSizeHigh;
            eof_pos.LowPart=findin.nFileSizeLow;
            eof_pos.HighPart=findin.nFileSizeHigh;
            eof_pos.QuadPart+=write_offset.QuadPart;
            if(MySetFilePointer(hOut,eof_pos.LowPart,eof_pos.HighPart,FILE_BEGIN,flags))
            {
              if(!MySetEndOfFile(hOut,flags))
              {
                Log(L"SetEndOfFile",NULL,NULL);
                break;
              }
            }
            else
            {
              Log(L"SetFilePointer",NULL,NULL);
              break;
            }
          }
          if(MySetFilePointer(hOut,findout.nFileSizeLow,findout.nFileSizeHigh,FILE_BEGIN,flags))
          {
            Ok=TRUE;
            do
            {
              if(UpdatePosInfo(0ULL,subadd)) break;
              if(!MyReadFile(hIn,buff,sizeof(buff),&dReaded,NULL,flags))
              {
                Log(L"ReadFile",src,NULL); Ok=FALSE; break;
              }
              subadd+=dReaded;
              if(!MyWriteFile(hOut,buff,dReaded,&dWritten,NULL,flags))
              {
                Log(L"WriteFile",dest,NULL); Ok=FALSE; break;
              }
            } while(dReaded==sizeof(buff));
          }
          else
          {
            Log(L"SetFilePointer",NULL,NULL);
            break;
          }
        }
        if(bRealRODST)
          SetFileAttributesW(dest,findout.dwFileAttributes);
        if(hIn!=INVALID_HANDLE_VALUE) CloseHandle(hIn);
        if(hOut!=INVALID_HANDLE_VALUE) CloseHandle(hOut);
        if(!Ok) return retError;
        if(MY_REMOVE)
        {
          if(bRealROSRC)
            SetFileAttributesW(src,findin.dwFileAttributes&(~FILE_ATTRIBUTE_READONLY));
          if(!MyDeleteFile(src,flags))
          {
            if(bRealROSRC)
              SetFileAttributesW(src,findin.dwFileAttributes);
            Log(L"DeleteFileW",src,NULL);
            return retError;
          }
        }
        break;
      }
    case COPYFLAG_SKIP:
      if(bExist)
        break;
      bFail=TRUE;
    case COPYFLAG_REFRESH:
      if((!bFail)&&(!bNewer))
        break;
    case COPYFLAG_OVERWRITE:
      {
        DWORD ret=TRUE; BOOL copyCancel=FALSE; int type=0;
        wchar_t *errname=L"MoveFileExW"; BOOL aborted=FALSE;
        if(UpdateInfo(src,dest)) return retStop;
        if(!MY_REMOVE) type=1;
        if(MY_DELETE) type=2;
        if(MY_WIPE) type=3;
        if(MY_ATTRMAIN) type=-1;
        switch(type)
        {
          case 0: //move
            if(bRealRODST)
              SetFileAttributesW(dest,findout.dwFileAttributes&(~FILE_ATTRIBUTE_READONLY));
            ret=MyMoveFileEx(src,dest,bFail?0:MOVEFILE_REPLACE_EXISTING,flags);
            if(ret)
              break;
            {
              DWORD local_err=GetLastError();
              if(bRealRODST) SetFileAttributesW(dest,findout.dwFileAttributes);
              if(local_err!=ERROR_NOT_SAME_DEVICE) break;
            }
          case 1: //copy
            errname=L"CopyFileExW";
            if(bRealRODST||bRealHSDST)
            {
              DWORD new_attr=findout.dwFileAttributes;
              if(bRealRODST)
                new_attr&=(~FILE_ATTRIBUTE_READONLY);
              if(bRealHSDST)
                new_attr&=(~(FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM));
              SetFileAttributesW(dest,new_attr);
            }
            ret=MyCopyFileEx(src,dest,CopyProgressRoutine,NULL,&copyCancel,bFail?COPY_FILE_FAIL_IF_EXISTS:0,flags);
            {
              DWORD local_err=GetLastError();
              if(ret) MyCopySecurityW(src,dest,FALSE,findin,flags);
              else
              {
                if(bRealRODST||bRealHSDST)
                  SetFileAttributesW(dest,findout.dwFileAttributes);
              }
              if((!ret)||type)
              {
                if((!ret)&&(local_err==ERROR_REQUEST_ABORTED))
                {
                  ret=TRUE;
                  aborted=TRUE;
                }
                break;
              }
            }
          case 2: //delete
            errname=L"DeleteFileW";
            if(bRealROSRC)
              SetFileAttributesW(src,findin.dwFileAttributes&(~FILE_ATTRIBUTE_READONLY));
            ret=MyDeleteFile(src,flags);
            if(!ret)
            {
              if(bRealROSRC)
                SetFileAttributesW(src,findin.dwFileAttributes);
            }
            break;
          case 3: //wipe
            errname=L"WipeFileW";
            ret=MyWipeFile(src,flags);
            break;
        }
        if((!aborted)&&ret&&MY_ATTR)
          ret=MySetFileAttributesEx((*dest)?dest:src,flags,&errname);
        if(ret)
          return retOk;
        else
        {
          Log(errname,src,dest);
          return retError;
        }
      }
      break;
    case COPYFLAG_ASK:
      if(bExist)
      {
        DWORD Result;
        if(UpdateInfo(src,dest)) return retStop;
        Result=AskUserInfo(ASKGROUP_OVERWRITE,ERROR_SUCCESS);
        switch(Result&ASKTYPE_MASK)
        {
          case ASKTYPE_CANCEL:
            *flags|=COPYFLAG_FAIL;
            return retError;
          case ASKTYPE_OVERWRITE:
            LocalFlag=COPYFLAG_OVERWRITE;
            break;
          case ASKTYPE_APPEND:
            LocalFlag=COPYFLAG_APPEND;
            break;
          case ASKTYPE_SKIP:
            LocalFlag=COPYFLAG_SKIP;
            break;
          default:
            LocalFlag=COPYFLAG_SKIP;
            break;
        }
        if(Result&ASKFLAG_ALL)
        {
          *flags&=~(COPYFLAG_OVERWRITE|COPYFLAG_APPEND|COPYFLAG_SKIP|COPYFLAG_REFRESH|COPYFLAG_ASK);
          *flags|=LocalFlag;
        }
      }
      else
        LocalFlag=COPYFLAG_OVERWRITE;
      goto second_start;
      break;
  }
  return retOk;
}

#define SafeDirectoryOperation(dirname,function) { BOOL Check=FALSE,Result; if(*flags&COPYFLAG_FAIL_ASK) Check=TRUE; Result=function(dirname,flags); if(Check&&!Result&&MY_FAIL) { ret=retError; break; } }

static DWORD ProccessSymbolicLink(wchar_t *src,wchar_t *dest,DWORD *flags);
static DWORD ScanDir(wchar_t *dir,unsigned long long *size,DWORD type);

static DWORD RecurseDir(wchar_t *dir,wchar_t *dest,DWORD *flags)
{
  WIN32_FIND_DATAW find; HANDLE hFind; DWORD ret=retOk;
  wchar_t findmaskW[MAX_PATH+2];
  wchar_t rSrcW[2*MAX_PATH],rDestW[2*MAX_PATH]=L"";
  void *OldSD=NULL; int correct_SD=FALSE;
  if(MY_WIPE) //try to get full access for directory
  {
    {
      DWORD needed;
      if(!GetFileSecurityW(dir,DACL_SECURITY_INFORMATION,NULL,0,&needed))
        if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
        {
          OldSD=malloc(needed);
          if(OldSD)
            if(GetFileSecurityW(dir,DACL_SECURITY_INFORMATION,OldSD,needed,&needed)) correct_SD=TRUE;
        }
    }
    {
      PSECURITY_DESCRIPTOR SD=(PSECURITY_DESCRIPTOR)malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
      if(SD)
      {
        if(InitializeSecurityDescriptor(SD,SECURITY_DESCRIPTOR_REVISION))
          if(SetSecurityDescriptorDacl(SD,TRUE,NULL,FALSE))
            SetFileSecurityW(dir,DACL_SECURITY_INFORMATION,SD);
        free(SD);
      }
    }
  }
  wcscpy(findmaskW,dir);
  wcscat(findmaskW,L"\\*");
  hFind=FindFirstFileW(findmaskW,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    do
    {
      if(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
      {
        if(wcscmp(find.cFileName,L".")&&wcscmp(find.cFileName,L".."))
        {
          wcscpy(rSrcW,dir);
          wcscat(rSrcW,L"\\");
          wcscat(rSrcW,find.cFileName);
          if(!MY_DELETE)
          {
            if(dest[wcslen(dest)-1]==L'\\')
            {
              wcscpy(rDestW,dest);
              wcscat(rDestW,find.cFileName);
              wcscat(rDestW,L"\\");
              SafeDirectoryOperation(rDestW,MyCreateDirectoryEx)
            }
            else
              wcscpy(rDestW,dest);
          }
          ret=(find.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)?ProccessSymbolicLink(rSrcW,rDestW,flags):RecurseDir(rSrcW,rDestW,flags);
          MY_COPY_SECURITY
          if(((ret==retError)&&MY_FAIL)||(ret==retStop))
            break;
          if(MY_REMOVE) SafeDirectoryOperation(rSrcW,MyRemoveDirectoryEx)
        }
      }
      else
      {
        wcscpy(rSrcW,dir);
        wcscat(rSrcW,L"\\");
        wcscat(rSrcW,find.cFileName);
        if(!MY_DELETE)
        {
          if(dest[wcslen(dest)-1]==L'\\')
          {
            wcscpy(rDestW,dest);
            wcscat(rDestW,find.cFileName);
          }
          else
            wcscpy(rDestW,dest);
        }
        ret=CopyCallback(rSrcW,rDestW,flags,find);
        if(MY_STATISTIC)
          if(UpdatePosInfo(MyGetFileSizeFast(find),0ULL))
            ret=retStop;
        if(((ret==retError)&&MY_FAIL)||(ret==retStop))
          break;
      }
    } while(FindNextFileW(hFind,&find));
    FindClose(hFind);
  }
  if(MY_WIPE) //try to restore old access for directory
  {
    if(correct_SD) SetFileSecurityW(dir,DACL_SECURITY_INFORMATION,OldSD);
    free(OldSD);
  }
  return ret;
}

static DWORD ProccessSymbolicLink(wchar_t *src,wchar_t *dest,DWORD *flags)
{
  DWORD ret=retOk,Result;
  if(!MY_DELETE)
  {
    DWORD LocalFlag=GET_LINK_TYPE(*flags);
second_start:
    switch(LocalFlag)
    {
      case LINK_TYPE_COPY:
        {
          BOOL OldMove=MY_MOVE;
          *flags&=~COPYFLAG_MOVE;
          ret=RecurseDir(src,dest,flags);
          if(OldMove) *flags|=COPYFLAG_MOVE;
        }
        break;
      case LINK_TYPE_LINK:
        if(dest[wcslen(dest)-1]==L'\\')
        {
          if(UpdateInfo(src,dest)) return retStop;
          if(!MyCopyLink(src,dest,flags))
          {
            Log(L"CopyLinkW",src,dest);
            ret=retError;
          }
        }
      case LINK_TYPE_SKIP:
        if(ret==retOk&&MY_STATISTIC)
        {
          unsigned long long size=0;
          ret=ScanDir(src,&size,INFOTYPE_COPY);
          if(ret==retOk&&UpdatePosInfo(size,0ULL)) return retStop;
        }
        break;
      case LINK_TYPE_ASK:
        if(UpdateInfo(src,dest)) return retStop;
        Result=AskUserInfo(ASKGROUP_LINK,ERROR_SUCCESS);
        switch(Result&ASKTYPE_MASK)
        {
          case ASKTYPE_CANCEL:
            *flags|=COPYFLAG_FAIL;
            return retError;
          case ASKTYPE_SKIP:
            LocalFlag=LINK_TYPE_SKIP;
            break;
          case ASKTYPE_COPY:
            LocalFlag=LINK_TYPE_COPY;
            break;
          case ASKTYPE_LINK:
            LocalFlag=LINK_TYPE_LINK;
            break;
          default:
            LocalFlag=LINK_TYPE_SKIP;
            break;
        }
        if(Result&ASKFLAG_ALL) *flags=SET_LINK_TYPE(*flags,LocalFlag);
        goto second_start;
        break;
    }
  }
  return ret;
}

static DWORD ScanDir(wchar_t *dir,unsigned long long *size,DWORD type)
{
  WIN32_FIND_DATAW find; HANDLE hFind;
  wchar_t findmaskW[MAX_PATH+2];
  wchar_t rSrcW[2*MAX_PATH];
  DWORD ret=retOk;
  wcscpy(findmaskW,dir);
  wcscat(findmaskW,L"\\*");
  hFind=FindFirstFileW(findmaskW,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    do
    {
      if(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
      {
        if(wcscmp(find.cFileName,L".")&&wcscmp(find.cFileName,L".."))
        {
          wcscpy(rSrcW,dir);
          wcscat(rSrcW,L"\\");
          wcscat(rSrcW,find.cFileName);
          ret=ScanDir(rSrcW,size,type);
          if(type>=INFOTYPE_DELETE)
            (*size)++;
        }
      }
      else
      {
        if(type<INFOTYPE_DELETE)
          *size+=(unsigned long long)find.nFileSizeLow+(unsigned long long)find.nFileSizeHigh*4294967296ULL;
        else
          (*size)++;
        if(IdleInfo()) ret=retStop;
      }
      if(ret==retStop) break;
    } while(FindNextFileW(hFind,&find));
    FindClose(hFind);
  }
  return ret;
}

static unsigned long long MyGetFileSize(wchar_t *file)
{
  WIN32_FIND_DATAW find; HANDLE hFind;
  hFind=FindFirstFileW(file,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
    return (unsigned long long)find.nFileSizeLow+(unsigned long long)find.nFileSizeHigh*4294967296ULL;
  }
  return 0;
}

static unsigned long long MyGetFileSizeFast(const WIN32_FIND_DATAW find)
{
  return (unsigned long long)find.nFileSizeLow+(unsigned long long)find.nFileSizeHigh*4294967296ULL;
}

static wchar_t *GetLastPathSection(wchar_t *path)
{
  wchar_t *SlashPtr=NULL;
  wchar_t *CurrPtr=path;
  while(*CurrPtr)
  {
    if((*CurrPtr==L'\\')&&(*(CurrPtr+1))) SlashPtr=CurrPtr+1;
    CurrPtr++;
  }
  if(!SlashPtr) SlashPtr=path;
  return SlashPtr;
}

DWORD WINAPI CopyThread(LPVOID lpvThreadParm)
{
  DWORD count,i;
  struct FileRec *files;
  struct FileRec *src;
  struct FileRec *dest;
  struct AdditionalData *add;
  DWORD flags_data,*flags=&flags_data,type=INFOTYPE_COPY,ret=retOk;
  unsigned long long size;
  WIN32_FIND_DATAW find; HANDLE hFind;

  wchar_t rSrcW[2*MAX_PATH],rDestW[2*MAX_PATH];
  HANDLE wait_event;

  wait_event=((struct FileRecs *)lpvThreadParm)->event;
  *flags=((struct FileRecs *)lpvThreadParm)->flags;
  count=((struct FileRecs *)lpvThreadParm)->count;
  files=((struct FileRecs *)lpvThreadParm)->files;
  add=((struct FileRecs *)lpvThreadParm)->add;
  src=(files+count-2);
  dest=(files+count-1);

  //add to info list
  if(MY_REMOVE) type++;
  if(MY_DELETE)
  {
    type++;
    if(!MY_WIPE)
      type++;
  }
  if(MY_ATTRMAIN) type=INFOTYPE_ATTR;
  {
    wchar_t label[2*MAX_PATH];
    wcscpy(label,src->name);
    if(count==3)
      wcscat(label,files[0].name);
    else
      wcscat(label,L"*");
    AddInfo(type,label,src->name,dest->name,MY_STATISTIC,add);
  }
  if(wait_event) SetEvent(wait_event);
  UnwaitInfo();

  //scan dirs for size
  if(MY_STATISTIC)
  {
    size=0;
    for(i=0;i<count-2;i++)
    {
      wcscpy(rSrcW,src->name);
      wcscat(rSrcW,files[i].name);
      if(files[i].type&FILETYPE_FILE)
      {
        if(type<INFOTYPE_DELETE)
          size+=MyGetFileSize(rSrcW);
        else
          size++;
        if(IdleInfo()) ret=retStop;
      }
      else if(files[i].type&FILETYPE_FOLDER)
      {
        ret=ScanDir(rSrcW,&size,type);
        if(type>=INFOTYPE_DELETE)
          size++;
      }
      if(ret==retStop) break;
    }
    if(SetStartInfo(size,(unsigned int)(type>=INFOTYPE_DELETE))) ret=retStop;
  }

  if(ret==retOk)
  {
    rDestW[0]=0; //for delete
    for(i=0;i<count-2;i++)
    {
      wcscpy(rSrcW,src->name);
      wcscat(rSrcW,files[i].name);
      if(files[i].type&FILETYPE_FILE)
      {
        if(!MY_DELETE)
        {
          if(dest->name[wcslen(dest->name)-1]==L'\\')
          {
            wcscpy(rDestW,dest->name);
            if(src->name[0])
              wcscat(rDestW,files[i].name);
            else
              wcscat(rDestW,GetLastPathSection(files[i].name));
          }
          else
            wcscpy(rDestW,dest->name);
        }
        find.nFileSizeLow=0;
        find.nFileSizeHigh=0;
        hFind=MyFindFirstFile(rSrcW,&find,flags);
        if(hFind==INVALID_HANDLE_VALUE)
        {
          Log(L"FindFirstFileW",rSrcW,NULL);
          ret=retError;
        }
        else
        {
          FindClose(hFind);
          ret=CopyCallback(rSrcW,rDestW,flags,find);
        }
        if(MY_STATISTIC)
          if(UpdatePosInfo(MyGetFileSizeFast(find),0ULL))
            ret=retStop;
        if(((ret==retError)&&MY_FAIL)||(ret==retStop))
          break;
      }
      else if(files[i].type&FILETYPE_FOLDER)
      {
        ret=retOk;
        if(!MY_DELETE)
        {
          if(dest->name[wcslen(dest->name)-1]==L'\\')
          {
            wcscpy(rDestW,dest->name);
            if(src->name[0])
              wcscat(rDestW,files[i].name);
            else
              wcscat(rDestW,GetLastPathSection(files[i].name));
            wcscat(rDestW,L"\\");
            if(MY_MOVE) if(MoveFileExW(rSrcW,rDestW,0))
            {
              if(UpdateInfo(rSrcW,rDestW)) ret=retStop;
              goto skip_move;
            }
            SafeDirectoryOperation(rDestW,MyCreateDirectoryEx)
          }
          else
            wcscpy(rDestW,dest->name);
        }

        ret=(files[i].type&FILETYPE_JUNCTION)?ProccessSymbolicLink(rSrcW,rDestW,flags):RecurseDir(rSrcW,rDestW,flags);

        hFind=MyFindFirstFile(rSrcW,&find,flags);
        if(hFind==INVALID_HANDLE_VALUE)
        {
          Log(L"FindFirstFileW",rSrcW,NULL);
          ret=retError;
        }
        else
        {
          FindClose(hFind);
          MY_COPY_SECURITY
        }
        if(MY_REMOVE&&(ret==retOk)) SafeDirectoryOperation(rSrcW,MyRemoveDirectoryEx)
skip_move:
        if(((ret==retError)&&MY_FAIL)||(ret==retStop))
          break;
      }
    }
  }
  if((src->type&FILETYPE_DELETE)&&(ret==retOk))
  {
    if(src->type&FILETYPE_FOLDER)
      MyRemoveDirectoryEx(src->name,flags);
  }
  DelInfo();
  free(lpvThreadParm);
  free(files);
  free(add);
  return 0;
}

#undef SafeDirectoryOperation

#undef MY_DELETE
#undef MY_MOVE
#undef MY_REMOVE
#undef MY_WIPE
#undef MY_FAIL
#undef MY_ROSRC
#undef MY_RODST
#undef MY_STATISTIC
#undef MY_COPY_SECURITY

//ASCII thread. Build structures for CopyThread from params.

DWORD WINAPI PrepareThread(LPVOID lpvThreadParm)
{
  struct StrRec *rec=lpvThreadParm;
  HANDLE hFind; WIN32_FIND_DATAW find;
  DWORD dCount=0;
  struct FileRec *files=NULL;
  struct FileRecs *sfiles=NULL;
  wchar_t SrcW[MAX_PATH]; wchar_t *slash;

  memset(SrcW,0,sizeof(SrcW));

  slash=wcsrchr(rec->from,'\\');
  if((!slash)||(slash<wcsrchr(rec->from,'/')))
    slash=wcsrchr(rec->from,'/');
  if(slash)
    wcsncpy(SrcW,rec->from,slash-rec->from+1);
  hFind=FindFirstFileW(rec->from,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    do
    {
      if(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
      {
        if(wcscmp(find.cFileName,L".")&&wcscmp(find.cFileName,L".."))
        {
          files=(struct FileRec *)realloc(files,sizeof(struct FileRec)*(dCount+1));
          files[dCount].type=FILETYPE_FOLDER;
          if(find.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
            files[dCount].type|=FILETYPE_JUNCTION;
          wcscpy(files[dCount].name,find.cFileName);
          dCount++;
        }
      }
      else
      {
        files=(struct FileRec *)realloc(files,sizeof(struct FileRec)*(dCount+1));
        files[dCount].type=FILETYPE_FILE;
        wcscpy(files[dCount].name,find.cFileName);
        dCount++;
      }
    } while(FindNextFileW(hFind,&find));
    FindClose(hFind);
  }
  if(dCount)
  {
    files=(struct FileRec *)realloc(files,sizeof(struct FileRec)*(dCount+2));
    files[dCount].type=FILETYPE_FOLDER;
    wcscpy(files[dCount].name,SrcW);
    dCount++;

    files[dCount].type=FILETYPE_FOLDER;
    wcscpy(files[dCount].name,rec->to);
    dCount++;

    sfiles=(struct FileRecs *)malloc(sizeof(struct FileRecs));
    sfiles->flags=rec->flags;
    sfiles->count=dCount;
    sfiles->files=files;
    CopyThread(sfiles);
    files=NULL;
    sfiles=NULL;
  }
  free(lpvThreadParm);
  return 0;
}
