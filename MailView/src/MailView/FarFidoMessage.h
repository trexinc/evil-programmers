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
// FidoMessage.h: interface for the CFidoMessage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FIDOMESSAGE_H__1F2BDF80_5167_40AD_9DB4_70B6B4D59452__INCLUDED_)
#define AFX_FIDOMESSAGE_H__1F2BDF80_5167_40AD_9DB4_70B6B4D59452__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Message.h"
#include "MsgLib/FidoMessage.h"

class CFarFidoMessage : public CFidoMessageT
{
private:

public:
  CFarFidoMessage();
  CFarFidoMessage( LPCSTR HostFile, long encoding, bool headOnly );
  virtual ~CFarFidoMessage();

  /** получить имя */
  virtual LPCSTR GetFmtName();

  virtual bool read( long wParam, long lParam, long encoding, bool headOnly );
  bool read( CMailbox * pMailbox, DWORD dwMsgID, long encoding, bool headOnly );
  virtual bool read( LPCSTR HostFile, long encoding, bool headOnly );

  virtual bool SetEncoding( long encoding );
};

#endif // !defined(AFX_FIDOMESSAGE_H__1F2BDF80_5167_40AD_9DB4_70B6B4D59452__INCLUDED_)
