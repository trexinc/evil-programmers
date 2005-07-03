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
// #define CODETEST

#ifdef CODETEST
 #include <stdio.h>
 #include <stdlib.h>
 #include <windows.h>
#else
 #include "farmail.hpp"
#endif

#define PRIMARY_CODE 4
#define SECONDARY_CODE 9

static void GetRandomString( uchar *str , int len )
{
  int j = 0;
  while ( len > 0 )
  {
    str[j++] = (uchar) ( 128+Random(128) );
    len--;
  }
  str[j] = 0;
}

static void GetFixedCodeString( uchar *str , int len )
{
  int j = 0;

  // hiding secondary code from hex viewers :)
  if ( j < len ) str[j++] = 'k';
  if ( j < len ) str[j++] = '7';
  if ( j < len ) str[j++] = 'n';
  if ( j < len ) str[j++] = 'T';
  if ( j < len ) str[j++] = 'D';
  if ( j < len ) str[j++] = 'H';
  if ( j < len ) str[j++] = '$';
  if ( j < len ) str[j++] = 't';
  if ( j < len ) str[j++] = '5';
  if ( j < len ) str[j++] = '7';
  if ( j < len ) str[j++] = 'e';
  if ( j < len ) str[j++] = '5';
  if ( j < len ) str[j++] = '3';
  if ( j < len ) str[j++] = 'x';
  if ( j < len ) str[j++] = 'F';

  str[j] = 0;
}


static void XORstr( uchar *str, uchar *psw, int len )
{
  for (int i=0, j=0; i<lstrlen((char*)str); i++ )
  {
    str[i] ^= psw[j];
    j++;
    if ( j>=len ) j = 0;
  }
}


void EncodeString( uchar *str )
{
  uchar psw[80];
  uchar code1[PRIMARY_CODE+1];
  uchar code2[SECONDARY_CODE+1];

  if ( !*str ) return;

  srand(GetTickCount());

  lstrcpy( (char*)psw, (char*)str );

  GetRandomString( code1 , PRIMARY_CODE );

  str[0] = 255;
  str[1] = 0;
  lstrcat( (char*)str, (char*)code1 );

  XORstr( psw, code1, PRIMARY_CODE );
  lstrcat( (char*)str, (char*)psw );

  GetFixedCodeString( code2 , SECONDARY_CODE );
  XORstr( str+1, code2, SECONDARY_CODE );
}


void DecodeString( uchar *str )
{
  uchar code1[PRIMARY_CODE+1];
  uchar code2[SECONDARY_CODE+1];
  uchar psw[80];

  if ( str[0] != 0xFF ) return ;

  GetFixedCodeString( code2 , SECONDARY_CODE );
  XORstr( str+1, code2, SECONDARY_CODE );

  lstrcpyn( (char*)code1, (char*)str+1, PRIMARY_CODE+1 );

  lstrcpy( (char*)psw, (char*)str+5 );
  XORstr( psw, code1, PRIMARY_CODE );

  lstrcpy( (char*)str, (char*)psw );
}



#ifdef CODETEST
void main( void )
{
 uchar str[80];

 while ( 1 )
 {
  printf("Enter string:");
  gets(str);
  printf("\n");

  EncodeString( str );
  printf("Encoded:%s\n", str );
  DecodeString( str );
  printf("Decoded:%s\n\n", str );

 }
}
#endif
