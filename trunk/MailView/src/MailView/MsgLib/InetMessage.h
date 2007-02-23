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
#ifndef ___InetMessage_H___
#define ___InetMessage_H___

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Message.h"
#include "../MailViewPlugin.h"

class CInetMessageT : public CMessageT
{
private:
  virtual void DecodeContent( PContent Content, LPCSTR TransferEncoding );
  virtual DWORD CalculateDecodedContentSize( PContent Content, LPCSTR TransferEncoding );

  PPerson m_From, m_To, m_Cc;

protected:
  virtual bool Init( long Encoding );

  TMsgInfo m_Info;

  CInetMessageT();
public:
  virtual ~CInetMessageT();

  virtual PMsgPart GetTextPart();

  virtual bool DecodeKludge( LPSTR Data );

  virtual FarString GetId();
  virtual void GetSent( LPFILETIME ft );
  virtual void GetReceived( LPFILETIME ft );
  virtual void GetAccessed( LPFILETIME ft );
  virtual FarString GetDate();
  virtual DWORD GetPriority();
  virtual DWORD GetFlags();
  virtual FarString GetSubject();
  virtual PPerson GetFrom();
  virtual PPerson GetTo();
  virtual PPerson GetCc();
  virtual FarString GetArea();
  virtual FarString GetNewsgroups();
  virtual FarString GetMailer();
  virtual FarString GetOrganization();
  void GetReferences( FarStringArray& strings );
  virtual DWORD GetAttchmentsCount();

  virtual bool SetEncoding( long Encoding );
};

#endif //!defined(___InetMessage_H___)
