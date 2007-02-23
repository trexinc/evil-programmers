/*
 thebat! mailbox access plugin for MailView
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
#include "TheBat.h"
#include "crt.hpp"

// Number of 100 nanosecond units from 01.01.1601 to 01.01.1970
#define EPOCH_BIAS    116444736000000000ll

void UnixTimeToFileTime( long time, LPFILETIME ft )
{
  *(PINT64)ft = EPOCH_BIAS + time * 10000000ll;
}


IMPLEMENT_INFORMATION( EMT_INET, "TheBat!", "TheBat! message base", "" )

class CTBBMailbox : public CMailbox
{
private:
  LPBYTE m_Data;
  DWORD  m_Size;

  BOOL GetMsgInfo( TBBMsgHeader * pHdr, PMsgInfo lpInfo );

public:
  CTBBMailbox( LPBYTE lpMem, DWORD dwSize );
  virtual ~CTBBMailbox();

  virtual DWORD GetNextMsg( DWORD dwPrevID );
  virtual BOOL GetMsgInfo( DWORD dwMsgID, PMsgInfo lpInfo );
  virtual BOOL GetMsg( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize );
};

PMailbox CMailbox::Create( LPCVOID lpMem, DWORD dwSize )
{
  TBBFileHeader * hdr = (TBBFileHeader*)lpMem;
  if ( hdr->Signature == TBFH_SIGNATURE )
    return new CTBBMailbox( (LPBYTE)lpMem, dwSize );
  return NULL;
}

PMailbox CMailbox::Create( LPCSTR szFileName )
{
  return NULL;
}

CTBBMailbox::CTBBMailbox( LPBYTE lpMem, DWORD dwSize ) : m_Data( lpMem ), m_Size( dwSize )
{
}

CTBBMailbox::~CTBBMailbox()
{
}

DWORD CTBBMailbox::GetNextMsg( DWORD dwPrevID )
{
  LPBYTE pNxt;

  if ( dwPrevID == BAD_MSG_ID )
  {
    if ( m_Size < sizeof( TBBFileHeader ) + sizeof( TBBMsgHeader ) )
      return BAD_MSG_ID;
    pNxt = m_Data + sizeof( TBBFileHeader );
  }
  else
  {
    TBBMsgHeader * pHdr = (TBBMsgHeader*)(dwPrevID + m_Data);
    pNxt = (LPBYTE)pHdr + sizeof( TBBMsgHeader ) + pHdr->MsgSize;
  }

  if ( (pNxt - m_Data) < m_Size )
  {
                TBBMsgHeader * pHdr = (TBBMsgHeader*)pNxt;

                if ( pHdr->Signature != TBMH_SIGNATURE || pHdr->StructSize != sizeof(TBBMsgHeader) )
      return BAD_MSG_ID;

    if ( (pNxt - m_Data) + pHdr->MsgSize > m_Size )
      return BAD_MSG_ID;

    return (DWORD)(pNxt - m_Data);
  }

  return BAD_MSG_ID;
}

BOOL CTBBMailbox::GetMsgInfo( TBBMsgHeader * pHdr, PMsgInfo lpInfo )
{
  if ( lpInfo == NULL )
    return FALSE;

  lpInfo->StructSize = sizeof( TMsgInfo );

  if ( pHdr->Priority > 0 )
    lpInfo->Priority = EMP_HIGH;
  else if ( pHdr->Priority < 0 )
    lpInfo->Priority = EMP_LOW;
  else
    lpInfo->Priority = EMP_NORMAL;

  lpInfo->Flags = 0;

  if ( pHdr->Flags & TBF_DELETED )
    lpInfo->Flags |= EMF_DELETED;

  if ( pHdr->Flags & TBF_READED )
    lpInfo->Flags |= EMF_READED;

  if ( pHdr->Flags & TBF_FLAGGED )
    lpInfo->Flags |= EMF_FLAGGED;

  if ( pHdr->Flags & TBF_REPLIED )
    lpInfo->Flags |= EMF_REPLIED;

  if ( pHdr->Flags & TBF_FORWARDED )
    lpInfo->Flags |= EMF_FORWDED;

  UnixTimeToFileTime( pHdr->Received, &lpInfo->Received );
  return TRUE;
}

BOOL CTBBMailbox::GetMsgInfo( DWORD dwMsgID, PMsgInfo lpInfo )
{
  if ( dwMsgID == BAD_MSG_ID )
    return FALSE;

  return GetMsgInfo( (TBBMsgHeader*)(m_Data + dwMsgID), lpInfo );
}

BOOL CTBBMailbox::GetMsg( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize )
{
  if ( dwMsgID == BAD_MSG_ID || lpSize == NULL )
    return FALSE;

  TBBMsgHeader * pHdr = (TBBMsgHeader*)(m_Data + dwMsgID);

  if ( lpMsg == NULL )
  {
    *lpSize = pHdr->MsgSize;
    return TRUE;
  }

  if ( *lpSize > pHdr->MsgSize )
    *lpSize = pHdr->MsgSize;

  memcpy( lpMsg, (m_Data + dwMsgID) + sizeof( TBBMsgHeader ), *lpSize );

  return TRUE;
}
