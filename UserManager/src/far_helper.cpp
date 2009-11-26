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

#ifdef UNICODE

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

const wchar_t* CFarDialog::Str(int index)
{
  return (const wchar_t*)Info.SendDlgMessage(iDlg,DM_GETCONSTTEXTPTR,index,0);
}

int CFarDialog::Check(int index)
{
  return (int)Info.SendDlgMessage(iDlg,DM_GETCHECK,index,0);
}

CFarPanel::CFarPanel(HANDLE aPlugin,int aCommand): iPlugin(aPlugin),iCurDir(NULL),iCurDirSize(0),iItem(NULL),iItemSize(0)
{
  iResult=Info.Control(aPlugin,aCommand,0,(LONG_PTR)&iInfo);
}

CFarPanel::~CFarPanel()
{
  free(iCurDir);
  free(iItem);
};

TCHAR* CFarPanel::CurDir(void)
{
  Realloc(iCurDir,iCurDirSize,Info.Control(iPlugin,FCTL_GETPANELDIR,0,0));
  Info.Control(iPlugin,FCTL_GETPANELDIR,iCurDirSize,(LONG_PTR)iCurDir);
  return iCurDir;
}

PluginPanelItem& CFarPanel::operator[](size_t index)
{
  Realloc(iItem,iItemSize,Info.Control(iPlugin,FCTL_GETPANELITEM,index,0));
  Info.Control(iPlugin,FCTL_GETPANELITEM,index,(LONG_PTR)iItem);
  return *iItem;
}

PluginPanelItem& CFarPanel::Selected(size_t index)
{
  Realloc(iItem,iItemSize,Info.Control(iPlugin,FCTL_GETSELECTEDPANELITEM,index,0));
  Info.Control(iPlugin,FCTL_GETSELECTEDPANELITEM,index,(LONG_PTR)iItem);
  return *iItem;
}

CFarPanelSelection::CFarPanelSelection(HANDLE aPlugin,bool aSelection): iPlugin(aPlugin),iSelection(aSelection),iItem(NULL),iItemSize(0)
{
  iInfo.CurrentItem=0;
  iInfo.SelectedItemsNumber=0;
  Info.Control(aPlugin,FCTL_GETPANELINFO,0,(LONG_PTR)&iInfo);
}

CFarPanelSelection::~CFarPanelSelection()
{
  free(iItem);
}

PluginPanelItem& CFarPanelSelection::operator[](size_t index)
{
  Realloc(iItem,iItemSize,Info.Control(iPlugin,iSelection?FCTL_GETSELECTEDPANELITEM:FCTL_GETCURRENTPANELITEM,index,0));
  Info.Control(iPlugin,iSelection?FCTL_GETSELECTEDPANELITEM:FCTL_GETCURRENTPANELITEM,index,(LONG_PTR)iItem);
  return *iItem;
}

#else

CFarPanelSelection::CFarPanelSelection(HANDLE aPlugin,bool aSelection): iPlugin(aPlugin),iSelection(aSelection)
{
  iInfo.CurrentItem=0;
  iInfo.SelectedItemsNumber=0;
  Info.Control(aPlugin,FCTL_GETPANELINFO,(void*)&iInfo);
}

#endif
