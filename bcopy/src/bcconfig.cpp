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
/*
  ,
  CONFIGDLG3_SETPWD,
  CONFIGDLG3_CLEARPWD
*/
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
  memset(MenuItems,0,sizeof(MenuItems));
  int Msgs[]={mConfigMenu1,mConfigMenu2,mConfigMenu3};

  for(unsigned int i=0;i<sizeofa(MenuItems);i++)
  {
    MenuItems[i].Checked=MenuItems[i].Separator=0;
    strcpy(MenuItems[i].Text,GetMsg(Msgs[i]));
  };
  int MenuCode=0;
  while(TRUE)
  {
    for(unsigned int i=0;i<sizeofa(MenuItems);i++)
      MenuItems[i].Selected=0;
    MenuItems[MenuCode].Selected=TRUE;
    // Show menu
    MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_WRAPMODE,GetMsg(mName),NULL,"Config",NULL,NULL,MenuItems,sizeofa(MenuItems));
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
      /* 0*/  {DI_DOUBLEBOX,3,1,72,23,0,0,0,0,(char *)mName},
      /* 1*/  {DI_CHECKBOX,5,2,0,0,1,0,0,0,(char *)mConfigHistory},
      /* 2*/  {DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)mConfigFullInfo},

      /* 3*/  {DI_TEXT,-1,4,0,0,0,0,DIF_SEPARATOR,0,""},
      /* 4*/  {DI_RADIOBUTTON,5,5,0,0,0,0,DIF_GROUP,0,(char *)mCpyDlgOpt1},
      /* 5*/  {DI_RADIOBUTTON,5,6,0,0,0,0,0,0,(char *)mCpyDlgOpt2},
      /* 6*/  {DI_RADIOBUTTON,5,7,0,0,0,0,0,0,(char *)mCpyDlgOpt3},
      /* 7*/  {DI_RADIOBUTTON,5,8,0,0,0,0,0,0,(char *)mCpyDlgOpt4},
      /* 8*/  {DI_RADIOBUTTON,5,9,0,0,0,0,0,0,(char *)mCpyDlgAsk},
      /* 9*/  {DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)mCpyDlgChkAccess},
      /*10*/  {DI_CHECKBOX,5,12,0,0,0,0,0,0,(char *)mCpyDlgChk2},
      /*11*/  {DI_CHECKBOX,5,13,0,0,0,0,0,0,(char *)mCpyDlgChk3},
      /*12*/  {DI_CHECKBOX,5,14,0,0,0,0,DIF_3STATE,0,(char *)mCpyDlgChk4},
      /*13*/  {DI_CHECKBOX,5,15,0,0,0,0,DIF_3STATE,0,(char *)mCpyDlgChkLink},

      /*14*/  {DI_TEXT,-1,16,0,0,0,0,DIF_SEPARATOR,0,""},
      /*15*/  {DI_CHECKBOX,5,17,0,0,0,0,0,0,(char *)mDelDlgChk1},
      /*16*/  {DI_CHECKBOX,5,18,0,0,0,0,DIF_3STATE,0,(char *)mConfigDelAbort},

      /*17*/  {DI_TEXT,-1,19,0,0,0,0,DIF_SEPARATOR,0,""},
      /*18*/  {DI_CHECKBOX,5,20,0,0,0,0,DIF_3STATE,0,(char *)mConfigWpeAbort},
      /*19*/  {DI_TEXT,-1,21,0,0,0,0,DIF_SEPARATOR,0,""},
      /*20*/  {DI_BUTTON,0,22,0,0,0,0,DIF_CENTERGROUP,1,(char *)mConfigSave},
      /*21*/  {DI_BUTTON,0,22,0,0,0,0,DIF_CENTERGROUP,0,(char *)mConfigCancel}
      };

      struct FarDialogItem DialogItems[sizeofa(InitItems)];
      InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));

      strcpy(DialogItems[CONFIGDLG_SEPCOPY].Data," ");
      strcat(DialogItems[CONFIGDLG_SEPCOPY].Data,GetMsg(mCpyDlgCopyTitle));
      strcat(DialogItems[CONFIGDLG_SEPCOPY].Data," ");
      strcpy(DialogItems[CONFIGDLG_SEPDEL].Data," ");
      strcat(DialogItems[CONFIGDLG_SEPDEL].Data,GetMsg(mDelDlgTitle));
      strcat(DialogItems[CONFIGDLG_SEPDEL].Data," ");
      strcpy(DialogItems[CONFIGDLG_SEPWIPE].Data," ");
      strcat(DialogItems[CONFIGDLG_SEPWIPE].Data,GetMsg(mWpeDlgTitle));
      strcat(DialogItems[CONFIGDLG_SEPWIPE].Data," ");
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

      int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,76,25,"Config1",DialogItems,sizeofa(DialogItems),0,0,Config1DialogProc,0);
      if(DlgCode==CONFIGDLG_SAVE)
      {
        Opt.CopyType=0;
        int i=CONFIGDLG_OVERWRITE;
        while(!DialogItems[i].Selected) {i++; Opt.CopyType++;}
        Opt.CopyHistory=DialogItems[CONFIGDLG_HISTORY].Selected;
        Opt.CopyFullInfo=DialogItems[CONFIGDLG_FULLINFO].Selected;
        Opt.CopyAccess=DialogItems[CONFIGDLG_ACCESS].Selected;
        Opt.CopyROSrc=DialogItems[CONFIGDLG_ROSRC].Selected;
        Opt.CopyRODest=DialogItems[CONFIGDLG_RODST].Selected;
        Opt.CopyAbort=DialogItems[CONFIGDLG_ABORT].Selected;
        Opt.CopyLink=DialogItems[CONFIGDLG_LINK].Selected;

        Opt.DeleteRO=DialogItems[CONFIGDLG_DELRO].Selected;
        Opt.DeleteAbort=DialogItems[CONFIGDLG_DELABORT].Selected;

        Opt.WipeAbort=DialogItems[CONFIGDLG_WIPEABORT].Selected;

        HKEY hKey;
        DWORD Disposition;
        if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
        {
          RegSetValueEx(hKey,"CopyType",0,REG_DWORD,(LPBYTE)&Opt.CopyType,sizeof(Opt.CopyType));
          RegSetValueEx(hKey,"CopyHistory",0,REG_DWORD,(LPBYTE)&Opt.CopyHistory,sizeof(Opt.CopyHistory));
          RegSetValueEx(hKey,"CopyROSrc",0,REG_DWORD,(LPBYTE)&Opt.CopyROSrc,sizeof(Opt.CopyROSrc));
          RegSetValueEx(hKey,"CopyRODest",0,REG_DWORD,(LPBYTE)&Opt.CopyRODest,sizeof(Opt.CopyRODest));
          RegSetValueEx(hKey,"CopyAbort",0,REG_DWORD,(LPBYTE)&Opt.CopyAbort,sizeof(Opt.CopyAbort));
          RegSetValueEx(hKey,"CopyLink",0,REG_DWORD,(LPBYTE)&Opt.CopyLink,sizeof(Opt.CopyLink));
          RegSetValueEx(hKey,"CopyFullInfo",0,REG_DWORD,(LPBYTE)&Opt.CopyFullInfo,sizeof(Opt.CopyFullInfo));
          RegSetValueEx(hKey,"CopyAccess",0,REG_DWORD,(LPBYTE)&Opt.CopyAccess,sizeof(Opt.CopyAccess));
          RegSetValueEx(hKey,"DeleteRO",0,REG_DWORD,(LPBYTE)&Opt.DeleteRO,sizeof(Opt.DeleteRO));
          RegSetValueEx(hKey,"DeleteAbort",0,REG_DWORD,(LPBYTE)&Opt.DeleteAbort,sizeof(Opt.DeleteAbort));
          RegSetValueEx(hKey,"WipeAbort",0,REG_DWORD,(LPBYTE)&Opt.WipeAbort,sizeof(Opt.WipeAbort));
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
      /* 0*/  {DI_DOUBLEBOX,3,1,72,22,0,0,0,0,(char *)mName},
      /* 1*/  {DI_CHECKBOX,5,2,0,0,1,0,0,0,(char *)mConfigAutostart},
      /* 2*/  {DI_TEXT,-1,3,0,0,0,0,DIF_SEPARATOR,0,""},
      /* 3*/  {DI_CHECKBOX,5,4,0,0,0,0,0,0,(char *)mConfigConfigMenu},
      /* 4*/  {DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)mConfigViewerMenu},
      /* 5*/  {DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)mConfigEditorMenu},
      /* 5*/  {DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)mConfigDialogMenu},
      /* 6*/  {DI_TEXT,-1,8,0,0,0,0,DIF_SEPARATOR,0,""},
      /* 7*/  {DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)mConfigInfoEmpty},
      /* 8*/  {DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)mConfigInfoOnly},
      /* 9*/  {DI_TEXT,-1,11,0,0,0,0,DIF_SEPARATOR,0,""},
      /*10*/  {DI_CHECKBOX,5,12,0,0,0,0,0,0,(char *)mComfigAutoShowInfo},
      /*11*/  {DI_CHECKBOX,5,13,0,0,0,0,0,0,(char *)mConfigExpandVars},
      /*12*/  {DI_CHECKBOX,5,14,0,0,0,0,0,0,(char *)mConfigPassive},
      /*13*/  {DI_CHECKBOX,5,15,0,0,0,0,0,0,(char *)mConfigFormatSize},
      /*14*/  {DI_CHECKBOX,5,16,0,0,0,0,0,0,(char *)mConfigCurrentTime},
      /*15*/  {DI_CHECKBOX,5,17,0,0,0,0,0,0,(char *)mConfigConfirmAbort},
      /*16*/  {DI_CHECKBOX,5,18,0,0,0,0,0,0,(char *)mConfigAutoRefresh},
      /*17*/  {DI_CHECKBOX,5,19,0,0,0,0,0,0,(char *)mConfigResolveDestination},
      /*18*/  {DI_TEXT,-1,20,0,0,0,0,DIF_SEPARATOR,0,""},
      /*19*/  {DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,1,(char *)mConfigSave},
      /*20*/  {DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,0,(char *)mConfigCancel},
      /*21*/  {DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(char *)mColorMain}
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
      strcpy(DialogItems[CONFIGDLG2_SEP2].Data," ");
      strcat(DialogItems[CONFIGDLG2_SEP2].Data,GetMsg(mConfigInfo));
      strcat(DialogItems[CONFIGDLG2_SEP2].Data," ");

      int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,76,24,"Config2",DialogItems,sizeofa(DialogItems),0,0,Config2DialogProc,0);
      if(DlgCode==CONFIGDLG2_SAVE)
      {
        PlgOpt.AutoStart=DialogItems[CONFIGDLG2_AUTOSTART].Selected;
        PlgOpt.ShowMenu=0;
        if(DialogItems[CONFIGDLG2_CONFIGMENU].Selected)
          PlgOpt.ShowMenu|=SHOW_IN_CONFIG;
        if(DialogItems[CONFIGDLG2_VIEWERMENU].Selected)
          PlgOpt.ShowMenu|=SHOW_IN_VIEWER;
        if(DialogItems[CONFIGDLG2_EDITORMENU].Selected)
          PlgOpt.ShowMenu|=SHOW_IN_EDITOR;
        if(DialogItems[CONFIGDLG2_DIALOGMENU].Selected)
          PlgOpt.ShowMenu|=SHOW_IN_DIALOG;
        PlgOpt.InfoMenu=0;
        if(DialogItems[CONFIGDLG2_INFOEMPTY].Selected)
          PlgOpt.InfoMenu|=INFO_MENU_ALT_0;
        if(DialogItems[CONFIGDLG2_INFOONLY].Selected)
          PlgOpt.InfoMenu|=INFO_MENU_ALT_1;
        PlgOpt.AutoShowInfo=DialogItems[CONFIGDLG2_AUTOSHOWINFO].Selected;
        PlgOpt.ExpandVars=DialogItems[CONFIGDLG2_EXPANDVARS].Selected;
        PlgOpt.CheckPassive=DialogItems[CONFIGDLG2_PASSIVE].Selected;
        PlgOpt.FormatSize=DialogItems[CONFIGDLG2_FORMATSIZE].Selected;
        PlgOpt.CurrentTime=DialogItems[CONFIGDLG2_CURRENTTIME].Selected;
        PlgOpt.ConfirmAbort=DialogItems[CONFIGDLG2_CONFIRMABORT].Selected;
        PlgOpt.AutoRefresh=DialogItems[CONFIGDLG2_AUTOREFRESH].Selected;
        PlgOpt.ResolveDestination=DialogItems[CONFIGDLG2_RESOLVEDESTINATION].Selected;
        HKEY hKey;
        DWORD Disposition;
        if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
        {
          RegSetValueEx(hKey,"TechAutoStart",0,REG_DWORD,(LPBYTE)&PlgOpt.AutoStart,sizeof(PlgOpt.AutoStart));
          RegSetValueEx(hKey,"TechShowMenu",0,REG_DWORD,(LPBYTE)&PlgOpt.ShowMenu,sizeof(PlgOpt.ShowMenu));
          RegSetValueEx(hKey,"TechInfoMenu",0,REG_DWORD,(LPBYTE)&PlgOpt.InfoMenu,sizeof(PlgOpt.InfoMenu));
          RegSetValueEx(hKey,"TechExpandVars",0,REG_DWORD,(LPBYTE)&PlgOpt.ExpandVars,sizeof(PlgOpt.ExpandVars));
          RegSetValueEx(hKey,"TechCheckPassive",0,REG_DWORD,(LPBYTE)&PlgOpt.CheckPassive,sizeof(PlgOpt.CheckPassive));
          RegSetValueEx(hKey,"TechFormatSize",0,REG_DWORD,(LPBYTE)&PlgOpt.FormatSize,sizeof(PlgOpt.FormatSize));
          RegSetValueEx(hKey,"TechAutoShowInfo",0,REG_DWORD,(LPBYTE)&PlgOpt.AutoShowInfo,sizeof(PlgOpt.AutoShowInfo));
          RegSetValueEx(hKey,"TechConfirmAbort",0,REG_DWORD,(LPBYTE)&PlgOpt.ConfirmAbort,sizeof(PlgOpt.ConfirmAbort));
          RegSetValueEx(hKey,"TechCurrentTime",0,REG_DWORD,(LPBYTE)&PlgOpt.CurrentTime,sizeof(PlgOpt.CurrentTime));
          RegSetValueEx(hKey,"TechAutoRefresh",0,REG_DWORD,(LPBYTE)&PlgOpt.AutoRefresh,sizeof(PlgOpt.AutoRefresh));
          RegSetValueEx(hKey,"TechResolveDestination",0,REG_DWORD,(LPBYTE)&PlgOpt.ResolveDestination,sizeof(PlgOpt.ResolveDestination));
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
      /*00*/  {DI_DOUBLEBOX,3,1,73,10,0,0,0,0,(char *)mName},
      /*01*/  {DI_TEXT,5,2,0,0,0,0,0,0,(char *)mConfigMaxErrors},
      /*02*/  {DI_FIXEDIT,0,2,0,0,1,(DWORD_PTR)"#####9",DIF_MASKEDIT,0,"20"},
      /*03*/  {DI_TEXT,5,3,0,0,0,0,0,0,(char *)mConfigThreadCount},
      /*04*/  {DI_FIXEDIT,0,3,0,0,0,(DWORD_PTR)"#9",DIF_MASKEDIT,0," 4"},
      /*05*/  {DI_TEXT,5,4,0,0,0,0,0,0,(char *)mConfigQueueSize},
      /*06*/  {DI_FIXEDIT,0,4,0,0,0,(DWORD_PTR)"#####9",DIF_MASKEDIT,0," 4"},
      /*07*/  {DI_TEXT,5,5,0,0,0,0,0,0,(char *)mConfigWork},
      /*08*/  {DI_COMBOBOX,0,5,71,0,0,0,DIF_DROPDOWNLIST,0,""},
      /*09*/  {DI_TEXT,5,6,0,0,0,0,0,0,(char *)mConfigHear},
      /*10*/  {DI_COMBOBOX,0,6,71,0,0,0,DIF_DROPDOWNLIST,0,""},
      /*11*/  {DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)mConfigAllowNetwork},
      /*12*/  {DI_TEXT,-1,8,0,0,0,0,DIF_SEPARATOR,0,""},
      /*13*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,1,(char *)mConfigSave},
      /*14*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(char *)mConfigCancel},
//      /*15*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(char *)mConfigSetPassword},
//      /*16*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(char *)mConfigClearPassword},
      };
      struct FarDialogItem DialogItems[sizeofa(InitItems)];
      InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));
      unsigned int max=0;
      {
        int labels[]={CONFIGDLG3_LMAXERROR,CONFIGDLG3_LTHREAD,CONFIGDLG3_LQUEUESIZE,CONFIGDLG3_LWORKPR,CONFIGDLG3_LHEARPR};
        for(unsigned int i=0;i<sizeofa(labels);i++)
        {
          unsigned int cur_len=(unsigned int)strlen(DialogItems[labels[i]].Data);
          if(max<cur_len) max=cur_len;
        }
      }
      DialogItems[CONFIGDLG3_EMAXERROR].X1=DialogItems[CONFIGDLG3_LMAXERROR].X1+max; DialogItems[CONFIGDLG3_EMAXERROR].X2=DialogItems[CONFIGDLG3_EMAXERROR].X1+5;
      DialogItems[CONFIGDLG3_ETHREAD].X1=DialogItems[CONFIGDLG3_LTHREAD].X1+max; DialogItems[CONFIGDLG3_ETHREAD].X2=DialogItems[CONFIGDLG3_ETHREAD].X1+1;
      DialogItems[CONFIGDLG3_EQUEUESIZE].X1=DialogItems[CONFIGDLG3_LQUEUESIZE].X1+max; DialogItems[CONFIGDLG3_EQUEUESIZE].X2=DialogItems[CONFIGDLG3_EQUEUESIZE].X1+5;
      DialogItems[CONFIGDLG3_EWORKPR].X1=DialogItems[CONFIGDLG3_LWORKPR].X1+max;
      DialogItems[CONFIGDLG3_EHEARPR].X1=DialogItems[CONFIGDLG3_LHEARPR].X1+max;

      FarListItem Priority[7];
      FarList Priorities={sizeofa(Priority),Priority};
      int PriorityValues[]={THREAD_PRIORITY_IDLE,THREAD_PRIORITY_LOWEST,THREAD_PRIORITY_BELOW_NORMAL,THREAD_PRIORITY_NORMAL,THREAD_PRIORITY_ABOVE_NORMAL,THREAD_PRIORITY_HIGHEST,THREAD_PRIORITY_TIME_CRITICAL};
      int WorkPriority=THREAD_PRIORITY_IDLE,HearPriority=THREAD_PRIORITY_HIGHEST;
      ComboPos data={0,5};
      for(unsigned int i=0;i<sizeofa(Priority);i++)
      {
        Priority[i].Flags=0;
        strcpy(Priority[i].Text,GetMsg(mPriorityIdle+i));
      }
      DialogItems[CONFIGDLG3_EWORKPR].ListItems=&Priorities;
      DialogItems[CONFIGDLG3_EHEARPR].ListItems=&Priorities;
      strcpy(DialogItems[CONFIGDLG3_EWORKPR].Data,GetMsg(mPriorityIdle));
      strcpy(DialogItems[CONFIGDLG3_EHEARPR].Data,GetMsg(mPriorityHighest));

      HKEY hKey; DWORD Type,DataSize; DWORD Value; DWORD Disposition;
      if((RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\"SVC_NAME"\\Parameters",0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
      {
        DataSize=sizeof(Value); Value=20;
        RegQueryValueEx(hKey,"MaxError",0,&Type,(LPBYTE)&Value,&DataSize);
        sprintf(DialogItems[CONFIGDLG3_EMAXERROR].Data,"%6ld",Value);
        DataSize=sizeof(Value); Value=4;
        RegQueryValueEx(hKey,"ThreadCount",0,&Type,(LPBYTE)&Value,&DataSize);
        if((Value>16)||(Value<1)) Value=4;
        sprintf(DialogItems[CONFIGDLG3_ETHREAD].Data,"%2ld",Value);
        DataSize=sizeof(Value); Value=4;
        RegQueryValueEx(hKey,"QueueSize",0,&Type,(LPBYTE)&Value,&DataSize);
        sprintf(DialogItems[CONFIGDLG3_EQUEUESIZE].Data,"%6ld",Value);
        DataSize=sizeof(Value); Value=0;
        RegQueryValueEx(hKey,"AllowNetwork",0,&Type,(LPBYTE)&Value,&DataSize);
        DialogItems[CONFIGDLG3_NETWORK].Selected=Value;
        DataSize=sizeof(WorkPriority);
        RegQueryValueEx(hKey,"WorkPriority",0,&Type,(LPBYTE)&WorkPriority,&DataSize);
        DataSize=sizeof(HearPriority);
        RegQueryValueEx(hKey,"HearPriority",0,&Type,(LPBYTE)&HearPriority,&DataSize);
      }
      for(unsigned int i=0;i<sizeofa(PriorityValues);i++)
      {
        if(WorkPriority==PriorityValues[i]) data.WorkPos=i;
        if(HearPriority==PriorityValues[i]) data.HearPos=i;
      }
      int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,77,12,"Config3",DialogItems,sizeofa(DialogItems),0,0,Config3DialogProc,(LONG_PTR)&data);
      if(DlgCode==CONFIGDLG3_SAVE)
      {
        if((RegCreateKeyEx(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\"SVC_NAME"\\Parameters",0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
        {
          Value=atoi(DialogItems[CONFIGDLG3_EMAXERROR].Data);
          RegSetValueEx(hKey,"MaxError",0,REG_DWORD,(LPBYTE)&Value,sizeof(Value));
          Value=atoi(DialogItems[CONFIGDLG3_ETHREAD].Data); if((Value<1)||(Value>16)) Value=4;
          RegSetValueEx(hKey,"ThreadCount",0,REG_DWORD,(LPBYTE)&Value,sizeof(Value));
          Value=atoi(DialogItems[CONFIGDLG3_EQUEUESIZE].Data);
          RegSetValueEx(hKey,"QueueSize",0,REG_DWORD,(LPBYTE)&Value,sizeof(Value));
          Value=DialogItems[CONFIGDLG3_NETWORK].Selected;
          RegSetValueEx(hKey,"AllowNetwork",0,REG_DWORD,(LPBYTE)&Value,sizeof(Value));
          WorkPriority=PriorityValues[DialogItems[CONFIGDLG3_EWORKPR].ListPos];
          HearPriority=PriorityValues[DialogItems[CONFIGDLG3_EHEARPR].ListPos];
          RegSetValueEx(hKey,"WorkPriority",0,REG_DWORD,(LPBYTE)&WorkPriority,sizeof(WorkPriority));
          RegSetValueEx(hKey,"HearPriority",0,REG_DWORD,(LPBYTE)&HearPriority,sizeof(HearPriority));
          RegCloseKey(hKey);
        }
        {
          const char *MsgItems[]={"",GetMsg(mActualize),GetMsg(mOk)};
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
            RegSetValueEx(hKey,"TechErrorColor",0,REG_DWORD,(LPBYTE)&PlgOpt.ErrorColor,sizeof(PlgOpt.ErrorColor));
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
/*
    case DN_BTNCLICK:
      if((Param1==CONFIGDLG3_SETPWD)||(Param1==CONFIGDLG3_CLEARPWD))
      {
        if(!CheckPipeEx())
        {
          ShowError(mErrorNoPipe,false);
          break;
        }
        char send[256+sizeof(DWORD)*3];
        DWORD *send_ptr=(DWORD *)send,send_size=sizeof(DWORD)*2;
        send_ptr[0]=OPERATION_PWD;
        if(Param1==CONFIGDLG3_SETPWD) //set password
        {
          send_ptr[1]=PWDFLAG_SET;
          FarDialogItem DialogItems[5]; int i=0;
          memset(DialogItems,0,sizeof(DialogItems));
          DialogItems[i].X1=3; DialogItems[i].X2=36; DialogItems[i].Y1=1; DialogItems[i].Y2=5;       //0
          DialogItems[i].Type=DI_DOUBLEBOX; strcpy(DialogItems[i].Data,GetMsg(mPwdTitle)); i++;
          DialogItems[i].X1=5; DialogItems[i].X2=34; DialogItems[i].Y1=2; DialogItems[i].Focus=1;    //1
          DialogItems[i].Type=DI_PSWEDIT; i++;
          DialogItems[i].X1=-1; DialogItems[i].Y1=3; DialogItems[i].Flags=DIF_SEPARATOR;             //2
          DialogItems[i].Type=DI_TEXT; i++;
          DialogItems[i].Y1=4; DialogItems[i].Flags=DIF_CENTERGROUP; DialogItems[i].DefaultButton=1; //3
          DialogItems[i].Type=DI_BUTTON; strcpy(DialogItems[i].Data,GetMsg(mPwdSet)); i++;
          DialogItems[i].Y1=4; DialogItems[i].Flags=DIF_CENTERGROUP;                                 //4
          DialogItems[i].Type=DI_BUTTON; strcpy(DialogItems[i].Data,GetMsg(mPwdCancel)); i++;
          int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,40,7,"Config4",DialogItems,sizeofa(DialogItems),0,0,PwdDialogProc,0);
          if(DlgCode!=3) break;
          send_ptr[2]=strlen(DialogItems[1].Data)+1;
          send_size=send_ptr[2]+sizeof(DWORD)*3;
          strcpy(send+sizeof(DWORD)*3,DialogItems[1].Data);
        }
        else if(Param1==CONFIGDLG3_CLEARPWD) //set password
        {
          send_ptr[1]=PWDFLAG_CLEAR;
        }
        DWORD dwBytesRead,dwBytesWritten;
        HANDLE hPipe=CreateFile(PIPE_NAME,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
        DWORD error=0; BOOL Succ=FALSE;
        if(hPipe!=INVALID_HANDLE_VALUE)
        {
          if(WriteFile(hPipe,send_ptr,send_size,&dwBytesWritten,NULL))
            if(ReadFile(hPipe,&error,sizeof(error),&dwBytesRead,NULL))
              if(!error)
                Succ=TRUE;
          CloseHandle(hPipe);
        }
        if(Succ)
        {
          const char *MsgItems[]={"",GetMsg(mPwdCleared),GetMsg(mOk)};
          if(Param1==CONFIGDLG3_SETPWD)
            MsgItems[1]=GetMsg(mPwdSeted);
          Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeofa(MsgItems),1);
        }
        else
        {
          if(error) SetLastError(error);
          const char *MsgItems[]={GetMsg(mError),GetMsg(mOk)};
          Info.Message(Info.ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,sizeofa(MsgItems),1);
        }
      }
      break;
*/
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}
