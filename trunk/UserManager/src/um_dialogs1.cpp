/*
    um_dialogs1.cpp
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

/*FIXME: in future make one universal dialog*/
#include <stdio.h>
#include <stdlib.h>
#include "far_helper.h"
#include "farkeys.hpp"
#include "umplugin.h"
#include "memory.h"
#include "guid.h"

static INT_PTR WINAPI EditAdvancedAccessDialogProc(HANDLE hDlg,int Msg,int Param1,INT_PTR Param2)
{
  int *DlgParams=(int *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_CONTROLINPUT:
      {
        const INPUT_RECORD* record=(const INPUT_RECORD*)Param2;
        if(record->EventType==KEY_EVENT)
        {
          int key=FSF.FarInputRecordToKey(record);
          if((key==KEY_SHIFTADD)||(key==KEY_SHIFTSUBTRACT))
          {
            int state=(key==KEY_SHIFTADD);
            FarDialogItem DialogItem;
            for(int i=0;i<DlgParams[1];i++)
            {
              Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,DlgParams[0]+i,(long)&DialogItem);
              DialogItem.Selected=state;
              Info.SendDlgMessage(hDlg,DM_SETDLGITEMSHORT,DlgParams[0]+i,(long)&DialogItem);
            }
          }
        }
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

static bool EditAdvancedAccess(UserManager *panel,int size,int *messages,unsigned int *access,int delimiter)
{
  bool res=false;
  CFarPanel pInfo((HANDLE)panel,FCTL_GETPANELINFO);
  if(pInfo.IsOk())
  {
    if((pInfo.ItemsNumber()>0)&&(!(pInfo[pInfo.CurrentItem()].FileAttributes&FILE_ATTRIBUTE_DIRECTORY)))
    {
      if(pInfo[pInfo.CurrentItem()].Flags&PPIF_USERDATA)
      {
        unsigned long mask=GetLevelFromUserData(pInfo[pInfo.CurrentItem()].UserData);
        //Show dialog
        /*
          0000000000111111111122222222223333333333444444444455555555556666666666777777
          0123456789012345678901234567890123456789012345678901234567890123456789012345
        00                                                                            00
        01   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»   01
        02   º [ ] Read data/List directory                                       º   02
        03   º [ ] Write data/Add file                                            º   03
        04   º [ ] Append data/Add subdirectory                                   º   04
        05   º [ ] Read extended attributes/Read properties                       º   05
        06   º [ ] Write extended attributes/Write properties                     º   06
        07   º [ ] Execute/Traverce???                                            º   07
        08   º [ ] Delete Child                                                   º   08
        09   º [ ] Read attributes                                                º   09
        10   º [ ] Write attributes                                               º   10
        11   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   11
        12   º [ ] Delete                                                         º   12
        13   º [ ] Read owner, group, dacl                                        º   13
        14   º [ ] Change Permissions                                             º   14
        15   º [ ] Take Ownership                                                 º   15
        16   º [ ] Synchronize                                                    º   16
        17   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   17
        18   º                 [ OK ]                [ Cancel ]                   º   18
        19   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   19
        20                                                                            20
          0000000000111111111122222222223333333333444444444455555555556666666666777777
          0123456789012345678901234567890123456789012345678901234567890123456789012345
        */
        int ItemCount=4+size+((delimiter>=0)?1:0);
        FarDialogItem *DialogItems=(FarDialogItem *)malloc(sizeof(FarDialogItem)*ItemCount);
        if(DialogItems)
        {
          int button_index=ItemCount-2,check_index=button_index-size;
          DialogItems[0].Type=DI_DOUBLEBOX; DialogItems[0].X1=3; DialogItems[0].Y1=1; DialogItems[0].X2=72; DialogItems[0].Y2=ItemCount;
          DialogItems[1].Type=DI_TEXT; DialogItems[1].X1=-1; DialogItems[1].Y1=ItemCount-2; DialogItems[1].Flags=DIF_SEPARATOR;
          if(delimiter>=0)
            DialogItems[2].Type=DI_TEXT; DialogItems[2].X1=-1; DialogItems[2].Y1=3+delimiter; DialogItems[2].Flags=DIF_SEPARATOR;

          DialogItems[button_index].Type=DI_BUTTON; DialogItems[button_index].Y1=ItemCount-1; DialogItems[button_index].Flags=DIF_CENTERGROUP|DIF_DEFAULTBUTTON; INIT_DLG_DATA(DialogItems[button_index],GetMsg(mPropButtonOk));
          button_index++;
          DialogItems[button_index].Type=DI_BUTTON; DialogItems[button_index].Y1=ItemCount-1; DialogItems[button_index].Flags=DIF_CENTERGROUP; INIT_DLG_DATA(DialogItems[button_index],GetMsg(mPropButtonCancel));
          button_index--;
          for(int i=0;i<size;i++)
          {
            DialogItems[check_index+i].Type=DI_CHECKBOX;
            DialogItems[check_index+i].X1=5;
            DialogItems[check_index+i].Y1=2+i+((i>delimiter&&delimiter>=0)?1:0);
            if(!i) DialogItems[check_index+i].Flags|=DIF_FOCUS;
            INIT_DLG_DATA(DialogItems[check_index+i],GetMsg(messages[i]));
            if(mask&access[i]) DialogItems[check_index+i].Selected=TRUE;
          }
          int params[2]={check_index,size};
          CFarDialog dialog;
          int DlgCode=dialog.Execute(MainGuid,EditAdvancedAccessGuid,-1,-1,76,ItemCount+2,_T("EditAdvancedAccess"),DialogItems,ItemCount,0,0,EditAdvancedAccessDialogProc,(long)params);
          if(DlgCode==button_index)
          {
            mask=0;
            for(int i=0;i<size;i++)
            {
              if(dialog.Check(check_index+i))
                mask|=access[i];
            }
            if(UpdateAcl(panel,panel->level,GetSidFromUserData(pInfo[pInfo.CurrentItem()].UserData),GetItemTypeFromUserData(pInfo[pInfo.CurrentItem()].UserData),mask,actionUpdate))
              res=true;
          }
          free(DialogItems);
        }
      }
    }
  }
  return res;
}

bool EditFileAdvancedAccess(UserManager *panel)
{
  const int size=14;
  int messages[size];
  int MsgIdx=mPropFileReadData;
  if(panel->flags&FLAG_FOLDER) MsgIdx=mPropDirReadData;
  for(int i=0;i<size;i++) messages[i]=MsgIdx+i;
  unsigned int access[]=
  {
    FILE_READ_DATA,FILE_WRITE_DATA,FILE_APPEND_DATA,FILE_READ_EA,FILE_WRITE_EA,FILE_EXECUTE,
    FILE_DELETE_CHILD,FILE_READ_ATTRIBUTES,FILE_WRITE_ATTRIBUTES,DELETE,READ_CONTROL,
    WRITE_DAC,WRITE_OWNER,SYNCHRONIZE,
  };
  return EditAdvancedAccess(panel,size,messages,access,8);
}

bool EditRegAdvancedAccess(UserManager *panel)
{
  const int size=10;
  int messages[]=
  {
    mPropRegQueryValue,mPropRegSetValue,mPropRegCreateSubkey,mPropRegEnumerateSubkeys,mPropRegNotify,mPropRegCreateLink,
    mPropRegDelete,mPropRegReadControl,mPropRegChangePermissions,mPropRegTakeOwnership,
  };
  unsigned int access[]=
  {
    KEY_QUERY_VALUE,KEY_SET_VALUE,KEY_CREATE_SUB_KEY,KEY_ENUMERATE_SUB_KEYS,
    KEY_NOTIFY,KEY_CREATE_LINK,
    DELETE,READ_CONTROL,WRITE_DAC,WRITE_OWNER,
  };
  return EditAdvancedAccess(panel,size,messages,access,-1);
}

bool EditPrinterAdvancedAccess(UserManager *panel)
{
  const int size=6;
  int messages[]=
  {
    mPropPrintUse,mPropPrintAdminister,mPropRegDelete,mPropRegReadControl,mPropRegChangePermissions,mPropRegTakeOwnership,
  };
  unsigned int access[]=
  {
    PRINTER_ACCESS_USE,PRINTER_ACCESS_ADMINISTER,
    DELETE,READ_CONTROL,WRITE_DAC,WRITE_OWNER,
  };
  return EditAdvancedAccess(panel,size,messages,access,-1);
}

bool EditJobAdvancedAccess(UserManager *panel)
{
  const int size=5;
  int messages[]=
  {
    mPropJobAdminister,mPropRegDelete,mPropRegReadControl,mPropRegChangePermissions,mPropRegTakeOwnership,
  };
  unsigned int access[]=
  {
    JOB_ACCESS_ADMINISTER,
    DELETE,READ_CONTROL,WRITE_DAC,WRITE_OWNER,
  };
  return EditAdvancedAccess(panel,size,messages,access,-1);
}
