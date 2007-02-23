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
#ifndef ___Message_H___
#define ___Message_H___

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MsgLib.h"
#include "../Person.h"

class CMessageT
{
  friend class CMsgPartT;
private:
  long m_Encoding;
  long m_defaultEncoding;

protected:
  void FreeParts();

  struct TMemoryBlockEx : public TMemoryBlock
  {
    DWORD Capacity;
    TMemoryBlockEx() : TMemoryBlock(), Capacity( 0 )
    {
    }
    ~TMemoryBlockEx()
    {
    }
  };

  void AllocData( DWORD nSize );
  TMemoryBlockEx m_Data;

  PMsgPart       m_Part;

  virtual bool Init( long Encoding ) = 0;
  void MakeParts();

  virtual void DecodeContent( PContent Content, LPCSTR TransferEncoding ) = 0;
  virtual DWORD CalculateDecodedContentSize( PContent Content, LPCSTR TransferEncoding ) = 0;

  CMessageT();

public:
  virtual ~CMessageT();

  virtual bool read( LPCSTR FileName, long encoding, bool headOnly );
  virtual bool read( long wParam, long lParam, long encoding, bool headOnly );

  PMsgPart GetNextPart( PMsgPart Prev );
  virtual PMsgPart GetTextPart() = 0;

  LPCSTR GetKludge( LPCSTR Name );
  FarString GetDecodedKludge( LPCSTR Name );
  PKludges GetKludges();

  virtual bool DecodeKludge( LPSTR Data );
  virtual bool DecodeContent( PContent Data );

  virtual long GetEncoding()
  {
    return m_Encoding;
  }

  virtual bool SetEncoding( long Encoding )
  {
    if ( Encoding == -1 )
      m_Encoding = getDefaultEncoding();
    else
      m_Encoding = Encoding;

    return true;
  }

  long getDefaultEncoding()
  {
    return m_defaultEncoding;
  }

  void setDefaultEncoding( long encoding )
  {
    m_defaultEncoding = encoding;
  }

  virtual bool setEncoding( LPCSTR encoding )
  {
    return true;
  }

  virtual LPCSTR GetFmtName() = 0;

  DWORD GetSize();

  virtual FarString GetId() = 0;
  virtual void GetSent( LPFILETIME ft ) = 0;
  virtual void GetReceived( LPFILETIME ft ) = 0;
  virtual void GetAccessed( LPFILETIME ft ) = 0;
  virtual FarString GetDate() = 0;
  virtual DWORD GetPriority() = 0;
  virtual DWORD GetFlags() = 0;
  virtual FarString GetSubject() = 0;
  virtual PPerson GetFrom() = 0;
  virtual PPerson GetTo() = 0;
  virtual PPerson GetCc() = 0;
  virtual FarString GetArea() = 0;
  virtual FarString GetNewsgroups() = 0;
  virtual FarString GetMailer() = 0;
  virtual FarString GetOrganization() = 0;

  // первый элемент должен содержать InReplyTo значение
  virtual void GetReferences( FarStringArray& strings ) = 0;

  virtual DWORD GetAttchmentsCount()
  {
    return 0;
  }
};

#endif //!defined(___Message_H___)
