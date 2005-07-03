/*
    FARMail plugin for FAR Manager
    Copyright (C) 2002-2004 FARMail Group

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

/*
UUEncode:

Note: The value of the first byte in the line minus 0x20
      is the total number of bytes encoded in that line. (max 45)

b1 = 0x20 + (( A >> 2                    ) & 0x3F)
b2 = 0x20 + (((A << 4) | ((B >> 4) & 0xF)) & 0x3F)
b3 = 0x20 + (((B << 2) | ((C >> 6) & 0x3)) & 0x3F)
b4 = 0x20 + (( C                         ) & 0x3F)

A = 10101010
B = 00011111
C = 11001101

        AAAAAA
b1 =  00101010 + 0x20

        AABBBB
b2 =  00100001 + 0x20

        BBBBCC
b3 =  00111111 + 0x20

        CCCCCC
b4 =  00001101 + 0x20

*/

void DecodeUUE ( char * dest, char *source , int num )
{
  if ( num > 1 )
  {
    int i = *(source++) - 0x20;
    num--;
    while (i > 0 && i < num)
    {
      int i1 = *(source++) - 0x20;
      int i2 = *(source++) - 0x20;
      int i3 = *(source++) - 0x20;
      int i4 = *(source++) - 0x20;

      int a = (i1<<2 & 0xfc) | (i2>>4 & 0x3);
      int b = (i2<<4 & 0xf0) | (i3>>2 & 0xf);
      int c = (i3<<6 & 0xc0) | (i4    & 0x3f);

      *(dest++) = (char)a;
      if ( i > 1 ) *(dest++) = (char)b;
      if ( i > 2 ) *(dest++) = (char)c;

      i-=3;
      num-=4;
    }
  }
  *dest = 0;
}
