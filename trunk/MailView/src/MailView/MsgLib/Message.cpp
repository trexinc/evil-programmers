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
#include <FarFile.h>
#include "MsgLib.h"

CMessageT::CMessageT() : m_Encoding( -1 ), m_defaultEncoding( -1 ), m_Part( NULL )
{
}

CMessageT::~CMessageT()
{
  FreeParts();
  if ( m_Data.data )
    delete [] m_Data.data;
}

void CMessageT::MakeParts()
{
  far_assert( m_Part != NULL );
  if ( !m_Part->m_DoneParts )
  {
    m_Part->MakeParts();
    m_Part->m_DoneParts = true;
  }
}

PMsgPart CMessageT::GetNextPart( PMsgPart Prev )
{
  MakeParts(); // !!! возможно надо оптимизнуть

  if ( Prev == NULL )
          return m_Part;

        return Prev->m_Next;
}

PKludges CMessageT::GetKludges()
{
  return m_Part->GetKludges();
}

LPCSTR CMessageT::GetKludge( LPCSTR Name )
{
  return m_Part->GetKludge( Name );
}

DWORD CMessageT::GetSize()
{
  return m_Data.size;
}

bool CMessageT::read( long wParam, long lParam, long encoding, bool headOnly )
{
  return false;
}

void CMessageT::AllocData( DWORD nSize )
{
  if ( nSize > m_Data.Capacity )
  {
    if ( m_Data.data )
      delete [] m_Data.data;
    // зарезервируем один байт. может понадобиться для неправильных
    // писем, которые не содержат завершающих CRLF
    m_Data.data = create BYTE[ nSize + 1 ];
    m_Data.Capacity = nSize + 1;
  }
}

bool CMessageT::read( LPCSTR FileName, long Encoding, bool headOnly )
{
  FreeParts();

  FarFile f;

  if ( !f.OpenForRead( FileName ) )
    return false;

  m_Data.size = f.GetSize();

  if ( m_Data.size == (DWORD)-1 )
  {
    m_Data.size = 0;
    return false;
  }

  AllocData( m_Data.size );

  m_Data.size = f.Read( m_Data.data, m_Data.size );

  return Init( Encoding );
}

void CMessageT::FreeParts()
{
  PMsgPart Part = m_Part;
  while ( Part )
  {
    PMsgPart Next = Part->m_Next;
    delete Part;
    Part = Next;
  }
  m_Part = NULL;
}

FarString CMessageT::GetDecodedKludge( LPCSTR Name )
{
  FarString Kludge = GetKludge( Name );

  if ( Kludge.Length() > 0 )
  {
    LPSTR Buf = Kludge.GetBuffer();
    DecodeKludge( Buf );
    while ( ( Buf = strchr( Buf, '\t' ) ) != NULL )
      *Buf = '\x20';
    Kludge.ReleaseBuffer();
  }

  return Kludge;
}

bool CMessageT::DecodeKludge( LPSTR Data )
{
  return true;
}

bool CMessageT::DecodeContent( PContent Data )
{
  return true;
}
