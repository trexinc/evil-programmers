/*
    FARMail plugin for FAR Manager
    Copyright (C) 2002-2004 FARMail Group
    Copyright (C) 1999,2000 Serge Alexandrov

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
#include "farmail.hpp"

void strcatchr( char *str, char s )
{
 char buf[2];
 buf[0] = s; buf[1] = 0;
 lstrcat( str, buf );
}

#if 0
//реально нигде не используется
//если вдруг где-то будет использоваться - обратить внимание на rfc2047 пункт 4.2.
//возможно "_" надо тоже кодировать.
int EncodeQuotedPrintable( char *source, int lensrc, char *dest )
{
 int i;
 *dest = 0;

 for ( i=0 ; i<lensrc; i++ ) {

    int c = ((unsigned char)source[i]);
    if ( ( c<32 && c!=9 ) || c == 61 || c >126 ) {
       char buf[10];
       FSF.sprintf( buf, "=%02X", c );
       lstrcat( dest, buf );
    } else {
       strcatchr( dest, (char)c );
    }

 }
 return 0;
}
#endif

int DecodeQuotedPrintable( char *source, int lensrc, char *dest )
{
  int i=0;
  *dest = 0;

  while ( i<lensrc )
  {
    int c = source[i];
    if ( c == '=' )
    {
      char buf[3];
      buf[0] = source[++i];
      buf[1] = source[++i];
      buf[2] = 0;

      long res;
      FSF.sscanf( buf, "%lx", &res );

      strcatchr( dest, (char)res );

      i++;
    }
    else
    {
      if(c=='_') c=' '; //rfc2047 4.2
      strcatchr( dest, (char)c );
      i++;
    }
  }
  return 0;
}

char * SplitHeaderLine( char *line, char *charset, char *encoding, char *text )
{
 *charset = 0;
 *encoding = 0;
 *text = 0;

 char * ptr = strstr( line, "=?" );
 char *ptr2 = strstr( line, "?=" );

 if ( !ptr2 ) ptr2 = line+lstrlen(line);

 if ( ptr && ptr2 ) {
    ptr += 2; // charset

    while ( *ptr != '?' && *ptr ) {
       strcatchr( charset , *ptr );
       ptr++;
    }
    strcatchr( charset, '\0' );

    if ( ! *ptr ) return ptr2;
    ptr++;

    while ( *ptr != '?' && *ptr ) {
       strcatchr( encoding , *ptr );
       ptr++;
    }
    strcatchr( encoding, '\0' );

    if ( ! *ptr ) return ptr2;

    ptr++;

    ptr2 = strstr( ptr, "?=" );
    while ( ( ptr2 && ptr < ptr2 ) || ( !ptr2 && *ptr ) ) {
       strcatchr( text , *ptr );
       ptr++;
    }

    if ( ptr2 ) ptr+=2;
    return ptr;

 } else return line;

}

int GetGeaderField(const char *header,char *field,const char *type,int len)
{
  char buf[50] = "\n";
  char *lwr_hdr = z_strdup( header );
  char *lwr_typ = z_strdup( type );

  *field = 0;

  if ( !lwr_hdr || !lwr_typ ) return 1;

  FSF.LStrlwr( lwr_hdr );
  FSF.LStrlwr( lwr_typ );

  lstrcat( buf, lwr_typ );

  const char *ptr = strstr( lwr_hdr, buf );
  if ( !ptr ) {
      // may be it's the first line?
      ptr = strstr( lwr_hdr, lwr_typ );
      if ( ptr && ptr != lwr_hdr ) ptr = NULL;
  } else ptr+=1;

  if ( !ptr ) {
      z_free(lwr_hdr);
      z_free(lwr_typ);
      return 1;
  }

  ptr = header + ( ptr - lwr_hdr );

  ptr += lstrlen( type );
  while ( *ptr == 32 || *ptr == 9 ) ptr++;

  while ( len )
  {
    if ( *ptr != 0x0d && *ptr != 0x0a && *ptr )
    {
      strcatchr( field , *ptr );
      ptr++;
      len--;
    }
    else if ( *ptr == 0x0d || *ptr == 0x0a )
    {
      while ( *ptr == 0x0d || *ptr == 0x0a ) ptr++;
      if ( *ptr != 32 && *ptr != 9 ) break; // next field
      while ( *ptr == 32 || *ptr == 9 ) ptr++;
      //rfc822 3.1.1.
      //Unfolding is accomplished by regarding CRLF immediately followed
      //by a LWSP-char as equivalent to the LWSP-char.
      if(len)
      {
        strcatchr(field,' ');
        len--;
      }
    }
    else
      break;
  }
  strcatchr( field, '\0' );
  //rfc2047 6.2.
  //When displaying a particular header field that contains multiple
  //'encoded-word's, any 'linear-white-space' that separates a pair of
  //adjacent 'encoded-word's is ignored.
  { //превращение ?=   =? в ?==?
    char *field_from=field,*field_to=field;
    while(true)
    {
      if(!strncmp(field_from,"?=",2))
      {
        char *field_from2=field_from+2;
        while(*field_from2==32||*field_from2==9) field_from2++;
        if(!strncmp(field_from2,"=?",2))
        {
          field_from=field_from2+2;
          lstrcpy(field_to,"?==?");
          field_to+=4;
        }
      }
      *field_to=*field_from++;
      if(!*field_to) break;
      field_to++;
    }
  }
  z_free(lwr_hdr);
  z_free(lwr_typ);
  return 0;
}
