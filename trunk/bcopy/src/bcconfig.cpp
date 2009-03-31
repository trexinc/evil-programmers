/*
    bcconfig.cpp
    Copyright (C) 2000-2009 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

enum
{
  CONFIGDLG_BORDER=0,
  CONFIGDLG_HISTORY,
  CONFIGDLG_FULLINFO,
  CONFIGDLG_SEPCOPY,
  CONFIGDLG_OVERWRITE,
  CONFIGDLG_APPEND,
  CONFIGDLG_SKIP,
  CONFIGDLG_REFRESH,
  CONFIGDLG_ASK,
  CONFIGDLG_ACCESS,
  CONFIGDLG_ROSRC,
  CONFIGDLG_RODST,
  CONFIGDLG_ABORT,
  CONFIGDLG_LINK,
  CONFIGDLG_SEPDEL,
  CONFIGDLG_DELRO,
  CONFIGDLG_DELABORT,
  CONFIGDLG_SEPWIPE,
  CONFIGDLG_WIPEABORT,
  CONFIGDLG_SEP,
  CONFIGDLG_SAVE,
  CONFIGDLG_CANCEL
};

enum
{
  CONFIGDLG2_BORDER=0,
  CONFIGDLG2_AUTOSTART,
  CONFIGDLG2_SEP1,
  CONFIGDLG2_CONFIGMENU,
  CONFIGDLG2_VIEWERMENU,
  CONFIGDLG2_EDITORMENU,
  CONFIGDLG2_DIALOGMENU,
  CONFIGDLG2_SEP2,
  CONFIGDLG2_INFOEMPTY,
  CONFIGDLG2_INFOONLY,
  CONFIGDLG2_SEP3,
  CONFIGDLG2_AUTOSHOWINFO,
  CONFIGDLG2_EXPANDVARS,
  CONFIGDLG2_PASSIVE,
  CONFIGDLG2_FORMATSIZE,
  CONFIGDLG2_CURRENTTIME,
  CONFIGDLG2_CONFIRMABORT,
  CONFIGDLG2_AUTOREFRESH,
  CONFIGDLG2_RESOLVEDESTINATION,
  CONFIGDLG2_SEP4,
  CONFIGDLG2_SAVE,
  CONFIGDLG2_CANCEL,
  CONFIGDLG2_COLOR,
};

enum
{
  CONFIGDLG3_BORDER=0,
  CONFIGDLG3_LMAXERROR,
  CONFIGDLG3_EMAXERROR,
  CONFIGDLG3_LTHREAD,
  CONFIGDLG3_ETHREAD,
  CONFIGDLG3_LQUEUESIZE,
  CONFIGDLG3_EQUEUESIZE,
  CONFIGDLG3_LWORKPR,
  CONFIGDLG3_EWORKPR,
  CONFIGDLG3_LHEARPR,
  CONFIGDLG3_EHEARPR,
  CONFIGDLG3_NETWORK,
  CONFIGDLG3_SEP,
  CONFIGDLG3_SAVE,
  CONFIGDLG3_CANCEL
};

static LONG_PTR WINAPI Config2DialogProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);
static LONG_PTR WINAPI Config3DialogProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);

struct ComboPos
{
  int WorkPos;
  int HearPos;
};

static int Config()
{
  FarMenuItem MenuItems[3];
#ifdef UNICODE
  TCHAR ItemText[3][128];
#endif
  memset(MenuItems,0,sizeof(MenuItems));
  int Msgs[]={mConfigMenu1,mConfigMenu2,mConfigMenu3};

  for(unsigned int i=0;i<sizeofa(MenuItems);i++)
  {
    MenuItems[i].Checked=MenuItems[i].Separator=0;
#ifdef UNICODE
    _tcscpy(ItemText[i],GetMsg(Msgs[i]));
    MenuItems[i].Text=ItemText[i];
#else
    _tcscpy(MenuItems[i].Text,GetMsg(Msgs[i])); // Text in menu
#endif
  };
  int MenuCode=0;
  while(TRUE)
  {
    for(unsigned int i=0;i<sizeofa(MenuItems);i++)
      MenuItems[i].Selected=0;
    MenuItems[MenuCode].Selected=TRUE;
    // Show menu
    MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_WRAPMODE,GetMsg(mName),NULL,_T("Config"),NULL,NULL,MenuItems,sizeofa(MenuItems));
    if(MenuCode==-1)
      break;
    else if(MenuCode==0)
    {
      //Show dialog
      /*
        0000000000111111111122222222223333333333444444444455555555556666666666777777
        0123456789012345678901234567890123456789012345678901234567890123456789012345
      00                                                                            00
      01   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ Backgroung copy ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»   01
      02   º [ ] Use system copy history                                        º   02
      03   º [ ] Show full info                                                 º   03
      04   ÇÄÄÄÄÄÄÄÄÄ Copy ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ º   04
      05   º ( ) Overwrite all existing files                                   º   05
      06   º ( ) Append to all existing files                                   º   06
      07   º ( ) Skip all existing files                                        º   07
      08   º ( ) Refresh old files                                              º   08
      09   º                                                                    º   09
      10   º [ ] Copy access rights                                             º   10
      11   º [ ] Clear R/O attribute on &source                                 º   11
      12   º [ ] Clear R/O attribute on &destination                            º   12
      13   º [ ] Abort on error                                                 º   13
      15   º ÚÄ Delete ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿ º   15
      16   º ³ [ ] Clear R/O attribute                                        ³ º   16
      17   º ³ [ ] Abort on error                                             ³ º   17
      18   º ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ º   18
      19   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   19
      20   º                 [ Save ]              [ Cancel ]                   º   20
      21   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   21
      22                                                                            22
        0000000000111111111122222222223333333333444444444455555555556666666666777777
        0123456789012345678901234567890123456789012345678901234567890123456789012345
      */

      static struct InitDialogItem InitItems[]={
      /* 0*/  {DI_DOUBLEBOX,3,1,72,23,0,0,0,0,(TCHAR *)mName},
      /* 1*/  {DI_CHECKBOX,5,2,0,0,1,0,0,0,(TCHAR *)mConfigHistory},
      /* 2*/  {DI_CHECKBOX,5,3,0,0,0,0,0,0,(TCHAR *)mConfigFullInfo},

      /* 3*/  {DI_TEXT,-1,4,0,0,0,0,DIF_SEPARATOR,0,_T("")},
      /* 4*/  {DI_RADIOBUTTON,5,5,0,0,0,0,DIF_GROUP,0,(TCHAR *)mCpyDlgOpt1},
      /* 5*/  {DI_RADIOBUTTON,5,6,0,0,0,0,0,0,(TCHAR *)mCpyDlgOpt2},
      /* 6*/  {DI_RADIOBUTTON,5,7,0,0,0,0,0,0,(TCHAR *)mCpyDlgOpt3},
      /* 7*/  {DI_RADIOBUTTON,5,8,0,0,0,0,0,0,(TCHAR *)mCpyDlgOpt4},
      /* 8*/  {DI_RADIOBUTTON,5,9,0,0,0,0,0,0,(TCHAR *)mCpyDlgAsk},
      /* 9*/  {DI_CHECKBOX,5,11,0,0,0,0,0,0,(TCHAR *)mCpyDlgChkAccess},
      /*10*/  {DI_CHECKBOX,5,12,0,0,0,0,0,0,(TCHAR *)mCpyDlgChk2},
      /*11*/  {DI_CHECKBOX,5,13,0,0,0,0,0,0,(TCHAR *)mCpyDlgChk3},
      /*12*/  {DI_CHECKBOX,5,14,0,0,0,0,DIF_3STATE,0,(TCHAR *)mCpyDlgChk4},
      /*13*/  {DI_CHECKBOX,5,15,0,0,0,0,DIF_3STATE,0,(TCHAR *)mCpyDlgChkLink},

      /*14*/  {DI_TEXT,-1,16,0,0,0,0,DIF_SEPARATOR,0,_T("")},
      /*15*/  {DI_CHECKBOX,5,17,0,0,0,0,0,0,(TCHAR *)mDelDlgChk1},
      /*16*/  {DI_CHECKBOX,5,18,0,0,0,0,DIF_3STATE,0,(TCHAR *)mConfigDelAbort},

      /*17*/  {DI_TEXT,-1,19,0,0,0,0,DIF_SEPARATOR,0,_T("")},
      /*18*/  {DI_CHECKBOX,5,20,0,0,0,0,DIF_3STATE,0,(TCHAR *)mConfigWpeAbort},
      /*19*/  {DI_TEXT,-1,21,0,0,0,0,DIF_SEPARATOR,0,_T("")},
      /*20*/  {DI_BUTTON,0,22,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)mConfigSave},
      /*21*/  {DI_BUTTON,0,22,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)mConfigCancel}
      };

      struct FarDialogItem DialogItems[sizeofa(InitItems)];
      InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));

      TCHAR sepcopy[512],sepdel[512],sepwipe[512];

      _tcscpy(sepcopy,_T(" "));
      _tcscat(sepcopy,GetMsg(mCpyDlgCopyTitle));
      _tcscat(sepcopy,_T(" "));
      INIT_DLG_DATA(DialogItems[CONFIGDLG_SEPCOPY],sepcopy);
      _tcscpy(sepdel,_T(" "));
      _tcscat(sepdel,GetMsg(mDelDlgTitle));
      _tcscat(sepdel,_T(" "));
      INIT_DLG_DATA(DialogItems[CONFIGDLG_SEPDEL],sepdel);
      _tcscpy(sepwipe,_T(" "));
      _tcscat(sepwipe,GetMsg(mWpeDlgTitle));
      _tcscat(sepwipe,_T(" "));
      INIT_DLG_DATA(DialogItems[CONFIGDLG_SEPWIPE],sepwipe);
      DialogItems[CONFIGDLG_HISTORY].Selected=Opt.CopyHistory;
      DialogItems[CONFIGDLG_FULLINFO].Selected=Opt.CopyFullInfo;
      DialogItems[CONFIGDLG_OVERWRITE+Opt.CopyType].Selected=1;
      DialogItems[CONFIGDLG_ACCESS].Selected=Opt.CopyAccess;
      DialogItems[CONFIGDLG_ROSRC].Selected=Opt.CopyROSrc;
      DialogItems[CONFIGDLG_RODST].Selected=Opt.CopyRODest;
      DialogItems[CONFIGDLG_ABORT].Selected=Opt.CopyAbort;
      DialogItems[CONFIGDLG_LINK].Selected=Opt.CopyLink;

      DialogItems[CONFIGDLG_DELRO].Selected=Opt.DeleteRO;
      DialogItems[CONFIGDLG_DELABORT].Selected=Opt.DeleteAbort;

      DialogItems[CONFIGDLG_WIPEABORT].Selected=Opt.WipeAbort;

      CFarDialog dialog;
      int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,76,25,_T("Config1"),DialogItems,sizeofa(DialogItems),0,0,Config1DialogProc,0);
      if(DlgCode==CONFIGDLG_SAVE)
      {
        Opt.CopyType=0;
        int i=CONFIGDLG_OVERWRITE;
        while(!dialog.Check(i)) {i++; Opt.CopyType++;}
        Opt.CopyHistory=dialog.Check(CONFIGDLG_HISTORY);
        Opt.CopyFullInfo=dialog.Check(CONFIGDLG_FULLINFO);
        Opt.CopyAccess=dialog.Check(CONFIGDLG_ACCESS);
        Opt.CopyROSrc=dialog.Check(CONFIGDLG_ROSRC);
        Opt.CopyRODest=dialog.Check(CONFIGDLG_RODST);
        Opt.CopyAbort=dialog.Check(CONFIGDLG_ABORT);
        Opt.CopyLink=dialog.Check(CONFIGDLG_LINK);

        Opt.DeleteRO=dialog.Check(CONFIGDLG_DELRO);
        Opt.DeleteAbort=dialog.Check(CONFIGDLG_DELABORT);

        Opt.WipeAbort=dialog.Check(CONFIGDLG_WIPEABORT);

        HKEY hKey;
        DWORD Disposition;
        if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
        {
          RegSetValueEx(hKey,_T("CopyType"),0,REG_DWORD,(LPBYTE)&Opt.CopyType,sizeof(Opt.CopyType));
          RegSetValueEx(hKey,_T("CopyHistory"),0,REG_DWORD,(LPBYTE)&Opt.CopyHistory,sizeof(Opt.CopyHistory));
          RegSetValueEx(hKey,_T("CopyROSrc"),0,REG_DWORD,(LPBYTE)&Opt.CopyROSrc,sizeof(Opt.CopyROSrc));
          RegSetValueEx(hKey,_T("CopyRODest"),0,REG_DWORD,(LPBYTE)&Opt.CopyRODest,sizeof(Opt.CopyRODest));
          RegSetValueEx(hKey,_T("CopyAbort"),0,REG_DWORD,(LPBYTE)&Opt.CopyAbort,sizeof(Opt.CopyAbort));
          RegSetValueEx(hKey,_T("CopyLink"),0,REG_DWORD,(LPBYTE)&Opt.CopyLink,sizeof(Opt.CopyLink));
          RegSetValueEx(hKey,_T("CopyFullInfo"),0,REG_DWORD,(LPBYTE)&Opt.CopyFullInfo,sizeof(Opt.CopyFullInfo));
          RegSetValueEx(hKey,_T("CopyAccess"),0,REG_DWORD,(LPBYTE)&Opt.CopyAccess,sizeof(Opt.CopyAccess));
          RegSetValueEx(hKey,_T("DeleteRO"),0,REG_DWORD,(LPBYTE)&Opt.DeleteRO,sizeof(Opt.DeleteRO));
          RegSetValueEx(hKey,_T("DeleteAbort"),0,REG_DWORD,(LPBYTE)&Opt.DeleteAbort,sizeof(Opt.DeleteAbort));
          RegSetValueEx(hKey,_T("WipeAbort"),0,REG_DWORD,(LPBYTE)&Opt.WipeAbort,sizeof(Opt.WipeAbort));
          RegCloseKey(hKey);
        }
      }
    }
    else if(MenuCode==1)
    {
      //Show dialog
      /*
        0000000000111111111122222222223333333333444444444455555555556666666666777777
        0123456789012345678901234567890123456789012345678901234567890123456789012345
      00                                                                            00
      01   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ Backgroung copy ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»   01
      02   º [ ] &Autostart service                                             º   02
      03   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   03
      04   º [ ] Add to &Plugins menu                                           º   04
      05   º [ ] Add to &Viewer menu                                            º   05
      06   º [ ] Add to &Editor menu                                            º   06
      07   º [ ] Add to &Dialog menu                                            º   06
      08   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ Info menu ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   07
      09   º [ ] &Display message when job list is empty                        º   08
      10   º [ ] Autoselect &only job                                           º   09
      11   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   10
      12   º [ ] Auto&show information dialog                                   º   11
      13   º [ ] E&xpand variables in copy dialog                               º   12
      14   º [ ] Don't &check passive panel type                                º   13
      15   º [ ] &Format size                                                   º   14
      16   º [ ] Show current time                                              º   15
      17   º [ ] Co&nfirm abort                                                 º   16
      18   º [ ] Auto&refresh                                                   º   17
      19   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   18
      20   º                 [ Save ]              [ Cancel ]                   º   19
      21   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   20
      22                                                                            21
        0000000000111111111122222222223333333333444444444455555555556666666666777777
        0123456789012345678901234567890123456789012345678901234567890123456789012345
      */
      static struct InitDialogItem InitItems[]={
      /* 0*/  {DI_DOUBLEBOX,3,1,72,22,0,0,0,0,(TCHAR *)mName},
      /* 1*/  {DI_CHECKBOX,5,2,0,0,1,0,0,0,(TCHAR *)mConfigAutostart},
      /* 2*/  {DI_TEXT,-1,3,0,0,0,0,DIF_SEPARATOR,0,_T("")},
      /* 3*/  {DI_CHECKBOX,5,4,0,0,0,0,0,0,(TCHAR *)mConfigConfigMenu},
      /* 4*/  {DI_CHECKBOX,5,5,0,0,0,0,0,0,(TCHAR *)mConfigViewerMenu},
      /* 5*/  {DI_CHECKBOX,5,6,0,0,0,0,0,0,(TCHAR *)mConfigEditorMenu},
      /* 5*/  {DI_CHECKBOX,5,7,0,0,0,0,0,0,(TCHAR *)mConfigDialogMenu},
      /* 6*/  {DI_TEXT,-1,8,0,0,0,0,DIF_SEPARATOR,0,_T("")},
      /* 7*/  {DI_CHECKBOX,5,9,0,0,0,0,0,0,(TCHAR *)mConfigInfoEmpty},
      /* 8*/  {DI_CHECKBOX,5,10,0,0,0,0,0,0,(TCHAR *)mConfigInfoOnly},
      /* 9*/  {DI_TEXT,-1,11,0,0,0,0,DIF_SEPARATOR,0,_T("")},
      /*10*/  {DI_CHECKBOX,5,12,0,0,0,0,0,0,(TCHAR *)mComfigAutoShowInfo},
      /*11*/  {DI_CHECKBOX,5,13,0,0,0,0,0,0,(TCHAR *)mConfigExpandVars},
      /*12*/  {DI_CHECKBOX,5,14,0,0,0,0,0,0,(TCHAR *)mConfigPassive},
      /*13*/  {DI_CHECKBOX,5,15,0,0,0,0,0,0,(TCHAR *)mConfigFormatSize},
      /*14*/  {DI_CHECKBOX,5,16,0,0,0,0,0,0,(TCHAR *)mConfigCurrentTime},
      /*15*/  {DI_CHECKBOX,5,17,0,0,0,0,0,0,(TCHAR *)mConfigConfirmAbort},
      /*16*/  {DI_CHECKBOX,5,18,0,0,0,0,0,0,(TCHAR *)mConfigAutoRefresh},
      /*17*/  {DI_CHECKBOX,5,19,0,0,0,0,0,0,(TCHAR *)mConfigResolveDestination},
      /*18*/  {DI_TEXT,-1,20,0,0,0,0,DIF_SEPARATOR,0,_T("")},
      /*19*/  {DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)mConfigSave},
      /*20*/  {DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)mConfigCancel},
      /*21*/  {DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(TCHAR *)mColorMain}
      };
      struct FarDialogItem DialogItems[sizeofa(InitItems)];
      InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));
      DialogItems[CONFIGDLG2_AUTOSTART].Selected=PlgOpt.AutoStart;
      DialogItems[CONFIGDLG2_CONFIGMENU].Selected=PlgOpt.ShowMenu&SHOW_IN_CONFIG;
      DialogItems[CONFIGDLG2_VIEWERMENU].Selected=PlgOpt.ShowMenu&SHOW_IN_VIEWER;
      DialogItems[CONFIGDLG2_EDITORMENU].Selected=PlgOpt.ShowMenu&SHOW_IN_EDITOR;
      DialogItems[CONFIGDLG2_DIALOGMENU].Selected=PlgOpt.ShowMenu&SHOW_IN_DIALOG;
      DialogItems[CONFIGDLG2_INFOEMPTY].Selected=PlgOpt.InfoMenu&INFO_MENU_ALT_0;
      DialogItems[CONFIGDLG2_INFOONLY].Selected=PlgOpt.InfoMenu&INFO_MENU_ALT_1;
      DialogItems[CONFIGDLG2_AUTOSHOWINFO].Selected=PlgOpt.AutoShowInfo;
      DialogItems[CONFIGDLG2_EXPANDVARS].Selected=PlgOpt.ExpandVars;
      DialogItems[CONFIGDLG2_PASSIVE].Selected=PlgOpt.CheckPassive;
      DialogItems[CONFIGDLG2_FORMATSIZE].Selected=PlgOpt.FormatSize;
      DialogItems[CONFIGDLG2_CURRENTTIME].Selected=PlgOpt.CurrentTime;
      DialogItems[CONFIGDLG2_CONFIRMABORT].Selected=PlgOpt.ConfirmAbort;
      DialogItems[CONFIGDLG2_AUTOREFRESH].Selected=PlgOpt.AutoRefresh;
      DialogItems[CONFIGDLG2_RESOLVEDESTINATION].Selected=PlgOpt.ResolveDestination;
      TCHAR sepinfo[512];
      _tcscpy(sepinfo,_T(" "));
      _tcscat(sepinfo,GetMsg(mConfigInfo));
      _tcscat(sepinfo,_T(" "));
      INIT_DLG_DATA(DialogItems[CONFIGDLG2_SEP2],sepinfo);

      CFarDialog dialog;
      int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,76,24,_T("Config2"),DialogItems,sizeofa(DialogItems),0,0,Config2DialogProc,0);
      if(DlgCode==CONFIGDLG2_SAVE)
      {
        PlgOpt.AutoStart=dialog.Check(CONFIGDLG2_AUTOSTART);
        PlgOpt.ShowMenu=0;
        if(dialog.Check(CONFIGDLG2_CONFIGMENU))
          PlgOpt.ShowMenu|=SHOW_IN_CONFIG;
        if(dialog.Check(CONFIGDLG2_VIEWERMENU))
          PlgOpt.ShowMenu|=SHOW_IN_VIEWER;
        if(dialog.Check(CONFIGDLG2_EDITORMENU))
          PlgOpt.ShowMenu|=SHOW_IN_EDITOR;
        if(dialog.Check(CONFIGDLG2_DIALOGMENU))
          PlgOpt.ShowMenu|=SHOW_IN_DIALOG;
        PlgOpt.InfoMenu=0;
        if(dialog.Check(CONFIGDLG2_INFOEMPTY))
          PlgOpt.InfoMenu|=INFO_MENU_ALT_0;
        if(dialog.Check(CONFIGDLG2_INFOONLY))
          PlgOpt.InfoMenu|=INFO_MENU_ALT_1;
        PlgOpt.AutoShowInfo=dialog.Check(CONFIGDLG2_AUTOSHOWINFO);
        PlgOpt.ExpandVars=dialog.Check(CONFIGDLG2_EXPANDVARS);
        PlgOpt.CheckPassive=dialog.Check(CONFIGDLG2_PASSIVE);
        PlgOpt.FormatSize=dialog.Check(CONFIGDLG2_FORMATSIZE);
        PlgOpt.CurrentTime=dialog.Check(CONFIGDLG2_CURRENTTIME);
        PlgOpt.ConfirmAbort=dialog.Check(CONFIGDLG2_CONFIRMABORT);
        PlgOpt.AutoRefresh=dialog.Check(CONFIGDLG2_AUTOREFRESH);
        PlgOpt.ResolveDestination=dialog.Check(CONFIGDLG2_RESOLVEDESTINATION);
        HKEY hKey;
        DWORD Disposition;
        if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
        {
          RegSetValueEx(hKey,_T("TechAutoStart"),0,REG_DWORD,(LPBYTE)&PlgOpt.AutoStart,sizeof(PlgOpt.AutoStart));
          RegSetValueEx(hKey,_T("TechShowMenu"),0,REG_DWORD,(LPBYTE)&PlgOpt.ShowMenu,sizeof(PlgOpt.ShowMenu));
          RegSetValueEx(hKey,_T("TechInfoMenu"),0,REG_DWORD,(LPBYTE)&PlgOpt.InfoMenu,sizeof(PlgOpt.InfoMenu));
          RegSetValueEx(hKey,_T("TechExpandVars"),0,REG_DWORD,(LPBYTE)&PlgOpt.ExpandVars,sizeof(PlgOpt.ExpandVars));
          RegSetValueEx(hKey,_T("TechCheckPassive"),0,REG_DWORD,(LPBYTE)&PlgOpt.CheckPassive,sizeof(PlgOpt.CheckPassive));
          RegSetValueEx(hKey,_T("TechFormatSize"),0,REG_DWORD,(LPBYTE)&PlgOpt.FormatSize,sizeof(PlgOpt.FormatSize));
          RegSetValueEx(hKey,_T("TechAutoShowInfo"),0,REG_DWORD,(LPBYTE)&PlgOpt.AutoShowInfo,sizeof(PlgOpt.AutoShowInfo));
          RegSetValueEx(hKey,_T("TechConfirmAbort"),0,REG_DWORD,(LPBYTE)&PlgOpt.ConfirmAbort,sizeof(PlgOpt.ConfirmAbort));
          RegSetValueEx(hKey,_T("TechCurrentTime"),0,REG_DWORD,(LPBYTE)&PlgOpt.CurrentTime,sizeof(PlgOpt.CurrentTime));
          RegSetValueEx(hKey,_T("TechAutoRefresh"),0,REG_DWORD,(LPBYTE)&PlgOpt.AutoRefresh,sizeof(PlgOpt.AutoRefresh));
          RegSetValueEx(hKey,_T("TechResolveDestination"),0,REG_DWORD,(LPBYTE)&PlgOpt.ResolveDestination,sizeof(PlgOpt.ResolveDestination));
          RegCloseKey(hKey);
        }
      }
    }
    else if(MenuCode==2)
    {
      //Show dialog
      /*
        00000000001111111111222222222233333333334444444444555555555566666666667777777
        01234567890123456789012345678901234567890123456789012345678901234567890123456
      00                                                                             00
      01   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ Backgroung copy ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»   01
      02   º &Maximum errors per thread:   ÛÛ                                    º   02
      03   º &Number of listening threads: ÛÛ                                    º   03
      04   º [ ] &Allow network connections to the pipe                          º   04
      05   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   05
      06   º       [ Save ] [ Cancel ] [ Set password ] [ Clear password ]       º   06
      07   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   07
      08                                                                             08
        00000000001111111111222222222233333333334444444444555555555566666666667777777
        01234567890123456789012345678901234567890123456789012345678901234567890123456
      */
      static struct InitDialogItem InitItems[]={
      /*00*/  {DI_DOUBLEBOX,3,1,73,10,0,0,0,0,(TCHAR *)mName},
      /*01*/  {DI_TEXT,5,2,0,0,0,0,0,0,(TCHAR *)mConfigMaxErrors},
      /*02*/  {DI_FIXEDIT,0,2,0,0,1,(DWORD_PTR)_T("#####9"),DIF_MASKEDIT,0,_T("20")},
      /*03*/  {DI_TEXT,5,3,0,0,0,0,0,0,(TCHAR *)mConfigThreadCount},
      /*04*/  {DI_FIXEDIT,0,3,0,0,0,(DWORD_PTR)_T("#9"),DIF_MASKEDIT,0,_T(" 4")},
      /*05*/  {DI_TEXT,5,4,0,0,0,0,0,0,(TCHAR *)mConfigQueueSize},
      /*06*/  {DI_FIXEDIT,0,4,0,0,0,(DWORD_PTR)_T("#####9"),DIF_MASKEDIT,0,_T(" 4")},
      /*07*/  {DI_TEXT,5,5,0,0,0,0,0,0,(TCHAR *)mConfigWork},
      /*08*/  {DI_COMBOBOX,0,5,71,0,0,0,DIF_DROPDOWNLIST,0,_T("")},
      /*09*/  {DI_TEXT,5,6,0,0,0,0,0,0,(TCHAR *)mConfigHear},
      /*10*/  {DI_COMBOBOX,0,6,71,0,0,0,DIF_DROPDOWNLIST,0,_T("")},
      /*11*/  {DI_CHECKBOX,5,7,0,0,0,0,0,0,(TCHAR *)mConfigAllowNetwork},
      /*12*/  {DI_TEXT,-1,8,0,0,0,0,DIF_SEPARATOR,0,_T("")},
      /*13*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)mConfigSave},
      /*14*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)mConfigCancel},
      };
      struct FarDialogItem DialogItems[sizeofa(InitItems)];
      InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));
      unsigned int max=0;
      {
        int labels[]={CONFIGDLG3_LMAXERROR,CONFIGDLG3_LTHREAD,CONFIGDLG3_LQUEUESIZE,CONFIGDLG3_LWORKPR,CONFIGDLG3_LHEARPR};
        for(unsigned int i=0;i<sizeofa(labels);i++)
        {
          unsigned int cur_len=(unsigned int)_tcslen(DLG_DATA(DialogItems[labels[i]]));
          if(max<cur_len) max=cur_len;
        }
      }
      DialogItems[CONFIGDLG3_EMAXERROR].X1=DialogItems[CONFIGDLG3_LMAXERROR].X1+max; DialogItems[CONFIGDLG3_EMAXERROR].X2=DialogItems[CONFIGDLG3_EMAXERROR].X1+5;
      DialogItems[CONFIGDLG3_ETHREAD].X1=DialogItems[CONFIGDLG3_LTHREAD].X1+max; DialogItems[CONFIGDLG3_ETHREAD].X2=DialogItems[CONFIGDLG3_ETHREAD].X1+1;
      DialogItems[CONFIGDLG3_EQUEUESIZE].X1=DialogItems[CONFIGDLG3_LQUEUESIZE].X1+max; DialogItems[CONFIGDLG3_EQUEUESIZE].X2=DialogItems[CONFIGDLG3_EQUEUESIZE].X1+5;
      DialogItems[CONFIGDLG3_EWORKPR].X1=DialogItems[CONFIGDLG3_LWORKPR].X1+max;
      DialogItems[CONFIGDLG3_EHEARPR].X1=DialogItems[CONFIGDLG3_LHEARPR].X1+max;

      FarListItem Priority[7];
#ifdef UNICODE
      TCHAR PriorityText[ArraySize(Priority)][128];
#endif
      FarList Priorities={sizeofa(Priority),Priority};
      int PriorityValues[]={THREAD_PRIORITY_IDLE,THREAD_PRIORITY_LOWEST,THREAD_PRIORITY_BELOW_NORMAL,THREAD_PRIORITY_NORMAL,THREAD_PRIORITY_ABOVE_NORMAL,THREAD_PRIORITY_HIGHEST,THREAD_PRIORITY_TIME_CRITICAL};
      int WorkPriority=THREAD_PRIORITY_IDLE,HearPriority=THREAD_PRIORITY_HIGHEST;
      ComboPos data={0,5};
      for(unsigned int i=0;i<sizeofa(Priority);i++)
      {
        Priority[i].Flags=0;
#ifdef UNICODE
        _tcscpy(PriorityText[i],GetMsg(mPriorityIdle+i));
        Priority[i].Text=PriorityText[i];
#else
        _tcscpy(Priority[i].Text,GetMsg(mPriorityIdle+i));
#endif
      }
      DialogItems[CONFIGDLG3_EWORKPR].ListItems=&Priorities;
      DialogItems[CONFIGDLG3_EHEARPR].ListItems=&Priorities;
      INIT_DLG_DATA(DialogItems[CONFIGDLG3_EWORKPR],GetMsg(mPriorityIdle));
      INIT_DLG_DATA(DialogItems[CONFIGDLG3_EHEARPR],GetMsg(mPriorityHighest));

      HKEY hKey; DWORD Type,DataSize; DWORD Value; DWORD Disposition;
      TCHAR dataerrors[7],datathreads[7],dataqueues[7];
      if((RegOpenKeyEx(HKEY_LOCAL_MACHINE,_T("SYSTEM\\CurrentControlSet\\Services\\"SVC_NAME"\\Parameters"),0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
      {
        DataSize=sizeof(Value); Value=20;
        RegQueryValueEx(hKey,_T("MaxError"),0,&Type,(LPBYTE)&Value,&DataSize);
        _stprintf(dataerrors,_T("%6ld"),Value);
        INIT_DLG_DATA(DialogItems[CONFIGDLG3_EMAXERROR],dataerrors);
        DataSize=sizeof(Value); Value=4;
        RegQueryValueEx(hKey,_T("ThreadCount"),0,&Type,(LPBYTE)&Value,&DataSize);
        if((Value>16)||(Value<1)) Value=4;
        _stprintf(datathreads,_T("%2ld"),Value);
        INIT_DLG_DATA(DialogItems[CONFIGDLG3_ETHREAD],datathreads);
        DataSize=sizeof(Value); Value=4;
        RegQueryValueEx(hKey,_T("QueueSize"),0,&Type,(LPBYTE)&Value,&DataSize);
        _stprintf(dataqueues,_T("%6ld"),Value);
        INIT_DLG_DATA(DialogItems[CONFIGDLG3_EQUEUESIZE],dataqueues);
        DataSize=sizeof(Value); Value=0;
        RegQueryValueEx(hKey,_T("AllowNetwork"),0,&Type,(LPBYTE)&Value,&DataSize);
        DialogItems[CONFIGDLG3_NETWORK].Selected=Value;
        DataSize=sizeof(WorkPriority);
        RegQueryValueEx(hKey,_T("WorkPriority"),0,&Type,(LPBYTE)&WorkPriority,&DataSize);
        DataSize=sizeof(HearPriority);
        RegQueryValueEx(hKey,_T("HearPriority"),0,&Type,(LPBYTE)&HearPriority,&DataSize);
      }
      for(unsigned int i=0;i<sizeofa(PriorityValues);i++)
      {
        if(WorkPriority==PriorityValues[i]) data.WorkPos=i;
        if(HearPriority==PriorityValues[i]) data.HearPos=i;
      }
      CFarDialog dialog;
      int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,77,12,_T("Config3"),DialogItems,sizeofa(DialogItems),0,0,Config3DialogProc,(LONG_PTR)&data);
      if(DlgCode==CONFIGDLG3_SAVE)
      {
        if((RegCreateKeyEx(HKEY_LOCAL_MACHINE,_T("SYSTEM\\CurrentControlSet\\Services\\"SVC_NAME"\\Parameters"),0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
        {
          Value=_ttoi(dialog.Str(CONFIGDLG3_EMAXERROR));
          RegSetValueEx(hKey,_T("MaxError"),0,REG_DWORD,(LPBYTE)&Value,sizeof(Value));
          Value=_ttoi(dialog.Str(CONFIGDLG3_ETHREAD)); if((Value<1)||(Value>16)) Value=4;
          RegSetValueEx(hKey,_T("ThreadCount"),0,REG_DWORD,(LPBYTE)&Value,sizeof(Value));
          Value=_ttoi(dialog.Str(CONFIGDLG3_EQUEUESIZE));
          RegSetValueEx(hKey,_T("QueueSize"),0,REG_DWORD,(LPBYTE)&Value,sizeof(Value));
          Value=dialog.Check(CONFIGDLG3_NETWORK);
          RegSetValueEx(hKey,_T("AllowNetwork"),0,REG_DWORD,(LPBYTE)&Value,sizeof(Value));
          WorkPriority=PriorityValues[dialog.ListPos(CONFIGDLG3_EWORKPR)];
          HearPriority=PriorityValues[dialog.ListPos(CONFIGDLG3_EHEARPR)];
          RegSetValueEx(hKey,_T("WorkPriority"),0,REG_DWORD,(LPBYTE)&WorkPriority,sizeof(WorkPriority));
          RegSetValueEx(hKey,_T("HearPriority"),0,REG_DWORD,(LPBYTE)&HearPriority,sizeof(HearPriority));
          RegCloseKey(hKey);
        }
        {
          const TCHAR *MsgItems[]={_T(""),GetMsg(mActualize),GetMsg(mOk)};
          Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeofa(MsgItems),1);
        }
      }
    }
  }
  return FALSE;
}

static LONG_PTR WINAPI Config2DialogProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  switch(Msg)
  {
    case DN_BTNCLICK:
      if(Param1==CONFIGDLG2_COLOR)
      {
        int fg=PlgOpt.ErrorColor&0x0F,bg=(PlgOpt.ErrorColor&0xF0)>>4;
        if(SelectColor(&fg,&bg))
        {
          PlgOpt.ErrorColor=fg|(bg<<4);
          HKEY hKey;
          DWORD Disposition;
          if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
          {
            RegSetValueEx(hKey,_T("TechErrorColor"),0,REG_DWORD,(LPBYTE)&PlgOpt.ErrorColor,sizeof(PlgOpt.ErrorColor));
            RegCloseKey(hKey);
          }
        }
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

static LONG_PTR WINAPI Config3DialogProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  switch(Msg)
  {
    case DN_INITDIALOG:
      {
        FarListPos pos={((ComboPos *)Param2)->WorkPos,-1};
        Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,CONFIGDLG3_EWORKPR,(LONG_PTR)&pos);
        pos.SelectPos=((ComboPos *)Param2)->HearPos;
        Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,CONFIGDLG3_EHEARPR,(LONG_PTR)&pos);
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}
