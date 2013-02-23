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
#include <initguid.h>
#include "guid.hpp"
// {37123652-3645-4776-86D6-A94FB2B12FC8}
DEFINE_GUID(CaseGuid, 0x37123652, 0x3645, 0x4776, 0x86, 0xd6, 0xa9, 0x4f, 0xb2, 0xb1, 0x2f, 0xc8);
// {39AE5A2C-A38D-40de-AA59-207E96E5A17D}
DEFINE_GUID(FileGuid, 0x39ae5a2c, 0xa38d, 0x40de, 0xaa, 0x59, 0x20, 0x7e, 0x96, 0xe5, 0xa1, 0x7d);
// {225B5412-36F5-47a2-BAD4-39BE7DFDB4B6}
DEFINE_GUID(PasteGuid, 0x225b5412, 0x36f5, 0x47a2, 0xba, 0xd4, 0x39, 0xbe, 0x7d, 0xfd, 0xb4, 0xb6);
// {C8D4D4A4-943B-436f-8414-D36CBAF37C53}
DEFINE_GUID(PwdGuid, 0xc8d4d4a4, 0x943b, 0x436f, 0x84, 0x14, 0xd3, 0x6c, 0xba, 0xf3, 0x7c, 0x53);
// {40E53FB1-6291-45a1-9DDF-B4A2048CA6BA}
DEFINE_GUID(SearchGuid, 0x40e53fb1, 0x6291, 0x45a1, 0x9d, 0xdf, 0xb4, 0xa2, 0x4, 0x8c, 0xa6, 0xba);
// {F4C58B97-BA69-4814-9FC9-DAB9307C0E88}
DEFINE_GUID(ReplaceGuid, 0xf4c58b97, 0xba69, 0x4814, 0x9f, 0xc9, 0xda, 0xb9, 0x30, 0x7c, 0xe, 0x88);
// {1117C0E3-76BA-443d-90E3-89D20C45F364}
DEFINE_GUID(UndoGuid, 0x1117c0e3, 0x76ba, 0x443d, 0x90, 0xe3, 0x89, 0xd2, 0xc, 0x45, 0xf3, 0x64);
// {D0C5118D-E750-4e00-A4D4-0E0E0063571F}
DEFINE_GUID(EditGuid, 0xd0c5118d, 0xe750, 0x4e00, 0xa4, 0xd4, 0xe, 0xe, 0x0, 0x63, 0x57, 0x1f);


PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

const TCHAR* GetMsg(int MsgId)
{
  return(Info.GetMsg(&MainGuid,MsgId));
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo* Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>(int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;
  InitCase();
  InitUndo();
}

void WINAPI GetPluginInfoW(struct PluginInfo* Info)
{
  memset(Info,0,sizeof(Info));
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_DIALOG|PF_DISABLEPANELS;
  static const GUID guids[]={CaseGuid,FileGuid,PasteGuid,PwdGuid,SearchGuid,ReplaceGuid,UndoGuid,EditGuid};
  const int items[]={mNameCase,mNameFile,mNamePaste,mNamePwd,mNameSearch,mNameReplace,mNameUndo,mNameEdit};
  static const TCHAR* PluginMenuStrings[ArraySize(items)];
  for(size_t ii=0;ii<ArraySize(items);ii++) PluginMenuStrings[ii]=GetMsg(items[ii]);
  Info->PluginMenu.Guids=guids;
  Info->PluginMenu.Strings=PluginMenuStrings;
  Info->PluginMenu.Count=ArraySize(PluginMenuStrings);
}

HANDLE WINAPI OpenW(const struct OpenInfo *anInfo)
{
  if(anInfo->OpenFrom==OPEN_DIALOG)
  {
    OpenDlgPluginData* data=(OpenDlgPluginData*)anInfo->Data;
    if(IsEqualGUID(*anInfo->Guid,CaseGuid))
    {
      DoCase(data->hDlg);
    }
    else if(IsEqualGUID(*anInfo->Guid,FileGuid))
    {
      DoOpenFile(data->hDlg);
    }
    else if(IsEqualGUID(*anInfo->Guid,PasteGuid))
    {
      DoPaste(data->hDlg);
    }
    else if(IsEqualGUID(*anInfo->Guid,PwdGuid))
    {
      DoPwd(data->hDlg);
    }
    else if(IsEqualGUID(*anInfo->Guid,SearchGuid))
    {
      DoSearch(data->hDlg);
    }
    else if(IsEqualGUID(*anInfo->Guid,ReplaceGuid))
    {
      DoReplace(data->hDlg);
    }
    else if(IsEqualGUID(*anInfo->Guid,UndoGuid))
    {
      DoUndo(data->hDlg);
    }
    else if(IsEqualGUID(*anInfo->Guid,EditGuid))
    {
      DoEdit(data->hDlg);
    }
  }
  return NULL;
}

void WINAPI ExitFARW(const struct ExitInfo *Info)
{
  FinishCase();
  FinishUndo();
}

intptr_t WINAPI ProcessDialogEventW(const struct ProcessDialogEventInfo *Info)
{
  if(Info->Event==DE_DLGPROCINIT)
  {
    FilterUndoInit(Info->Param->hDlg,Info->Param->Msg,Info->Param->Param1,Info->Param->Param2);
  }
  else if(Info->Event==DE_DLGPROCEND)
  {
    FilterUndoEnd(Info->Param->hDlg,Info->Param->Msg,Info->Param->Param1,Info->Param->Param2,Info->Param->Result);
  }
  return FALSE;
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=MAKEFARVERSION(1,0,0,3,VS_RELEASE);
  Info->Guid=MainGuid;
  Info->Title=L"Dialog Tools";
  Info->Description=L"Small tools for Far dialogs";
  Info->Author=L"Vadim Yegorov";
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
