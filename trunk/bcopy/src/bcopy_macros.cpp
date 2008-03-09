#include <stdio.h>
#include "../../plugin.hpp"
#include "../../farkeys.hpp"
#include "bcplugin.h"
#include "memory.h"

static const char *MacroNames[MACRO_COUNT]={"Copy","Move","Delete","Attributes","Wipe","Overwrite","Retry","Info","Link","InfoMenu"};

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

static bool add_macro(int index,const char *KeyIn,const char *SeqIn)
{
  if(!KeyIn||!*KeyIn) return false;
  if(!SeqIn||!*SeqIn) return false;
  LONG KeyOut=FSF.FarNameToKey(KeyIn);
  if(KeyOut==-1) return false;
  //parse
  bool res=true;
  LONG *keys=NULL;
  char *buffer=NULL;
  unsigned long maxlen=0,key_count=0;

  for(unsigned long last=0;last<2;last++)
  {
    const char *ptr=SeqIn; unsigned long curlen,count=0;
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
      buffer=(char *)malloc(sizeof(DWORD)*(maxlen+1));
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

void load_macros(const char *registry)
{
  HKEY hKey=NULL; char path[1024],curr_path[1024];
  strcpy(path,registry);
  strcat(path,"\\Macros\\");
  for(int i=0;i<MACRO_COUNT;i++)
  {
    strcpy(curr_path,path);
    strcat(curr_path,MacroNames[i]);
    if(RegOpenKeyEx(HKEY_CURRENT_USER,curr_path,0,KEY_READ,&hKey)==ERROR_SUCCESS)
    {
      char NameBuffer[1024],ValueBuffer[1024];
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
  FarBuild=Info.AdvControl(Info.ModuleNumber,ACTL_GETFARVERSION,0);
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
