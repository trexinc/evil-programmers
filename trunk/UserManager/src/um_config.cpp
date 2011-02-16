/*
    um_config.cpp
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
#include "far_helper.h"
#include "umplugin.h"
#include "guid.h"

static INT_PTR WINAPI ConfigDialogProc(HANDLE hDlg, int Msg,int Param1,INT_PTR Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

int Config(void)
{
  //Show dialog
  /*
    0000000000111111111122222222223333333333444444444455555555556666666666777777
    0123456789012345678901234567890123456789012345678901234567890123456789012345
  00                                                                            00
  01   ษออออออออออออออออออออออออออ Event viewer ออออออออออออออออออออออออออออป   01
  02   บ [x] Add to Disks menu                                              บ   02
  03   บ   2 Disks menu hotkey ('1'-'9'). Leave empty to autoassign         บ   03
  04   บ [ ] Add to Plugins menu                                            บ   04
  05   บ [ ] Add to Config menu                                             บ   05
  06   วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ   06
  07   บ [ ] Full user names                                                บ   07
  08   วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ   08
  09   บ Command line prefix to start the properties:                       บ   09
  10   บ acl                                                                บ   10
  11   วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ   11
  12   บ                         [ Ok ]  [ Cancel ]                         บ   12
  13   ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ   13
  14                                                                            14
    0000000000111111111122222222223333333333444444444455555555556666666666777777
    0123456789012345678901234567890123456789012345678901234567890123456789012345
  */

  FarDialogItem DialogItems[]=
  {
  /* 0*/  {DI_DOUBLEBOX,3, 1,72,12,{0},NULL,NULL,                 0,                                0,GetMsg(mName),                 0},
  /* 1*/  {DI_CHECKBOX, 5, 2, 0, 0,{0},NULL,NULL,                 0,                                0,GetMsg(mConfigAddToDisksMenu), 0},
  /* 2*/  {DI_CHECKBOX, 5, 3, 0, 0,{0},NULL,NULL,                 0,                                0,GetMsg(mConfigAddToPluginMenu),0},
  /* 3*/  {DI_CHECKBOX, 5, 4, 0, 0,{0},NULL,NULL,                 0,                                0,GetMsg(mConfigAddToConfigMenu),0},
  /* 4*/  {DI_TEXT,    -1, 5, 0, 0,{0},NULL,NULL,                 DIF_SEPARATOR,                    0,_T(""),                        0},
  /* 5*/  {DI_CHECKBOX, 5, 6, 0, 0,{0},NULL,NULL,                 0,                                0,GetMsg(mConfigFullUserName),   0},
  /* 6*/  {DI_TEXT,    -1, 7, 0, 0,{0},NULL,NULL,                 DIF_SEPARATOR,                    0,_T(""),                        0},
  /* 7*/  {DI_TEXT,     5, 8, 0, 0,{0},NULL,NULL,                 0,                                0,GetMsg(mConfigPrefix),         0},
  /* 8*/  {DI_FIXEDIT,  5, 9,19,12,{0},NULL,_T("AAAAAAAAAAAAAAA"),DIF_MASKEDIT,                     0,_T(""),                        0},
  /* 9*/  {DI_TEXT,    -1,10, 0, 0,{0},NULL,NULL,                 DIF_SEPARATOR,                    0,_T(""),                        0},
  /*10*/  {DI_BUTTON,   0,11, 0, 0,{0},NULL,NULL,                 DIF_CENTERGROUP|DIF_DEFAULTBUTTON,0,GetMsg(mConfigSave),           0},
  /*11*/  {DI_BUTTON,   0,11, 0, 0,{0},NULL,NULL,                 DIF_CENTERGROUP,                  0,GetMsg(mConfigCancel),         0}
  };

  TCHAR digit[21];

  DialogItems[1].Selected=Opt.AddToDisksMenu;
  DialogItems[2].Selected=Opt.AddToPluginsMenu;
  DialogItems[3].Selected=Opt.AddToConfigMenu;
  DialogItems[5].Selected=Opt.FullUserNames;
  INIT_DLG_DATA(DialogItems[8],Opt.Prefix);

  CFarDialog dialog;
  int DlgCode=dialog.Execute(MainGuid,ConfigGuid,-1,-1,76,14,_T("Config"),DialogItems,ArraySize(DialogItems),0,0,ConfigDialogProc,0);
  if (DlgCode!=10)
    return FALSE;
  Opt.AddToDisksMenu=dialog.Check(1);
  Opt.AddToPluginsMenu=dialog.Check(2);
  Opt.AddToConfigMenu=dialog.Check(3);
  Opt.FullUserNames=dialog.Check(5);
  _tcscpy(Opt.Prefix,dialog.Str(8));
  FSF.Trim(Opt.Prefix);

  CFarSettings set(MainGuid);
  set.Set(_T("AddToDisksMenu"),Opt.AddToDisksMenu);
  set.Set(_T("AddToPluginsMenu"),Opt.AddToPluginsMenu);
  set.Set(_T("AddToConfigMenu"),Opt.AddToConfigMenu);
  set.Set(_T("FullUserNames"),Opt.FullUserNames);
  set.Set(_T("Prefix"),Opt.Prefix);

  return TRUE;
}
