/*
    far_helper.cpp
    Copyright (C) 2009 zg

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

#include "far_helper.h"
#include "memory.h"

void Realloc(TCHAR*& aData,int& aLength,int aNewLength)
{
  if(aNewLength>aLength)
  {
    void* newData=realloc(aData,aNewLength*sizeof(TCHAR));
    if(newData)
    {
      aData=(TCHAR*)newData;
      aLength=aNewLength;
    }
  }
}

void Realloc(PluginPanelItem*& aData,int& aSize,int aNewSize)
{
  if(aNewSize>aSize)
  {
    void* newData=realloc(aData,aNewSize);
    if(newData)
    {
      aData=(PluginPanelItem*)newData;
      aSize=aNewSize;
    }
  }
}

CFarDialog::~CFarDialog()
{
  Info.DialogFree(iDlg);
}

int CFarDialog::Execute(const GUID& PluginId,const GUID& Id,int X1,int Y1,int X2,int Y2,const TCHAR* HelpTopic,struct FarDialogItem* Item,int ItemsNumber,DWORD Reserved,DWORD Flags,FARWINDOWPROC DlgProc,void* Param)
{
  iDlg=Info.DialogInit(&PluginId,&Id,X1,Y1,X2,Y2,HelpTopic,Item,ItemsNumber,Reserved,Flags,DlgProc,Param);
  return Info.DialogRun(iDlg);
}

const wchar_t* CFarDialog::Str(int index)
{
  return (const wchar_t*)Info.SendDlgMessage(iDlg,DM_GETCONSTTEXTPTR,index,0);
}

int CFarDialog::Check(int index)
{
  return (int)Info.SendDlgMessage(iDlg,DM_GETCHECK,index,0);
}

DWORD CFarDialog::Type(int index)
{
  FarDialogItem DialogItem;
  if(Info.SendDlgMessage(iDlg,DM_GETDLGITEMSHORT,index,&DialogItem)) return DialogItem.Type;
  return 0;
}

CFarPanel::CFarPanel(HANDLE aPlugin,FILE_CONTROL_COMMANDS aCommand): iPlugin(aPlugin),iCurDir(NULL),iCurDirSize(0),iItem(NULL),iItemSize(0)
{
  iInfo.StructSize=sizeof(iInfo);
  iResult=Info.PanelControl(aPlugin,aCommand,0,&iInfo);
}

CFarPanel::~CFarPanel()
{
  free(iCurDir);
  free(iItem);
};

TCHAR* CFarPanel::CurDir(void)
{
  size_t dirSize=Info.PanelControl(iPlugin,FCTL_GETPANELDIRECTORY,0,NULL);
  FarPanelDirectory* dirInfo=(FarPanelDirectory*)malloc(dirSize);
  if(dirInfo)
  {
    dirInfo->StructSize=sizeof(FarPanelDirectory);
    Info.PanelControl(iPlugin,FCTL_GETPANELDIRECTORY,dirSize,dirInfo);
    size_t dirNameSize=(wcslen(dirInfo->Name)+1)*sizeof(wchar_t);
    Realloc(iCurDir,iCurDirSize,dirNameSize);
    memcpy(iCurDir,dirInfo->Name,dirNameSize);
    free(dirInfo);
  }
  return iCurDir;
}

PluginPanelItem& CFarPanel::operator[](size_t index)
{
  FarGetPluginPanelItem piinfo={sizeof(FarGetPluginPanelItem),0,NULL};
  piinfo.Size=Info.PanelControl(iPlugin,FCTL_GETPANELITEM,index,&piinfo);
  Realloc(iItem,iItemSize,piinfo.Size);
  piinfo.Item=iItem;
  Info.PanelControl(iPlugin,FCTL_GETPANELITEM,index,&piinfo);
  return *iItem;
}

PluginPanelItem& CFarPanel::Selected(size_t index)
{
  FarGetPluginPanelItem piinfo={sizeof(FarGetPluginPanelItem),0,NULL};
  piinfo.Size=Info.PanelControl(iPlugin,FCTL_GETSELECTEDPANELITEM,index,&piinfo);
  Realloc(iItem,iItemSize,piinfo.Size);
  piinfo.Item=iItem;
  Info.PanelControl(iPlugin,FCTL_GETSELECTEDPANELITEM,index,&piinfo);
  return *iItem;
}

CFarPanelSelection::CFarPanelSelection(HANDLE aPlugin,bool aSelection): iPlugin(aPlugin),iSelection(aSelection),iItem(NULL),iItemSize(0)
{
  iInfo.StructSize=sizeof(iInfo);
  iInfo.CurrentItem=0;
  iInfo.SelectedItemsNumber=0;
  Info.PanelControl(aPlugin,FCTL_GETPANELINFO,0,&iInfo);
}

CFarPanelSelection::~CFarPanelSelection()
{
  free(iItem);
}

PluginPanelItem& CFarPanelSelection::operator[](size_t index)
{
  FarGetPluginPanelItem piinfo={sizeof(FarGetPluginPanelItem),0,NULL};
  piinfo.Size=Info.PanelControl(iPlugin,iSelection?FCTL_GETSELECTEDPANELITEM:FCTL_GETCURRENTPANELITEM,index,&piinfo);
  Realloc(iItem,iItemSize,piinfo.Size);
  piinfo.Item=iItem;
  Info.PanelControl(iPlugin,iSelection?FCTL_GETSELECTEDPANELITEM:FCTL_GETCURRENTPANELITEM,index,&piinfo);
  return *iItem;
}

/*** CFarSettings ***/

CFarSettings::CFarSettings(const GUID& PluginId)
{
  FarSettingsCreate settings={sizeof(FarSettingsCreate),PluginId,INVALID_HANDLE_VALUE};
  iSettings=Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings)?settings.Handle:0;
  iRoot=0;
}

CFarSettings::~CFarSettings()
{
  Info.SettingsControl(iSettings,SCTL_FREE,0,0);
}

void CFarSettings::Set(const wchar_t* aName,__int64 aValue)
{
  FarSettingsItem item={sizeof(FarSettingsItem),iRoot,aName,FST_QWORD,{0}};
  item.Number=aValue;
  Info.SettingsControl(iSettings,SCTL_SET,0,&item);
}

void CFarSettings::Set(const wchar_t* aName,const wchar_t* aValue)
{
  FarSettingsItem item={sizeof(FarSettingsItem),iRoot,aName,FST_STRING,{0}};
  item.String=aValue;
  Info.SettingsControl(iSettings,SCTL_SET,0,&item);
}

void CFarSettings::Get(const wchar_t* aName,__int64& aValue)
{
  FarSettingsItem item={sizeof(FarSettingsItem),iRoot,aName,FST_QWORD,{0}};
  if(Info.SettingsControl(iSettings,SCTL_GET,0,&item))
  {
    aValue=item.Number;
  }
}

void CFarSettings::Get(const wchar_t* aName,wchar_t* aValue,size_t aSize)
{
  FarSettingsItem item={sizeof(FarSettingsItem),iRoot,aName,FST_STRING,{0}};
  if(Info.SettingsControl(iSettings,SCTL_GET,0,&item))
  {
    _tcsncpy(aValue,item.String,aSize-1);
    aValue[aSize-1]=0;
  }
}
