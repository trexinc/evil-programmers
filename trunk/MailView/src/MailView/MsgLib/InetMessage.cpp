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
#include "StdAfx.h"
#include "InetMessage.h"
#include "InetMsgPart.h"
#include "../Kludges.h"
#include "../DateTime.h"
#include "../Decoder.h"
#include "MimeContent.h"

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

CInetMessageT::CInetMessageT()
  : CMessageT()
  , m_From( NULL )
  , m_To( NULL )
  , m_Cc( NULL )
{
}

CInetMessageT::~CInetMessageT()
{
  if ( m_From )
    delete m_From;
  if ( m_To )
    delete m_To;
  if ( m_Cc )
    delete m_Cc;
}

bool CInetMessageT::Init( long Encoding )
{
  if ( m_From )
  {
    delete m_From;
    m_From = NULL;
  }
  if ( m_To )
  {
    delete m_To;
    m_To = NULL;
  }
  if ( m_Cc )
  {
    delete m_Cc;
    m_Cc = NULL;
  }
  memset( &m_Info, 0, sizeof( m_Info ) );
  m_Part = create CInetMsgPart( this, (LPSTR)m_Data.data, m_Data.size );
  return SetEncoding( Encoding );
}

PMsgPart CInetMessageT::GetTextPart()
{
  MakeParts();

  LPCSTR ContentType = GetKludge( K_RFC_ContentType );
  if ( FarSF::LStrnicmp( ContentType, "multipart", 9 ) == 0 )
  {
    PMsgPart TextPart = NULL;
    PMsgPart NextPart = m_Part;
    while ( NextPart = GetNextPart( NextPart ) )
    {
      ContentType = NextPart->GetKludge( K_RFC_ContentType );
      if ( FarSF::LStrnicmp( ContentType, "text", 4 ) == 0 )
      {
        if ( FarSF::LStrnicmp( ContentType + 5, "plain", 5 ) == 0 )
          return NextPart;
        TextPart = NextPart;
      }
    }
    if ( TextPart )
      return TextPart;
  }

  ContentType = GetKludge( K_RFC_ContentType );
  if ( *ContentType == '\0' || FarSF::LStrnicmp( ContentType, "text", 4 ) == 0 )
    return m_Part;

  return NULL;
}

void CInetMessageT::GetSent( LPFILETIME ft )
{
  if ( m_Info.Sent.dwHighDateTime == 0 && m_Info.Sent.dwLowDateTime == 0 )
    DateTimeUtils::MailDateToFileTime( GetKludge( K_RFC_Date ), m_Info.Sent );
  *(PINT64)ft = *(PINT64)&m_Info.Sent;
}

void CInetMessageT::GetReceived( LPFILETIME ft )
{
  if ( m_Info.Received.dwHighDateTime == 0 && m_Info.Received.dwLowDateTime == 0 )
  {
    LPCSTR p = GetKludge( "From " );
    if ( *p != '\0' )
    {
      while ( *p != '\0' && *p != '\x20' ) p ++; // address
      m_Info.Received = DateTimeUtils::MailDateToFileTime( p );
    }
    else
    {
      p = GetKludge( K_RFC_Received );
      if ( *p != '\0' && ( p = strchr( p, ';' ) ) )
        m_Info.Received = DateTimeUtils::MailDateToFileTime( p+1 );
    }
  }

  *(PINT64)ft = *(PINT64)&m_Info.Received;
}

void CInetMessageT::GetAccessed( LPFILETIME ft )
{
  if ( m_Info.Accessed.dwHighDateTime == 0 && m_Info.Accessed.dwLowDateTime == 0 )
    GetSystemTimeAsFileTime( &m_Info.Accessed );

  *(PINT64)ft = *(PINT64)&m_Info.Accessed;
}

void CInetMessageT::DecodeContent(PContent Content, LPCSTR TransferEncoding )
{
  CMimeDecoder * decoder = CMimeDecoder::Create( TransferEncoding );
  DWORD result = decoder->decode(Content->data, Content->size);
  delete decoder;
  if ( result != (DWORD)-1 )
    Content->size = result;
}

DWORD CInetMessageT::CalculateDecodedContentSize(PContent Content, LPCSTR TransferEncoding)
{
  CMimeDecoder * decoder = CMimeDecoder::Create(TransferEncoding);
  DWORD result = decoder->getSize(Content->data, Content->size);
  delete decoder;
  if ( result == (DWORD)-1 )
    result = 0;
  return result;
}

FarString CInetMessageT::GetSubject()
{
  return GetDecodedKludge( K_RFC_Subject );
}

PPerson CInetMessageT::GetFrom()
{
  if ( m_From == NULL )
  {
    FarString From = GetDecodedKludge( K_RFC_From );
    m_From = CPerson::Create( From );
    if ( m_From == NULL )
    {
      m_From = create CPerson;
      m_From->Name = From;
    }
  }

  return m_From;
}

PPerson CInetMessageT::GetTo()
{
  if ( m_To == NULL )
  {
    FarString To;
    if ( *(GetKludge( K_RFC_XFTNMsgId )) != '\0' )
    {
      if ( *(GetKludge( K_RFC_XCommentTo )) != '\0' )
        To = GetDecodedKludge( K_RFC_XCommentTo );
      else
        To = GetDecodedKludge( K_RFC_To );
    }
    else
      To = GetDecodedKludge( K_RFC_To );
    m_To = CPerson::Create( To );
    if ( m_To == NULL )
    {
      m_To = create CPerson;
      m_To->Name = To;
    }
  }
  return m_To;
}

PPerson CInetMessageT::GetCc()
{
  if ( m_Cc == NULL )
  {
    FarString cc = GetDecodedKludge( K_RFC_Cc );

    m_Cc = CPerson::Create( cc );
    if ( m_Cc == NULL )
    {
      m_Cc = create CPerson;
      m_Cc->Name = cc;
    }
  }

  return m_Cc;
}


FarString CInetMessageT::GetArea()
{
  return GetDecodedKludge( K_RFC_XFTNArea );
}

FarString CInetMessageT::GetNewsgroups()
{
  return GetDecodedKludge( K_RFC_Newsgroups );
}

DWORD CInetMessageT::GetPriority()
{
  if ( m_Info.Priority == EMP_AUTO )
  {
    LPCSTR XPriority = GetKludge( K_RFC_XPriority );
    switch( FarSF::atoi( XPriority ) )
    {
    case 5:
      m_Info.Priority = EMP_LOW;
      break;
    case 2:
      m_Info.Priority = EMP_HIGH;
      break;
    default:
      m_Info.Priority = EMP_NORMAL;
      break;
    }

  }
  return m_Info.Priority;
}

DWORD CInetMessageT::GetFlags()
{
  return m_Info.Flags;
}

bool CInetMessageT::DecodeKludge( LPSTR Data )
{
  return true;
}

DWORD CInetMessageT::GetAttchmentsCount()
{
  MakeParts();

  CMimeContent mc(GetKludge(K_RFC_ContentType));
  if (FarSF::LStricmp(mc.getType(), "multipart") != 0)
    return 0;
  if (FarSF::LStricmp(mc.getSubType(), "mixed") != 0 && FarSF::LStricmp(mc.getSubType(), "related") != 0)
    return 0;

  return 1;
}

FarString CInetMessageT::GetId()
{
  return GetKludge( K_RFC_MessageId );
}

FarString CInetMessageT::GetDate()
{
  return GetDecodedKludge( K_RFC_Date );
}

FarString CInetMessageT::GetMailer()
{
  return GetDecodedKludge( K_RFC_XMailer );
}

FarString CInetMessageT::GetOrganization()
{
  return GetDecodedKludge( K_RFC_Organization );
}

extern void ParseReferences( LPCSTR str, FarStringArray& result, bool in_reply_to );
void CInetMessageT::GetReferences( FarStringArray& strings )
{
  ParseReferences( GetKludge( K_RFC_InReplyTo ), strings, false );
  ParseReferences( GetKludge( K_RFC_References ), strings, false );
//  ParseReferences( GetKludge( K_XRusnewsReplyId ), strings, false );
}

bool CInetMessageT::SetEncoding( long Encoding )
{
  if ( GetEncoding() != Encoding )
  {
    if ( m_From )
    {
      delete m_From;
      m_From = NULL;
    }
    if ( m_To )
    {
      delete m_To;
      m_To = NULL;
    }
    if ( m_Cc )
    {
      delete m_Cc;
      m_Cc = NULL;
    }
  }
  return CMessageT::SetEncoding( Encoding );
}
