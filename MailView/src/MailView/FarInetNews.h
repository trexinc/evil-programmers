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
// InetNews.h: interface for the CInetNews class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INETNEWS_H__38E7A5FA_C646_4859_A0F8_D1363AC9EC45__INCLUDED_)
#define AFX_INETNEWS_H__38E7A5FA_C646_4859_A0F8_D1363AC9EC45__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MsgLib/InetMessage.h"
#include "Mailbox.h"
#include "FarMultiLang.h"

class CFarInetNews : public CInetMessageT
{
public:
  CFarInetNews() : CInetMessageT()
  {
  }

  CFarInetNews( LPCSTR FileName, long encoding = FCT_DEFAULT, bool headOnly = false );

  virtual ~CFarInetNews()
  {
  }

  virtual LPCSTR GetFmtName();
};

#endif // !defined(AFX_INETNEWS_H__38E7A5FA_C646_4859_A0F8_D1363AC9EC45__INCLUDED_)
