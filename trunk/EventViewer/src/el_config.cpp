static long WINAPI ConfigDialogProc(HANDLE hDlg, int Msg,int Param1,long Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

enum
{
  CONFIG_BORDER=0,
  CONFIG_ADDDISK,
  CONFIG_DISKHOTKEY,
  CONFIG_DISKHOTKEYLABEL,
  CONFIG_ADDMENU,
  CONFIG_SEP1,
  CONFIG_BROWSEEVT,
  CONFIG_STRIPEXT,
  CONFIG_RESTORE,
  CONFIG_SEP2,
  CONFIG_FORWARD,
  CONFIG_BACKWARD,
  CONFIG_SHOWHEADER,
  CONFIG_SHOWDESC,
  CONFIG_SHOWDATA,
  CONFIG_SEP3,
  CONFIG_VSEP,
  CONFIG_PREFIXLABEL,
  CONFIG_PREFIX,
  CONFIG_SEP4,
  CONFIG_SAVE,
  CONFIG_CANCEL,
};

static int Config()
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
  05   วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ   05
  06   บ [ ] Browse .evt files                                              บ   06
  07   บ [ ] Strip second extension                                         บ   07
  08   วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ   08
  09   บ (*) Scan forward                                                   บ   09
  10   บ ( ) Scan backward                                                  บ   10
  11   วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ   11
  12   บ Command line prefix to start the nt events:                        บ   12
  13   บ evt                                                                บ   13
  14   วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ   14
  15   บ                         [ Ok ]  [ Cancel ]                         บ   15
  16   ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ   16
  17                                                                            17
    0000000000111111111122222222223333333333444444444455555555556666666666777777
    0123456789012345678901234567890123456789012345678901234567890123456789012345
  */

  static struct InitDialogItem InitItems[]={
  /* 0*/  {DI_DOUBLEBOX,3,1,72,18,0,0,0,0,(TCHAR *)mName},
  /* 1*/  {DI_CHECKBOX,5,2,0,0,0,0,0,0,(TCHAR *)mConfigAddToDisksMenu},
  /* 2*/  {DI_FIXEDIT,7,3,7,3,1,0,0,0,_T("")},
  /* 3*/  {DI_TEXT,9,3,0,0,0,0,0,0,(TCHAR *)mConfigDisksMenuDigit},
  /* 4*/  {DI_CHECKBOX,5,4,0,0,0,0,0,0,(TCHAR *)mConfigAddToPluginMenu},
  /* 5*/  {DI_TEXT,-1,5,0,0,0,0,DIF_SEPARATOR,0,_T("")},
  /* 6*/  {DI_CHECKBOX,5,6,0,0,0,0,0,0,(TCHAR *)mConfigBrowseEvtFiles},
  /* 7*/  {DI_CHECKBOX,5,7,0,0,0,0,0,0,(TCHAR *)mConfigStripExtension},
  /* 7*/  {DI_CHECKBOX,5,8,0,0,0,0,0,0,(TCHAR *)mConfigRestore},
  /* 8*/  {DI_TEXT,-1,9,0,0,0,0,DIF_SEPARATOR,0,_T("")},
  /* 9*/  {DI_RADIOBUTTON,5,10,0,0,0,0,DIF_GROUP,0,(TCHAR *)mConfigScan1},
  /*10*/  {DI_RADIOBUTTON,5,11,0,0,0,0,0,0,(TCHAR *)mConfigScan2},
  /*10*/  {DI_CHECKBOX,39,10,0,0,0,0,0,0,(TCHAR *)mConfigShowHeader},
  /*10*/  {DI_CHECKBOX,39,11,0,0,0,0,0,0,(TCHAR *)mConfigShowDescription},
  /*10*/  {DI_CHECKBOX,39,12,0,0,0,0,0,0,(TCHAR *)mConfigShowData},
  /*11*/  {DI_TEXT,-1,13,0,0,0,0,DIF_SEPARATOR,0,_T("")},
#ifdef UNICODE
  /* 3*/  {DI_VTEXT,37,9,0,0,0,0,0,0,_T("xxxxx")},
#else
  /* 3*/  {DI_VTEXT,37,9,0,0,0,0,0,0,_T("ยณณณม")},
#endif
  /*12*/  {DI_TEXT,5,14,0,0,0,0,0,0,(TCHAR *)mConfigPrefix},
  /*13*/  {DI_FIXEDIT,5,15,19,12,0,(int)_T("AAAAAAAAAAAAAAA"),DIF_MASKEDIT,0,_T("")},
  /*14*/  {DI_TEXT,-1,16,0,0,0,0,DIF_SEPARATOR,0,_T("")},
  /*15*/  {DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)mConfigSave},
  /*16*/  {DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)mConfigCancel}
  };
  struct FarDialogItem DialogItems[ArraySize(InitItems)];
  InitDialogItems(InitItems,DialogItems,ArraySize(InitItems));
#ifdef UNICODE
  TCHAR DisksMenuDigitText[21],PrefixText[21];
#endif

  DialogItems[CONFIG_ADDDISK].Selected=Opt.AddToDisksMenu;
  if (Opt.DisksMenuDigit)
  {
#ifdef UNICODE
    FSF.sprintf(DisksMenuDigitText,_T("%d"),Opt.DisksMenuDigit);
    DialogItems[CONFIG_DISKHOTKEY].PtrData=DisksMenuDigitText;
#else
    FSF.sprintf(DialogItems[CONFIG_DISKHOTKEY].Data,_T("%d"),Opt.DisksMenuDigit);
#endif
  }
  DialogItems[CONFIG_ADDMENU].Selected=Opt.AddToPluginsMenu;
  DialogItems[CONFIG_BROWSEEVT].Selected=Opt.BrowseEvtFiles;
  DialogItems[CONFIG_STRIPEXT].Selected=Opt.StripExt;
  DialogItems[CONFIG_RESTORE].Selected=Opt.Restore;
  DialogItems[CONFIG_FORWARD+Opt.ScanType].Selected=1;
#ifdef UNICODE
  FSF.sprintf(PrefixText,_T("%s"),Opt.Prefix);
  DialogItems[CONFIG_PREFIX].PtrData=PrefixText;
#else
  FSF.sprintf(DialogItems[CONFIG_PREFIX].Data,_T("%s"),Opt.Prefix);
#endif
  DialogItems[CONFIG_SHOWHEADER].Selected=QVOpt.ShowHeader;
  DialogItems[CONFIG_SHOWDESC].Selected=QVOpt.ShowDescription;
  DialogItems[CONFIG_SHOWDATA].Selected=QVOpt.ShowData;

  CFarDialog dialog;
  int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,76,20,_T("Config"),DialogItems,ArraySize(DialogItems),0,0,ConfigDialogProc,0);
  if (DlgCode!=CONFIG_SAVE)
    return(FALSE);
  Opt.AddToDisksMenu=dialog.Check(CONFIG_ADDDISK);
  Opt.DisksMenuDigit=FSF.atoi(dialog.Str(CONFIG_DISKHOTKEY));
  Opt.AddToPluginsMenu=dialog.Check(CONFIG_ADDMENU);
  Opt.BrowseEvtFiles=dialog.Check(CONFIG_BROWSEEVT);
  Opt.StripExt=dialog.Check(CONFIG_STRIPEXT);
  Opt.Restore=dialog.Check(CONFIG_RESTORE);
  QVOpt.ShowHeader=dialog.Check(CONFIG_SHOWHEADER);
  QVOpt.ShowDescription=dialog.Check(CONFIG_SHOWDESC);
  QVOpt.ShowData=dialog.Check(CONFIG_SHOWDATA);
  _tcscpy(Opt.Prefix,dialog.Str(CONFIG_PREFIX));
  FSF.Trim(Opt.Prefix);
  int i=CONFIG_FORWARD;
  Opt.ScanType=0;
  while(!dialog.Check(i)) {i++; Opt.ScanType++;}
  HKEY hKey;
  DWORD Disposition;
  if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
  {
    RegSetValueEx(hKey,_T("AddToDisksMenu"),0,REG_DWORD,(LPBYTE)&Opt.AddToDisksMenu,sizeof(Opt.AddToDisksMenu));
    RegSetValueEx(hKey,_T("DisksMenuDigit"),0,REG_DWORD,(LPBYTE)&Opt.DisksMenuDigit,sizeof(Opt.DisksMenuDigit));
    RegSetValueEx(hKey,_T("AddToPluginsMenu"),0,REG_DWORD,(LPBYTE)&Opt.AddToPluginsMenu,sizeof(Opt.AddToPluginsMenu));
    RegSetValueEx(hKey,_T("BrowseEvtFiles"),0,REG_DWORD,(LPBYTE)&Opt.BrowseEvtFiles,sizeof(Opt.BrowseEvtFiles));
    RegSetValueEx(hKey,_T("StripExt"),0,REG_DWORD,(LPBYTE)&Opt.StripExt,sizeof(Opt.StripExt));
    RegSetValueEx(hKey,_T("ScanType"),0,REG_DWORD,(LPBYTE)&Opt.ScanType,sizeof(Opt.ScanType));
    RegSetValueEx(hKey,_T("Prefix"),0,REG_SZ,(LPBYTE)Opt.Prefix,(_tcslen(Opt.Prefix)+1)*sizeof(TCHAR));

    RegSetValueEx(hKey,_T("Restore"),0,REG_DWORD,(LPBYTE)&Opt.Restore,sizeof(Opt.Restore));
    RegSetValueEx(hKey,_T("ShowHeader"),0,REG_DWORD,(LPBYTE)&QVOpt.ShowHeader,sizeof(QVOpt.ShowHeader));
    RegSetValueEx(hKey,_T("ShowDescription"),0,REG_DWORD,(LPBYTE)&QVOpt.ShowDescription,sizeof(QVOpt.ShowDescription));
    RegSetValueEx(hKey,_T("ShowData"),0,REG_DWORD,(LPBYTE)&QVOpt.ShowData,sizeof(QVOpt.ShowData));
    RegCloseKey(hKey);
  }
  return(TRUE);
}
