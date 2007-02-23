/*
 OE mailbox access plugin for MailView
 Copyright (C) 2005 Alex Yaroslavsky
 Copyright (C) 2002-2003 Dennis Trachuk

 using LibDBX by Dave Smith <dave.s@earthcorp.com>

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
#include "libdbx.h"

IMPLEMENT_INFORMATION( EMT_INET, "OE", "Outlook Express DBX 5.0 database", "*.dbx" )

class CDBXMailbox : public CMailbox
{
private:
  DBX      * hDBX;
  DBXEMAIL * pMsg;
  int        iMsg;
public:
  CDBXMailbox( DBX * dbx );
  virtual ~CDBXMailbox();

  virtual DWORD GetNextMsg( DWORD dwPrevID );
  virtual BOOL GetMsgInfo( DWORD dwMsgID, PMsgInfo lpInfo );
  virtual BOOL GetMsg( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize );
};

PMailbox CMailbox::Create( LPCVOID lpMem, DWORD dwSize )
{
  return NULL;
}

PMailbox CMailbox::Create( LPCSTR szFileName )
{
  DBX * dbx = dbx_open( szFileName );
  if ( dbx )
  {
    if ( dbx->type == DBX_TYPE_EMAIL || dbx->type == DBX_TYPE_NEWS )
      return new CDBXMailbox( dbx );

    dbx_close( dbx );
  }
  return NULL;
}

CDBXMailbox::CDBXMailbox( DBX * dbx ) : hDBX( dbx ), pMsg( NULL ), iMsg( -1 )
{
}

CDBXMailbox::~CDBXMailbox()
{
  dbx_free( hDBX, pMsg );
  dbx_close( hDBX );
}

DWORD CDBXMailbox::GetNextMsg( DWORD dwPrevID )
{
  if ( dwPrevID == BAD_MSG_ID )
  {
    if ( hDBX->indexCount > 0 )
      return 0;
  }
  else if ( hDBX->indexCount >= (int)dwPrevID )
    return dwPrevID + 1;

  return BAD_MSG_ID;
}

BOOL CDBXMailbox::GetMsgInfo( DWORD dwMsgID, PMsgInfo lpInfo )
{
  if ( dwMsgID == BAD_MSG_ID || lpInfo == NULL )
    return FALSE;

  lpInfo->StructSize = sizeof( TMsgInfo );

  if ( pMsg->flag && DBX_EMAIL_FLAG_ISSEEN )
    lpInfo->Flags |= EMF_READED;


  lpInfo->Received.dwLowDateTime  = pMsg->date.dwLowDateTime;
  lpInfo->Received.dwHighDateTime = pMsg->date.dwHighDateTime;

  return TRUE;
}

BOOL CDBXMailbox::GetMsg( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize )
{
  if ( dwMsgID == BAD_MSG_ID || lpSize == NULL )
    return FALSE;

  if ( iMsg != (int)dwMsgID)
  {
    dbx_free( hDBX, pMsg );

    iMsg = (int)dwMsgID;
    pMsg = (DBXEMAIL*)dbx_get( hDBX, iMsg, DBX_FLAG_BODY );
  }

  if ( pMsg == NULL )
  {
    iMsg = -1;
    return FALSE;
  }

  if ( pMsg->email == NULL )
    return FALSE;

  DWORD dwSize = strlen( pMsg->email ) + 1;

  if ( lpMsg == NULL )
  {
    *lpSize = dwSize;
    return TRUE;
  }

  if ( *lpSize > dwSize )
    *lpSize = dwSize;

  memcpy( lpMsg, pMsg->email, *lpSize );

  return TRUE;
}
