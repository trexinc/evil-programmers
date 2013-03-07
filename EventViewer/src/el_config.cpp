static intptr_t WINAPI ConfigDialogProc(HANDLE hDlg, intptr_t Msg,intptr_t Param1,void* Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

enum
{
  CONFIG_BORDER=0,
  CONFIG_ADDDISK,
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
  03   บ [ ] Add to Plugins menu                                            บ   03
  04   วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ   04
  05   บ [ ] Browse .evt files                                              บ   05
  06   บ [ ] Strip second extension                                         บ   06
  07   วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ   07
  08   บ (*) Scan forward                                                   บ   08
  09   บ ( ) Scan backward                                                  บ   09
  10   วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ   10
  11   บ Command line prefix to start the nt events:                        บ   11
  12   บ evt                                                                บ   12
  13   วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ   13
  14   บ                         [ Ok ]  [ Cancel ]                         บ   14
  15   ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ   15
  16                                                                            16
    0000000000111111111122222222223333333333444444444455555555556666666666777777
    0123456789012345678901234567890123456789012345678901234567890123456789012345
  */
  static struct FarDialogItem DialogItems[]={
  //        Type           X1 Y1   X2  Y2 Sel History                     Mask Flags                              Data
  /* 0*/  {DI_DOUBLEBOX,    3,  1, 72, 17,{0},0,                          0,   0,                                 GetMsg(mName),                  0,0},
  /* 1*/  {DI_CHECKBOX,     5,  2,  0,  0,{0},0,                          0,   0,                                 GetMsg(mConfigAddToDisksMenu),  0,0},
  /* 2*/  {DI_CHECKBOX,     5,  3,  0,  0,{0},0,                          0,   0,                                 GetMsg(mConfigAddToPluginMenu), 0,0},
  /* 3*/  {DI_TEXT,        -1,  4,  0,  0,{0},0,                          0,   DIF_SEPARATOR,                     L"",                            0,0},
  /* 4*/  {DI_CHECKBOX,     5,  5,  0,  0,{0},0,                          0,   0,                                 GetMsg(mConfigBrowseEvtFiles),  0,0},
  /* 5*/  {DI_CHECKBOX,     5,  6,  0,  0,{0},0,                          0,   0,                                 GetMsg(mConfigStripExtension),  0,0},
  /* 6*/  {DI_CHECKBOX,     5,  7,  0,  0,{0},0,                          0,   0,                                 GetMsg(mConfigRestore),         0,0},
  /* 7*/  {DI_TEXT,        -1,  8,  0,  0,{0},0,                          0,   DIF_SEPARATOR,                     L"",                            0,0},
  /* 8*/  {DI_RADIOBUTTON,  5,  9,  0,  0,{0},0,                          0,   DIF_GROUP,                         GetMsg(mConfigScan1),           0,0},
  /* 9*/  {DI_RADIOBUTTON,  5, 10,  0,  0,{0},0,                          0,   0,                                 GetMsg(mConfigScan2),           0,0},
  /*11*/  {DI_CHECKBOX,    39,  9,  0,  0,{0},0,                          0,   0,                                 GetMsg(mConfigShowHeader),      0,0},
  /*12*/  {DI_CHECKBOX,    39, 10,  0,  0,{0},0,                          0,   0,                                 GetMsg(mConfigShowDescription), 0,0},
  /*13*/  {DI_CHECKBOX,    39, 11,  0,  0,{0},0,                          0,   0,                                 GetMsg(mConfigShowData),        0,0},
  /*14*/  {DI_TEXT,        -1, 12,  0,  0,{0},0,                          0,   DIF_SEPARATOR,                     L"",                            0,0},
  /*15*/  {DI_VTEXT,       37,  8,  0,  0,{0},0,                          0,   0,                                 L"xxxxx",                       0,0},
  /*16*/  {DI_TEXT,         5, 13,  0,  0,{0},0,                          0,   0,                                 GetMsg(mConfigPrefix),          0,0},
  /*17*/  {DI_FIXEDIT,      5, 14, 19, 12,{0},L"AAAAAAAAAAAAAAA",         0,   DIF_MASKEDIT,                      L"",                            0,0},
  /*18*/  {DI_TEXT,        -1, 15,  0,  0,{0},0,                          0,   DIF_SEPARATOR,                     L"",                            0,0},
  /*19*/  {DI_BUTTON,       0, 16,  0,  0,{0},0,                          0,   DIF_CENTERGROUP|DIF_DEFAULTBUTTON, GetMsg(mConfigSave),            0,0},
  /*20*/  {DI_BUTTON,       0, 16,  0,  0,{0},0,                          0,   DIF_CENTERGROUP,                   GetMsg(mConfigCancel),          0,0}
  };
 
  wchar_t PrefixText[21];

  DialogItems[CONFIG_ADDDISK].Selected=Opt.AddToDisksMenu;
  DialogItems[CONFIG_ADDMENU].Selected=Opt.AddToPluginsMenu;
  DialogItems[CONFIG_BROWSEEVT].Selected=Opt.BrowseEvtFiles;
  DialogItems[CONFIG_STRIPEXT].Selected=Opt.StripExt;
  DialogItems[CONFIG_RESTORE].Selected=Opt.Restore;
  // ่็-็เ static ํ๓ๆํ๎ ๎แํ๓๋่๒ü ๏๐ๅไ๛ไ๓๙ๅๅ ๑๎๑๒๎ÿํ่ๅ 
  DialogItems[CONFIG_FORWARD].Selected=0;
  DialogItems[CONFIG_FORWARD+1].Selected=0;
  DialogItems[CONFIG_FORWARD+Opt.ScanType].Selected=1;
  FSF.sprintf(PrefixText,L"%s",Opt.Prefix);
  DialogItems[CONFIG_PREFIX].Data=PrefixText;
  DialogItems[CONFIG_SHOWHEADER].Selected=QVOpt.ShowHeader;
  DialogItems[CONFIG_SHOWDESC].Selected=QVOpt.ShowDescription;
  DialogItems[CONFIG_SHOWDATA].Selected=QVOpt.ShowData;

  CFarDialog dialog;
  intptr_t DlgCode=dialog.Execute(MainGuid,ConfigGuid,-1,-1,76,19,L"Config",DialogItems,ArraySize(DialogItems),0,0,ConfigDialogProc,0);
  if (DlgCode!=CONFIG_SAVE)
    return(FALSE);
  Opt.AddToDisksMenu=dialog.Check(CONFIG_ADDDISK);
  Opt.AddToPluginsMenu=dialog.Check(CONFIG_ADDMENU);
  Opt.BrowseEvtFiles=dialog.Check(CONFIG_BROWSEEVT);
  Opt.StripExt=dialog.Check(CONFIG_STRIPEXT);
  Opt.Restore=dialog.Check(CONFIG_RESTORE);
  QVOpt.ShowHeader=dialog.Check(CONFIG_SHOWHEADER);
  QVOpt.ShowDescription=dialog.Check(CONFIG_SHOWDESC);
  QVOpt.ShowData=dialog.Check(CONFIG_SHOWDATA);
  wcscpy(Opt.Prefix,dialog.Str(CONFIG_PREFIX));
  FSF.Trim(Opt.Prefix);
  int i=CONFIG_FORWARD;
  Opt.ScanType=0;
  while(!dialog.Check(i)) {i++; Opt.ScanType++;}

  PluginSettings settings(MainGuid,Info.SettingsControl);
  settings.Set(0,L"AddToDisksMenu",Opt.AddToDisksMenu);
  settings.Set(0,L"AddToPluginsMenu",Opt.AddToPluginsMenu);
  settings.Set(0,L"BrowseEvtFiles",Opt.BrowseEvtFiles);
  settings.Set(0,L"StripExt",Opt.StripExt);
  settings.Set(0,L"ScanType",Opt.ScanType);
  settings.Set(0,L"Prefix",Opt.Prefix);
  settings.Set(0,L"Restore",Opt.Restore);
  settings.Set(0,L"ShowHeader",QVOpt.ShowHeader);
  settings.Set(0,L"ShowDescription",QVOpt.ShowDescription);
  settings.Set(0,L"ShowData",QVOpt.ShowData);

  return(TRUE);
}
