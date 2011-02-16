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
#include <initguid.h>
#include "guid.hpp"

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

static TMenuCompletion *MCmpl=NULL;
static TAutoCompletion *ACmpl=NULL;
static int ShowMenu(int Offset=0);

avl_window_tree *windows=NULL;

const TCHAR *GetMsg(int MsgId)
{
  return Info.GetMsg(&MainGuid,MsgId);
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=MAKEFARVERSION(4,0,2);
  Info->Guid=MainGuid;
  Info->Title=L"Word Completion";
  Info->Description=L"Complete/autocomplete words in editor";
  Info->Author=L"Vadim Yegorov";
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>(int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;
  MCmpl=new TMenuCompletion;
  ACmpl=new TAutoCompletion;
  windows=new avl_window_tree();
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
  static const TCHAR *PluginMenuStrings[1];
  // Text in Plugins Configuration menu
  PluginMenuStrings[0]=GetMsg(MEditCmpl);
  Info->PluginConfig.Guids=&MainGuid;
  Info->PluginConfig.Strings=PluginMenuStrings;
  Info->PluginConfig.Count=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  // Text in Plugins menu
  Info->PluginMenu.Guids=&MainGuid;
  Info->PluginMenu.Strings=PluginMenuStrings;
  Info->PluginMenu.Count=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
}

HANDLE WINAPI OpenPluginW(int OpenFrom,const GUID* Guid,int Item)
{
  (void)OpenFrom;
  (void)Item;
  ShowMenu();
  return INVALID_HANDLE_VALUE;
}

int WINAPI ConfigureW(const GUID* Guid)
{
  ShowMenu(2);
  return FALSE;
}

int WINAPI ProcessEditorEventW(int Event,void *Param)
{
  return ACmpl->ProcessEditorEvent(Event,Param);
}

int WINAPI ProcessEditorInputW(const INPUT_RECORD *Rec)
{
  return ACmpl->ProcessEditorInput(Rec);
}

void WINAPI ExitFARW(void)
{
  delete ACmpl;
  delete MCmpl;
  delete windows;
}

int ShowMenu(int Offset)
{
  int Msgs[]={MComplete,MComplete,MMenuCfg,MAutoCfg};
  TCHAR Bottom[30];
  FSF.sprintf(Bottom,_T("[%d]"),windows->count());
  FarMenuItem Items[sizeof(Msgs)/sizeof(Msgs[0])];
  TCHAR ItemText[sizeof(Msgs)/sizeof(Msgs[0])][128];
  for(unsigned int i=0;i<(sizeof(Msgs)/sizeof(Msgs[0]));i++)
  {
    Items[i].Flags=0;
    _tcscpy(ItemText[i],GetMsg(Msgs[i]));
    Items[i].Text=ItemText[i];
  }
  ItemText[1][0]=0;
  Items[1].Flags|=MIF_SEPARATOR;
  int MenuCode=0,SelectedItem=0;
  do
  {
    Items[SelectedItem].Flags|=MIF_SELECTED;
    MenuCode=Info.Menu(&MainGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,GetMsg(MEditCmpl),Bottom,_T("Contents"),NULL,NULL,&Items[Offset],sizeof(Msgs)/sizeof(Msgs[0])-Offset);
    Items[SelectedItem].Flags&=~MIF_SELECTED;
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
