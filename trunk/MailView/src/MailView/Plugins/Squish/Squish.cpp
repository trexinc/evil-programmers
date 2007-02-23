/*
 squish mailbox access plugin for MailView
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
#include "Squish.h"
#include "crt.hpp"

IMPLEMENT_INFORMATION( EMT_FIDO, "Squish", "Squish message base", "*.sqd" )

class CSquishMailbox : public CMailbox
{
private:
  LPBYTE m_Data;
  DWORD  m_Size;

public:
  CSquishMailbox( LPBYTE szData, DWORD dwSize );
  virtual ~CSquishMailbox();

  virtual DWORD GetNextMsg( DWORD dwPrevID );
  virtual BOOL GetMsgInfo( DWORD dwMsgID, PMsgInfo lpInfo );
  virtual BOOL GetMsg( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize );
};

PMailbox CMailbox::Create( LPCVOID lpMem, DWORD dwSize )
{
  if ( dwSize < sizeof( TSquishFileHeader ) )
    return NULL;

  PSquishFileHeader hdr = (PSquishFileHeader)lpMem;
  if ( hdr->StructSize != sizeof( TSquishFileHeader ) )
    return NULL;

  return new CSquishMailbox( (LPBYTE)lpMem, dwSize );
}

PMailbox CMailbox::Create( LPCSTR szFileName )
{
  return NULL;
}

CSquishMailbox::CSquishMailbox( LPBYTE lpData, DWORD dwSize ) : m_Data( lpData ), m_Size( dwSize )
{
}

CSquishMailbox::~CSquishMailbox()
{
}

DWORD CSquishMailbox::GetNextMsg( DWORD dwPrevID )
{
  PSquishFileHeader hdr = (PSquishFileHeader)m_Data;

  DWORD nxt = dwPrevID == BAD_MSG_ID ? nxt = hdr->First :
    ((PSquishFrameHeader)(m_Data + dwPrevID))->Next;

  if ( nxt >= m_Size )
    return BAD_MSG_ID;

  PSquishFrameHeader frm = (PSquishFrameHeader)(m_Data + nxt);

  return frm->Signature == SFH_SIGNATURE ? nxt : BAD_MSG_ID;
}

BOOL CSquishMailbox::GetMsgInfo( DWORD dwMsgID, PMsgInfo lpInfo )
{
  if ( dwMsgID == BAD_MSG_ID || lpInfo == NULL )
    return FALSE;

  lpInfo->StructSize = sizeof( TMsgInfo );

  PSquishMessageHeader hdr = (PSquishMessageHeader)(m_Data + dwMsgID + sizeof( TSquishFrameHeader ));

  hdr->Written.ToFileTime( &lpInfo->Sent );
  hdr->Arrived.ToFileTime( &lpInfo->Received );

  return FALSE;
}

BOOL CSquishMailbox::GetMsg( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize )
{
  if ( dwMsgID == BAD_MSG_ID || dwMsgID >= m_Size || lpSize == NULL )
    return FALSE;

  PSquishFrameHeader frmHdr = (PSquishFrameHeader)(m_Data + dwMsgID );

  if ( lpMsg == NULL )
  {
    *lpSize = sizeof( TFTNMessageHeader ) + frmHdr->MsgLength - sizeof( TSquishMessageHeader );
    return TRUE;
  }

  if ( *lpSize < sizeof( TFTNMessageHeader ) )
    return FALSE;

  if ( *lpSize > sizeof( TFTNMessageHeader ) + frmHdr->MsgLength - sizeof( TSquishMessageHeader ) )
    *lpSize = sizeof( TFTNMessageHeader ) + frmHdr->MsgLength - sizeof( TSquishMessageHeader );

  PSquishMessageHeader srcMsg = (PSquishMessageHeader)((LPBYTE)frmHdr + sizeof( TSquishFrameHeader ));
  PFTNMessageHeader    dstMsg = (PFTNMessageHeader)lpMsg;

  dstMsg->Signature  = FMH_SIGNATURE;
  dstMsg->StructSize = sizeof( TFTNMessageHeader );

  memcpy( dstMsg->From, srcMsg->From, sizeof( TFTNName ) );
  memcpy( dstMsg->To, srcMsg->To, sizeof( TFTNName ) );
  memcpy( dstMsg->Subject, srcMsg->Subject, sizeof( TFTNSubj ) );
  memcpy( &dstMsg->AddrFrom, &srcMsg->AddrFrom, sizeof( TFTNAddr ) );
  memcpy( &dstMsg->AddrTo, &srcMsg->AddrTo, sizeof( TFTNAddr ) );

  dstMsg->MessageId = srcMsg->MessageId;
  dstMsg->InReplyTo = srcMsg->ReplyTo;

  memcpy( lpMsg + sizeof( TFTNMessageHeader ),
    (LPBYTE)srcMsg + sizeof( TSquishMessageHeader ),
    *lpSize - sizeof( TFTNMessageHeader ) );

  return TRUE;
}
