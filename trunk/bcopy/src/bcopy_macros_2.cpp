#include <stdio.h>
#include "../../plugin.hpp"
#include "../../farkeys.hpp"
#include "bcplugin.h"
#include "memory.h"

static const char *MacroNames[MACRO_COUNT]={"Copy","Move","Delete","Attributes","Wipe","Overwrite","Retry","Info","Link","InfoMenu"};

struct MacroEntry
{
  DWORD Key;
  char *Seq;
  bool DisableOutput;
};

static MacroEntry *macros[MACRO_COUNT]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static DWORD macro_count[MACRO_COUNT]={0,0,0,0,0,0,0,0,0,0};

static DWORD UpperKey(DWORD Key)
{
  long result=Key;
  if(FSF.LIsAlpha(Key)) result=FSF.LUpper(Key);
  return result;
}

static bool add_macro(int index,const char *KeyIn,const char *Path)
{
  if(!KeyIn||!*KeyIn) return false;
  LONG KeyOut=FSF.FarNameToKey(KeyIn);
  if(KeyOut==-1) return false;
  //read
  bool res=false;
  char KeyPath[1024]; HKEY hKey=NULL;
  strcpy(KeyPath,Path);
  strcat(KeyPath,"\\");
  strcat(KeyPath,KeyIn);
  if(RegOpenKeyEx(HKEY_CURRENT_USER,KeyPath,0,KEY_READ,&hKey)==ERROR_SUCCESS)
  {
    DWORD DisableOutput=TRUE,Type,DataSize=0; char ValueBuffer[1024];
    ValueBuffer[0]=0;
    DataSize=sizeof(DisableOutput);
    RegQueryValueEx(hKey,"DisableOutput",0,&Type,(LPBYTE)&DisableOutput,&DataSize);
    DataSize=sizeof(ValueBuffer);
    RegQueryValueEx(hKey,"Sequence",0,&Type,(LPBYTE)ValueBuffer,&DataSize);
    if(ValueBuffer[0])
    {
      int SeqSize=strlen(ValueBuffer)+1; char *Seq;
      Seq=(char *)malloc(SeqSize);
      if(Seq)
      {
        Seq[0]=0;
        strcat(Seq,ValueBuffer);
        MacroEntry *new_macros=(MacroEntry *)realloc(macros[index],sizeof(MacroEntry)*(macro_count[index]+1));
        if(new_macros)
        {
          macros[index]=new_macros;
          macros[index][macro_count[index]].Key=UpperKey(KeyOut);
          macros[index][macro_count[index]].Seq=Seq;
          macros[index][macro_count[index]].DisableOutput=DisableOutput;
          macro_count[index]++;
          res=true;
        }
        else free(Seq);
      }
    }
    RegCloseKey(hKey);
  }
  return res;
}

void load_macros_2(const char *registry)
{
  HKEY hKey=NULL; char path[1024],curr_path[1024];
  strcpy(path,registry);
  strcat(path,"\\KeyMacros\\");
  for(int i=0;i<MACRO_COUNT;i++)
  {
    strcpy(curr_path,path);
    strcat(curr_path,MacroNames[i]);
    if(RegOpenKeyEx(HKEY_CURRENT_USER,curr_path,0,KEY_READ,&hKey)==ERROR_SUCCESS)
    {
      char NameBuffer[1024];
      DWORD j;
      LONG Result;
      for(j=0;;j++)
      {
        Result=RegEnumKey(hKey,j,NameBuffer,sizeof(NameBuffer));
        if(Result==ERROR_NO_MORE_ITEMS)
          break;
        if(Result==ERROR_SUCCESS)
        {
          add_macro(i,NameBuffer,curr_path);
        }
        else break;
      }
      RegCloseKey(hKey);
    }
  }
}

void free_macros_2(void)
{
  for(int i=0;i<MACRO_COUNT;i++)
  {
    for(DWORD j=0;j<macro_count[i];j++)
      free(macros[i][j].Seq);
    free(macros[i]); macros[i]=NULL;
    macro_count[i]=0;
  }
}

bool run_macro_2(int index,DWORD Key)
{
  bool result=false;
  Key=UpperKey(Key);
  for(DWORD i=0;i<macro_count[index];i++)
  {
    if(macros[index][i].Key==Key)
    {
      result=true;
      ActlKeyMacro seq;
      seq.Command=MCMD_POSTMACROSTRING;
      seq.Param.PlainText.SequenceText=macros[index][i].Seq;
      seq.Param.PlainText.Flags=0;
      if(macros[index][i].DisableOutput) seq.Param.PlainText.Flags|=KSFLAGS_DISABLEOUTPUT;
      Info.AdvControl(Info.ModuleNumber,ACTL_KEYMACRO,&seq);
      break;
    }
  }
  return result;
}
