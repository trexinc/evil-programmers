/*
    FARMail plugin for FAR Manager
    Copyright (C) 2002-2005 FARMail Group
    Copyright (C) 1999,2000 Serge Alexandrov

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "farmail.hpp"

extern void Settings_Encodings(const char *txt);

static CHARSET_TABLE *CharsetTable = NULL;

CHARSET_TABLE ** GetCharsetTable( void )
{
 return &CharsetTable;
}

void ReadRegistryInterface(void)
{
  HKEY hRoot = HKEY_CURRENT_USER;
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, ADDTODISKSMENU,  &Opt.AddToDisksMenu, 1 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, DISKSMENUDIGIT,  &Opt.DisksMenuDigit, 1 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, ADDTOPLUGINMENU, &Opt.AddToPluginMenu, 1 );

  char *lpModeKey = (char*)z_malloc (512);
  HKEY hKey;

  const char *lpKeyNames[PLUGIN_PANEL_COUNT] = {"Mailboxes", "IMAP4 Folders", "POP3", "IMAP4"};
  DWORD dwSize = 4;

  for (int i = 0; i < PLUGIN_PANEL_COUNT; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      FSF.sprintf (lpModeKey, "%s\\PanelModes\\%s\\Mode%d", PluginCommonKey, lpKeyNames[i], j);

      if ( RegOpenKeyEx ( hRoot, lpModeKey, 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
      {
        Opt.Modes[i][j].lpColumnTypes = RegQueryStringValueEx ( hKey, "ColumnTypes", Opt.Modes[i][j].lpColumnTypes);
        Opt.Modes[i][j].lpColumnWidths = RegQueryStringValueEx ( hKey, "ColumnWidths", Opt.Modes[i][j].lpColumnWidths);
        Opt.Modes[i][j].lpStatusColumnTypes = RegQueryStringValueEx ( hKey, "StatusColumnTypes", Opt.Modes[i][j].lpStatusColumnTypes);
        Opt.Modes[i][j].lpStatusColumnWidths = RegQueryStringValueEx ( hKey, "StatusColumnWidths", Opt.Modes[i][j].lpStatusColumnWidths);

		FSF.Trim (Opt.Modes[i][j].lpColumnTypes);
		FSF.Trim (Opt.Modes[i][j].lpColumnWidths);
		FSF.Trim (Opt.Modes[i][j].lpStatusColumnTypes);
		FSF.Trim (Opt.Modes[i][j].lpStatusColumnWidths);

        RegQueryValueEx (hKey, "Fullscreen", NULL, NULL, (PBYTE)&Opt.Modes[i][j].bFullScreen, &dwSize);
        RegCloseKey (hKey);
      }
    }
  }

  z_free (lpModeKey);
}

void ReadRegistryFiles(void)
{
  HKEY hRoot = HKEY_CURRENT_USER;
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, EXTENSION,    Opt.EXT, "msg", 20 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, FIXEDZEROS,   &Opt.LeadingZeros, 0 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, SAVEMESSAGES, &Opt.SaveMessages, 0 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, SAVEFOLDER,   Opt.SaveDir, NULLSTR, MAX_PATH );
  FSF.Unquote(Opt.SaveDir);
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, USEINBOX,     &Opt.UseInbox, 0 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, PATHTOINBOX,  Opt.PathToInbox, NULLSTR, MAX_PATH );
  FSF.Unquote(Opt.PathToInbox);
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, USEOUTBOX,    &Opt.UseOutbox, 0 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, PATHTOOUTBOX, Opt.PathToOutbox, NULLSTR, MAX_PATH );
  FSF.Unquote(Opt.PathToOutbox);
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, FILEDATE,     &Opt.FileDate, 0 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, NAMEFORMAT,   Opt.Format, NULLSTR, 100 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, USENAMEF,     &Opt.UseNameF, 0 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, SAVEMESSAGEID,&Opt.SaveMessageID, 0 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, MESSAGEIDTEMPLATE, Opt.MessageIDTemplate, NULLSTR, 512 );

}

void ReadRegistryConnection(void)
{
  HKEY hRoot = HKEY_CURRENT_USER;
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, TIMEOUT,        &Opt.Timeout, 60 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, QV1,            &Opt.QuickLines, 0 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, QV2,            &Opt.QuickBytes, 0 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, RESUME,         &Opt.Resume, 0 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, DEBUGSESSION,   &Opt.DebugSession,   0 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, IMAP_INBOX,     Opt.IMAP_Inbox, INBOX, 512 );
  FSF.Trim(Opt.IMAP_Inbox);
  if (!*Opt.IMAP_Inbox)
    lstrcpy(Opt.IMAP_Inbox,INBOX);
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, DISPLAYZEROSIZEMESS, &Opt.DisplayZeroSizeMess, 0 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, DISABLETOP,     &Opt.DisableTOP, 0 );
}

void ReadRegistryUidl(void)
{
  HKEY hRoot=HKEY_CURRENT_USER;
  GetRegKey2(hRoot,PluginUidlKey,NULLSTR,NEWATTRIBUTES,&Opt.NewAttributes,FILE_ATTRIBUTE_READONLY);
  GetRegKey2(hRoot,PluginUidlKey,NULLSTR,READATTRIBUTES,&Opt.ReadAttributes,FILE_ATTRIBUTE_ARCHIVE);
  GetRegKey2(hRoot,PluginUidlKey,NULLSTR,MARKEDATTRIBUTES,&Opt.MarkedAttributes,FILE_ATTRIBUTE_SYSTEM);
  GetRegKey2(hRoot,PluginUidlKey,NULLSTR,DELETEDATTRIBUTES,&Opt.DeletedAttributes,FILE_ATTRIBUTE_HIDDEN);
  GetRegKey2(hRoot,PluginUidlKey,NULLSTR,USEATTRHIGHLIGHTING,&Opt.UseAttrHighlighting,0);
}

void ReadRegistryOther(void)
{
  HKEY hRoot = HKEY_CURRENT_USER;
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, USECC,         &Opt.UseCC, 0);
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, USEHEADER,     &Opt.UseHeader, 1);
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, CONFIRMBINARY, &Opt.ConfirmBinary, 1);
}

enum enumSettingsInterface {
  ID_SI_TITLE = 0,
  ID_SI_ADDTODISKMENU,
  ID_SI_DISKMENUHOTKEY,
  ID_SI_DISKMENUHOTKEYEDIT,
  ID_SI_ADDTOPLUGINMENU,
  ID_SI_SEPARATOR1,
  ID_SI_PANELMODESFOR,
  ID_SI_COMBOBOX,
  D_SI_SEPARATOR2,
  ID_SI_LISTBOX,
  ID_SI_VSEPARATOR,
  ID_SI_COLUMNTYPES,
  ID_SI_COLUMNTYPESEDIT,
  ID_SI_COLUMNWIDTHS,
  ID_SI_COLUMNWIDTHSEDIT,
  ID_SI_STATUSCOLUMNTYPES,
  ID_SI_STATUSCOLUMNTYPESEDIT,
  ID_SI_STATUSCOLUMNWIDTHS,
  ID_SI_STATUSCOLUMNWIDTHSEDIT,
  ID_SI_FULLSCREEN,
  ID_SI_SEPARATOR3,
  ID_SI_OK,
  ID_SI_CANCEL
  };



struct ModeDialogResult {
  PanelModeOptions Modes[PLUGIN_PANEL_COUNT][10];
  int nPanel;
  int LastPositions[PLUGIN_PANEL_COUNT];
};


void InitializeTempPanelOptions (ModeDialogResult *pParam)
{
  for (int i = 0; i < PLUGIN_PANEL_COUNT; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      pParam->Modes[i][j].lpColumnTypes = z_strdup (Opt.Modes[i][j].lpColumnTypes);
      pParam->Modes[i][j].lpColumnWidths = z_strdup (Opt.Modes[i][j].lpColumnWidths);
      pParam->Modes[i][j].lpStatusColumnTypes = z_strdup (Opt.Modes[i][j].lpStatusColumnTypes);
      pParam->Modes[i][j].lpStatusColumnWidths = z_strdup (Opt.Modes[i][j].lpStatusColumnWidths);

      pParam->Modes[i][j].bFullScreen = Opt.Modes[i][j].bFullScreen;
    }
  }
}

void InitializeModeList (
    HANDLE hDlg,
    ModeDialogResult *pParam
    )
{
  for (int i = 0; i < 10; i++)
  {
    FarListItemData lData;

    lData.Data = &pParam->Modes[pParam->nPanel][i];//ModeOptions;
    lData.DataSize = 4;
    lData.Index = i;

    _Info.SendDlgMessage (hDlg, DM_LISTSETDATA, ID_SI_LISTBOX, (long)&lData);
  }

  _Info.SendDlgMessage (hDlg, DM_LISTSETMOUSEREACTION, ID_SI_LISTBOX, 2/*LMRT_NEVER*/);
}

void PrepareModeOptions (
    HANDLE hDlg,
    int nMode
    )
{
  PanelModeOptions *pModeOptions = (PanelModeOptions*)_Info.SendDlgMessage (
      hDlg,
      DM_LISTGETDATA,
      ID_SI_LISTBOX,
      nMode
      );

  _Info.SendDlgMessage (hDlg, DM_SETTEXTPTR, ID_SI_COLUMNTYPESEDIT, (long)pModeOptions->lpColumnTypes);
  _Info.SendDlgMessage (hDlg, DM_SETTEXTPTR, ID_SI_COLUMNWIDTHSEDIT, (long)pModeOptions->lpColumnWidths);
  _Info.SendDlgMessage (hDlg, DM_SETTEXTPTR, ID_SI_STATUSCOLUMNTYPESEDIT, (long)pModeOptions->lpStatusColumnTypes);
  _Info.SendDlgMessage (hDlg, DM_SETTEXTPTR, ID_SI_STATUSCOLUMNWIDTHSEDIT, (long)pModeOptions->lpStatusColumnWidths);

  _Info.SendDlgMessage (hDlg, DM_ENABLE, ID_SI_FULLSCREEN, *pModeOptions->lpColumnTypes);
  _Info.SendDlgMessage (hDlg, DM_SETCHECK, ID_SI_FULLSCREEN, pModeOptions->bFullScreen?BSTATE_CHECKED:BSTATE_UNCHECKED);

  _Info.SendDlgMessage (hDlg, DM_REDRAW, 0, 0);
}


void FlushModeOptions (
    HANDLE hDlg,
    int nMode
    )
{
  PanelModeOptions *pModeOptions = (PanelModeOptions*)_Info.SendDlgMessage (
      hDlg,
      DM_LISTGETDATA,
      ID_SI_LISTBOX,
      nMode
      );

  char *lpTempString = (char*)z_malloc (260);

  _Info.SendDlgMessage (hDlg, DM_GETTEXTPTR, ID_SI_COLUMNTYPESEDIT, (long)lpTempString);

  z_free (pModeOptions->lpColumnTypes);
  pModeOptions->lpColumnTypes= z_strdup (lpTempString);

  _Info.SendDlgMessage (hDlg, DM_GETTEXTPTR, ID_SI_COLUMNWIDTHSEDIT, (long)lpTempString);

  z_free (pModeOptions->lpColumnWidths);
  pModeOptions->lpColumnWidths = z_strdup (lpTempString);

  _Info.SendDlgMessage (hDlg, DM_GETTEXTPTR, ID_SI_STATUSCOLUMNTYPESEDIT, (long)lpTempString);

  z_free (pModeOptions->lpStatusColumnTypes);
  pModeOptions->lpStatusColumnTypes = z_strdup (lpTempString);

  _Info.SendDlgMessage (hDlg, DM_GETTEXTPTR, ID_SI_STATUSCOLUMNWIDTHSEDIT, (long)lpTempString);

  z_free (pModeOptions->lpStatusColumnWidths);
  pModeOptions->lpStatusColumnWidths = z_strdup (lpTempString);

  FSF.Trim (pModeOptions->lpColumnTypes);
  FSF.Trim (pModeOptions->lpColumnWidths);
  FSF.Trim (pModeOptions->lpStatusColumnTypes);
  FSF.Trim (pModeOptions->lpStatusColumnWidths);

  pModeOptions->bFullScreen = _Info.SendDlgMessage (hDlg, DM_GETCHECK, ID_SI_FULLSCREEN, 0) == BSTATE_CHECKED;

  z_free (lpTempString);
}


void FinalizeTempPanelOptions (ModeDialogResult *pParam, bool bSave)
{
  for (int i = 0; i < PLUGIN_PANEL_COUNT; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      if ( bSave )
      {
        Opt.Modes[i][j].lpColumnTypes = z_strdup (pParam->Modes[i][j].lpColumnTypes);
        Opt.Modes[i][j].lpColumnWidths = z_strdup (pParam->Modes[i][j].lpColumnWidths);
        Opt.Modes[i][j].lpStatusColumnTypes = z_strdup (pParam->Modes[i][j].lpStatusColumnTypes);
        Opt.Modes[i][j].lpStatusColumnWidths = z_strdup (pParam->Modes[i][j].lpStatusColumnWidths);

        Opt.Modes[i][j].bFullScreen = pParam->Modes[i][j].bFullScreen;
      }

      z_free (pParam->Modes[i][j].lpColumnTypes);
      z_free (pParam->Modes[i][j].lpColumnWidths);
      z_free (pParam->Modes[i][j].lpStatusColumnTypes);
      z_free (pParam->Modes[i][j].lpStatusColumnWidths);
    }
  }
}


int __stdcall hndSettingsInterface (
    HANDLE hDlg,
    int nMsg,
    int nParam1,
    int nParam2
    )
{
  char *lpModeString;
  FarListPos lPos;

  ModeDialogResult *pParam = (ModeDialogResult*)_Info.SendDlgMessage (
      hDlg,
      DM_GETDLGDATA,
      0,
      0
      );

  switch ( nMsg ) {

  case DN_BTNCLICK:

    if ( nParam1 == 1 )
    {
      _Info.SendDlgMessage (hDlg, DM_ENABLE, ID_SI_DISKMENUHOTKEY, nParam2);
      _Info.SendDlgMessage (hDlg, DM_ENABLE, ID_SI_DISKMENUHOTKEYEDIT, nParam2);
    }

    break;

  case DN_INITDIALOG:

    _Info.SendDlgMessage (hDlg, DM_LISTADDSTR, ID_SI_COMBOBOX, (long)GetMsg (MesConfig_MailboxesPanel));
    _Info.SendDlgMessage (hDlg, DM_LISTADDSTR, ID_SI_COMBOBOX, (long)GetMsg (MesConfig_IMAP4FoldersPanel));
    _Info.SendDlgMessage (hDlg, DM_LISTADDSTR, ID_SI_COMBOBOX, (long)GetMsg (MesConfig_POP3MailboxPanel));
    _Info.SendDlgMessage (hDlg, DM_LISTADDSTR, ID_SI_COMBOBOX, (long)GetMsg (MesConfig_IMAP4MailboxPanel));

    lpModeString = (char*)z_malloc (100);

    for (int i = 0; i < 10; i++)
    {
      FSF.sprintf (lpModeString, GetMsg (MesConfig_Mode), i);
      _Info.SendDlgMessage (hDlg, DM_LISTADDSTR, ID_SI_LISTBOX, (long)lpModeString);
    }

    z_free (lpModeString);

    InitializeTempPanelOptions (pParam);
    InitializeModeList (hDlg, pParam);
    PrepareModeOptions (hDlg, 0);

    break;

  case DN_EDITCHANGE:

    if ( nParam1 == ID_SI_COLUMNTYPESEDIT )
    {
      lpModeString = z_strdup (reinterpret_cast<FarDialogItem*>(nParam2)->Data);

      FSF.Trim (lpModeString);

      _Info.SendDlgMessage (hDlg, DM_ENABLE, ID_SI_FULLSCREEN, *lpModeString);
    }

    break;
   

  case DN_LISTCHANGE:

    if ( nParam1 == ID_SI_LISTBOX )
    {
      FlushModeOptions (hDlg, pParam->LastPositions[pParam->nPanel]);
      PrepareModeOptions (hDlg, nParam2);

      pParam->LastPositions[pParam->nPanel] = nParam2;
    }

    if ( nParam1 == ID_SI_COMBOBOX )
    {
      if ( nParam2 != pParam->nPanel )
      {
        FlushModeOptions (hDlg, pParam->LastPositions[pParam->nPanel]);

        pParam->nPanel = nParam2;

        InitializeModeList (hDlg, pParam);

        PrepareModeOptions (hDlg, pParam->LastPositions[pParam->nPanel]);

        lPos.TopPos = -1;
        lPos.SelectPos = pParam->LastPositions[pParam->nPanel];

        _Info.SendDlgMessage (hDlg, DM_LISTSETCURPOS, ID_SI_LISTBOX, (long)&lPos);
      }
    }

    break;

  case DN_CLOSE:

    FlushModeOptions(hDlg, pParam->LastPositions[pParam->nPanel]);
    FinalizeTempPanelOptions(pParam, nParam1 == ID_SI_OK);
    break;


  };

  return _Info.DefDlgProc (hDlg, nMsg, nParam1, nParam2);
}



static void Settings_Interface(const char *txt)
{
  int l;
  HKEY hRoot = HKEY_CURRENT_USER;

  static struct InitDialogItem InitItems[]=
  {
    // type, x1, y1, x2, y2, focus, selected, flags, default, data
    { DI_DOUBLEBOX,3,1,50,21,0,0,0,0,(char*)MesConfig_MainTitle },
    { DI_CHECKBOX, 5, 2, 0, 0, 1,0,0,0, (char*)MesConfig_AddToDisksMenuChkBx },
    { DI_TEXT, 9, 3, 0,0,0,0,0,0, (char *)MesConfig_DisksMenuHotkey },
    { DI_FIXEDIT, 7, 3, 7,4,0,0,0,0, NULLSTR },
    { DI_CHECKBOX, 5, 4, 0, 0, 1,0,0,0, (char*)MesConfig_AddToPluginsMenu },
    { DI_TEXT, 3, 5, 0, 0, 0,0,DIF_SEPARATOR, 0, NULLSTR },
    { DI_TEXT, 5, 6, 0, 0, 0, 0, 0, 0, (char*)MesConfig_SelectPanel },
    { DI_COMBOBOX, 5, 7, 5+43, 0, 0, 0, DIF_DROPDOWNLIST, 0, NULLSTR },
    { DI_TEXT, 3, 8, 0, 0, 0, 0, DIF_SEPARATOR, 0, NULLSTR },
    { DI_LISTBOX, 4, 9, 14, 9+9, 0, 0, DIF_LISTNOBOX|DIF_LISTWRAPMODE, 0, NULLSTR },
    { DI_VTEXT, 15, 9, 0, 10, 0, 0, 0, 0, "³³³³³³³³³³" },
    { DI_TEXT, 17, 9, 0, 0, 0, 0, 0, 0, (char *)MesConfig_ColumnTypes },
    { DI_EDIT, 17, 10, 17+31, 0, 0, 0, 0, 0, NULLSTR },

    { DI_TEXT, 17, 11, 0, 0, 0, 0, 0, 0, (char *)MesConfig_ColumnWidths },
    { DI_EDIT, 17, 12, 17+31, 0, 0, 0, 0, 0, NULLSTR },

    { DI_TEXT, 17, 13, 0, 0, 0, 0, 0, 0, (char *)MesConfig_StatusColumnTypes },
    { DI_EDIT, 17, 14, 17+31, 0, 0, 0, 0, 0, NULLSTR },

    { DI_TEXT, 17, 15, 0, 0, 0, 0, 0, 0, (char *)MesConfig_StatusColumnWidths },
    { DI_EDIT, 17, 16, 17+31, 0, 0, 0, 0, 0, NULLSTR },

    { DI_CHECKBOX, 17, 18, 0, 0, 0,0,0,0, (char*)MesConfig_FullScreenChkBx },

    { DI_TEXT, 3, 19, 0, 0, 0,0,DIF_SEPARATOR, 0, NULLSTR },
    { DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,1,(char *)MesOk },
    { DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,0,(char *)MesCancel }
  };


  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  ReadRegistryInterface();

  FSF.sprintf( DialogItems[ID_SI_DISKMENUHOTKEYEDIT].Data , "%1d", Opt.DisksMenuDigit );

  if ( Opt.AddToDisksMenu )
    DialogItems[ID_SI_ADDTODISKMENU].Selected = TRUE;
  else
  {
    DialogItems[ID_SI_DISKMENUHOTKEY].Flags = DIF_DISABLE;
    DialogItems[ID_SI_DISKMENUHOTKEYEDIT].Flags = DIF_DISABLE;
  }

  if ( Opt.AddToPluginMenu )
    DialogItems [ID_SI_ADDTOPLUGINMENU].Selected = TRUE;

  l= lstrlen(DialogItems[ID_SI_TITLE].Data);
  lstrcat( DialogItems[ID_SI_TITLE].Data, ": " );
  lstrcat( DialogItems[ID_SI_TITLE].Data, txt );
  FSF.LStrlwr( DialogItems[ID_SI_TITLE].Data+l );

  ModeDialogResult Result;

  memset (&Result, 0, sizeof (ModeDialogResult));

  if ( _Info.DialogEx (
      _Info.ModuleNumber,
      -1,
      -1,
      54,
      23,
      "Interface",
      DialogItems,
      sizeof(DialogItems)/sizeof(DialogItems[0]),
      0,
      0,
      (FARWINDOWPROC)hndSettingsInterface,
      (long)&Result
      ) == ID_SI_OK )
  {

    Opt.DisksMenuDigit = FSF.atoi (DialogItems[ID_SI_DISKMENUHOTKEYEDIT].Data);

    if ( DialogItems[ID_SI_ADDTODISKMENU].Selected )
      Opt.AddToDisksMenu = 1;
    else
      Opt.AddToDisksMenu = 0;

    if ( DialogItems[ID_SI_ADDTOPLUGINMENU].Selected )
      Opt.AddToPluginMenu = 1;
    else
      Opt.AddToPluginMenu = 0;

    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, ADDTODISKSMENU , Opt.AddToDisksMenu );
    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, DISKSMENUDIGIT , Opt.DisksMenuDigit );
    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, ADDTOPLUGINMENU, Opt.AddToPluginMenu );

    const char *lpKeyNames[PLUGIN_PANEL_COUNT] = {"Mailboxes", "IMAP4 Folders", "POP3", "IMAP4"};
    char *lpModeKey = (char*)z_malloc (512);

    for (int i = 0; i < PLUGIN_PANEL_COUNT; i++)
    {
      for (int j = 0; j < 10; j++)
      {
        FSF.sprintf (lpModeKey, "PanelModes\\%s\\Mode%d", lpKeyNames[i], j);

        SetRegKey2 (hRoot, PluginCommonKey, lpModeKey, "ColumnTypes", Opt.Modes[i][j].lpColumnTypes);
        SetRegKey2 (hRoot, PluginCommonKey, lpModeKey, "ColumnWidths", Opt.Modes[i][j].lpColumnWidths);
        SetRegKey2 (hRoot, PluginCommonKey, lpModeKey, "StatusColumnTypes", Opt.Modes[i][j].lpStatusColumnTypes);
        SetRegKey2 (hRoot, PluginCommonKey, lpModeKey, "StatusColumnWidths", Opt.Modes[i][j].lpStatusColumnWidths);
        SetRegKey2 (hRoot, PluginCommonKey, lpModeKey, "FullScreen", Opt.Modes[i][j].bFullScreen);
      }
    }

    z_free (lpModeKey);
  }
}

static void Settings_Files(const char *txt)
{
  static struct InitDialogItem InitItems[] =
  {
    // type, x1, y1, x2, y2, focus, selected, flags, default, data
    { DI_DOUBLEBOX,3, 1, 65,19,0,0,0,0, (char*)MesConfig_MainTitle },
    { DI_TEXT,     5, 2, 0,  0,0,0,0,0, (char*)MesConfig_MsgFixedNameLength },
    { DI_FIXEDIT, 43, 2, 43 ,0,1,0,0,0, NULLSTR },
    { DI_TEXT,     5, 3, 0,  0,0,0,0,0, (char*)MesConfig_MsgExtension },
    { DI_FIXEDIT, 43, 3, 62, 0,0,0,0,0, NULLSTR },

    { DI_SINGLEBOX,5, 4,63,10,0,0,DIF_LEFTTEXT,0, (char *)MesConfig_IncomingSubTitle },
    { DI_CHECKBOX, 6, 5, 0, 0,0,0,0,0, (char*)MesConfig_AssignNewNameChkBx },
    { DI_TEXT,     6, 6, 0, 0,0,0,0,0, (char*)MesConfig_NameFormat },
    { DI_EDIT,    14, 6,62, 0,0,0,0,0, NULLSTR },
    { DI_CHECKBOX, 6, 7, 0, 0,0,0,0,0, (char*)MesConfig_UseInboxChkBx },
    { DI_TEXT,     6, 8, 0, 0,0,0,0,0, (char*)MesConfig_InboxFilename },
    { DI_EDIT,    17, 8,62, 0,0,0,0,0, NULLSTR },
    { DI_CHECKBOX, 6, 9, 0, 0,0,0,0,0, (char*)MesConfig_UseMsgDateChkBx },

    { DI_SINGLEBOX,5,11,63,17,0,0,DIF_LEFTTEXT,0, (char *)MesConfig_OutgoingSubTitle },
    { DI_CHECKBOX, 6,12, 0, 0,0,0,0,0, (char*)MesConfig_SaveOutgoingChkBx },
    { DI_RADIOBUTTON, 6,13, 0,0,0,0,DIF_GROUP,0, (char*)MesConfig_OutgoingDirectory },
    { DI_RADIOBUTTON, 6,14, 0,0,0,0,0,0, (char*)MesConfig_Outbox },
    { DI_EDIT,    21,13,62, 0,0,0,0,0, NULLSTR },
    { DI_EDIT,    21,14,62, 0,0,0,0,0, NULLSTR },
    { DI_CHECKBOX, 6,15, 0, 0,0,0,0,0, (char*)MesConfig_SaveOutgoingMsgIdChkBx },
    { DI_TEXT,     6,16, 0, 0,0,0,0,0, (char*)MesConfig_MsgIdTemplate },
    { DI_EDIT,    17,16,62, 0,0,0,0,0, NULLSTR },

    { DI_BUTTON,   0,18, 0,0,0,0,DIF_CENTERGROUP,1,(char*)MesOk },
    { DI_BUTTON,   0,18, 0,0,0,0,DIF_CENTERGROUP,0,(char*)MesCancel }
  };
  enum
  {
    C_ZEROS = 2,
    C_TXT5,
    C_EXTENSION,
    C_BOX1,
    C_ASSIGNNAMES_CHK,
    C_TXT1,
    C_FORMAT1,
    C2_SAVEINCOMING,
    C2_TXT2,
    C2_INBOX,
    C2_DATE,
    C2_BOX2,
    C2_SAVE,
    C2_RB1,
    C2_RB2,
    C2_DIR,
    C2_OUTBOX,
    C2_SAVEMSGID,
    C2_TXT1,
    C2_MSGIDTEMPLATE,
    C_OK,
    C_CANCEL
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  ReadRegistryFiles();

  FSF.sprintf( DialogItems[C_ZEROS].Data , "%1d", Opt.LeadingZeros );
  lstrcpy( DialogItems[C_EXTENSION].Data , Opt.EXT );
  lstrcpy( DialogItems[C2_DIR].Data , Opt.SaveDir );
  lstrcpy( DialogItems[C2_OUTBOX].Data, Opt.PathToOutbox);
  lstrcpy( DialogItems[C2_MSGIDTEMPLATE].Data, Opt.MessageIDTemplate );
  lstrcpy( DialogItems[C2_INBOX].Data, Opt.PathToInbox);
  lstrcpy( DialogItems[C_FORMAT1].Data , Opt.Format );

  if ( Opt.SaveMessages ) DialogItems[C2_SAVE].Selected = TRUE;
  if ( Opt.SaveMessageID ) DialogItems[C2_SAVEMSGID].Selected = TRUE;
  if ( Opt.UseOutbox ) DialogItems[C2_RB2].Selected = TRUE;
  else                 DialogItems[C2_RB1].Selected = TRUE;
  if ( Opt.UseInbox )  DialogItems[C2_SAVEINCOMING].Selected = TRUE;
  if ( Opt.FileDate )     DialogItems[C2_DATE].Selected = TRUE;
  if ( Opt.UseNameF )     DialogItems[C_ASSIGNNAMES_CHK].Selected = TRUE;

  {
    int l = lstrlen(DialogItems[0].Data);
    lstrcat( DialogItems[0].Data, ": " );
    lstrcat( DialogItems[0].Data, txt );
    FSF.LStrlwr( DialogItems[0].Data+l );
  }

  if ( _Info.Dialog(_Info.ModuleNumber,-1,-1,69,21,"FilesDirs",DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0])) == C_OK )
  {
    lstrcpy( Opt.EXT, DialogItems[C_EXTENSION].Data );
    Opt.LeadingZeros   = FSF.atoi( DialogItems[C_ZEROS].Data );
    lstrcpy( Opt.SaveDir, DialogItems[C2_DIR].Data);
    FSF.Unquote(Opt.SaveDir);
    lstrcpy( Opt.PathToOutbox, DialogItems[C2_OUTBOX].Data);
    FSF.Unquote(Opt.PathToOutbox);
    lstrcpy( Opt.MessageIDTemplate, DialogItems[C2_MSGIDTEMPLATE].Data);
    lstrcpy( Opt.PathToInbox, DialogItems[C2_INBOX].Data);
    FSF.Unquote(Opt.PathToInbox);
    if ( DialogItems[C2_SAVE].Selected ) Opt.SaveMessages = 1;
    else                                 Opt.SaveMessages = 0;
    if ( DialogItems[C2_SAVEMSGID].Selected ) Opt.SaveMessageID = 1;
    else                                      Opt.SaveMessageID = 0;
    if ( DialogItems[C2_SAVEINCOMING].Selected ) Opt.UseInbox = 1;
    else                                         Opt.UseInbox = 0;
    if ( DialogItems[C2_RB2].Selected ) Opt.UseOutbox = 1;
    else                                Opt.UseOutbox = 0;
    if ( DialogItems[C2_DATE].Selected ) Opt.FileDate = 1;
    else                                 Opt.FileDate = 0;

    lstrcpyn( Opt.Format, DialogItems[C_FORMAT1].Data , 100 );

    if ( DialogItems[C_ASSIGNNAMES_CHK].Selected ) Opt.UseNameF = 1;
    else                                           Opt.UseNameF = 0;

    HKEY hRoot = HKEY_CURRENT_USER;
    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, EXTENSION,     Opt.EXT );
    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, FIXEDZEROS,    Opt.LeadingZeros );
    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, USEINBOX,      Opt.UseInbox );
    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, PATHTOINBOX,   Opt.PathToInbox );
    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, USEOUTBOX,     Opt.UseOutbox);
    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, PATHTOOUTBOX,  Opt.PathToOutbox);
    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, SAVEFOLDER,    Opt.SaveDir );
    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, SAVEMESSAGES,  Opt.SaveMessages );
    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, SAVEMESSAGEID, Opt.SaveMessageID );
    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, MESSAGEIDTEMPLATE, Opt.MessageIDTemplate );
    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, FILEDATE,      Opt.FileDate );
    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, NAMEFORMAT,    Opt.Format );
    SetRegKey2( hRoot, PluginCommonKey, NULLSTR, USENAMEF,      Opt.UseNameF );
  }
}

static void Settings_Connection(const char *txt)
{
  int l;
  HKEY hRoot = HKEY_CURRENT_USER;
  static const char HistoryIMAP4Folders[] = "FARMailIMAP4Folders";

  static struct InitDialogItem InitItems[]={
    // type, x1, y1, x2, y2, focus, selected, flags, default, data
    { DI_DOUBLEBOX,3,1,67,15,0,0,0,0,(char*)MesConfig_MainTitle },

    { DI_TEXT, 5, 2, 0,0,0,0,0,0, (char *)MesConfig_WSKTimeout1 },
    { DI_FIXEDIT, 25, 2, 27,0,1,0,0,0, NULLSTR },
    { DI_TEXT, 29, 2, 0,0,0,0,0,0, (char *)MesConfig_WSKTimeout2 },
    { DI_CHECKBOX, 5, 3, 0, 0, 0,0,0,0, (char*)MesConfig_WriteLogChkBx },

    { DI_SINGLEBOX, 5, 4, 65,8,0,0,DIF_LEFTTEXT,0, (char *)MesConfig_IMAP4SubTitle },

    { DI_TEXT, 6, 5, 0,0,0,0,0,0, (char *)MesConfig_QuickViewTitle1 },
    { DI_FIXEDIT, 26, 5, 29,0,0,0,0,0, NULLSTR },
    { DI_TEXT, 31, 5, 0,0,0,0,0,0, (char *)MesConfig_QuickViewBytes },

    { DI_CHECKBOX, 6, 6, 0, 0, 0,0,0,0, (char*)MesConfig_ResumeOnIMAP4ChkBx },

    { DI_TEXT, 6, 7, 0,0,0,0,0,0, (char *)MesConfig_IMAP4Folders },
    { DI_EDIT, 28, 7, 63,0,0,(DWORD)HistoryIMAP4Folders,DIF_HISTORY,0, NULLSTR },

    { DI_SINGLEBOX, 5, 9, 65 ,13,0,0,DIF_LEFTTEXT,0, (char *)MesConfig_POP3SubTitle },

    { DI_TEXT, 6, 10, 0,0,0,0,0,0, (char *)MesConfig_QuickViewTitle2 },
    { DI_FIXEDIT, 26, 10, 29,0,0,0,0,0, NULLSTR },
    { DI_TEXT, 31, 10, 0,0,0,0,0,0, (char *)MesConfig_QuickViewLines },

    { DI_CHECKBOX, 6, 11, 0, 0, 0,0,0,0, (char*)MesConfig_DisplayZeroSizeMsgsChkBx },
    { DI_CHECKBOX, 6, 12, 0, 0, 0,0,0,0, (char*)MesConfig_DisableTOP_and_LISTChkBx },

    { DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,1,(char *)MesOk },
    { DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,0,(char *)MesCancel }
  };
  enum {
         C_TIMEOUT = 2,
         C_TXT9,
         C_LOG,
         C_TXT10,
         C_TXT11,
         C_QV2,
         C_TXT12,
         C2_RESUME,
         C_TXT111,
         C_IMAP_INBOX,
         C_TXT13,
         C_TXT14,
         C_QV1,
         C_TXT15,
         C_ZEROMESS,
         C_DISABLETOP,
         C_OK,
         C_CANCEL
       };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  ReadRegistryConnection();

  FSF.sprintf( DialogItems[C_TIMEOUT].Data , D1, Opt.Timeout );
  FSF.sprintf( DialogItems[C_QV1].Data , D1, Opt.QuickLines );
  FSF.sprintf( DialogItems[C_QV2].Data , D1, Opt.QuickBytes );
  if ( Opt.Resume )       DialogItems[C2_RESUME].Selected = TRUE;
  if ( Opt.DebugSession ) DialogItems[C_LOG].Selected = TRUE;
  lstrcpy( DialogItems[C_IMAP_INBOX].Data , Opt.IMAP_Inbox );
  if (Opt.DisplayZeroSizeMess) DialogItems[C_ZEROMESS].Selected = TRUE;
  if (Opt.DisableTOP) DialogItems[C_DISABLETOP].Selected = TRUE;

  l= lstrlen(DialogItems[0].Data);
  lstrcat( DialogItems[0].Data, ": " );
  lstrcat( DialogItems[0].Data, txt );
  FSF.LStrlwr( DialogItems[0].Data+l );

  if ( _Info.Dialog(_Info.ModuleNumber,-1,-1,71,17,"Connection",DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0])) == C_OK ) {

     Opt.Timeout        = FSF.atoi( DialogItems[C_TIMEOUT].Data );
     Opt.QuickLines     = FSF.atoi( DialogItems[C_QV1].Data );
     Opt.QuickBytes     = FSF.atoi( DialogItems[C_QV2].Data );
     if ( DialogItems[C2_RESUME].Selected ) Opt.Resume = 1;
     else                                   Opt.Resume = 0;
     if ( DialogItems[C_LOG].Selected ) Opt.DebugSession = 1;
     else                               Opt.DebugSession = 0;
     lstrcpy(Opt.IMAP_Inbox, DialogItems[C_IMAP_INBOX].Data );
     FSF.Trim(Opt.IMAP_Inbox);
     if (!*Opt.IMAP_Inbox)
       lstrcpy(Opt.IMAP_Inbox,INBOX);
     if ( DialogItems[C_ZEROMESS].Selected ) Opt.DisplayZeroSizeMess = 1;
     else                                    Opt.DisplayZeroSizeMess = 0;
     if ( DialogItems[C_DISABLETOP].Selected ) Opt.DisableTOP = 1;
     else                                      Opt.DisableTOP = 0;

     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, TIMEOUT       , Opt.Timeout );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, QV1           , Opt.QuickLines );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, QV2           , Opt.QuickBytes );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, RESUME        , Opt.Resume );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, DEBUGSESSION  , Opt.DebugSession );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, IMAP_INBOX    , Opt.IMAP_Inbox );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, DISPLAYZEROSIZEMESS, Opt.DisplayZeroSizeMess );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, DISABLETOP, Opt.DisableTOP );

  }
}

static void Settings_Other(const char *txt)
{
  HKEY hRoot = HKEY_CURRENT_USER;
  static struct InitDialogItem InitItems[]=
  {
    // type, x1, y1, x2, y2, focus, selected, flags, default, data
    { DI_DOUBLEBOX,3,1,65,7,0,0,0,0,(char*)MesConfig_MainTitle }, // -9

    { DI_CHECKBOX, 5, 2, 0, 0, 1,0,0,0, (char*)MesConfig_UseCCChkBx },
    { DI_CHECKBOX, 5, 3, 0, 0, 0,0,0,0, (char*)MesConfig_SearchHeaderInfoChkBx },
    { DI_CHECKBOX, 5, 4, 0, 0, 0,0,0,0, (char*)MesConfig_ConfirmBinary },
    {DI_TEXT,3,5,0,0,0,0,DIF_SEPARATOR,0,NULLSTR},

    { DI_BUTTON, 0,6,0,0,0,0,DIF_CENTERGROUP,1,(char *)MesOk },
    { DI_BUTTON, 0,6,0,0,0,0,DIF_CENTERGROUP,0,(char *)MesCancel }
  };
  enum
  {
    C_CC_BCC = 1,
    C2_SEARCH,
    C_CONFIRMBINARY,
    C_TXT1,
    C_OK,
    C_CANCEL
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  ReadRegistryOther();

  if(Opt.UseCC) DialogItems[C_CC_BCC].Selected=TRUE;
  if(Opt.UseHeader) DialogItems[C2_SEARCH].Selected=TRUE;
  if(Opt.ConfirmBinary) DialogItems[C_CONFIRMBINARY].Selected=TRUE;

  {
    int l = lstrlen(DialogItems[0].Data);
    lstrcat( DialogItems[0].Data, ": " );
    lstrcat( DialogItems[0].Data, txt );
    FSF.LStrlwr( DialogItems[0].Data+l );
  }

  if ( _Info.Dialog(_Info.ModuleNumber,-1,-1,69,9,"Other",DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0])) == C_OK ) {

     if ( DialogItems[C_CC_BCC].Selected ) Opt.UseCC = 1;
     else                                  Opt.UseCC = 0;
     if ( DialogItems[C2_SEARCH].Selected ) Opt.UseHeader = 1;
     else                                   Opt.UseHeader = 0;
     if(DialogItems[C_CONFIRMBINARY].Selected) Opt.ConfirmBinary=1;
     else Opt.ConfirmBinary = 0;

     SetRegKey2(hRoot,PluginCommonKey,NULLSTR,USECC,Opt.UseCC);
     SetRegKey2(hRoot,PluginCommonKey,NULLSTR,USEHEADER,Opt.UseHeader);
     SetRegKey2(hRoot,PluginCommonKey,NULLSTR,CONFIRMBINARY,Opt.ConfirmBinary);
  }
}

enum
{
  C_STATES_LIST=1,
  C_STATES_R,
  C_STATES_A,
  C_STATES_H,
  C_STATES_S,
  C_STATES_C,
  C_STATES_E,
  C_STATES_USEATRHL,
  C_STATES_OK,
  C_STATES_CANCEL
};

static const int Attributes[]={FILE_ATTRIBUTE_READONLY,FILE_ATTRIBUTE_ARCHIVE,FILE_ATTRIBUTE_HIDDEN,FILE_ATTRIBUTE_SYSTEM,FILE_ATTRIBUTE_COMPRESSED,FILE_ATTRIBUTE_ENCRYPTED};

static long WINAPI StateDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  int *DlgParams=(int *)_Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_INITDIALOG:
      _Info.SendDlgMessage(hDlg,DM_SETDLGDATA,0,Param2);
      _Info.SendDlgMessage(hDlg,DN_LISTCHANGE,C_STATES_LIST,0);
      break;
    case DN_LISTCHANGE:
      for(unsigned int i=0;i<sizeofa(Attributes);i++)
        Dialog_SetState(C_STATES_R+i,DlgParams[Param2]&Attributes[i]);
      _Info.SendDlgMessage(hDlg,DM_REDRAW,0,0);
      break;
    case DN_BTNCLICK:
      if(Param1>=C_STATES_R&&Param1<=C_STATES_E)
      {
        int attr=0;
        for(unsigned int i=0;i<sizeofa(Attributes);i++)
          attr|=Dialog_GetState(C_STATES_R+i)?Attributes[i]:0;
        DlgParams[_Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,C_STATES_LIST,0)]=attr;
      }
      break;
  }
  return _Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

static void Settings_States(const char *txt)
{
  HKEY hRoot=HKEY_CURRENT_USER;
  InitDialogItem InitItems[]=
  {
    {DI_DOUBLEBOX,3,1,65,12,0,0,0,0,(char*)MesConfig_MainTitle},
    {DI_LISTBOX,4,2,33,7,1,0,DIF_LISTWRAPMODE|DIF_LISTNOBOX,0,NULLSTR},
    {DI_CHECKBOX,36,2,0,0,0,0,0,0,(char *)MesConfig_Attr_R},
    {DI_CHECKBOX,36,3,0,0,0,0,0,0,(char *)MesConfig_Attr_A},
    {DI_CHECKBOX,36,4,0,0,0,0,0,0,(char *)MesConfig_Attr_H},
    {DI_CHECKBOX,36,5,0,0,0,0,0,0,(char *)MesConfig_Attr_S},
    {DI_CHECKBOX,36,6,0,0,0,0,0,0,(char *)MesConfig_Attr_C},
    {DI_CHECKBOX,36,7,0,0,0,0,0,0,(char *)MesConfig_Attr_E},
    {DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MesConfig_UseAttrHighlighting},
    {DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,1,(char *)MesOk},
    {DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,0,(char *)MesCancel},
    {DI_TEXT,3,8,0,0,0,0,DIF_SEPARATOR,0,NULLSTR},
    {DI_TEXT,3,10,0,0,0,0,DIF_SEPARATOR,0,NULLSTR},
    {DI_VTEXT,34,2,0,0,0,0,0,0,"³³³³³³Á"}
  };
  FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));
  ReadRegistryUidl();
  DialogItems[C_STATES_USEATRHL].Selected=Opt.UseAttrHighlighting?TRUE:FALSE;
  int ListIndex[]={MesConfig_NewMessage,MesConfig_ReadMessage,MesConfig_MarkedMessage,MesConfig_DeletedMessage};
  FarListItem State[sizeofa(ListIndex)];
  FarList States={sizeofa(State),State};
  int Params[sizeofa(ListIndex)]={Opt.NewAttributes,Opt.ReadAttributes,Opt.MarkedAttributes,Opt.DeletedAttributes};
  for(unsigned int i=0;i<sizeofa(State);i++)
  {
    State[i].Flags=0;
    lstrcpy(State[i].Text,GetMsg(ListIndex[i]));
  }
  State[0].Flags=LIF_SELECTED;
  DialogItems[C_STATES_LIST].ListItems=&States;
  {
    int l=lstrlen(DialogItems[0].Data);
    lstrcat(DialogItems[0].Data,": ");
    lstrcat(DialogItems[0].Data,txt);
    FSF.LStrlwr(DialogItems[0].Data+l);
  }
  if(_Info.DialogEx(_Info.ModuleNumber,-1,-1,69,14,"States",DialogItems,sizeofa(DialogItems),0,0,StateDialogProc,(long)Params)==C_STATES_OK)
  {
    Opt.NewAttributes=Params[0]; Opt.ReadAttributes=Params[1]; Opt.MarkedAttributes=Params[2]; Opt.DeletedAttributes=Params[3];
    Opt.UseAttrHighlighting=DialogItems[C_STATES_USEATRHL].Selected?TRUE:FALSE;
    SetRegKey2(hRoot,PluginUidlKey,NULLSTR,NEWATTRIBUTES,Opt.NewAttributes);
    SetRegKey2(hRoot,PluginUidlKey,NULLSTR,READATTRIBUTES,Opt.ReadAttributes);
    SetRegKey2(hRoot,PluginUidlKey,NULLSTR,MARKEDATTRIBUTES,Opt.MarkedAttributes);
    SetRegKey2(hRoot,PluginUidlKey,NULLSTR,DELETEDATTRIBUTES,Opt.DeletedAttributes);
    SetRegKey2(hRoot,PluginUidlKey,NULLSTR,USEATTRHIGHLIGHTING,Opt.UseAttrHighlighting);
  }
}

int FARMailConfig( void )
{
  int choice = 0;
  unsigned int i;
  int Msgs[]={MesConfig_Interface,MesConfig_FilesDirectories,MesConfig_Connection,MesConfig_Encodings,MesConfig_States,MesConfig_Other};

  struct FarMenuItem items[sizeofa(Msgs)+2];
  /*
    {NULLSTR,1,0,0}, // interface
    {NULLSTR,0,0,0}, // files and directories
    {NULLSTR,0,0,0}, // connection
    {NULLSTR,0,0,0}, // charsets
    {NULLSTR,0,0,0}, // other
    {NULLSTR,0,0,1}, // separator
    {NULLSTR,0,0,0}  // plugins
  */
  memset(items,0,sizeof(items));
  items[0].Selected = TRUE;
  items[6].Separator = TRUE;
  for(size_t i=0;i<sizeofa(Msgs);i++)
    FSF.sprintf(items[i].Text,"%c. %s",i+'1',GetMsg(Msgs[i]));
  FSF.sprintf(items[sizeofa(items)-1].Text,"%c. %s",'0',GetMsg(MesPluginConfig));

  while(choice>=0)
  {
    for(i=0;i<sizeofa(items);i++) items[i].Selected=0;
    items[choice].Selected=1;
    choice=_Info.Menu(_Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,GetMsg( MesConfig_MainTitle ),NULL,"PluginSettings",NULL,NULL,items,sizeofa(items));
    if(choice>=0)
    {
      switch(choice)
      {
        case 0:
          Settings_Interface(GetMsg(Msgs[choice]));
          break;
        case 1:
          Settings_Files(GetMsg(Msgs[choice]));
          break;
        case 2:
          Settings_Connection(GetMsg(Msgs[choice]));
          break;
        case 3:
          Settings_Encodings(GetMsg(Msgs[choice]));
          break;
        case 4:
          Settings_States(GetMsg(Msgs[choice]));
          break;
        case 5:
          Settings_Other(GetMsg(Msgs[choice]));
          break;
        case 7:
          pm->ShowConfigMenu();
          break;
      }
    }
  }
  return 0;
}
