/*
    dt.cpp
    Copyright (C) 2008 zg

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

#include "dt.hpp"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
TCHAR PluginRootKey[100];
BOOL IsOldFAR=TRUE;

const TCHAR* GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

void InitDialogItems(const InitDialogItem *Init,FarDialogItem *Item,int ItemsNumber)
{
  for (int i=0;i<ItemsNumber;i++)
  {
    Item[i].Type=Init[i].Type;
    Item[i].X1=Init[i].X1;
    Item[i].Y1=Init[i].Y1;
    Item[i].X2=Init[i].X2;
    Item[i].Y2=Init[i].Y2;
    Item[i].Focus=Init[i].Focus;
    Item[i].Selected=Init[i].Selected;
    Item[i].Flags=Init[i].Flags;
    Item[i].DefaultButton=Init[i].DefaultButton;
#ifdef UNICODE
    Item[i].MaxLen=0;
#endif
    if((unsigned)Init[i].Data<2000)
#ifdef UNICODE
      Item[i].PtrData=GetMsg((unsigned int)(DWORD_PTR)Init[i].Data);
#else
      strcpy(Item[i].Data,GetMsg((unsigned int)(DWORD_PTR)Init[i].Data));
#endif
    else
#ifdef UNICODE
      Item[i].PtrData=Init[i].Data;
#else
      strcpy(Item[i].Data,Init[i].Data);
#endif
  }
}

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo* Info)
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
    _tcscpy(PluginRootKey,Info->RootKey);
    _tcscat(PluginRootKey,_T("\\DialogTools"));
    InitCase();
#ifndef UNICODE
  }
#endif
}

void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo* Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_DIALOG|PF_DISABLEPANELS;
  const int items[]={mNameCase,mNameFile,mNamePaste,mNamePwd,mNameSearch,mNameReplace,mNameUndo,mNameEdit};
  static const TCHAR* PluginMenuStrings[ArraySize(items)];
  for(size_t ii=0;ii<ArraySize(items);ii++) PluginMenuStrings[ii]=GetMsg(items[ii]);

  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=ArraySize(PluginMenuStrings);

  Info->PluginConfigStrings=PluginMenuStrings;
  Info->PluginConfigStringsNumber=0;
}

HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom,INT_PTR Item)
{
  if(OpenFrom==OPEN_DIALOG)
  {
    OpenDlgPluginData* data=(OpenDlgPluginData*)Item;
    switch(data->ItemNumber)
    {
      case mNameCase:
        DoCase(data->hDlg);
        break;
      case mNameFile:
        DoOpenFile(data->hDlg);
        break;
      case mNamePaste:
        DoPaste(data->hDlg);
        break;
      case mNamePwd:
        DoPwd(data->hDlg);
        break;
      case mNameSearch:
        DoSearch(data->hDlg);
        break;
      case mNameReplace:
        DoReplace(data->hDlg);
        break;
      case mNameUndo:
        DoUndo(data->hDlg);
        break;
      case mNameEdit:
        DoEdit(data->hDlg);
        break;
    }
  }
  return INVALID_HANDLE_VALUE;
}

#ifndef UNICODE
int WINAPI EXP_NAME(GetMinFarVersion)(void)
{
  return MAKEFARVERSION(1,71,2307);
}
#endif

void WINAPI EXP_NAME(ExitFAR)(void)
{
  if(!IsOldFAR)
  {
    FinishUndo();
  }
}

int WINAPI EXP_NAME(ProcessDialogEvent)(int Event,void* Param)
{
  if(Event==DE_DLGPROCINIT)
  {
    FarDialogEvent* event=(FarDialogEvent*)Param;
    FilterUndo(event->hDlg,event->Msg,event->Param1,event->Param2);
  }
  return FALSE;
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
