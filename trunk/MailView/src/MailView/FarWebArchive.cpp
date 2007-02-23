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
// WebArchive.cpp: implementation of the CWebArchive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FarWebArchive.h"
#include "LangId.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFarWebArchive::CFarWebArchive( LPCSTR FileName, long Encoding, bool headOnly )
: CFarInetMessage()
{
  read( FileName, Encoding, headOnly );
}

LPCSTR CFarWebArchive::GetFmtName()
{
  return Far::GetMsg( MWebArchive );
}

bool CFarWebArchive::read( LPCSTR FileName, long Encoding, bool headOnly )
{
  HANDLE hFile = CreateFile( FileName, 0, FILE_SHARE_READ|FILE_SHARE_WRITE,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if ( hFile != INVALID_HANDLE_VALUE )
  {
    GetFileTime( hFile, &m_CreationTime, &m_LastAccessTime, &m_LastWriteTime );
    CloseHandle( hFile );
  }
  return CInetMessageT::read( FileName, Encoding, headOnly );
}

bool CFarWebArchive::read( long wParam, long lParam, long Encoding, bool headOnly )
{
  m_CreationTime.dwHighDateTime   =
  m_CreationTime.dwLowDateTime    =
  m_LastAccessTime.dwHighDateTime =
  m_LastAccessTime.dwLowDateTime  =
  m_LastWriteTime.dwHighDateTime  =
  m_LastWriteTime.dwLowDateTime   = 0;
  return CInetMessageT::read( wParam, lParam, Encoding, headOnly );
}

void CFarWebArchive::GetSent( LPFILETIME ft )
{
  *(PINT64)ft = *(PINT64)&m_CreationTime;
}

void CFarWebArchive::GetReceived( LPFILETIME ft )
{
  *(PINT64)ft = *(PINT64)&m_LastWriteTime;
}

void CFarWebArchive::GetAccessed( LPFILETIME ft )
{
  *(PINT64)ft = *(PINT64)&m_LastAccessTime;
}
