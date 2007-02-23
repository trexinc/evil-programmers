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
#include "StdAfx.h"
#include "../Kludges.h"
#include "MimeContent.h"
#include "../StrPtr.h"

#include "InetMsgPart.h"

LPSTR FindString( LPCSTR str1, LPCSTR str2, LPCSTR Last )
{
  if ( str1 == NULL )
    return NULL;

  LPCSTR cp = str1;
  LPCSTR s1;
  LPCSTR s2;
  while ( cp < Last )
  {
    s1 = cp;
    s2 = str2;

    while ( s1 < Last && *s2 != '\0' && ( *s1 - *s2 ) == 0 )
      s1++, s2++;

    if ( *s2 == '\0' )
      return (LPSTR)cp;

    cp++;
  }

  return NULL;
}

CInetMsgPart::CInetMsgPart( PMessage Message, LPSTR lpData, DWORD dwSize ) : CMsgPartT( Message )
{
  LPSTR Last = lpData + dwSize;

  StrPtr sp = { NULL, 0, lpData };

  while ( sp.Nxt )
  {
    GetStrPtr( sp.Nxt, &sp, Last );

    if ( sp.Len == 0 )
      break;

    sp.Str[ sp.Len ] = '\0';

    if ( ( *sp.Str == '\t' || *sp.Str == '\x20' ) && m_Kludges->Count() > 0 )
    {
      *sp.Str = '\t'; // unfold
      strcat( m_Kludges->At( m_Kludges->Count() - 1 ), sp.Str );
    }
    else
    {
      m_Kludges->Add( sp.Str );
    }
  }

  m_Content->data = (PBYTE)sp.Nxt;
  m_Content->size = sp.Nxt? dwSize - ( sp.Nxt - lpData ) : 0;
}

CInetMsgPart::~CInetMsgPart()
{
}

PMsgPart CInetMsgPart::MakeParts()
{
  CMimeContent mc(GetKludge(K_RFC_ContentType));
  if (FarSF::LStricmp(mc.getType(), "multipart") != 0)
    return this;

  LPCSTR boundary = mc.getDataValue("boundary");

  if (boundary == NULL)
    return this;

  int boundaryLength = strlen(boundary);

  CInetMsgPart * Last = this;

  LPSTR data = (LPSTR)m_Content->data;
  LPSTR last = data + m_Content->size;

  LPSTR ptr = FindString(data, boundary, last);
  if (ptr != NULL && *(ptr-1) == '-' && *(ptr-2) == '-')
  {
    data = ptr + boundaryLength;
    do
    {
      while (*data == '\r' || *data == '\n' || *data == '\t' || *data == '\x20')
        data++;

      ptr = FindString(data, boundary, last);
      if (ptr != NULL && *(ptr-1) == '-' && *(ptr-2) == '-')
      {
        DWORD nextSize = ptr - data - 2;
        if (nextSize > 0)
        {
          CInetMsgPart * Next = create CInetMsgPart(m_Message, data, nextSize);
          Last->m_Next = Next;
          Last = (CInetMsgPart*)Next->MakeParts();
        }

        // проверим, а не последн€€ ли это секци€
        if (ptr[boundaryLength] == '-' && ptr[boundaryLength + 1] == '-')
          break;

        data = ptr + boundaryLength;
      }
      else
      {
        // уберем конечные пробелы
        while (last > data && (*last == '\r' || *last == '\n' || *last == '\t' || *last == '\x20'))
          last--;

        if (last > data)
        {
          CInetMsgPart * Next = create CInetMsgPart(m_Message, data, last - data);
          Last->m_Next = Next;
          Last = (CInetMsgPart*)Next->MakeParts();
        }

        break;
      }
    } while (data < last);

    m_Content->size = 0;
  }

  return Last;
}
