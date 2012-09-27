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
#include "plugin.hpp"
#include <lm.h>
#include <ntsecapi.h>
#include "far_helper.h"
#include "umplugin.h"
#include "memory.h"

extern LSA_HANDLE GetPolicyHandle(wchar_t *computer);

bool AddOwner(UserManager *panel,UserManager *anotherpanel,bool selection)
{
  bool res=false;
  CFarPanelSelection sp((HANDLE)panel,selection);
  if(sp.Number()==1)
  {
    if(sp[0].UserData.FreeData)
    {
      PSID user=GetSidFromUserData(sp[0].UserData.Data);
      res=AddOwnerInternal(anotherpanel,user);
    }
  }
  return res;
}

bool AddACE(UserManager *panel,UserManager *anotherpanel,bool selection)
{
  bool res=false;
  CFarPanelSelection sp((HANDLE)panel,selection);
  if(sp.Number())
  {
    for(int i=0;i<sp.Number();i++)
      if(sp[i].UserData.FreeData)
        UpdateAcl(anotherpanel,anotherpanel->level,GetSidFromUserData(sp[i].UserData.Data),GetItemTypeFromUserData(sp[i].UserData.Data),default_mask[anotherpanel->level],actionInsert);
    res=true;
  }
  return res;
}

bool AddUserToGroup(UserManager *panel,UserManager *anotherpanel,bool selection)
{
  bool res=false;
  CFarPanelSelection sp((HANDLE)panel,selection);
  if(sp.Number())
  {
    for(int i=0;i<sp.Number();i++)
      if(sp[i].UserData.FreeData)
      {
        if(anotherpanel->global)
        {
          NetGroupAddUser(anotherpanel->domain,anotherpanel->nonfixed,GetWideNameFromUserData(sp[i].UserData.Data));
        }
        else
        {
          LOCALGROUP_MEMBERS_INFO_0 new_member;
          new_member.lgrmi0_sid=GetSidFromUserData(sp[i].UserData.Data);
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
  CFarPanelSelection sp((HANDLE)panel,selection);
  if(sp.Number())
  {
    LSA_HANDLE PolicyHandle;
    PolicyHandle=GetPolicyHandle(anotherpanel->computer);
    if(PolicyHandle)
    {
      for(int i=0;i<sp.Number();i++)
        if(sp[i].UserData.FreeData)
        {
          LSA_UNICODE_STRING RightName;
          RightName.Buffer=anotherpanel->nonfixed;
          RightName.Length=wcslen(RightName.Buffer)*sizeof(wchar_t);
          RightName.MaximumLength=RightName.Length+sizeof(wchar_t);
          LsaAddAccountRights(PolicyHandle,GetSidFromUserData(sp[i].UserData.Data),&RightName,1);
        }
      LsaClose(PolicyHandle);
    }
    res=true;
  }
  return res;
}
