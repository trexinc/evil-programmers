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
#ifndef ___MailBox_H___
#define ___MailBox_H___

#include <FarFile.h>
#include "MailViewPlugin.h"

class CMailbox
{
private:
  typedef BOOL  (WINAPI *TGetStringProc)(LPSTR,LPDWORD);
  typedef HANDLE  (WINAPI *TOpenMemProc)(LPCVOID,DWORD);
  typedef HANDLE  (WINAPI *TOpenFileProc)(LPCSTR);
  typedef void  (WINAPI *TCloseProc)(HANDLE);
  typedef HANDLE  (WINAPI *TCreateProc)(LPCSTR);
  typedef DWORD (WINAPI *TGetMsgTypeProc)();
  typedef DWORD (WINAPI *TGetNextMsgProc)(HANDLE,DWORD);
  typedef BOOL  (WINAPI *TGetMsgInfoProc)(HANDLE,DWORD,PMsgInfo);
  typedef BOOL  (WINAPI *TSetMsgInfoProc)(HANDLE,DWORD,const PMsgInfo);
  typedef BOOL  (WINAPI *TGetMsgProc)(HANDLE,DWORD,PBYTE,LPDWORD);
  typedef MV_RESULT (WINAPI *TGetMsgHeadProc)(HANDLE,DWORD,PBYTE,LPDWORD);
  typedef DWORD (WINAPI *TAddMsgProc)(HANDLE,PBYTE,const DWORD);
  typedef BOOL  (WINAPI *TDelMsgProc)(HANDLE,DWORD);
  typedef MV_RESULT (WINAPI *TPurgeProc)(HANDLE);

  TGetMsgTypeProc m_GetMsgType;

  TGetStringProc  m_GetPlugName;
  TGetStringProc  m_GetShortName;

  TOpenMemProc    m_OpenMem;
  TOpenFileProc   m_Open;
  TCloseProc      m_Close;

  TGetNextMsgProc m_GetNextMsg;

  TGetMsgHeadProc m_GetMsgHead;

  TGetMsgInfoProc m_GetMsgInfo;
  TSetMsgInfoProc m_SetMsgInfo;
  TGetMsgProc     m_GetMsg;
  TAddMsgProc     m_AddMsg;

  TCreateProc     m_Create;

  TDelMsgProc     m_DelMsg;

  TPurgeProc      m_Purge;

  HANDLE                m_hMailbox;
  FarMemoryMappedFile * m_pFile;
  FarFileName           m_fileName;
  LPSTR                 m_shortName;
  HMODULE               m_pluginDll;

public:
  CMailbox( HMODULE hPluginDLL );

  ~CMailbox();

  bool open( const FarFileName& fileName, bool createNew = false );

  void close();

  HMODULE getModule() const
  {
    return m_pluginDll;
  }

  DWORD getNextMsg( DWORD dwPrevID )
  {
    return m_GetNextMsg ? m_GetNextMsg( m_hMailbox, dwPrevID ) : BAD_MSG_ID;
  }

  MV_RESULT getMsgHead( DWORD msgId, PBYTE msgHead, LPDWORD size )
  {
    MV_RESULT res = m_GetMsgHead ? m_GetMsgHead( m_hMailbox, msgId, msgHead, size ) : MV_NOTIMPL;
    return res == MV_NOTIMPL ? getMsg( msgId, msgHead, size ) : res;
  }

  MV_RESULT getMsg(DWORD dwMsgID, PBYTE pMsg, LPDWORD Size)
  {
    BOOL res = m_GetMsg ? m_GetMsg( m_hMailbox, dwMsgID, pMsg, Size ) : FALSE;
    return res == FALSE ? MV_FAIL : MV_OK;
  }

  BOOL getMsgInfo( DWORD dwMsgID, PMsgInfo pInfo )
  {
    memset( pInfo, 0, sizeof( TMsgInfo ) );
    return m_GetMsgInfo ? m_GetMsgInfo( m_hMailbox, dwMsgID, pInfo ) : FALSE;
  }

  BOOL setMsgInfo( DWORD dwMsgID, const PMsgInfo pInfo )
  {
    far_assert( pInfo != NULL );
    return m_SetMsgInfo ? m_SetMsgInfo( m_hMailbox, dwMsgID, pInfo ) : FALSE;
  }

  DWORD addMsg(PBYTE pMsg, const DWORD Size)
  {
    return checkFeature( featureAddMsg ) ? m_AddMsg( m_hMailbox, pMsg, Size ) : BAD_MSG_ID;
  }

  MV_RESULT purge()
  {
    return checkFeature( featurePurge ) ? m_Purge( m_hMailbox ) : MV_NOTIMPL;
  }

  DWORD delMsg( DWORD dwMsgId )
  {
    return !m_pFile && m_DelMsg ? m_DelMsg( m_hMailbox, dwMsgId ) : FALSE;
  }

  enum Feature { featureAddMsg, featureDelMsg, featurePurge };

  bool checkFeature( Feature feature )
  {
    switch ( feature )
    {
    case featureAddMsg:
      return !m_pFile && m_AddMsg;
    case featureDelMsg:
      return !m_pFile && m_DelMsg;
    case featurePurge:
      return !m_pFile && m_Purge;
    }
    return false;
  }

  DWORD getMsgType()
  {
    return m_GetMsgType ? m_GetMsgType() : /*EMT_AUTO*/0;
  }

  LPCSTR getShortName();

  FarFileName getFileName() const
  {
    return m_fileName;
  }

  bool isClosed() const
  {
    return m_hMailbox == NULL;
  }
  bool isOpened() const
  {
    return m_hMailbox != NULL;
  }

private:
  static FarString getPlugString( HMODULE hLib, LPCSTR ProcAddr )
  {
    FarString result;
    CMailbox::TGetStringProc func = (CMailbox::TGetStringProc)GetProcAddress( hLib, ProcAddr );
    if ( func )
    {
      DWORD size = 0;
      if ( func( NULL, &size ) )
      {
        if ( func( result.GetBuffer( size - 1 ), &size ) )
          result.ReleaseBuffer( size - 1 );
        else
          return FarString();
      }
    }
    return result;
  }

public:

  static FarString getLongName( HMODULE hLib )
  {
    return getPlugString( hLib, "Mailbox_GetName" );
  }
  static FarString getShortName( HMODULE hLib )
  {
    return getPlugString( hLib, "Mailbox_GetShortName" );
  }
  static FarString getFilesMasks( HMODULE hLib )
  {
    FarString result = getPlugString( hLib, "Mailbox_GetFilesMasks" );
    if ( result.IsEmpty() )
      result = "*.*";
    return result;
  }
};

#endif //!defined(___MailBox_H___)
