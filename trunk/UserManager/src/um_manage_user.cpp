/*
    um_manage_user.cpp
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
#include <lm.h>
#include "umplugin.h"
#include "memory.h"
#include "guid.h"

/*
q  LPWSTR usri3_name;
q  LPWSTR usri3_password;
-  DWORD usri3_password_age;
-  DWORD usri3_priv;
a  LPWSTR usri3_home_dir;
q  LPWSTR usri3_comment;
1234  DWORD usri3_flags;
a  LPWSTR usri3_script_path;
-  DWORD usri3_auth_flags;
q  LPWSTR usri3_full_name;
q  LPWSTR usri3_usr_comment;
q  LPWSTR usri3_parms;
  LPWSTR usri3_workstations;
-  DWORD usri3_last_logon;
-  DWORD usri3_last_logoff;
  DWORD usri3_acct_expires;
  DWORD usri3_max_storage;
  DWORD usri3_units_per_week;
  PBYTE usri3_logon_hours;
  DWORD usri3_bad_pw_count;
  DWORD usri3_num_logons;
- LPWSTR usri3_logon_server;
  DWORD usri3_country_code;
  DWORD usri3_code_page;
-  DWORD usri3_user_id;
-  DWORD usri3_primary_group_id;
a  LPWSTR usri3_profile;
a  LPWSTR usri3_home_dir_drive;
5  DWORD usri3_password_expired;
*/

NET_API_STATUS UpdateName(wchar_t *server,wchar_t *username,wchar_t *value)
{
  NET_API_STATUS res=0;
  if(wcscmp(username,L""))
  {
    USER_INFO_0 info;
    info.usri0_name=value;
    res=NetUserSetInfo(server,username,0,(LPBYTE)&info,NULL);
  }
  else //add user
  {
    USER_INFO_1 info;
    info.usri1_name=value;
    info.usri1_password=NULL;
    info.usri1_priv=USER_PRIV_USER;
    info.usri1_home_dir=NULL;
    info.usri1_comment=NULL;
    info.usri1_flags=UF_SCRIPT|UF_NORMAL_ACCOUNT;
    info.usri1_script_path=NULL;
    res=NetUserAdd(server,1,(LPBYTE)&info,NULL);
  }
  return res;
}

NET_API_STATUS UpdateFullname(wchar_t *server,wchar_t *username,wchar_t *value)
{
  NET_API_STATUS res=0;
  USER_INFO_1011 info;
  info.usri1011_full_name=value;
  res=NetUserSetInfo(server,username,1011,(LPBYTE)&info,NULL);
  return res;
}

NET_API_STATUS UpdateDescription(wchar_t *server,wchar_t *username,wchar_t *value)
{
  NET_API_STATUS res=0;
  USER_INFO_1007 info;
  info.usri1007_comment=value;
  res=NetUserSetInfo(server,username,1007,(LPBYTE)&info,NULL);
  return res;
}

NET_API_STATUS UpdateComment(wchar_t *server,wchar_t *username,wchar_t *value)
{
  NET_API_STATUS res=0;
  USER_INFO_1012 info;
  info.usri1012_usr_comment=value;
  res=NetUserSetInfo(server,username,1012,(LPBYTE)&info,NULL);
  return res;
}

NET_API_STATUS UpdateParams(wchar_t *server,wchar_t *username,wchar_t *value)
{
  NET_API_STATUS res=0;
  USER_INFO_3 *info;
  res=NetUserGetInfo(server,username,3,(LPBYTE *)&info);
  if(res==NERR_Success)
  {
    info->usri3_parms=value;
    res=NetUserSetInfo(server,username,3,(LPBYTE)info,NULL);
    NetApiBufferFree(info);
  }
  return res;
}

NET_API_STATUS UpdatePwd(wchar_t *server,wchar_t *username,wchar_t *value)
{
  NET_API_STATUS res=0;
  USER_INFO_1003 info;
  info.usri1003_password=value;
  res=NetUserSetInfo(server,username,1003,(LPBYTE)&info,NULL);
  return res;
}

NET_API_STATUS UpdateExpired(wchar_t *server,wchar_t *username,wchar_t *value)
{
  NET_API_STATUS res=0;
  USER_INFO_3 *info;
  res=NetUserGetInfo(server,username,3,(LPBYTE *)&info);
  if(res==NERR_Success)
  {
    info->usri3_password_expired=*((int *)value);
    res=NetUserSetInfo(server,username,3,(LPBYTE)info,NULL);
    NetApiBufferFree(info);
  }
  return res;
}

NET_API_STATUS UpdateFlag(wchar_t *server,wchar_t *username,int value,DWORD flag)
{
  NET_API_STATUS res=0;
  USER_INFO_1 *infoin;
  res=NetUserGetInfo(server,username,1,(LPBYTE *)&infoin);
  if(res==NERR_Success)
  {
    USER_INFO_1008 info;
    if(value)
      info.usri1008_flags=infoin->usri1_flags|flag;
    else
      info.usri1008_flags=infoin->usri1_flags&~flag;
    res=NetUserSetInfo(server,username,1008,(LPBYTE)&info,NULL);
    NetApiBufferFree(infoin);
  }
  return res;
}

NET_API_STATUS UpdateFlag1(wchar_t *server,wchar_t *username,wchar_t *value)
{
  return UpdateFlag(server,username,*((int *)value),UF_PASSWD_CANT_CHANGE);
}

NET_API_STATUS UpdateFlag2(wchar_t *server,wchar_t *username,wchar_t *value)
{
  return UpdateFlag(server,username,*((int *)value),UF_DONT_EXPIRE_PASSWD);
}

NET_API_STATUS UpdateFlag3(wchar_t *server,wchar_t *username,wchar_t *value)
{
  return UpdateFlag(server,username,*((int *)value),UF_ACCOUNTDISABLE);
}

NET_API_STATUS UpdateFlag4(wchar_t *server,wchar_t *username,wchar_t *value)
{
  return UpdateFlag(server,username,*((int *)value),UF_LOCKOUT);
}

NET_API_STATUS UpdateProfile(wchar_t *server,wchar_t *username,wchar_t *value)
{
  NET_API_STATUS res=0;
  USER_INFO_1052 info;
  info.usri1052_profile=value;
  res=NetUserSetInfo(server,username,1052,(LPBYTE)&info,NULL);
  return res;
}

NET_API_STATUS UpdateScript(wchar_t *server,wchar_t *username,wchar_t *value)
{
  NET_API_STATUS res=0;
  USER_INFO_1009 info;
  info.usri1009_script_path=value;
  res=NetUserSetInfo(server,username,1009,(LPBYTE)&info,NULL);
  return res;
}

NET_API_STATUS UpdateDrive(wchar_t *server,wchar_t *username,wchar_t *value)
{
  NET_API_STATUS res=0;
  USER_INFO_1053 info;
  info.usri1053_home_dir_drive=value;
  res=NetUserSetInfo(server,username,1053,(LPBYTE)&info,NULL);
  return res;
}

NET_API_STATUS UpdateDir(wchar_t *server,wchar_t *username,wchar_t *value)
{
  NET_API_STATUS res=0;
  USER_INFO_1006 info;
  info.usri1006_home_dir=value;
  res=NetUserSetInfo(server,username,1006,(LPBYTE)&info,NULL);
  return res;
}

NET_API_STATUS UpdateWorkstations(wchar_t *server,wchar_t *username,wchar_t *value)
{
  NET_API_STATUS res=0;
  USER_INFO_1014 info;
  info.usri1014_workstations=value;
  res=NetUserSetInfo(server,username,1014,(LPBYTE)&info,NULL);
  return res;
}

typedef NET_API_STATUS (*update_func)(wchar_t *server,wchar_t *username,wchar_t *value);

const int indexName=2;
const int indexFullname=4;
const int indexDescription=6;
const int indexComment=8;
const int indexParams=10;
const int indexPwd1=12;
const int indexPwd2=14;
const int indexExpired=15;
const int indexFlags=16;
const int indexOk=21;
const int indexProfile=23;
const int indexPProfile=2;
const int indexPScript=4;
const int indexPDrive=6;
const int indexPDir=7;
const int indexPWorkstations=9;
const int indexPOk=11;
const int updateName=0;
const int updateFullname=1;
const int updateDescription=2;
const int updateComment=3;
const int updateParams=4;
const int updatePwd=5;
const int updateExpired=6;
const int updateFlags=7;
const int updateProfile=8;
const int updateScript=9;
const int updateHomeDrive=10;
const int updateHomeDir=11;
const int updateWorkstations=12;

struct Link
{
  int from;
  int to;
  bool name;
  update_func update;
};

const Link links_main[]=
{
  {indexName,updateName,true,UpdateName},
  {indexFullname,updateFullname,false,UpdateFullname},
  {indexDescription,updateDescription,false,UpdateDescription},
  {indexComment,updateComment,false,UpdateComment},
  {indexParams,updateParams,false,UpdateParams},
  {indexPwd1,updatePwd,false,UpdatePwd},
  {indexPwd2,updatePwd,false,NULL},
  {indexFlags,updateFlags,false,UpdateFlag1},
  {indexFlags+1,updateFlags,false,UpdateFlag2},
  {indexFlags+2,updateFlags,false,UpdateFlag3},
  {indexFlags+3,updateFlags,false,UpdateFlag4},
  {indexExpired,updateExpired,false,UpdateExpired},
};

const Link links_profile[]=
{
  {indexPProfile,updateProfile,false,UpdateProfile},
  {indexPScript,updateScript,false,UpdateScript},
  {indexPDrive,updateHomeDrive,false,UpdateDrive},
  {indexPDir,updateHomeDir,false,UpdateDir},
  {indexPWorkstations,updateWorkstations,false,UpdateWorkstations},
};

struct UserProfileItem
{
  int Type;
  TCHAR Data[512];
};

struct UserProfile
{
  UserProfileItem items[ArraySize(links_profile)];
};

struct UserParam
{
  bool changed[16];
  UserProfile *profile;
};

static INT_PTR WINAPI ProfileDialogProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
  UserParam *DlgParams=(UserParam *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_SETDLGDATA,0,Param2);
      Info.SendDlgMessage(hDlg,DM_SETMAXTEXTLENGTH,indexPProfile,(void*)(MAX_PATH-1));
      Info.SendDlgMessage(hDlg,DM_SETMAXTEXTLENGTH,indexPScript,(void*)(MAX_PATH-1));
      Info.SendDlgMessage(hDlg,DM_SETMAXTEXTLENGTH,indexPDir,(void*)(MAX_PATH-1));
      break;
    case DN_EDITCHANGE:
      for(unsigned int i=0;i<sizeof(links_profile)/sizeof(links_profile[0]);i++)
      {
        if(links_profile[i].from==Param1)
        {
          DlgParams->changed[links_profile[i].to]=true;
          break;
        }
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

static void UpdateItem(int Index,UserParam *DlgParams)
{
  for(unsigned int i=0;i<sizeof(links_main)/sizeof(links_main[0]);i++)
  {
    if(links_main[i].from==Index)
    {
      DlgParams->changed[links_main[i].to]=true;
      break;
    }
  }
}

static void UpdateExpired(HANDLE hDlg,UserParam *DlgParams)
{
  FarDialogItem DialogItem;
  Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,indexExpired,&DialogItem);
  if(DialogItem.Selected)
  {
    for(int i=0;i<2;i++)
    {
      Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,indexFlags+i,&DialogItem);
      if(DialogItem.Selected)
      {
        DialogItem.Selected=FALSE;
        UpdateItem(indexFlags+i,DlgParams);
      }
      DialogItem.Flags|=DIF_DISABLE;
      Info.SendDlgMessage(hDlg,DM_SETDLGITEMSHORT,indexFlags+i,&DialogItem);
    }
  }
  else
  {
    for(int i=0;i<2;i++)
    {
      Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,indexFlags+i,&DialogItem);
      DialogItem.Flags&=~DIF_DISABLE;
      Info.SendDlgMessage(hDlg,DM_SETDLGITEMSHORT,indexFlags+i,&DialogItem);
    }
  }
}

static INT_PTR WINAPI ManageGroupUserDialogProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
  UserParam *DlgParams=(UserParam *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_SETDLGDATA,0,Param2);
      Info.SendDlgMessage(hDlg,DM_SETMAXTEXTLENGTH,indexName,(void*)(UNLEN-1));
      Info.SendDlgMessage(hDlg,DM_SETMAXTEXTLENGTH,indexPwd1,(void*)(PWLEN-1));
      Info.SendDlgMessage(hDlg,DM_SETMAXTEXTLENGTH,indexPwd2,(void*)(PWLEN-1));
      UpdateExpired(hDlg,(UserParam *)Param2);
      break;
    case DN_CONTROLINPUT:
      {
        const INPUT_RECORD* record=(const INPUT_RECORD*)Param2;
        if(record->EventType==KEY_EVENT)
        {
          if((Param1==indexPwd1)||(Param1==indexPwd2))
          {
            if(!DlgParams->changed[updatePwd])
              Info.SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,Param1,(void*)1);
          }
        }
      }
      break;
    case DN_BTNCLICK:
      UpdateExpired(hDlg,DlgParams);
      if(Param1==(indexFlags+1))
      {
        FarDialogItem DialogItem;
        Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,indexFlags+3,&DialogItem);
        DialogItem.Flags|=DIF_DISABLE;
        Info.SendDlgMessage(hDlg,DM_SETDLGITEMSHORT,indexFlags+3,&DialogItem);
      }
      if(Param1==indexProfile)
      {
        UserParam *old_params=(UserParam *)malloc(sizeof(UserParam));
        if(old_params)
        {
          memcpy(old_params,DlgParams,sizeof(UserParam));
          /*
            0000000000111111111122222222223333333333444444444455555555556666666666777777
            0123456789012345678901234567890123456789012345678901234567890123456789012345
          00                                                                            00
          01   ЩЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЛ   01
          02   К &User Profile Path:                                                К   02
          03   К лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл К   03
          04   К &Logon Script Name:                                                К   04
          05   К лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл К   05
          06   К &Home Directory:                                                   К   06
          07   К ллл лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл К   07
          08   К &Workstations                                                      К   08
          09   К лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл К   09
          10   ЧФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЖ   10
          11   К [ OK ] [ Cancel ]                                                  К   11
          12   ШЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭМ   12
          13                                                                            13
            0000000000111111111122222222223333333333444444444455555555556666666666777777
            0123456789012345678901234567890123456789012345678901234567890123456789012345
          */
          static const TCHAR *ProfileHistoryName=_T("UserManager\\Profile");
          static const TCHAR *ScriptHistoryName=_T("UserManager\\Script");
          static const TCHAR *HomeHistoryName=_T("UserManager\\Home");
          static const TCHAR *WorkstationsHistoryName=_T("UserManager\\Workstations");
          FarDialogItem DialogItemsProfile[]=
          {
          /* 0*/  {DI_DOUBLEBOX,3, 1,72,12,{0},NULL,                   NULL,0,                                _T(""),                   0,0},
          /* 1*/  {DI_TEXT,     5, 2, 0, 0,{0},NULL,                   NULL,0,                                GetMsg(mUserProfile),     0,0},
          /* 2*/  {DI_EDIT,     5, 3,70, 0,{0},ProfileHistoryName,     NULL,DIF_HISTORY|DIF_FOCUS,            _T(""),                   0,0},
          /* 3*/  {DI_TEXT,     5, 4, 0, 0,{0},NULL,                   NULL,0,                                GetMsg(mUserScript),      0,0},
          /* 4*/  {DI_EDIT,     5, 5,70, 0,{0},ScriptHistoryName,      NULL,DIF_HISTORY,                      _T(""),                   0,0},
          /* 5*/  {DI_TEXT,     5, 6, 0, 0,{0},NULL,                   NULL,0,                                GetMsg(mUserHome),        0,0},
          /* 6*/  {DI_COMBOBOX, 5, 7, 7, 0,{0},NULL,                   NULL,DIF_DROPDOWNLIST,                 _T(""),                   0,0},
          /* 7*/  {DI_EDIT,     9, 7,70, 0,{0},HomeHistoryName,        NULL,DIF_HISTORY,                      _T(""),                   0,0},
          /* 8*/  {DI_TEXT,     5, 8, 0, 0,{0},NULL,                   NULL,0,                                GetMsg(mUserWorkstations),0,0},
          /* 9*/  {DI_EDIT,     5, 9,70, 0,{0},WorkstationsHistoryName,NULL,DIF_HISTORY,                      _T(""),                   0,0},
          /*10*/  {DI_TEXT,     5,10, 0, 0,{0},NULL,                   NULL,DIF_BOXCOLOR|DIF_SEPARATOR,       _T(""),                   0,0},
          /*11*/  {DI_BUTTON,   0,11, 0, 0,{0},NULL,                   NULL,DIF_CENTERGROUP|DIF_DEFAULTBUTTON,GetMsg(mButtonOk),        0,0},
          /*12*/  {DI_BUTTON,   0,11, 0, 0,{0},NULL,                   NULL,DIF_CENTERGROUP,                  GetMsg(mButtonCancel),    0,0},
          };
          TCHAR profile_title[128];
          FarListItem Drive[25];
          TCHAR DriveText[ArraySize(Drive)][3];
          FarList Drives={sizeof(Drive)/sizeof(Drive[0]),Drive};
          Drive[0].Flags=0;
          DriveText[0][0]=0;
          Drive[0].Text=DriveText[0];
          for(int i=1;i<25;i++)
          {
            Drive[i].Flags=0;
            DriveText[i][0]='A'+i+1;
            DriveText[i][1]=':';
            DriveText[i][2]=0;
            Drive[i].Text=DriveText[i];
          }
          DialogItemsProfile[indexPDrive].ListItems=&Drives;
          FSF.sprintf(profile_title,GetMsg(mUserProfileTitle),(const wchar_t*)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,indexName,0));
          DialogItemsProfile[0].Data=profile_title;
          for(size_t ii=0;ii<ArraySize(links_profile);ii++)
          {
            INIT_DLG_DATA(DialogItemsProfile[links_profile[ii].from],DlgParams->profile->items[ii].Data);
          }
          CFarDialog dialog;
          int DlgCode=dialog.Execute(MainGuid,ProfileGuid,-1,-1,76,14,NULL,DialogItemsProfile,ArraySize(DialogItemsProfile),0,0,ProfileDialogProc,DlgParams);
          if(DlgCode==indexPOk)
          {
            for(size_t ii=0;ii<ArraySize(links_profile);ii++)
            {
              _tcscpy(DlgParams->profile->items[ii].Data,dialog.Str(links_profile[ii].from));
            }
          }
          else
          {
            memcpy(DlgParams,old_params,sizeof(UserParam));
          }
        }
        free(old_params);
      }
    case DN_EDITCHANGE:
      UpdateItem(Param1,DlgParams);
      break;
    case DN_CLOSE:
      if(Param1==indexOk)
      {
        if(DlgParams->changed[updatePwd])
        {
          TCHAR pwd1[512],pwd2[512];
          FarDialogItemData data_pwd1,data_pwd2;
          data_pwd1.PtrLength=sizeof(pwd1);
          data_pwd1.PtrData=pwd1;
          data_pwd2.PtrLength=sizeof(pwd2);
          data_pwd2.PtrData=pwd2;
          Info.SendDlgMessage(hDlg,DM_GETTEXT,indexPwd1,&data_pwd1);
          Info.SendDlgMessage(hDlg,DM_GETTEXT,indexPwd2,&data_pwd2);
          if(_tcscmp(pwd1,pwd2))
          {
            const TCHAR *MsgItems[]={GetMsg(mError),GetMsg(mOtherPwdError1),GetMsg(mOtherPwdError2),GetMsg(mOtherPwdError3),GetMsg(mButtonOk)};
            Info.Message(&MainGuid,&PwdErrorMessageGuid,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
            return FALSE;
          }
        }
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool ManageUser(UserManager *panel,bool type)
{
  bool res=false;
  CFarPanel pInfo((HANDLE)panel,FCTL_GETPANELINFO);
  if(pInfo.IsOk())
  {
    if(pInfo.ItemsNumber()>0)
    {
      if(pInfo[pInfo.CurrentItem()].FileAttributes&FILE_ATTRIBUTE_DIRECTORY&&type)
      {
        if(pInfo[pInfo.CurrentItem()].UserData) //don't call on ".." entry
          res=ManageGroup(panel,true,GetWideNameFromUserData(pInfo[pInfo.CurrentItem()].UserData));
      }
      else
      {
        wchar_t username[512]=L"",*server=(panel->global)?(panel->domain):(panel->computer_ptr);
        /*
          0000000000111111111122222222223333333333444444444455555555556666666666777777
          0123456789012345678901234567890123456789012345678901234567890123456789012345
        00                                                                            00
        01   ЩЭЭЭЭЭЭЭЭЭЭЭЭЭЭ New user ЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЛ   01
        02   К &Username:                                                         К   02
        03   К лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл К   03
        04   К Full &name                                                         К   04
        05   К лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл К   05
        06   К &Description                                                       К   06
        07   К лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл К   07
        08   К User comment:                                                      К   08
        09   К лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл К   09
        10   К User params                                                        К   10
        11   К лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл К   11
        12   К &Password                                                          К   12
        13   К лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл К   13
        14   К &Confirm password                                                  К   14
        15   К лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл К   15
        16   К [ ] User &must change password at next logon                       К   16
        17   К [ ] U&ser cannot change password                                   К   17
        18   К [ ] Pass&word never expiries                                       К   18
        19   К [ ] Account disa&bled                                              К   19
        20   К [ ] Account loc&ked out                                            К   20
        21   ЧФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФЖ   21
        22   К [ OK ] [ Cancel ] [ Profile ] [ Other ]                            К   22
        23   ШЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭЭМ   23
        24                                                                            24
          0000000000111111111122222222223333333333444444444455555555556666666666777777
          0123456789012345678901234567890123456789012345678901234567890123456789012345
        */
        static const TCHAR *UsernameHistoryName=_T("UserManager\\Username");
        static const TCHAR *FullnameHistoryName=_T("UserManager\\Fullname");
        static const TCHAR *DescriptionHistoryName=_T("UserManager\\Description");
        static const TCHAR *CommentHistoryName=_T("UserManager\\Comment");
        static const TCHAR *ParamsHistoryName=_T("UserManager\\Params");
        FarDialogItem DialogItems[]={
        /* 0*/  {DI_DOUBLEBOX,3, 1,72,23,{0},NULL,                  NULL,0,                                GetMsg(mUserNew),          0,0},
        /* 1*/  {DI_TEXT,     5, 2, 0, 0,{0},NULL,                  NULL,0,                                GetMsg(mUserUsername),     0,0},
        /* 2*/  {DI_EDIT,     5, 3,70, 0,{0},UsernameHistoryName,   NULL,DIF_HISTORY|DIF_FOCUS,            _T(""),                    0,0},
        /* 3*/  {DI_TEXT,     5, 4, 0, 0,{0},NULL,                  NULL,0,                                GetMsg(mUserFullname),     0,0},
        /* 4*/  {DI_EDIT,     5, 5,70, 0,{0},FullnameHistoryName,   NULL,DIF_HISTORY,                      _T(""),                    0,0},
        /* 5*/  {DI_TEXT,     5, 6, 0, 0,{0},NULL,                  NULL,0,                                GetMsg(mUserDescription),  0,0},
        /* 6*/  {DI_EDIT,     5, 7,70, 0,{0},DescriptionHistoryName,NULL,DIF_HISTORY,                      _T(""),                    0,0},
        /* 7*/  {DI_TEXT,     5, 8, 0, 0,{0},NULL,                  NULL,0,                                GetMsg(mUserComment),      0,0},
        /* 8*/  {DI_EDIT,     5, 9,70, 0,{0},CommentHistoryName,    NULL,DIF_HISTORY,                      _T(""),                    0,0},
        /* 9*/  {DI_TEXT,     5,10, 0, 0,{0},NULL,                  NULL,0,                                GetMsg(mUserParams),       0,0},
        /*10*/  {DI_EDIT,     5,11,70, 0,{0},ParamsHistoryName,     NULL,DIF_HISTORY,                      _T(""),                    0,0},
        /*11*/  {DI_TEXT,     5,12, 0, 0,{0},NULL,                  NULL,0,                                GetMsg(mUserPassword1),    0,0},
        /*12*/  {DI_PSWEDIT,  5,13,70, 0,{0},NULL,                  NULL,0,                                _T(""),                    0,0},
        /*13*/  {DI_TEXT,     5,14, 0, 0,{0},NULL,                  NULL,0,                                GetMsg(mUserPassword2),    0,0},
        /*14*/  {DI_PSWEDIT,  5,15,70, 0,{0},NULL,                  NULL,0,                                _T(""),                    0,0},
        /*15*/  {DI_CHECKBOX, 5,16, 0, 0,{0},NULL,                  NULL,0,                                GetMsg(mUserChk1),         0,0},
        /*16*/  {DI_CHECKBOX, 5,17, 0, 0,{0},NULL,                  NULL,0,                                GetMsg(mUserChk2),         0,0},
        /*17*/  {DI_CHECKBOX, 5,18, 0, 0,{0},NULL,                  NULL,0,                                GetMsg(mUserChk3),         0,0},
        /*18*/  {DI_CHECKBOX, 5,19, 0, 0,{0},NULL,                  NULL,0,                                GetMsg(mUserChk4),         0,0},
        /*19*/  {DI_CHECKBOX, 5,20, 0, 0,{0},NULL,                  NULL,DIF_DISABLE,                      GetMsg(mUserChk5),         0,0},
        /*20*/  {DI_TEXT,     5,21, 0, 0,{0},NULL,                  NULL,DIF_BOXCOLOR|DIF_SEPARATOR,       _T(""),                    0,0},
        /*21*/  {DI_BUTTON,   0,22, 0, 0,{0},NULL,                  NULL,DIF_CENTERGROUP|DIF_DEFAULTBUTTON,GetMsg(mButtonOk),         0,0},
        /*22*/  {DI_BUTTON,   0,22, 0, 0,{0},NULL,                  NULL,DIF_CENTERGROUP,                  GetMsg(mButtonCancel),     0,0},
        /*23*/  {DI_BUTTON,   0,22, 0, 0,{0},NULL,                  NULL,DIF_CENTERGROUP|DIF_BTNNOCLOSE,   GetMsg(mUserButtonProfile),0,0},
        };
        UserProfile profile_data;
        UserParam params={{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},&profile_data};
        USER_INFO_3 *info=NULL;
        TCHAR title[128];
        if(type) //read properties
        {
          wcscpy(username,GetWideNameFromUserData(pInfo[pInfo.CurrentItem()].UserData));
          if(NetUserGetInfo(server,username,3,(LPBYTE *)&info)==NERR_Success)
          {
            DialogItems[indexName].Data=info->usri3_name;
            DialogItems[indexFullname].Data=info->usri3_full_name;
            DialogItems[indexDescription].Data=info->usri3_comment;
            DialogItems[indexComment].Data=info->usri3_usr_comment;
            DialogItems[indexParams].Data=info->usri3_parms;
            if(info->usri3_password_expired)
              DialogItems[indexExpired].Selected=1;
            if(info->usri3_flags&UF_PASSWD_CANT_CHANGE) DialogItems[indexFlags].Selected=1;
            if(info->usri3_flags&UF_DONT_EXPIRE_PASSWD) DialogItems[indexFlags+1].Selected=1;
            if(info->usri3_flags&UF_ACCOUNTDISABLE) DialogItems[indexFlags+2].Selected=1;
            if(info->usri3_flags&UF_LOCKOUT)
            {
              DialogItems[indexFlags+3].Selected=1;
              DialogItems[indexFlags+3].Flags&=~DIF_DISABLE;
            }
            FSF.sprintf(title,GetMsg(mUserEdit),DialogItems[indexName].Data);
            DialogItems[0].Data=title;
            DialogItems[indexPwd1].Data=_T("@@@@@@@@");
            DialogItems[indexPwd2].Data=_T("@@@@@@@@");

            _tcscpy(profile_data.items[0].Data,info->usri3_profile);
            _tcscpy(profile_data.items[1].Data,info->usri3_script_path);
            _tcscpy(profile_data.items[2].Data,info->usri3_home_dir_drive);
            _tcscpy(profile_data.items[3].Data,info->usri3_home_dir);
            _tcscpy(profile_data.items[4].Data,info->usri3_workstations);
            profile_data.items[0].Type=profile_data.items[1].Type=profile_data.items[3].Type=profile_data.items[4].Type=DI_EDIT;
            profile_data.items[2].Type=DI_COMBOBOX;
          }
          else return false;
        }
        else
        {
          DialogItems[indexProfile].Flags|=DIF_DISABLE;
        }
        CFarDialog dialog;
        int DlgCode=dialog.Execute(MainGuid,ManageUserGuid,-1,-1,76,25,NULL,DialogItems,ArraySize(DialogItems),0,0,ManageGroupUserDialogProc,&params);
        if(DlgCode==indexOk)
        {
          wchar_t data[512],*data_ptr;
          NET_API_STATUS net_res;
          for(unsigned int i=0;i<sizeof(links_main)/sizeof(links_main[0]);i++)
          {
            if((params.changed[links_main[i].to]||(!type))&&links_main[i].update)
            {
              data_ptr=NULL;
              int data_int=0;
              switch(dialog.Type(links_main[i].from))
              {
                case DI_EDIT:
                case DI_PSWEDIT:
                  _tcscpy(data,dialog.Str(links_main[i].from));
                  data_ptr=data;
                  break;
                case DI_CHECKBOX:
                  data_int=dialog.Check(links_main[i].from);
                  data_ptr=(wchar_t *)&data_int;
                  break;
              }
              net_res=links_main[i].update(server,username,data_ptr);
              if(links_main[i].name&&(net_res==NERR_Success))
                wcscpy(username,data);
              if(net_res==NERR_Success) res=true;
            }
          }
          for(unsigned int i=0;i<sizeof(links_profile)/sizeof(links_profile[0]);i++)
          {
            if(params.changed[links_profile[i].to]&&links_profile[i].update)
            {
              data_ptr=NULL;
              int data_int=0;
              switch(profile_data.items[i].Type)
              {
                case DI_COMBOBOX:
                case DI_EDIT:
                  _tcscpy(data,profile_data.items[i].Data);
                  data_ptr=data;
                  break;
              }
              net_res=links_profile[i].update(server,username,data_ptr);
              if(links_profile[i].name&&(net_res==NERR_Success))
                wcscpy(username,data);
              if(net_res==NERR_Success) res=true;
            }
          }
          if((!type)&&res&&(panel->level==levelUsers))
          {
            if(panel->global)
            {
              NetGroupAddUser(server,panel->nonfixed,username);
            }
            else
            {
              LOCALGROUP_MEMBERS_INFO_3 new_member;
              new_member.lgrmi3_domainandname=username;
              NetLocalGroupAddMembers(server,panel->nonfixed,3,(LPBYTE)&new_member,1);
            }
          }
        }
        if(info) NetApiBufferFree(info);
      }
    }
  }
  return res;
}
