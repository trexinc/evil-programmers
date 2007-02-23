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
// InetMessage.h: interface for the CInetMessage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INETMESSAGE_H__E19AB692_0D88_4AAB_A83F_98B4DE19EEF3__INCLUDED_)
#define AFX_INETMESSAGE_H__E19AB692_0D88_4AAB_A83F_98B4DE19EEF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MsgLib/InetMessage.h"
#include "Mailbox.h"
#include "FarMultiLang.h"

class CFarInetMessage : public CInetMessageT
{
private:

public:
  virtual ~CFarInetMessage()
  {
  }
  CFarInetMessage() : CInetMessageT()
  {
  }
  CFarInetMessage( LPCSTR fileName, long encoding, bool headOnly );
  virtual LPCSTR GetFmtName();

  virtual bool read( long wParam, long lParam, long encoding, bool headOnly  );
  bool read( CMailbox * mailbox, DWORD dwMsgID, long encoding, bool headOnly );

  virtual bool SetEncoding( long encoding );
  virtual bool setEncoding( LPCSTR encoding );

  virtual bool DecodeKludge( LPSTR Data );

};

#endif // !defined(AFX_INETMESSAGE_H__E19AB692_0D88_4AAB_A83F_98B4DE19EEF3__INCLUDED_)
