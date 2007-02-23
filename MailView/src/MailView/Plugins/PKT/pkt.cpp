/*
 pkt access plugin for MailView
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
#include "pkt.h"
#include "crt.hpp"

IMPLEMENT_INFORMATION( EMT_FIDO, "pkt", "Fido Packet base", "*.pkt" )

class CPKTMailbox : public CMailbox
{
private:
  LPBYTE m_Data;
  DWORD  m_Size;


  struct { DWORD ID, FromSize, ToSize, SubjectSize, BodySize; } m_CurMsg;

  DWORD _GetNextMsg( DWORD dwPrevID );
public:
  CPKTMailbox( LPBYTE szData, DWORD dwSize );
  virtual ~CPKTMailbox();

  virtual DWORD GetNextMsg( DWORD dwPrevID );
  virtual BOOL GetMsg( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize );
};

PMailbox CMailbox::Create( LPCVOID lpMem, DWORD dwSize )
{
  if ( dwSize < sizeof( TPacketFileHeader ) )
    return NULL;

  PPacketFileHeader hdr = (PPacketFileHeader)lpMem;

  return new CPKTMailbox( (LPBYTE)lpMem, dwSize );
}

PMailbox CMailbox::Create( LPCSTR szFileName )
{
  return NULL;
}

CPKTMailbox::CPKTMailbox( LPBYTE lpData, DWORD dwSize ) : m_Data( lpData ), m_Size( dwSize )
{
  m_CurMsg.ID = BAD_MSG_ID;
}

CPKTMailbox::~CPKTMailbox()
{
}

DWORD CPKTMailbox::_GetNextMsg( DWORD dwPrevID )
{
  if ( dwPrevID == m_CurMsg.ID )
    return m_CurMsg.ID +
      sizeof( TPacketMessageHeader ) +
      m_CurMsg.BodySize +
      m_CurMsg.SubjectSize +
      m_CurMsg.FromSize +
      m_CurMsg.ToSize;

  LPBYTE lpData, lpFind, lpLast = m_Data + m_Size;

  lpData = m_Data + dwPrevID + sizeof( TPacketMessageHeader );
  lpFind = (LPBYTE)memchr( lpData, 0, lpLast - lpData );
  if ( lpFind == NULL ) // where is fucking "to" field
    return BAD_MSG_ID;

  m_CurMsg.ToSize = lpFind - lpData + 1;

  lpData = lpFind + 1;
  lpFind = (LPBYTE)memchr( lpData, 0, lpLast - lpData );
  if ( lpFind == NULL ) // where is fucking "from" field
    return BAD_MSG_ID;

  m_CurMsg.FromSize = lpFind - lpData + 1;

  lpData = lpFind + 1;
  lpFind = (LPBYTE)memchr( lpData, 0, lpLast - lpData );
  if ( lpFind == NULL ) // where is fucking "subject" field
    return BAD_MSG_ID;

  m_CurMsg.SubjectSize = lpFind - lpData + 1;

  lpData = lpFind + 1;
  lpFind = (LPBYTE)memchr( lpData, 0, lpLast - lpData );
  if ( lpFind == NULL ) // maby eof?
    return BAD_MSG_ID;

  m_CurMsg.BodySize = lpFind - lpData + 1;

  lpData = lpFind + 1;

  m_CurMsg.ID = dwPrevID;

  return lpData - m_Data;
}

DWORD CPKTMailbox::GetNextMsg( DWORD dwPrevID )
{
  DWORD dwNextID = dwPrevID == BAD_MSG_ID ? sizeof( TPacketFileHeader ) : _GetNextMsg( dwPrevID );

  if ( dwNextID >= m_Size || *(m_Data + dwNextID) == '\0' )
    return BAD_MSG_ID;

  return dwNextID;
}

BOOL CPKTMailbox::GetMsg( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize )
{
  if ( dwMsgID == BAD_MSG_ID || dwMsgID >= m_Size || lpSize == NULL )
    return FALSE;

  if ( m_CurMsg.ID != dwMsgID )
    _GetNextMsg( dwMsgID );

  if ( m_CurMsg.ID != dwMsgID )
    return FALSE;

  if ( lpMsg == NULL )
  {
    *lpSize = sizeof( TFTNMessageHeader ) + m_CurMsg.BodySize;
    return TRUE;
  }

  if ( *lpSize < sizeof( TFTNMessageHeader ) )
    return FALSE;

  if ( *lpSize > sizeof( TFTNMessageHeader ) + m_CurMsg.BodySize )
    *lpSize = sizeof( TFTNMessageHeader ) + m_CurMsg.BodySize;

  PPacketMessageHeader srcMsg = (PPacketMessageHeader)(m_Data + m_CurMsg.ID);
  PFTNMessageHeader    dstMsg = (PFTNMessageHeader)lpMsg;

  dstMsg->Signature  = FMH_SIGNATURE;
  dstMsg->StructSize = sizeof( TFTNMessageHeader );

  LPBYTE lpEnd = (LPBYTE)srcMsg + sizeof( TPacketMessageHeader );
  memcpy( dstMsg->To     , lpEnd , m_CurMsg.ToSize );
  memcpy( dstMsg->From   , lpEnd + m_CurMsg.ToSize , m_CurMsg.FromSize );
  memcpy( dstMsg->Subject, lpEnd + m_CurMsg.ToSize + m_CurMsg.FromSize, m_CurMsg.SubjectSize );

  dstMsg->AddrFrom.Zone  = BAD_FTNADDR_FIELD;
  dstMsg->AddrFrom.Net   = BAD_FTNADDR_FIELD;
  dstMsg->AddrFrom.Node  = BAD_FTNADDR_FIELD;
  dstMsg->AddrFrom.Point = BAD_FTNADDR_FIELD;

  dstMsg->AddrTo.Zone  = BAD_FTNADDR_FIELD;
  dstMsg->AddrTo.Net   = BAD_FTNADDR_FIELD;
  dstMsg->AddrTo.Node  = BAD_FTNADDR_FIELD;
  dstMsg->AddrTo.Point = BAD_FTNADDR_FIELD;

  dstMsg->MessageId = 0;
  dstMsg->InReplyTo = 0;

  memcpy( lpMsg + sizeof( TFTNMessageHeader ),
    (LPBYTE)srcMsg + sizeof( TPacketMessageHeader ) +
    m_CurMsg.SubjectSize + m_CurMsg.FromSize + m_CurMsg.ToSize,
    *lpSize - sizeof( TFTNMessageHeader ) );

  return TRUE;
}
