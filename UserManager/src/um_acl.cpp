/*
    um_acl.cpp
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
#include "umplugin.h"
#include "memory.h"

int GetAclState(int level,unsigned char type,unsigned long flags)
{
  int result=UM_ITEM_EMPTY;
  switch(ace_types[level])
  {
    case 1:
      if(type==ACCESS_ALLOWED_ACE_TYPE) result=UM_ITEM_ALLOW;
      else if(type==ACCESS_DENIED_ACE_TYPE) result=UM_ITEM_DENY;
      break;
    case 2:
      if(type==SYSTEM_AUDIT_ACE_TYPE)
      {
        if((flags&(SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG))==(SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG)) result=UM_ITEM_SUCCESS_FAIL;
        else if(flags&SUCCESSFUL_ACCESS_ACE_FLAG) result=UM_ITEM_SUCCESS;
        else if(flags&FAILED_ACCESS_ACE_FLAG) result=UM_ITEM_FAIL;
      }
      break;
  }
  return result;
}

bool GetAcl(UserManager *panel,int level,AclData **data)
{
  *data=NULL; bool res=false;
  PSECURITY_DESCRIPTOR SD=NULL;
  SD=get_security_descriptor[level](panel,security_information[level]);
  if(SD)
  {
    *data=(AclData *)malloc(sizeof(AclData));
    if(*data)
    {
      BOOL Present=FALSE,Defaulted; PACL acl;
      if(get_security_descriptor_acl[security_information[level]](SD,&Present,&acl,&Defaulted))
      {
        bool free_acl=false;
        if(Present&&(!acl)) { acl=CreateDefaultAcl(level); free_acl=true; }
        if(Present&&acl)
        {
          ACL_SIZE_INFORMATION size_info;
          if(GetAclInformation(acl,&size_info,sizeof(size_info),AclSizeInformation))
          {
            res=true;
            for(unsigned int i=0;i<size_info.AceCount;i++,res=true)
            {
              res=false;
              ACCESS_ALLOWED_ACE *Ace;
              if(GetAce(acl,i,(void **)&Ace))
              {
                unsigned char Flags=Ace->Header.AceFlags; unsigned long Mask=Ace->Mask;
                if(check_ace_type[level](check_ace_type_param[level],Ace->Header.AceType,&Flags,&Mask))
                {
                  PSID sid=&(Ace->SidStart);
                  if(IsValidSid(sid))
                  {
                    AceData *currAceData,**lastAceData;
                    currAceData=(*data)->Aces;
                    lastAceData=&((*data)->Aces);
                    while(currAceData)
                    {
                      if(EqualSid(sid,currAceData->user)&&GetAclState(level,Ace->Header.AceType,Ace->Header.AceFlags)==GetAclState(level,currAceData->ace_type,currAceData->ace_flags))
                        break;
                      lastAceData=&(currAceData->next);
                      currAceData=currAceData->next;
                    }
                    if(!currAceData)
                    {
                      currAceData=(AceData *)malloc(sizeof(AceData)+GetLengthSid(sid));
                      if(!currAceData) break;
                      currAceData->length=sizeof(AceData)+GetLengthSid(sid);
                      currAceData->next=NULL;
                      //currAceData->next=(*data)->Aces;
                      *lastAceData=currAceData;
                      //(*data)->Aces=currAceData;
                      currAceData->user=(PSID)&(currAceData[1]);
                      CopySid(currAceData->length-sizeof(AceData),currAceData->user,sid);
                      currAceData->ace_mask=Mask;
                      currAceData->ace_flags=Flags;
                      currAceData->ace_type=Ace->Header.AceType;
                      ((*data)->Count)++;
                    }
                  } // IsValidSid
                } //CheckAceType
              } //GetAce
              else break;
            } //for
          } //GetAclInformation
        } //Present&&acl
        else res=true;
        if(free_acl) free(acl);
      } //get_security_descriptor_acl
    } // *data
    free(SD);
  } //get_security_descriptor
  if((!res)&&(*data)) FreeAcl(*data);
  return res;
}

void FreeAcl(AclData *data)
{
  if(!data) return;
  AceData *curr=data->Aces,*tmp;
  while(curr)
  {
    tmp=curr;
    curr=curr->next;
    free(tmp);
  }
  free(data);
}

bool UpdateAcl(UserManager *panel,int level,PSID user,int type,DWORD mask,int action)
{
  AclData *data=NULL,*data_add=NULL;
  bool err=false;
  if(GetAcl(panel,level,&data))
  {
    AceData *tmpAce=data->Aces;
    AceData **tmpAcePtr=&(data->Aces),**tmpAcePtr2=NULL;
    switch(action)
    {
      case actionUpdate:
      case actionChangeType:
        while(tmpAce)
        {
          if(EqualSid(user,tmpAce->user)&&type==GetAclState(level,tmpAce->ace_type,tmpAce->ace_flags))
            break;
          tmpAce=tmpAce->next;
        }
        if(!tmpAce) { err=true; break; }
        if(action==actionUpdate) tmpAce->ace_mask=mask;
        else if(action==actionChangeType)
        {
          switch(type)
          {
            case UM_ITEM_ALLOW:
              tmpAce->ace_type=ACCESS_DENIED_ACE_TYPE;
              break;
            case UM_ITEM_DENY:
              tmpAce->ace_type=ACCESS_ALLOWED_ACE_TYPE;
              break;
            case UM_ITEM_SUCCESS:
              tmpAce->ace_flags&=~(SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG);
              tmpAce->ace_flags|=FAILED_ACCESS_ACE_FLAG;
              break;
            case UM_ITEM_FAIL:
              tmpAce->ace_flags|=(SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG);
              break;
            case UM_ITEM_SUCCESS_FAIL:
              tmpAce->ace_flags&=~(SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG);
              tmpAce->ace_flags|=SUCCESSFUL_ACCESS_ACE_FLAG;
              break;
          }
        }
        break;
      case actionInsert:
        tmpAce=(AceData *)malloc(sizeof(AceData)+GetLengthSid(user));
        tmpAce->length=sizeof(AceData)+GetLengthSid(user);
        tmpAce->ace_mask=mask;
        tmpAce->ace_type=default_type[level];
        tmpAce->ace_flags=default_flags[level];
        tmpAce->user=(PSID)&tmpAce[1];
        CopySid(tmpAce->length-sizeof(AceData),tmpAce->user,user);
        tmpAce->next=(*tmpAcePtr);
        (*tmpAcePtr)=tmpAce;
        break;
      case actionDelete:
        err=true;
        while(tmpAce)
        {
          if(EqualSid(user,tmpAce->user)&&type==GetAclState(level,tmpAce->ace_type,tmpAce->ace_flags))
          {
            err=false;
            (*tmpAcePtr)=tmpAce->next;
            free(tmpAce);
            break;
          }
          tmpAce=tmpAce->next;
          tmpAcePtr=&((*tmpAcePtr)->next);
        }
        break;
      case actionMoveUp:
        err=true;
        while(tmpAce)
        {
          if(EqualSid(user,tmpAce->user)&&type==GetAclState(level,tmpAce->ace_type,tmpAce->ace_flags))
          {
            if(tmpAcePtr2)
            {
              AceData *tmpAce2=*tmpAcePtr2;
              *tmpAcePtr2=tmpAce;
              tmpAce2->next=tmpAce->next;
              tmpAce->next=tmpAce2;
            }
            err=false;
            break;
          }
          tmpAcePtr2=tmpAcePtr;
          tmpAce=tmpAce->next;
          tmpAcePtr=&((*tmpAcePtr)->next);
        }
        break;
      case actionMoveDown:
        err=true;
        while(tmpAce)
        {
          if(EqualSid(user,tmpAce->user)&&type==GetAclState(level,tmpAce->ace_type,tmpAce->ace_flags))
          {
            if(tmpAce->next)
            {
              *tmpAcePtr=tmpAce->next;
              tmpAce->next=tmpAce->next->next;
              (*tmpAcePtr)->next=tmpAce;
            }
            err=false;
            break;
          }
          tmpAce=tmpAce->next;
          tmpAcePtr=&((*tmpAcePtr)->next);
        }
        break;
    }
    if(!err)
    {
      for(int i=0;i<5;i++)
      {
        if(relative_types[level][i]!=-1)
        {
          if(GetAcl(panel,relative_types[level][i],&data_add))
          {
            data->Count+=data_add->Count;
            tmpAcePtr=&(data->Aces);
            while(*tmpAcePtr)
              tmpAcePtr=&((*tmpAcePtr)->next);
            (*tmpAcePtr)=data_add->Aces;
            free(data_add);
          }
          else { err=true; break; }
        }
      }

      unsigned int acl_size=sizeof(ACL);
      tmpAce=data->Aces;
      while(tmpAce)
      {
        acl_size+=tmpAce->length-sizeof(AceData)+sizeof(ACCESS_ALLOWED_ACE)-sizeof(DWORD);
        tmpAce=tmpAce->next;
      }
      PACL acl=NULL; ACCESS_ALLOWED_ACE *last_ace;
      acl=(PACL)malloc(acl_size);
      if(acl)
      {
        if(InitializeAcl(acl,acl_size,ACL_REVISION))
        {
          tmpAce=data->Aces;
          int AceIndex=0;
          while(tmpAce)
          {
            switch(tmpAce->ace_type)
            {
              case ACCESS_ALLOWED_ACE_TYPE:
                if(!AddAccessAllowedAce(acl,ACL_REVISION,tmpAce->ace_mask,tmpAce->user)) err=true;
                break;
              case ACCESS_DENIED_ACE_TYPE:
                if(!AddAccessDeniedAce(acl,ACL_REVISION,tmpAce->ace_mask,tmpAce->user)) err=true;
                break;
              case SYSTEM_AUDIT_ACE_TYPE:
                if(!AddAuditAccessAce(acl,ACL_REVISION,tmpAce->ace_mask,tmpAce->user,(tmpAce->ace_flags&SUCCESSFUL_ACCESS_ACE_FLAG),(tmpAce->ace_flags&FAILED_ACCESS_ACE_FLAG))) err=true;
                break;
            }
            if(err) break;

            if(!GetAce(acl,AceIndex,(void **)&last_ace)) { err=true; break; }
            last_ace->Header.AceFlags=tmpAce->ace_flags;
            AceIndex++;
            tmpAce=tmpAce->next;
          }
          if(!err)
          {
            err=true;
            PSECURITY_DESCRIPTOR SD=NULL;
            SD=get_security_descriptor[level](panel,security_information[level]);
            if(SD)
            {
              char *ASD_full=NULL; PSECURITY_DESCRIPTOR ASD,RealSD=NULL; DWORD ASD_size=0;
              PACL dacl,sacl; PSID owner,group; DWORD dacl_size=0,sacl_size=0,owner_size=0,group_size=0;
              if(!MakeAbsoluteSD(SD,NULL,&ASD_size,NULL,&dacl_size,NULL,&sacl_size,NULL,&owner_size,NULL,&group_size))
              {
                if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
                {
                  ASD_full=(char *)malloc(ASD_size+dacl_size+sacl_size+owner_size+group_size);
                  if(ASD_full)
                  {
                    ASD=(PSECURITY_DESCRIPTOR)ASD_full; ASD_full+=ASD_size;
                    dacl=(PACL)ASD_full; ASD_full+=dacl_size;
                    sacl=(PACL)ASD_full; ASD_full+=sacl_size;
                    owner=(PSID)ASD_full; ASD_full+=owner_size;
                    group=(PSID)ASD_full;
                    if(MakeAbsoluteSD(SD,ASD,&ASD_size,dacl,&dacl_size,sacl,&sacl_size,owner,&owner_size,group,&group_size))
                      RealSD=ASD;
                  } //ASD_full
                }
                else if(GetLastError()==ERROR_BAD_DESCRIPTOR_FORMAT)
                  RealSD=SD;
                if(RealSD)
                  if(set_security_descriptor_acl[security_information[level]](RealSD,TRUE,acl,FALSE))
                    if(set_security_descriptor[level](panel,security_information[level],RealSD)) err=false;
              } //MakeAbsoluteSD
              free(ASD_full);
              free(SD);
            } //GetFileSecurity
          }
        }
        free(acl);
      }
    }
    FreeAcl(data);
  }
  return !err;
}
