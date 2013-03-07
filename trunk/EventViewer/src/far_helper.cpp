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

void Realloc(wchar_t*& aData,size_t& aLength,size_t aNewLength)
{
  if(aNewLength>aLength)
  {
    void* newData=realloc(aData,aNewLength*sizeof(wchar_t));
    if(newData)
    {
      aData=(wchar_t*)newData;
      aLength=aNewLength;
    }
  }
}

void Realloc(PluginPanelItem*& aData,size_t& aSize,size_t aNewSize)
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

CFarPanel::~CFarPanel()
{
  free(iCurDir);
  free(iItem);
};

wchar_t* CFarPanel::CurDir(void)
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
