/*
    um_adds.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include "..\..\plugin.hpp"
#include <lm.h>
#include <ntsecapi.h>
#include "umplugin.h"
#include "memory.h"

extern LSA_HANDLE GetPolicyHandle(wchar_t *computer);

bool AddOwner(UserManager *panel,UserManager *anotherpanel,bool selection)
{
  bool res=false;
  PluginPanelItem *PanelItem; int ItemsNumber;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection);
  if(ItemsNumber==1)
  {
    if(PanelItem[0].Flags&PPIF_USERDATA)
    {
      PSID user=GetSidFromUserData(PanelItem[0].UserData);
      res=AddOwnerInternal(anotherpanel,user);
    }
  }
  return res;
}

bool AddACE(UserManager *panel,UserManager *anotherpanel,bool selection)
{
  bool res=false;
  PluginPanelItem *PanelItem; int ItemsNumber;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection);
  if(ItemsNumber)
  {
    for(int i=0;i<ItemsNumber;i++)
      if(PanelItem[i].Flags&PPIF_USERDATA)
        UpdateAcl(anotherpanel,anotherpanel->level,GetSidFromUserData(PanelItem[i].UserData),GetItemTypeFromUserData(PanelItem[i].UserData),default_mask[anotherpanel->level],actionInsert);
    res=true;
  }
  return res;
}

bool AddUserToGroup(UserManager *panel,UserManager *anotherpanel,bool selection)
{
  bool res=false;
  PluginPanelItem *PanelItem; int ItemsNumber;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection);
  if(ItemsNumber)
  {
    for(int i=0;i<ItemsNumber;i++)
      if(PanelItem[i].Flags&PPIF_USERDATA)
      {
        if(anotherpanel->global)
        {
          NetGroupAddUser(anotherpanel->domain,anotherpanel->nonfixed,GetWideNameFromUserData(PanelItem[i].UserData));
        }
        else
        {
          LOCALGROUP_MEMBERS_INFO_0 new_member;
          new_member.lgrmi0_sid=GetSidFromUserData(PanelItem[i].UserData);
          NetLocalGroupAddMembers(panel->computer_ptr,anotherpanel->nonfixed,0,(LPBYTE)&new_member,1);
        }
      }
    res=true;
  }
  return res;
}

bool AddUserToRight(UserManager *panel,UserManager *anotherpanel,bool selection)
{
  bool res=false;
  PluginPanelItem *PanelItem; int ItemsNumber;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection);
  if(ItemsNumber)
  {
    LSA_HANDLE PolicyHandle;
    PolicyHandle=GetPolicyHandle(anotherpanel->computer);
    if(PolicyHandle)
    {
      for(int i=0;i<ItemsNumber;i++)
        if(PanelItem[i].Flags&PPIF_USERDATA)
        {
          LSA_UNICODE_STRING RightName;
          RightName.Buffer=anotherpanel->nonfixed;
          RightName.Length=wcslen(RightName.Buffer)*sizeof(wchar_t);
          RightName.MaximumLength=RightName.Length+sizeof(wchar_t);
          LsaAddAccountRights(PolicyHandle,GetSidFromUserData(PanelItem[i].UserData),&RightName,1);
        }
      LsaClose(PolicyHandle);
    }
    res=true;
  }
  return res;
}
