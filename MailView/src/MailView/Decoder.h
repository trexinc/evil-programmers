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

#ifndef _DECODER_H_
#define _DECODER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <FarPlus.h>

// default decoder for "binary", "7bit" and "8bit" data
class CMimeDecoder
{
private:
protected:
public:
  CMimeDecoder();
  virtual ~CMimeDecoder();

  virtual DWORD getSize(PBYTE data, DWORD size)
  {
    return size;
  }

  virtual DWORD decode(PBYTE data, DWORD size)
  {
    return size;
  }

  static CMimeDecoder * Create( LPCSTR TransferEncoding );

  static bool isCoded( LPCSTR TransferEncoding );
};


class CQuotedPrintableDecoder : public CMimeDecoder
{
private:
public:
  CQuotedPrintableDecoder() : CMimeDecoder()
  {
  }
  virtual ~CQuotedPrintableDecoder()
  {
  }

  virtual DWORD getSize(PBYTE data, DWORD size );
  virtual DWORD decode(PBYTE data, DWORD size );
};

class CQuotedPrintableSubjectDecoder : public CQuotedPrintableDecoder
{
private:
public:
  CQuotedPrintableSubjectDecoder() : CQuotedPrintableDecoder()
  {
  }
  virtual ~CQuotedPrintableSubjectDecoder()
  {
  }

  virtual DWORD decode(PBYTE data, DWORD size);
};

class CBase64Decoder : public CMimeDecoder
{
private:
  static const int m_Index64[ 128 ];
  //char m_XChars[ 256 ];
  LPBYTE m_TmpData;
  DWORD  m_TmpSize;
public:
  CBase64Decoder();
  virtual ~CBase64Decoder();

  virtual DWORD getSize(PBYTE data, DWORD size);
  virtual DWORD decode(PBYTE data, DWORD size);
};

class CUUEDecoder : public CMimeDecoder
{
private:
public:
  CUUEDecoder() : CMimeDecoder()
  {
  }
  virtual ~CUUEDecoder()
  {
  }

  virtual DWORD getSize(PBYTE data, DWORD size);
  virtual DWORD decode(PBYTE data, DWORD size);
};


#endif // !defined(_DECODER_H_)
