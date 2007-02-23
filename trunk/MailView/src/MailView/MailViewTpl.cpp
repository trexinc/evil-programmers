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
#include "MailViewTpl.h"
#include <FarFile.h>
#include "Person.h"
#include "Decoder.h"
#include "FarInetMessage.h"
#include "Kludges.h"
#include "MsgLib/MimeContent.h"
#include "StrPtr.h"
#include "DateTime.h"
#include "WordWrap.h"
#include "MailView.h"
#include "File.h"
#include <ctype.h>

#define M_SUBJECT         "Subject"
#define M_OSUBJECT        "O"M_SUBJECT
#define M_FROM            "From"
#define M_TO              "To"
#define M_DATE            "Date"
#define M_MAILER          "Mailer"
#define M_ORGANIZATION    "Comp"
#define M_MESSAGEID       "MsgId"
#define M_FROM_NAME       "FName"
#define M_FROM_ADDR       "FAddr"
#define M_TO_NAME         "TName"
#define M_TO_ADDR         "TAddr"
#define M_SENT_DATE       "SDate"
#define M_SENT_TIME       "STime"
#define M_RECEIVED_DATE   "RDate"
#define M_RECEIVED_TIME   "RTime"
#define M_HEADERS         "Headers"
#define M_KLUDGE          "Kludge"
#define M_TEXT            "Text"
#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

extern void ProcessSubject( LPSTR pStr );

FarString ProcessSubject( const FarString& subj )
{
  FarString result = subj;
  ProcessSubject( result.GetBuffer() );
  result.ReleaseBuffer();
  return result;
}

long GetMsgEncoding(PMessage Msg, PMsgPart TextPart, long defaultEncoding)
{
  long encoding = Msg->GetEncoding();

  if ( encoding != FCT__INVALID )
    return encoding;

  if ( TextPart )
  {
    CMimeContent mc(TextPart->GetKludge(K_RFC_ContentType));

    LPCSTR charset = mc.getDataValue("charset");
    if (charset)
      encoding = getCharacterTableNoDefault(charset);
  }

  return encoding == FCT__INVALID ? defaultEncoding : encoding;
}

void CMessageTemplate::processMacro( LPCSTR str, int len, int fill, IWriteSink & f )
{
  if ( FarSF::LStricmp( str, M_SUBJECT ) == 0 )
    processSubject( len, fill, f );
  else if ( FarSF::LStricmp( str, M_FROM ) == 0 )
    processFrom( len, fill, f );
  else if ( FarSF::LStricmp( str, M_TO ) == 0 )
    processTo( len, fill, f );
  else if ( FarSF::LStricmp( str, M_DATE ) == 0 )
    processDate( len, fill, f );
  else if ( FarSF::LStricmp( str, M_ORGANIZATION ) == 0 )
    processOrganization( len, fill, f );
  else if ( FarSF::LStricmp( str, M_MAILER ) == 0 )
    processMailer( len, fill, f );
  else if ( FarSF::LStricmp( str, M_OSUBJECT ) == 0 )
    processOSubject( len, fill, f );
  else if ( FarSF::LStricmp( str, M_MESSAGEID ) == 0 )
    processMessageId( len, fill, f );
  else if ( FarSF::LStricmp( str, M_FROM_NAME ) == 0 )
    processFromName( len, fill, f );
  else if ( FarSF::LStricmp( str, M_FROM_ADDR ) == 0 )
    processFromAddr( len, fill, f );
  else if ( FarSF::LStricmp( str, M_TO_NAME ) == 0 )
    processToName( len, fill, f );
  else if ( FarSF::LStricmp( str, M_TO_ADDR ) == 0 )
    processToAddr( len, fill, f );
  else if ( FarSF::LStricmp( str, M_SENT_DATE ) == 0 )
    processSentDate( len, fill, f );
  else if ( FarSF::LStricmp( str, M_SENT_TIME ) == 0 )
    processSentTime( len, fill, f );
  else if ( FarSF::LStricmp( str, M_RECEIVED_DATE ) == 0 )
    processReceivedDate( len, fill, f );
  else if ( FarSF::LStricmp( str, M_RECEIVED_TIME ) == 0 )
    processReceivedTime( len, fill, f );
  else if ( FarSF::LStricmp( str, M_TEXT ) == 0 )
    processText( len, f );
  else if ( FarSF::LStricmp( str, M_HEADERS ) == 0 )
    processHeaders( f );
  else if ( FarSF::LStrnicmp( str, M_KLUDGE, strlen(M_KLUDGE) ) == 0 )
  {
    str += _countof(M_KLUDGE);

    while ( *str && ( isspace( (unsigned char)*str ) || *str == '(' ||
      *str == '\"' || *str == '\'' ) ) str ++;

    if ( *str )
    {
      LPCSTR end = str + 1;

      while ( *end && !isspace( (unsigned char)*end ) && *end != ')' &&
        *end != '\"' && *end != '\'' ) end ++;

      processKludge( FarString( str, end - str ), f );
    }
  }
}

void CMessageTemplate::writeFormatted( const FarString& str, int len, int fill, IWriteSink & f )
{
  if ( len > 0 )
  {
    f.write( str.c_str(), min( len, str.Length() ) );

    if ( fill != 0 )
    {
      for ( int i = str.Length(); i < len; i ++ )
        f.write( (const char*)&fill, 1 );
    }
  }
  else
    f.write( str.c_str(), str.Length() );

}

CMailViewTpl::CMailViewTpl( const FarFileName& TplFileName, PMessage Msg, long defaultEncoding )
  : CMessageTemplate()
  , m_origMsg( Msg )
  , m_origDel( false )
  , m_fileName( TplFileName )
  , m_defaultEncoding( defaultEncoding )
{
  far_assert( m_origMsg );
}

CMailViewTpl::CMailViewTpl( const FarFileName& TplFileName, LPCSTR MsgFileName, long defaultEncoding  )
  : CMessageTemplate()
  , m_origMsg( create CFarInetMessage( MsgFileName, FCT_DEFAULT, false ) )
  , m_origDel( true )
  , m_fileName( TplFileName )
  , m_defaultEncoding( defaultEncoding )
{
}

CMailViewTpl::~CMailViewTpl()
{
  if ( m_origDel )
    delete m_origMsg;
}

void CMailViewTpl::processSubject( int len, int fill, IWriteSink & f )
{
  writeFormatted( m_origMsg->GetSubject(), len, fill, f );
}

void CMailViewTpl::processFrom( int len, int fill, IWriteSink & f )
{
  writeFormatted( m_origMsg->GetFrom()->GetMailboxName(), len, fill, f );
}

void CMailViewTpl::processTo( int len, int fill, IWriteSink & f )
{
  writeFormatted( m_origMsg->GetTo()->GetMailboxName(), len, fill, f );
}

void CMailViewTpl::processDate( int len, int fill, IWriteSink & f )
{
  writeFormatted( m_origMsg->GetDate(), len, fill, f );
}

void CMailViewTpl::processOrganization( int len, int fill, IWriteSink & f )
{
  writeFormatted( m_origMsg->GetOrganization(), len, fill, f );
}

void CMailViewTpl::processMailer( int len, int fill, IWriteSink & f )
{
  writeFormatted( m_origMsg->GetMailer(), len, fill, f );
}

void CMailViewTpl::processOSubject( int len, int fill, IWriteSink & f )
{
  writeFormatted( ProcessSubject( m_origMsg->GetSubject() ), len, fill, f );
}

void CMailViewTpl::processMessageId( int len, int fill, IWriteSink & f )
{
  FarString s = m_origMsg->GetId();
  if ( s.Length() > 2 && s[ 0 ] == '<' )
  {
    s = s.Mid( 1 );
    int pos = s.IndexOf( '>' );
    if ( pos != -1 )
      s = s.Left( pos );
  }
  writeFormatted( s, len, fill, f );
}

void CMailViewTpl::processFromName( int len, int fill, IWriteSink & f )
{
  writeFormatted( m_origMsg->GetFrom()->Name, len, fill, f );
}

void CMailViewTpl::processFromAddr( int len, int fill, IWriteSink & f )
{
  writeFormatted( m_origMsg->GetFrom()->Addr, len, fill, f );
}

void CMailViewTpl::processToName( int len, int fill, IWriteSink & f )
{
  writeFormatted( m_origMsg->GetTo()->Name, len, fill, f );
}

void CMailViewTpl::processToAddr( int len, int fill, IWriteSink & f )
{
  writeFormatted( m_origMsg->GetTo()->Addr, len, fill, f );
}

void CMailViewTpl::processSentDate( int len, int fill, IWriteSink & f )
{
  FILETIME ft;
  m_origMsg->GetSent( &ft );
  writeFormatted( ((FileTime)ft).getDate(ViewCfg()->getDateFormat(), LCID_DateTime), len, fill, f );
}

void CMailViewTpl::processSentTime( int len, int fill, IWriteSink & f )
{
  FILETIME ft;
  m_origMsg->GetSent( &ft );
  writeFormatted( ((FileTime)ft).getTime(ViewCfg()->getTimeFormat(), LCID_DateTime), len, fill, f );
}

void CMailViewTpl::processReceivedDate( int len, int fill, IWriteSink & f )
{
  FILETIME ft;
  m_origMsg->GetReceived( &ft );
  writeFormatted( ((FileTime)ft).getDate(ViewCfg()->getDateFormat(), LCID_DateTime), len, fill, f );
}

void CMailViewTpl::processReceivedTime( int len, int fill, IWriteSink & f )
{
  FILETIME ft;
  m_origMsg->GetReceived( &ft );
  writeFormatted( ((FileTime)ft).getTime(ViewCfg()->getTimeFormat(), LCID_DateTime), len, fill, f );
}

void CMailViewTpl::processHeaders( IWriteSink & f )
{
  PKludges Kludges = m_origMsg->GetKludges();

  for ( int i = 0; i < Kludges->Count(); i ++ )
  {
    FarString kludge = Kludges->At( i );

    m_origMsg->DecodeKludge( kludge.GetBuffer() ); kludge.ReleaseBuffer();

    int pos = 1;

    while ( ( pos = kludge.IndexOf( '\t', pos ) ) != -1 )
    {
      FarString ss = kludge.Left( pos );
      if (ss.Length() > 0 && ss[ss.Length() - 1] == ':')
      {
        pos ++;
        continue;
      }
      f.write( ss.c_str(), ss.Length() );
      f.write( "\r\n", 2 );
      kludge = kludge.Right( kludge.Length() - pos );
    }

    f.write( kludge.c_str(), kludge.Length() );

    if ( i < Kludges->Count() - 1 )
      f.write( "\r\n", 2 );
  }
}

void CMailViewTpl::processKludge( LPCSTR kludge, IWriteSink & f )
{
  FarString s = m_origMsg->GetDecodedKludge( kludge );

  f.write( s.c_str(), s.Length() );
}

void CMailViewTpl::processText( int len, IWriteSink & f )
{
  // TODO: check File
  FarFileName bodyFile = FarFileName::MakeTemp( "mvb" );

  File::mkdirs( bodyFile.GetPath() );

  PMsgPart TextPart = m_origMsg->GetTextPart();
  if ( TextPart )
  {
    long Encoding = GetMsgEncoding( m_origMsg, TextPart, m_defaultEncoding );

    if ( len > 20 )
    {
      FarStringArray Src;
      StrPtr sp = { NULL, 0, (LPSTR)TextPart->GetContentData() };
      LPCSTR Last = sp.Nxt + TextPart->GetContentSize();
      while ( sp.Nxt )
      {
        GetStrPtr( sp.Nxt, &sp, Last );
        Src.Add( ToOEMString( FarString( sp.Str, sp.Len ), Encoding ) );
      }
      FarStringArray * Dst = WordWrapLines( Src, len, true );
      for ( int i = 0; i < Dst->Count(); i ++ )
      {
        f.write( Dst->At( i ), strlen( Dst->At( i ) ) );
        f.write( "\r\n", 2 );
      }
      delete Dst;
    }
    else
    {
      FarString s = (LPCSTR)TextPart->GetContentData();

      s.SetLength( TextPart->GetContentSize() );

      s = ToOEMString( s, Encoding );

      f.write( s.c_str(), s.Length() );
    }

    File::deleteit( bodyFile );
  }
}

#define IsMacro( str, macro ) ( macroLen = sizeof( macro )-1, FarSF::LStrnicmp( str, macro, sizeof( macro )-1 ) == 0 )

bool CMailViewTpl::SaveToFile( LPCSTR fileName )
{
  FarFile f;
  if ( !f.CreateForWrite( fileName ) )
    return false;

  class CWriteSink : public IWriteSink
  {
  private:
    FarFile * f;
  public:
    CWriteSink( FarFile * ff ) : f( ff )
    {
    }
    virtual ~CWriteSink()
    {
    }
    virtual void write( LPCSTR str, int len )
    {
      f->Write( str, len );
    }
  };

  CWriteSink ws( &f );
  processFile( m_fileName, &ws );

  f.Close();

  return true;
}

extern bool CreateDirectory( LPSTR FullPath );

bool CMailViewTpl::View( bool bEditor, bool bLock )
{
  FarFileName TempFile = FarFileName::MakeTemp( "mvt" );
  CreateDirectory( TempFile.GetBuffer() );
  TempFile.ReleaseBuffer();
  TempFile.AddEndSlash();
  TempFile.SetName( FarFileName::MakeTemp( "mvt" ) );
  TempFile.SetExt( ".msg" );

  if ( SaveToFile( TempFile ) == false )
    return false;

  FarString Subj = m_origMsg->GetSubject();
  if ( Subj.IsEmpty() )
    Subj = "<none>";

  SetFileAttributes( TempFile, GetFileAttributes( TempFile )|(bLock?FILE_ATTRIBUTE_READONLY:0) );

  if ( bEditor )
    Far::Editor( TempFile, Subj, 0, 0, -1, -1, 0, 1,
      EF_DELETEONCLOSE|EF_DISABLEHISTORY|EF_ENABLE_F6|EF_NONMODAL );
  else
    Far::Viewer( TempFile, Subj, 0, 0, -1, -1,
      VF_DELETEONCLOSE|VF_DISABLEHISTORY|VF_ENABLE_F6|VF_NONMODAL );

  return true;
}
