/*
    um_childs.cpp
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
#include "farkeys.hpp"
#include "umplugin.h"
#include "memory.h"

struct A2CData
{
  int Obj[2];
  int Perm[2];
  AclData *ObjPerm[4];
  int Owner;
  PSID sid;
};

static int levels[3][2]=
{
  {levelFilePermNewFolder,levelFileAuditFolder},
  {levelFilePermNewFile,levelFileAuditFile},
  {levelRegRightsNewKey,levelRegAuditKey},
};

static int root_levels[3]=
{
  levelRoot,levelRoot,levelRegRoot,
};

static int shifts[3]={0,2,0};
static int flags[2][3]=
{
  {CONTAINER_INHERIT_ACE,0,CONTAINER_INHERIT_ACE},
  {OBJECT_INHERIT_ACE,0,0},
};

enum
{
  typeFolder,
  typeFile,
  typeRegistry,
};

static void SetAccess(UserManager *panel,AclData **ObjPerm,int type)
{
  unsigned int acl_size[2]={sizeof(ACL),sizeof(ACL)};
  AceData *tmpAce;
  unsigned long err=0;
  for(int i=0;i<2;i++)
  {
    for(int j=0;j<2;j++)
    {
      if(ObjPerm[i*2+j])
      {
        tmpAce=ObjPerm[i*2+j]->Aces;
        while(tmpAce)
        {
          acl_size[j]+=tmpAce->length-sizeof(AceData)+sizeof(ACCESS_ALLOWED_ACE)-sizeof(DWORD);
          tmpAce=tmpAce->next;
        }
      }
    }
  }
  PACL acl[2]={NULL,NULL}; ACCESS_ALLOWED_ACE *last_ace;
  bool flag;
  flag=true;
  for(int i=0;i<2;i++)
  {
    acl[i]=(PACL)malloc(acl_size[i]);
    if(!acl[i]) flag=false;
  }
  if(flag)
  {
    for(int i=0;i<2;i++)
    {
      if(InitializeAcl(acl[i],acl_size[i],ACL_REVISION))
      {
        int AceIndex=0;
        for(int j=0;j<2;j++)
        {
          if(ObjPerm[j*2+i])
          {
            tmpAce=ObjPerm[j*2+i]->Aces;
            while(tmpAce)
            {
              switch(tmpAce->ace_type)
              {
                case ACCESS_ALLOWED_ACE_TYPE:
                  if(!AddAccessAllowedAce(acl[i],ACL_REVISION,tmpAce->ace_mask,tmpAce->user)) err=GetLastError();
                  break;
                case ACCESS_DENIED_ACE_TYPE:
                  if(!AddAccessDeniedAce(acl[i],ACL_REVISION,tmpAce->ace_mask,tmpAce->user)) err=GetLastError();
                  break;
                case SYSTEM_AUDIT_ACE_TYPE:
                  if(!AddAuditAccessAce(acl[i],ACL_REVISION,tmpAce->ace_mask,tmpAce->user,(tmpAce->ace_flags&SUCCESSFUL_ACCESS_ACE_FLAG),(tmpAce->ace_flags&FAILED_ACCESS_ACE_FLAG))) err=GetLastError();
                  break;
              }
              if(err) break;

              if(!GetAce(acl[i],AceIndex,(void **)&last_ace)) { err=GetLastError(); break; }
              last_ace->Header.AceFlags|=flags[j][type];
              AceIndex++;
              tmpAce=tmpAce->next;
            }
          }
        }
      }
    }
    PSECURITY_DESCRIPTOR SD;
    SD=(PSECURITY_DESCRIPTOR)malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
    if(SD)
    {
      if(InitializeSecurityDescriptor(SD,SECURITY_DESCRIPTOR_REVISION))
      {
        SetSecurityDescriptorDacl(SD,TRUE,acl[0],FALSE);
        SetSecurityDescriptorSacl(SD,TRUE,acl[1],FALSE);
        if(type==typeRegistry)
        {
          HKEY hKey;
          if((RegOpenBackupKeyExW((HKEY)panel->param,panel->hostfile,WRITE_DAC|ACCESS_SYSTEM_SECURITY,&hKey))==ERROR_SUCCESS)
          {
            RegSetKeySecurity(hKey,DACL_SECURITY_INFORMATION|SACL_SECURITY_INFORMATION,SD);
            RegCloseKey(hKey);
          }
        }
        else
          SetFileSecurityW(panel->hostfile,DACL_SECURITY_INFORMATION|SACL_SECURITY_INFORMATION,SD);
      }
      free(SD);
    }
  }
  for(int i=0;i<2;i++)
    free(acl[i]);
}

static void SetAccessMain(unsigned long param,wchar_t *dir,wchar_t *file,int type,A2CData *td)
{
  UserManager panel;
  memset(&panel,0,sizeof(panel)); //FIXME?
  panel.level=root_levels[type];
  wcscpy(panel.hostfile,dir);
  wcsaddendslash(panel.hostfile);
  wcscat(panel.hostfile,file);
  panel.param=param;
  AclData *ObjPerm[4]={NULL,NULL,NULL,NULL};
  for(int i=0;i<2;i++)
  {
    if(td->Perm[i])
      ObjPerm[i]=td->ObjPerm[i+shifts[type]];
    else
      GetAcl(&panel,levels[type][i],&ObjPerm[i]);
  }
  if(type==typeFolder)
  {
    for(int i=0;i<2;i++)
    {
      if(td->Perm[i])
        ObjPerm[i+2]=td->ObjPerm[i+2];
      else
        GetAcl(&panel,levels[typeFile][i],&ObjPerm[i+2]);
    }
  }
  SetAccess(&panel,ObjPerm,type);
  for(int i=0;i<2;i++)
    for(int j=0;j<2;j++)
      if(!(td->Perm[j]))
      {
        FreeAcl(ObjPerm[i*2+j]);
        ObjPerm[i*2+j]=NULL;
      }
  if(td->Owner)
  {
    wchar_t *username; TCHAR *username_oem;
    GetUserNameEx(NULL,td->sid,true,&username,&username_oem);
    AddOwnerInternal(&panel,td->sid);
  }
}

static void RecurceRegAcl(unsigned long param,wchar_t *dir,A2CData *td)
{
  HKEY hKey=NULL;
  if(RegOpenBackupKeyExW((HKEY)param,dir,KEY_READ,&hKey)==ERROR_SUCCESS)
  {
    wchar_t NameBuffer[2048]; LONG Result;
    for(int i=0;;i++)
    {
      Result=RegEnumKeyW(hKey,i,NameBuffer,sizeof(NameBuffer)/sizeof(NameBuffer[0]));
      if(Result==ERROR_NO_MORE_ITEMS)
        break;
      if(Result==ERROR_SUCCESS)
      {
        wchar_t newdir[8*1024];
        wcscpy(newdir,dir);
        wcsaddendslash(newdir);
        wcscat(newdir,NameBuffer);
        RecurceRegAcl(param,newdir,td);
        SetAccessMain(param,dir,NameBuffer,typeRegistry,td);
      }
    }
    RegCloseKey(hKey);
  }
}

static void RecurceAcl(wchar_t *dir,A2CData *td)
{
  wchar_t mask[MAX_PATH];
  wcscpy(mask,dir);
  wcsaddendslash(mask);
  wcscat(mask,L"*");
  WIN32_FIND_DATAW find;
  HANDLE hFind=FindFirstFileW(mask,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    do
    {
      if(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
      {
        if(wcscmp(find.cFileName,L".")&&wcscmp(find.cFileName,L".."))
        {
          wchar_t new_dir[MAX_PATH];
          wcscpy(new_dir,dir);
          wcsaddendslash(new_dir);
          wcscat(new_dir,find.cFileName);
          RecurceAcl(new_dir,td);
          if(td->Obj[0])
            SetAccessMain(0,dir,find.cFileName,typeFolder,td);
        }
      }
      else
      {
        if(td->Obj[1])
          SetAccessMain(0,dir,find.cFileName,typeFile,td);
      }
    } while(FindNextFileW(hFind,&find));
    FindClose(hFind);
  }
}

long WINAPI ProcessChildsDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void ProcessChilds(CFarPanel& pInfo)
{
  int FolderCount=0; const TCHAR *FolderName=_T("");
  for(int i=0;i<pInfo.SelectedItemsNumber();i++)
    if(pInfo.Selected(i).FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
    {
      FolderCount++;
      FolderName=pInfo.Selected(i).FindData.PANEL_FILENAME;
    }
  if(FolderCount)
  {
    bool registry=false;
    //detect registry
    if(pInfo.Plugin())
    {
      unsigned long param; wchar_t path[MAX_PATH];
      int level=parse_dir(pInfo.CurDir(),pInfo.Selected(0).FindData.PANEL_FILENAME,NULL,pathtypePlugin,&param,path,NULL);
      if(level==levelRegRoot)
        registry=true;
      else
        return;
    }
    enum
    {
      CHILDS_BORDER=0,
      CHILDS_LABEL,
      CHILDS_OBJECTS,
      CHILDS_FOLDERS,
      CHILDS_FILES,
      CHILDS_KEYS,
      CHILDS_ACL,
      CHILDS_RIGHTS,
      CHILDS_AUDIT,
      CHILDS_OWNER,
      CHILDS_OK,
      CHILDS_CANCEL
    };
    /*
      0000000000111111111122222222223333333333444444444455555555556666666666777777
      0123456789012345678901234567890123456789012345678901234567890123456789012345
    00                                                                            00
    01   ��������������� Process childs �������������������������������������ͻ   01
    02   � Process 2 folders                                                  �   02
    03   � �� Objects �����������������������������������������������������Ŀ �   03
    04   � � [ ] Folders                                                    � �   04
    05   � � [ ] Files                                                      � �   05
    06   � � [ ] Keys                                                       � �   06
    07   � ������������������������������������������������������������������ �   07
    08   � �� ACL types ���������������������������������������������������Ŀ �   08
    09   � � [ ] Rights                                                     � �   09
    10   � � [ ] Audit                                                      � �   10
    11   � ������������������������������������������������������������������ �   11
    12   �                 [ OK ]                [ Cancel ]                   �   12
    13   ��������������������������������������������������������������������ͼ   13
    14                                                                            14
      0000000000111111111122222222223333333333444444444455555555556666666666777777
      0123456789012345678901234567890123456789012345678901234567890123456789012345
    */
    static struct InitDialogItem InitItems[]={
    /* 0*/  {DI_DOUBLEBOX,3,1,72,14,0,0,0,0,(TCHAR *)mA2CTitle},
    /* 1*/  {DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
    /* 2*/  {DI_SINGLEBOX,5,3,70,7,0,0,DIF_LEFTTEXT,0,(TCHAR *)mA2CObjects},
    /* 3*/  {DI_CHECKBOX,7,4,0,0,0,1,0,0,(TCHAR *)mA2CFolders},
    /* 4*/  {DI_CHECKBOX,7,5,0,0,0,1,0,0,(TCHAR *)mA2CFiles},
    /* 5*/  {DI_CHECKBOX,7,6,0,0,0,1,0,0,(TCHAR *)mA2CKeys},
    /* 6*/  {DI_SINGLEBOX,5,8,70,12,0,0,DIF_LEFTTEXT,0,(TCHAR *)mA2CACL},
    /* 7*/  {DI_CHECKBOX,7, 9,0,0,0,1,0,0,(TCHAR *)mA2CRights},
    /* 8*/  {DI_CHECKBOX,7,10,0,0,0,1,0,0,(TCHAR *)mA2CAudit},
    /* 9*/  {DI_CHECKBOX,7,11,0,0,0,1,0,0,(TCHAR *)mA2COwner},
    /*10*/  {DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)mPropButtonOk},
    /*11*/  {DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)mPropButtonCancel}
    };
    struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
    InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
    TCHAR childs_label[512];
    if(FolderCount>1)
      FSF.sprintf(childs_label,GetMsg(mA2CProcessN+NumberType(FolderCount)),FolderCount);
    else
    {
      TCHAR Truncated[MAX_PATH];
      FSF.sprintf(Truncated,_T("%s"),FolderName);
      FSF.TruncPathStr(Truncated,50);
      FSF.sprintf(childs_label,GetMsg(mA2CProcessOne),Truncated);
    }
    INIT_DLG_DATA(DialogItems[CHILDS_LABEL],childs_label);
    if(registry)
    {
      DialogItems[CHILDS_FOLDERS].Flags|=DIF_DISABLE;
      DialogItems[CHILDS_FILES].Flags|=DIF_DISABLE;
      DialogItems[CHILDS_KEYS].Focus=TRUE;
    }
    else
    {
      DialogItems[CHILDS_FOLDERS].Focus=TRUE;
      DialogItems[CHILDS_KEYS].Flags|=DIF_DISABLE;
    }
    CFarDialog dialog;
    int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,76,16,_T("ProcessChildren"),DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,ProcessChildsDialogProc,0);
    if(DlgCode==CHILDS_OK)
    {
      A2CData td; bool process=false,access=false;
      if(registry)
      {
        td.Obj[0]=dialog.Check(CHILDS_KEYS);
        process=td.Obj[0];
      }
      else
      {
        for(int i=0;i<2;i++)
        {
          td.Obj[i]=dialog.Check(i+CHILDS_FOLDERS);
          process=process||td.Obj[i];
        }
      }
      for(int i=0;i<2;i++)
      {
        td.Perm[i]=dialog.Check(i+CHILDS_RIGHTS);
        access=access||td.Perm[i];
      }
      td.Owner=dialog.Check(CHILDS_OWNER);
      access=access||td.Owner;
      if(process)
      {
        UserManager panel;
        memset(&panel,0,sizeof(panel)); //FIXME?
        for(int i=0;i<pInfo.SelectedItemsNumber();i++)
          if(pInfo.Selected(i).FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
          {
            for(int j=0;j<4;j++)
              td.ObjPerm[j]=NULL;
            td.sid=NULL;
            if(registry)
            {
              panel.level=parse_dir(pInfo.CurDir(),pInfo.Selected(i).FindData.PANEL_FILENAME,NULL,pathtypePlugin,&panel.param,panel.hostfile,NULL);
              if(panel.level==levelRegRoot)
              {
                if(td.Obj[0])
                  for(int k=0;k<2;k++)
                    if(td.Perm[k])
                    {
                      if(!GetAcl(&panel,levels[2][k],&td.ObjPerm[k])) goto error;
                    }
                if(td.Owner&&plain_dirs_owners[panel.level])
                {
                  wchar_t *owner;
                  TCHAR *owner_oem;
                  PSID sid;
                  if(plain_dirs_owners[panel.level](&panel,&sid,&owner,&owner_oem))
                  {
                    td.sid=(PSID)malloc(GetLengthSid(sid));
                    if(td.sid) CopySid(GetLengthSid(sid),td.sid,sid);
                  }
                }
                RecurceRegAcl(panel.param,panel.hostfile,&td);
                for(int l=0;l<4;l++)
                {
                  FreeAcl(td.ObjPerm[l]);
                  td.ObjPerm[l]=NULL;
                }
                free(td.sid);
                td.sid=NULL;
              }
            }
            else
            {
              wchar_t filename_w[MAX_PATH];
              if(GetWideName(pInfo.CurDir(),&pInfo.Selected(i).FindData,filename_w))
              {
                panel.level=parse_dir(pInfo.CurDir(),pInfo.Selected(i).FindData.PANEL_FILENAME,filename_w,pathtypeReal,&panel.param,panel.hostfile,NULL);
                if(panel.level==levelRoot)
                {
                  for(int j=0;j<2;j++)
                  {
                    if(td.Obj[j])
                      for(int k=0;k<2;k++)
                        if(td.Perm[k])
                        {
                          if(!GetAcl(&panel,levels[j][k],&td.ObjPerm[k+j*2])) goto error;
                        }
                  }
                  if(td.Owner&&plain_dirs_owners[panel.level])
                  {
                    wchar_t *owner;
                    TCHAR *owner_oem;
                    PSID sid;
                    if(plain_dirs_owners[panel.level](&panel,&sid,&owner,&owner_oem))
                    {
                      td.sid=(PSID)malloc(GetLengthSid(sid));
                      if(td.sid) CopySid(GetLengthSid(sid),td.sid,sid);
                    }
                  }
                  RecurceAcl(panel.hostfile,&td);
                  for(int l=0;l<4;l++)
                  {
                    FreeAcl(td.ObjPerm[l]);
                    td.ObjPerm[l]=NULL;
                  }
                  free(td.sid);
                  td.sid=NULL;
                }
              }
            }
          }
error:
        for(int i=0;i<4;i++)
        {
          FreeAcl(td.ObjPerm[i]);
          td.ObjPerm[i]=NULL;
        }
        free(td.sid);
        td.sid=NULL;
      }
    }
  }
  else ShowCustomError(mA2CNoFolder);
}