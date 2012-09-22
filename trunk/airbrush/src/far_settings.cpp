/*
    far_settings.cpp
    Copyright (C) 2011 zg

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

#include "far_settings.h"

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

void CFarSettings::Change(const wchar_t* aName)
{
  FarSettingsValue value={sizeof(FarSettingsValue),0,aName};
  iRoot=Info.SettingsControl(iSettings,SCTL_CREATESUBKEY,0,&value);
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

void CFarSettings::Set(const wchar_t* aName,const void* aValue,size_t aSize)
{
  FarSettingsItem item={sizeof(FarSettingsItem),iRoot,aName,FST_DATA,{0}};
  item.Data.Size=aSize;
  item.Data.Data=aValue;
  Info.SettingsControl(iSettings,SCTL_SET,0,&item);
}

__int64 CFarSettings::Get(const wchar_t* aName,__int64 aDefault)
{
  __int64 result=aDefault;
  FarSettingsItem item={sizeof(FarSettingsItem),iRoot,aName,FST_QWORD,{0}};
  if(Info.SettingsControl(iSettings,SCTL_GET,0,&item))
  {
    result=item.Number;
  }
  return result;
}

bool CFarSettings::Get(const wchar_t* aName,bool aDefault)
{
  bool result=aDefault;
  FarSettingsItem item={sizeof(FarSettingsItem),iRoot,aName,FST_QWORD,{0}};
  if(Info.SettingsControl(iSettings,SCTL_GET,0,&item))
  {
    result=item.Number?true:false;
  }
  return result;
}

bool CFarSettings::Get(const wchar_t* aName,wchar_t* aValue,size_t aSize)
{
  FarSettingsItem item={sizeof(FarSettingsItem),iRoot,aName,FST_STRING,{0}};
  if(Info.SettingsControl(iSettings,SCTL_GET,0,&item))
  {
    lstrcpyn(aValue,item.String,aSize);
    return true;
  }
  return false;
}

size_t CFarSettings::Get(const wchar_t* aName,void* aValue,size_t aSize)
{
  FarSettingsItem item={sizeof(FarSettingsItem),iRoot,aName,FST_DATA,{0}};
  if(Info.SettingsControl(iSettings,SCTL_GET,0,&item))
  {
    size_t result=(item.Data.Size>aSize)?aSize:item.Data.Size;
    memcpy(aValue,item.Data.Data,result);
    return result;
  }
  return 0;
}
