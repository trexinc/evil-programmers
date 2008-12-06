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

const char *GetMsg(int MsgId)
{
  return Info.GetMsg(Info.ModuleNumber,MsgId);
}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>(int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
  if(Info->StructSize>FAR165_INFO_SIZE)
  {
    IsOldFAR=FALSE;
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;
    MCmpl=new TMenuCompletion(Info->RootKey);
    ACmpl=new TAutoCompletion(Info->RootKey);
    windows=new avl_window_tree();
  }
}

void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  if(!IsOldFAR)
  {
    Info->StructSize=sizeof(*Info);
    Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
    static const char *PluginMenuStrings[1];
    // Text in Plugins Configuration menu
    PluginMenuStrings[0]=GetMsg(MEditCmpl);
    Info->PluginConfigStrings=PluginMenuStrings;
    Info->PluginConfigStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
    // Text in Plugins menu
    Info->PluginMenuStrings=PluginMenuStrings;
    Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  }
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  (void)OpenFrom;
  (void)Item;
  if(!IsOldFAR) ShowMenu();
  return INVALID_HANDLE_VALUE;
}

int WINAPI _export GetMinFarVersion(void)
{
  return MAKEFARVERSION(1,70,1282);
}

int WINAPI _export Configure(int ItemNumber)
{
  if(!IsOldFAR) ShowMenu(2);
  return FALSE;
}

int WINAPI _export ProcessEditorEvent(int Event,void *Param)
{
  if(!IsOldFAR) return ACmpl->ProcessEditorEvent(Event,Param);
  return 0;
}

int WINAPI _export ProcessEditorInput(const INPUT_RECORD *Rec)
{
  if(!IsOldFAR) return ACmpl->ProcessEditorInput(Rec);
  return 0;
}

void WINAPI _export ExitFAR(void)
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
  char Bottom[30];
  FSF.sprintf(Bottom,"[%d]",windows->count());
  FarMenuItem Items[sizeof(Msgs)/sizeof(Msgs[0])];
  for(unsigned int i=0;i<(sizeof(Msgs)/sizeof(Msgs[0]));i++)
  {
    Items[i].Selected=Items[i].Checked=Items[i].Separator=FALSE;
    strcpy(Items[i].Text,GetMsg(Msgs[i]));
  }
  Items[1].Text[0]=0;
  Items[1].Separator=TRUE;
  int MenuCode=0,SelectedItem=0;
  do
  {
    Items[SelectedItem].Selected=TRUE;
    MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,GetMsg(MEditCmpl),Bottom,"Contents",NULL,NULL,&Items[Offset],sizeof(Msgs)/sizeof(Msgs[0])-Offset);
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
