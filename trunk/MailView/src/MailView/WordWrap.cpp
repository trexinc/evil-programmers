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
#include "../plugin.hpp"
#include "WordWrap.h"
#include <ctype.h>

#pragma message( "----- WARNING: need implementation of WordWrapLines using ACTL_GETSYSWORDDIV" )

inline LPSTR GetNextSpace( LPSTR str, LPSTR WordDelims )
{
  while ( *str && *str != '\x20' && *str != '\t' ) str ++;
  //while ( *str && strchr( WordDelims, *str ) == NULL ) str ++;
  return str;
}

FarStringArray * WordWrapLines( FarStringArray& Lines, int Width, bool UseQuotes )
{
  LPSTR WordDelims = create char[ Far::AdvControl( ACTL_GETSYSWORDDIV, NULL ) + 1 ];
  WordDelims[ Far::AdvControl( ACTL_GETSYSWORDDIV, WordDelims ) ] = 0;

  FarStringArray * ss = create FarStringArray( true );
  FarString Quote;
  for ( int i = 0; i < Lines.Count(); i ++ )
  {
    LPSTR str = Lines.At( i );
    if ( *str == '\0' )
    {
      ss->Add( str );
      continue;
    }

    if ( UseQuotes )
    {
      char * qstr = str;

      while ( isspace( (unsigned char)*qstr ) )
        qstr ++;
      while ( FarSF::LIsAlpha( (BYTE)*qstr ) )
        qstr ++;

      if ( *qstr == '>' )
      {
        while ( *qstr == '>' )
          qstr ++;

        while ( isspace( (unsigned char)*qstr ) )
          qstr ++;

        Quote = FarString( str, qstr - str );

        str = qstr;
      }
      else
        Quote.Empty();
    }

    int W = Width - Quote.Length();

    int len = strlen( str );

    if ( *str == '\0' && !Quote.IsEmpty() )
      ss->Add( Quote );

    while ( *str )
    {
      if ( len <= W )
      {
        ss->Add( Quote + str );
        break;
      }
      LPSTR ptr = GetNextSpace( str, WordDelims );
      if ( ptr - str > W )
        ptr = str + W;

      while ( *ptr )
      {
        LPSTR nxt = GetNextSpace( ptr + 1, WordDelims );
        if ( *nxt == '\0' || nxt > str + W )
        {
          //char c = *ptr;
          *ptr = '\0';
          ss->Add( Quote + str );
#ifndef _SIMPLE_WRAP
          do { ptr ++;
          } while ( *ptr == '\x20' || *ptr == '\t' );
#else
          ptr++;
#endif
          len -= ptr - str;
          break;
        }
        ptr = nxt;
      }
      str = ptr;
    }
  }

  delete [] WordDelims;

  return ss;
}
