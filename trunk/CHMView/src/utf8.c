/*
    UTF8 simple decoder/encoder
    Copyright (C) 2004 Oleg Bondar

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

#include "utf8.h"

/* http://www.cl.cam.ac.uk/~mgk25/unicode.html

UTF-8 has the following properties:

    UCS characters U+0000 to U+007F (ASCII) are encoded simply as bytes 0x00 to
    0x7F (ASCII compatibility). This means that files and strings which contain
    only 7-bit ASCII characters have the same encoding under both ASCII and UTF-8.

    All UCS characters >U+007F are encoded as a sequence of several bytes, each of
    which has the most significant bit set. Therefore, no ASCII byte (0x00-0x7F)
    can appear as part of any other character.

    The first byte of a multibyte sequence that represents a non-ASCII character is
    always in the range 0xC0 to 0xFD and it indicates how many bytes follow for
    this character. All further bytes in a multibyte sequence are in the range 0x80
    to 0xBF. This allows easy resynchronization and makes the encoding stateless
    and robust against missing bytes.

    All possible 231 UCS codes can be encoded.

    UTF-8 encoded characters may theoretically be up to six bytes long, however
    16-bit BMP characters are only up to three bytes long.

    The sorting order of Bigendian UCS-4 byte strings is preserved.

    The bytes 0xFE and 0xFF are never used in the UTF-8 encoding.

The following byte sequences are used to represent a character. The sequence to
be used depends on the Unicode number of the character:


U-00000000 - U-0000007F:  0xxxxxxx
U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

The xxx bit positions are filled with the bits of the character code number in
binary representation. The rightmost x bit is the least-significant bit. Only
the shortest possible multibyte sequence which can represent the code number of
the character can be used. Note that in multibyte sequences, the number of
leading 1 bits in the first byte is identical to the number of bytes in the
entire sequence.
*/

/* assume: wchar_t == unsigned short == UCS-2 (2 byte long) */

/* encode UCS-2 into UTF-8 */
char *encode_UTF8(char *utf8s, wchar_t *ws)
{
    char    *s;

    s = utf8s;
    while(*ws) {
        if(*ws <= 0x007F) {                 /* 1 octet */
            *s++ = *ws;
        }
        else if(*ws <= 0x07FF) {            /* 2 octets */
            *s++ = 0xC0 | ((*ws >> 6) & 0x1F);
            *s++ = 0x80 | (*ws & 0x3F);
        }
        else if(*ws <= 0xFFFF) {            /* 3 octets */
            *s++ = 0xE0 | ((*ws >> 12) & 0x0F);
            *s++ = 0x80 | ((*ws >> 6) & 0x3F);
            *s++ = 0x80 | (*ws & 0x3F);
        }
        else {                              /* >= 4 octets -- not fit in UCS-2 */
            *s++ = '_';
        }
        ++ws;
    }
    *s = 0;
    return utf8s;
}

/* returns size of buffer in chars for convert UCS-2 into UTF-8 */
#if 0
int UTF8_strsize(wchar_t *ws)
{
    int     sz = 0;

    while(*ws) {
        if(*ws <= 0x007F)       sz += 1;
        else if(*ws <= 0x07FF)  sz += 2;
        else if(*ws <= 0xFFFF)  sz += 3;
        else                    sz += 1;
        ++ws;
    }
    return sz;
}
#endif
/*
0 0000  4 0100  8 1000  C 1100
1 0001  5 0101  9 1001  D 1101
2 0010  6 0110  A 1010  E 1110
3 0011  7 0111  B 1011  F 1111
 */

/* decode UTF-8 into UCS-2 */
wchar_t *decode_UTF8(wchar_t *ws, unsigned char *utf8s)
{
    wchar_t     *wc;

    wc = ws;
    while(*utf8s) {
        if(!(*utf8s & 0x80)) *wc = *utf8s++;    /* 1 octet */
        else if((*utf8s & 0xE0) == 0xC0) {      /* 2 octets */
            *wc = (*utf8s++ & 0x1F) << 6;       /* 1st */
            if((*utf8s & 0xC0) == 0x80) *wc |= *utf8s++ & 0x3F; /* 2nd */
            else *wc = *utf8s % 26 + 'a';
        }
        else if((*utf8s & 0xF0) == 0xE0) {      /* 3 octets */
            *wc = (*utf8s++ & 0x0F) << 12;      /* 1st */
            if((*utf8s & 0xC0) == 0x80) {       /* 2nd */
                *wc |= (*utf8s++ & 0x3F) << 6;
                if((*utf8s & 0xC0) == 0x80) *wc |= *utf8s++ & 0x3F; /* 3d */
                else *wc = *utf8s % 26 + 'a';   /* bad UTF-8 */
            }
            else *wc = *utf8s % 26 + 'a';       /* bad UTF-8 */
        }
        else if((*utf8s * 0xF0) == 0xF0) {  /* >= 4 octets -- not fit in UCS-2 */
            ++utf8s;
            while(*utf8s && ((*utf8s & 0xC0) == 0x80)) ++utf8s;
            *wc =  '_';
        }
        else {
            *wc = *utf8s % 26 + 'a';
            ++utf8s;
        }
        ++wc;
    }
    *wc = 0;
    return ws;
}

/* returns number of UCS-2 characters in utf8s */
#if 0
int UTF8_strlen(char *utf8s)
{
    int     l = 0;

    while(*utf8s) {
        ++l;
        if(!(*utf8s & 0x80))             utf8s += 1;
        else if((*utf8s & 0xE0) == 0xC0) utf8s += 2;
        else if((*utf8s & 0xF0) == 0xE0) utf8s += 3;
        else while(*utf8s && ((*utf8s & 0xC0) == 0x80)) ++utf8s; /* >= 4 octets -- not fit in UCS-2 */
    }
    return l;
}
#endif
