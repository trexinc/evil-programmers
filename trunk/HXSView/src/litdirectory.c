//some changes made to make it work with HXS
//AY

/*--[litdirectory.c]-----------------------------------------------------------
 | Copyright (C) 2002 Dan A. Jackson
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
#include <stdlib.h>
#include <string.h>
#include "litlib.h"
#include "litinternal.h"

/* This file contains routines for reading and writing to directory
 | structures.
 */


static void read_encint(U8 **handle, int *remaining, U64 *value);
static entry_type * read_entry(U8 **handle, int *remaining);


/*--[read_encint]--------------------------------------------------------------
 |
 | This reads an encoded integer from the data stream.
 |   - ENCINT data type is a variable length integer, where the MSB
 |   indicates if another byte is needed and the lower 7 bits represent
 |   the data.
 |
 |   For Example:
 |                          +-+-------------+
 |       0x7F stays 0x7F    |0|1 1 1 1 1 1 1|
 |                          +-+-------------+
 |
 |                             +-+-------------+   +-+-------------+
 |      0xFE becomes 0xFF 0x00 |1|1 1 1 1 1 1 1|   |0|0 0 0 0 0 0 0|
 |                             +-+-------------+   +-+-------------+
 |
*/
void read_encint(U8 **handle, int *remaining, U64 *value)
{
    U8   b, *p;

    p = *handle;
    *value = 0;
    while (*remaining > 0) {
        b = *(p++);
        (*remaining)--;
        (*value) <<= 7;
        (*value) |= (b & 0x7f);
        if (!(b & 0x80)) break;
    }
    *handle = p;
}

/*--[write_encint]-------------------------------------------------------------
 |
 | Store an encoded integer into a location.
*/
int write_encint(U8 * p,int index,int total,U64 x)
{
    int     len,i;
    U64     mask;

    len = 0;
    mask = 0x7f;
    while (x > mask) { len++; mask = (mask << 7)|0x7f;  }
    len++;
    if (len > (total - index)) return -1;

    for (i = 1; i < len; i++)
    {
        *(p+index) = 0x80 + (U8)((x & mask) >> ((len - i) * 7));
        mask >>= 7;
        index++;
    }
    *(p+index) = (U8)((x & mask) &0xff);
    return len;
}


/*--[read_entry]---------------------------------------------------------------
 |
 | This routine reads an entry given a pointer and the number of bytes
 | remaining.
 |
 | A directory entry is:
 |   ENCODED_INTEGER      name_length
 |   BYTES                name
 |   ENCODED_INTEGER      section
 |   ENCODED_INTEGER      offset
 |   ENCODED_INTEGER      size
 |
*/
entry_type * read_entry(U8 **handle, int *remaining)
{
    int    namelen;
    U64    long_namelen;
    U8     * nameptr;
    entry_type * entry;

    read_encint(handle, remaining, &long_namelen);
    namelen = (int)(long_namelen & 0x7fffffff);
    if (long_namelen != namelen) {
        lit_error(ERR_R,"Directory entry had 64-bit name length!");
        return NULL;
    }
    nameptr = *handle;
    if ((namelen + 3) <= *remaining) {
        (*handle)+= namelen;
        *remaining -= namelen;
    } else {
        lit_error(ERR_R,"Read past end of directory chunk!");
        return NULL;
    }

    entry = malloc(sizeof(entry_type)+namelen + 1);
    if (!entry) {
        lit_error(ERR_LIBC,"malloc(%d) failed!", sizeof(entry_type)+namelen+1);
        return NULL;
    }
    entry->next = NULL;
    entry->data_source = ENTRY_SOURCE_LITFILE;
    entry->data_pointer = NULL;
    entry->namelen = namelen;
    memcpy(&entry->name, nameptr, namelen);
    /* Avoid warnings about going past the end of an array */
    *( (U8 *)(&entry->name) + namelen) = '\0';
    read_encint(handle,remaining,&entry->section);
    read_encint(handle,remaining,&entry->offset);
    read_encint(handle,remaining,&entry->size);
    return entry;
}

/*** DIRECTORY STRUCTURES *****************************************************

    +-----------------------------+
    |                             |
    | Directory header            |
    |                             |
    +-----------------------------+
    |                             |
    | Directory Entries ---->     |
    |                             |
    +-----------------------------+
    |                             |
    |                             |
    |    <---- QuickRef Entries   |
    +-----------------------------+

 +-------------------------------+--------------------------------+
 | Block Tag "IFCM"              | Version (typically 1)          |
 +-------------------------------+--------------------------------+
 | "Chunk Size"                  | Unknown (0x100000 or 0x20000)  |
 +-------------------------------+--------------------------------+
 | 0xFFFFFFFF 0xFFFFFFFF (Unknown)                                |
 +-------------------------------+--------------------------------+
 | Number of chunks                                               |
 +----------------------------------------------------------------+


 +-------------------------------+--------------------------------+
 | Second Tag "AOLL"             | Number of bytes after entries  |
 +-------------------------------+--------------------------------+
 | Current chunk id (0 based)                                     |
 +----------------------------------------------------------------+
 | Previous AOLL chunk id                                         |
 +----------------------------------------------------------------+
 | Next AOLL chunk id (0 based)                                   |
 +----------------------------------------------------------------+
 | Entries so far                                                 |
 +----------------------------------------------------------------+
 | 1 (Distance in chunks to next)                                 |
 +----------------------------------------------------------------+
 | Directory entries
 +-------------------------------.....

******************************************************************************/
#define IFCM_TAG        0x4D434649
#define AOLL_TAG        0x4C4C4F41
#define AOLI_TAG        0x494C4F41


/*--[lit_i_read_directory]-----------------------------------------------------
 |
 | This reads the directory chunks from the piece and converts the list into
 | entry elements in the lit_file structure.
 |
*/
int lit_i_read_directory(lit_file * litfile, U8 * piece, int piece_size)
{
    U8      * p = NULL;
    int     nEntries, nRemaining, nChunks, chunk, idx, sizeChunk;
    entry_type * entry, * prev;

    if (!piece || (READ_U32(piece) != IFCM_TAG)) {
        lit_error(ERR_R,
           "Header Piece #3 is not the main directory! (TAG=%08lx)",
           (piece)?(READ_U32(piece)):0);
        return E_LIT_FORMAT_ERROR;
    }
    sizeChunk = READ_INT32(piece + 8);
    nChunks = READ_INT32(piece + 24);

    if ((32 + (nChunks * sizeChunk)) !=  piece_size) {
        lit_error(ERR_R, "IFCM HEADER (%d chunks of %d bytes) != %d total.",
        nChunks, sizeChunk, piece_size - 32);
        return E_LIT_FORMAT_ERROR;
    }
    prev = NULL;
    for (chunk = 0; chunk < nChunks; chunk++)
    {
        p = piece + 32 + (chunk * sizeChunk);

        /* This can either be AOLL or AOLI.
         * AOLI isn't useful for reading */
        if (READ_U32(p) != AOLL_TAG)  continue;

        nRemaining = READ_INT32(p + 4);
        if (nRemaining >= sizeChunk) {
            lit_error(ERR_R,
"AOLL remaining count is NEGATIVE! (%d of %d) %x\n",
            (int)nRemaining, (int)sizeChunk, (int)nRemaining);
            return E_LIT_FORMAT_ERROR;
        }
        nRemaining = sizeChunk - (nRemaining + 48);

        nEntries = READ_U16(p + sizeChunk - 2);

        /* Sometimes, the nEntries doesn't get written. When this happens,
         * I don't know how many to read.  Fortunately, there is "nRemaining",
         * and if everything is working fine, read_entry will consume JUST
         * enough bytes */
        if (!nEntries) nEntries = 65535;

        p += 48;
        if (nRemaining < 0) return E_LIT_FORMAT_ERROR;
        for (idx = 0; idx < nEntries; idx ++) {
            if (nRemaining <= 0) break;
            entry = read_entry(&p, &nRemaining);
            if (!entry) {
                return E_LIT_FORMAT_ERROR;
            }
            if (!prev) {
                litfile->entry = entry;
                prev = entry;
            } else {
                prev->next = entry;
                prev = entry;
            }
        }
    }
    return 0;
}

/*--[lit_i_free_dir_type]------------------------------------------------------
 |
 | releases memory associated with a dir_type
 |
*/
void lit_i_free_dir_type(dir_type * dirtype)
{
    if (dirtype->entry_ptr) free(dirtype->entry_ptr);
    if (dirtype->count_ptr) free(dirtype->count_ptr);
    memset(dirtype, 0, sizeof(dir_type));
}
