/*
 unix-style mailbox access plugin for MailView
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
#include "../MailboxPlugin.h"
#include "crt.hpp"

#define MIN_MSG_SIZE 32

IMPLEMENT_INFORMATION( EMT_INET, "Unix", "Unix-style mailbox", "*.mbx,*.mbox" )

class CUnixMailbox : public CMailbox
{
private:
  LPCSTR m_Data;
  DWORD  m_Size;

  DWORD  m_CurID;
  DWORD  m_NxtID;
public:
  CUnixMailbox( LPCSTR szData, DWORD dwSize );
  virtual ~CUnixMailbox();

  virtual DWORD GetNextMsg( DWORD dwPrevID );
  virtual BOOL GetMsg( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize );
};

PMailbox CMailbox::Create( LPCVOID lpMem, DWORD dwSize )
{
  return dwSize > MIN_MSG_SIZE ? new CUnixMailbox( (LPCSTR)lpMem, dwSize ) : NULL;
}

PMailbox CMailbox::Create( LPCSTR szFileName/*, BOOL createNew*/ )
{
/*  if ( createNew == FALSE )
    return NULL;

  HANDLE hFile = CreateFile( szFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL );

  if ( hFile == INVALID_HANDLE_VALUE )
  {
    return FALSE;
  }

  CloseHandle( hFile );
*/
  return NULL;
}

CUnixMailbox::CUnixMailbox( LPCSTR szData, DWORD dwSize )
: m_Data ( szData ),
  m_Size ( dwSize ),
  m_CurID( BAD_MSG_ID ),
  m_NxtID( BAD_MSG_ID )
{
}

CUnixMailbox::~CUnixMailbox()
{
}

DWORD CUnixMailbox::GetNextMsg( DWORD dwPrevID )
{
  LPCSTR ptr = m_Data;
  DWORD  msz = m_Size - MIN_MSG_SIZE;

  if ( dwPrevID == BAD_MSG_ID )
  {
    while ( ( ptr - m_Data ) < msz && ( *ptr == '\x20' || *ptr == '\n' ) )
      ptr ++;
    if ( memcmp( ptr, "From ", 5 ) == 0 )
      return (DWORD)(ptr - m_Data);
    else
      return BAD_MSG_ID;
  }

  if ( m_CurID == dwPrevID )
    return m_NxtID;

  ptr += dwPrevID + 1;

  while ( ( ptr - m_Data ) < msz && ( ptr = (LPCSTR)memchr( ptr, '\n', msz - ( ptr - m_Data ) ) ) )
  {
    if ( memcmp( (++ptr), "From ", 5 ) == 0 )
      return (DWORD)(ptr - m_Data);
  }

  return BAD_MSG_ID;
}

LPSTR strnstr( LPCSTR str1, LPCSTR str2, LPCSTR eptr )
{
        if ( *str2 == '\0' )
    return (LPSTR)str1;

        LPCSTR s1, s2;

        while ( str1 < eptr )
        {
                s1 = str1;
                s2 = str2;

                while ( str1 < eptr && *s2 && !(*s1-*s2) )
                        s1++, s2++;

                if ( *s2 == '\0' )
                        return (LPSTR)str1;

                str1++;
        }

        return NULL;
}

BOOL CUnixMailbox::GetMsg( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize )
{
  if ( dwMsgID == BAD_MSG_ID || dwMsgID >= m_Size || lpSize == NULL )
    return false;

  if ( dwMsgID != m_CurID )
  {
    m_NxtID = GetNextMsg( dwMsgID );
    m_CurID = dwMsgID;
  }

  DWORD dwSize = m_NxtID != BAD_MSG_ID ? m_NxtID - m_CurID : m_Size - m_CurID;

  if (lpMsg == NULL)
  {
    *lpSize = dwSize;
    return true;
  }

  if ( *lpSize > dwSize )
    *lpSize = dwSize;

  if ( *lpSize == 0 )
    return true;

  memcpy( lpMsg, m_Data + m_CurID, *lpSize );

  LPSTR cPtr = (LPSTR)lpMsg;
  LPSTR ePtr = (LPSTR)lpMsg + *lpSize;
  while ( ( cPtr = strnstr( cPtr, "\n>From ", ePtr ) ) != NULL )
    cPtr ++, memmove( cPtr, cPtr + 1, ePtr - cPtr ), (*lpSize) --;

  return true;
}
