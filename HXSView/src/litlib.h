//some changes made to make it work with HXS
//AY

/*****************************************************************************/ /*--[litlib.h]-----------------------------------------------------------------
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

#ifndef LITLIB_H
#define LITLIB_H

#include "littypes.h"


/* This function supplied by the client */
#define ERR_RW 3
#define ERR_R  1
#define ERR_W  2

#define ERR_LIBC  0x0080
#define ERR_WIN32 0x0100

//void lit_error(int what, char * fmt, ...);
#define lit_error(what,fmt,...) (void)0

/* Return value is a "free-able" block of memory */
typedef U8* (* READFCN)(void *,U32, int);

#define ENTRY_SOURCE_INVALID    0
#define ENTRY_SOURCE_LITFILE    1
#define ENTRY_SOURCE_POINTER    2

typedef struct entry_type
{
    struct  entry_type * next;

    /* Where this data is coming from */
    int     data_source;

    /* This means different things depending on what the source is --
     * if the source is ENTRY_SOURCE_LITFILE, then this is either NULL
     * (ie, this is in the current LIT file), or points to a different
     * "lit_file" structure, in which case, reading this entry involves a
     * recursive call with the name with THAT lit_file structure.
     *
     * For ENTRY_SOURCE_POINTER this points directly to the data */
    void    * data_pointer;

    /* What section does this entry belong to. */
    U64     section;

    /* Offset into current file */
    U64     offset;

    /* Byte count of data (not necessarily valid for "ENTRY_SOURCE_LITFILE" */
    U64     size;

    /* Size of name (not including trailing zero */
    int     namelen;

    /* First character of null terminated name. */
    U8      name;

} entry_type;


#define MAX_SECTION_NAME    128
typedef struct section_type
{
    U8     * data_pointer;
    int    size;
    U8     name[MAX_SECTION_NAME+1];
} section_type;

typedef struct lit_file
{
    /* Abstract pointer to file information, passed into "open" */
    void        * file_pointer;

    /* Function pointer to read from a particular location */
    READFCN     readat;

    /* Total size of the file in bytes */
    U32         filesize;

    /* Offset into the file where content starts */
    U32         content_offset;

    /* Number of entries in the internal directory structure */
    int         num_file_entries;
    /* Pointer to first entry (typically '/') in the internal
     * directory structure */
    entry_type  * entry;

    /* Number of "sections" in the name list --
     * Typically there will be 4 sections, which are
     *   0 -  Uncompressed    (LIT format files are in this section)
     *   1 -  MSCompressed    (Not used for these files)
     *   2 -  EbEncryptDS     (The ebook files themselves)
     *   3 -  EbEncryptOnlyDS (Encryption validation strings, mainly)
     */
    int     num_sections;
    section_type * sections;

    /* GUIDs from the header */
    U8      header_guid[16];
    U8      piece3_guid[16];
    U8      piece4_guid[16];

    /* LIT fields that are mostly fixed.. */
    U32     entry_chunklen; /* usually 0x2000 */
    U32     count_chunklen; /* Usually 0x200  */

    /* Lit informational fields, filled in by a lit creator... */
    U32     language_id;


    /* These fields are uncertain, and will almost definitely be
     * renamed at some point in the future */
    U32     creator_id;
    U32     entry_unknown;
    U32     count_unknown;
    /* Almost certainly wrong! */
    U32     timestamp;
} lit_file;

#define E_LIT_BAD_STRUCT        -2000
#define E_LIT_INTERNAL_ERROR    -2001
#define E_LIT_OUT_OF_MEMORY     -2002
#define E_LIT_NULL_POINTER      -2003

#define E_LIT_READ_ERROR        -2010
#define E_LIT_LZX_ERROR         -2011
#define E_LIT_WRITE_ERROR       -2012

#define E_LIT_FILE_NOT_FOUND    -2030

#define E_LIT_FORMAT_ERROR      -2100
#define E_LIT_NEWER_VERSION     -2110
#define E_LIT_64BIT_VALUE       -2120
#define E_LIT_DECRYPT_FAIL      -2130
#define E_LIT_BAD_UTF8          -2140
#define E_LIT_NOT_ENOUGH_SPACE  -2150
#define E_LIT_NOT_ENOUGH_BYTES  -2160

#define E_LIT_UNSUPPORTED       -2500

#define E_LIT_DRM_ERROR         -2600

int lit_close(lit_file * litfile);
int lit_read_from_file(lit_file * litfile);
int lit_get_file(lit_file * litfile, const char * name, U8 ** ptr, int * psize);
#endif
