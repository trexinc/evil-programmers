/*
    EditCmpl.cpp
    Copyright (C) 1999 andrey tretjakov
    Copyright (C) 2000-2001 Igor Lyubimov
    Copyright (C) 2002-2008 zg

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

#include "plugin.hpp"
#include "EditCmpl.hpp"
#include "mcmpl.hpp"
#include "acmpl.hpp"
#include "language.hpp"

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void)hDll;
  (void)dwReason;
  (void)lpReserved;
  return TRUE;
}

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
BOOL IsOldFAR;

static TMenuCompletion *MCmpl=NULL;
static TAutoCompletion *ACmpl=NULL;
static int ShowMenu(int Offset=0);

avl_window_tree *windows=NULL;

const TCHAR *GetMsg(int MsgId)
{
  return Info.GetMsg(Info.ModuleNumber,MsgId);
}

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>(int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
#ifndef UNICODE
  if(Info->StructSize>FAR165_INFO_SIZE)
  {
#endif
    IsOldFAR=FALSE;
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;
    MCmpl=new TMenuCompletion(Info->RootKey);
    ACmpl=new TAutoCompletion(Info->RootKey);
    windows=new avl_window_tree();
#ifndef UNICODE
  }
#endif
}

void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
  if(!IsOldFAR)
  {
    Info->StructSize=sizeof(*Info);
    Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
    static const TCHAR *PluginMenuStrings[1];
    // Text in Plugins Configuration menu
    PluginMenuStrings[0]=GetMsg(MEditCmpl);
    Info->PluginConfigStrings=PluginMenuStrings;
    Info->PluginConfigStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
    // Text in Plugins menu
    Info->PluginMenuStrings=PluginMenuStrings;
    Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  }
}

HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom,int Item)
{
  (void)OpenFrom;
  (void)Item;
  if(!IsOldFAR) ShowMenu();
  return INVALID_HANDLE_VALUE;
}

#ifndef UNICODE
int WINAPI EXP_NAME(GetMinFarVersion)(void)
{
  return MAKEFARVERSION(1,70,1282);
}
#endif

int WINAPI EXP_NAME(Configure)(int ItemNumber)
{
  if(!IsOldFAR) ShowMenu(2);
  return FALSE;
}

int WINAPI EXP_NAME(ProcessEditorEvent)(int Event,void *Param)
{
  if(!IsOldFAR) return ACmpl->ProcessEditorEvent(Event,Param);
  return 0;
}

int WINAPI EXP_NAME(ProcessEditorInput)(const INPUT_RECORD *Rec)
{
  if(!IsOldFAR) return ACmpl->ProcessEditorInput(Rec);
  return 0;
}

void WINAPI EXP_NAME(ExitFAR)(void)
{
  if(!IsOldFAR)
  {
    delete ACmpl;
    delete MCmpl;
    delete windows;
  }
}

int ShowMenu(int Offset)
{
  int Msgs[]={MComplete,MComplete,MMenuCfg,MAutoCfg};
  TCHAR Bottom[30];
  FSF.sprintf(Bottom,_T("[%d]"),windows->count());
  FarMenuItem Items[sizeof(Msgs)/sizeof(Msgs[0])];
#ifdef UNICODE
  TCHAR ItemText[sizeof(Msgs)/sizeof(Msgs[0])][128];
#endif
  for(unsigned int i=0;i<(sizeof(Msgs)/sizeof(Msgs[0]));i++)
  {
    Items[i].Selected=Items[i].Checked=Items[i].Separator=FALSE;
#ifdef UNICODE
    _tcscpy(ItemText[i],GetMsg(Msgs[i]));
    Items[i].Text=ItemText[i];
#else
    _tcscpy(Items[i].Text,GetMsg(Msgs[i]));
#endif
  }
#ifdef UNICODE
  ItemText[1][0]=0;
#else
  Items[1].Text[0]=0;
#endif
  Items[1].Separator=TRUE;
  int MenuCode=0,SelectedItem=0;
  do
  {
    Items[SelectedItem].Selected=TRUE;
    MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,GetMsg(MEditCmpl),Bottom,_T("Contents"),NULL,NULL,&Items[Offset],sizeof(Msgs)/sizeof(Msgs[0])-Offset);
    Items[SelectedItem].Selected=FALSE;
    SelectedItem=MenuCode;
    if(MenuCode>=0) MenuCode+=Offset;
    switch(MenuCode)
    {
      case 0: // "OK"
        if(!ACmpl->CompleteWord())
          MCmpl->CompleteWord();
        break;
      case 2: // Menu
        MCmpl->ShowDialog();
        break;
      case 3: // Auto
        ACmpl->ShowDialog();
        break;
    }
  } while(MenuCode>0);
  return TRUE;
}
