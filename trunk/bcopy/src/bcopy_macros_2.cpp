/*
    bcopy_macros_2.cpp
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

#include <stdio.h>
#include "far_helper.h"
#include "farkeys.hpp"
#include "bcplugin.h"
#include "memory.h"
#include <tchar.h>

static const TCHAR *MacroNames[MACRO_COUNT]={_T("Copy"),_T("Move"),_T("Delete"),_T("Attributes"),_T("Wipe"),_T("Overwrite"),_T("Retry"),_T("Info"),_T("Link"),_T("InfoMenu")};

struct MacroEntry
{
  DWORD Key;
  TCHAR *Seq;
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

static bool add_macro(int index,const TCHAR *KeyIn,const TCHAR *Path)
{
  if(!KeyIn||!*KeyIn) return false;
  LONG KeyOut=FSF.FarNameToKey(KeyIn);
  if(KeyOut==-1) return false;
  //read
  bool res=false;
  TCHAR KeyPath[1024]; HKEY hKey=NULL;
  _tcscpy(KeyPath,Path);
  _tcscat(KeyPath,_T("\\"));
  _tcscat(KeyPath,KeyIn);
  if(RegOpenKeyEx(HKEY_CURRENT_USER,KeyPath,0,KEY_READ,&hKey)==ERROR_SUCCESS)
  {
    DWORD DisableOutput=TRUE,Type,DataSize=0; TCHAR ValueBuffer[1024];
    ValueBuffer[0]=0;
    DataSize=sizeof(DisableOutput);
    RegQueryValueEx(hKey,_T("DisableOutput"),0,&Type,(LPBYTE)&DisableOutput,&DataSize);
    DataSize=sizeof(ValueBuffer);
    RegQueryValueEx(hKey,_T("Sequence"),0,&Type,(LPBYTE)ValueBuffer,&DataSize);
    if(ValueBuffer[0])
    {
      int SeqSize=(int)_tcslen(ValueBuffer)+1; TCHAR *Seq;
      Seq=(TCHAR *)malloc(SeqSize*sizeof(TCHAR));
      if(Seq)
      {
        Seq[0]=0;
        _tcscat(Seq,ValueBuffer);
        MacroEntry *new_macros=(MacroEntry *)realloc(macros[index],sizeof(MacroEntry)*(macro_count[index]+1));
        if(new_macros)
        {
          macros[index]=new_macros;
          macros[index][macro_count[index]].Key=UpperKey(KeyOut);
          macros[index][macro_count[index]].Seq=Seq;
          macros[index][macro_count[index]].DisableOutput=DisableOutput?true:false;
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

void load_macros_2(const TCHAR *registry)
{
  HKEY hKey=NULL; TCHAR path[1024],curr_path[1024];
  _tcscpy(path,registry);
  _tcscat(path,_T("\\KeyMacros\\"));
  for(int i=0;i<MACRO_COUNT;i++)
  {
    _tcscpy(curr_path,path);
    _tcscat(curr_path,MacroNames[i]);
    if(RegOpenKeyEx(HKEY_CURRENT_USER,curr_path,0,KEY_READ,&hKey)==ERROR_SUCCESS)
    {
      TCHAR NameBuffer[1024];
      DWORD j;
      LONG Result;
      for(j=0;;j++)
      {
        Result=RegEnumKey(hKey,j,NameBuffer,ArraySize(NameBuffer));
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
