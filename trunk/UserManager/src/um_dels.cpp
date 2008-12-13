/*
    um_dels.cpp
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
#include "far_helper.h"
#include <lm.h>
#include <ntsecapi.h>
#include "umplugin.h"
#include "memory.h"

extern LSA_HANDLE GetPolicyHandle(wchar_t *computer);

#ifdef UNICODE
#define SELECTED_ITEMS(index) (*(PanelItem[index]))
#else
#define SELECTED_ITEMS(index) PanelItem[index]
#endif

bool TakeOwnership(UserManager *panel,bool selection)
{
  return AddOwnerInternal(panel,current_user());
}

bool DeleteACE(UserManager *panel,bool selection)
{
  bool res=false,has_dir=false;
  int ItemsNumber;
#ifdef UNICODE
  PluginPanelItem **PanelItem;
  SSelectionInfo info;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection,info);
#else
  PluginPanelItem *PanelItem;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection);
#endif
  if(ItemsNumber)
  {
    for(int i=0;i<ItemsNumber;i++)
      if(SELECTED_ITEMS(i).FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
      {
        has_dir=true;
        break;
      }
    if(has_dir) ShowCustomError(mDelFolderWarning);
    else
    {
      TCHAR warning[TINY_BUFFER];
      if(ItemsNumber==1)
      {
        TCHAR Truncated[MAX_PATH];
        _tcscpy(Truncated,SELECTED_ITEMS(0).FindData.PANEL_FILENAME);
        FSF.TruncPathStr(Truncated,50);
        FSF.sprintf(warning,GetMsg(mDelOne),Truncated);
      }
      else
        FSF.sprintf(warning,GetMsg(mDelACEN+NumberType(ItemsNumber)),ItemsNumber);
      const TCHAR *MsgItems[]={GetMsg(mButtonDelete),warning,GetMsg(mButtonDelete),GetMsg(mButtonCancel)};
      if(!Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2))
        for(int i=0;i<ItemsNumber;i++)
        {
          if(SELECTED_ITEMS(i).Flags&PPIF_USERDATA)
          {
            if(UpdateAcl(panel,panel->level,GetSidFromUserData(SELECTED_ITEMS(i).UserData),GetItemTypeFromUserData(SELECTED_ITEMS(i).UserData),GetLevelFromUserData(SELECTED_ITEMS(i).UserData),actionDelete))
              res=true;
            else
              break;
          }
        }
    }
  }
#ifdef UNICODE
  FreeSelectedList((HANDLE)panel,info);
#endif
  return res;
}

bool DeleteShare(UserManager *panel,bool selection)
{
  bool res=false;
  int ItemsNumber;
#ifdef UNICODE
  PluginPanelItem **PanelItem;
  SSelectionInfo info;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection,info);
#else
  PluginPanelItem *PanelItem;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection);
#endif
  if(ItemsNumber)
  {
    TCHAR warning[TINY_BUFFER];
    if(ItemsNumber==1)
    {
      TCHAR Truncated[MAX_PATH];
      _tcscpy(Truncated,SELECTED_ITEMS(0).FindData.PANEL_FILENAME);
      FSF.TruncPathStr(Truncated,50);
      FSF.sprintf(warning,GetMsg(mDelOne),Truncated);
    }
    else
      FSF.sprintf(warning,GetMsg(mDelShareN+NumberType(ItemsNumber)),ItemsNumber);
    const TCHAR *MsgItems[]={GetMsg(mButtonDelete),warning,GetMsg(mButtonDelete),GetMsg(mButtonCancel)};
    if(!Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2))
    {
      res=true;
      for(int i=0;i<ItemsNumber;i++)
        if(panel->level==levelPrinterShared)
        {
          HANDLE printer; PRINTER_DEFAULTSW defaults; PRINTER_INFO_2W *data=NULL;
          memset(&defaults,0,sizeof(defaults));
          defaults.DesiredAccess=PRINTER_ALL_ACCESS;
          if(OpenPrinterW(panel->hostfile,&printer,&defaults))
          {
            DWORD Needed;
            if(!GetPrinterW(printer,2,NULL,0,&Needed))
            {
              if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
              {
                data=(PRINTER_INFO_2W *)malloc(Needed);
                if(data)
                {
                  if(GetPrinterW(printer,2,(PBYTE)data,Needed,&Needed))
                  {
                    data->Attributes&=~PRINTER_ATTRIBUTE_SHARED;
                    if(SetPrinterW(printer,2,(PBYTE)data,0)) res=true;
                  }
                  free(data); data=NULL;
                }
              }
            }
            ClosePrinter(printer);
          }
        }
        else if(SELECTED_ITEMS(i).Flags&PPIF_USERDATA)
        {
          NetShareDel(panel->computer_ptr,GetWideNameFromUserData(SELECTED_ITEMS(i).UserData),0);
        }
    }
  }
#ifdef UNICODE
  FreeSelectedList((HANDLE)panel,info);
#endif
  return res;
}

bool DeleteGroup(UserManager *panel,bool selection)
{
  bool res=false;
  int ItemsNumber;
#ifdef UNICODE
  PluginPanelItem **PanelItem;
  SSelectionInfo info;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection,info);
#else
  PluginPanelItem *PanelItem;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection);
#endif
  if(ItemsNumber)
  {
    TCHAR warning[TINY_BUFFER];
    if(ItemsNumber==1)
    {
      TCHAR Truncated[MAX_PATH];
      _tcscpy(Truncated,SELECTED_ITEMS(0).FindData.PANEL_FILENAME);
      FSF.TruncPathStr(Truncated,50);
      FSF.sprintf(warning,GetMsg(mDelOne),Truncated);
    }
    else
      FSF.sprintf(warning,GetMsg(mDelObjectN+NumberType(ItemsNumber)),ItemsNumber);
    const TCHAR *MsgItems[]={GetMsg(mButtonDelete),warning,GetMsg(mButtonDelete),GetMsg(mButtonCancel)};
    if(!Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2))
    {
      res=true;
      for(int i=0;i<ItemsNumber;i++)
      {
        if(SELECTED_ITEMS(i).Flags&PPIF_USERDATA)
        {
          if(SELECTED_ITEMS(i).FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
          {
            if(panel->global)
            {
              NetGroupDel(panel->domain,GetWideNameFromUserData(SELECTED_ITEMS(i).UserData));
            }
            else
            {
              NetLocalGroupDel(panel->computer_ptr,GetWideNameFromUserData(SELECTED_ITEMS(i).UserData));
            }
          }
          else
            NetUserDel((panel->global)?(panel->domain):(panel->computer_ptr),GetWideNameFromUserData(SELECTED_ITEMS(i).UserData));
        }
      }
    }
  }
#ifdef UNICODE
  FreeSelectedList((HANDLE)panel,info);
#endif
  return res;
}

bool RemoveUser(UserManager *panel,bool selection)
{
  bool res=false;
  int ItemsNumber;
#ifdef UNICODE
  PluginPanelItem **PanelItem;
  SSelectionInfo info;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection,info);
#else
  PluginPanelItem *PanelItem;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection);
#endif
  if(ItemsNumber)
  {
    TCHAR warning[TINY_BUFFER];
    if(ItemsNumber==1)
    {
      TCHAR Truncated[MAX_PATH];
      _tcscpy(Truncated,SELECTED_ITEMS(0).FindData.PANEL_FILENAME);
      FSF.TruncPathStr(Truncated,50);
      FSF.sprintf(warning,GetMsg(mRemoveOne),Truncated);
    }
    else
      FSF.sprintf(warning,GetMsg(mRemoveUserN+NumberType(ItemsNumber)),ItemsNumber);
    const TCHAR *MsgItems[]={GetMsg(mButtonRemove),warning,GetMsg(mButtonRemove),GetMsg(mButtonCancel)};
    if(!Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2))
    {
      res=true;
      for(int i=0;i<ItemsNumber;i++)
      {
        if(SELECTED_ITEMS(i).Flags&PPIF_USERDATA)
        {
          if(panel->global)
          {
            NetGroupDelUser(panel->domain,panel->nonfixed,GetWideNameFromUserData(SELECTED_ITEMS(i).UserData));
          }
          else
          {
            LOCALGROUP_MEMBERS_INFO_0 new_member;
            new_member.lgrmi0_sid=GetSidFromUserData(SELECTED_ITEMS(i).UserData);
            NetLocalGroupDelMembers(panel->computer_ptr,panel->nonfixed,0,(LPBYTE)&new_member,1);
          }
        }
      }
    }
  }
#ifdef UNICODE
  FreeSelectedList((HANDLE)panel,info);
#endif
  return res;
}

bool DeleteUser(UserManager *panel,bool selection)
{
  bool res=false;
  int ItemsNumber;
#ifdef UNICODE
  PluginPanelItem **PanelItem;
  SSelectionInfo info;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection,info);
#else
  PluginPanelItem *PanelItem;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection);
#endif
  if(ItemsNumber)
  {
    TCHAR warning[TINY_BUFFER];
    if(ItemsNumber==1)
    {
      TCHAR Truncated[MAX_PATH];
      _tcscpy(Truncated,SELECTED_ITEMS(0).FindData.PANEL_FILENAME);
      FSF.TruncPathStr(Truncated,50);
      FSF.sprintf(warning,GetMsg(mDelOne),Truncated);
    }
    else
      FSF.sprintf(warning,GetMsg(mDelUserN+NumberType(ItemsNumber)),ItemsNumber);
    const TCHAR *MsgItems[]={GetMsg(mButtonDelete),warning,GetMsg(mButtonDelete),GetMsg(mButtonCancel)};
    if(!Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2))
    {
      res=true;
      for(int i=0;i<ItemsNumber;i++)
      {
        if(SELECTED_ITEMS(i).Flags&PPIF_USERDATA)
          NetUserDel((panel->global)?(panel->domain):(panel->computer_ptr),GetWideNameFromUserData(SELECTED_ITEMS(i).UserData));
      }
    }
  }
#ifdef UNICODE
  FreeSelectedList((HANDLE)panel,info);
#endif
  return res;
}

bool DeleteRightUsers(UserManager *panel,bool selection)
{
  bool res=false;
  int ItemsNumber;
#ifdef UNICODE
  PluginPanelItem **PanelItem;
  SSelectionInfo info;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection,info);
#else
  PluginPanelItem *PanelItem;
  GetSelectedList((HANDLE)panel,&PanelItem,&ItemsNumber,selection);
#endif
  if(ItemsNumber)
  {
    TCHAR warning[TINY_BUFFER];
    if(ItemsNumber==1)
    {
      TCHAR Truncated[MAX_PATH];
      _tcscpy(Truncated,SELECTED_ITEMS(0).FindData.PANEL_FILENAME);
      FSF.TruncPathStr(Truncated,50);
      FSF.sprintf(warning,GetMsg(mDelOne),Truncated);
    }
    else
      FSF.sprintf(warning,GetMsg(mDelUserN+NumberType(ItemsNumber)),ItemsNumber);
    const TCHAR *MsgItems[]={GetMsg(mButtonDelete),warning,GetMsg(mButtonDelete),GetMsg(mButtonCancel)};
    if(!Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2))
    {
      res=true;
      LSA_HANDLE PolicyHandle;
      PolicyHandle=GetPolicyHandle(panel->computer);
      if(PolicyHandle)
      {
        for(int i=0;i<ItemsNumber;i++)
        {
          if(SELECTED_ITEMS(i).Flags&PPIF_USERDATA)
          {
            LSA_UNICODE_STRING RightName;
            RightName.Buffer=panel->nonfixed;
            RightName.Length=wcslen(RightName.Buffer)*sizeof(wchar_t);
            RightName.MaximumLength=RightName.Length+sizeof(wchar_t);
            LsaRemoveAccountRights(PolicyHandle,GetSidFromUserData(SELECTED_ITEMS(i).UserData),FALSE,&RightName,1);
          }
        }
        LsaClose(PolicyHandle);
      }
    }
  }
#ifdef UNICODE
  FreeSelectedList((HANDLE)panel,info);
#endif
  return res;
}
