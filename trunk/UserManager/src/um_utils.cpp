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
#include "../../plugin.hpp"
#include <ntsecapi.h>
#include "umplugin.h"
#include "memory.h"

char *GetMsg(int MsgId)
{
  return((char *)Info.GetMsg(Info.ModuleNumber,MsgId));
}

void InitDialogItems(InitDialogItem *Init,FarDialogItem *Item,int ItemsNumber)
{
  for (int i=0;i<ItemsNumber;i++)
  {
    Item[i].Type=Init[i].Type;
    Item[i].X1=Init[i].X1;
    Item[i].Y1=Init[i].Y1;
    Item[i].X2=Init[i].X2;
    Item[i].Y2=Init[i].Y2;
    Item[i].Focus=Init[i].Focus;
    Item[i].Selected=Init[i].Selected;
    Item[i].Flags=Init[i].Flags;
    Item[i].DefaultButton=Init[i].DefaultButton;
    if ((unsigned)Init[i].Data<2000)
      strcpy(Item[i].Data,GetMsg((unsigned int)Init[i].Data));
    else
      strcpy(Item[i].Data,Init[i].Data);
  }
}

void ShowError(DWORD Error)
{
  if(!Error) SetLastError(Error);
  const char *MsgItems[]={GetMsg(mError),GetMsg(mButtonOk)};
  Info.Message(Info.ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
}

void ShowCustomError(int index)
{
  const char *MsgItems[]={GetMsg(mError),GetMsg(index),GetMsg(mButtonOk)};
  Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
}

void GetCurrentPath(int level,char *nonfixed,char *result)
{
  char buff[MAX_PATH];
  char *dirs[]=
  {
    //files
    "",
    GetMsg(mDirRights),
    GetMsg(mDirNewFolder),
    GetMsg(mDirNewFile),
    GetMsg(mDirAudit),
    GetMsg(mDirNewFolder),
    GetMsg(mDirNewFile),
    //reg
    "",
    GetMsg(mDirRights),
    GetMsg(mDirNewKey),
    "Pad",
    GetMsg(mDirAudit),
    GetMsg(mDirNewKey),
    "Pad",
    //share
    GetMsg(mDirShared),
    NULL,
    GetMsg(mDirRights),
    //printer
    "",
    GetMsg(mDirRights),
    GetMsg(mDirNewContainer),
    GetMsg(mDirNewJob),
    GetMsg(mDirAudit),
    GetMsg(mDirNewContainer),
    GetMsg(mDirNewJob),
    //printer share
    GetMsg(mDirShared),
    //users
    "",
    NULL,
    //rights
    "",
    NULL,
  };
  strcpy(result,"");
  while(level!=up_dirs[level])
  {
    strcpy(buff,result);
    if(dirs[level])
      strcpy(result,dirs[level]);
    else
      strcpy(result,nonfixed);
    if(buff[0])
    {
      strcat(result,"\\");
      strcat(result,buff);
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

void EnablePrivilege(const char *name)
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

bool IsPrivilegeEnabled(const char *name)
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
          for(unsigned int i=0;i<privileges->PrivilegeCount;i++)
          {
            if(li.LowPart==(privileges->Privileges[i].Luid.LowPart)&&li.HighPart==(privileges->Privileges[i].Luid.HighPart)) //FIXME: compare structs
            {
              if(privileges->Privileges[i].Attributes&SE_PRIVILEGE_ENABLED)
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

bool CheckChDir(HANDLE hPlugin,const char *NewDir,char *RealDir,wchar_t *RealDirW,int *level)
{
  bool res=false;
  PanelInfo PInfo;
  Info.Control(hPlugin,FCTL_GETPANELINFO,&PInfo);
  for(int i=0;i<PInfo.ItemsNumber;i++)
  {
    if((PInfo.PanelItems[i].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)&&strcmp(PInfo.PanelItems[i].FindData.cFileName,".."))
    {
      if(!_stricmp(PInfo.PanelItems[i].FindData.cFileName,NewDir))
      {
        if(PInfo.PanelItems[i].Flags&PPIF_USERDATA)
        {
          strcpy(RealDir,PInfo.PanelItems[i].FindData.cFileName);
          wcscpy(RealDirW,GetWideNameFromUserData(PInfo.PanelItems[i].UserData));
          *level=GetLevelFromUserData(PInfo.PanelItems[i].UserData);
          res=true;
          break;
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

void GetSelectedList(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,bool selection)
{
  *pPanelItem=NULL; *pItemsNumber=0;
  PanelInfo PInfo;
  Info.Control(hPlugin,FCTL_GETPANELINFO,&PInfo);
  if(selection)
  {
    *pPanelItem=PInfo.SelectedItems;
    *pItemsNumber=PInfo.SelectedItemsNumber;
  }
  else
  {
    if(PInfo.ItemsNumber>0)
    {
      *pPanelItem=&(PInfo.PanelItems[PInfo.CurrentItem]);
      *pItemsNumber=1;
    }
  }
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

char *get_access_string(int level,int mask)
{
  static char access_string[7];
  if((mask&common_full_access[level])==common_full_access[level])
    strcpy(access_string,"FULL");
  else
  {
    char *res[]={" "," "," "," "," "," "};
    char *full[]={"R","W","X","D","P","O"};
    strcpy(access_string,"");
    for(int i=0;i<6;i++)
    {
      if((mask&common_rights[level][i])==common_rights[level][i])
        res[i]=full[i];
      strcat(access_string,res[i]);
    }
  }
  return access_string;
}

char *get_sid_string(PSID sid)
{
  char *res=NULL;
  UNICODE_STRING sid_str;
  memset(&sid_str,0,sizeof(sid_str));
  if(!RtlConvertSidToUnicodeString(&sid_str,sid,TRUE))
  {
    res=(char *)malloc((sid_str.Length/sizeof(wchar_t)+1)*sizeof(char));
    if(res)
      WideCharToMultiByte(CP_OEMCP,0,sid_str.Buffer,sid_str.Length/sizeof(wchar_t),res,sid_str.Length/sizeof(wchar_t),NULL,NULL);
    RtlFreeUnicodeString(&sid_str);
  }
  return res;
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
  for(unsigned int i=0;i<(sizeof(RegRoots)/sizeof(RegRoots[0]));i++)
  {
    if(!_wcsnicmp(path,RegRoots[i].root,wcslen(RegRoots[i].root)))
    {
      res=true;
      *hKey=RegRoots[i].value;
      wcscpy(rest_path,path+wcslen(RegRoots[i].root));
      break;
    }
  }
  return res;
}

static bool check_for_printer(wchar_t *printer_name,wchar_t *host)
{
  bool res=false;
  HANDLE printer;
  if(printer_name&&OpenPrinterW(printer_name,&printer,NULL))
  {
    ClosePrinter(printer);
    wcscpy(host,printer_name);
    res=true;
  }
  return res;
}

static bool check_for_createfile(wchar_t *device_name,wchar_t *host)
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
    for(int i=0;i<len;i++)
    {
      if(string[i]==L'/')
        string[i]=L'\\';
    }
    if(string[len-1]!=L'\\')
    {
      string[len]=L'\\';
      string[len+1]=0;
    }
  }
  return;
}

int parse_dir(char *root_oem,char *obj_oem,wchar_t *obj,int obj_type,unsigned long *param,wchar_t *host,char *host_oem)
{
  int result=-1;
  HANDLE testHandle;
  wchar_t *real_obj=obj,obj2[MAX_PATH],root[MAX_PATH],path[MAX_PATH];
  if((!real_obj)&&obj_oem)
  {
    MultiByteToWideChar(CP_OEMCP,0,obj_oem,-1,obj2,sizeof(obj2)/sizeof(obj2[0]));
    real_obj=obj2;
  }
  MultiByteToWideChar(CP_OEMCP,0,root_oem,-1,root,sizeof(root)/sizeof(root[0]));
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
        wchar_t *filename;
        DWORD full_res=GetFullPathNameW(path,MAX_PATH,host,&filename);
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
      //if(GetFileAttributesW(path)!=0xffffffff)
      if((testHandle=CreateFileW(path,0,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL))!=INVALID_HANDLE_VALUE)
      {
        CloseHandle(testHandle);
        goto parse_dir_real;
      }
      goto parse_dir_plugin;
  }
  if((result>-1)&&host_oem) WideCharToMultiByte(CP_OEMCP,0,host,-1,host_oem,MAX_PATH,NULL,NULL);
  return result;
}

static const char *GetRealName(const WIN32_FIND_DATAA *src)
{
  WIN32_FIND_DATAA find,find_ok; HANDLE hFind; BOOL Res;
  hFind=FindFirstFileA(src->cFileName,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    memcpy(&find_ok,&find,sizeof(find));
    Res=FindNextFileA(hFind,&find);
    FindClose(hFind);
    if((!Res)&&(!_stricmp(src->cAlternateFileName,find_ok.cAlternateFileName)))
    {
      return src->cFileName;
    }
  }
  if(src->cAlternateFileName[0])
  {
    hFind=FindFirstFileA(src->cAlternateFileName,&find);
    if(hFind!=INVALID_HANDLE_VALUE)
    {
      FindClose(hFind);
      return src->cAlternateFileName;
    }
  }
  return NULL;
}

bool GetWideName(char *root,const WIN32_FIND_DATAA *src,wchar_t *name)
{
  bool res=false;
  const char *real_shotname=GetRealName(src);
  if(real_shotname)
  {
    char path[MAX_PATH];
    wchar_t path_w[MAX_PATH];
    strcpy(path,root);
    FSF.AddEndSlash(path);
    strcat(path,real_shotname);
    MultiByteToWideChar(CP_OEMCP,0,path,-1,path_w,sizeof(path_w)/sizeof(path_w[0]));
    WIN32_FIND_DATAW find;
    HANDLE hFind=FindFirstFileW(path_w,&find);
    if(hFind!=INVALID_HANDLE_VALUE)
    {
      FindClose(hFind);
      wcscpy(name,find.cFileName);
      res=true;
    }
  }
  return res;
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
