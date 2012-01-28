/*
    um_utils.cpp
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
#include <ntsecapi.h>
#include "umplugin.h"
#include "memory.h"
#include "guid.h"

const TCHAR *GetMsg(int MsgId)
{
  return Info.GetMsg(&MainGuid,MsgId);
}

void ShowError(DWORD Error)
{
  if(!Error) SetLastError(Error);
  const TCHAR *MsgItems[]={GetMsg(mError),GetMsg(mButtonOk)};
  Info.Message(&MainGuid,&ErrorMessageGuid,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
}

void ShowCustomError(int index)
{
  const TCHAR *MsgItems[]={GetMsg(mError),GetMsg(index),GetMsg(mButtonOk)};
  Info.Message(&MainGuid,&CustomErrorMessageGuid,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
}

void GetCurrentPath(int level,TCHAR *nonfixed,TCHAR *result)
{
  TCHAR buff[MAX_PATH];
  const TCHAR *dirs[]=
  {
    //files
    _T(""),
    GetMsg(mDirRights),
    GetMsg(mDirNewFolder),
    GetMsg(mDirNewFile),
    GetMsg(mDirAudit),
    GetMsg(mDirNewFolder),
    GetMsg(mDirNewFile),
    //reg
    _T(""),
    GetMsg(mDirRights),
    GetMsg(mDirNewKey),
    _T("Pad"),
    GetMsg(mDirAudit),
    GetMsg(mDirNewKey),
    _T("Pad"),
    //share
    GetMsg(mDirShared),
    NULL,
    GetMsg(mDirRights),
    //printer
    _T(""),
    GetMsg(mDirRights),
    GetMsg(mDirNewContainer),
    GetMsg(mDirNewJob),
    GetMsg(mDirAudit),
    GetMsg(mDirNewContainer),
    GetMsg(mDirNewJob),
    //printer share
    GetMsg(mDirShared),
    //users
    _T(""),
    NULL,
    //rights
    _T(""),
    NULL,
  };
  _tcscpy(result,_T(""));
  while(level!=up_dirs[level])
  {
    _tcscpy(buff,result);
    if(dirs[level])
      _tcscpy(result,dirs[level]);
    else
      _tcscpy(result,nonfixed);
    if(buff[0])
    {
      _tcscat(result,_T("\\"));
      _tcscat(result,buff);
    }
    level=up_dirs[level];
  }
}

DWORD generic_mask_to_file_mask(DWORD mask)
{
  DWORD res=mask&FILE_ALL_ACCESS;
  if(mask&GENERIC_ALL)
    res=FILE_ALL_ACCESS;
  else
  {
    if(mask&GENERIC_READ)
      res|=FILE_GENERIC_READ;
    if(mask&GENERIC_WRITE)
      res|=FILE_GENERIC_WRITE;
    if(mask&GENERIC_EXECUTE)
      res|=FILE_GENERIC_EXECUTE;
  }
  return res;
}

DWORD generic_mask_to_reg_mask(DWORD mask)
{
  DWORD res=mask&KEY_ALL_ACCESS;
  if(mask&GENERIC_ALL)
    res=KEY_ALL_ACCESS;
  else
  {
    if(mask&GENERIC_READ)
      res|=KEY_READ;
    if(mask&GENERIC_WRITE)
      res|=KEY_WRITE;
    if(mask&GENERIC_EXECUTE)
      res|=KEY_EXECUTE;
  }
  return res;
}

DWORD generic_mask_to_printer_mask(DWORD mask)
{
  DWORD res=mask&PRINTER_ALL_ACCESS;
  if(mask&GENERIC_ALL)
    res=PRINTER_ALL_ACCESS;
  else
  {
    if(mask&GENERIC_READ)
      res|=PRINTER_READ;
    if(mask&GENERIC_WRITE)
      res|=PRINTER_WRITE;
    if(mask&GENERIC_EXECUTE)
      res|=PRINTER_EXECUTE;
  }
  return res;
}

DWORD generic_mask_to_job_mask(DWORD mask)
{
  DWORD res=mask&JOB_ALL_ACCESS;
  if(mask&GENERIC_ALL)
    res=JOB_ALL_ACCESS;
  else
  {
    if(mask&GENERIC_READ)
      res|=JOB_READ;
    if(mask&GENERIC_WRITE)
      res|=JOB_WRITE;
    if(mask&GENERIC_EXECUTE)
      res|=JOB_EXECUTE;
  }
  return res;
}

void EnablePrivilege(const TCHAR *name)
{
  TOKEN_PRIVILEGES priv;
  HANDLE token;
  priv.PrivilegeCount=1;
  priv.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;

  if(LookupPrivilegeValue(0,name,&(priv.Privileges[0].Luid)))
  {
    if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&token))
    {
      AdjustTokenPrivileges(token,FALSE,&priv,sizeof(priv),0,0);
      CloseHandle(token);
    }
  }
}

bool IsPrivilegeEnabled(const TCHAR *name)
{
  bool res=false;
  HANDLE token=INVALID_HANDLE_VALUE; LUID li;
  unsigned long size; PTOKEN_PRIVILEGES privileges=NULL;
  if(LookupPrivilegeValue(0,name,&li))
  {
    if(OpenProcessToken(GetCurrentProcess(),TOKEN_READ,&token))
    {
      GetTokenInformation(token,TokenPrivileges,0,0,&size);
      privileges=(PTOKEN_PRIVILEGES)malloc(size);
      if(privileges)
      {
        if(GetTokenInformation(token,TokenPrivileges,privileges,size,&size))
        {
          for(unsigned int ii=0;ii<privileges->PrivilegeCount;ii++)
          {
            if(li.LowPart==(privileges->Privileges[ii].Luid.LowPart)&&li.HighPart==(privileges->Privileges[ii].Luid.HighPart)) //FIXME: compare structs
            {
              if(privileges->Privileges[ii].Attributes&SE_PRIVILEGE_ENABLED)
              {
                res=true;
                break;
              }
            }
          }
        }
        free(privileges);
      }
      CloseHandle(token);
    }
  }
  return res;
}

bool CheckChDir(HANDLE hPlugin,const TCHAR *NewDir,TCHAR *RealDir,wchar_t *RealDirW,int *level)
{
  bool res=false;
  CFarPanel pInfo(hPlugin,FCTL_GETPANELINFO);
  if(pInfo.IsOk())
  {
    for(int ii=0;ii<pInfo.ItemsNumber();ii++)
    {
      if((pInfo[ii].FileAttributes&FILE_ATTRIBUTE_DIRECTORY)&&_tcscmp(pInfo[ii].FileName,_T("..")))
      {
        if(!_tcsicmp(pInfo[ii].FileName,NewDir))
        {
          if(pInfo[ii].Flags&PPIF_USERDATA)
          {
            _tcscpy(RealDir,pInfo[ii].FileName);
            wcscpy(RealDirW,GetWideNameFromUserData(pInfo[ii].UserData));
            *level=GetLevelFromUserData(pInfo[ii].UserData);
            res=true;
            break;
          }
        }
      }
    }
  }
  return res;
}

int NumberType(int num)
{
  int Result=0,mod;
  mod=num%100;
  if((mod>20)||(mod<5))
  {
    mod=num%10;
    if((mod<5)&&(mod>0)) Result--;
    if(mod==1) Result--;
  }
  return Result;
}

PSECURITY_DESCRIPTOR CreateDefaultSD(void)
{
  PSECURITY_DESCRIPTOR SD=NULL; bool ok=false;
  SD=(PSECURITY_DESCRIPTOR)malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
  if(SD)
  {
    if(InitializeSecurityDescriptor(SD,SECURITY_DESCRIPTOR_REVISION))
      if(SetSecurityDescriptorDacl(SD,TRUE,(PACL)NULL,FALSE))
        ok=true;
    if(!ok)
    {
      free(SD);
      SD=NULL;
    }
  }
  return SD;
}

PACL CreateDefaultAcl(int level)
{
  SID_IDENTIFIER_AUTHORITY SIDAuthEveryone={SECURITY_WORLD_SID_AUTHORITY};
  PSID AccessSid=NULL;
  DWORD acl_size=sizeof(ACL);
  PACL Acl=NULL; bool ok=false;
  if(AllocateAndInitializeSid(&SIDAuthEveryone,1,SECURITY_WORLD_RID,0,0,0,0,0,0,0,&AccessSid))
  {
    acl_size+=sizeof(ACCESS_ALLOWED_ACE)-sizeof(DWORD)+GetLengthSid(AccessSid);
    Acl=(PACL)malloc(acl_size);
    if(InitializeAcl(Acl,acl_size,ACL_REVISION))
      switch(default_acl[level])
      {
        case 1:
          if(AddAccessAllowedAce(Acl,ACL_REVISION,FILE_ALL_ACCESS,AccessSid))
            ok=true;
          break;
        case 2:
          ok=true;
          break;
      }
    if(AccessSid) FreeSid(AccessSid);
  }
  return Acl;
}

TCHAR *get_access_string(int level,int mask)
{
  static TCHAR access_string[7];
  if((mask&common_full_access[level])==common_full_access[level])
    _tcscpy(access_string,_T("FULL"));
  else
  {
    const TCHAR *res[]={_T(" "),_T(" "),_T(" "),_T(" "),_T(" "),_T(" ")};
    const TCHAR *full[]={_T("R"),_T("W"),_T("X"),_T("D"),_T("P"),_T("O")};
    _tcscpy(access_string,_T(""));
    for(int ii=0;ii<6;ii++)
    {
      if((mask&common_rights[level][ii])==common_rights[level][ii])
        res[ii]=full[ii];
      _tcscat(access_string,res[ii]);
    }
  }
  return access_string;
}

const RegRoot RegRoots[]=
{
  {L"HKEY_CLASSES_ROOT\\",HKEY_CLASSES_ROOT},
  {L"HKEY_CURRENT_USER\\",HKEY_CURRENT_USER},
  {L"HKEY_LOCAL_MACHINE\\",HKEY_LOCAL_MACHINE},
  {L"HKEY_USERS\\",HKEY_USERS},
  {L"HKEY_PERFORMANCE_DATA\\",HKEY_PERFORMANCE_DATA},
  {L"HKEY_CURRENT_CONFIG\\",HKEY_CURRENT_CONFIG},
  {L"HKEY_DYN_DATA\\",HKEY_DYN_DATA},
};

static bool check_for_registry(wchar_t *path,HKEY *hKey,wchar_t *rest_path)
{
  bool res=false;
  if('\\'==path[0]) ++path;
  for(unsigned int ii=0;ii<(sizeof(RegRoots)/sizeof(RegRoots[0]));ii++)
  {
    if(!_wcsnicmp(path,RegRoots[ii].root,wcslen(RegRoots[ii].root)))
    {
      res=true;
      *hKey=RegRoots[ii].value;
      wcscpy(rest_path,path+wcslen(RegRoots[ii].root));
      break;
    }
  }
  return res;
}

static bool check_for_printer(const wchar_t *printer_name,wchar_t *host)
{
  bool res=false;
  HANDLE printer;
  if(printer_name&&OpenPrinterW((wchar_t*)printer_name,&printer,NULL))
  {
    ClosePrinter(printer);
    wcscpy(host,printer_name);
    res=true;
  }
  return res;
}

static bool check_for_createfile(const wchar_t *device_name,wchar_t *host)
{
  HANDLE handle=CreateFileW(device_name,0,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
  if(handle!=INVALID_HANDLE_VALUE)
  {
    CloseHandle(handle);
    wcscpy(host,device_name);
    return true;
  }
  return false;
}

void wcsaddendslash(wchar_t *string)
{
  int len=wcslen(string);
  if(len)
  {
    for(int ii=0;ii<len;ii++)
    {
      if(string[ii]==L'/')
        string[ii]=L'\\';
    }
    if(string[len-1]!=L'\\')
    {
      string[len]=L'\\';
      string[len+1]=0;
    }
  }
  return;
}

int parse_dir(TCHAR *root_oem,const TCHAR *obj_oem,const wchar_t *obj,int obj_type,ULONG_PTR *param,wchar_t *host,TCHAR *host_oem)
{
  int result=-1;
  HANDLE testHandle;
  const wchar_t *real_obj=obj;
  wchar_t obj2[MAX_PATH],root[MAX_PATH],path[MAX_PATH];
  if((!real_obj)&&obj_oem)
  {
    _tcscpy(obj2,obj_oem);
    real_obj=obj2;
  }
  _tcscpy(root,root_oem);
  wcscpy(path,root);
  if(real_obj)
  {
    wcsaddendslash(path);
    wcscat(path,real_obj);
  }
  switch(obj_type)
  {
    case pathtypeReal:
    case pathtypeTree:
parse_dir_real:
      if(obj_type==pathtypeUnknown)
      {
        DWORD full_res=FSF.ConvertPath(CPM_FULL,path,host,MAX_PATH);
        if(!full_res||full_res>=MAX_PATH) wcscpy(host,path);
      }
      else wcscpy(host,path);
      *param=0;
      result=levelRoot;
      break;
    case pathtypePlugin:
parse_dir_plugin:
      if(check_for_registry(path,(HKEY *)param,host))
      {
        result=levelRegRoot;
      }
      else if(check_for_printer(real_obj?real_obj:path,host))
      {
        result=levelPrinterRoot;
      }
      //additionals support
      else if(check_for_createfile(real_obj,host))
      {
        result=levelRoot;
      }
      break;
    case pathtypeUnknown:
      {
        //if(GetFileAttributesW(path)!=0xffffffff)
        wchar_t path_real[MAX_PATH];
        FSF.ConvertPath(CPM_FULL,path,path_real,ArraySize(path_real));

        if((testHandle=CreateFileW(path_real,0,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL))!=INVALID_HANDLE_VALUE)
        {
          CloseHandle(testHandle);
          goto parse_dir_real;
        }
        goto parse_dir_plugin;
      }
  }
  if((result>-1)&&host_oem)
  {
    _tcscpy(host_oem,host);
  }
  return result;
}

LSA_HANDLE GetPolicyHandle(wchar_t *computer)
{
  LSA_HANDLE PolicyHandle;
  LSA_OBJECT_ATTRIBUTES ObjectAttributes;
  LSA_UNICODE_STRING SystemName;
  NTSTATUS res;

  memset(&ObjectAttributes,0,sizeof(ObjectAttributes));
  SystemName.Buffer=computer;
  SystemName.Length=wcslen(SystemName.Buffer)*sizeof(wchar_t);
  SystemName.MaximumLength=SystemName.Length+sizeof(wchar_t);

  res=LsaOpenPolicy(&SystemName,&ObjectAttributes,POLICY_ALL_ACCESS,&PolicyHandle);
  if(res!=STATUS_SUCCESS)
  {
    PolicyHandle=NULL;
    SetLastError(LsaNtStatusToWinError(res));
  }
  return PolicyHandle;
}

LONG RegOpenBackupKeyExW(HKEY hKey,LPCWSTR lpSubKey,REGSAM samDesired,PHKEY phkResult)
{
  LONG result=RegOpenKeyExW(hKey,lpSubKey,0,samDesired,phkResult);
  if(result==ERROR_ACCESS_DENIED) result=RegCreateKeyExW(hKey,lpSubKey,0,NULL,REG_OPTION_BACKUP_RESTORE,0,NULL,phkResult,NULL);
  return result;
}
