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

static const char BASE64[] =
{
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q',
  'R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h',
  'i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y',
  'z','0','1','2','3','4','5','6','7','8','9','+','/'
};

void EncodeBase64 ( char * dest, char *source , int num )
{
 int i = 0, i0,i1,i2,i3;
 char ch[5];

 *dest = 0;
 ch[4] = 0;

 while ( i < num ) {

    DWORD dw ;

    int c1, c2, c3;

    c1 = (unsigned char)source[i];
    i++;
    if ( i<num ) c2 = (unsigned char)source[i]; else c2 = 0;
    i++;
    if ( i<num ) c3 = (unsigned char)source[i]; else c3 = 0;
    i++;

    dw = c3;
    dw += ( c2 << 8 );
    dw += ( c1 << 16 );

    i0 = ( dw & 0xFC0000 ) >> 18 ;
    i1 = ( dw & 0x3F000  ) >> 12 ;
    i2 = ( dw & 0xFC0    ) >> 6 ;
    i3 = ( dw & 0x3F     )      ;

    ch[0] = BASE64[ i0 ];
    ch[1] = BASE64[ i1 ];
    ch[2] = BASE64[ i2 ];
    ch[3] = BASE64[ i3 ];

    if ( i > num )   ch[3] = '=';
    if ( i > num+1 ) ch[2] = '=';

    lstrcat( dest, ch );
 }


}


static DWORD __GetCode( char c )
{
 DWORD i;
 for ( i=0; i<64; i++ )
    if ( BASE64[i] == c ) return i;
 return 0;
}




void DecodeBase64 ( char * dest, char *source , int num )
{
 *dest = '\0';

 while ( num > 0 ) {

    char c3,c4;

    DWORD i1 = __GetCode( *(source++) );
    DWORD i2 = __GetCode( *(source++) );
    DWORD i3 = __GetCode( c3 = *(source++) );
    DWORD i4 = __GetCode( c4 = *(source++) );

    DWORD dw = i4 + ( i3 << 6 ) + ( i2 << 12 ) + (i1 << 18 );

    int cc1 = ( dw & 0xFF0000 ) >> 16;
    int cc2 = ( dw & 0x00FF00 ) >> 8;
    int cc3 = ( dw & 0xFF ) ;

    strcatchr( dest, (char)cc1 );
    if ( c3 != '=' ) strcatchr( dest, (char)cc2 );
    if ( c4 != '=' ) strcatchr( dest, (char)cc3 );

    num-=4;

 }
 strcatchr( dest, '\0' );

}
