#include <windows.h>
#include <tchar.h>
#include "memory.h"

struct CacheRecord
{
  PSID sid;
  TCHAR *username;
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

static const TCHAR *add_sid_cache(TCHAR *computer1,TCHAR *computer2,PSID sid)
{
  const TCHAR *res=_T("N/A");
  const TCHAR *account_unknown=_T("Account Unknown");
  CacheRecord *new_rec=(CacheRecord *)malloc(sizeof(CacheRecord));
  if(new_rec)
  {
    new_rec->sid=(PSID)malloc(GetLengthSid(sid));
    if(new_rec->sid)
    {
      DWORD name_size=0,domain_size=0; SID_NAME_USE type; TCHAR *computer=computer1;
      CopySid(GetLengthSid(sid),new_rec->sid,sid);
add_sid_cache_retry:
      if(!LookupAccountSid(computer,sid,NULL,&name_size,NULL,&domain_size,&type))
      {
        if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
        {
          new_rec->username=(TCHAR *)malloc((name_size+domain_size)*sizeof(TCHAR));
          if(new_rec->username)
          {
            TCHAR *domain_ptr=new_rec->username,*user_ptr=new_rec->username+domain_size;
            if(domain_size==1)
            {
              user_ptr=new_rec->username;
              domain_ptr=new_rec->username+name_size;
            }
            if(LookupAccountSid(computer,sid,user_ptr,&name_size,domain_ptr,&domain_size,&type))
            {
              if(domain_size)
                new_rec->username[domain_size]='\\';
#ifndef UNICODE
              CharToOem(new_rec->username,new_rec->username);
#endif
              goto add_sid_cache_link;
            }
          }
        }
        else if(((GetLastError()==RPC_S_SERVER_UNAVAILABLE)||(GetLastError()==ERROR_NONE_MAPPED))&&(computer==computer1))
        {
          computer=computer2;
          goto add_sid_cache_retry;
        }
        else
        {
          new_rec->username=(TCHAR *)malloc((_tcslen(account_unknown)+1)*sizeof(TCHAR));
          if(new_rec->username)
          {
            _tcscpy(new_rec->username,account_unknown);
add_sid_cache_link:
            new_rec->next=sid_cache;
            sid_cache=new_rec;
            res=new_rec->username;
            goto add_sid_cache_ok;
          }
        }
      }
    }
    free(new_rec->sid);
    free(new_rec->username);
    free(new_rec);
  }
add_sid_cache_ok:
  return res;
}

static TCHAR *get_sid_cache(PSID sid)
{
  TCHAR *res=NULL;
  CacheRecord *tmp_rec=sid_cache;
  while(tmp_rec)
  {
    if(EqualSid(tmp_rec->sid,sid))
    {
      res=tmp_rec->username;
      break;
    }
    tmp_rec=tmp_rec->next;
  }
  return res;
}

const TCHAR *GetUserName(TCHAR *computer,EVENTLOGRECORD *rec)
{
  const TCHAR *res=_T("N/A"); PSID sid=(char*)rec+rec->UserSidOffset; TCHAR *computer2=(TCHAR *)(rec+1);
  computer2+=_tcslen(computer2)+1;
  if(IsValidSid(sid))
  {
    res=get_sid_cache(sid);
    if(!res)
      res=add_sid_cache(computer,computer2,sid);
  }
  return res;
}
