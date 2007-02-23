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
// MailboxCfg.cpp: implementation of the CMailboxCfg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MailboxCfg.h"
#include "LangId.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define MAX_DEFAULT_CHARSET 31

CMailboxCfg::CMailboxCfg( LPCSTR IniFile, CMailboxCfg * Parent )
  : CIniConfig         ( IniFile )
  , m_Parent           ( Parent )
  , m_StartPanelMode   ( 0 )
  , m_StartSortMode    ( 0 )
  , m_StartSortOrder   ( 0 )
  , m_TheradsViewMode  ( 0 )
  , m_TreeViewMode     ( 11 )
  , m_TreeViewMode2    ( 0 )
  , m_MinCacheSize     ( 250 )
  , m_ProcessSubject   ( TRUE )
  , m_MarkMsgAsReadTime( 3000 )
  , m_AllowWriteAccess ( 0 )
  , m_DefaultCharset   ( create char[ MAX_DEFAULT_CHARSET + 1 ] )
  , m_AttachDir        ( create char[ 0x1000 ] )
  , m_ConfigExt        ( create char[ 64 ] )
  , m_CacheExt         ( create char[ 64 ] )
  , m_emptySubj        ( "<none>" )
  , m_UseNTFSStreams   ( false )
  , m_UseAttributeHighlighting (false)
  , m_bModified        ( false )
  , m_copyOutputFormat ( "%MsgId%" )
{
  *m_DefaultCharset = '\0';
  strcpy( m_ConfigExt, ".mbs" );
  strcpy( m_CacheExt,  ".mbc" );

  memset( &m_PanelMode, 0, sizeof( PanelMode ) );
  m_PanelMode.ColumnTypes        = create char[ 32 ];
  m_PanelMode.ColumnWidths       = create char[ 32 ];
  m_PanelMode.StatusColumnTypes  = create char[ 32 ];
  m_PanelMode.StatusColumnWidths = create char[ 32 ];

  strcpy( m_PanelMode.ColumnWidths, "0,2" );
  strcpy( m_PanelMode.StatusColumnWidths, "0,6,8" );

  char * Tmp = create char[ 128 ];
  strcpy( Tmp, "S,F" );
  ConvertColumnTypesToFarFormat( Tmp, m_PanelMode.ColumnTypes );
  strcpy( Tmp, "AF,P,MS" );
  ConvertColumnTypesToFarFormat( Tmp, m_PanelMode.StatusColumnTypes );

  m_PanelMode.FullScreen = FALSE;

  *m_AttachDir = '\0';

  if ( Parent == NULL )
    Parent = this;

  ReadString( "DefaultCharset", Parent->m_DefaultCharset, m_DefaultCharset, 32 );
  ReadString( "ColumnWidths", Parent->m_PanelMode.ColumnWidths, m_PanelMode.ColumnWidths, 128 );
  ReadString( "StatusColumnWidths", Parent->m_PanelMode.StatusColumnWidths, m_PanelMode.StatusColumnWidths, 128 );

  if ( ReadString( "ColumnTypes", NULL, Tmp, 128 ) == 0 )
    strcpy( m_PanelMode.ColumnTypes, Parent->m_PanelMode.ColumnTypes );
  else
    ConvertColumnTypesToFarFormat( Tmp, m_PanelMode.ColumnTypes );

  if ( ReadString( "StatusColumnTypes", NULL, Tmp, 128 ) == 0 )
    strcpy( m_PanelMode.StatusColumnTypes, Parent->m_PanelMode.StatusColumnTypes );
  else
    ConvertColumnTypesToFarFormat( Tmp, m_PanelMode.StatusColumnTypes );

  m_PanelMode.FullScreen = ReadInt( "FullScreen", Parent->m_PanelMode.FullScreen );
  m_StartPanelMode       = ReadInt( "StartPanelMode", Parent->m_StartPanelMode );
  m_StartSortMode        = ReadInt( "StartSortMode", Parent->m_StartSortMode );
  m_StartSortOrder       = ReadInt( "StartSortOrder", Parent->m_StartSortOrder );
  m_TheradsViewMode      = ReadInt( "TheradsViewMode", Parent->m_TheradsViewMode );

  m_TreeViewMode         = ReadInt( "TreeViewMode", Parent->m_TreeViewMode );
  m_MinCacheSize         = ReadInt( "MinCacheSize", Parent->m_MinCacheSize );
  m_ProcessSubject       = ReadInt( "ProcessSubject", Parent->m_ProcessSubject );

  m_MarkMsgAsReadTime    = ReadInt( "MarkMsgAsReadTime", Parent->m_MarkMsgAsReadTime );

  m_AllowWriteAccess     = ReadInt( "AllowWriteAccess", Parent->m_AllowWriteAccess );

  m_UseNTFSStreams       = ReadBool( "UseNTFSStreams", Parent->m_UseNTFSStreams );
  m_UseAttributeHighlighting = ReadBool( "UseAttributeHighlighting", Parent->m_UseAttributeHighlighting );

  ReadString( "AttachDir", STR_EmptyStr, m_AttachDir, 0x1000 );

  ReadString( "CacheExtension", Parent->m_CacheExt, m_CacheExt, 64 );
  ReadString( "SettingsExtension", Parent->m_ConfigExt, m_ConfigExt, 64 );

  LPSTR tmpvalue;

  tmpvalue = m_emptySubj.GetBuffer( 256 );
  ReadString( "EmptySubj", Parent->m_emptySubj, tmpvalue, 256 );
  m_emptySubj.ReleaseBuffer();

  tmpvalue = m_copyOutputFormat.GetBuffer( 256 );
  ReadString( "CopyOutputFormat", Parent->m_copyOutputFormat, tmpvalue, 256 );
  m_copyOutputFormat.ReleaseBuffer();

  if ( m_StartPanelMode > 9 || m_StartPanelMode < 0 )
    m_StartPanelMode = 0;

  m_PanelMode.ColumnTitles = create LPSTR[ 32 ];
  strncpy( Tmp, m_PanelMode.ColumnTypes, 128 );
  LPSTR p = strtok( Tmp, "\x20,;" );
  for ( int i = 0; i < 32 && p; i ++, p = strtok( NULL, "\x20,;" ) )
  {
    m_PanelMode.ColumnTitles[ i ] = (LPSTR)GetTitle( p );
  }
  delete [] Tmp;

  m_TreeViewMode2 = 0;
  if ( m_TreeViewMode & TVM_DRAW_TREE )
  {
    if ( m_TreeViewMode & TVM_WIDE_TREE )
      m_TreeViewMode2 = 2;
    if ( m_TreeViewMode & TVM_INSERT_SP )
      m_TreeViewMode2 ++;
  }
  else
  {
    m_TreeViewMode2 &= ~TVM_WIDE_TREE;
    m_TreeViewMode2 &= ~TVM_DRAW_ROOT;
  }

}

#define WriteIntChanges( name ) \
if ( m_Parent->m_##name != m_##name ) WriteInt( "Mailbox", #name, m_##name ); \
else EraseKey( INI_MAILBOX, #name ) \

#define WriteBoolChanges( name ) \
if ( m_Parent->m_##name != m_##name ) WriteBool( "Mailbox", #name, m_##name ); \
else EraseKey( INI_MAILBOX, #name ) \

#define WriteStringChanges( name ) \
  if ( FarSF::LStricmp( m_Parent->m_##name, m_##name ) != 0 ) WriteString( "Mailbox", #name, m_##name ); \
  else EraseKey( INI_MAILBOX, #name ) \

CMailboxCfg::~CMailboxCfg()
{
  if ( m_bModified && m_Parent )
  {
    WriteIntChanges( StartSortMode );
    WriteIntChanges( StartSortOrder );
    WriteIntChanges( TheradsViewMode );
    WriteBoolChanges( UseNTFSStreams );

    WriteStringChanges( copyOutputFormat );

    if ( FarSF::LStricmp( m_Parent->m_DefaultCharset, m_DefaultCharset ) == 0 )
      EraseKey( INI_MAILBOX, "DefaultCharset" );
    else
      WriteString( INI_MAILBOX, "DefaultCharset", m_DefaultCharset );

    if ( IsEmptySection( INI_MAILBOX ) )
      DeleteFile( GetIniFileName() );
  }

  delete [] m_AttachDir;
  delete [] m_DefaultCharset;
  delete [] m_ConfigExt;
  delete [] m_CacheExt;

  delete [] m_PanelMode.ColumnTypes;
  delete [] m_PanelMode.ColumnWidths;
  delete [] m_PanelMode.StatusColumnTypes;
  delete [] m_PanelMode.StatusColumnWidths;
  delete [] m_PanelMode.ColumnTitles;
}

//////////////////////////////////////////////////////////////////////////
// MailView  - Far     - Description
// S         - Z       - Subject
// F         - C2      - Flags
// AF        - O       - Address From
// AT        - C0      - Address To
// P         - C1      - Priority
// DM[B,M]   - DM[B,M] - Received
// DC[B,M]   - DC[B,M] - Sent
// DA[B,M]   - DA[B,M] - Accessed
//                       где: B - краткий (в стиле Unix) формат времени файла;
//                            M - использование текстовых имен месяцев;
// MS[C,T]   - S[C,T]  - размер файла
//                       где: C - форматировать размер файла запятыми
//                            T - использовать 1000 вместо 1024 как делитель,
//                                если ширины колонки не хватает для показа
//                                полного размера файла

void CMailboxCfg::ConvertColumnTypesToFarFormat( LPSTR inPtr, LPSTR outPtr )
{
  *outPtr = '\0';

  LPSTR Tmp = create char[ 256 ];

  for ( LPSTR p = strtok( inPtr, "\x20,;" ); p; p = strtok( NULL, "\x20,;" ) )
  {
    switch ( FarSF::LUpper( *p ) )
    {
    case 'S':
      strcpy( Tmp, "Z" );
      break;
    case 'A':
      *Tmp = 0;
      if ( strlen( p ) > 1 )
      {
        if ( FarSF::LUpper( p[ 1 ] ) == 'F' )
          strcpy( Tmp, "O" );
        else if ( FarSF::LUpper( p[ 1 ] ) == 'T' )
          strcpy( Tmp, "C0" );
      }
      break;
    case 'P':
      strcpy( Tmp, "C1" );
      break;
    case 'F':
      strcpy( Tmp, "C2" );
      break;
    case 'D':
      /*if ( strlen( p ) > 1 && FarSF::LUpper( p[ 1 ] ) != 'C' )
        p[ 1 ] = 'M';
      */strcpy( Tmp, p );
      break;
    case 'M':
      strcpy( Tmp, p + 1 );
      break;
    default :
      *Tmp = 0;
      break;
    }

    if ( *Tmp )
    {
      if ( *outPtr )
        strcat( outPtr, "," );
      strcat( outPtr, Tmp );
    }
  }
  delete [] Tmp;
}

LPCSTR CMailboxCfg::GetTitle( LPSTR szPanelMode )
{
  if ( szPanelMode[ 0 ] == 'Z' )
  {
    return Far::GetMsg( MSubject );
  }
  else if ( szPanelMode[ 0 ] == 'C' )
  {
    if ( szPanelMode[ 1 ] == '0' )
      return Far::GetMsg( MTo );
    else if ( szPanelMode[ 1 ] == '1' )
      return Far::GetMsg( MPriority );
    else if ( szPanelMode[ 1 ] == '2' )
      return Far::GetMsg( MFlags );
  }
  else if ( szPanelMode[ 0 ] == 'O' )
  {
    return Far::GetMsg( MFrom );
  }
  else if ( szPanelMode[ 0 ] == 'D' )
  {
    if ( szPanelMode[ 1 ] == 'M' )
      return Far::GetMsg( MReceived );
    else if ( szPanelMode[ 1 ] == 'C' )
      return Far::GetMsg( MSent );
    else if ( szPanelMode[ 1 ] == 'A' )
      return Far::GetMsg( MAccessed );
  }
  //if ( szPanelMode[ 0 ] == 'S' ) // пущай размер пишет фар
  return NULL;
}

void CMailboxCfg::SetTheradsViewMode( TTheradsViewMode mode )
{
  if ( mode != m_TheradsViewMode )
  {
    m_bModified = true;
    m_TheradsViewMode = (int)mode;
  }
}

void CMailboxCfg::SetStartSortMode( TSortMode mode )
{
  if ( mode != m_StartSortMode )
  {
    m_bModified = true;
    m_StartSortMode = mode;
  }
}

void CMailboxCfg::SetStartSortOrder( int order )
{
  if ( order != m_StartSortOrder )
  {
    m_bModified = true;
    m_StartSortOrder = order;
  }
}

void CMailboxCfg::SetDefaultCharset( LPCSTR charset )
{
  far_assert( charset != NULL );
  if ( FarSF::LStricmp( m_DefaultCharset, charset ) != 0 )
  {
    m_bModified = true;
    strncpy( m_DefaultCharset, charset, MAX_DEFAULT_CHARSET );
  }
}

void CMailboxCfg::setCopyOutputFormat( const FarString& value )
{
  if ( m_copyOutputFormat.CompareNoCase( value ) != 0 )
  {
    m_bModified = true;
    m_copyOutputFormat = value;
  }
}
