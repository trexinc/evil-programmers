/*
 OE mailbox access plugin for MailView
 Copyright (C) 2005 Alex Yaroslavsky
 Copyright (C) 2002-2003 Dennis Trachuk

 using OE5 DBX file scanner API (DBXOUT) by andrey_no <andrey_no@mail.primorye.ru>

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
#include <windows.h>
#include "../../MailViewPlugin.h"
#include "../CopyString.h"

#include "dbx_exp.h"

#pragma comment(lib, "DbxOut.lib" )

#pragma comment(linker, "/export:Mailbox_GetName=_GetName@8")
#pragma comment(linker, "/export:Mailbox_GetShortName=_GetName@8")
#pragma comment(linker, "/export:Mailbox_GetFileMasks=_GetFileMasks@8")

#pragma comment(linker, "/export:Mailbox_OpenFile=_OpenDbxFile@4")
#pragma comment(linker, "/export:Mailbox_Close=_Close@4")

#pragma comment(linker, "/export:Mailbox_GetNextMsg=_GetNextMsg@8")

#pragma comment(linker, "/export:Mailbox_GetMsgInfo=_GetMsgInfo@12")
#pragma comment(linker, "/export:Mailbox_GetMsg=_GetMsg@16")

#if defined(_MSC_VER) && !defined(_DEBUG)
//#pragma comment(linker, "-subsystem:console")
//#pragma comment(linker, "-merge:.rdata=.text")
//#pragma comment(linker, "-merge:.text=.data")
#pragma comment(linker, "/ignore:4078")
#pragma comment(linker, "/merge:.data=.")
#pragma comment(linker, "/merge:.rdata=.")
#pragma comment(linker, "/merge:.text=.")
#endif

//#define _STATIC_DBXOUT

#ifndef _STATIC_DBXOUT

#pragma comment(linker, "/entry:DllMain")

//////////////////////////////////////////////////////////////////////////

char * PointToName( const char * FileName )
{
  const char * p = FileName;
  while ( *FileName )
  {
    if ( *FileName == '\\' || *FileName == '/' || *FileName == ':' )
      p = FileName + 1;
    FileName ++;
  }
  return (char*)p;
}

HINSTANCE hMod = NULL;
BOOL APIENTRY DllMain( HANDLE hModule, DWORD ulReason, LPVOID lpReserved )
{
  switch ( ulReason )
  {
  case DLL_PROCESS_ATTACH:
    char * DbxOut_dll = new char[ _MAX_PATH ];
    *DbxOut_dll = 0;
    GetModuleFileNameA( (HINSTANCE)hModule, DbxOut_dll, _MAX_PATH );
    char * p = PointToName( DbxOut_dll );
    if ( p == NULL )
      p = DbxOut_dll;
    strcpy( p, "DbxOut.dll" );
    hMod = LoadLibraryA( DbxOut_dll );
    delete [] DbxOut_dll;
    break;

  case DLL_PROCESS_DETACH:
    if ( hMod )
      FreeLibrary( hMod );
    hMod = NULL;
    break;
  }
  return TRUE;
}

#endif //!defined(_STATIC_DBXOUT)
//////////////////////////////////////////////////////////////////////////

extern "C" BOOL WINAPI GetName( LPSTR Name, LPDWORD pSize )
{
  return CopyString( "Outlook Express", Name, pSize );
}

extern "C" BOOL WINAPI GetFileMasks( LPSTR Masks, LPDWORD pSize )
{
  return CopyString( "*.dbx", Masks, pSize );
}

//////////////////////////////////////////////////////////////////////////

typedef HDBX    (*TDBXOpenProc)( LPCTSTR  );
typedef DWORD   (*TDBXCloseProc)( HDBX );
typedef BOOL    (*TDBXIsMessageDatabaseProc)( HDBX );
typedef DWORD   (*TDBXGetMsgCountProc)( HDBX );
typedef DWORD   (*TDBXGetMsgByPosProc)( HDBX, DWORD, LPTSTR, LPDWORD );
typedef PDBX_LOCK_DATA  (*TDBXLockMsgHeaderByPosProc)( HDBX, DWORD );
typedef VOID    (*TDBXUnlockProc)( PDBX_LOCK_DATA );
typedef PFILETIME (*TDBXGetTimeProc)( DBX_HEADER_DATA* );

#ifndef _STATIC_DBXOUT
TDBXOpenProc               dynDBXOpen               = NULL;
TDBXCloseProc              dynDBXClose              = NULL;
TDBXIsMessageDatabaseProc  dynDBXIsMessageDatabase  = NULL;
TDBXGetMsgCountProc        dynDBXGetMsgCount        = NULL;
TDBXGetMsgByPosProc        dynDBXGetMsgByPos        = NULL;
TDBXLockMsgHeaderByPosProc dynDBXLockMsgHeaderByPos = NULL;
TDBXUnlockProc       dynDBXUnlock             = NULL;
TDBXGetTimeProc            dynDBXGetTimeRecieved    = NULL;
TDBXGetTimeProc            dynDBXGetTimeAccessed    = NULL;
TDBXGetTimeProc            dynDBXGetTimeSend        = NULL;
#else
TDBXOpenProc               dynDBXOpen               = DBXOpen;
TDBXCloseProc              dynDBXClose              = DBXClose;
TDBXIsMessageDatabaseProc  dynDBXIsMessageDatabase  = DBXIsMessageDatabase;
TDBXGetMsgCountProc        dynDBXGetMsgCount        = DBXGetMsgCount;
TDBXGetMsgByPosProc        dynDBXGetMsgByPos        = DBXGetMsgByPos;
TDBXLockMsgHeaderByPosProc dynDBXLockMsgHeaderByPos = DBXLockMsgHeaderByPos;
TDBXUnlockProc       dynDBXUnlock             = DBXUnlock;
TDBXGetTimeProc            dynDBXGetTimeRecieved    = DBXGetTimeRecieved;
TDBXGetTimeProc            dynDBXGetTimeAccessed    = DBXGetTimeAccessed;
TDBXGetTimeProc            dynDBXGetTimeSend        = DBXGetTimeSend;
#endif // _STATIC_DBXOUT
//////////////////////////////////////////////////////////////////////////

extern "C" HANDLE WINAPI OpenDbxFile( LPCSTR FileName )
{
#ifndef _STATIC_DBXOUT
  if ( hMod == NULL )
    return NULL;

  dynDBXOpen               = (TDBXOpenProc)GetProcAddress( hMod, "DBXOpen" );
  dynDBXClose              = (TDBXCloseProc)GetProcAddress( hMod, "DBXClose" );
  dynDBXIsMessageDatabase  = (TDBXIsMessageDatabaseProc)GetProcAddress( hMod, "DBXIsMessageDatabase" );
  dynDBXGetMsgCount        = (TDBXGetMsgCountProc)GetProcAddress( hMod, "DBXGetMsgCount" );
  dynDBXGetMsgByPos        = (TDBXGetMsgByPosProc)GetProcAddress( hMod, "DBXGetMsgByPos" );
  dynDBXLockMsgHeaderByPos = (TDBXLockMsgHeaderByPosProc)GetProcAddress( hMod, "DBXLockMsgHeaderByPos" );
  dynDBXUnlock             = (TDBXUnlockProc)GetProcAddress( hMod, "DBXUnlock" );
  dynDBXGetTimeRecieved    = (TDBXGetTimeProc)GetProcAddress( hMod, "DBXGetTimeRecieved" );
  dynDBXGetTimeAccessed    = (TDBXGetTimeProc)GetProcAddress( hMod, "DBXGetTimeAccessed" );
  dynDBXGetTimeSend        = (TDBXGetTimeProc)GetProcAddress( hMod, "DBXGetTimeSend" );


  if ( dynDBXOpen == NULL || dynDBXClose == NULL || dynDBXIsMessageDatabase == NULL ||
    dynDBXGetMsgCount == NULL || dynDBXGetMsgByPos == NULL ||
    dynDBXLockMsgHeaderByPos == NULL || dynDBXUnlock == NULL ||
    dynDBXGetTimeRecieved == NULL || dynDBXGetTimeAccessed == NULL ||
    dynDBXGetTimeSend == NULL )
    return NULL;
#endif //!defined(_STATIC_DBXOUT)

  HDBX hDBX = dynDBXOpen( FileName );
  if ( hDBX != (HDBX)DBXERROR && dynDBXIsMessageDatabase( hDBX ) )
    return (HANDLE)hDBX;

  return NULL;
}

extern "C" void WINAPI Close( HANDLE hMailbox )
{
  dynDBXClose( (HDBX)hMailbox );
}

extern "C" DWORD WINAPI GetNextMsg( HANDLE hMailbox, DWORD dwPrevID )
{
  if ( dwPrevID == BAD_MSG_ID )
    {
    if ( dynDBXGetMsgCount( (HDBX)hMailbox ) > 0 )
      return 0;
    }
  else if ( dynDBXGetMsgCount( (HDBX)hMailbox ) >= dwPrevID )
    return dwPrevID+1;

  return BAD_MSG_ID;
}

extern "C" BOOL WINAPI GetMsgInfo( HANDLE hMailbox, DWORD dwMsgID, TMsgInfo * pInfo )
{
  if ( dwMsgID == BAD_MSG_ID )
    return FALSE;

  if ( pInfo )
  {
    PDBX_LOCK_DATA pData = dynDBXLockMsgHeaderByPos( (HDBX)hMailbox, dwMsgID );
    memcpy( &pInfo->Received, dynDBXGetTimeRecieved( pData ), sizeof ( FILETIME ) );
    memcpy( &pInfo->Accessed, dynDBXGetTimeAccessed( pData ), sizeof ( FILETIME ) );
    memcpy( &pInfo->Sent, dynDBXGetTimeSend( pData ), sizeof ( FILETIME ) );
    dynDBXUnlock( pData );

    return TRUE;
  }

  return FALSE;
}

extern "C" BOOL WINAPI GetMsg( HANDLE hMailbox, DWORD dwMsgID, LPBYTE pMsg, LPDWORD pSize )
{
  if ( dwMsgID == BAD_MSG_ID )
    return FALSE;

  return dynDBXGetMsgByPos( (HDBX)hMailbox, dwMsgID, pMsg, pSize ) != DBXERROR ? TRUE : FALSE;
}
