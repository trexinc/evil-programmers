//some changes made to make it work with HXS
//AY

/*--[litheaders.c]-------------------------------------------------------------
 | Copyright (C) 2002  Dan A. Jackson
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

/* This file contains routines for reading and writing to the .LIT headers */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "litlib.h"
#include "litinternal.h"



/* LIT HEADER STRUCTURE *****************************************************
     +-----------------------------+
     |                             |
     | Primary Header              |
     |                             |
     +-----------------------------+
     | Piece Structures            |
     | -- Offset                   |*-------------------+
     | -- Length                   |*---------------+   |
     |                             |*-----------+   |   |
     |                             |*-------+   |   |   |
     |                             |*---+   |   |   |   |
     +-----------------------------+    |   |   |   |   |
     |                             |    |   |   |   |   |
     | Secondary Header            |    |   |   |   |   |
  +-*|   (Content Length)          |    |   |   |   |   |
  |  +-----------------------------+    |   |   |   |   |
  |                                     |   |   |   |   |
  |                                     |   |   |   |   |
  |  +-----------------------------+    |   |   |   |   |
  |  | Piece 0 Data                |<---+   |   |   |   |
  |  |   Total File Length         |        |   |   |   |
  |  +-----------------------------+        |   |   |   |
  |                                         |   |   |   |
  |                                         |   |   |   |
  |  +-----------------------------+        |   |   |   |
  |  | Piece 1 Data                |<-------+   |   |   |
  |  |   Internal Directory        |            |   |   |
  |  |                             |            |   |   |
  |  +-----------------------------+            |   |   |
  |                                             |   |   |
  |                                             |   |   |
  |  +-----------------------------+            |   |   |
  |  | Piece 2 Data                |<-----------+   |   |
  |  |   Index information for     |                |   |
  |  |   internal directory        |                |   |
  |  +-----------------------------+                |   |
  |                                                 |   |
  |                                                 |   |
  |  +-----------------------------+                |   |
  |  | Piece 3 Data                |<---------------+   |
  |  |   GUID (0A9007C3-7640-D311- |                    |
  |  |         87890000F8105754}   |                    |
  |  +-----------------------------+                    |
  |                                                     |
  |  +-----------------------------+                    |
  |  | Piece 4 Data                |<-------------------+
  |  |   GUID (0A9007C4-7640-D311- |
  |  |         87890000F8105754}   |
  |  +-----------------------------+
  |
  |
  +->+-----------------------------+
     | Section 0                   |
     |                             |
     .                             .
     . ... remainder of the file   .
     .                             .
     |                             |
     |                             |
     +-----------------------------+

**************************************************************************/

/** Primary Header *******************************************************
 +-------------------------------+--------------------------------+
 |                     'ITOLITLS'  signature                      |
 +-------------------------------+--------------------------------+
 | Version                       | Primary Header Length          |
 +-------------------------------+--------------------------------+
 | Number of Pieces              | Secondary Header Length        |
 +-------------------------------+--------------------------------+
 | File GUID (0A9007C1-7640-D311-87890000F8105754}                |
 +                                                                +
 |                                                                |
 +-------------------------------+--------------------------------+
**************************************************************************/

#define PRI_VERSION     8
#define PRI_HDRLEN      12
#define PRI_NUMPIECES   16
#define PRI_SECHDRLEN   20
#define PRI_GUID        24
#define PRI_SIZE        40

static const char * lit_magic_string = "ITOLITLS";

/** Piece structures ******************************************************
 +----------------------------------------------------------------+
 | Piece Offset (Low Dword)      . Piece Offset (High Dword)      |
 +----------------------------------------------------------------+
 | Piece Size (Low Dword)        . Piece Size   (High Dword)      |
 +----------------------------------------------------------------+
**************************************************************************/
#define PIECE_LOW_OFFSET    0
#define PIECE_HIGH_OFFSET   4
#define PIECE_LOW_SIZE      8
#define PIECE_HIGH_SIZE     12
#define PIECE_SIZE          16

/** PIECE 0 of the header -- File size ***********************************
 +-------------------------------+--------------------------------+
 | 0x1FE (Tag)                   | 0 (Version)                    |
 +-------------------------------+--------------------------------+
 | File Length                                                    |
 +-------------------------------+--------------------------------+
 | 0 -- Unknown                                                   |
 +----------------------------------------------------------------+

Version 2 -- Speculation only, because I have no idea how to get the
"real" file length from this version.
 +-------------------------------+--------------------------------+
 | 0x1FE (Tag)                   | 2 (Version)                    |
 +-------------------------------+--------------------------------+
 | Content Offset (? Could be a coincidence !)                    |
 +-------------------------------+--------------------------------+
 | 0 -- Unknown                                                   |
 +----------------------------------------------------------------+
 | Offset (points to a version 0, 0x1FE block but smaller length  |
 +----------------------------------------------------------------+
 | Size (0x18), normal for verison 0, 0x1FE block                 |
 +----------------------------------------------------------------+
 | Offset (points to an **empty** IFCM block)                     |
 +----------------------------------------------------------------+
 | Size (0x20), minimum size for an IFCM block)                   |
 +----------------------------------------------------------------+

*************************************************************************/

#define SIZEPIECE_TAG       0x1FE
#define SIZEPIECE_SIZE      24
#define SIZEPIECE_VERSION   4
#define SIZEPIECE_FILESIZE  8


/*** COMMON BLOCK FORMAT **************************************************

 +-------------------------------+--------------------------------+
 | Block Type                    | Block Version                  |
 +-------------------------------+--------------------------------+
 | Block Size (bytes)            |
 +-------------------------------+
*************************************************************************/
#define LIT_BLOCK_TAG   0
#define LIT_BLOCK_VER   4
#define LIT_BLOCK_LEN   8


/*--[lit_i_read_headers]-------------------------------------------------------
 | Here I read the header area of the .LIT file into the working structure.
 | The main information extracted from the header:
 |      Content Offset
 |      Directory Table
 |      Total file length
 |
*/
int lit_i_read_headers(lit_file * litfile)
{
    U8      * header = NULL, * cur_piece = NULL;
    int     r;
    int     i, total_hdr_size;

    /* Primary Header Fields */
    U32     version;
    int     sec_hdr_len, hdr_len, num_pieces;
    /* Piece information */
    U32     offset;
    int     size;

    if (litfile->filesize < PRI_SIZE) {
        lit_error(ERR_R,"LIT FORMAT ERROR: File is too small. "
            "Is %d bytes, must be at least %d bytes!", litfile->filesize,
            PRI_SIZE);
        return E_LIT_FORMAT_ERROR;
    }

    header = litfile->readat(litfile->file_pointer, 0, PRI_SIZE);
    if (!header) {
        return E_LIT_READ_ERROR;
    }

    if (strncmp(header, lit_magic_string, strlen(lit_magic_string)) != 0) {
        lit_error(ERR_R,"This file does not appear to be a .LIT file.");
        free(header);
        return E_LIT_FORMAT_ERROR;
    }

    version = READ_U32(header+PRI_VERSION);
    hdr_len = READ_INT32(header+PRI_HDRLEN);
    num_pieces = READ_INT32(header+PRI_NUMPIECES);
    sec_hdr_len = READ_INT32(header+PRI_SECHDRLEN);
    if (version != 1) {
        lit_error(ERR_R,
            "I only handle version 1, this file is version %d.",
            (int)version);
        free(header);
        return E_LIT_NEWER_VERSION;
    }
    memcpy(litfile->header_guid, header + PRI_GUID, 16);

    free(header);
    header = NULL;

    total_hdr_size = hdr_len + num_pieces*PIECE_SIZE + sec_hdr_len;
    if ((size_t)total_hdr_size >= litfile->filesize) {
        lit_error(ERR_R, "LIT FORMAT ERROR: File is too small for header.");
        return E_LIT_FORMAT_ERROR;
    }

    header = litfile->readat(litfile->file_pointer, 0, total_hdr_size);
    if (!header) return E_LIT_READ_ERROR;
    r = lit_i_read_secondary_header(litfile,
        header+hdr_len+num_pieces*PIECE_SIZE, sec_hdr_len);
    if (r) {
        free(header);
        return r;
    }
    for (i = 0; i < num_pieces; i++)
    {
        if ( READ_U32(header + hdr_len + i*PIECE_SIZE + PIECE_HIGH_OFFSET) ||
             READ_U32(header + hdr_len + i*PIECE_SIZE + PIECE_HIGH_SIZE) ) {
            lit_error(ERR_R," Piece %i has 64bit value -- Offset: %lx:%lx, "
                "Size: %lx:%lx", i,
                READ_U32(header + hdr_len +i*PIECE_SIZE+PIECE_HIGH_OFFSET),
                READ_U32(header + hdr_len +i*PIECE_SIZE+PIECE_LOW_OFFSET),
                READ_U32(header + hdr_len +i*PIECE_SIZE+PIECE_HIGH_SIZE),
                READ_U32(header + hdr_len +i*PIECE_SIZE+PIECE_LOW_SIZE));
            free(header);
            return E_LIT_64BIT_VALUE;
        }
        offset = READ_U32(header + hdr_len + i*PIECE_SIZE + PIECE_LOW_OFFSET);
        size = READ_INT32(header + hdr_len + i*PIECE_SIZE + PIECE_LOW_SIZE);

        if ( (offset+size) > litfile->filesize) {
            lit_error(ERR_R,"LIT FORMAT ERROR:"
                "PIECE %d (Ofs:%lx Size %d) is outside length %d!",
                i, offset, size, (int)litfile->filesize);
            free(header);
            return E_LIT_FORMAT_ERROR;
        }
        cur_piece = litfile->readat(litfile->file_pointer, offset, size);
        if (!cur_piece) {
            free(header);
            return E_LIT_READ_ERROR;
        }
        switch (i)
        {
        case 0:
/* Handling this is too much bother,
 * Version 2 of the header appears to have a second level of piece tables
 * which is just more trouble than it is worth.
 * "freeLIT" can create files with version 2 for those interested.
 * Since I already know what the length SHOULD be (unless someone mangled
 * the file with an ASCII ftp transfer, and that would break other things),
 * there is really little reason YET to process this piece.  */
#if 0
            if ((size < SIZEPIECE_SIZE) ||
                (READ_U32(cur_piece) != SIZEPIECE_TAG))
            {
                lit_error(ERR_R,"LIT FORMAT ERROR:"
                    "PIECE %d has invalid size (%d) or tag value (%08lx).",
                    i, size, READ_U32(cur_piece));
                free(header);
                free(cur_piece);
                return E_LIT_FORMAT_ERROR;
            }
            if (READ_U32(cur_piece + SIZEPIECE_FILESIZE + 4)) {
                lit_error(ERR_R,"LIT FILE HAS 64-BIT FILE SIZE:  %08lx:%08lx!",
                    READ_U32(cur_piece+SIZEPIECE_FILESIZE+4),
                    READ_U32(cur_piece+SIZEPIECE_FILESIZE));
                free(header);
                free(cur_piece);
                return E_LIT_64BIT_VALUE;
            }
            filesize = READ_U32(cur_piece + SIZEPIECE_FILESIZE);

            /* Having more bytes isn't a problem, at least not for
             * this code. MSREADER doesn't like it though.  */
            if (filesize < litfile->filesize) {
                lit_error(ERR_R,"LIT FILE ERROR - "
                    "File thinks it has %d bytes, but I really have %d.",
                    (int)filesize, litfile->filesize);
                free(header);
                free(cur_piece);
                return E_LIT_FORMAT_ERROR;
            }
#endif

            break;
        case 1:
            if ((READ_U32(cur_piece + 8)!= litfile->entry_chunklen) ||
                (READ_U32(cur_piece + 12)!=litfile->entry_unknown) ) {
                lit_error(ERR_R,"LIT FORMAT ERROR: Secondary header is wrong."
                    "(SecHdr) Len: %lx, Unk: %lx  (Piece) Len: %lx Unk: %lx\n",
                    litfile->entry_chunklen, litfile->entry_unknown,
                    READ_U32(cur_piece + 8),
                    READ_U32(cur_piece + 12));
                return E_LIT_FORMAT_ERROR;
            }
            r = lit_i_read_directory(litfile,cur_piece,size);
            if (r) {
                free(header); free(cur_piece);
                return r;
            }
            break;
        case 2:
            if ((READ_U32(cur_piece + 8)!= litfile->count_chunklen) ||
                (READ_U32(cur_piece + 12)!=litfile->count_unknown) ) {
                lit_error(ERR_R,"LIT FORMAT ERROR: Secondary header is wrong."
                    "(SecHdr) Len: %lx, Unk: %lx  (Piece) Len: %lx Unk: %lx\n",
                    litfile->count_chunklen, litfile->count_unknown,
                    READ_U32(cur_piece + 8),
                    READ_U32(cur_piece + 12));
                return E_LIT_FORMAT_ERROR;
            }
            /* There is no information I need from this piece */
            break;
        case 3:
            if (size != sizeof(litfile->piece3_guid)) {
                lit_error(ERR_R,"LIT FORMAT ERROR: Piece %d's size is %d, "
                    "expected %d!",i,size, sizeof(litfile->piece3_guid));
                free(header);
                free(cur_piece);
                return E_LIT_FORMAT_ERROR;
            }
            memcpy(litfile->piece3_guid, cur_piece, size);
            break;
        case 4:
            if (size != sizeof(litfile->piece4_guid)) {
                lit_error(ERR_R,"LIT FORMAT ERROR: Piece %d's size is %d, "
                    "expected %d!",i,size, sizeof(litfile->piece4_guid));
                free(header);
                free(cur_piece);
                return E_LIT_FORMAT_ERROR;
            }
            memcpy(litfile->piece4_guid, cur_piece, size);
            break;
        }
        free(cur_piece);
        cur_piece = NULL;
    }
    free(header);
    header = NULL;

    return 0;
}

/** SECONDARY HEADER FORMAT *********************************************

 Mostly in "COMMON BLOCK FORMAT"

 +-------------------------------+--------------------------------+
 | Block Type                    | Block Version                  |
 +-------------------------------+--------------------------------+
 | Block Size (bytes)            |
 +-------------------------------+

 Specifically (Note the first block is missing the type... )

 +-------------------------------+--------------------------------+
 |  Version (2)                  | Block Length (0x98)            |
 +-------------------------------+--------------------------------+ +8
 | Chunk Index of first "AOLI" or "Index Chunk" in FILE           |
 | directory   (0xFFFFFFFF if no index chunk is present)          |
 +----------------------------------------------------------------+ +16
 | 0 -- Unknown                                                   |
 +----------------------------------------------------------------+ +24
 | Chunk index of LAST "AOLL" or "List Chunk" in FILE directory   |
 +----------------------------------------------------------------+ +32
 | 0 -- Unknown                                                   |
 +-------------------------------+--------------------------------+ +40
 | Chunk Size of List Chunks (?) | Unknown (always 2)             |
 | in FILE directory (0x2000)    |                                |
 +-------------------------------+--------------------------------+ +48
 | 0 -- Unknown                  | Depth of FILE directory        |
 |                               | (2 with an AOLI, 1 normally)   |
 +-------------------------------+--------------------------------+ +56
 | 0 -- Unknown                                                   |
 +-------------------------------+--------------------------------+ +64
 | Total number of FILE entries  | 0 -- Unknown                   |
 +-------------------------------+--------------------------------+ +72
 | Chunk Index of first "AOLI" or "Index Chunk" in ENTRY COUNTS   |
 | directory   (0xFFFFFFFF if no index chunk is present)          |
 +----------------------------------------------------------------+ +80
 | 0 -- Unknown                                                   |
 +----------------------------------------------------------------+ +88
 | Chunk index of LAST "AOLL" or "List Chunk" in COUNTS directory |
 +----------------------------------------------------------------+ +96
 | 0 -- Unknown                                                   |
 +-------------------------------+--------------------------------+ +104
 | Chunk Size of List Chunks (?) | Unknown (always 2)             |
 | in COUNTS directory (0x200)   |                                |
 +-------------------------------+--------------------------------+ +112
 | 0 -- Unknown                  | Depth of COUNTS directory      |
 |                               | (2 with an AOLI, 1 normally)   |
 +-------------------------------+--------------------------------+ +120
 | 0 -- Unknown                                                   |
 +-------------------------------+--------------------------------+ +128
 | Total number of COUNTS entries| 0 -- Unknown                   |
 +-------------------------------+--------------------------------+ +136
 | 0x100000 - Unknown, same as in| 0x20000 -- Unknown, same as in |
 | first IFCM of FILE directory  | first IFCM of COUNTS directory |
 +-------------------------------+--------------------------------+ +144
 | 0 -- Unknown                                                   |
 +----------------------------------------------------------------+ +152
 | 'CAOL' - Block Tag            | Block version (2)              |
 +-------------------------------+--------------------------------+
 | Block Length (0x50)           | Unknown (? - MSREADER IGNORES) |
 |                               | (may be a creator ID?)         |
 +-------------------------------+--------------------------------+
 | 0 -- Unknown                  | 0x2000 (Chunk size of Entries) |
 +-------------------------------+--------------------------------+
 | 0x200 (Chunk size of COUNTS)  | 0x100000 (Unknown, see above)  |
 +-------------------------------+--------------------------------+
 | 0x20000 (Unknown, like above) | 0 -- Unknown                   |
 +-------------------------------+--------------------------------+
 | 0 -- Unknown                                                   |
 +-------------------------------+--------------------------------+
 | 'ITSF' - Block Tag            | Block version (4)              |
 +-------------------------------+--------------------------------+
 | Block Length (0x20)           | 0x01 (Unknown)                 |
 +-------------------------------+--------------------------------+
 | Content Offset                                                 |
 +-------------------------------+--------------------------------+
 | Timestamp (?)                 | Windows Language ID (0x409)    |
 +-------------------------------+--------------------------------+

*********************************************************************/

#define SECHDR_BLOCK_VER         0
#define SECHDR_BLOCK_LEN         4
#define SECHDR_ENTRY_AOLI_IDX    8
#define SECHDR_ENTRY_LAST_AOLL  24
#define SECHDR_ENTRY_CHUNKLEN   40
#define SECHDR_ENTRY_TWO        44
#define SECHDR_ENTRY_DEPTH      52
#define SECHDR_ENTRY_ENTRIES    64
#define SECHDR_COUNT_AOLI_IDX   72
#define SECHDR_COUNT_LAST_AOLL  88
#define SECHDR_COUNT_CHUNKLEN  104
#define SECHDR_COUNT_TWO       108
#define SECHDR_COUNT_DEPTH     116
#define SECHDR_COUNT_ENTRIES   128
#define SECHDR_ENTRY_UNKNOWN   136
#define SECHDR_COUNT_UNKNOWN   140

#define SECHDR_VERSION          2
#define SECHDR_SIZE             152

#define CAOL_TAG                0x4c4f4143
#define CAOL_SIZE               48
#define CAOL_VERSION            2

#define CAOL_CREATOR_ID         12
#define CAOL_ENTRY_CHUNKLEN     20
#define CAOL_COUNT_CHUNKLEN     24
#define CAOL_ENTRY_UNKNOWN      28
#define CAOL_COUNT_UNKNOWN      32

#define ITSF_TAG                0x46535449
#define ITSF_VERSION             4
#define ITSF_SIZE               32
#define ITSF_UNKNOWN            12
#define ITSF_CONTENT_OFFSET     16
#define ITSF_TIMESTAMP          24
#define ITSF_LANGUAGE_ID        28


/*--[lit_i_read_secondary_header]----------------------------------------------
 |
 | Reads useful fields from the secondary header into the lit_file block.
 | This processes the secondary header as if it were made up of subblocks,
 | each with their own tag, version and length.  This may not be correct for
 | future versions.
 |
 | Note that a lot of these fields appear redundant, and are unnecessary
 | in processing .LIT files.  Possibly these are vestiges of the storage
 | layer, or have some meaning on occasional files which haven't been
 | observed yet.
*/
int lit_i_read_secondary_header(lit_file * litfile, U8 * hdr, int len)
{
    int  blocklen, offset;
    U32  blocktype, blockver;

    litfile->content_offset = 0;

    /* All the fields in the main header are unnecessary from the simplistic
     * directory parsing methodology.
     * These include the offsets to the index chunks for faster directory
     * processing, but with less than a hundred entries in most cases, there
     * is really little point. */
    /* Note special case -- TAG is missing, so use -4 */
    offset = READ_INT32(hdr+LIT_BLOCK_LEN - 4);
    blocktype = 0;
    while (offset < len) {
        blocktype = READ_U32(hdr+offset+LIT_BLOCK_TAG);
        blockver = READ_U32(hdr+offset+LIT_BLOCK_VER);
        blocklen = READ_INT32(hdr+offset+LIT_BLOCK_LEN);
        if (blocklen < 0) break;

        switch (blocktype)
        {
        case CAOL_TAG:
            if (blockver != 2) {
                lit_error(ERR_R, "This file is a newer LIT format -"
                    "I expected CAOL version 2, this is %ld!",
                    (U32)blockver);
                return E_LIT_NEWER_VERSION;
            }
            litfile->creator_id = READ_U32(hdr+offset+CAOL_CREATOR_ID);
            litfile->entry_chunklen = READ_U32(hdr+offset+CAOL_ENTRY_CHUNKLEN);
            litfile->count_chunklen = READ_U32(hdr+offset+CAOL_COUNT_CHUNKLEN);
            litfile->entry_unknown  = READ_U32(hdr+offset+CAOL_ENTRY_UNKNOWN);
            litfile->count_unknown  = READ_U32(hdr+offset+CAOL_COUNT_UNKNOWN);

            /* Cheat - point to the embedded ITSF header */
            blocklen = 48;
            break;

        case ITSF_TAG:
            if (blockver != 4) {
                lit_error(ERR_R, "This file is a newer LIT format -"
                    " I expected ITSF version 4, this is %ld!",
                    blockver);
                return E_LIT_NEWER_VERSION;
            }
            if (READ_U32(hdr+offset+ITSF_CONTENT_OFFSET+4)) {
                lit_error(ERR_R, "This file has a 64-bit content offset!");
                return E_LIT_64BIT_VALUE;
            }
            litfile->timestamp      = READ_U32(hdr+offset+ITSF_TIMESTAMP);
            litfile->language_id    = READ_U32(hdr+offset+ITSF_LANGUAGE_ID);
            litfile->content_offset = READ_U32(hdr+offset+ITSF_CONTENT_OFFSET);
            break;
        }
        offset += blocklen;
    }

    if (offset != len) {
        lit_error(ERR_R,"Secondary header offset %ld "
            "ended up outside length %ld.", (U32)offset, (U32)len);
        return E_LIT_FORMAT_ERROR;
    }

    if (litfile->content_offset == 0) {
        lit_error(ERR_R,"I was unable to figure out the content offset!");
        return E_LIT_FORMAT_ERROR;
    }
    return 0;
}
