/*
    DialogManager plugin for FAR Manager
    Copyright (C) 2003 Vadim Yegorov
    Copyright (C) 2004-2005 Vadim Yegorov and Alex Yaroslavsky

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <stdio.h>
#include <stdlib.h>
#include "plugin.hpp"
#include "farkeys.hpp"
#include "dialogM.hpp"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
char PluginRootKey[100];
char PluginHotkeyKey[100];
char PluginPluginsRootKey[100];
BOOL IsOldFAR=TRUE;
DWORD ShowInConfig=TRUE;
DWORD ShowInPanels=TRUE;
DWORD ShowInConfigReg=TRUE;
DWORD ShowInPanelsReg=TRUE;
int HotKey=KEY_CTRLF11;
int DebugKey=-1;
HANDLE DialogMutex=NULL;

//patch data
long first_data=0;
long second_data=0;
long first_new_data=0;
long second_new_data=0;
long old_ip;
void *code_ptr=0;

long Param1;
long Param2;
long Param3;
long Param4;

static void patch(void *ptr);
static void unpatch(void);

#ifdef __cplusplus
extern "C"{
#endif
long WINAPI DialogHook(HANDLE hDlg,int Msg,int Param1,long Param2,long Result);
#ifdef __cplusplus
};
#endif

extern long Invoke_Patch;

static void UpdateShow(void)
{
  ShowInConfig=ShowInConfigReg;
  ShowInPanels=ShowInPanelsReg;
  if(!PluginCount(FMMSG_CONFIG)) ShowInConfig=FALSE;
  if(!PluginCount(FMMSG_PANEL)) ShowInPanels=FALSE;
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
    strcpy(PluginRootKey,Info->RootKey);
    strcat(PluginRootKey,"\\DialogM");
    strcpy(PluginHotkeyKey,PluginRootKey);
    strcat(PluginHotkeyKey,"\\PluginHotkeys");
    strcpy(PluginPluginsRootKey,PluginRootKey);
    strcat(PluginPluginsRootKey,"\\Plugins");
    HKEY hKey;
    if((RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
    {
      DWORD Type,DataSize=sizeof(ShowInConfigReg); char Key[256];
      RegQueryValueEx(hKey,"ShowInConfig",0,&Type,(LPBYTE)&ShowInConfigReg,&DataSize);
      DataSize=sizeof(ShowInPanelsReg);
      RegQueryValueEx(hKey,"ShowInPanels",0,&Type,(LPBYTE)&ShowInPanelsReg,&DataSize);
      DataSize=sizeof(Key);
      if(RegQueryValueEx(hKey,"HotKey",0,&Type,(LPBYTE)&Key,&DataSize)==ERROR_SUCCESS) HotKey=FSF.FarNameToKey(Key);
      DataSize=sizeof(Key);
      if(RegQueryValueEx(hKey,"DebugKey",0,&Type,(LPBYTE)&Key,&DataSize)==ERROR_SUCCESS) DebugKey=FSF.FarNameToKey(Key);
      RegCloseKey(hKey);
    }
    DialogMutex=CreateMutex(NULL,FALSE,NULL);
    if(DialogMutex)
    {
      unsigned char *ptr=(unsigned char *)Info->DefDlgProc;
      for(int i=0;i<256&&ptr[i]!=0xC2;i++)
        if(ptr[i]==0xE8||ptr[i]==0xE9)
        {
          long *addr=(long *)(ptr+i+1);
          patch(ptr+i+5+*addr);
          break;
        }
      if(code_ptr)
      {
        LoadPlugs();
        UpdateShow();
      }
    }
  }
}

void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_PRELOAD;
  static const char *PluginMenuStrings[1];
  PluginMenuStrings[0]=::Info.GetMsg(::Info.ModuleNumber,mName);

  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=0;
  if(ShowInPanels) Info->PluginMenuStringsNumber=sizeofa(PluginMenuStrings);

  Info->PluginConfigStrings=PluginMenuStrings;
  Info->PluginConfigStringsNumber=0;
  if(ShowInConfig) Info->PluginConfigStringsNumber=sizeofa(PluginMenuStrings);
}

void WINAPI _export ExitFAR()
{
  if(!IsOldFAR)
  {
    if(code_ptr) UnloadPlugs();
    unpatch();
    if(DialogMutex)
    {
      CloseHandle(DialogMutex);
      DialogMutex=NULL;
    }
  }
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  if(OpenFrom==OPEN_PLUGINSMENU&&!Item)
    ShowMenu(FMMSG_PANEL,NULL,NULL);
  return INVALID_HANDLE_VALUE;
}

int WINAPI ProcessEditorEvent(int Event,void *Param)
{
  (void)Event;
  (void)Param;
  if(!IsOldFAR) SendBroadcastMessage(FMMSG_CLEANUP,NULL,NULL);
  return 0;
}
int WINAPI ProcessEditorInput(const INPUT_RECORD *Rec)
{
  (void)Rec;
  if(!IsOldFAR) SendBroadcastMessage(FMMSG_CLEANUP,NULL,NULL);
  return 0;
}

int WINAPI _export GetMinFarVersion(void)
{
  return MAKEFARVERSION(1,70,1638);
}

int WINAPI _export Configure(int ItemNumber)
{
  if(!ItemNumber)
  {
    int result=ShowMenu(FMMSG_CONFIG,NULL,NULL);
    LoadPluginsData();
    UpdateShow();
    return result;
  }
  return FALSE;
}

long WINAPI DialogHook(HANDLE hDlg,int Msg,int Param1,long Param2,long Result)
{
  if(Msg==DN_KEY&&Param2==HotKey)
  {
    MenuInData In={sizeof(MenuInData),hDlg,Param1};
    ShowMenu(FMMSG_MENU,&In,NULL);
    return TRUE;
  }
  else if(Msg==DN_KEY&&Param2==DebugKey)
  {
    asm("int $3");
    return TRUE;
  }
  else
  {
    FilterInData In={sizeof(FilterInData),hDlg,Msg,Param1,Param2,&Result};
    SendBroadcastMessage(FMMSG_FILTER,&In,NULL);
  }
  return Result;
}

static void patch(void *ptr)
{
  long *save_ptr=(long *)ptr;
  first_data=save_ptr[0];
  second_data=save_ptr[1];
  unsigned char *patch1_ptr=(unsigned char *)ptr;
  unsigned long *patch2_ptr=(unsigned long *)(patch1_ptr+1);
  MEMORY_BASIC_INFORMATION meminfo;
  if(VirtualQuery(ptr,&meminfo,sizeof(meminfo)))
  {
    DWORD oldaccess;
    if(VirtualProtect(meminfo.BaseAddress,meminfo.RegionSize,PAGE_EXECUTE_WRITECOPY,&oldaccess))
    {
      *patch1_ptr=0xe9;
      *patch2_ptr=(char *)&Invoke_Patch-(char *)ptr-5;
      first_new_data=save_ptr[0];
      second_new_data=save_ptr[1];
      code_ptr=ptr;
    }
  }
}

static void unpatch(void)
{
  if(code_ptr)
  {
    long *save_ptr=(long *)code_ptr;
    save_ptr[0]=first_data;
    save_ptr[1]=second_data;
    code_ptr=0;
  }
}

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
