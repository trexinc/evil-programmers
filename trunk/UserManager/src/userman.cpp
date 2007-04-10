/*
    userman.cpp
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
#include <limits.h>
#include "../../plugin.hpp"
#include <lm.h>
#include <ntsecapi.h>
#include "umplugin.h"
#include "memory.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
char PluginRootKey[80];
BOOL IsOldFAR=TRUE;

struct Options Opt={false,0,true,true,true,"acl"};

UserManager *panels[2]={NULL,NULL};

WellKnownSID AddSID[]=
{
  {{{SECURITY_NULL_SID_AUTHORITY}},SECURITY_NULL_RID},
  {{{SECURITY_WORLD_SID_AUTHORITY}},SECURITY_WORLD_RID},
  {{{SECURITY_LOCAL_SID_AUTHORITY}},SECURITY_LOCAL_RID},
  {{{SECURITY_CREATOR_SID_AUTHORITY}},SECURITY_CREATOR_OWNER_RID},
  {{{SECURITY_CREATOR_SID_AUTHORITY}},SECURITY_CREATOR_GROUP_RID},
  {{{SECURITY_CREATOR_SID_AUTHORITY}},SECURITY_CREATOR_OWNER_SERVER_RID},
  {{{SECURITY_CREATOR_SID_AUTHORITY}},SECURITY_CREATOR_GROUP_SERVER_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_DIALUP_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_NETWORK_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_BATCH_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_INTERACTIVE_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_SERVICE_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_ANONYMOUS_LOGON_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_PROXY_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_ENTERPRISE_CONTROLLERS_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_PRINCIPAL_SELF_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_AUTHENTICATED_USER_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_RESTRICTED_CODE_RID},
  {{{SECURITY_NT_AUTHORITY}},13}, //SECURITY_TERMINAL_SERVER_USER_RID
  {{{SECURITY_NT_AUTHORITY}},14}, //SECURITY_REMOTE_INTERACTIVE_LOGON_RID
  {{{SECURITY_NT_AUTHORITY}},SECURITY_LOCAL_SYSTEM_RID},
  {{{SECURITY_NT_AUTHORITY}},19}, //SECURITY_LOCAL_SERVICE_RID
  {{{SECURITY_NT_AUTHORITY}},20}, //SECURITY_NETWORK SERVICE_RID
};

wchar_t *AddPriveleges[]=
{
  L"SeAssignPrimaryTokenPrivilege",
  L"SeAuditPrivilege",
  L"SeBackupPrivilege",
  L"SeChangeNotifyPrivilege",
  L"SeCreatePagefilePrivilege",
  L"SeCreatePermanentPrivilege",
  L"SeCreateTokenPrivilege",
  L"SeDebugPrivilege",
  L"SeEnableDelegationPrivilege",
  L"SeIncreaseBasePriorityPrivilege",
  L"SeIncreaseQuotaPrivilege",
  L"SeLoadDriverPrivilege",
  L"SeLockMemoryPrivilege",
  L"SeMachineAccountPrivilege",
  L"SeProfileSingleProcessPrivilege",
  L"SeRemoteShutdownPrivilege",
  L"SeRestorePrivilege",
  L"SeSecurityPrivilege",
  L"SeShutdownPrivilege",
  L"SeSyncAgentPrivilege",
  L"SeSystemEnvironmentPrivilege",
  L"SeSystemProfilePrivilege",
  L"SeSystemtimePrivilege",
  L"SeTakeOwnershipPrivilege",
  L"SeTcbPrivilege",
  L"SeUndockPrivilege",
  L"SeUnsolicitedInputPrivilege",
  L"SeManageVolumePrivilege",
  L"SeInteractiveLogonRight",
  L"SeNetworkLogonRight",
  L"SeBatchLogonRight",
  L"SeServiceLogonRight",
};

char *AddPrivelegeNames[]=
{
  "Replace a process level token",
  "Generate security audits",
  "Back up files and directories",
  "Bypass traverse checking",
  "Create a paging file",
  "Create permanent shared objects",
  "Create a token object",
  "Debug programs",
  "Enable computer and user accounts to be trusted for delegation",
  "Increase scheduling priority",
  "Increase quotas",
  "Load and unload device drivers",
  "Lock pages in memory",
  "Add workstations to domain",
  "Profile single process",
  "Force shutdown from a remote system",
  "Restore files and directories",
  "Manage auditing and security log",
  "Shut down the system",
  "Synchronize directory service data",
  "Modify firmware environment values",
  "Profile system performance",
  "Change the system time",
  "Take ownership of files or other objects",
  "Act as part of the operating system",
  "Remove computer from docking station",
  "Read unsolicited input from a terminal device",
  "Manage volume",
  "Log on locally",
  "Access this computer from the network",
  "Log on as a batch job",
  "Log on as a service",
};

static char *default_column_data="";

extern LSA_HANDLE GetPolicyHandle(wchar_t *computer);

static void add_panel(UserManager *panel)
{
  if(panels[0]==NULL)
    panels[0]=panel;
  else if(panels[1]==NULL)
    panels[1]=panel;
}

static void del_panel(UserManager *panel)
{
  if(panels[0]==panel)
    panels[0]=NULL;
  else if(panels[1]==panel)
    panels[1]=NULL;
}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>(int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
  if(Info->StructSize>FAR165_INFO_SIZE)
  {
    IsOldFAR=FALSE;
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;

    strcpy(PluginRootKey,Info->RootKey);
    strcat(PluginRootKey,"\\userman");

    HKEY hKey;
    DWORD Type;
    DWORD DataSize=0;
    if((RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
    {
      DataSize=sizeof(Opt.AddToDisksMenu);
      RegQueryValueEx(hKey,"AddToDisksMenu",0,&Type,(LPBYTE)&Opt.AddToDisksMenu,&DataSize);
      DataSize=sizeof(Opt.DisksMenuDigit);
      RegQueryValueEx(hKey,"DisksMenuDigit",0,&Type,(LPBYTE)&Opt.DisksMenuDigit,&DataSize);
      DataSize=sizeof(Opt.AddToPluginsMenu);
      RegQueryValueEx(hKey,"AddToPluginsMenu",0,&Type,(LPBYTE)&Opt.AddToPluginsMenu,&DataSize);
      DataSize=sizeof(Opt.AddToConfigMenu);
      RegQueryValueEx(hKey,"AddToConfigMenu",0,&Type,(LPBYTE)&Opt.AddToConfigMenu,&DataSize);
      DataSize=sizeof(Opt.FullUserNames);
      RegQueryValueEx(hKey,"FullUserNames",0,&Type,(LPBYTE)&Opt.FullUserNames,&DataSize);
      DataSize=sizeof(Opt.Prefix);
      RegQueryValueEx(hKey,"Prefix",0,&Type,(LPBYTE)Opt.Prefix,&DataSize);
      RegCloseKey(hKey);
      if(!Opt.Prefix[0])
        strcpy(Opt.Prefix,"acl");
    }
    if(!IsPrivilegeEnabled(SE_SECURITY_NAME))
      EnablePrivilege(SE_SECURITY_NAME);
    if(!IsPrivilegeEnabled(SE_TAKE_OWNERSHIP_NAME))
      EnablePrivilege(SE_TAKE_OWNERSHIP_NAME);
    if(!IsPrivilegeEnabled(SE_RESTORE_NAME))
      EnablePrivilege(SE_RESTORE_NAME);
    if(!IsPrivilegeEnabled(SE_BACKUP_NAME))
      EnablePrivilege(SE_BACKUP_NAME);

    EnablePrivilege(SE_CREATE_TOKEN_NAME);
    EnablePrivilege(SE_ASSIGNPRIMARYTOKEN_NAME);
    EnablePrivilege(SE_LOCK_MEMORY_NAME);
    EnablePrivilege(SE_INCREASE_QUOTA_NAME);
    EnablePrivilege(SE_UNSOLICITED_INPUT_NAME);
    EnablePrivilege(SE_MACHINE_ACCOUNT_NAME);
    EnablePrivilege(SE_TCB_NAME);
    EnablePrivilege(SE_SECURITY_NAME);
    EnablePrivilege(SE_TAKE_OWNERSHIP_NAME);
    EnablePrivilege(SE_LOAD_DRIVER_NAME);
    EnablePrivilege(SE_SYSTEM_PROFILE_NAME);
    EnablePrivilege(SE_SYSTEMTIME_NAME);
    EnablePrivilege(SE_PROF_SINGLE_PROCESS_NAME);
    EnablePrivilege(SE_INC_BASE_PRIORITY_NAME);
    EnablePrivilege(SE_CREATE_PAGEFILE_NAME);
    EnablePrivilege(SE_CREATE_PERMANENT_NAME);
    EnablePrivilege(SE_BACKUP_NAME);
    EnablePrivilege(SE_RESTORE_NAME);
    EnablePrivilege(SE_SHUTDOWN_NAME);
    EnablePrivilege(SE_DEBUG_NAME);
    EnablePrivilege(SE_AUDIT_NAME);
    EnablePrivilege(SE_SYSTEM_ENVIRONMENT_NAME);
    EnablePrivilege(SE_CHANGE_NOTIFY_NAME);
    EnablePrivilege(SE_REMOTE_SHUTDOWN_NAME);

    init_current_user();
  }
}

void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  if(!IsOldFAR)
  {
    Info->StructSize=sizeof(*Info);
    Info->Flags=0;

    static char *DisksMenuStrings[1];
    DisksMenuStrings[0]=GetMsg(mNameDisk);
    static int DisksMenuNumbers[1];
    Info->DiskMenuStrings=DisksMenuStrings;
    DisksMenuNumbers[0]=Opt.DisksMenuDigit;
    Info->DiskMenuNumbers=DisksMenuNumbers;
    Info->DiskMenuStringsNumber=Opt.AddToDisksMenu?1:0;

    static char *PluginMenuStrings[1];
    PluginMenuStrings[0]=GetMsg(mName);
    Info->PluginMenuStrings=PluginMenuStrings;
    Info->PluginMenuStringsNumber=Opt.AddToPluginsMenu?(sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0])):0;
    Info->PluginConfigStrings=PluginMenuStrings;
    Info->PluginConfigStringsNumber=Opt.AddToConfigMenu?(sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0])):0;
    Info->CommandPrefix=Opt.Prefix;
  }
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  if(IsOldFAR) return INVALID_HANDLE_VALUE;
  PanelInfo PInfo;
  Info.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&PInfo);
  UserManager *panel=NULL;
  panel=(UserManager *)malloc(sizeof(UserManager));
  if(panel)
  {
    panel->level=-1;
    if(OpenFrom==OPEN_PLUGINSMENU)
    {
      int Msgs[]={mMenuProp,mMenuApply,mMenuSep1,mMenuUsers,mMenuRights};
      FarMenuItem MenuItems[mMenuRights-mMenuProp+1];
      for(unsigned int i=0;i<(mMenuRights-mMenuProp+1);i++)
      {
        MenuItems[i].Selected=MenuItems[i].Checked=MenuItems[i].Separator=FALSE;
        strcpy(MenuItems[i].Text,GetMsg(Msgs[i])); // Text in menu
        if(MenuItems[i].Text[0]=='-') MenuItems[i].Separator=TRUE;
      };
      MenuItems[0].Selected=TRUE;
      int MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,
                         NULL,NULL,"Contents",NULL,NULL,
                         MenuItems,sizeof(MenuItems)/sizeof(MenuItems[0]));
      switch(MenuCode)
      {
        case 0:
          if(PInfo.PanelType==PTYPE_FILEPANEL)
          {
            if(PInfo.SelectedItemsNumber>0)
            {
              if(PInfo.Plugin)
                panel->level=parse_dir(PInfo.CurDir,PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName,NULL,pathtypePlugin,&(panel->param),panel->hostfile,panel->hostfile_oem);
              else
              {
                wchar_t filename_w[MAX_PATH];
                if(GetWideName(PInfo.CurDir,&PInfo.PanelItems[PInfo.CurrentItem].FindData,filename_w))
                  panel->level=parse_dir(PInfo.CurDir,PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName,filename_w,pathtypeReal,&(panel->param),panel->hostfile,panel->hostfile_oem);
              }
            }
          }
          else if(PInfo.PanelType==PTYPE_TREEPANEL)
            panel->level=parse_dir(PInfo.CurDir,NULL,NULL,pathtypeTree,&(panel->param),panel->hostfile,panel->hostfile_oem);
          break;
        case 1:
          if(PInfo.PanelType==PTYPE_FILEPANEL)
            ProcessChilds(&PInfo);
          break;
        case 3:
          panel->level=levelGroups;
          break;
        case 4:
          panel->level=levelRights;
          break;
      }
    }
    else if(OpenFrom==OPEN_DISKMENU)
    {
      panel->level=levelGroups;
    }
    else if(OpenFrom==OPEN_COMMANDLINE)
    {
      char *cmd=(char *)Item;
      char name[MAX_PATH];
      if(strlen(cmd))
      {
        FSF.Unquote(FSF.Trim(strcpy(name,cmd)));
        panel->level=parse_dir(name,NULL,NULL,pathtypeUnknown,&(panel->param),panel->hostfile,panel->hostfile_oem);
      }
      else panel->level=levelGroups;
    }
    if(panel->level>-1)
    {
      panel->computer_ptr=NULL;
      panel->flags=0;
      wcscpy(panel->domain,L"");
      if(panel->level==levelRoot)
      {
        DWORD attr=GetFileAttributesW(panel->hostfile);
        if((attr!=0xFFFFFFFF)&&(attr&FILE_ATTRIBUTE_DIRECTORY))
          panel->flags|=FLAG_FOLDER;
        //network support.
        if(!wcsncmp(panel->hostfile,L"\\\\",2))
        {
          panel->flags|=FLAG_NETPATH;
          wchar_t *ptr=wcschr(panel->hostfile+2,'\\');
          if(ptr)
          {
            wcsncpy(panel->computer,panel->hostfile+2,ptr-(panel->hostfile+2));
            panel->computer[ptr-(panel->hostfile+2)]=0;
            ptr++;
            wchar_t *file_ptr=wcschr(ptr,'\\');
//            if(file_ptr)
            {
              wchar_t share[MAX_PATH];
              if(file_ptr)
              {
                wcsncpy(share,ptr,file_ptr-ptr);
                share[file_ptr-ptr]=0;
                file_ptr++;
              }
              else wcscpy(share,ptr);
              NET_API_STATUS res;
              SHARE_INFO_502 *shares;
              DWORD entriesread,totalentries,resumehandle=0;
              res=NetShareEnum(panel->computer,502,(LPBYTE *)&shares,MAX_PREFERRED_LENGTH,&entriesread,&totalentries,&resumehandle);
              if((res==ERROR_SUCCESS)/*||(res==ERROR_MORE_DATA)*/)
              {
                for(unsigned long i=0;i<entriesread;i++)
                {
                  if(!wcscmp(share,(wchar_t *)shares[i].shi502_netname))
                  {
                    panel->computer_ptr=panel->computer;
                    wcscpy(panel->domain,(wchar_t *)shares[i].shi502_path);
                    if(file_ptr)
                    {
                      wcsaddendslash(panel->domain);
                      wcscat(panel->domain,file_ptr);
                    }
                    break;
                  }
                }
                NetApiBufferFree(shares);
              }
            }
          }
        }
      }
      add_panel(panel);
      return (HANDLE)panel;
    }
    free(panel);
  }
  return INVALID_HANDLE_VALUE;
}

void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
  UserManager *panel=(UserManager *)hPlugin;
  del_panel(panel);
  free(panel);
}

static void NormalizeCustomColumns(PluginPanelItem *tmpItem)
{
  for(int i=0;i<CUSTOM_COLUMN_COUNT;i++)
    if(!tmpItem->CustomColumnData[i])
      tmpItem->CustomColumnData[i]=default_column_data;
  tmpItem->CustomColumnNumber=CUSTOM_COLUMN_COUNT;
}

static void SetDescription(PluginPanelItem *tmpItem,wchar_t *Description)
{
  tmpItem->Description=(char *)malloc((wcslen(Description)+1)*sizeof(char));
  if(tmpItem->Description)
    WideCharToMultiByte(CP_OEMCP,0,Description,-1,tmpItem->Description,wcslen(Description)+1,NULL,NULL);
}

int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  if(!IsOldFAR)
  {
    UserManager *panel=(UserManager *)hPlugin;
    *pPanelItem=NULL; *pItemsNumber=0;
    panel->error=false;
    if(plain_dirs_dir[panel->level])
    {
      int ItemCount=(panel->flags&FLAG_FOLDER)?plain_dirs_dir[panel->level][0]:plain_dirs_dir[panel->level][1];
      int OwnerCount=plain_dirs_owners[panel->level]?1:0;
      *pPanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*(ItemCount+OwnerCount));
      if(*pPanelItem)
      {
        *pItemsNumber=ItemCount+OwnerCount;
        for(int i=0;i<ItemCount;i++)
        {
          (*pPanelItem)[i].FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
          strcpy((*pPanelItem)[i].FindData.cFileName,GetMsg(plain_dirs_dir[panel->level][i*2+2]));
          AddDefaultUserdata((*pPanelItem+i),plain_dirs_dir[panel->level][i*2+3],i,0,NULL,NULL,(*pPanelItem)[i].FindData.cFileName);
        }
        if(OwnerCount)
        {
          wchar_t *owner;
          char *owner_oem;
          PSID sid;
          if(plain_dirs_owners[panel->level](panel,&sid,&owner,&owner_oem))
          {
            strcpy((*pPanelItem)[ItemCount].FindData.cFileName,owner_oem);
            AddDefaultUserdata((*pPanelItem)+ItemCount,-1,ItemCount,0,sid,owner,owner_oem);
          }
        }
      }
    }
    else if(perm_dirs_dir[panel->level])
    {
      AclData *data=NULL;
      panel->error=!GetAcl(panel,panel->level,&data);
      if(!(panel->error))
      {
        int item_count=0;
        item_count=data->Count;
        if(perm_dirs_dir[panel->level]==PERM_KEY)
        { item_count++; }
        else if((perm_dirs_dir[panel->level]==PERM_FF)&&(panel->flags&FLAG_FOLDER))
        { item_count++; item_count++; }
        else if(perm_dirs_dir[panel->level]==PERM_PRINT)
        { item_count++; item_count++; }
        *pPanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*item_count);
        if(*pPanelItem)
        {
          *pItemsNumber=item_count;
          AceData *tmpAce=data->Aces;
          PluginPanelItem *tmpItem=*pPanelItem;
          wchar_t *username; char *username_oem;
          int cur_sortorder=0;
          while(tmpAce)
          {
            GetUserNameEx(panel->computer_ptr,tmpAce->user,Opt.FullUserNames,&username,&username_oem);
            switch(GetAclState(panel->level,tmpAce->ace_type,tmpAce->ace_flags))
            {
              case UM_ITEM_EMPTY:
                strcpy(tmpItem->FindData.cFileName,"");
                break;
              case UM_ITEM_SUCCESS:
              case UM_ITEM_ALLOW:
                strcpy(tmpItem->FindData.cFileName,"+.");
                break;
              case UM_ITEM_FAIL:
              case UM_ITEM_DENY:
                strcpy(tmpItem->FindData.cFileName,"-.");
                break;
              case UM_ITEM_SUCCESS_FAIL:
                strcpy(tmpItem->FindData.cFileName,"*.");
                break;
            }
            strcat(tmpItem->FindData.cFileName,username_oem);
            tmpItem->CustomColumnData=(char **)malloc(sizeof(char *)*CUSTOM_COLUMN_COUNT);
            if(tmpItem->CustomColumnData)
            {
              tmpItem->CustomColumnData[0]=(char *)malloc(7*sizeof(char));
              if(tmpItem->CustomColumnData[0])
                strcpy(tmpItem->CustomColumnData[0],get_access_string(panel->level,tmpAce->ace_mask));
              tmpItem->CustomColumnData[1]=get_sid_string(tmpAce->user);
              NormalizeCustomColumns(tmpItem);
            }
            AddDefaultUserdata(tmpItem,tmpAce->ace_mask,cur_sortorder++,GetAclState(panel->level,tmpAce->ace_type,tmpAce->ace_flags),tmpAce->user,username,NULL);
            tmpAce=tmpAce->next;
            tmpItem++;
          }
          if(perm_dirs_dir[panel->level]==PERM_KEY)
          {
            strcpy(tmpItem->FindData.cFileName,GetMsg(mDirNewKey));
            tmpItem->FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
            AddDefaultUserdata(tmpItem,panel->level+1,cur_sortorder++,0,NULL,NULL,tmpItem->FindData.cFileName);
            tmpItem++;
          }
          else if((perm_dirs_dir[panel->level]==PERM_FF)&&(panel->flags&FLAG_FOLDER))
          {
            strcpy(tmpItem->FindData.cFileName,GetMsg(mDirNewFolder));
            tmpItem->FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
            AddDefaultUserdata(tmpItem,panel->level+1,cur_sortorder++,0,NULL,NULL,tmpItem->FindData.cFileName);
            tmpItem++;
            strcpy(tmpItem->FindData.cFileName,GetMsg(mDirNewFile));
            tmpItem->FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
            AddDefaultUserdata(tmpItem,panel->level+2,cur_sortorder++,0,NULL,NULL,tmpItem->FindData.cFileName);
            tmpItem++;
          }
          else if(perm_dirs_dir[panel->level]==PERM_PRINT)
          {
            strcpy(tmpItem->FindData.cFileName,GetMsg(mDirNewContainer));
            tmpItem->FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
            AddDefaultUserdata(tmpItem,panel->level+1,cur_sortorder++,0,NULL,NULL,tmpItem->FindData.cFileName);
            tmpItem++;
            strcpy(tmpItem->FindData.cFileName,GetMsg(mDirNewJob));
            tmpItem->FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
            AddDefaultUserdata(tmpItem,panel->level+2,cur_sortorder++,0,NULL,NULL,tmpItem->FindData.cFileName);
            tmpItem++;
          }
        }
        else
        {
          panel->error=true;
          if(!(OpMode&OPM_FIND)) ShowError(0);
        }
        FreeAcl(data);
      }
      else if(!(OpMode&OPM_FIND)) ShowCustomError(mErrorCantGetACL);
    }
    else if(panel->level==levelShared)
    {
      SHARE_INFO_502 *shares;
      DWORD entriesread,totalentries,resumehandle=0;
      NET_API_STATUS err=NetShareEnum(panel->computer_ptr,502,(LPBYTE *)&shares,MAX_PREFERRED_LENGTH,&entriesread,&totalentries,&resumehandle);
      if(err==NERR_Success)
      {
        int count=0;
        for(unsigned long i=0;i<entriesread;i++)
          if(!_wcsicmp((wchar_t *)shares[i].shi502_path,(panel->computer_ptr)?panel->domain:panel->hostfile))
            count++;
        *pPanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*count);
        if(*pPanelItem)
        {
          *pItemsNumber=count;
          for(unsigned long i=0,j=0;i<entriesread;i++)
          {
            if(j>=(unsigned long)count) break;
            if(!_wcsicmp((wchar_t *)shares[i].shi502_path,(panel->computer_ptr)?panel->domain:panel->hostfile))
            {
              WideCharToMultiByte(CP_OEMCP,0,(wchar_t *)shares[i].shi502_netname,-1,(*pPanelItem)[j].FindData.cFileName,MAX_PATH,NULL,NULL);
              (*pPanelItem)[j].FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
              SetDescription((*pPanelItem)+j,(wchar_t *)shares[i].shi502_remark);
              AddDefaultUserdata((*pPanelItem+j),levelSharedIn,0,0,NULL,(wchar_t *)shares[i].shi502_netname,NULL);
              (*pPanelItem)[j].CustomColumnData=(char **)malloc(sizeof(char *)*CUSTOM_COLUMN_COUNT);
              if((*pPanelItem)[j].CustomColumnData)
              {
                (*pPanelItem)[j].CustomColumnData[0]=(char *)malloc(11*sizeof(char));
                if((*pPanelItem)[j].CustomColumnData[0])
                {
                  if(shares[i].shi502_max_uses==0xffffffff)
                    strcpy((*pPanelItem)[j].CustomColumnData[0],"maximum");
                  else
                    sprintf((*pPanelItem)[j].CustomColumnData[0],"%ld",shares[i].shi502_max_uses);
                }
                (*pPanelItem)[j].CustomColumnData[1]=(char *)malloc(11*sizeof(char));
                if((*pPanelItem)[j].CustomColumnData[1])
                  sprintf((*pPanelItem)[j].CustomColumnData[1],"%ld",shares[i].shi502_current_uses);
                (*pPanelItem)[j].CustomColumnData[2]=(char *)malloc((wcslen((wchar_t *)shares[i].shi502_path)+1)*sizeof(char));
                if((*pPanelItem)[j].CustomColumnData[2])
                  WideCharToMultiByte(CP_OEMCP,0,(wchar_t *)shares[i].shi502_path,-1,(*pPanelItem)[j].CustomColumnData[2],wcslen((wchar_t *)shares[i].shi502_path)+1,NULL,NULL);
                NormalizeCustomColumns((*pPanelItem)+j);
              }
              j++;
            }
          }
        }
        NetApiBufferFree(shares);
      }
    }
    else if(panel->level==levelPrinterShared)
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
                if(data->Attributes&PRINTER_ATTRIBUTE_SHARED)
                {
                  *pPanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem));
                  if(*pPanelItem)
                  {
                    *pItemsNumber=1;
                    WideCharToMultiByte(CP_OEMCP,0,data->pShareName,-1,(*pPanelItem)[0].FindData.cFileName,MAX_PATH,NULL,NULL);
                  }
                }
              }
              free(data); data=NULL;
            }
          }
        }
        ClosePrinter(printer);
      }
    }
    else if(panel->level==levelGroups)
    {
      LPBYTE groups=NULL,users=NULL;
      DWORD entriesread1,totalentries1,resumehandle1=0; //groups
      DWORD entriesread2,totalentries2,resumehandle2=0; //users
      NET_API_STATUS err1,err2;
      if(panel->global)
        err1=NetGroupEnum(panel->domain,1,&groups,MAX_PREFERRED_LENGTH,&entriesread1,&totalentries1,&resumehandle1);
      else
        err1=NetLocalGroupEnum(panel->computer_ptr,1,&groups,MAX_PREFERRED_LENGTH,&entriesread1,&totalentries1,&resumehandle1);
      err2=NetUserEnum((panel->global)?(panel->domain):(panel->computer_ptr),10,FILTER_NORMAL_ACCOUNT,&users,MAX_PREFERRED_LENGTH,&entriesread2,&totalentries2,&resumehandle2);
      if((err1==NERR_Success)&&(err2==NERR_Success))
      {
        wchar_t *group_name;
        wchar_t *group_comment;
        *pPanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*(entriesread1+entriesread2+sizeof(AddSID)/sizeof(AddSID[0])+1));
        if(*pPanelItem)
        {
          *pItemsNumber=entriesread1+entriesread2+sizeof(AddSID)/sizeof(AddSID[0])+1;
          for(unsigned long i=0;i<entriesread1;i++)
          {
            if(panel->global)
            {
              group_name=((GROUP_INFO_1 *)groups)[i].grpi1_name;
              group_comment=((GROUP_INFO_1 *)groups)[i].grpi1_comment;
            }
            else
            {
              group_name=((LOCALGROUP_INFO_1 *)groups)[i].lgrpi1_name;
              group_comment=((LOCALGROUP_INFO_1 *)groups)[i].lgrpi1_comment;
            }
            WideCharToMultiByte(CP_OEMCP,0,group_name,-1,(*pPanelItem)[i].FindData.cFileName,MAX_PATH,NULL,NULL);
            (*pPanelItem)[i].FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
            bool sid_ok=false;
            DWORD sid_size=0,domain_size=0; PSID sid=NULL; wchar_t *domain=NULL; SID_NAME_USE type;
            if(!LookupAccountNameW((panel->global)?(panel->domain):(panel->computer_ptr),group_name,NULL,&sid_size,NULL,&domain_size,&type))
              if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
              {
                domain=(wchar_t *)malloc(domain_size*sizeof(wchar_t)+sid_size);
                sid=domain+domain_size;
                if(domain)
                  if(LookupAccountNameW((panel->global)?(panel->domain):(panel->computer_ptr),group_name,sid,&sid_size,domain,&domain_size,&type))
                    sid_ok=true;
              }
            if(sid_ok)
              AddDefaultUserdata((*pPanelItem)+i,levelUsers,0,0,sid,group_name,NULL);
            SetDescription((*pPanelItem)+i,group_comment);
            free(domain);
          }
          for(unsigned long i=0;i<entriesread2;i++)
          {
            WideCharToMultiByte(CP_OEMCP,0,((USER_INFO_10 *)users)[i].usri10_name,-1,(*pPanelItem)[i+entriesread1].FindData.cFileName,MAX_PATH,NULL,NULL);
            bool sid_ok=false;
            DWORD sid_size=0,domain_size=0; PSID sid=NULL; wchar_t *domain=NULL; SID_NAME_USE type;
            if(!LookupAccountNameW((panel->global)?(panel->domain):(panel->computer_ptr),((USER_INFO_10 *)users)[i].usri10_name,NULL,&sid_size,NULL,&domain_size,&type))
              if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
              {
                domain=(wchar_t *)malloc(domain_size*sizeof(wchar_t)+sid_size);
                sid=domain+domain_size;
                if(domain)
                  if(LookupAccountNameW((panel->global)?(panel->domain):(panel->computer_ptr),((USER_INFO_10 *)users)[i].usri10_name,sid,&sid_size,domain,&domain_size,&type))
                    sid_ok=true;
              }
            if(sid_ok)
            {
              AddDefaultUserdata((*pPanelItem)+i+entriesread1,levelUsers,0,0,sid,((USER_INFO_10 *)users)[i].usri10_name,NULL);
              if(is_current_user(sid)) (*pPanelItem)[i+entriesread1].FindData.dwFileAttributes=FILE_ATTRIBUTE_READONLY;
            }
            SetDescription((*pPanelItem)+i+entriesread1,((USER_INFO_10 *)users)[i].usri10_comment);
            free(domain);
          }
          for(unsigned int i=0;i<sizeof(AddSID)/sizeof(AddSID[0]);i++)
          {
            PSID sid=NULL;
            if(AllocateAndInitializeSid(AddSID[i].sid_id,1,AddSID[i].SubAuthority,0,0,0,0,0,0,0,&sid))
            {
              wchar_t *username; char *username_oem;
              GetUserNameEx(panel->computer_ptr,sid,false,&username,&username_oem);
              strcpy((*pPanelItem)[i+entriesread1+entriesread2].FindData.cFileName,username_oem);
              AddDefaultUserdata((*pPanelItem)+i+entriesread1+entriesread2,0,0,0,sid,username,NULL);
              if(is_current_user(sid)) (*pPanelItem)[i+entriesread1+entriesread2].FindData.dwFileAttributes=FILE_ATTRIBUTE_READONLY;
            }
          }
          for(unsigned int i=0;i<(entriesread1+entriesread2+sizeof(AddSID)/sizeof(AddSID[0]));i++)
          {
            (*pPanelItem)[i].CustomColumnData=(char **)malloc(sizeof(char *)*CUSTOM_COLUMN_COUNT);
            if((*pPanelItem)[i].CustomColumnData)
            {
              if((*pPanelItem)[i].Flags&PPIF_USERDATA)
                (*pPanelItem)[i].CustomColumnData[1]=get_sid_string(GetSidFromUserData((*pPanelItem)[i].UserData));
              NormalizeCustomColumns((*pPanelItem)+i);
            }
          }
          strcpy((*pPanelItem)[entriesread1+entriesread2+sizeof(AddSID)/sizeof(AddSID[0])].FindData.cFileName,"..");
          (*pPanelItem)[entriesread1+entriesread2+sizeof(AddSID)/sizeof(AddSID[0])].FindData.dwFileAttributes=0;
        }
      }
      if(groups) NetApiBufferFree(groups);
      if(users) NetApiBufferFree(users);
    }
    else if(panel->level==levelUsers)
    {
      LPBYTE members;
      LOCALGROUP_MEMBERS_INFO_1* localMembers=NULL;
      GROUP_USERS_INFO_0* globalMembers=NULL;
      DWORD entriesread,totalentries,resumehandle=0;
      NET_API_STATUS err;
      if(panel->global)
      {
        err=NetGroupGetUsers(panel->domain,panel->nonfixed,0,&members,MAX_PREFERRED_LENGTH,&entriesread,&totalentries,&resumehandle);
        globalMembers=(GROUP_USERS_INFO_0*)members;
      }
      else
      {
        err=NetLocalGroupGetMembers(panel->computer_ptr,panel->nonfixed,1,&members,MAX_PREFERRED_LENGTH,&entriesread,&totalentries,&resumehandle);
        localMembers=(LOCALGROUP_MEMBERS_INFO_1*)members;
      }
      if(err==NERR_Success)
      {
        *pPanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*(entriesread+1));
        if(*pPanelItem)
        {
          *pItemsNumber=entriesread+1;
          for(unsigned long i=0;i<entriesread;i++)
          {
            if(panel->global)
            {
              WideCharToMultiByte(CP_OEMCP,0,globalMembers[i].grui0_name,-1,(*pPanelItem)[i].FindData.cFileName,MAX_PATH,NULL,NULL);
              AddDefaultUserdata((*pPanelItem)+i,-1,0,0,NULL,globalMembers[i].grui0_name,NULL);
            }
            else
            {
              WideCharToMultiByte(CP_OEMCP,0,localMembers[i].lgrmi1_name,-1,(*pPanelItem)[i].FindData.cFileName,MAX_PATH,NULL,NULL);
              AddDefaultUserdata((*pPanelItem)+i,-1,0,0,localMembers[i].lgrmi1_sid,localMembers[i].lgrmi1_name,NULL);
              if(is_current_user(localMembers[i].lgrmi1_sid)) (*pPanelItem)[i].FindData.dwFileAttributes=FILE_ATTRIBUTE_READONLY;
              (*pPanelItem)[i].CustomColumnData=(char **)malloc(sizeof(char *)*CUSTOM_COLUMN_COUNT);
              if((*pPanelItem)[i].CustomColumnData)
              {
                (*pPanelItem)[i].CustomColumnData[1]=get_sid_string(localMembers[i].lgrmi1_sid);
                NormalizeCustomColumns((*pPanelItem)+i);
              }
            }
          }
          strcpy((*pPanelItem)[entriesread].FindData.cFileName,"..");
          (*pPanelItem)[entriesread].FindData.dwFileAttributes=0;
        }
        NetApiBufferFree(members);
      }
    }
    else if(panel->level==levelRights)
    {
      *pPanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*(sizeof(AddPriveleges)/sizeof(AddPriveleges[0])));
      if(*pPanelItem)
        for(unsigned int i=0;i<sizeof(AddPriveleges)/sizeof(AddPriveleges[0]);i++)
        {
          (*pPanelItem)[i].FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
          strcpy((*pPanelItem)[*pItemsNumber].FindData.cFileName,AddPrivelegeNames[i]);
          AddDefaultUserdata((*pPanelItem)+(*pItemsNumber),levelRightUsers,0,0,NULL,AddPriveleges[i],NULL);
          (*pItemsNumber)++;
        }
    }
    else if(panel->level==levelRightUsers)
    {
      LSA_HANDLE PolicyHandle;
      PolicyHandle=GetPolicyHandle(panel->computer);
      if(PolicyHandle)
      {
        LSA_UNICODE_STRING RightName;
        LSA_ENUMERATION_INFORMATION *info; ULONG count=0;
        RightName.Buffer=panel->nonfixed;
        RightName.Length=wcslen(RightName.Buffer)*sizeof(wchar_t);
        RightName.MaximumLength=RightName.Length+sizeof(wchar_t);
        if(LsaEnumerateAccountsWithUserRight(PolicyHandle,&RightName,(PVOID *)&info,&count)==STATUS_SUCCESS)
        {
          *pPanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*count);
          if(*pPanelItem)
          {
            wchar_t *username; char *username_oem;
            PluginPanelItem *tmpItem=*pPanelItem;
            (*pItemsNumber)=count;
            for(unsigned long i=0;i<count;i++,tmpItem++)
            {
              GetUserNameEx(panel->computer_ptr,info[i].Sid,Opt.FullUserNames,&username,&username_oem);
              strcpy(tmpItem->FindData.cFileName,username_oem);
              AddDefaultUserdata(tmpItem,-1,0,0,info[i].Sid,username,NULL);
            }
          }
          LsaFreeMemory((PVOID)info);
        }
        LsaClose(PolicyHandle);
      }
    }
    return TRUE;
  }
  return FALSE;
}

void WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  for(int i=0;i<ItemsNumber;i++)
  {
    free((void *)PanelItem[i].UserData);
    free(PanelItem[i].Description);
    free(PanelItem[i].Owner);
    if(PanelItem[i].CustomColumnData)
      for(int j=0;j<CUSTOM_COLUMN_COUNT;j++)
        if(PanelItem[i].CustomColumnData[j]!=default_column_data)
          free(PanelItem[i].CustomColumnData[j]);
    free(PanelItem[i].CustomColumnData);
  }
  free(PanelItem);
  UserManager *panel=(UserManager *)hPlugin;
  if(panel->error)
  {
    panel->error=false;
    SetDirectory(hPlugin,"..",0);
    Info.Control(hPlugin,FCTL_UPDATEPANEL,(void *)1);
    Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
  }
}

int WINAPI _export SetDirectory(HANDLE hPlugin,const char *Dir,int OpMode)
{
  if(!IsOldFAR)
  {
    if(OpMode&OPM_FIND) return FALSE;
    int res=TRUE;
    UserManager *panel=(UserManager *)hPlugin;
    char buff[MAX_PATH]; wchar_t buffW[MAX_PATH];
    if(!strcmp(Dir,"\\"))
    {
      panel->level=root_dirs[panel->level];
    }
    else if(!strcmp(Dir,".."))
    {
      if(up_dirs[panel->level]==panel->level) res=FALSE;
      panel->level=up_dirs[panel->level];
    }
    else if(CheckChDir(hPlugin,Dir,buff,buffW,&panel->level))
    {
      if(nonfixed_dirs[panel->level])
      {
        strcpy(panel->nonfixed_oem,buff);
        wcscpy(panel->nonfixed,buffW);
      }
    }
    if(!has_nonfixed_dirs[panel->level])
    {
      strcpy(panel->nonfixed_oem,"");
      wcscpy(panel->nonfixed,L"");
    }
    GetCurrentPath(panel->level,panel->nonfixed_oem,panel->path);
    return res;
  }
  return FALSE;
}

void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  if(!IsOldFAR)
  {
    UserManager *panel=(UserManager *)hPlugin;
    Info->StructSize=sizeof(*Info);
    Info->Flags=OPIF_ADDDOTS|OPIF_SHOWNAMESONLY/*|OPIF_FINDFOLDERS*/;
    if((panel->level==levelUsers)||(panel->level==levelGroups))
      Info->Flags|=OPIF_USEATTRHIGHLIGHTING;
    else
      Info->Flags|=OPIF_USEHIGHLIGHTING;
    Info->HostFile=NULL;
    if(has_hostfile[panel->level])
      Info->HostFile=panel->hostfile_oem;
    Info->CurDir=panel->path;

    static char Title[TINY_BUFFER];
    switch(title_type[panel->level])
    {
      case 0:
        sprintf(Title," %s:%s\\%s ",GetMsg(title_type_string[panel->level]),FSF.PointToName(panel->hostfile_oem),panel->path);
        break;
      case 1:
        sprintf(Title," %s\\%s ",GetMsg(title_type_string[panel->level]),panel->path);
        break;
    }
    Info->PanelTitle=Title;

    static struct PanelMode PanelModesArray[10];
    static char *ColumnTitles[10][32];
    int msg_start=panel_modes[panel->level];
    char *tmp_msg;

    memset(PanelModesArray,0,sizeof(PanelModesArray));
    memset(ColumnTitles,0,sizeof(ColumnTitles));
    for(int i=0;i<10;i++)
    {
      for(int j=0;j<5;j++)
      {
        tmp_msg=GetMsg(msg_start+i*5+j);
        if(tmp_msg[0])
          switch(j)
          {
            case 0:
              PanelModesArray[i].ColumnTypes=tmp_msg;
              break;
            case 1:
              PanelModesArray[i].ColumnWidths=tmp_msg;
              break;
            case 2:
              PanelModesArray[i].StatusColumnTypes=tmp_msg;
              break;
            case 3:
              PanelModesArray[i].StatusColumnWidths=tmp_msg;
              break;
            case 4:
              PanelModesArray[i].FullScreen=tmp_msg[0]-'0';
              break;
          }
      }
      int j=0; char *scan=PanelModesArray[i].ColumnTypes;
      if(scan)
      {
        while(TRUE)
        {
          if(j==32) break;
          ColumnTitles[i][j]=get_panel_titles[panel->level](scan);
          scan=strchr(scan,',');
          if(!scan) break;
          scan++;
          if(!scan[0]) break;
          j++;
        }
        PanelModesArray[i].ColumnTitles=ColumnTitles[i];
      }
    }

    Info->PanelModesArray=PanelModesArray;
    Info->PanelModesNumber=sizeof(PanelModesArray)/sizeof(PanelModesArray[0]);
    Info->StartPanelMode='3';
    static struct KeyBarTitles KeyBar;
    memset(&KeyBar,0,sizeof(KeyBar));
    //keys
    KeyBar.Titles[3-1]="";
    if(panel->level==levelGroups)
    {
      if(panel->global)
        KeyBar.Titles[3-1]=GetMsg(mKeyLocal);
      else
        KeyBar.Titles[3-1]=GetMsg(mKeyGlobal);
    }
    else if(perm_dirs_dir[panel->level]!=PERM_NO)
    {
      KeyBar.Titles[3-1]=GetMsg(mKeyChange);
    }
    KeyBar.Titles[4-1]=GetMsg(label_f4[panel->level]);
    if(!press_f5_from[panel->level])
      KeyBar.Titles[5-1]="";
    KeyBar.Titles[6-1]=GetMsg(label_f6[panel->level]);
    KeyBar.Titles[7-1]=GetMsg(label_f7[panel->level]);
    if(!press_f8[panel->level]) KeyBar.Titles[8-1]="";
    if(press_f8[panel->level]==TakeOwnership) KeyBar.Titles[8-1]=GetMsg(mKeyTakeOwnership);
    //shift-keys
    KeyBar.ShiftTitles[1-1]="";
    KeyBar.ShiftTitles[2-1]="";
    KeyBar.ShiftTitles[3-1]="";
    KeyBar.ShiftTitles[4-1]=GetMsg(label_shift_f4[panel->level]);
    if(!press_f5_from[panel->level])
      KeyBar.ShiftTitles[5-1]="";
    KeyBar.ShiftTitles[6-1]=GetMsg(label_shift_f6[panel->level]);
    if(!press_f8[panel->level]) KeyBar.ShiftTitles[8-1]="";
    //alt-keys
    KeyBar.AltTitles[3-1]="";
    KeyBar.AltTitles[4-1]=GetMsg(label_alt_f4[panel->level]);
    KeyBar.AltTitles[5-1]="";
    KeyBar.AltTitles[6-1]="";
    //ctrl+shift-keys
    KeyBar.CtrlShiftTitles[3-1]="";
    KeyBar.CtrlShiftTitles[4-1]="";
    Info->KeyBar=&KeyBar;
  }
}

int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  UserManager *panel=(UserManager *)hPlugin;
  if((panel->error)&&(Key!=VK_RETURN)) return TRUE;
  //F4
  if((ControlState==0)&&(Key==VK_F4))
  {
    if(press_f4[panel->level])
      if(press_f4[panel->level](panel))
      {
        Info.Control(hPlugin,FCTL_UPDATEPANEL,(void *)1);
        Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
      }
    return TRUE;
  }
  //Alt-F4
  if((ControlState==PKF_ALT)&&(Key==VK_F4))
  {
    if(press_alt_f4[panel->level])
      if(press_alt_f4[panel->level](panel))
      {
        Info.Control(hPlugin,FCTL_UPDATEPANEL,(void *)1);
        Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
      }
    return TRUE;
  }
  //Shift-F4
  if((ControlState==PKF_SHIFT)&&(Key==VK_F4))
  {
    if(press_shift_f4[panel->level])
      if(press_shift_f4[panel->level](panel))
      {
        Info.Control(hPlugin,FCTL_UPDATEPANEL,(void *)1);
        Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
      }
    return TRUE;
  }
  //[Shift-]F5
  if(((ControlState==PKF_SHIFT)||(ControlState==0))&&(Key==VK_F5))
  {
    if(press_f5_from[panel->level])
    {
      UserManager *anotherpanel=NULL;
      if(panels[0]==panel) anotherpanel=panels[1];
      else if(panels[1]==panel) anotherpanel=panels[0];
      if(anotherpanel&&press_f5[anotherpanel->level])
      {
        if(press_f5[anotherpanel->level](panel,anotherpanel,ControlState!=PKF_SHIFT))
        {
          Info.Control(hPlugin,FCTL_UPDATEPANEL,NULL);
          Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
          Info.Control(hPlugin,FCTL_UPDATEANOTHERPANEL,(void *)1);
          Info.Control(hPlugin,FCTL_REDRAWANOTHERPANEL,NULL);
        }
      }
    }
    return TRUE;
  }
  //[Shift-]F8
  if(((ControlState==PKF_SHIFT)||(ControlState==0))&&(Key==VK_F8))
  {
    if(press_f8[panel->level])
      if(press_f8[panel->level](panel,ControlState!=PKF_SHIFT))
      {
        Info.Control(hPlugin,FCTL_UPDATEPANEL,(void *)1);
        Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
      }
    return TRUE;
  }
  //[Shift-]F7
  if(((ControlState==PKF_SHIFT)||(ControlState==0))&&(Key==VK_F7))
  {
    if(press_f7[panel->level])
      if(press_f7[panel->level](panel))
      {
        Info.Control(hPlugin,FCTL_UPDATEPANEL,(void *)1);
        Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
      }
    return TRUE;
  }
  //[Shift-]F6
  if(((ControlState==PKF_SHIFT)||(ControlState==0))&&(Key==VK_F6))
  {
    if(press_f6[panel->level])
      if(press_f6[panel->level](panel,ControlState!=PKF_SHIFT))
      {
        Info.Control(hPlugin,FCTL_UPDATEPANEL,(void *)1);
        Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
      }
    return TRUE;
  }
  //F3
  if((ControlState==0)&&(Key==VK_F3))
  {
    if(panel->level==levelGroups)
    {
      panel->global=!panel->global;
      if(panel->global)
      {
        wchar_t *buffer;
        HANDLE hSScr=Info.SaveScreen(0,0,-1,-1);
        const char *MsgItems[]={"",GetMsg(mOtherDomainSearch)};
        Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
        NET_API_STATUS err=NetGetAnyDCName(panel->computer_ptr,NULL,(LPBYTE *)&buffer);
        if(err==ERROR_NO_SUCH_DOMAIN)
        {
          err=NetGetDCName(panel->computer_ptr,NULL,(LPBYTE *)&buffer);
        }
        if(err==NERR_Success)
        {
          wcscpy(panel->domain,buffer);
          NetApiBufferFree(buffer);
        }
        else
        {
          panel->global=false;
        }
        Info.RestoreScreen(hSScr);
      }
      else
        wcscpy(panel->domain,L"");
      Info.Control(hPlugin,FCTL_UPDATEPANEL,NULL);
      Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
    }
    else if(perm_dirs_dir[panel->level]!=PERM_NO)
    {
      PanelInfo PInfo;
      Info.Control((HANDLE)panel,FCTL_GETPANELINFO,&PInfo);
      if((PInfo.ItemsNumber>0)&&(!(PInfo.PanelItems[PInfo.CurrentItem].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)))
      {
        if(PInfo.PanelItems[PInfo.CurrentItem].Flags&PPIF_USERDATA)
        {
          UpdateAcl(panel,panel->level,GetSidFromUserData(PInfo.PanelItems[PInfo.CurrentItem].UserData),GetItemTypeFromUserData(PInfo.PanelItems[PInfo.CurrentItem].UserData),0,actionChangeType);
        }
      }
      Info.Control(hPlugin,FCTL_UPDATEPANEL,NULL);
      Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
    }
    return TRUE;
  }
  //AltUp,AltDown
  if(ControlState==PKF_ALT&&(Key==VK_UP||Key==VK_DOWN)&&perm_dirs_dir[panel->level]!=PERM_NO)
  {
    PanelInfo PInfo;
    Info.Control((HANDLE)panel,FCTL_GETPANELINFO,&PInfo);
    if((PInfo.ItemsNumber>0)&&(!(PInfo.PanelItems[PInfo.CurrentItem].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)))
    {
      if(PInfo.PanelItems[PInfo.CurrentItem].Flags&PPIF_USERDATA)
      {
        UpdateAcl(panel,panel->level,GetSidFromUserData(PInfo.PanelItems[PInfo.CurrentItem].UserData),GetItemTypeFromUserData(PInfo.PanelItems[PInfo.CurrentItem].UserData),0,(Key==VK_UP)?actionMoveUp:actionMoveDown);
      }
    }
    Info.Control(hPlugin,FCTL_UPDATEPANEL,NULL);
    Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
    return TRUE;
  }
  return FALSE;
}

int WINAPI _export Configure(int ItemNumber)
{
  switch(ItemNumber)
  {
    case 0:
      return Config();
  }
  return FALSE;
}

int WINAPI _export GetMinFarVersion(void)
{
  return MAKEFARVERSION(1,70,1282);
}

void WINAPI ExitFAR()
{
  free_sid_cache();
  free_current_user();
}

int WINAPI _export Compare(HANDLE hPlugin,const struct PluginPanelItem *Item1,const struct PluginPanelItem *Item2,unsigned int Mode)
{
  (void)Mode;
  UserManager *panel=(UserManager *)hPlugin;
  if(sort[panel->level])
  {
    if((Item1->UserData)&&(Item2->UserData))
    {
      int res;
      res=GetSortOrderFromUserData(Item1->UserData)-GetSortOrderFromUserData(Item2->UserData);
      if(res) res=res/abs(res);
      return res;
    }
  }
  return -2;
}

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void)hDll;
  (void)dwReason;
  (void)lpReserved;
  return TRUE;
}
