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
#include "stdafx.h"
#include "FarCopyDlg.h"
#include <FarPlus.h>
#include <stdio.h>
#include <FarFile.h>

#include "LangID.h"
#include "datetime.h"

/*
00000000001111111111222222222233333333334444444444
01234567890123456789012345678901234567890123456789
ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ
ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ
ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ
ÛÛÛÛÛ                                                  ÛÛÛÛÛ
ÛÛÛÛÛ   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ Copy ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»   ÛÛÛÛÛ
ÛÛÛÛÛ   º Copying the file                         º   ±±ÛÛÛ
ÛÛÛÛÛ   º X:\Path\Name.Ext                         º   ±±ÛÛÛ
ÛÛÛÛÛ   º to                                       º   ±±ÛÛÛ
ÛÛÛÛÛ   º Y:\Path\Name.Ext                         º   ±±ÛÛÛ
ÛÛÛÛÛ   º °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° º   ±±ÛÛÛ
ÛÛÛÛÛ   º ÄÄÄÄÄÄÄÄÄÄÄÄÄ Total: 9,999 ÄÄÄÄÄÄÄÄÄÄÄÄÄ º   ±±ÛÛÛ
ÛÛÛÛÛ   º °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° º   ±±ÛÛÛ
ÛÛÛÛÛ   º ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ º   ±±ÛÛÛ
ÛÛÛÛÛ   º Time: 00:00:00 Left: 00:00:00      0 b/s º   ±±ÛÛÛ
ÛÛÛÛÛ   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   ±±ÛÛÛ
ÛÛÛÛÛ                                                  ±±ÛÛÛ
ÛÛÛÛÛÛÛ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±ÛÛÛ
ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ
ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ
*/


FarCopyDlg::~FarCopyDlg()
{
  Far::RestoreScreen( m_hScreen );
}

FarCopyDlg::FarCopyDlg( int nTotalSize )
: m_StartTime( GetTickCount() ),
  m_TotalSize( nTotalSize ),
  m_CurSize( 0 ),
  m_hScreen( Far::SaveScreen() ),
  m_AcceptForAll( opNone ),
  m_Confirmations( 0 )
{
  Init();
}

void FarCopyDlg::Init()
{
  m_Items[ 0 ] = (LPSTR)Far::GetMsg( MdlgCopy_Copy );
  m_Items[ 1 ] = (LPSTR)Far::GetMsg( MdlgCopy_CopyingTheFile );
  m_Items[ 2 ] = (LPSTR)STR_EmptyStr;
  m_Items[ 3 ] = (LPSTR)Far::GetMsg( MdlgCopy_To );
  m_Items[ 4 ] = (LPSTR)STR_EmptyStr;
  m_Items[ 5 ] = m_CurBar;
  m_Items[ 6 ] = m_Total;
  m_Items[ 7 ] = m_AllBar;
  static char sep[] = "\1";
  m_Items[ 8 ] = sep;
  m_Items[ 9 ] = m_Times;
  m_Times[ 0 ] = '\0';

  FarSF::snprintf( m_Total, sizeof(m_Total), "\1 %s: ", Far::GetMsg( MdlgCopy_Total ) );

  div_t dt; dt.rem = m_TotalSize;
  for ( int i = 1000000000; i >= 1000; i /= 1000 )
  {
    dt = div( dt.rem, i );
    if ( dt.quot )
      strcat( FarSF::itoa( dt.quot, m_Total + strlen( m_Total ), 10 ), "," );
  }
  strcat( FarSF::itoa( dt.rem, m_Total + strlen( m_Total ), 10 ), "\x20" );

  memset( m_CurBar, '\xB0', 40 ); m_CurBar[ 40 ] = '\0';
  memset( m_AllBar, '\xB0', 40 ); m_AllBar[ 40 ] = '\0';

  if ( Far::AdvControl( ACTL_GETINTERFACESETTINGS, 0 ) & FIS_SHOWCOPYINGTIMEINFO )
    m_ItemsCount = 10;
  else
    m_ItemsCount = 8;

  m_Confirmations = Far::AdvControl( ACTL_GETCONFIRMATIONS, 0 );

  FarMessage::Show( FMSG_LEFTALIGN, NULL, m_Items, m_ItemsCount, 0 );
}

FarCopyDlg::TOperation FarCopyDlg::Copy( LPCSTR From, LPCSTR To, int nSize, const FILETIME& WriteTime )
{
  Ticks time( Ticks::now() - m_StartTime );

  SYSTEMTIME st;
  time.toFileTime().toSystemTime( st );

  int sz;

  m_Items[ 2 ] = strcpy( create char[ strlen( From ) + 40 ], From );
  FarSF::TruncPathStr( m_Items[ 2 ], 40 );
  sz = strlen( m_Items[ 2 ] );
  memset( m_Items[ 2 ] + sz, '\x20', 40 - sz );
  m_Items[ 2 ][ 40 ] = '\0';

  m_Items[ 4 ] = strcpy( create char[ strlen( To ) + 40 ], To );
  FarSF::TruncPathStr( m_Items[ 4 ], 40 );
  sz = strlen( m_Items[ 4 ] );
  memset( m_Items[ 4 ] + sz, '\x20', 40 - sz );
  m_Items[ 4 ][ 40 ] = '\0';

  m_CurSize += nSize;
  if ( m_CurSize > m_TotalSize )
    m_CurSize = m_TotalSize;

  memset( m_AllBar, '\xDB', m_CurSize * 40 / m_TotalSize );

  float Left = m_CurSize * time ? float( m_TotalSize - m_CurSize ) / m_CurSize * time : 0.f;

  SYSTEMTIME lt; Ticks( (DWORD)Left ).toFileTime().toSystemTime( lt );

#define SPEED_MOD (1024*9)
  int Speed = time > 0 ? int( (m_CurSize+0.5f) / (time/1000.f) ) : 0;
  char p = '\x20';
  if ( Speed / SPEED_MOD > 0 )
  {
    p = 'K';
    Speed /= 1024;
    if ( Speed / SPEED_MOD > 0 )
    {
      p = 'M';
      Speed /= 1024;
      if ( Speed / SPEED_MOD > 0 )
      {
        p = 'G';
        Speed /= 1024;
        if ( Speed / SPEED_MOD > 0 )
        {
          p = 'T';
          Speed /= 1024;
        }
      }
    }
  }

  FarSF::snprintf( m_Times, sizeof(m_Times), Far::GetMsg( MdlgCopy_TimeElapsedAndLeft ),
    st.wHour, st.wMinute, st.wSecond,
    lt.wHour, lt.wMinute, lt.wSecond, Speed, p );

  TOperation op = opNone;

  static DWORD LastTicks = 0;
  if ( GetTickCount() - LastTicks > 100 )
  {
    FarMessage::Show( FMSG_LEFTALIGN|FMSG_KEEPBACKGROUND, NULL, m_Items, m_ItemsCount, 0 );
    LastTicks = GetTickCount();
    if ( FarSF::CheckForEsc() )
    {
      FarMessage Msg( FMSG_WARNING, NULL );
      Msg.AddLine( MConfirm );
      Msg.AddLine( MAbortOperation );
      Msg.AddButton( MYes );
      Msg.AddButton( MNo );
      if ( (m_Confirmations&FCS_INTERRUPTOPERATION)==0 || Msg.Show() == 0 )
        op = opCancel;
    }
  }

  delete [] m_Items[ 2 ];
  delete [] m_Items[ 4 ];

  if ( op == opCancel )
    return op;

  if ( m_AcceptForAll != opNone )
    return (TOperation)m_AcceptForAll;

  if ( (m_Confirmations&FCS_COPYOVERWRITE)==0 )
    return opOverwrite;

  if ( GetFileAttributes( To ) != (DWORD)-1 )
  {
    FarMessage msg( FMSG_WARNING|FMSG_DOWN, ":CopyFiles" );
    msg.AddLine( MWarning );
    msg.AddLine( MFileAlreadyExists );
    msg.AddLine( FarSF::TruncPathStr( To, 64 ) );
    msg.AddSeparator();

    WIN32_FIND_DATA fd;
    FarFileInfo::GetInfo( To, fd );


    FarString TimeNew = ((FileTime)WriteTime).toString( "dd.MM.yyyy", "hh:mm:ss" );
    FarString TimeOld = ((FileTime)fd.ftLastWriteTime ).toString( "dd.MM.yyyy", "hh:mm:ss" );

    if ( TimeNew.Length() > TimeOld.Length() )
    {
      TimeOld = FarString( '\x20', TimeNew.Length() - TimeOld.Length() ) + TimeOld;
    }
    else if ( TimeNew.Length() < TimeOld.Length() )
    {
      TimeNew = FarString( '\x20', TimeOld.Length() - TimeNew.Length() ) + TimeNew;
    }

    LPCSTR mNew = Far::GetMsg( MNew );
    msg.AddFmt( "%s%*d %s", mNew, 28-strlen( mNew ), nSize, TimeNew.c_str() );

    LPCSTR mOld = Far::GetMsg( MExisting );
    msg.AddFmt( "%s%*d %s", mOld, 28-strlen( mOld ), fd.nFileSizeLow, TimeOld.c_str() );

    msg.AddSeparator();
    msg.AddButton( MOverwrite );
    msg.AddButton( MAll );
    msg.AddButton( MSkip );
    msg.AddButton( MSkipAll );
    //msg.AddButton( MAppend );
    msg.AddButton( MCancel );

    switch ( msg.Show() )
    {
    case 3:
      m_AcceptForAll = opSkip;
    case 2:
      op = opSkip;
      break;
    case 1:
      m_AcceptForAll = opOverwrite;
    case 0:
      op = opOverwrite;
      break;
    default:
      op = opCancel;
      break;
    }
  }
  else
    op = opOverwrite;

  return op;
}
