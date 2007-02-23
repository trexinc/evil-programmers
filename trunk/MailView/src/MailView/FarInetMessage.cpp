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
// InetMessage.cpp: implementation of the CInetMessage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FarInetMessage.h"
#include "MsgLib/MimeContent.h"
#include "Kludges.h"
#include "Decoder.h"
#include "LangId.h"
#include "MailView.h"
#include <ctype.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LPCSTR CFarInetMessage::GetFmtName()
{
  return Far::GetMsg( MInetMessage );
}

CFarInetMessage::CFarInetMessage( LPCSTR fileName, long encoding, bool headOnly )
  : CInetMessageT()
{
  CInetMessageT::read( fileName, encoding, headOnly );
}

bool CFarInetMessage::read( long wParam, long lParam, long encoding, bool headOnly )
{
  return read( (CMailbox*)wParam, (DWORD)lParam, encoding, headOnly );
}

bool CFarInetMessage::read( CMailbox * mailbox, DWORD dwMsgId, long encoding, bool headOnly )
{
  FreeParts();

  if ( headOnly )
  {
    m_Data.size = 1024;
    if ( mailbox->getMsgHead( dwMsgId, NULL, &m_Data.size ) != MV_OK )
      return false;
  }
  else
  {
    if ( mailbox->getMsg( dwMsgId, NULL, &m_Data.size ) != MV_OK )
      return false;
  }

  AllocData( m_Data.size );

  if ( headOnly )
  {
    if ( mailbox->getMsgHead( dwMsgId, m_Data.data, &m_Data.size ) != MV_OK )
      return false;
  }
  else
  {
    if ( mailbox->getMsg( dwMsgId, m_Data.data, &m_Data.size ) != MV_OK )
      return false;
  }

  bool result = Init( encoding );

  mailbox->getMsgInfo( dwMsgId, &m_Info );

  //m_Mailbox = Mailbox;

  return result;
}

bool CFarInetMessage::SetEncoding( long Encoding )
{
  if ( Encoding == FCT_DEFAULT )
  {
    Encoding = FCT__INVALID;

    CMimeContent mc(GetKludge(K_RFC_ContentType));
    if (FarSF::LStricmp(mc.getType(), "text") == 0)
    {
      LPCSTR charset = mc.getDataValue("charset");
      if (charset != NULL)
        Encoding = getCharacterTable(charset);
    }
  }
  else if ( Encoding == FCT_DETECT )
    Encoding = Far::DetectCharTable( (LPSTR)m_Data.data, m_Data.size );

  return CInetMessageT::SetEncoding( Encoding );
}

bool CFarInetMessage::setEncoding( LPCSTR encoding )
{
  return encoding ? SetEncoding( getCharacterTable( encoding ) ) : false;
}


extern char * strxcpy( char * d, const char * s, int n );
FarString UnMimeHeader( LPCSTR header, int FarEncoding )
{
  char enc[ 256 ], tmp[ 256 ];
  FarString res;

  CBase64Decoder b64;
  CQuotedPrintableSubjectDecoder qp;

  LPCSTR ps = header, pe = ps, pb, pm;

  while ( ( pb = strstr( ps, "=?" ) ) != NULL )
  {
    if ( pb - ps > 0 )
      res += ToOEMString( FarString( ps, pb - ps ), FarEncoding );

    pb += 2;

    if ( ( pm = strchr( pb, '?' ) ) != NULL )
    {
      int dc = FarSF::LUpper( *++pm );
      if ( dc == 'B' || dc == 'Q' )
      {
        if ( ( pe = strstr( pm + 2, "?=" ) ) != NULL )
        {
          strxcpy( enc, pb, pm - pb );

          strxcpy( tmp, pm + 2, pe - pm - 1 );

          CMimeDecoder * d;
          if ( dc == 'B' )
            d = &b64;
          else
            d = &qp;

          dc = d->decode( (PBYTE)tmp, strlen( tmp ) );
          if ( dc > 0 )
            tmp[ dc ] = '\0';

          long d_enc = getCharacterTableNoDefault( enc );

          if ( d_enc == FCT__INVALID )
            d_enc = FarEncoding;

          res += ToOEMString( tmp, d_enc );

          pe += 2;
          pb = pe;

          while ( *pe && isspace( (unsigned char)*pe ) ) pe ++;

          if ( pe[ 0 ] == '=' && pe[ 1 ] == '?' )
            pb = pe;
        }
        else
          pb = pm;
      }
    }

    ps = pb;
  }

  if ( *ps != '\0' )
    res += ToOEMString( ps, FarEncoding );

  return res;
}

bool CFarInetMessage::DecodeKludge( LPSTR Data )
{
  far_assert( Data != NULL );
  if ( *Data != '\0')
    strcpy( Data, UnMimeHeader( Data, GetEncoding() ) );
  return true;
}
