//some changes made to make it work with HXS
//AY

/*--[litinternal.h]------------------------------------------------------------
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

/*
 | This file contains definitions for internal routines and structures which
 | the LIT library doesn't wish to expose to the world at large
 */
#ifndef LITINTERNAL_H
#define LITINTERNAL_H

typedef struct dir_type {
    U8      * entry_ptr;
    U32     entry_size;
    U32     entry_last_chunk;
    U8      * count_ptr;
    U32     count_size;
    U32     count_last_chunk;
    int     num_entries;
    int     num_counts;
    U32     entry_aoli_idx;
    U32     total_content_size;
} dir_type;

int lit_i_read_directory(lit_file * litfile, U8 * piece, int piece_size);
int lit_i_read_headers(lit_file * litfile);
int lit_i_read_secondary_header(lit_file * litfile, U8 * hdr, int size);
int lit_i_cache_section(lit_file * litfile, section_type * pSection );
int lit_i_read_sections(lit_file * litfile );
void lit_i_free_dir_type(dir_type * dirtype);

/*
 | Utility routines...
 */
char * lit_i_strmerge(const char * first, ...);
U8 * lit_i_read_utf8_string(U8 * p, int remaining, int * size);
int lit_i_utf8_len_to_bytes(U8 * src, int lenSrc, int sizeSrc);

#endif
