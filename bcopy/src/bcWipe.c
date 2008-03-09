#include <stdio.h>
#include <windows.h>
#include "memory.h"
#include "bcCommon.h"
#include "bcsvc.h"

#define BUFF_SIZE (64*1024)

BOOL WipeFileW(wchar_t *filename)
{
  DWORD Error=0,OldAttr,needed; void *SD=NULL; int correct_SD=FALSE;
  wchar_t dir[2*MAX_PATH],tmpname[MAX_PATH],*fileptr=wcsrchr(filename,L'\\');
  unsigned char *buffer=(unsigned char *)malloc(BUFF_SIZE);
  if(fileptr&&buffer)
  {
    OldAttr=GetFileAttributesW(filename);
    SetFileAttributesW(filename,OldAttr&(~FILE_ATTRIBUTE_READONLY));
    if(!GetFileSecurityW(filename,DACL_SECURITY_INFORMATION,NULL,0,&needed))
      if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
      {
        SD=malloc(needed);
        if(SD)
          if(GetFileSecurityW(filename,DACL_SECURITY_INFORMATION,SD,needed,&needed)) correct_SD=TRUE;
      }
    wcsncpy(dir,filename,fileptr-filename+1);
    dir[fileptr-filename+1]=0;
    if(GetTempFileNameW(dir,L"bc",0,tmpname))
    {
      if(MoveFileExW(filename,tmpname,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
      {
        HANDLE f=CreateFileW(tmpname,FILE_GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
        if(f!=INVALID_HANDLE_VALUE)
        {
          BY_HANDLE_FILE_INFORMATION info;
          if(GetFileInformationByHandle(f,&info))
          {
            unsigned long long size=(unsigned long long)info.nFileSizeLow+(unsigned long long)info.nFileSizeHigh*4294967296ULL;
            unsigned long long processed_size=0;
            while(size)
            {
              unsigned long outsize=(unsigned long)((size>=BUFF_SIZE)?BUFF_SIZE:size),transferred;
              WriteFile(f,buffer,outsize,&transferred,NULL);
              size-=outsize;
              processed_size+=outsize;
              if(UpdatePosInfo(0ULL,processed_size)) break;
            }
          }
          if((SetFilePointer(f,0,NULL,FILE_BEGIN)==INVALID_SET_FILE_POINTER)||(!SetEndOfFile(f))) Error=GetLastError();
          CloseHandle(f);
        }
        if(Error) MoveFileExW(tmpname,filename,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
        else if(!DeleteFileW(tmpname)) Error=GetLastError();
      }
      else
      {
        Error=GetLastError();
        DeleteFileW(tmpname);
      }
    } else Error=GetLastError();
    if(Error)
    {
      SetFileAttributesW(filename,OldAttr);
      if(correct_SD) SetFileSecurityW(filename,DACL_SECURITY_INFORMATION,SD);
    }
  }
  free(SD);
  free(buffer);
  if(Error)
  {
    SetLastError(Error);
    return FALSE;
  }
  return TRUE;
}
