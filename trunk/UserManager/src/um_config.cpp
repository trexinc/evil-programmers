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
#include "..\..\plugin.hpp"
#include "umplugin.h"

static long WINAPI ConfigDialogProc(HANDLE hDlg, int Msg,int Param1,long Param2)
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

  static struct InitDialogItem InitItems[]={
  /* 0*/  {DI_DOUBLEBOX,3,1,72,13,0,0,0,0,(char *)mName},
  /* 1*/  {DI_CHECKBOX,5,2,0,0,0,0,0,0,(char *)mConfigAddToDisksMenu},
  /* 2*/  {DI_FIXEDIT,7,3,7,3,1,0,0,0,""},
  /* 3*/  {DI_TEXT,9,3,0,0,0,0,0,0,(char *)mConfigDisksMenuDigit},
  /* 4*/  {DI_CHECKBOX,5,4,0,0,0,0,0,0,(char *)mConfigAddToPluginMenu},
  /* 5*/  {DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)mConfigAddToConfigMenu},
  /* 6*/  {DI_TEXT,-1,6,0,0,0,0,DIF_SEPARATOR,0,""},
  /* 7*/  {DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)mConfigFullUserName},
  /* 8*/  {DI_TEXT,-1,8,0,0,0,0,DIF_SEPARATOR,0,""},
  /* 9*/  {DI_TEXT,5,9,0,0,0,0,0,0,(char *)mConfigPrefix},
  /*10*/  {DI_FIXEDIT,5,10,19,12,0,(int)"AAAAAAAAAAAAAAA",DIF_MASKEDIT,0,""},
  /*11*/  {DI_TEXT,-1,11,0,0,0,0,DIF_SEPARATOR,0,""},
  /*12*/  {DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,1,(char *)mConfigSave},
  /*13*/  {DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,0,(char *)mConfigCancel}
  };
  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  DialogItems[1].Selected=Opt.AddToDisksMenu;
  if (Opt.DisksMenuDigit)
    sprintf(DialogItems[2].Data,"%d",Opt.DisksMenuDigit);
  DialogItems[4].Selected=Opt.AddToPluginsMenu;
  DialogItems[5].Selected=Opt.AddToConfigMenu;
  DialogItems[7].Selected=Opt.FullUserNames;
  strcpy(DialogItems[10].Data,Opt.Prefix);

  int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,76,15,"Config",DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,ConfigDialogProc,0);
  if (DlgCode!=12)
    return FALSE;
  Opt.AddToDisksMenu=DialogItems[1].Selected;
  Opt.DisksMenuDigit=FSF.atoi(DialogItems[2].Data);
  Opt.AddToPluginsMenu=DialogItems[4].Selected;
  Opt.AddToConfigMenu=DialogItems[5].Selected;
  Opt.FullUserNames=DialogItems[7].Selected;
  strcpy(Opt.Prefix,DialogItems[10].Data);
  FSF.Trim(Opt.Prefix);
  HKEY hKey;
  DWORD Disposition;
  if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition)) == ERROR_SUCCESS)
  {
    RegSetValueEx(hKey,"AddToDisksMenu",0,REG_DWORD,(LPBYTE)&Opt.AddToDisksMenu,sizeof(Opt.AddToDisksMenu));
    RegSetValueEx(hKey,"DisksMenuDigit",0,REG_DWORD,(LPBYTE)&Opt.DisksMenuDigit,sizeof(Opt.DisksMenuDigit));
    RegSetValueEx(hKey,"AddToPluginsMenu",0,REG_DWORD,(LPBYTE)&Opt.AddToPluginsMenu,sizeof(Opt.AddToPluginsMenu));
    RegSetValueEx(hKey,"AddToConfigMenu",0,REG_DWORD,(LPBYTE)&Opt.AddToConfigMenu,sizeof(Opt.AddToConfigMenu));
    RegSetValueEx(hKey,"FullUserNames",0,REG_DWORD,(LPBYTE)&Opt.FullUserNames,sizeof(Opt.FullUserNames));
    RegSetValueEx(hKey,"Prefix",0,REG_SZ,(LPBYTE)Opt.Prefix,strlen(Opt.Prefix)+1);
    RegCloseKey(hKey);
  }
  return TRUE;
}
