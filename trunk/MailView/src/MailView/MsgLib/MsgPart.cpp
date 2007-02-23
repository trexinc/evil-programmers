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
#include "MsgPart.h"
#include "../Kludges.h"

extern LPCSTR STR_EmptyStr;

CMsgPartT::CMsgPartT( PMessage Message )
: m_Decoded( false ),
  m_DoneParts( false ),
  m_Kludges( create FarStringArray( false ) ),
  m_Content( create TMemoryBlock ),
  m_Message( Message ),
  m_Next( NULL )
{
}

CMsgPartT::~CMsgPartT()
{
  delete m_Kludges;
  delete m_Content;
}

PMsgPart CMsgPartT::MakeParts()
{
  return this;
}

PContent CMsgPartT::GetDecodedContent()
{
  if ( m_Decoded == false )
  {
    m_Message->DecodeContent( m_Content, GetKludge( K_RFC_ContentTransferEncoding ) );

    m_Decoded = true;
  }

  return m_Content;
}

PVOID CMsgPartT::GetContentData()
{
  return GetDecodedContent()->data;
}

DWORD CMsgPartT::GetContentSize()
{
  if ( m_Decoded == false )
    return m_Message->CalculateDecodedContentSize( m_Content, GetKludge( K_RFC_ContentTransferEncoding ) );
  return GetDecodedContent()->size;
}

LPCSTR CMsgPartT::GetKludge( LPCSTR Name )
{
  int Len = strlen( Name );
  int Cnt = m_Kludges->Count();

  if ( Name[ Len - 1 ] != '\x20' )
  {
    for ( int i = 0; i < Cnt; i ++ )
    {
      LPCSTR Ptr = m_Kludges->At( i );
      if ( FarSF::LStrnicmp( Name, Ptr, Len ) == 0 )
      {
        Ptr += Len;
        if ( *Ptr == ':' )
        {
          Ptr ++;
          /*if ( *Ptr == '\x20' || *Ptr == '\t' )
            Ptr ++;*/
          while ( *Ptr && /*isspace( (unsigned char)*Ptr )*/*Ptr == '\x20' || *Ptr == '\t' )
            Ptr ++;
          return Ptr;
        }
      }
    }
  }
  else
  {
    for ( int i = 0; i < Cnt; i ++ )
    {
      LPCSTR Ptr = m_Kludges->At( i );
      if ( FarSF::LStrnicmp( Name, Ptr, Len ) == 0 )
      {
        return Ptr+Len;
      }
    }
  }

  return STR_EmptyStr;
}
