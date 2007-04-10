/*
    um_userdata.cpp
    Copyright (C) 2001-2007 zg

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "..\..\plugin.hpp"
#include "memory.h"

struct PluginUserData
{
  int size;
  int level;
  int sortorder;
  int itemtype;
  int user_diff;
  int wide_name_diff;
};

void AddDefaultUserdata(PluginPanelItem *Item,int level,int sortorder,int itemtype,PSID sid,wchar_t *wide_name,char *oem_name)
{
  PluginUserData *user_data;
  int user_data_size=sizeof(PluginUserData),sid_size=0,name_size=0;
  if(sid&&IsValidSid(sid))
    sid_size=GetLengthSid(sid);
  if(wide_name) name_size=wcslen(wide_name)+1;
  else if(oem_name) name_size=strlen(oem_name)+1;
  name_size*=sizeof(wchar_t);
  user_data_size+=sid_size+name_size;

  user_data=(PluginUserData *)malloc(user_data_size);
  if(user_data)
  {
    user_data->size=user_data_size;
    user_data->level=level;
    user_data->sortorder=sortorder;
    user_data->itemtype=itemtype;
    if(sid_size)
    {
      CopySid(sid_size,(PSID)(user_data+1),sid);
      user_data->user_diff=sizeof(PluginUserData);
    }
    if(name_size)
    {
      wchar_t *ptr=(wchar_t *)((char *)(user_data+1)+sid_size);
      user_data->wide_name_diff=sizeof(PluginUserData)+sid_size;
      if(wide_name) wcscpy(ptr,wide_name);
      else MultiByteToWideChar(CP_OEMCP,0,oem_name,-1,ptr,name_size/sizeof(wchar_t));
    }
    Item->Flags=PPIF_USERDATA;
    Item->UserData=(DWORD)user_data;
  }
}

int GetLevelFromUserData(DWORD UserData)
{
  PluginUserData *user_data=(PluginUserData *)UserData;
  return user_data->level;
}

int GetSortOrderFromUserData(DWORD UserData)
{
  PluginUserData *user_data=(PluginUserData *)UserData;
  return user_data->sortorder;
}

int GetItemTypeFromUserData(DWORD UserData)
{
  PluginUserData *user_data=(PluginUserData *)UserData;
  return user_data->itemtype;
}

PSID GetSidFromUserData(DWORD UserData)
{
  char *ptr=(char *)UserData;
  PluginUserData *user_data=(PluginUserData *)UserData;
  return (PSID)(ptr+user_data->user_diff);
}

wchar_t *GetWideNameFromUserData(DWORD UserData)
{
  char *ptr=(char *)UserData;
  PluginUserData *user_data=(PluginUserData *)UserData;
  return (wchar_t *)(ptr+user_data->wide_name_diff);
}
