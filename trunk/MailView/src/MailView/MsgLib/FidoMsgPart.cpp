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
#include "stdafx.h"
#include "FidoMsgPart.h"
#include "../StrPtr.h"


CFidoMsgPart::CFidoMsgPart( PMessage Message, LPSTR lpData, DWORD dwSize ) : CMsgPartT( Message )
{
  m_Kludges->SetOwnsItems( true );

  LPSTR Last = lpData + dwSize;
  LPSTR Next = lpData;
  while ( Next < Last )
  {
    LPSTR Ptr = Next + 1;
    while ( Ptr < Last && *Ptr != '\r' && *Ptr != '\n' && *Ptr != '\1' && *Ptr != '\0' )
      Ptr ++;

    if( Ptr >= Last )
      break;

    if ( *Ptr == '\0' )
    {
      if ( FarSF::LStrnicmp( Ptr + 1, "From: ", 6 ) != 0 )
      {
        Next = Ptr + 1;
        break;
      }

      Ptr ++;
    }

    LPSTR Nxt = Ptr;
    while ( Nxt < Last && ( *Nxt== '\r' || *Nxt == '\n' ) )
      Nxt ++;

    m_Kludges->Add( Next, Ptr - Next );

    if ( *Nxt != '\1' && *Next != '\1' )
    {
      //if ( FarSF::LStrnicmp( Nxt, "From: ", 6 ) != 0 )
        break;
    }

    Next = Nxt;
  }

  m_Content->data = (PBYTE)Next;
  m_Content->size = Last - Next;
}

CFidoMsgPart::~CFidoMsgPart()
{
}

PMsgPart CFidoMsgPart::MakeParts()
{
  return this;
}
