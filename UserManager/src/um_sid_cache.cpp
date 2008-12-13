/*
    um_sid_cache.cpp
    Copyright (C) 2001-2007 zg

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <windows.h>
#include "memory.h"
#include <tchar.h>

struct CacheRecord
{
  PSID sid;
  wchar_t *username;
  wchar_t *username_only;
  TCHAR *username_oem;
  TCHAR *username_only_oem;
  CacheRecord *next;
};

static CacheRecord *sid_cache=NULL;

void free_sid_cache(void)
{
  CacheRecord *tmp_rec;
  while(sid_cache)
  {
    tmp_rec=sid_cache;
    sid_cache=sid_cache->next;
    free(tmp_rec->sid);
    free(tmp_rec->username);
    free(tmp_rec);
  }
};

static void add_sid_cache(wchar_t *computer,PSID sid,bool full,wchar_t **username,TCHAR **username_oem)
{
  *username=(wchar_t*)L"Account Unknown"; *username_oem=(TCHAR*)"Account Unknown";
  CacheRecord *new_rec=(CacheRecord *)malloc(sizeof(CacheRecord));
  if(new_rec)
  {
    new_rec->sid=(PSID)malloc(GetLengthSid(sid));
    if(new_rec->sid)
    {
      DWORD name_size=0,domain_size=0; SID_NAME_USE type;
      CopySid(GetLengthSid(sid),new_rec->sid,sid);
      if(!LookupAccountSidW(computer,sid,NULL,&name_size,NULL,&domain_size,&type))
        if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
        {
          new_rec->username=(wchar_t *)malloc((name_size+domain_size)*(sizeof(wchar_t)+sizeof(TCHAR)));
          if(new_rec->username)
          {
            new_rec->username_oem=(TCHAR *)(new_rec->username+domain_size+name_size);
            wchar_t *domain_ptr=new_rec->username,*user_ptr=new_rec->username+domain_size;
            if(domain_size==1)
            {
              user_ptr=new_rec->username;
              domain_ptr=new_rec->username+name_size;
            }
            if(LookupAccountSidW(computer,sid,user_ptr,&name_size,domain_ptr,&domain_size,&type))
            {
              if(domain_size)
                new_rec->username[domain_size]='\\';
              new_rec->username_only=user_ptr;
              new_rec->next=sid_cache;
              sid_cache=new_rec;
#ifdef UNICODE
              _tcscpy(new_rec->username_oem,new_rec->username);
#else
              WideCharToMultiByte(CP_OEMCP,0,new_rec->username,-1,new_rec->username_oem,wcslen(new_rec->username)+1,NULL,NULL);
#endif
              new_rec->username_only_oem=new_rec->username_oem+(new_rec->username_only-new_rec->username);
              if(full)
              {
                *username=new_rec->username;
                *username_oem=new_rec->username_oem;
              }
              else
              {
                *username=new_rec->username_only;
                *username_oem=new_rec->username_only_oem;
              }
              return;
            }
          }
        }
    }
    free(new_rec->sid);
    free(new_rec->username);
    free(new_rec);
  }
}

static void get_sid_cache(PSID sid,bool full,wchar_t **username,TCHAR **username_oem)
{
  *username=NULL; *username_oem=NULL;
  CacheRecord *tmp_rec=sid_cache;
  while(tmp_rec)
  {
    if(EqualSid(tmp_rec->sid,sid))
    {
      if(full)
      {
        *username=tmp_rec->username;
        *username_oem=tmp_rec->username_oem;
      }
      else
      {
        *username=tmp_rec->username_only;
        *username_oem=tmp_rec->username_only_oem;
      }
      break;
    }
    tmp_rec=tmp_rec->next;
  }
}

void GetUserNameEx(wchar_t *computer,PSID sid,bool full,wchar_t **username,TCHAR **username_oem)
{
  *username=(wchar_t*)L"Account Unknown"; *username_oem=(TCHAR*)_T("Account Unknown");
  if(IsValidSid(sid))
  {
    get_sid_cache(sid,full,username,username_oem);
    if(!(*username))
    {
      add_sid_cache(computer,sid,full,username,username_oem);
    }
  }
}
