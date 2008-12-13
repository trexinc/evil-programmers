/*
    um_manage_group.cpp
    Copyright (C) 2002-2007 zg

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
#include "farkeys.hpp"
#include <lm.h>
#include "umplugin.h"
#include "memory.h"

const int indexName=2;
const int indexComment=4;
const int updateName=0;
const int updateComment=1;
const int updateSize=2;

//FIXME: NetApiBufferFree

long WINAPI ManageGroupDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  bool *updated=(bool *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_SETDLGDATA,0,Param2);
      Info.SendDlgMessage(hDlg,DM_SETTEXTLENGTH,indexName,GNLEN-1);
      Info.SendDlgMessage(hDlg,DM_SETTEXTLENGTH,indexComment,MAXCOMMENTSZ-1);
      break;
    case DN_EDITCHANGE:
      if(Param1==indexName) updated[updateName]=true;
      if(Param1==indexComment) updated[updateComment]=true;
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool ManageGroup(UserManager *panel,bool type,const wchar_t *in_group)
{
  bool res=false;
  wchar_t group[512]=L"",*server=(panel->global)?(panel->domain):(panel->computer_ptr);
  /*
    0000000000111111111122222222223333333333444444444455555555556666666666777777
    0123456789012345678901234567890123456789012345678901234567890123456789012345
  00                                                                            00
  01   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ New Group ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»   01
  02   º Group name:                                                        º   02
  03   º TEMP                                                              |º   03
  04   º Group comment:                                                     º   04
  05   º TEMP                                                              |º   05
  06   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   06
  07   º                 [ OK ]                  [ Cancel ]                 º   07
  08   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   08
  09                                                                            09
    0000000000111111111122222222223333333333444444444455555555556666666666777777
    0123456789012345678901234567890123456789012345678901234567890123456789012345
  */
  static const TCHAR *NewGroupHistoryName=_T("UserManager\\NewGroup");
  static const TCHAR *NewGroupHistoryComment=_T("UserManager\\NewGroupComment");
  static struct InitDialogItem InitDlg[]={
  /* 0*/  {DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(TCHAR *)mGroupNewGroup},
  /* 1*/  {DI_TEXT,5,2,0,0,0,0,0,0,(TCHAR *)mGroupName},
  /* 2*/  {DI_EDIT,5,3,70,0,1,(DWORD)NewGroupHistoryName,DIF_HISTORY,0,_T("")},
  /* 3*/  {DI_TEXT,5,4,0,0,0,0,0,0,(TCHAR *)mGroupComment},
  /* 4*/  {DI_EDIT,5,5,70,0,1,(DWORD)NewGroupHistoryComment,DIF_HISTORY,0,_T("")},
  /* 5*/  {DI_TEXT,5,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
  /* 6*/  {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)mButtonOk},
  /* 7*/  {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)mButtonCancel},
  };
  struct FarDialogItem DialogItems[sizeof(InitDlg)/sizeof(InitDlg[0])];
  InitDialogItems(InitDlg,DialogItems,sizeof(InitDlg)/sizeof(InitDlg[0]));
#ifdef UNICODE
  TCHAR name[512],comment[512],title[128];
#endif
  if(type)
  {
    wcscpy(group,in_group);
    if(panel->global)
    {
      GROUP_INFO_1 *info;
      if(NetGroupGetInfo(server,group,1,(LPBYTE *)&info)==NERR_Success)
      {
#ifdef UNICODE
        _tcscpy(name,info->grpi1_name);
        DialogItems[indexName].PtrData=name;
        _tcscpy(comment,info->grpi1_comment);
        DialogItems[indexComment].PtrData=comment;
#else
        WideCharToMultiByte(CP_OEMCP,0,info->grpi1_name,-1,DialogItems[indexName].Data,UNLEN,NULL,NULL);
        WideCharToMultiByte(CP_OEMCP,0,info->grpi1_comment,-1,DialogItems[indexComment].Data,sizeof(DialogItems[indexComment].Data),NULL,NULL);
#endif
        NetApiBufferFree(info);
      }
      else return false;
    }
    else
    {
      LOCALGROUP_INFO_1 *info;
      if(NetLocalGroupGetInfo(server,group,1,(LPBYTE *)&info)==NERR_Success)
      {
#ifdef UNICODE
        _tcscpy(name,info->lgrpi1_name);
        DialogItems[indexName].PtrData=name;
        _tcscpy(comment,info->lgrpi1_comment);
        DialogItems[indexComment].PtrData=comment;
#else
        WideCharToMultiByte(CP_OEMCP,0,info->lgrpi1_name,-1,DialogItems[indexName].Data,UNLEN,NULL,NULL);
        WideCharToMultiByte(CP_OEMCP,0,info->lgrpi1_comment,-1,DialogItems[indexComment].Data,sizeof(DialogItems[indexComment].Data),NULL,NULL);
#endif
        NetApiBufferFree(info);
      }
      else return false;
    }
#ifdef UNICODE
    FSF.sprintf(title,GetMsg(mGroupEditGroup),DialogItems[indexName].PtrData);
    DialogItems[0].PtrData=title;
#else
    FSF.sprintf(DialogItems[0].Data,GetMsg(mGroupEditGroup),DialogItems[indexName].Data);
#endif
  }
  bool params[updateSize];
  CFarDialog dialog;
  int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,76,10,_T("ManageGroup"),DialogItems,(sizeof(InitDlg)/sizeof(InitDlg[0])),0,0,ManageGroupDialogProc,(DWORD)params);
  if(DlgCode==6)
  {
    if(params[updateName])
    {
      wchar_t NewGroup[GNLEN];
#ifdef UNICODE
      _tcscpy(NewGroup,dialog.Str(indexName));
#else
      MultiByteToWideChar(CP_OEMCP,0,DialogItems[indexName].Data,-1,NewGroup,sizeof(NewGroup)/sizeof(NewGroup[0]));
#endif
      if(panel->global)
      {
        GROUP_INFO_0 info;
        info.grpi0_name=NewGroup;
        if(type)
        {
          if(NetGroupSetInfo(panel->domain,group,0,(LPBYTE)&info,NULL)==NERR_Success)
            res=true;
        }
        else
        {
          if(NetGroupAdd(panel->domain,0,(LPBYTE)&info,NULL)==NERR_Success)
            res=true;
        }
      }
      else
      {
        LOCALGROUP_INFO_0 info;
        info.lgrpi0_name=NewGroup;
        if(type)
        {
          if(NetLocalGroupSetInfo(panel->computer_ptr,group,0,(LPBYTE)&info,NULL)==NERR_Success)
            res=true;
        }
        else
        {
          if(NetLocalGroupAdd(panel->computer_ptr,0,(LPBYTE)&info,NULL)==NERR_Success)
            res=true;
        }
      }
      if(res) wcscpy(group,NewGroup); //???
    }
    if(params[updateComment])
    {
      wchar_t NewGroupComment[MAXCOMMENTSZ];
#ifdef UNICODE
      _tcscpy(NewGroupComment,dialog.Str(indexComment));
#else
      MultiByteToWideChar(CP_OEMCP,0,DialogItems[indexComment].Data,-1,NewGroupComment,sizeof(NewGroupComment)/sizeof(NewGroupComment[0]));
#endif
      if(panel->global)
      {
        GROUP_INFO_1002 info;
        info.grpi1002_comment=NewGroupComment;
        if(NetGroupSetInfo(panel->domain,group,1002,(LPBYTE)&info,NULL)==NERR_Success)
          res=true;
      }
      else
      {
        LOCALGROUP_INFO_1002 info;
        info.lgrpi1002_comment=NewGroupComment;
        if(NetLocalGroupSetInfo(panel->computer_ptr,group,1002,(LPBYTE)&info,NULL)==NERR_Success)
          res=true;
      }
    }
  }
  return res;
}
