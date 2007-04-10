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
#include "..\..\plugin.hpp"
#include "umplugin.h"
#include "memory.h"

long WINAPI EditCommonAccessDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  FarDialogItem DialogItem;
  switch(Msg)
  {
    case DN_INITDIALOG:
    case DN_BTNCLICK:
      Info.SendDlgMessage(hDlg,DM_GETDLGITEM,2,(long)&DialogItem);
      if(DialogItem.Selected)
      {
        for(int i=4;i<10;i++)
        {
          Info.SendDlgMessage(hDlg,DM_GETDLGITEM,i,(long)&DialogItem);
          DialogItem.Selected=FALSE;
          DialogItem.Flags|=DIF_DISABLE;
          Info.SendDlgMessage(hDlg,DM_SETDLGITEM,i,(long)&DialogItem);
        }
      }
      else
      {
        for(int i=4;i<10;i++)
        {
          Info.SendDlgMessage(hDlg,DM_GETDLGITEM,i,(long)&DialogItem);
          DialogItem.Flags&=~DIF_DISABLE;
          Info.SendDlgMessage(hDlg,DM_SETDLGITEM,i,(long)&DialogItem);
        }
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool EditCommonAccess(UserManager *panel)
{
  bool res=false;
  PanelInfo PInfo;
  Info.Control((HANDLE)panel,FCTL_GETPANELINFO,&PInfo);
  if((PInfo.ItemsNumber>0)&&(!(PInfo.PanelItems[PInfo.CurrentItem].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)))
  {
    if(PInfo.PanelItems[PInfo.CurrentItem].Flags&PPIF_USERDATA)
    {
      unsigned long mask=GetLevelFromUserData(PInfo.PanelItems[PInfo.CurrentItem].UserData);
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
      static struct InitDialogItem InitItems[]={
      /* 0*/  {DI_DOUBLEBOX,3,1,72,13,0,0,0,0,""},
      /* 1*/  {DI_SINGLEBOX,5,4,70,11,0,0,DIF_LEFTTEXT,0,""},
      /* 2*/  {DI_RADIOBUTTON,5,2,0,0,1,0,DIF_GROUP,0,(char *)mPropSimpleFull},
      /* 3*/  {DI_RADIOBUTTON,5,3,0,0,0,0,0,0,(char *)mPropSimpleSpecial},
      /* 4*/  {DI_CHECKBOX,7, 5,0,0,0,0,0,0,(char *)mPropSimpleRead},
      /* 5*/  {DI_CHECKBOX,7, 6,0,0,0,0,0,0,(char *)mPropSimpleWrite},
      /* 6*/  {DI_CHECKBOX,7, 7,0,0,0,0,0,0,(char *)mPropSimpleExecute},
      /* 7*/  {DI_CHECKBOX,7, 8,0,0,0,0,0,0,(char *)mPropSimpleDelete},
      /* 8*/  {DI_CHECKBOX,7, 9,0,0,0,0,0,0,(char *)mPropSimpleChangePermissions},
      /* 9*/  {DI_CHECKBOX,7,10,0,0,0,0,0,0,(char *)mPropSimpleTakeOwnership},
      /*10*/  {DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,1,(char *)mPropButtonOk},
      /*11*/  {DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,0,(char *)mPropButtonCancel}
      };
      struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
      InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
      if((mask&common_full_access[panel->level])==common_full_access[panel->level])
        DialogItems[2].Selected=TRUE;
      else
      {
        DialogItems[3].Selected=TRUE;
        for(int i=4;i<10;i++)
          if((mask&common_rights[panel->level][i-4])==common_rights[panel->level][i-4])
            DialogItems[i].Selected=TRUE;
      }
      int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,76,15,"EditCommonAccess",DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,EditCommonAccessDialogProc,0);
      if(DlgCode==10)
      {
        mask=0;
        if(DialogItems[2].Selected)
          mask=common_full_access[panel->level];
        else
        {
          for(int i=4;i<10;i++)
            if(DialogItems[i].Selected)
              mask|=common_rights[panel->level][i-4];
        }
        if(UpdateAcl(panel,panel->level,GetSidFromUserData(PInfo.PanelItems[PInfo.CurrentItem].UserData),GetItemTypeFromUserData(PInfo.PanelItems[PInfo.CurrentItem].UserData),mask,actionUpdate))
          res=true;
      }
    }
  }
  return res;
}
