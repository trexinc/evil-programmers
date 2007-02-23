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
// FidoMessage.cpp: implementation of the CFidoMessage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FarFidoMessage.h"
#include "LangId.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LPCSTR CFarFidoMessage::GetFmtName()
{
  return Far::GetMsg( MFidoMessage );
}

CFarFidoMessage::CFarFidoMessage() : CFidoMessageT()
{
}

CFarFidoMessage::CFarFidoMessage( LPCSTR hostFile, long encoding, bool headOnly )
: CFidoMessageT()
{
  CFidoMessageT::read( hostFile, encoding, headOnly );
}

CFarFidoMessage::~CFarFidoMessage()
{
}

bool CFarFidoMessage::read( LPCSTR hostFile, long encoding, bool headOnly )
{
  return true;
}

bool CFarFidoMessage::read( CMailbox * pMailbox, DWORD dwMsgId, long encoding, bool headOnly )
{
  FreeParts();

  if ( headOnly )
  {
    m_Data.size = 1024;
    if ( pMailbox->getMsgHead( dwMsgId, NULL, &m_Data.size ) != MV_OK )
      return false;
  }
  else
  {
    if ( pMailbox->getMsg( dwMsgId, NULL, &m_Data.size ) != MV_OK )
      return false;
  }

  AllocData( m_Data.size );

  if ( headOnly )
  {
    if ( pMailbox->getMsgHead( dwMsgId, m_Data.data, &m_Data.size ) != MV_OK )
      return false;
  }
  else
  {
    if ( pMailbox->getMsg( dwMsgId, m_Data.data, &m_Data.size ) != MV_OK )
      return false;
  }

  bool result = Init( encoding );

  pMailbox->getMsgInfo( dwMsgId, &m_Info );

  return result;
}

bool CFarFidoMessage::read( long wParam, long lParam, long encoding, bool headOnly )
{
  return read( (CMailbox*)wParam, (DWORD)lParam, encoding, headOnly );
}

bool CFarFidoMessage::SetEncoding( long encoding )
{
  if ( encoding == FCT_DEFAULT )
    encoding = FCT__INVALID;
  else if ( encoding == FCT_DETECT )
    encoding = Far::DetectCharTable( (LPSTR)m_Data.data, m_Data.size );

  return CFidoMessageT::SetEncoding( encoding );
}
