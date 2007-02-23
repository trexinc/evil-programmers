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
// MultiLanguage.cpp: implementation of the CMultiLanguage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MailView.h"
#include "MultiLanguage.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CMultiLanguage::Attach( HMODULE hLib )
  {
  Detach();
  m_hLib         = hLib;
  m_pOnInit      = (TOnInit)      GetProcAddress( m_hLib, "OnInit" );
  m_pOnExit      = (TOnExit)      GetProcAddress( m_hLib, "OnExit" );
  m_pGetCount    = (TGetCount)    GetProcAddress( m_hLib, "GetCount" );
  m_pGetName     = (TGetName)     GetProcAddress( m_hLib, "GetDescription" );
  m_pGetMimeName = (TGetMimeName) GetProcAddress( m_hLib, "GetMimeName" );
  m_pGetIndex    = (TGetIndex)    GetProcAddress( m_hLib, "GetIndex" );
  m_pGetCP       = (TGetCP)       GetProcAddress( m_hLib, "GetCodePage" );
  m_pGetFamilyCP = (TGetFamilyCP) GetProcAddress( m_hLib, "GetFamilyCP" );
  m_pUncToStr    = (TUncToStr)    GetProcAddress( m_hLib, "UncToStr" );
  m_pStrToUnc    = (TStrToUnc)    GetProcAddress( m_hLib, "StrToUnc" );
  if ( m_pOnInit )
    {
    m_pOnInit( NULL );
    }
  }
