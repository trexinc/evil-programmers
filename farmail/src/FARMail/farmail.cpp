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

#if defined(__GNUC__) && !defined(TDEBUG)

#ifdef __cplusplus
extern "C"
{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}

#endif

#if defined(__BORLANDC__) && defined(TDEBUG)
#ifdef __cplusplus
extern "C"
{
#endif
  BOOL WINAPI _export LibMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved)
  {
    (void) hDLLInst;
    (void) fdwReason;
    (void) lpvReserved;
    return TRUE;
  }
#ifdef __cplusplus
}
#endif
#endif

#if defined (_MSC_VER)
BOOL __stdcall DllMain (HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
  return TRUE;
}
#endif

struct PluginStartupInfo _Info;
FARSTANDARDFUNCTIONS FSF;
Options Opt;
PluginManager *pm=NULL;
char PluginRootKey[80];
char PluginMailBoxKey[100];
char PluginCommonKey[100];
char PluginHotkeyKey[100];
char PluginUidlKey[100];
HANDLE UidlMutex=NULL;

const char FASTDOWNLOADCOPY_PREFIX[] = "fdc";
const char FASTDOWNLOADMOVE_PREFIX[] = "fdm";
const char FASTEXPUNGE_PREFIX     [] = "fx";

const char NULLSTR           [] = "";
const char ADDTODISKSMENU    [] = "AddToDisksMenu";
const char DISKSMENUDIGIT    [] = "DisksMenuDigit";
const char EXTENSION         [] = "Extension";
const char DEBUGSESSION      [] = "DebugSession";
const char DISPLAYZEROSIZEMESS[] = "DisplayZeroSizeMess";
const char FIXEDZEROS        [] = "LeadZero";
const char COLUMNS           [] = "Columns";
const char WIDTHS            [] = "Widths";
const char STATUSCOLUMNS     [] = "StatusColumns";
const char STATUSWIDTHS      [] = "StatusWidths";
const char FULLSIZE          [] = "FullSize";
const char DEFAULTCHARSET    [] = "DefaultCharset";
const char DEFAULTOUTCHARSET [] = "DefaultOutCharset";
const char TIMEOUT           [] = "Timeout";
const char QV1               [] = "QuickViewLines";
const char QV2               [] = "QuickViewBytes";
const char USECC             [] = "UseCCFields";

const char SAVEMESSAGES      [] = "SaveMessages";
const char SAVEFOLDER        [] = "SaveDir";
const char USEINBOX          [] = "UseInbox";
const char PATHTOINBOX       [] = "PathToInbox";
const char USEOUTBOX         [] = "UseOutbox";
const char PATHTOOUTBOX      [] = "PathToOutbox";
const char SAVEMESSAGEID     [] = "SaveMessageID";
const char MESSAGEIDTEMPLATE [] = "MessageIDTemplate";
const char USEHEADER         [] = "SearchHeader";
const char RESUME            [] = "ResumeDownload";
const char FILEDATE          [] = "UseMessageDate";
const char DISABLETOP        [] = "DisableTOP";

const char NAMEFORMAT        [] = "NameFormat";
const char USENAMEF          [] = "UseNameFt";
const char IMAP_INBOX        [] = "IMAP_Inbox";

const char UNIQUE            [] = "Unique";
const char ADDTOPLUGINMENU   [] = "AddToPluginMenu";
const char CONFIRMBINARY     [] = "ConfirmBinary";

const char INBOX             [] = "\"INBOX\"";

static HANDLE farmail_to_handler=NULL;

const char USEATTRHIGHLIGHTING[] = "UseAttrHighlighting";
const char NEWATTRIBUTES     [] = "NewAttributes";
const char READATTRIBUTES    [] = "ReadAttributes";
const char MARKEDATTRIBUTES  [] = "MarkedAttributes";
const char DELETEDATTRIBUTES [] = "DeletedAttributes";

const char *GetMsg(int MsgId)
{
  return(_Info.GetMsg(_Info.ModuleNumber,MsgId));
}

void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item,int ItemsNumber)
{
  for (int I=0;I<ItemsNumber;I++)
  {
    Item[I].Type=Init[I].Type;
    Item[I].X1=Init[I].X1;
    Item[I].Y1=Init[I].Y1;
    Item[I].X2=Init[I].X2;
    Item[I].Y2=Init[I].Y2;
    Item[I].Focus=Init[I].Focus;
    Item[I].Selected=Init[I].Selected;
    Item[I].Flags=Init[I].Flags;
    Item[I].DefaultButton=Init[I].DefaultButton;
    if ((unsigned int)Init[I].Data<2000)
      lstrcpy(Item[I].Data,GetMsg((unsigned int)Init[I].Data));
    else
      lstrcpy(Item[I].Data,Init[I].Data);
  }
}

void ReadRegistryExtra(void)
{
  GetRegKey2( HKEY_CURRENT_USER, PluginCommonKey, NULLSTR, UNIQUE, &Opt.Unique, 1 );
}

int WINAPI _export GetMinFarVersion(void)
{
  return MAKEFARVERSION(1,70,1282);
}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  char *p;

  _Info=*Info;
  FSF = *Info->FSF;
  _Info.FSF = &FSF;

  FSF.sprintf(PluginRootKey,"%s\\FARMail",Info->RootKey);
  FSF.sprintf(PluginMailBoxKey,"%s\\Mailboxes",PluginRootKey);
  FSF.sprintf(PluginCommonKey,"%s\\Common",PluginRootKey);
  FSF.sprintf(PluginHotkeyKey,"%s\\PluginHotkeys",PluginRootKey);
  FSF.sprintf(PluginUidlKey,"%s\\Uidl",PluginRootKey);

  lstrcpy( Opt.LOGFILE, _Info.ModuleName );
  p = Opt.LOGFILE + lstrlen( Opt.LOGFILE );
  while ( p > Opt.LOGFILE && *p != '.' ) p--;
  if ( *p == '.' ) { *(++p) = 'p'; *(++p) = 'o'; *(++p) = 'p'; *(++p) = '\0'; }

  lstrcpy( Opt.LOGFILE2, _Info.ModuleName );
  p = Opt.LOGFILE2 + lstrlen( Opt.LOGFILE2 );
  while ( p > Opt.LOGFILE2 && *p != '.' ) p--;
  if ( *p == '.' ) { *(++p) = 'o'; *(++p) = 'u'; *(++p) = 't'; *(++p) = '\0'; }

  lstrcpy( Opt.LOGFILE3, _Info.ModuleName );
  p = Opt.LOGFILE3 + lstrlen( Opt.LOGFILE3 );
  while ( p > Opt.LOGFILE3 && *p != '.' ) p--;
  if ( *p == '.' ) { *(++p) = 'i'; *(++p) = 'm'; *(++p) = 'a'; *(++p) = 'p'; *(++p) = '\0'; }

  memset (&Opt.Modes, 0, sizeof (Opt.Modes));

  Opt.Modes[PLUGIN_PANEL_MAILBOXES][0].lpColumnTypes = z_strdup ("N,C0");
  Opt.Modes[PLUGIN_PANEL_MAILBOXES][0].lpColumnWidths = z_strdup ("20,0");
  Opt.Modes[PLUGIN_PANEL_MAILBOXES][0].lpStatusColumnTypes = (char*)z_malloc (1);
  Opt.Modes[PLUGIN_PANEL_MAILBOXES][0].lpStatusColumnWidths = (char*)z_malloc (1);

  Opt.Modes[PLUGIN_PANEL_IMAP4_FOLDERS][0].lpColumnTypes = z_strdup ("N,C0,C1,C2,C3");
  Opt.Modes[PLUGIN_PANEL_IMAP4_FOLDERS][0].lpColumnWidths = z_strdup ("0,2,3,3,3");
  Opt.Modes[PLUGIN_PANEL_IMAP4_FOLDERS][0].lpStatusColumnTypes = (char*)z_malloc (1);
  Opt.Modes[PLUGIN_PANEL_IMAP4_FOLDERS][0].lpStatusColumnWidths = (char*)z_malloc (1);

  Opt.Modes[PLUGIN_PANEL_POP3][0].lpColumnTypes = z_strdup ("N,S,C0,C1,C2");
  Opt.Modes[PLUGIN_PANEL_POP3][0].lpColumnWidths = z_strdup ("7,6,8,7,0");
  Opt.Modes[PLUGIN_PANEL_POP3][0].lpStatusColumnTypes = (char*)z_malloc (1);
  Opt.Modes[PLUGIN_PANEL_POP3][0].lpStatusColumnWidths = (char*)z_malloc (1);

  Opt.Modes[PLUGIN_PANEL_IMAP4][0].lpColumnTypes = z_strdup ("N,S,C0,C1,C2");
  Opt.Modes[PLUGIN_PANEL_IMAP4][0].lpColumnWidths = z_strdup ("7,6,8,7,0");
  Opt.Modes[PLUGIN_PANEL_IMAP4][0].lpStatusColumnTypes = (char*)z_malloc (1);
  Opt.Modes[PLUGIN_PANEL_IMAP4][0].lpStatusColumnWidths = (char*)z_malloc (1);

  for (int i = 0; i < PLUGIN_PANEL_COUNT; i++)
  {
    for (int j = 1; j < 10; j++) //1!!!
    {
      Opt.Modes[i][j].lpColumnTypes = z_strdup ("");
      Opt.Modes[i][j].lpColumnWidths = z_strdup ("");
      Opt.Modes[i][j].lpStatusColumnTypes = z_strdup ("");
      Opt.Modes[i][j].lpStatusColumnWidths = z_strdup ("");
    }
  }

  ReadRegistryInterface();
  ReadRegistryFiles();
  ReadRegistryConnection();
  ReadRegistryEncodings();
  ReadRegistryUidl();
  ReadRegistryOther();
  ReadRegistryExtra();

  //uidl share mutex
  UidlMutex=CreateMutex(NULL,FALSE,"FARMailUidlShareMutex");
  { //init plugins
    char plug_dir[MAX_PATH];
    lstrcpy(plug_dir,Info->ModuleName);
    *(FSF.PointToName(plug_dir))=0;
    lstrcat(plug_dir,"fmp\\");
    pm=new PluginManager(plug_dir);
  }
}

void WINAPI ExitFAR()
{
  for (int i = 0; i < PLUGIN_PANEL_COUNT; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      z_free (Opt.Modes[i][j].lpColumnTypes);
      z_free (Opt.Modes[i][j].lpColumnWidths);
      z_free (Opt.Modes[i][j].lpStatusColumnTypes);
      z_free (Opt.Modes[i][j].lpStatusColumnWidths);
    }
  }

  delete pm;
  pm=NULL;
  CloseHandle(UidlMutex); //uidl share mutex
}

BOOL WINAPI Handler(DWORD event)
{
  if (event == CTRL_BREAK_EVENT && farmail_to_handler)
  {
    ((FARMail*)farmail_to_handler)->ProcessEvent(FE_BREAK,NULL);
    return TRUE;
  }
  return FALSE;
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  (void) Item;
  if ( _Info.StructSize < sizeof( _Info ) )
  {
    SayError( GetMsg( MesOldVer ) );
    return(INVALID_HANDLE_VALUE);
  }

  HANDLE hPlugin=new FARMail;
  if (hPlugin==NULL)
    return(INVALID_HANDLE_VALUE);

  if ( OpenFrom == OPEN_EDITOR )
  {
    farmail_to_handler=hPlugin;
    SetConsoleCtrlHandler(Handler,TRUE);
    ( (FARMail*)hPlugin )->SendFromEditor();
    SetConsoleCtrlHandler(Handler,FALSE);
    farmail_to_handler=NULL;
    delete (FARMail*)hPlugin;
    return(INVALID_HANDLE_VALUE);

  }
  else if (OpenFrom == OPEN_COMMANDLINE)
  {
    if (strstr((const char *)Item,":"))
    {
      farmail_to_handler=hPlugin;
      SetConsoleCtrlHandler(Handler,TRUE);
      int ret = ( (FARMail*)hPlugin )->ProcessCommandLine((const char *)Item);
      SetConsoleCtrlHandler(Handler,FALSE);
      farmail_to_handler=NULL;
      delete (FARMail*)hPlugin;
      if (!ret) SayError(GetMsg(MesBadCmdLine));
      return(INVALID_HANDLE_VALUE);
    }
  }

  return(hPlugin);
}

void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
  delete (FARMail *)hPlugin;
}

int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  FARMail *Browser=(FARMail *)hPlugin;
  return(Browser->GetFindData(pPanelItem,pItemsNumber,OpMode));
}

void WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  FARMail *Browser=(FARMail *)hPlugin;
  Browser->FreeFindData(PanelItem,ItemsNumber);
}

void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR;
  if(!Opt.AddToPluginMenu) Info->Flags|=PF_DISABLEPANELS;
  static const char *DiskMenuStrings[1];
  DiskMenuStrings[0]=GetMsg(MesDiskMenuTitle);
  static int DiskMenuNumbers[1];
  Info->DiskMenuStrings=DiskMenuStrings;
  DiskMenuNumbers[0]=Opt.DisksMenuDigit;
  Info->DiskMenuNumbers=DiskMenuNumbers;
  Info->DiskMenuStringsNumber=Opt.AddToDisksMenu ? 1:0;
  static const char *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MesPluginMenuTitle);
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);

  static const char *PluginCfgStrings[1];
  PluginCfgStrings[0]=GetMsg(MesConfig_MainTitle);
  Info->PluginConfigStrings=PluginCfgStrings;
  Info->PluginConfigStringsNumber=sizeof(PluginCfgStrings)/sizeof(PluginCfgStrings[0]);

  static const char *prefix = "mail";
  Info->CommandPrefix = prefix;
}

void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  FARMail *Browser=(FARMail *)hPlugin;
  Browser->GetOpenPluginInfo(Info);
}

int WINAPI _export SetDirectory(HANDLE hPlugin,const char *Dir,int OpMode)
{
  FARMail *Browser=(FARMail *)hPlugin;
  return(Browser->SetDirectory(Dir,OpMode));
}

int WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  FARMail *Browser=(FARMail *)hPlugin;
  return(Browser->DeleteFiles(PanelItem,ItemsNumber,OpMode));
}

int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  FARMail *Browser=(FARMail *)hPlugin;
  return(Browser->ProcessKey(Key,ControlState));
}

int WINAPI _export GetFiles( HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, char *DestPath, int OpMode)
{
  FARMail *Browser=(FARMail *)hPlugin;
  int ret = Browser->GetFiles(PanelItem, ItemsNumber, Move, DestPath, OpMode);
  SetRegKey2( HKEY_CURRENT_USER, PluginCommonKey, NULLSTR, UNIQUE , Opt.Unique );
  return(ret);
}

int WINAPI _export ProcessEvent( HANDLE hPlugin, int Event, void *Param )
{
  FARMail *Browser=(FARMail *)hPlugin;
  return(Browser->ProcessEvent( Event, Param ) );
}

int WINAPI _export Configure(int ItemNumber)
{
  switch(ItemNumber)
  {
    case 0:
      if ( !ConstructCharset(GetCharsetTable()) )
      {
        FARMailConfig();
        DestructCharset(GetCharsetTable());
        return 0;
      }
  }
  return(FALSE);
}

int WINAPI _export PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode)
{
  FARMail *Browser=(FARMail *)hPlugin;
  return(Browser->PutFiles(PanelItem,ItemsNumber,Move,OpMode));
}
