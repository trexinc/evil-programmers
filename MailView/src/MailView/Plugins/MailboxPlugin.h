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
#ifndef ___MVPlugin_H___
#define ___MVPlugin_H___

#include <windows.h>
#include "../MailViewPlugin.h"

typedef class CMailbox * PMailbox;

class CMailbox
{
protected:
  CMailbox()
  {
  }

  static BOOL CopyString( LPCSTR szSrc, LPSTR szDst, LPDWORD lpSize );

public:
  virtual ~CMailbox()
  {
  }

  static PMailbox Create( LPCSTR szFileName );
  static PMailbox Create( LPCVOID lpMem, DWORD dwSize );

  static DWORD GetMsgType();

  static BOOL GetName( LPSTR szName, LPDWORD lpSize );
  static BOOL GetShortName( LPSTR szName, LPDWORD lpSize );
  static BOOL GetFilesMasks( LPSTR szMasks, LPDWORD lpSize );

  virtual DWORD GetNextMsg( DWORD dwPrevID ) = 0;

  virtual BOOL GetMsgInfo( DWORD dwMsgID, PMsgInfo lpInfo );
  virtual BOOL SetMsgInfo( DWORD dwMsgID, PMsgInfo lpInfo );

  virtual MV_RESULT GetMsgHead( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize );

  virtual BOOL GetMsg( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize ) = 0;
  virtual BOOL AddMsg( DWORD dwMsgID, const LPBYTE lpMsg, DWORD dwSize );
  virtual BOOL DelMsg( DWORD dwMsgID );

  virtual MV_RESULT Purge();
};

#define IMPLEMENT_INFORMATION(MsgType,ShortName,Name,FilesMasks)\
DWORD CMailbox::GetMsgType() { return MsgType; } \
BOOL CMailbox::GetName(LPSTR szName,LPDWORD lpSize) { return CopyString(Name,szName,lpSize); } \
BOOL CMailbox::GetShortName(LPSTR szName,LPDWORD lpSize) { return CopyString(ShortName,szName,lpSize); } \
BOOL CMailbox::GetFilesMasks(LPSTR szMasks,LPDWORD lpSize) { return CopyString(FilesMasks,szMasks,lpSize); }
//#define IMPLEMENT_CREATION(ClassName)

inline BOOL CMailbox::GetMsgInfo( DWORD dwMsgID, PMsgInfo lpInfo )
{
  return FALSE;
}

inline BOOL CMailbox::SetMsgInfo( DWORD dwMsgID, PMsgInfo lpInfo )
{
  return FALSE;
}

inline MV_RESULT CMailbox::GetMsgHead( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize )
{
  return MV_NOTIMPL;
}

inline BOOL CMailbox::AddMsg( DWORD dwMsgID, const LPBYTE lpMsg, DWORD dwSize )
{
  return FALSE;
}

inline BOOL CMailbox::DelMsg( DWORD dwMsgID )
{
  return FALSE;
}

inline MV_RESULT CMailbox::Purge()
{
  return MV_NOTIMPL;
}

#endif //!defined(___MVPlugin_H___)
