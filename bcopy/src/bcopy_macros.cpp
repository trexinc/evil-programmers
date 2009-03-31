#include <stdio.h>
#include "plugin.hpp"
#include "farkeys.hpp"
#include "bcplugin.h"
#include "memory.h"
#include <tchar.h>

static const TCHAR *MacroNames[MACRO_COUNT]={_T("Copy"),_T("Move"),_T("Delete"),_T("Attributes"),_T("Wipe"),_T("Overwrite"),_T("Retry"),_T("Info"),_T("Link"),_T("InfoMenu")};

struct MacroEntry
{
  DWORD Key;
  DWORD *Seq;
  DWORD Count;
};

static MacroEntry *macros[MACRO_COUNT]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static DWORD macro_count[MACRO_COUNT]={0,0,0,0,0,0,0,0,0,0};

static DWORD UpperKey(DWORD Key)
{
  long result=Key;
  if(FSF.LIsAlpha(Key)) result=FSF.LUpper(Key);
  return result;
}

static bool add_macro(int index,const TCHAR *KeyIn,const TCHAR *SeqIn)
{
  if(!KeyIn||!*KeyIn) return false;
  if(!SeqIn||!*SeqIn) return false;
  LONG KeyOut=FSF.FarNameToKey(KeyIn);
  if(KeyOut==-1) return false;
  //parse
  bool res=true;
  LONG *keys=NULL;
  TCHAR *buffer=NULL;
  unsigned long maxlen=0,key_count=0;

  for(unsigned long last=0;last<2;last++)
  {
    const TCHAR *ptr=SeqIn; unsigned long curlen,count=0;
    while(*ptr)
    {
      while(*ptr==' '||*ptr=='\t') ptr++;
      if(!*ptr) break;
      curlen=0;
      while(*ptr&&*ptr!=' '&&*ptr!='\t')
      {
        if(last) buffer[curlen]=*ptr;
        curlen++;
        ptr++;
      }
      if(last)
      {
        buffer[curlen]=0;
        keys[count]=FSF.FarNameToKey(buffer);
        if(keys[count]==-1)
        {
          res=false;
          break;
        }
      }
      else
        if(curlen>maxlen) maxlen=curlen;
      count++;
    }
    if(!last)
    {
      key_count=count;
      keys=(LONG *)malloc(sizeof(DWORD)*key_count);
      buffer=(TCHAR *)malloc(sizeof(DWORD)*(maxlen+1));
      if(!(keys&&buffer)) res=false;
    }
    if(!res) break;
  }
  if(buffer) free(buffer);
  if(res)
  {
    MacroEntry *new_macros=(MacroEntry *)realloc(macros[index],sizeof(MacroEntry)*(macro_count[index]+1));
    if(new_macros)
    {
      macros[index]=new_macros;
      macros[index][macro_count[index]].Key=UpperKey(KeyOut);
      macros[index][macro_count[index]].Seq=(DWORD *)keys;
      macros[index][macro_count[index]].Count=key_count;
      macro_count[index]++;
    }
    else res=false;
  }
  if(!res&&keys) free(keys);
  return res;
}

static unsigned long FarBuild=0;

void load_macros(const TCHAR *registry)
{
  HKEY hKey=NULL; TCHAR path[1024],curr_path[1024];
  _tcscpy(path,registry);
  _tcscat(path,_T("\\Macros\\"));
  for(int i=0;i<MACRO_COUNT;i++)
  {
    _tcscpy(curr_path,path);
    _tcscat(curr_path,MacroNames[i]);
    if(RegOpenKeyEx(HKEY_CURRENT_USER,curr_path,0,KEY_READ,&hKey)==ERROR_SUCCESS)
    {
      TCHAR NameBuffer[1024],ValueBuffer[1024];
      DWORD j,NameSize,ValueSize;
      LONG Result;
      for(j=0;;j++)
      {
        NameSize=sizeof(NameBuffer);
        ValueSize=sizeof(ValueBuffer);
        Result=RegEnumValue(hKey,j,NameBuffer,&NameSize,NULL,NULL,(LPBYTE)ValueBuffer,&ValueSize);
        if(Result==ERROR_NO_MORE_ITEMS)
          break;
        if(Result==ERROR_SUCCESS) add_macro(i,NameBuffer,ValueBuffer);
        else break;
      }
      RegCloseKey(hKey);
    }
  }
  FarBuild=(unsigned long)Info.AdvControl(Info.ModuleNumber,ACTL_GETFARVERSION,0);
  FarBuild=HIWORD(FarBuild);
}

void free_macros(void)
{
  for(int i=0;i<MACRO_COUNT;i++)
  {
    for(DWORD j=0;j<macro_count[i];j++)
      free(macros[i][j].Seq);
    free(macros[i]); macros[i]=NULL;
    macro_count[i]=0;
  }
}

bool run_macro(int index,DWORD Key) //FIXME: remove after far release
{
  bool result=false;
  if(FarBuild>=1700)
  {
    result=run_macro_2(index,Key);
  }
  if(!result)
  {
    Key=UpperKey(Key);
    for(DWORD i=0;i<macro_count[index];i++)
    {
      if(macros[index][i].Key==Key)
      {
        result=true;
        KeySequence seq={KSFLAGS_DISABLEOUTPUT,macros[index][i].Count,macros[index][i].Seq};
        Info.AdvControl(Info.ModuleNumber,ACTL_POSTKEYSEQUENCE,&seq);
        break;
      }
    }
  }
  return result;
}
