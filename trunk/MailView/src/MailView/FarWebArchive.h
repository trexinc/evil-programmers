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
// WebArchive.h: interface for the CWebArchive class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WEBARCHIVE_H__21CD083D_4DDA_48BC_8D54_793BD3003F72__INCLUDED_)
#define AFX_WEBARCHIVE_H__21CD083D_4DDA_48BC_8D54_793BD3003F72__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FarInetMessage.h"
#include "MsgLib/InetMessage.h"
#include "Mailbox.h"


class CFarWebArchive : public CFarInetMessage
{
private:
  FILETIME m_CreationTime;
  FILETIME m_LastAccessTime;
  FILETIME m_LastWriteTime;
public:
  CFarWebArchive() : CFarInetMessage()
  {
  }
  CFarWebArchive( LPCSTR FileName, long Encoding = FCT_DEFAULT, bool headOnly = false );
  virtual ~CFarWebArchive()
  {
  }

  virtual bool read( LPCSTR FileName, long encoding, bool headOnly );
  virtual bool read( long wParam, long lParam, long encoding, bool headOnly );

  virtual LPCSTR GetFmtName();

  virtual void GetSent( LPFILETIME ft );
  virtual void GetReceived( LPFILETIME ft );
  virtual void GetAccessed( LPFILETIME ft );
};

#endif // !defined(AFX_WEBARCHIVE_H__21CD083D_4DDA_48BC_8D54_793BD3003F72__INCLUDED_)
