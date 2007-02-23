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
#include <string.h>
#include "StrPtr.h"

//////////////////////////////////////////////////////////////////////////
//

void GetStrPtr( const char * Text, StrPtr * pStrPtr )
{
  if ( Text == NULL )
  {
    pStrPtr->Len = 0;
    pStrPtr->Nxt = NULL;
    pStrPtr->Str = NULL;
    return;
  }

  pStrPtr->Str = (char*)Text;

  const char * p = pStrPtr->Str;

  while ( *p && *p != '\r' && *p != '\n' )
  {
    p ++;
  }

  pStrPtr->Len = p - Text;

  if ( *p == 0 )
  {
    pStrPtr->Nxt = NULL;
    return;
  }

  pStrPtr->Len = p - Text;

  if ( p[ 0 ] == '\r' && p[ 1 ] == '\n' )
  {
    p ++;
  }

  p ++;

  pStrPtr->Nxt = (char*)p;

  if ( *pStrPtr->Nxt == 0 )
  {
    pStrPtr->Nxt = NULL;
  }
}

void GetStrPtr( const char * Text, StrPtr * pStrPtr, const char * Last )
{
  if ( Text == NULL || Text >= Last )
  {
    pStrPtr->Len = 0;
    pStrPtr->Nxt = NULL;
    pStrPtr->Str = NULL;
    return;
  }

  pStrPtr->Str = (char*)Text;

  const char * p = pStrPtr->Str;

  while ( *p && p < Last && *p != '\r' && *p != '\n' )
    p ++;

  pStrPtr->Len = p - Text;

  if ( *p == '\0' || p >= Last )
  {
    pStrPtr->Nxt = NULL;
    return;
  }

  if ( p[ 0 ] == '\r' && p[ 1 ] == '\n' )
  {
    p ++;
  }

  p ++;

  pStrPtr->Nxt = (char*)p;

  if ( *pStrPtr->Nxt == 0 || p >= Last )
  {
    pStrPtr->Nxt = NULL;
  }
}
