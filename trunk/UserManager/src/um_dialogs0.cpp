/*
    um_dialogs0.cpp
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
#include "umplugin.h"
#include "memory.h"
#include "guid.h"

static INT_PTR WINAPI EditCommonAccessDialogProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
  FarDialogItem DialogItem;
  switch(Msg)
  {
    case DN_INITDIALOG:
    case DN_BTNCLICK:
      Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,2,&DialogItem);
      if(DialogItem.Selected)
      {
        for(int i=4;i<10;i++)
        {
          Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,i,&DialogItem);
          DialogItem.Selected=FALSE;
          DialogItem.Flags|=DIF_DISABLE;
          Info.SendDlgMessage(hDlg,DM_SETDLGITEMSHORT,i,&DialogItem);
        }
      }
      else
      {
        for(int i=4;i<10;i++)
        {
          Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,i,&DialogItem);
          DialogItem.Flags&=~DIF_DISABLE;
          Info.SendDlgMessage(hDlg,DM_SETDLGITEMSHORT,i,&DialogItem);
        }
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool EditCommonAccess(UserManager *panel)
{
  bool res=false;
  CFarPanel pInfo((HANDLE)panel,FCTL_GETPANELINFO);
  if(pInfo.IsOk())
  {
    if((pInfo.ItemsNumber()>0)&&(!(pInfo[pInfo.CurrentItem()].FileAttributes&FILE_ATTRIBUTE_DIRECTORY)))
    {
      if(pInfo[pInfo.CurrentItem()].UserData.FreeData)
      {
        unsigned long mask=GetLevelFromUserData(pInfo[pInfo.CurrentItem()].UserData.Data);
        //Show dialog
        /*
          0000000000111111111122222222223333333333444444444455555555556666666666777777
          0123456789012345678901234567890123456789012345678901234567890123456789012345
        00                                                                            00
        01   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»   01
        02   º (*) Full                                                           º   02
        03   º ( ) Special                                                        º   03
        04   º ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿ º   04
        05   º ³ [ ] Read                                                       ³ º   05
        06   º ³ [ ] Write                                                      ³ º   06
        07   º ³ [ ] Execute                                                    ³ º   07
        08   º ³ [ ] Delete                                                     ³ º   08
        09   º ³ [ ] Change Permissions                                         ³ º   09
        10   º ³ [ ] Take Ownership                                             ³ º   10
        11   º ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ º   11
        12   º                 [ OK ]                [ Cancel ]                   º   12
        13   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   13
        14                                                                            14
          0000000000111111111122222222223333333333444444444455555555556666666666777777
          0123456789012345678901234567890123456789012345678901234567890123456789012345
        */
        FarDialogItem DialogItems[]=
        {
        /* 0*/  {DI_DOUBLEBOX,  3, 1,72,13,{0},NULL,NULL,0,                                _T(""),                              0,0,{0,0}},
        /* 1*/  {DI_SINGLEBOX,  5, 4,70,11,{0},NULL,NULL,DIF_LEFTTEXT,                     _T(""),                              0,0,{0,0}},
        /* 2*/  {DI_RADIOBUTTON,5, 2, 0, 0,{0},NULL,NULL,DIF_GROUP|DIF_FOCUS,              GetMsg(mPropSimpleFull),             0,0,{0,0}},
        /* 3*/  {DI_RADIOBUTTON,5, 3, 0, 0,{0},NULL,NULL,0,                                GetMsg(mPropSimpleSpecial),          0,0,{0,0}},
        /* 4*/  {DI_CHECKBOX,   7, 5, 0, 0,{0},NULL,NULL,0,                                GetMsg(mPropSimpleRead),             0,0,{0,0}},
        /* 5*/  {DI_CHECKBOX,   7, 6, 0, 0,{0},NULL,NULL,0,                                GetMsg(mPropSimpleWrite),            0,0,{0,0}},
        /* 6*/  {DI_CHECKBOX,   7, 7, 0, 0,{0},NULL,NULL,0,                                GetMsg(mPropSimpleExecute),          0,0,{0,0}},
        /* 7*/  {DI_CHECKBOX,   7, 8, 0, 0,{0},NULL,NULL,0,                                GetMsg(mPropSimpleDelete),           0,0,{0,0}},
        /* 8*/  {DI_CHECKBOX,   7, 9, 0, 0,{0},NULL,NULL,0,                                GetMsg(mPropSimpleChangePermissions),0,0,{0,0}},
        /* 9*/  {DI_CHECKBOX,   7,10, 0, 0,{0},NULL,NULL,0,                                GetMsg(mPropSimpleTakeOwnership),    0,0,{0,0}},
        /*10*/  {DI_BUTTON,     0,12, 0, 0,{0},NULL,NULL,DIF_CENTERGROUP|DIF_DEFAULTBUTTON,GetMsg(mPropButtonOk),               0,0,{0,0}},
        /*11*/  {DI_BUTTON,     0,12, 0, 0,{0},NULL,NULL,DIF_CENTERGROUP,                  GetMsg(mPropButtonCancel),           0,0,{0,0}}
        };
        if((mask&common_full_access[panel->level])==common_full_access[panel->level])
          DialogItems[2].Selected=TRUE;
        else
        {
          DialogItems[3].Selected=TRUE;
          for(int i=4;i<10;i++)
            if((mask&common_rights[panel->level][i-4])==common_rights[panel->level][i-4])
              DialogItems[i].Selected=TRUE;
        }
        CFarDialog dialog;
        int DlgCode=dialog.Execute(MainGuid,EditCommonAccessGuid,-1,-1,76,15,_T("EditCommonAccess"),DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,EditCommonAccessDialogProc,0);
        if(DlgCode==10)
        {
          mask=0;
          if(dialog.Check(2))
            mask=common_full_access[panel->level];
          else
          {
            for(int i=4;i<10;i++)
              if(dialog.Check(i))
                mask|=common_rights[panel->level][i-4];
          }
          if(UpdateAcl(panel,panel->level,GetSidFromUserData(pInfo[pInfo.CurrentItem()].UserData.Data),GetItemTypeFromUserData(pInfo[pInfo.CurrentItem()].UserData.Data),mask,actionUpdate))
            res=true;
        }
      }
    }
  }
  return res;
}
