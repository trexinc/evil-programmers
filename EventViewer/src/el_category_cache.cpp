#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include "memory.h"
#include "evplugin.h"
#include "far_helper.h"

extern FARSTANDARDFUNCTIONS FSF;

struct CacheRecord
{
  TCHAR *source;
  TCHAR *category;
  TCHAR category_bad[6];
  unsigned short category_index;
  bool flag;
  CacheRecord *next;
};

static CacheRecord *category_cache=NULL;

void free_category_cache(void)
{
  CacheRecord *tmp_rec;
  while(category_cache)
  {
    tmp_rec=category_cache;
    category_cache=category_cache->next;
    free(tmp_rec->source);
    free(tmp_rec->category);
    free(tmp_rec);
  }
};

static const TCHAR *add_category_cache(TCHAR *source,unsigned short category_index)
{
  const TCHAR *res=_T("");
  CacheRecord *new_rec=(CacheRecord *)malloc(sizeof(CacheRecord));
  if(new_rec)
  {
    new_rec->category_index=category_index;
    new_rec->flag=false;
    new_rec->source=(TCHAR *)malloc((_tcslen(source)+1)*sizeof(TCHAR));
    if(new_rec->source)
    {
      new_rec->next=category_cache;
      category_cache=new_rec;
      _tcscpy(new_rec->source,source);
      HKEY hKey=NULL;
      TCHAR CatFile[MAX_PATH]; CatFile[0]=0;
      if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,EVENTLOG_KEY,0,KEY_READ,&hKey)==ERROR_SUCCESS)
      {
        TCHAR NameBuffer[MAX_PATH]; LONG Result;
        for(int i=0;;i++)
        {
          Result=RegEnumKey(hKey,i,NameBuffer,ArraySize(NameBuffer));
          if(Result==ERROR_NO_MORE_ITEMS)
            break;
          if(Result==ERROR_SUCCESS)
          {
            TCHAR Key[1024];
            TCHAR CatFileWork[MAX_PATH];
            HKEY hKey2; DWORD Type; DWORD DataSize=0;
            FSF.sprintf(Key,_T("%s\\%s\\%s"),EVENTLOG_KEY,NameBuffer,source);
            if((RegOpenKeyEx(HKEY_LOCAL_MACHINE,Key,0,KEY_QUERY_VALUE,&hKey2))==ERROR_SUCCESS)
            {
              DataSize=sizeof(CatFileWork);
              if(RegQueryValueEx(hKey2,_T("CategoryMessageFile"),0,&Type,(LPBYTE)CatFileWork,&DataSize)==ERROR_SUCCESS)
              {
                ExpandEnvironmentStrings(CatFileWork,CatFile,ArraySize(CatFile));
#ifndef UNICODE
                CharToOem(CatFile,CatFile);
#endif
              }
              RegCloseKey(hKey2);
              break;
            }
          }
        }
        RegCloseKey(hKey);
      }
      if(CatFile[0])
      {
        HINSTANCE lib;
        TCHAR *category; DWORD category_size=0;
        lib=LoadLibrary(CatFile);
        if(lib)
        {
          category_size=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_MAX_WIDTH_MASK,lib,category_index,LANG_NEUTRAL,(TCHAR *)&category,512,NULL);
          FreeLibrary(lib);
        }
        if(category_size)
        {
          new_rec->category=(TCHAR *)malloc((_tcslen(category)+1)*sizeof(TCHAR));
          if(new_rec->category)
          {
            _tcscpy(new_rec->category,category);
#ifndef UNICODE
            CharToOem(new_rec->category,new_rec->category);
#endif
            res=new_rec->category;
            new_rec->flag=true;
          }
          LocalFree(category);
        }
      }
      if(!new_rec->flag)
      {
        FSF.sprintf(new_rec->category_bad,_T("%d"),category_index);
        res=new_rec->category_bad;
      }
    } // new_rec->source
    else free(new_rec);
  }
  return res;
}

static TCHAR *get_category_cache(TCHAR *source,unsigned short category_index)
{
  TCHAR *res=NULL;
  CacheRecord *tmp_rec=category_cache;
  while(tmp_rec)
  {
    if(source)
    {
      if(!_tcsicmp(tmp_rec->source,source))
      {
        if(tmp_rec->flag)
        {
          if(tmp_rec->category_index==category_index)
          {
            res=tmp_rec->category;
            break;
          }
        }
        else
        {
          FSF.sprintf(tmp_rec->category_bad,_T("%d"),category_index);
          res=tmp_rec->category_bad;
          break;
        }
      }
    }
    tmp_rec=tmp_rec->next;
  }
  return res;
}

const TCHAR *GetCategory(EVENTLOGRECORD *rec)
{
  const TCHAR *res=_T(""); TCHAR *source=(TCHAR *)(rec+1); unsigned short category_index=rec->EventCategory;
  res=get_category_cache(source,category_index);
  if(!res)
    res=add_category_cache(source,category_index);
  return res;
}
