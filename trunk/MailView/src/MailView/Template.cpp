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
// Template.cpp: implementation of the CTemplate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Template.h"

#include <FarPlus.h>
#include <FarFile.h>
#include <ctype.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CTemplate::processFile( LPCSTR fileName, IWriteSink *f )
{
  FarTextFile ff;

  if ( !ff.OpenForRead( fileName ) )
    return;

  LPCSTR p;
  while ( !ff.IsEOF() )
  {
    // !!! сраный оптимизатор
    // !!! здесь создение объекта, а потом прирпвнивание почему-то не проходит...
    FarString s = ff.ReadLine();

    p = s.c_str();

    if ( *p == ';' )
      continue;

    processLine( p, f );

    f->write( "\r\n", 2 );
  }
}

void CTemplate::processLine( LPCSTR str, IWriteSink *f )
{
  LPCSTR end;

  while ( str && *str )
  {
    end = strchr( str, '%' );
    if ( end == NULL )
    {
      f->write( str, strlen( str ) );
    }
    else
    {
      end ++;

      if ( *end == '%' )
      {
        f->write( str, end - str );

        end ++;
      }
      else
      {
        f->write( str, end - str - 1 );

        str = end;

        end = strchr( str, '%' );

        if ( end )
        {

          char * macro = (char*)memcpy( new char[ end - str + 1 ], str, end - str );

          macro[ end - str ] = '\0';

          char * p = strchr( macro, ':' );

          int fillChar = 0;
          int macroLen = 0;

          if ( p )
          {
            *p++ = '\0';

            if ( isdigit( (unsigned char)*p ) == 0 )
              fillChar = (unsigned char)*p++;

            macroLen = FarSF::atoi( p );
          }


          processMacro( macro, macroLen, fillChar, *f );

          delete [] macro;

          end ++;
        }
        else
        {
          f->write( str, strlen( str ) );
        }

        str = end;
      }
    }

    str = end;
  }
}
