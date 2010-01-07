/*
    um_arrays.cpp
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
#include <limits.h>
#include "far_helper.h"
#include <lm.h>
#include "umplugin.h"
#include "memory.h"

//еще один массив находится в функции GetCurrentPath в файле um_utils.cpp

const int up_dirs[]=
{
  //files
  levelRoot,
  levelRoot,
  levelFilePerm,
  levelFilePerm,
  levelRoot,
  levelFileAudit,
  levelFileAudit,
  //reg
  levelRegRoot,
  levelRegRoot,
  levelRegRights,
  levelRegRights,
  levelRegRoot,
  levelRegAudit,
  levelRegAudit,
  //share
  levelRoot,
  levelShared,
  levelSharedIn,
  //printer
  levelPrinterRoot,
  levelPrinterRoot,
  levelPrinterPerm,
  levelPrinterPerm,
  levelPrinterRoot,
  levelPrinterAudit,
  levelPrinterAudit,
  //printer share
  levelPrinterRoot,
  //users
  levelGroups,
  levelGroups,
  //rights
  levelRights,
  levelRights,
};

const int root_dirs[]=
{
  //files
  levelRoot,
  levelRoot,
  levelRoot,
  levelRoot,
  levelRoot,
  levelRoot,
  levelRoot,
  //regs
  levelRegRoot,
  levelRegRoot,
  levelRegRoot,
  levelRegRoot,
  levelRegRoot,
  levelRegRoot,
  levelRegRoot,
  //share
  levelRoot,
  levelRoot,
  levelRoot,
  //printer
  levelPrinterRoot,
  levelPrinterRoot,
  levelPrinterRoot,
  levelPrinterRoot,
  levelPrinterRoot,
  levelPrinterRoot,
  levelPrinterRoot,
  //printer share
  levelPrinterRoot,
  //users
  levelGroups,
  levelGroups,
  //rights
  levelRights,
  levelRights,
};

const bool nonfixed_dirs[]=
{
  //files
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  //reg
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  //share
  false,
  true,
  false,
  //printer
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  //printer share
  false,
  //users
  false,
  true,
  //rights
  false,
  true,
};

const bool has_nonfixed_dirs[]=
{
  //files
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  //reg
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  //share
  false,
  true,
  true,
  //printer
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  //printer share
  false,
  //users
  false,
  true,
  //rights
  false,
  true,
};

static const int root_dir[]=
{
  3,
  2,
  mDirRights,
  levelFilePerm,
  mDirAudit,
  levelFileAudit,
  mDirShared,
  levelShared,
};

static const int reg_root_dir[]=
{
  2,
  2,
  mDirRights,
  levelRegRights,
  mDirAudit,
  levelRegAudit,
};

static const int share_root_dir[]=
{
  1,
  1,
  mDirRights,
  levelSharedInAllowed,
};

static const int printer_root_dir[]=
{
  3,
  3,
  mDirRights,
  levelPrinterPerm,
  mDirAudit,
  levelPrinterAudit,
  mDirShared,
  levelPrinterShared,
};

const int *plain_dirs_dir[]=
{
  //files
  root_dir,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  //reg
  reg_root_dir,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  //share
  NULL,
  share_root_dir,
  NULL,
  //printer
  printer_root_dir,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  //printer share
  NULL,
  //users
  NULL,
  NULL,
  //rights
  NULL,
  NULL,
};

bool GetFileOwner(UserManager *panel,PSID *sid,wchar_t **owner,TCHAR **owner_oem)
{
  bool res=false;
  DWORD needed; PSECURITY_DESCRIPTOR SD=NULL;
  if(!GetFileSecurityW(panel->hostfile,OWNER_SECURITY_INFORMATION,NULL,0,&needed))
    if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
    {
      SD=(PSECURITY_DESCRIPTOR)malloc(needed);
      if(SD)
        if(GetFileSecurityW(panel->hostfile,OWNER_SECURITY_INFORMATION,SD,needed,&needed))
        {
          BOOL flag;
          if(GetSecurityDescriptorOwner(SD,sid,&flag))
          {
            res=true;
            GetUserNameEx(panel->computer_ptr,*sid,Opt.FullUserNames,owner,owner_oem);
          }
        }
        free(SD); SD=NULL; //FIXME: SD:sid used later
    }
  return res;
}

bool GetRegOwner(UserManager *panel,PSID *sid,wchar_t **owner,TCHAR **owner_oem)
{
  bool res=false;
  DWORD needed=0; PSECURITY_DESCRIPTOR SD=NULL;
  HKEY hKey; DWORD Disposition;
  if(RegOpenBackupKeyExW((HKEY)panel->param,panel->hostfile,KEY_READ,&hKey)==ERROR_SUCCESS)
  {
    if(RegGetKeySecurity(hKey,OWNER_SECURITY_INFORMATION,NULL,&needed)==ERROR_INSUFFICIENT_BUFFER)
    {
      SD=(PSECURITY_DESCRIPTOR)malloc(needed);
      if(SD)
      {
        if(RegGetKeySecurity(hKey,OWNER_SECURITY_INFORMATION,SD,&needed)==ERROR_SUCCESS)
        {
          BOOL flag;
          if(GetSecurityDescriptorOwner(SD,sid,&flag))
          {
            res=true;
            GetUserNameEx(panel->computer_ptr,*sid,Opt.FullUserNames,owner,owner_oem);
          }
        }
        free(SD); SD=NULL; //FIXME: SD:sid used later
      }
    }
    RegCloseKey(hKey);
  }
  return res;
}

bool GetPrinterOwner(UserManager *panel,PSID *sid,wchar_t **owner,TCHAR **owner_oem)
{
  bool res=false;
  HANDLE printer; PRINTER_DEFAULTSW defaults; PRINTER_INFO_3 *data=NULL;
  memset(&defaults,0,sizeof(defaults));
  defaults.DesiredAccess=READ_CONTROL;
  if(OpenPrinterW(panel->hostfile,&printer,&defaults))
  {
    DWORD Needed;
    if(!GetPrinterW(printer,3,NULL,0,&Needed))
    {
      if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
      {
        data=(PRINTER_INFO_3 *)malloc(Needed);
        if(data)
        {
          if(GetPrinterW(printer,3,(PBYTE)data,Needed,&Needed))
          {
            BOOL flag;
            if(GetSecurityDescriptorOwner(data->pSecurityDescriptor,sid,&flag))
            {
              res=true;
              GetUserNameEx(panel->computer_ptr,*sid,Opt.FullUserNames,owner,owner_oem);
            }
          }
          free(data); data=NULL; //FIXME: SD:sid used later
        }
      }
    }
    ClosePrinter(printer);
  }
  return res;
}

const GetOwner plain_dirs_owners[]=
{
  //files
  GetFileOwner,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  //reg
  GetRegOwner,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  //share
  NULL,
  NULL,
  NULL,
  //printer
  GetPrinterOwner,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  //printer share
  NULL,
  //users
  NULL,
  NULL,
  //rights
  NULL,
  NULL,
};

const unsigned char perm_dirs_dir[]=
{
  //files
  PERM_NO,
  PERM_FF,
  PERM_YES,
  PERM_YES,
  PERM_FF,
  PERM_YES,
  PERM_YES,
  //reg
  PERM_NO,
  PERM_KEY,
  PERM_YES,
  PERM_NO,
  PERM_KEY,
  PERM_YES,
  PERM_NO,
  //share
  PERM_NO,
  PERM_NO,
  PERM_YES,
  //printer
  PERM_NO,
  PERM_PRINT,
  PERM_YES,
  PERM_YES,
  PERM_PRINT,
  PERM_YES,
  PERM_YES,
  //printer share
  PERM_NO,
  //users
  PERM_NO,
  PERM_NO,
  //rights
  PERM_NO,
  PERM_NO,
};

PSECURITY_DESCRIPTOR GetFileSD(UserManager *panel,SECURITY_INFORMATION si)
{
  DWORD needed; PSECURITY_DESCRIPTOR SD=NULL;
  if(!GetFileSecurityW(panel->hostfile,si,NULL,0,&needed))
    if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
    {
      SD=(PSECURITY_DESCRIPTOR)malloc(needed);
      if(SD)
        if(!GetFileSecurityW(panel->hostfile,si,SD,needed,&needed))
        {
          free(SD); SD=NULL;
        }
    }
  return SD;
}

PSECURITY_DESCRIPTOR GetRegSD(UserManager *panel,SECURITY_INFORMATION si)
{
  DWORD needed=0; PSECURITY_DESCRIPTOR SD=NULL;
  HKEY hKey;
  if(RegOpenBackupKeyExW((HKEY)panel->param,panel->hostfile,reg_security_descriptor_access_read[si],&hKey)==ERROR_SUCCESS)
  {
    if(RegGetKeySecurity(hKey,si,NULL,&needed)==ERROR_INSUFFICIENT_BUFFER)
    {
      SD=(PSECURITY_DESCRIPTOR)malloc(needed);
      if(SD)
        if(RegGetKeySecurity(hKey,si,SD,&needed)!=ERROR_SUCCESS)
        {
          free(SD); SD=NULL;
        }
    }
    RegCloseKey(hKey);
  }
  return SD;
}

PSECURITY_DESCRIPTOR GetShareSD(UserManager *panel,SECURITY_INFORMATION si)
{
  PSECURITY_DESCRIPTOR SD=NULL;
  SHARE_INFO_502 *info;
  if(NetShareGetInfo(panel->computer_ptr,panel->nonfixed,502,(LPBYTE *)&info)==NERR_Success)
  {
    if(info->shi502_security_descriptor)
    {
      unsigned long len=GetSecurityDescriptorLength(info->shi502_security_descriptor);
      if(len)
      {
        SD=(PSECURITY_DESCRIPTOR)malloc(len);
        if(SD)
          memcpy(SD,info->shi502_security_descriptor,len);
      }
    }
    else
      SD=CreateDefaultSD();
    NetApiBufferFree(info);
  }
  return SD;
}

PSECURITY_DESCRIPTOR GetPrinterSD(UserManager *panel,SECURITY_INFORMATION si)
{
  PSECURITY_DESCRIPTOR SD=NULL;
  HANDLE printer; PRINTER_DEFAULTSW defaults; PRINTER_INFO_3 *data=NULL;
  memset(&defaults,0,sizeof(defaults));
  defaults.DesiredAccess=printer_security_descriptor_access_read[si];
  if(OpenPrinterW(panel->hostfile,&printer,&defaults))
  {
    DWORD Needed;
    if(!GetPrinterW(printer,3,NULL,0,&Needed))
    {
      if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
      {
        data=(PRINTER_INFO_3 *)malloc(Needed);
        if(data)
        {
          if(GetPrinterW(printer,3,(PBYTE)data,Needed,&Needed))
          {
            if(data->pSecurityDescriptor)
            {
              unsigned long len=GetSecurityDescriptorLength(data->pSecurityDescriptor);
              if(len)
              {
                SD=(PSECURITY_DESCRIPTOR)malloc(len);
                if(SD)
                  memcpy(SD,data->pSecurityDescriptor,len);
              }
            }
          }
          else
          {
            free(data); data=NULL;
          }
        }
      }
    }
    ClosePrinter(printer);
  }
  return SD;
}

bool SetFileSD(UserManager *panel,SECURITY_INFORMATION si,PSECURITY_DESCRIPTOR SD)
{
  if(SetFileSecurityW(panel->hostfile,si,SD)) return true;
  return false;
}

bool SetRegSD(UserManager *panel,SECURITY_INFORMATION si,PSECURITY_DESCRIPTOR SD)
{
  bool res=false;
  HKEY hKey;
  if(RegOpenBackupKeyExW((HKEY)panel->param,panel->hostfile,reg_security_descriptor_access_write[si],&hKey)==ERROR_SUCCESS)
  {
    if(RegSetKeySecurity(hKey,si,SD)==ERROR_SUCCESS)
      res=true;
    RegCloseKey(hKey);
  }
  return res;
}

bool SetShareSD(UserManager *panel,SECURITY_INFORMATION si,PSECURITY_DESCRIPTOR SD)
{
  bool res=false;
  SHARE_INFO_502 *info;
  if(NetShareGetInfo(panel->computer_ptr,panel->nonfixed,502,(LPBYTE *)&info)==NERR_Success)
  {
    info->shi502_security_descriptor=SD;
    if(NetShareSetInfo(panel->computer_ptr,panel->nonfixed,502,(LPBYTE)info,NULL)==NERR_Success)
      res=true;
    NetApiBufferFree(info);
  }
  return res;
}

bool SetPrinterSD(UserManager *panel,SECURITY_INFORMATION si,PSECURITY_DESCRIPTOR SD)
{
  bool res=false;
  PRINTER_INFO_3 data={SD}; HANDLE printer; PRINTER_DEFAULTSW defaults;
  memset(&defaults,0,sizeof(defaults));
  defaults.DesiredAccess=printer_security_descriptor_access_write[si];
  if(OpenPrinterW(panel->hostfile,&printer,&defaults))
  {
    if(SetPrinterW(printer,3,(PBYTE)&data,0))
      res=true;
    ClosePrinter(printer);
  }
  return res;
}

const SECURITY_INFORMATION security_information[]=
{
  //files
  0,
  DACL_SECURITY_INFORMATION,
  DACL_SECURITY_INFORMATION,
  DACL_SECURITY_INFORMATION,
  SACL_SECURITY_INFORMATION,
  SACL_SECURITY_INFORMATION,
  SACL_SECURITY_INFORMATION,
  //reg
  0,
  DACL_SECURITY_INFORMATION,
  DACL_SECURITY_INFORMATION,
  DACL_SECURITY_INFORMATION,
  SACL_SECURITY_INFORMATION,
  SACL_SECURITY_INFORMATION,
  SACL_SECURITY_INFORMATION,
  //share
  0,
  0,
  DACL_SECURITY_INFORMATION,
  //printer
  0,
  DACL_SECURITY_INFORMATION,
  DACL_SECURITY_INFORMATION,
  DACL_SECURITY_INFORMATION,
  SACL_SECURITY_INFORMATION,
  SACL_SECURITY_INFORMATION,
  SACL_SECURITY_INFORMATION,
  //printer share
  0,
  //users
  0,
  0,
  //rights
  0,
  0,
};

const GetSD get_security_descriptor[]=
{
  //files
  NULL,
  GetFileSD,
  GetFileSD,
  GetFileSD,
  GetFileSD,
  GetFileSD,
  GetFileSD,
  //reg
  NULL,
  GetRegSD,
  GetRegSD,
  GetRegSD,
  GetRegSD,
  GetRegSD,
  GetRegSD,
  //share
  NULL,
  NULL,
  GetShareSD,
  //printer
  NULL,
  GetPrinterSD,
  GetPrinterSD,
  GetPrinterSD,
  GetPrinterSD,
  GetPrinterSD,
  GetPrinterSD,
  //printer share
  NULL,
  //users
  NULL,
  NULL,
  //groups
  NULL,
  NULL,
};

const SetSD set_security_descriptor[]=
{
  //files
  SetFileSD,
  SetFileSD,
  SetFileSD,
  SetFileSD,
  SetFileSD,
  SetFileSD,
  SetFileSD,
  //reg
  SetRegSD,
  SetRegSD,
  SetRegSD,
  SetRegSD,
  SetRegSD,
  SetRegSD,
  SetRegSD,
  //share
  NULL,
  NULL,
  SetShareSD,
  //printer
  SetPrinterSD,
  SetPrinterSD,
  SetPrinterSD,
  SetPrinterSD,
  SetPrinterSD,
  SetPrinterSD,
  SetPrinterSD,
  //printer share
  NULL,
  //users
  NULL,
  NULL,
  //groups
  NULL,
  NULL,
};

const GetSDAcl get_security_descriptor_acl[]=
{
  NULL,
  NULL,
  NULL,
  NULL,
  GetSecurityDescriptorDacl,
  NULL,
  NULL,
  NULL,
  GetSecurityDescriptorSacl,
};

const SetSDAcl set_security_descriptor_acl[]=
{
  NULL,
  NULL,
  NULL,
  NULL,
  SetSecurityDescriptorDacl,
  NULL,
  NULL,
  NULL,
  SetSecurityDescriptorSacl,
};

const unsigned long reg_security_descriptor_access_read[]=
{
  0,
  KEY_READ,
  0,
  0,
  KEY_READ,
  0,
  0,
  0,
  ACCESS_SYSTEM_SECURITY,
};

const unsigned long reg_security_descriptor_access_write[]=
{
  0,
  WRITE_OWNER,
  0,
  0,
  WRITE_DAC,
  0,
  0,
  0,
  ACCESS_SYSTEM_SECURITY,
};

const unsigned long printer_security_descriptor_access_read[]=
{
  0,
  READ_CONTROL,
  0,
  0,
  READ_CONTROL,
  0,
  0,
  0,
  ACCESS_SYSTEM_SECURITY,
};

const unsigned long printer_security_descriptor_access_write[]=
{
  0,
  WRITE_OWNER,
  0,
  0,
  WRITE_DAC|WRITE_OWNER, //FIXME: really WRITE_OWNER not needed
  0,
  0,
  0,
  ACCESS_SYSTEM_SECURITY|WRITE_OWNER, //FIXME: really WRITE_OWNER not needed
};

typedef DWORD (*convert_mask)(DWORD mask);

bool CheckTypePerm(unsigned char Param,unsigned char Type,unsigned char *Flags,unsigned long *Mask)
{
  convert_mask convert[]={generic_mask_to_file_mask,generic_mask_to_reg_mask,generic_mask_to_printer_mask};
  if((Type==ACCESS_ALLOWED_ACE_TYPE||Type==ACCESS_DENIED_ACE_TYPE)&&(!(INHERIT_ONLY_ACE&(*Flags))))
  {
    *Mask=convert[Param](*Mask);
    *Flags=0;
    return true;
  }
  return false;
}

bool CheckTypePermInherited(unsigned char Param,unsigned char Type,unsigned char *Flags,unsigned long *Mask)
{
  unsigned char flags_test[]={CONTAINER_INHERIT_ACE,OBJECT_INHERIT_ACE,CONTAINER_INHERIT_ACE,CONTAINER_INHERIT_ACE,OBJECT_INHERIT_ACE};
  convert_mask convert[]={generic_mask_to_file_mask,generic_mask_to_file_mask,generic_mask_to_reg_mask,generic_mask_to_job_mask,generic_mask_to_job_mask};
  if((Type==ACCESS_ALLOWED_ACE_TYPE||Type==ACCESS_DENIED_ACE_TYPE)&&(flags_test[Param]&(*Flags)))
  {
    *Mask=convert[Param](*Mask);
    *Flags=flags_test[Param]|INHERIT_ONLY_ACE;
    return true;
  }
  return false;
}

bool CheckTypeAudit(unsigned char Param,unsigned char Type,unsigned char *Flags,unsigned long *Mask)
{
  convert_mask convert[]={generic_mask_to_file_mask,generic_mask_to_reg_mask,generic_mask_to_printer_mask};
  if(Type==SYSTEM_AUDIT_ACE_TYPE&&(!(INHERIT_ONLY_ACE&(*Flags))))
  {
    *Mask=convert[Param](*Mask);
    *Flags=(*Flags)&(SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG);
    return true;
  }
  return false;
}

bool CheckTypeInheritedAudit(unsigned char Param,unsigned char Type,unsigned char *Flags,unsigned long *Mask)
{
  unsigned char flags_test[]={CONTAINER_INHERIT_ACE,OBJECT_INHERIT_ACE,CONTAINER_INHERIT_ACE,CONTAINER_INHERIT_ACE,OBJECT_INHERIT_ACE};
  convert_mask convert[]={generic_mask_to_file_mask,generic_mask_to_file_mask,generic_mask_to_reg_mask,generic_mask_to_job_mask,generic_mask_to_job_mask};
  if((Type==SYSTEM_AUDIT_ACE_TYPE)&&(flags_test[Param]==(flags_test[Param]&(*Flags))))
  {
    *Mask=convert[Param](*Mask);
    *Flags=((*Flags)&(SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG))|flags_test[Param]|INHERIT_ONLY_ACE;
    return true;
  }
  return false;
}

const unsigned char ace_types[]=
{
  0, // levelRoot,
  1, // levelFilePerm,
  1,
  1,
  2, // levelFileAudit,
  2,
  2,
  0, // levelRegRoot,
  1, // levelRegRights,
  1,
  1,
  2, // levelRegAudit,
  2,
  2,
  0, // levelShared,
  0, // levelSharedIn,
  1, // levelSharedInAllowed,
  0, // levelPrinterRoot
  1, // levelPrinterPerm
  1, // levelPrinterPermContainer
  1, // levelPrinterPermJob
  2, // levelPrinterAudit
  2, // levelPrinterAuditContainer
  2, // levelPrinterAuditJob
  0, // levelPrinterShared,
  0, // levelGroups
  0, // levelUsers
  0, // levelRights
  0, // levelRightUsers
};

const unsigned char check_ace_type_param[]=
{
  0, // levelRoot,
  0, // levelFilePerm,
  0,
  1,
  0, // levelFileAudit,
  0,
  1,
  0, // levelRegRoot,
  1, // levelRegRights,
  2,
  2,
  1, // levelRegAudit,
  2,
  2,
  0, // levelShared,
  0, // levelSharedIn,
  0, // levelSharedInAllowed,
  0, // levelPrinterRoot
  2, // levelPrinterPerm
  3, // levelPrinterPermContainer
  4, // levelPrinterPermJob
  2, // levelPrinterAudit
  3, // levelPrinterAuditContainer
  4, // levelPrinterAuditJob
  0, // levelPrinterShared,
  0, // levelGroups
  0, // levelUsers
  0, // levelRights
  0, // levelRightUsers
};

const CheckType check_ace_type[]=
{
  //files
  NULL,
  CheckTypePerm,
  CheckTypePermInherited,
  CheckTypePermInherited,
  CheckTypeAudit,
  CheckTypeInheritedAudit,
  CheckTypeInheritedAudit,
  //reg
  NULL,
  CheckTypePerm,
  CheckTypePermInherited,
  NULL,
  CheckTypeAudit,
  CheckTypeInheritedAudit,
  NULL,
  //share
  NULL,
  NULL,
  CheckTypePerm,
  //printer
  NULL,
  CheckTypePerm,
  CheckTypePermInherited,
  CheckTypePermInherited,
  CheckTypeAudit,
  CheckTypeInheritedAudit,
  CheckTypeInheritedAudit,
  //printer share
  NULL,
  //users
  NULL,
  NULL,
  //rights
  NULL,
  NULL,
};

const int relative_types[][5]= //FIXME
{
  //files
  {-1,-1,-1,-1,-1},
  {levelFilePermNewFolder,levelFilePermNewFile,-1,-1,-1},
  {levelFilePerm,levelFilePermNewFile,-1,-1,-1},
  {levelFilePerm,levelFilePermNewFolder,-1,-1,-1},
  {levelFileAuditFolder,levelFileAuditFile,-1,-1,-1},
  {levelFileAudit,levelFileAuditFile,-1,-1,-1},
  {levelFileAudit,levelFileAuditFolder,-1,-1,-1},
  //reg
  {-1,-1,-1,-1,-1},
  {levelRegRightsNewKey,-1,-1,-1,-1},
  {levelRegRights,-1,-1,-1,-1},
  {-1,-1,-1,-1,-1},
  {levelRegAuditKey,-1,-1,-1,-1},
  {levelRegAudit,-1,-1,-1,-1},
  {-1,-1,-1,-1,-1},
  //share
  {-1,-1,-1,-1,-1},
  {-1,-1,-1,-1,-1},
  {-1,-1,-1,-1,-1},
  //printer
  {-1,-1,-1,-1,-1},
  {levelPrinterPermContainer,levelPrinterPermJob,-1,-1,-1},
  {levelPrinterPerm,levelPrinterPermJob,-1,-1,-1},
  {levelPrinterPerm,levelPrinterPermContainer,-1,-1,-1},
  {levelPrinterAuditContainer,levelPrinterAuditJob,-1,-1,-1},
  {levelPrinterAudit,levelPrinterAuditJob,-1,-1,-1},
  {levelPrinterAudit,levelPrinterAuditContainer,-1,-1,-1},
  //printer share
  {-1,-1,-1,-1,-1},
  //users
  {-1,-1,-1,-1,-1},
  {-1,-1,-1,-1,-1},
  //rights
  {-1,-1,-1,-1,-1},
  {-1,-1,-1,-1,-1},
};

const bool has_hostfile[]=
{
  //files
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  //reg
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  //share
  true,
  true,
  true,
  //printer
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  //printer share
  false,
  //users
  false,
  false,
  //rights
  false,
  false,
};

const unsigned char title_type[]=
{
  //files
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  //reg
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  //share
  0,
  0,
  0,
  //printer
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  //printer share
  0,
  //users
  1,
  1,
  //rights
  1,
  1,
};

const int title_type_string[]=
{
  //files
  mTitleACL,
  mTitleACL,
  mTitleACL,
  mTitleACL,
  mTitleACL,
  mTitleACL,
  mTitleACL,
  //reg
  mTitleACL,
  mTitleACL,
  mTitleACL,
  mTitleACL,
  mTitleACL,
  mTitleACL,
  mTitleACL,
  //share
  mTitleACL,
  mTitleACL,
  mTitleACL,
  //printer
  mTitleACL,
  mTitleACL,
  mTitleACL,
  mTitleACL,
  mTitleACL,
  mTitleACL,
  mTitleACL,
  //printer share
  mTitleACL,
  //users
  mTitleUsers,
  mTitleUsers,
  //rights
  mTitleRights,
  mTitleRights,
};

static bool EditGroupUser(UserManager *panel)
{
  return ManageUser(panel,true);
}

static bool CreateUser(UserManager *panel)
{
  return ManageUser(panel,false);
}

const PressButton press_f4[]=
{
  //files
  NULL,
  EditCommonAccess,
  EditCommonAccess,
  EditCommonAccess,
  EditCommonAccess,
  EditCommonAccess,
  EditCommonAccess,
  //reg
  NULL,
  EditCommonAccess,
  EditCommonAccess,
  NULL,
  EditCommonAccess,
  EditCommonAccess,
  NULL,
  //share
  EditShareProperties,
  NULL,
  EditCommonAccess,
  //printer
  NULL,
  EditCommonAccess,
  EditCommonAccess,
  EditCommonAccess,
  EditCommonAccess,
  EditCommonAccess,
  EditCommonAccess,
  //printer share
  NULL,
  //users
  EditGroupUser,
  EditGroupUser,
  //groups
  NULL,
  NULL,
};

const PressButton press_shift_f4[]=
{
  NULL,//levelRoot
  NULL,//levelFilePerm
  NULL,//levelFilePermNewFolder
  NULL,//levelFilePermNewFile
  NULL,//levelFileAudit
  NULL,//levelFileAuditFolder
  NULL,//levelFileAuditFile
  NULL,//levelRegRoot
  NULL,//levelRegRights
  NULL,//levelRegRightsNewKey
  NULL,//levelRegRightsNewPad
  NULL,//levelRegAudit
  NULL,//levelRegAuditKey
  NULL,//levelRegAuditPad
  NULL,//levelShared
  NULL,//levelSharedIn
  NULL,//levelSharedInAllowed
  NULL,//levelPrinterRoot
  NULL,//levelPrinterPerm
  NULL,//levelPrinterPermContainer
  NULL,//levelPrinterPermJob
  NULL,//levelPrinterAudit
  NULL,//levelPrinterAuditContainer
  NULL,//levelPrinterAuditJob
  NULL,//levelPrinterShared
  CreateUser,//levelGroups
  CreateUser,//levelUsers
  NULL,//levelRights
  NULL,//levelRightUsers
};

const PressButton press_alt_f4[]=
{
  //files
  NULL,
  EditFileAdvancedAccess,
  EditFileAdvancedAccess,
  EditFileAdvancedAccess,
  EditFileAdvancedAccess,
  EditFileAdvancedAccess,
  EditFileAdvancedAccess,
  //reg
  NULL,
  EditRegAdvancedAccess,
  EditRegAdvancedAccess,
  NULL,
  EditRegAdvancedAccess,
  EditRegAdvancedAccess,
  NULL,
  //share
  NULL,
  NULL,
  EditFileAdvancedAccess,
  //printer
  NULL,
  EditPrinterAdvancedAccess,
  EditJobAdvancedAccess,
  EditJobAdvancedAccess,
  EditPrinterAdvancedAccess,
  EditJobAdvancedAccess,
  EditJobAdvancedAccess,
  //printer share
  NULL,
  //users
  NULL,
  NULL,
  //rights
  NULL,
  NULL,
};

const bool press_f5_from[]=
{
  //files
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  //reg
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  //share
  false,
  false,
  false,
  //printer
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  //printer share
  false,
  //users
  true,
  true,
  //rights
  false,
  false,
};

const PressButton3 press_f5[]=
{
  //files
  AddOwner,
  AddACE,
  AddACE,
  AddACE,
  AddACE,
  AddACE,
  AddACE,
  //reg
  AddOwner,
  AddACE,
  AddACE,
  NULL,
  AddACE,
  AddACE,
  NULL,
  //share
  NULL,
  NULL,
  AddACE,
  //printer
  AddOwner,
  AddACE,
  AddACE,
  AddACE,
  AddACE,
  AddACE,
  AddACE,
  //printer share
  NULL,
  //users
  NULL,
  AddUserToGroup,
  //rights
  NULL,
  AddUserToRight,
};

const PressButton2 press_f6[]=
{
  NULL,//levelRoot
  NULL,//levelFilePerm
  NULL,//levelFilePermNewFolder
  NULL,//levelFilePermNewFile
  NULL,//levelFileAudit
  NULL,//levelFileAuditFolder
  NULL,//levelFileAuditFile
  NULL,//levelRegRoot
  NULL,//levelRegRights
  NULL,//levelRegRightsNewKey
  NULL,//levelRegRightsNewPad
  NULL,//levelRegAudit
  NULL,//levelRegAuditKey
  NULL,//levelRegAuditPad
  NULL,//levelShared
  NULL,//levelSharedIn
  NULL,//levelSharedInAllowed
  NULL,//levelPrinterRoot
  NULL,//levelPrinterPerm
  NULL,//levelPrinterPermContainer
  NULL,//levelPrinterPermJob
  NULL,//levelPrinterAudit
  NULL,//levelPrinterAuditContainer
  NULL,//levelPrinterAuditJob
  NULL,//levelPrinterShared
  GetComputer,//levelGroups
  RemoveUser,//levelUsers
  GetComputer,//levelRights
  GetComputer,//levelRightUsers
};

static bool AddGroup(UserManager *panel)
{
  return ManageGroup(panel,false,L"");
}

const PressButton press_f7[]=
{
  //files
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  //reg
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  //share
  AddShare,
  NULL,
  NULL,
  //printer
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  //printer share
  AddShare,
  //users
  AddGroup,
  NULL,
  //rights
  NULL,
  NULL,
};

const PressButton2 press_f8[]=
{
  //files
  TakeOwnership,
  DeleteACE,
  DeleteACE,
  DeleteACE,
  DeleteACE,
  DeleteACE,
  DeleteACE,
  //reg
  TakeOwnership,
  DeleteACE,
  DeleteACE,
  NULL,
  DeleteACE,
  DeleteACE,
  NULL,
  //share
  DeleteShare,
  NULL,
  DeleteACE,
  //printer
  TakeOwnership,
  DeleteACE,
  DeleteACE,
  DeleteACE,
  DeleteACE,
  DeleteACE,
  DeleteACE,
  //printer share
  DeleteShare,
  //users
  DeleteGroup,
  DeleteUser,
  //rights
  NULL,
  DeleteRightUsers,
};

const unsigned char default_type[]= //FIXME
{
  //files
  0,
  ACCESS_ALLOWED_ACE_TYPE,
  ACCESS_ALLOWED_ACE_TYPE,
  ACCESS_ALLOWED_ACE_TYPE,
  SYSTEM_AUDIT_ACE_TYPE,
  SYSTEM_AUDIT_ACE_TYPE,
  SYSTEM_AUDIT_ACE_TYPE,
  //reg
  0,
  ACCESS_ALLOWED_ACE_TYPE,
  ACCESS_ALLOWED_ACE_TYPE,
  0,
  SYSTEM_AUDIT_ACE_TYPE,
  SYSTEM_AUDIT_ACE_TYPE,
  0,
  //share
  0,
  0,
  ACCESS_ALLOWED_ACE_TYPE,
  //printer
  0,
  ACCESS_ALLOWED_ACE_TYPE,
  ACCESS_ALLOWED_ACE_TYPE,
  ACCESS_ALLOWED_ACE_TYPE,
  SYSTEM_AUDIT_ACE_TYPE,
  SYSTEM_AUDIT_ACE_TYPE,
  SYSTEM_AUDIT_ACE_TYPE,
  //printer share
  0,
  //users
  0,
  0,
  //rights
  0,
  0,
};

const unsigned char default_flags[]=
{
  //files
  0,
  0,
  CONTAINER_INHERIT_ACE|INHERIT_ONLY_ACE,
  OBJECT_INHERIT_ACE|INHERIT_ONLY_ACE,
  SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG,
  CONTAINER_INHERIT_ACE|INHERIT_ONLY_ACE|SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG,
  OBJECT_INHERIT_ACE|INHERIT_ONLY_ACE|SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG,
  //reg
  0,
  0,
  CONTAINER_INHERIT_ACE|INHERIT_ONLY_ACE,
  0,
  SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG,
  CONTAINER_INHERIT_ACE|INHERIT_ONLY_ACE|SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG,
  0,
  //share
  0,
  0,
  0,
  //printer
  0,
  0,
  CONTAINER_INHERIT_ACE|INHERIT_ONLY_ACE,
  OBJECT_INHERIT_ACE|INHERIT_ONLY_ACE,
  SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG,
  CONTAINER_INHERIT_ACE|INHERIT_ONLY_ACE|SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG,
  OBJECT_INHERIT_ACE|INHERIT_ONLY_ACE|SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG,
  //printer share
  0,
  //users
  0,
  0,
  //rights
  0,
  0,
};

const unsigned long default_mask[]=
{
  //files
  0,
  FILE_ALL_ACCESS,
  FILE_ALL_ACCESS,
  FILE_ALL_ACCESS,
  FILE_ALL_ACCESS,
  FILE_ALL_ACCESS,
  FILE_ALL_ACCESS,
  //reg
  0,
  KEY_ALL_ACCESS,
  KEY_ALL_ACCESS,
  0,
  KEY_ALL_ACCESS,
  KEY_ALL_ACCESS,
  0,
  //share
  0,
  0,
  FILE_ALL_ACCESS,
  //printer
  0,
  PRINTER_ALL_ACCESS,
  JOB_ALL_ACCESS,
  JOB_ALL_ACCESS,
  PRINTER_ALL_ACCESS,
  JOB_ALL_ACCESS,
  JOB_ALL_ACCESS,
  //printer share
  0,
  //users
  0,
  0,
  //rights
  0,
  0,
};

const int default_acl[]=
{
  //files
  0,
  1,
  1,
  1,
  2,
  2,
  2,
  //reg
  0,
  1,
  1,
  0,
  2,
  2,
  0,
  //share
  0,
  0,
  1,
  //printer
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  //printer share
  0,
  //users
  0,
  0,
  //rights
  0,
  0,
};

static const unsigned int file_common_rights[]=
{
  FILE_GENERIC_READ,FILE_GENERIC_WRITE,FILE_GENERIC_EXECUTE,DELETE,WRITE_DAC,WRITE_OWNER
};

static const unsigned int reg_common_rights[]=
{
  KEY_READ,KEY_WRITE,KEY_EXECUTE,DELETE,WRITE_DAC,WRITE_OWNER
};

static const unsigned int printer_common_rights[]=
{
  PRINTER_READ,PRINTER_WRITE,PRINTER_EXECUTE,DELETE,WRITE_DAC,WRITE_OWNER
};

static const unsigned int job_common_rights[]=
{
  JOB_READ,JOB_WRITE,JOB_EXECUTE,DELETE,WRITE_DAC,WRITE_OWNER
};

const unsigned int *common_rights[]=
{
  //files
  NULL,
  file_common_rights,
  file_common_rights,
  file_common_rights,
  file_common_rights,
  file_common_rights,
  file_common_rights,
  //reg
  NULL,
  reg_common_rights,
  reg_common_rights,
  NULL,
  reg_common_rights,
  reg_common_rights,
  NULL,
  //share
  NULL,
  NULL,
  file_common_rights,
  //printer
  NULL,
  printer_common_rights,
  job_common_rights,
  job_common_rights,
  printer_common_rights,
  job_common_rights,
  job_common_rights,
  //printer share
  NULL,
  //users
  NULL,
  NULL,
  //rights
  NULL,
  NULL,
};

const unsigned int common_full_access[]=
{
  //files
  0,
  FILE_ALL_ACCESS,
  FILE_ALL_ACCESS,
  FILE_ALL_ACCESS,
  FILE_ALL_ACCESS,
  FILE_ALL_ACCESS,
  FILE_ALL_ACCESS,
  //reg
  0,
  KEY_ALL_ACCESS,
  KEY_ALL_ACCESS,
  0,
  KEY_ALL_ACCESS,
  KEY_ALL_ACCESS,
  0,
  //share
  0,
  0,
  FILE_ALL_ACCESS,
  //printer
  0,
  PRINTER_ALL_ACCESS,
  JOB_ALL_ACCESS,
  JOB_ALL_ACCESS,
  PRINTER_ALL_ACCESS,
  JOB_ALL_ACCESS,
  JOB_ALL_ACCESS,
  //printer share
  0,
  //users
  0,
  0,
  //rights
  0,
  0,
};

const int panel_modes[]=
{
  //files
  mPlainPanelType0,
  mPermPanelType0,
  mPermPanelType0,
  mPermPanelType0,
  mPermPanelType0,
  mPermPanelType0,
  mPermPanelType0,
  //reg
  mPlainPanelType0,
  mPermPanelType0,
  mPermPanelType0,
  mPlainPanelType0,
  mPermPanelType0,
  mPermPanelType0,
  mPlainPanelType0,
  //share
  mSharePanelType0,
  mPlainPanelType0,
  mPermPanelType0,
  //printer
  mPlainPanelType0,
  mPermPanelType0,
  mPermPanelType0,
  mPermPanelType0,
  mPermPanelType0,
  mPermPanelType0,
  mPermPanelType0,
  //printer share
  mPlainPanelType0,
  //users
  mUserPanelType0,
  mUserPanelType0,
  //rights
  mPlainPanelType0,
  mUserPanelType0,
};

TCHAR *get_plain_panel_title(const TCHAR *str)
{
  TCHAR *Result=NULL;
  return Result;
}

TCHAR *get_access_panel_title(const TCHAR *str)
{
  TCHAR *Result=NULL;
  if((!_tcsncmp(str,_T("C0"),2))&&((!str[2])||(str[2]==',')))
  {
    Result=(TCHAR*)GetMsg(mTitleAccess);
  }
  else if((!_tcsncmp(str,_T("C1"),2))&&((!str[2])||(str[2]==',')))
  {
    Result=(TCHAR*)GetMsg(mTitleSID);
  }
  return Result;
}

TCHAR *get_audit_panel_title(const TCHAR *str)
{
  TCHAR *Result=NULL;
  if((!_tcsncmp(str,_T("C0"),2))&&((!str[2])||(str[2]==',')))
  {
    Result=(TCHAR*)GetMsg(mTitleAudit);
  }
  else if((!_tcsncmp(str,_T("C1"),2))&&((!str[2])||(str[2]==',')))
  {
    Result=(TCHAR*)GetMsg(mTitleSID);
  }
  return Result;
}

TCHAR *get_share_panel_title(const TCHAR *str)
{
  TCHAR *Result=NULL;
  if((!_tcsncmp(str,_T("C0"),2))&&((!str[2])||(str[2]==',')))
  {
    Result=(TCHAR*)GetMsg(mTitleMaximum);
  }
  else if((!_tcsncmp(str,_T("C1"),2))&&((!str[2])||(str[2]==',')))
  {
    Result=(TCHAR*)GetMsg(mTitleCurrent);
  }
  else if((!_tcsncmp(str,_T("C2"),2))&&((!str[2])||(str[2]==',')))
  {
    Result=(TCHAR*)GetMsg(mTitleLocalPath);
  }
  return Result;
}

TCHAR *get_user_panel_title(const TCHAR *str)
{
  TCHAR *Result=NULL;
  if((!_tcsncmp(str,_T("C1"),2))&&((!str[2])||(str[2]==',')))
  {
    Result=(TCHAR*)GetMsg(mTitleSID);
  }
  return Result;
}

const ParseColumns get_panel_titles[]=
{
  //files
  get_plain_panel_title,
  get_access_panel_title,
  get_access_panel_title,
  get_access_panel_title,
  get_audit_panel_title,
  get_audit_panel_title,
  get_audit_panel_title,
  //reg
  get_plain_panel_title,
  get_access_panel_title,
  get_access_panel_title,
  get_plain_panel_title,
  get_audit_panel_title,
  get_audit_panel_title,
  get_plain_panel_title,
  //share
  get_share_panel_title,
  get_plain_panel_title,
  get_access_panel_title,
  //printer
  get_plain_panel_title,
  get_access_panel_title,
  get_access_panel_title,
  get_access_panel_title,
  get_audit_panel_title,
  get_audit_panel_title,
  get_audit_panel_title,
  //printer share
  get_plain_panel_title,
  //users
  get_user_panel_title,
  get_user_panel_title,
  //rights
  get_plain_panel_title,
  get_user_panel_title,
};

const int label_f4[]=
{
  //files
  mKeyEmpty,
  mKeyPerm,
  mKeyPerm,
  mKeyPerm,
  mKeyAudit,
  mKeyAudit,
  mKeyAudit,
  //reg
  mKeyEmpty,
  mKeyPerm,
  mKeyPerm,
  mKeyEmpty,
  mKeyAudit,
  mKeyAudit,
  mKeyEmpty,
  //share
  mKeyProperties,
  mKeyEmpty,
  mKeyPerm,
  //printer
  mKeyEmpty,
  mKeyPerm,
  mKeyPerm,
  mKeyPerm,
  mKeyAudit,
  mKeyAudit,
  mKeyAudit,
  //printer share
  mKeyEmpty,
  //users
  mKeyProperties,
  mKeyProperties,
  //rights
  mKeyEmpty,
  mKeyEmpty,
};

const int label_alt_f4[]=
{
  //files
  mKeyEmpty,
  mKeyAdvPerm,
  mKeyAdvPerm,
  mKeyAdvPerm,
  mKeyAdvAudit,
  mKeyAdvAudit,
  mKeyAdvAudit,
  //reg
  mKeyEmpty,
  mKeyAdvPerm,
  mKeyAdvPerm,
  mKeyEmpty,
  mKeyAdvAudit,
  mKeyAdvAudit,
  mKeyEmpty,
  //share
  mKeyEmpty,
  mKeyEmpty,
  mKeyAdvPerm,
  //printer
  mKeyEmpty,
  mKeyAdvPerm,
  mKeyAdvPerm,
  mKeyAdvPerm,
  mKeyAdvAudit,
  mKeyAdvAudit,
  mKeyAdvAudit,
  //printer share
  mKeyEmpty,
  //users
  mKeyEmpty,
  mKeyEmpty,
  //rights
  mKeyEmpty,
  mKeyEmpty,
};

const int label_shift_f4[]=
{
  mKeyEmpty,//levelRoot
  mKeyEmpty,//levelFilePerm
  mKeyEmpty,//levelFilePermNewFolder
  mKeyEmpty,//levelFilePermNewFile
  mKeyEmpty,//levelFileAudit
  mKeyEmpty,//levelFileAuditFolder
  mKeyEmpty,//levelFileAuditFile
  mKeyEmpty,//levelRegRoot
  mKeyEmpty,//levelRegRights
  mKeyEmpty,//levelRegRightsNewKey
  mKeyEmpty,//levelRegRightsNewPad
  mKeyEmpty,//levelRegAudit
  mKeyEmpty,//levelRegAuditKey
  mKeyEmpty,//levelRegAuditPad
  mKeyEmpty,//levelShared
  mKeyEmpty,//levelSharedIn
  mKeyEmpty,//levelSharedInAllowed
  mKeyEmpty,//levelPrinterRoot
  mKeyEmpty,//levelPrinterPerm
  mKeyEmpty,//levelPrinterPermContainer
  mKeyEmpty,//levelPrinterPermJob
  mKeyEmpty,//levelPrinterAudit
  mKeyEmpty,//levelPrinterAuditContainer
  mKeyEmpty,//levelPrinterAuditJob
  mKeyEmpty,//levelPrinterShared
  mKeyAddUser,//levelGroups
  mKeyAddUser,//levelUsers
  mKeyEmpty,//levelRights
  mKeyEmpty,//levelRightUsers
};

const int label_f6[]=
{
  mKeyEmpty,//levelRoot
  mKeyEmpty,//levelFilePerm
  mKeyEmpty,//levelFilePermNewFolder
  mKeyEmpty,//levelFilePermNewFile
  mKeyEmpty,//levelFileAudit
  mKeyEmpty,//levelFileAuditFolder
  mKeyEmpty,//levelFileAuditFile
  mKeyEmpty,//levelRegRoot
  mKeyEmpty,//levelRegRights
  mKeyEmpty,//levelRegRightsNewKey
  mKeyEmpty,//levelRegRightsNewPad
  mKeyEmpty,//levelRegAudit
  mKeyEmpty,//levelRegAuditKey
  mKeyEmpty,//levelRegAuditPad
  mKeyEmpty,//levelShared
  mKeyEmpty,//levelSharedIn
  mKeyEmpty,//levelSharedInAllowed
  mKeyEmpty,//levelPrinterRoot
  mKeyEmpty,//levelPrinterPerm
  mKeyEmpty,//levelPrinterPermContainer
  mKeyEmpty,//levelPrinterPermJob
  mKeyEmpty,//levelPrinterAudit
  mKeyEmpty,//levelPrinterAuditContainer
  mKeyEmpty,//levelPrinterAuditJob
  mKeyEmpty,//levelPrinterShared
  mKeyRemote,//levelGroups
  mKeyRemove,//levelUsers
  mKeyRemote,//levelRights
  mKeyRemote,//levelRightUsers
};

const int label_shift_f6[]=
{
  mKeyEmpty,//levelRoot
  mKeyEmpty,//levelFilePerm
  mKeyEmpty,//levelFilePermNewFolder
  mKeyEmpty,//levelFilePermNewFile
  mKeyEmpty,//levelFileAudit
  mKeyEmpty,//levelFileAuditFolder
  mKeyEmpty,//levelFileAuditFile
  mKeyEmpty,//levelRegRoot
  mKeyEmpty,//levelRegRights
  mKeyEmpty,//levelRegRightsNewKey
  mKeyEmpty,//levelRegRightsNewPad
  mKeyEmpty,//levelRegAudit
  mKeyEmpty,//levelRegAuditKey
  mKeyEmpty,//levelRegAuditPad
  mKeyEmpty,//levelShared
  mKeyEmpty,//levelSharedIn
  mKeyEmpty,//levelSharedInAllowed
  mKeyEmpty,//levelPrinterRoot
  mKeyEmpty,//levelPrinterPerm
  mKeyEmpty,//levelPrinterPermContainer
  mKeyEmpty,//levelPrinterPermJob
  mKeyEmpty,//levelPrinterAudit
  mKeyEmpty,//levelPrinterAuditContainer
  mKeyEmpty,//levelPrinterAuditJob
  mKeyEmpty,//levelPrinterShared
  mKeyLocal,//levelGroups
  mKeyRemove,//levelUsers
  mKeyLocal,//levelRights
  mKeyLocal,//levelRightUsers
};

const int label_f7[]=
{
  //files
  mKeyEmpty,
  mKeyEmpty,
  mKeyEmpty,
  mKeyEmpty,
  mKeyEmpty,
  mKeyEmpty,
  mKeyEmpty,
  //reg
  mKeyEmpty,
  mKeyEmpty,
  mKeyEmpty,
  mKeyEmpty,
  mKeyEmpty,
  mKeyEmpty,
  mKeyEmpty,
  //share
  mKeyAddShare,
  mKeyEmpty,
  mKeyEmpty,
  //printer
  mKeyEmpty,
  mKeyEmpty,
  mKeyEmpty,
  mKeyEmpty,
  mKeyEmpty,
  mKeyEmpty,
  mKeyEmpty,
  //printer share
  mKeyAddShare,
  //users
  mKeyAddGroup,
  mKeyEmpty,
  //rights
  mKeyEmpty,
  mKeyEmpty,
};

const bool sort[]=
{
  //files
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  //reg
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  //share
  false,
  true,
  true,
  //printer
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  //printer share
  false,
  //users
  false,
  false,
  //rights
  false,
  false,
};
