/*
 MailView plugin for FAR Manager
 Copyright (C) 2005 Alex Yaroslavsky
 Copyright (C) 2002-2003 Dennis Trachuk

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
// MsgBaseMan.cpp: implementation of the CMailView class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FarDialog.h"
#include <FarLog.h>
#include <FarMenu.h>

#include "MailView.h"

#include "Message.h"
#include "FarMailbox.h"
#include "FarInetMessage.h"
#include "FarFidoMessage.h"
#include "FarWebArchive.h"
#include "FarInetNews.h"
#include "MailViewDlg.h"
#include "MailViewTpl.h"
#include "File.h"

FarFileName ExtractRelativePath( LPCSTR BaseName, LPCSTR DestName );

PMessage CreateFarMessage( LPCSTR fileName, long encoding, bool headOnly );


LPCSTR STR_NotImplemented = "sorry, this feature is not implemented";

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
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
#else
/////////////////////////////////////////////////////////////////////////////
// Standard DLL Entry-Point
BOOL APIENTRY DllMain( HANDLE hModule, DWORD ulReasonForCall, LPVOID lpReserved )
{
/*  switch ( ulReasonForCall )
  {
  case DLL_PROCESS_ATTACH:
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
    break;
  }
*/
    return TRUE;
}
#endif
/////////////////////////////////////////////////////////////////////////////
// Plugin Declaration
DECLARE_PLUGIN( CMailView )
/////////////////////////////////////////////////////////////////////////////

CMailView * CMailView::m_Handle   = NULL;
/////////////////////////////////////////////////////////////////////////////
// Constructor
CMailView::CMailView() : FarCustomPlugin()
{
  m_Handle = this;

  Far::AddPluginConfig( MMailView );
  Far::SetCommandPrefix( "mv:mb:mve:mvv" );
  Far::SetPluginFlags( PF_FULLCMDLINE );

  m_ThisRoot = Far::GetModuleName();
  m_IniFile  = m_ThisRoot;
  m_ThisRoot = m_ThisRoot.GetPath();
  m_IniFile.SetExt( ".ini" );

  m_MailboxCfg = create CMailboxCfg( m_IniFile, NULL );

  CIniConfig ini( m_IniFile );
  //////////////////////////////////////////////////////////////////////////
#ifndef _USE_FARRFCCHARSET
  m_MLang = create FarMultiLang( ini );
#else
  m_MLang = create FarMultiLang;
#endif

  char * Tmp = create char[ 0x8000 ];

  int Len = ini.ReadString( INI_GENERAL, "PluginsPath", m_ThisRoot, Tmp, 0x8000 );
  FarSF::AddEndSlash( Tmp );
  if ( Len <= 0 )
    strcat( Tmp, "Plugins\\" );


  m_Plugins.Search( Tmp );
  m_Plugins.LoadSettings( ini );

  m_MsgCfg.LoadSettings( ini );
  m_NwsCfg.LoadSettings( ini );
  m_MhtCfg.LoadSettings( ini );

  ini.ReadString( INI_MESSAGE, "WarnBeforeExecuting", STR_EmptyStr, Tmp, 0x8000 );
  m_DangerFilesMasks = Tmp;

  m_DangerFilesWarnRule = ini.ReadInt( INI_MESSAGE, "WarnBeforeExecutingRule", 4 );

  ini.ReadString( INI_MESSAGE, "VirusChecker", STR_EmptyStr, Tmp, 0x8000 );
  m_VirusChecker = Tmp;

  m_VirusCheckerSuccessCode = ini.ReadInt( INI_MESSAGE, "VirusCheckerSuccessCode", 0 );

  m_MessageStartPanelMode = ini.ReadInt( INI_MESSAGE, "StartPanelMode", 0 );
  if ( m_MessageStartPanelMode > 9 || m_MessageStartPanelMode < 0 )
    m_MessageStartPanelMode = 0;

  FarString ExcludeMasksIncl, ExcludeMasksExcl;

  for ( int i = 0; i < 10; i ++ )
  {
    m_ExcludeMasks = "ExcludeMasks";
    if ( i > 0 )
      m_ExcludeMasks += FarSF::itoa( i, Tmp, 10 );
    ini.ReadString( INI_GENERAL, m_ExcludeMasks, STR_EmptyStr, Tmp, 0x8000 );
    char * Ptr = strchr( Tmp, '|' );
    if ( Ptr )
    {
      *Ptr = '\0';
      if ( *(++Ptr) )
      {
        if ( !ExcludeMasksExcl.IsEmpty() )
          ExcludeMasksExcl += ',';
        ExcludeMasksExcl += Ptr;
      }
    }
    if ( *Tmp )
    {
      if ( !ExcludeMasksIncl.IsEmpty() )
        ExcludeMasksIncl += ',';
      ExcludeMasksIncl += Tmp;
    }
  }
  m_ExcludeMasks = ExcludeMasksIncl;
  if ( !ExcludeMasksExcl.IsEmpty() )
    m_ExcludeMasks += '|' + ExcludeMasksExcl;

  delete [] Tmp;

  m_ViewCfg.LoadSettings( ini );
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
CMailView::~CMailView()
{
  SaveSettings( true );

  delete m_MLang;
  delete m_MailboxCfg;

  m_Handle = NULL;
}

/////////////////////////////////////////////////////////////////////////////
//
void CMailView::SaveSettings( bool bForce )
{
  if ( bForce || Far::AdvControl( ACTL_GETSYSTEMSETTINGS, 0 ) & FSS_AUTOSAVESETUP )
  {
    CIniConfig ini( m_IniFile );

    m_MsgCfg.SaveSettings( ini );
    m_NwsCfg.SaveSettings( ini );
    m_MhtCfg.SaveSettings( ini );
    m_Plugins.SaveSettings( ini );
    m_ViewCfg.SaveSettings( ini );
  }
}

/////////////////////////////////////////////////////////////////////////////
//
HANDLE CMailView::Open( const int OpenFrom, const int Item )
{
  HANDLE hPlugin;
  switch ( OpenFrom )
  {
  case OPEN_COMMANDLINE :
    hPlugin = OpenFile( (LPCSTR)Item, NULL, 0 );
    break;
  default:
    hPlugin = INVALID_HANDLE_VALUE;
    break;
  }
  return hPlugin;
}

/////////////////////////////////////////////////////////////////////////////
//
HANDLE CMailView::OpenFile( LPCSTR Name, const BYTE * Data, const int DataSize )
{
//  HANDLE hFile = CreateFile( "D:\\1.htm", GENERIC_READ|GENERIC_WRITE,
//    FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL );
  if ( Name == NULL || *Name == 0 )
    return INVALID_HANDLE_VALUE;

  FarString fileName;
  if ( FarSF::LStrnicmp( Name, "mv:", 3 ) == 0 || FarSF::LStrnicmp( Name, "mb:", 3 ) == 0 )
    fileName = Name + 3;
  else if ( FarSF::LStrnicmp( Name, "mve:", 3 ) == 0 || FarSF::LStrnicmp( Name, "mvv:", 3 ) == 0 )
    fileName = Name + 4;
  else
    fileName = Name;

  fileName.Expand().Unquote().Trim();

  if ( !File::exists( fileName ) || File::isDirectory( fileName ) )
    return INVALID_HANDLE_VALUE;

  if ( !m_ExcludeMasks.IsEmpty() && FarSF::CmpNameListEx( m_ExcludeMasks, fileName ) )
    return INVALID_HANDLE_VALUE;

  if ( FarSF::LStrnicmp( Name, "mv:", 3 ) == 0 )
  {
    bool bFullWnd  = m_ViewCfg.GetFullScreen();
    bool bWordWrap = m_ViewCfg.GetWordWrap();

    DWORD tc = GetTickCount();

    while ( CMailViewDialog( fileName, bFullWnd, FCT_DETECT ).Execute( &bWordWrap ) == NULL )
      bFullWnd = !bFullWnd;

    m_ViewCfg.SetFullScreen( bFullWnd );
    m_ViewCfg.SetWordWrap( bWordWrap );

    tc = GetTickCount() - tc;
    if ( tc > (DWORD)m_MailboxCfg->GetMarkMsgAsReadTime() )
    {
      DWORD attrs = GetFileAttributes( fileName );
      if ( attrs != (DWORD)-1 )
        SetFileAttributes( fileName, attrs | FILE_ATTRIBUTE_ARCHIVE );
    }

    return INVALID_HANDLE_VALUE;
  }

  if ( FarSF::LStrnicmp( Name, "mvv:", 4 ) == 0 || FarSF::LStrnicmp( Name, "mve:", 4 ) == 0 )
  {
    PMessage Msg = CreateFarMessage( fileName, FCT_DEFAULT, false );

    if ( Name[ 2 ] == 'e' || Name[ 2 ] == 'E' )
      CMailViewTpl( ViewCfg()->GetETpl(), Msg, FCT_DETECT ).View( true, ViewCfg()->GetLockView() );
    else
      CMailViewTpl( ViewCfg()->GetVTpl(), Msg, FCT_DETECT ).View( false, ViewCfg()->GetLockView() );

    delete Msg;

    return INVALID_HANDLE_VALUE;
  }

  LPCSTR PointToName = File::getName( fileName );
  if ( PointToName == NULL )
    PointToName = fileName.c_str();

  HANDLE hPlugin = INVALID_HANDLE_VALUE;

  for ( int i = 0; i < m_Plugins.Count(); i ++ )
  {
    if ( !FarSF::CmpNameListEx( m_Plugins.GetFilesMasks( i ), PointToName ) )
      continue;

    CFarMailbox * MBox = create CFarMailbox( m_Plugins.GetLibrary( i ), fileName, m_MailboxCfg );
    if ( MBox->Open() )
    {
      hPlugin = (HANDLE) MBox;
      break;
    }

    delete MBox;
  }

  if ( hPlugin == INVALID_HANDLE_VALUE )
  {
    PMessage Msg = NULL;
    if ( FarSF::CmpNameListEx( m_MhtCfg.GetFilesMasks(), fileName ) )
      Msg = create CFarWebArchive( fileName );
    else if ( FarSF::CmpNameListEx( m_NwsCfg.GetFilesMasks(), fileName ) )
      Msg = create CFarInetNews( fileName );
    else if ( FarSF::CmpNameListEx( m_MsgCfg.GetFilesMasks(), fileName ) )
      Msg = CreateFarMessage( fileName, FCT_DEFAULT, false );
    if ( Msg )
      hPlugin = (HANDLE) create CMessage( Msg, fileName );
  }

  return hPlugin;
}

/////////////////////////////////////////////////////////////////////////////
//
FarString getCharsetName( long table )
{
  far_assert( CMailView::m_Handle != NULL );
  return CMailView::m_Handle->m_MLang->getCharsetName( table );
}

FarString getDisplayCharsetName( long table )
{
  far_assert( CMailView::m_Handle != NULL );
  return CMailView::m_Handle->m_MLang->getDisplayCharsetName( table );
}

void getFarCharacterTables( FarStringArray & name, FarIntArray & data, bool rfcOnly )
{
  far_assert( CMailView::m_Handle != NULL );
  CMailView::m_Handle->m_MLang->getFarCharacterTables( name, data, rfcOnly );
}

void getRfcCharacterTables( FarStringArray & name, FarIntArray & data, bool addHotKeys )
{
  far_assert( CMailView::m_Handle != NULL );
  CMailView::m_Handle->m_MLang->getRfcCharacterTables( name, data, addHotKeys );
}

long getCharacterTable( LPCSTR RFCName )
{
  far_assert( CMailView::m_Handle != NULL );
  far_assert( RFCName != NULL );

  return CMailView::m_Handle->m_MLang->getCharacterTable( RFCName,
    CMailView::m_Handle->m_MailboxCfg->GetDefaultCharset() );
}

long getCharacterTableNoDefault( LPCSTR RFCName )
{
  far_assert( CMailView::m_Handle != NULL );
  far_assert( RFCName != NULL );

  return CMailView::m_Handle->m_MLang->getCharacterTable( RFCName );
}

FarString ToOEMString( const FarString & Str, LPCSTR RfcEncoding )
{
  far_assert( RfcEncoding != NULL );
  far_assert( CMailView::m_Handle != NULL );

  return ToOEMString( Str, getCharacterTable( *RfcEncoding == 0 ?
    CMailView::m_Handle->m_MailboxCfg->GetDefaultCharset() : RfcEncoding ) );
}

bool CMailView::Configure( int ItemNumber )
{
  FarMenu mnu( MMailViewConfiguration );
  mnu.AddItem( MViewer )->SetData( (DWORD)&m_ViewCfg );
  mnu.AddSeparator();
  mnu.AddItem( MMessages )->SetData( (DWORD)&m_MsgCfg );
  mnu.AddItem( MNews )->SetData( (DWORD)&m_NwsCfg );
  mnu.AddItem( MWebArchives )->SetData( (DWORD)&m_MhtCfg );
  mnu.AddSeparator();
  mnu.AddItem( MMailboxes )->SetData( (DWORD)&m_Plugins );
  //mnu.AddSeparator();
  //mnu.AddItem( "About" );

  FarMenu::PItem Item;
  while ( ( Item = mnu.Show() ) != NULL )
  {
    if ( Item->GetData() )
    {
      ((CMailViewCfg*)Item->GetData())->Configure();
    }
    mnu.SelectItem( Item );
  }

  SaveSettings();

  return true;
}

/////////////////////////////////////////////////////////////////////////////
//
long SelectCharacterTableMenu( long SelectedTable, bool rfcOnly )
{
  far_assert( CMailView::m_Handle != NULL );
  return CMailView::m_Handle->m_MLang->SelectCharacterTableMenu( SelectedTable, rfcOnly );
}

void SplitPath( LPCSTR Path, LPSTR Drive, LPSTR Dir, LPSTR FName, LPSTR Ext )
{
  _splitpath( Path, Drive, Dir, FName, Ext );
  int sz = strlen( Dir );
  if ( strlen( Drive ) == 0 && sz > 3 && Dir[ 0 ] == '\\' && Dir[ 1 ] == '\\' )
  {
    LPSTR NewDir = Dir + 2;
    Drive[ 0 ] = Drive[ 1 ] = '\\';
    while ( (NewDir - Dir) < sz && *NewDir != '\\' )
    {
      Drive[ NewDir - Dir ] = *NewDir;
      NewDir ++;
    }
    Drive[ NewDir - Dir ] = '\0';
    //    strncpy( Drive, Dir, NewDir - Dir + 1 );
    strncpy( Dir, NewDir, sz - ( NewDir - Dir ) + 1 );
  }
}

int SplitDirs( LPSTR Path, LPCSTR * Dirs )
{
  int NumDirs = 0;
  for ( LPSTR tok = strtok( Path, "\\/" ); tok != NULL; tok = strtok( NULL, "\\/" ) )
  {
    Dirs[ NumDirs++ ] = tok;
  }
  return NumDirs;
}

FarFileName ExtractRelativePath( LPCSTR BaseName, LPCSTR DestName )
{
  char BaseDrive[ _MAX_PATH ], BasePath[ _MAX_PATH ], BaseFName[ _MAX_FNAME ], BaseExt[ _MAX_EXT ];
  char DestDrive[ _MAX_PATH ], DestPath[ _MAX_PATH ], DestFName[ _MAX_FNAME ], DestExt[ _MAX_EXT ];

  SplitPath( BaseName, BaseDrive, BasePath, BaseFName, BaseExt );
  SplitPath( DestName, DestDrive, DestPath, DestFName, DestExt );

  if ( FarSF::LStricmp( BaseDrive, DestDrive ) != 0 ||
    ( *BaseDrive != '\0' && BaseDrive[ 1 ] != ':' ) ||
    ( *DestDrive != '\0' && DestDrive[ 1 ] != ':' ) )
    return DestName;

  LPCSTR BaseDirs[ 255 ];
  LPCSTR DestDirs[ 255 ];

  int BaseDirCount = SplitDirs( BasePath, BaseDirs );
  int DestDirCount = SplitDirs( DestPath, DestDirs );

  int Level = 0;
  while ( Level < BaseDirCount && Level < DestDirCount &&
    FarSF::LStricmp( BaseDirs[ Level ], DestDirs[ Level ] ) == 0 )
    Level ++;

  FarFileName Result;

  for ( int i = Level; i < BaseDirCount; i ++ )
    Result += "..\\";

  for ( int i = Level; i < DestDirCount; i ++ )
  {
    Result += DestDirs[ i ];
    Result += '\\';
  }

  Result += DestFName;
  Result += DestExt;

  return Result;
}
// --------------------------------------------------------------------------

bool IsNotRelativePath( const char * FileName )
{
  return ( *FileName++ != '\0' && ( *FileName == ':' || ( *FileName == '\\' && *(++FileName) == '\\' ) ) );
  //return ( strlen( Name ) > 3 && ( Name[ 1 ] == ':' || ( Name[ 0 ] == '\\' && Name[ 1 ] == '\\' ) ) );
}

FarFileName ExtractFullPath( FarFileName& BaseName, FarFileName& DestName )
{
  if ( IsNotRelativePath( DestName ) )
    return DestName;

  char DestDrive[ _MAX_PATH ];
  char DestPath [ _MAX_PATH ];
  char DestFName[ _MAX_FNAME ];
  char DestExt  [ _MAX_EXT ];

  SplitPath( DestName, DestDrive, DestPath, DestFName, DestExt );

  FarString Temp = DestPath;
  Temp += DestFName;
  Temp += DestExt;

  return FarFileName::MakeName( BaseName.GetPath(), Temp ).GetFullName();
}

PMessage CreateFarMessage( LPCSTR fileName, long encoding, bool headOnly )
{
  if ( CFarFidoMessage::IsMessage( fileName ) )
    return create CFarFidoMessage( fileName, encoding, headOnly );
  return create CFarInetMessage( fileName, encoding, headOnly );
}
