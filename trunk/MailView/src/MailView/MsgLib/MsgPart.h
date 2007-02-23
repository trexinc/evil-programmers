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
#ifndef ___MsgPart_H___
#define ___MsgPart_H___

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MsgLib.h"

class CMsgPartT
{
  friend class CMessageT;

private:
  bool m_Decoded;
  PContent GetDecodedContent();

protected:
  bool m_DoneParts;

  PKludges m_Kludges;
  PContent m_Content;

  PMessage m_Message;
  PMsgPart m_Next;

  CMsgPartT( PMessage Message );

  virtual PMsgPart MakeParts();
public:
  virtual ~CMsgPartT();

  PMessage GetMessage();
  PKludges GetKludges();

  PVOID GetContentData();
  DWORD GetContentSize();

  bool SaveToFile( LPCSTR FileName, bool bOverwrite = true );

  PMsgPart GetNext();

  virtual LPCSTR GetKludge( LPCSTR Name );
};

inline PMessage CMsgPartT::GetMessage()
{
  return m_Message;
}

inline PKludges CMsgPartT::GetKludges()
{
  return m_Kludges;
}

inline bool CMsgPartT::SaveToFile( LPCSTR FileName, bool bOverwrite )
{
  return GetDecodedContent()->SaveToFile( FileName, bOverwrite );
}

inline PMsgPart CMsgPartT::GetNext()
{
  return m_Next;
}

#endif //!defined(___MsgPart_H___)
