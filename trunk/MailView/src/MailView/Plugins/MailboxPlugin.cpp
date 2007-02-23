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
#include "MailboxPlugin.h"
#include "crt.hpp"

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}
#endif

#if defined(_MSC_VER)
#pragma comment(linker, "/export:Mailbox_GetName=_Mailbox_GetName@8")
#pragma comment(linker, "/export:Mailbox_GetShortName=_Mailbox_GetShortName@8")
#pragma comment(linker, "/export:Mailbox_GetFilesMasks=_Mailbox_GetFilesMasks@8")
#pragma comment(linker, "/export:Mailbox_GetMsgType=_Mailbox_GetMsgType@0")

#pragma comment(linker, "/export:Mailbox_OpenFile=_Mailbox_OpenFile@4")
#pragma comment(linker, "/export:Mailbox_OpenMem=_Mailbox_OpenMem@8")
#pragma comment(linker, "/export:Mailbox_Close=_Mailbox_Close@4")

#pragma comment(linker, "/export:Mailbox_GetNextMsg=_Mailbox_GetNextMsg@8")

#pragma comment(linker, "/export:Mailbox_GetMsgInfo=_Mailbox_GetMsgInfo@12")
#pragma comment(linker, "/export:Mailbox_SetMsgInfo=_Mailbox_SetMsgInfo@12")

#pragma comment(linker, "/export:Mailbox_GetMsgHead=_Mailbox_GetMsgHead@16")
#pragma comment(linker, "/export:Mailbox_GetMsg=_Mailbox_GetMsg@16")

#pragma comment(linker, "/export:Mailbox_DelMsg=_Mailbox_DelMsg@8")

#pragma comment(linker, "/export:Mailbox_Purge=_Mailbox_Purge@4")

//#pragma comment(linker, "/export:Mailbox_AddMsg=_Mailbox_AddMsg@12")

#if !defined(_DEBUG)
//#pragma comment(linker, "-subsystem:console")
//#pragma comment(linker, "-merge:.rdata=.text")
//#pragma comment(linker, "-merge:.text=.data")
#if _MSC_VER < 7
#pragma comment(linker, "/ignore:4078")
#endif
#pragma comment(linker, "/merge:.data=.")
#pragma comment(linker, "/merge:.rdata=.")
#pragma comment(linker, "/merge:.text=.")
#endif
#endif

BOOL CMailbox::CopyString( LPCSTR szSrc, LPSTR szDst, LPDWORD lpSize )
{
  if ( lpSize == NULL )
    return FALSE;

  DWORD nLen = lstrlen( szSrc );

  if ( szDst == NULL )
  {
    *lpSize = nLen + 1;
    return TRUE;
  }

  if ( nLen > *lpSize )
    nLen = *lpSize;

  memcpy( szDst, szSrc, nLen );

  szDst[ nLen++ ] = '\0';

  *lpSize = nLen;

  return TRUE;
}

extern "C" BOOL WINAPI Mailbox_GetName( LPSTR szName, LPDWORD lpSize )
{
  return CMailbox::GetName( szName, lpSize );
}

extern "C" BOOL WINAPI Mailbox_GetShortName( LPSTR szName, LPDWORD lpSize )
{
  return CMailbox::GetShortName( szName, lpSize );
}

extern "C" BOOL WINAPI Mailbox_GetFilesMasks( LPSTR szMasks, LPDWORD lpSize )
{
  return CMailbox::GetFilesMasks( szMasks, lpSize );
}

extern "C" DWORD WINAPI Mailbox_GetMsgType()
{
  return CMailbox::GetMsgType();
}

extern "C" HANDLE WINAPI Mailbox_OpenFile( LPCSTR szFileName )
{
  return (HANDLE) CMailbox::Create( szFileName );
}

extern "C" HANDLE WINAPI Mailbox_OpenMem( LPCVOID lpMem, DWORD dwSize )
{
  return (HANDLE) CMailbox::Create( lpMem, dwSize );
}

extern "C" void WINAPI Mailbox_Close( HANDLE hMailbox )
{
  if ( hMailbox != NULL )
    delete (PMailbox) hMailbox;
}

extern "C" DWORD WINAPI Mailbox_GetNextMsg( HANDLE hMailbox, DWORD dwPrevID )
{
  PMailbox mbox = (PMailbox) hMailbox;

  if ( mbox == NULL )
    return BAD_MSG_ID;

  return mbox->GetNextMsg( dwPrevID );
}

extern "C" BOOL WINAPI Mailbox_GetMsgInfo( HANDLE hMailbox, DWORD dwMsgID, PMsgInfo lpInfo )
{
  PMailbox mbox = (PMailbox) hMailbox;

  if ( mbox == NULL )
    return BAD_MSG_ID;

  return mbox->GetMsgInfo( dwMsgID, lpInfo );
}

extern "C" BOOL WINAPI Mailbox_SetMsgInfo( HANDLE hMailbox, DWORD dwMsgID, PMsgInfo lpInfo )
{
  PMailbox mbox = (PMailbox) hMailbox;

  if ( mbox == NULL )
    return BAD_MSG_ID;

  return mbox->SetMsgInfo( dwMsgID, lpInfo );
}

extern "C" MV_RESULT WINAPI Mailbox_GetMsgHead( HANDLE hMailbox, DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize )
{
  PMailbox mbox = (PMailbox) hMailbox;

  if ( mbox == NULL )
    return MV_INVALIDARG;

  return mbox->GetMsgHead( dwMsgID, lpMsg, lpSize );
}

extern "C" BOOL WINAPI Mailbox_GetMsg( HANDLE hMailbox, DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize )
{
  PMailbox mbox = (PMailbox) hMailbox;

  if ( mbox == NULL )
    return FALSE;

  return mbox->GetMsg( dwMsgID, lpMsg, lpSize );
}

extern "C" BOOL WINAPI Mailbox_DelMsg( HANDLE hMailbox, DWORD dwMsgID )
{
  PMailbox mbox = (PMailbox) hMailbox;

  if ( mbox == NULL )
    return FALSE;

  return mbox->DelMsg( dwMsgID );
}

extern "C" MV_RESULT WINAPI Mailbox_Purge( HANDLE hMailbox )
{
  PMailbox mbox = (PMailbox) hMailbox;

  if ( mbox == NULL )
    return MV_INVALIDARG;

  return mbox->Purge();
}
