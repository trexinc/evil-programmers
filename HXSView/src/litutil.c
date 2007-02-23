//some changes made to make it work with HXS
//AY

/*--[litlib.c]-----------------------------------------------------------------
 | Copyright (C) 2004 Digital Rights Software
 |
 | This file is part of the "openclit" library for processing .LIT files.
 |
 | "Openclit" is free software; you can redistribute it and/or modify
 | it under the terms of the GNU General Public License as published by
 | the Free Software Foundation; either version 2 of the License, or
 | (at your option) any later version.
 |
 | This program is distributed in the hope that it will be useful,
 | but WITHOUT ANY WARRANTY; without even the implied warranty of
 | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 | GNU General Public License for more details.
 |
 | You should have received a copy of the GNU General Public License
 | along with this program; if not, write to the Free Software
 | Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 |
 | The GNU General Public License may also be available at the following
 | URL: http://www.gnu.org/licenses/gpl.html
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "litlib.h"
#include "litinternal.h"

int lit_i_utf8_len_to_bytes(U8 * src, int lenSrc, int sizeSrc);

/*****************************************************************************/
/*--[strmerge]----------------------------------------------------------------
 | Routine to "safely" merge strings, while reporting errors on failure
 | This keeps merging strings until it finds a NULL.
 | Bad things happens to the user that forgets that final NULL!
*/
char * lit_i_strmerge(const char * first, ...)
{
    va_list ap;
    const char * s;
    char    * new, *ptr;
    int     len;

    s = first;
    len = 0;
    va_start(ap, first);
    do {
        len += strlen(s);
        s = va_arg(ap, char *);
    } while (s);

    ptr = new = (char *)malloc(len + 1);
    if (!new) {
        lit_error(0,"Unable to malloc %d bytes for a string.",len);
        return NULL;
    }
    s = first;
    va_start(ap, first);
    do {
        strcpy(ptr, s);
        ptr += strlen(ptr);
        s = va_arg(ap, char *);
    } while (s);
    *(ptr) = '\0';

    va_end(ap);
    return new;
}

/*--[lit_i_read_utf8_string]---------------------------------------------------
 |
 | This routine allocates memory for and copies an UTF8 string from the
 | current location.
*/
U8 * lit_i_read_utf8_string(U8 * p, int remaining, int * size)
{
    U8      * sNew;
    int     len, nbytes;

    len = 1;
    if (remaining < 1) goto out_of_bytes;

    len = *p;

    nbytes = lit_i_utf8_len_to_bytes((p + 1), len, remaining - 1);
    if (nbytes < 0) goto out_of_bytes;

    sNew = (U8 *)malloc(nbytes + 1);
    if (!sNew) {
        lit_error(ERR_R|ERR_LIBC,"malloc(%d) failed!",nbytes+1);
        return NULL;
    }
    if (!sNew) return NULL;
    memcpy( sNew, (p+1), nbytes);
    sNew[nbytes] = '\0';
    if (size) *size = nbytes + 1;

    return sNew;
out_of_bytes:
    lit_error(ERR_R,
"utf8 reader tried to read past an internal file boundary!\n"
"\tTried to read %d characters, but only %d bytes are left.",
        len, remaining);
    return NULL;
}

/*--[lit_i_utf8_len_to_bytes]--------------------------------------------------
 |
 | This converts a count of UTF8 elements to the amount of bytes that it
 | consumes. This is for the parsing of '/manifest' files, where the length
 | field isn't the number of bytes as might be expected.
 |
 | Most other LIT strings (such as the directory) store byte counts for
 | the length.
*/
int lit_i_utf8_len_to_bytes(U8 * src, int lenSrc, int sizeSrc)
{
    int     nchars, nbytes;
    nbytes = 0;
    nchars = 0;
    while (nchars < lenSrc) {
        /* The only thing NOT counted in lenSrc is a character 10xx xxxx */

        if ((*(src + nbytes) & 0xC0) != 0x80) nchars++;
        nbytes++;
        if (nbytes > sizeSrc) {
            return -1;
        }
    }
    /* Copy over any trailing unicode bytes */
    while (( *(src + nbytes) & 0xC0) == 0x80) {
        nbytes++;
        if (nbytes > sizeSrc) {
            return -1;
        }
    }
    return nbytes;
}
