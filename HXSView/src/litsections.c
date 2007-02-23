//some changes made to make it work with HXS
//AY

/*--[litsections.c]------------------------------------------------------------
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

/* This file contains the code to read and manage "sections"
 |
 | Sections are second-level files-within-a-file-within-a-file structures,
 | so that multiple internal files can be packed together to acheive better
 | compression.  */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "litlib.h"
#include "litinternal.h"
#include "lzx.h"

static int decompress_section(lit_file * litfile, char * section_name,
    U8 * pControl, int sizeControl, U8 * pContent, int sizeContent,
    U8 ** ppUncompressed, int * psizeUncompressed);
static const char * guid2string(U8 * guid);

/* Constant names */
const char * content_tail   = "/Content";
const char * control_tail   = "/ControlData";
const char * namelist_string = "::DataSpace/NameList";
const char * storage_string = "::DataSpace/Storage/";
const char * transform_string = "/Transform/List";
const char * rt_tail  = "/InstanceData/ResetTable";
//const char * desencrypt_guid = "{67F6E4A2-60BF-11D3-8540-00C04F58C3CF}";
const char * lzxcompress_guid= "{0A9007C6-4076-11D3-8789-0000F8105754}";
const U32 LZXC_TAG = 0x43585a4c;


/*--[guid2string]--------------------------------------------------------------
 |
 | Careful - uses a static string.  Will be awkward if called the same
 | time in a statement.
 | Assumes GUID is in Little Endian Order
 |
*/
const char * guid2string(U8 * guid)
{
    static char guid_buffer[7+8+4+4+4+12+1];

    sprintf(guid_buffer, "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        (U32)READ_U32(guid), (U16)READ_U16((guid+4)),
        (U16)READ_U16((guid+6)),
        (U8)*(guid+8), (U8)*(guid+9), /*-*/
        (U8)*(guid+10), (U8)*(guid+11), (U8)*(guid+12),
        (U8)*(guid+13), (U8)*(guid+14),
        (U8)*(guid+15));
   return guid_buffer;
}



/*****************************************************************************

 Encryption ControlData
 +-------------------------------+--------------------------------+
 | Number of dwords following (3)| 0x1729  (Unknown, Tag?)        |
 +-------------------------------+--------------------------------+
 | 1  (Unknown, Constant)        | 0xA5A5  (Unknown, Constant)    |
 +-------------------------------+--------------------------------+

 Compression ControlData
 +-------------------------------+--------------------------------+
 | Number of dwords (always 7)   | 'LZXC'  Tag                    |
 +-------------------------------+--------------------------------+
 | 3 (Unknown, Constant)         | Encoded Window size (15 - 21)  |
 +-------------------------------+--------------------------------+
 | Same as encoded window size   | 0x02 (Unknown - Always constant|
 | See CHMFORMAT for speculation.|                                |
 +-------------------------------+--------------------------------+
 | 0 (Unknown)                                                    |
 +-------------------------------+--------------------------------+

 Reset Table
 +-------------------------------+--------------------------------+
 | Version (3)                   | Number of reset table entries  |
 +-------------------------------+--------------------------------+
 | Unknown (always 8)            | Header length (0x28)           |
 +-------------------------------+--------------------------------+
 | Uncompressed length                                            |
 +-------------------------------+--------------------------------+
 | Compressed length (or close)                                   |
 +-------------------------------+--------------------------------+
 | Reset Interval                | Padding                        |
 +-------------------------------+--------------------------------+
 | Reset location (0)                                             |
 +----------------------------------------------------------------+
 ... Successive reset locations follow ....



 ****************************************************************************/
#define LZXC_TAG 0x43585a4c

#define CONTROL_TAG              4
#define CONTROL_WINDOW_SIZE     12

#define RESET_NENTRIES            4
#define RESET_HDRLEN             12
#define RESET_UCLENGTH           16
#define RESET_INTERVAL           32


/*--[lit_i_cache_section]------------------------------------------------------
 |
 | Here is the code to actually read a section from a .LIT file -- this is
 | fairly ugly, because the section is usually transformed (read encrypted
 | or compressed).
 |
*/
int lit_i_cache_section(lit_file * litfile, section_type * pSection )
{
    char        * path = NULL;
    int         status;
    const char  *guid;
    int         idxTransform, idxControl, nbytes;
    U8          * pList = NULL, * pContent = NULL, *ptr = NULL;
    U8          * pControl;
    int         sizeTransform, sizeContent, sizeControl, nDwords;

    path = lit_i_strmerge(storage_string,pSection->name,transform_string,0);
    if (!path) { return E_LIT_OUT_OF_MEMORY; }

    status = lit_get_file(litfile, path, &pList,&sizeTransform);
    if (status) goto bad;

    free(path); path = NULL;

    path = lit_i_strmerge(storage_string,pSection->name, content_tail,0);
    if (!path) { return E_LIT_OUT_OF_MEMORY; }

    status = lit_get_file(litfile, path, &pContent,&sizeContent);
    if (status) goto bad;

    free(path); path = NULL;

    path = lit_i_strmerge(storage_string,pSection->name, control_tail,0);
    if (!path) { return E_LIT_OUT_OF_MEMORY; }

    status = lit_get_file(litfile, path, &pControl,&sizeControl);
    if (status) goto bad;

    free(path); path = NULL;

    idxTransform = 0;
    idxControl = 0;
    while ((sizeTransform - idxTransform) >= 16)
    {
        nDwords = READ_INT32(pControl + idxControl) + 1;
        if (((idxControl + nDwords*4) > sizeControl) || (nDwords <= 0)) {
            lit_error(ERR_R,"ControlData is too short! (%d > %d), %d.\n",
                idxControl + nDwords*4, sizeControl, nDwords);
            status = E_LIT_FORMAT_ERROR;
            goto bad;
        }

        guid = guid2string(pList + idxTransform);

        if (strcmp(guid, lzxcompress_guid) == 0) {
            status = decompress_section(litfile, pSection->name,
                pControl+idxControl, sizeControl-idxControl,
                pContent, sizeContent, &ptr, &nbytes);
            if (status) goto bad;
            free(pContent);
            pContent = ptr;
            ptr = NULL;
            sizeContent = nbytes;
            idxControl += (nDwords * 4);
        } else {
            lit_error(ERR_R,"Unrecognized transform: \"%s\".", guid);
            status = E_LIT_UNSUPPORTED;
            goto bad;
        }

        idxTransform += 16;
    }
    pSection->data_pointer = pContent;
    pSection->size = sizeContent;

    if (pControl) free(pControl);
    if (path) free(path);
    if (pList) free(pList);
    return 0;
bad:
    if (ptr) free(ptr);
    if (path) free(path);
    if (pList) free(pList);
    if (pContent) free(pContent);
    if (pControl) free(pControl);
    return status;
}

/*--[decompress_section]-------------------------------------------------------
 |
 | A somewhat tricky routine as it handles interfacing with the external
 | LZX library. There is a reason for using the ResetTable here, as
 | otherwise certain files will not be correctly handled!
 |
 | Generally --
 | I need to get the LZXC control information to get the window size.
 | This lets me initialize the LZX library.
 |
 | From there, I read the ResetTable to know the uncompressed length and
 | each "LZXReset()" point.
 |
 | Note that the LZXReset() that this routine requires is not part of the
 | standard lzx distribution. The version that works with these files is from
 | the chmlib implementation and already has been modified.
*/
int decompress_section(lit_file * litfile, char * section_name,
    U8 * pControl, int sizeControl, U8 * pContent, int sizeContent,
    U8 ** ppUncompressed, int * psizeUncompressed)
{
    char        *path;
    int         sizeRT, ofsEntry, base, dst, u;
    int         bytesRemaining, uclength, window_bytes, accum, size;
    U8          * ptr, * pRT;
    int         window_size, status;

    if ((sizeControl < 32) || (READ_U32(pControl+CONTROL_TAG) != LZXC_TAG)) {
        lit_error(ERR_R, "Invalid ControlData tag value %08lx should be %08lx!",
            (sizeControl > 8)?READ_U32(pControl+CONTROL_TAG):0,
            LZXC_TAG);
        return E_LIT_FORMAT_ERROR;
    }

    window_size = 14;
    u = READ_U32(pControl + CONTROL_WINDOW_SIZE);
    while (u) {
        u >>= 1;
        window_size++;
    }
    if ((window_size < 15) || (window_size > 21)) {
        lit_error(ERR_R, "Invalid window in ControlData - %d from %lx.",
            window_size, READ_U32(pControl+CONTROL_WINDOW_SIZE));
        return -1;
    }

    status = LZXinit(window_size);
    if (status) {
        lit_error(ERR_R, "LZXinit(%d) failed, status = %d.",
            window_size, status);
        return E_LIT_LZX_ERROR;
    }

    path = lit_i_strmerge(storage_string,section_name,"/Transform/",
      lzxcompress_guid, rt_tail, NULL);
    if (!path) {
        return E_LIT_OUT_OF_MEMORY;
    }
    status = lit_get_file(litfile, path, &pRT, &sizeRT);
    if (status) { free(path); return status;}

    free(path);
    path = NULL;

    if (sizeRT < (RESET_INTERVAL+8)) {
        lit_error(ERR_R, "Reset table is too short (%d bytes).",
            sizeRT);
        free(pRT);
        return E_LIT_FORMAT_ERROR;
    }
    if (READ_U32(pRT + RESET_UCLENGTH + 4)) {
        lit_error(ERR_R,"Reset table has 64bit value for UCLENGTH!");
        free(pRT);
        return E_LIT_64BIT_VALUE;
    }

    /* Skip first entry -- always 0! */
    ofsEntry = READ_INT32(pRT + RESET_HDRLEN) + 8;
    uclength = READ_INT32(pRT + RESET_UCLENGTH);
    accum    = READ_INT32(pRT + RESET_INTERVAL);

    ptr = malloc(uclength+1);
    /* Check for corruption */
    ptr[uclength] = 0xCC;

    if (!ptr) {
        lit_error(ERR_R, "Unable to malloc uc length (%d bytes)!",
            uclength);
        free(pRT);
        return E_LIT_OUT_OF_MEMORY;
    }
    bytesRemaining = uclength;
    window_bytes = (1 << window_size);

    base = 0;
    dst  = 0;

    while (ofsEntry  < sizeRT)
    {
        if (accum == window_bytes) {
            accum = 0;

            size = READ_INT32(pRT + ofsEntry);
            u = READ_INT32(pRT + ofsEntry + 4);
            if (u) {
                lit_error(ERR_R, "Reset table entry greater than 32 bits!");
                free(pRT); free(ptr);
                return E_LIT_64BIT_VALUE;
            }
            if (size >= sizeContent) {
                lit_error(ERR_R, "ResetTable entry out of bounds, %lx. (%d)",
                    size, ofsEntry);
                free(ptr); free(pRT); return E_LIT_FORMAT_ERROR;
            }
            status = 0;
            if (bytesRemaining >= window_bytes) {
                LZXreset();
                status = LZXdecompress(pContent + base, ptr+dst,
                    size - base, window_bytes);
                bytesRemaining -= window_bytes;
                dst += window_bytes;
                base = size;
            }
            if (status) {
                lit_error(ERR_R, "LZXdecompress failed, status = %d.",
                    status);
                free(ptr);
                free(pRT);
                return -1;
            }
        }
        accum += READ_INT32(pRT + RESET_INTERVAL);
        ofsEntry += 8;
    }
    free(pRT);
    if (bytesRemaining < window_bytes) {
        LZXreset();
        status = LZXdecompress(pContent + base, ptr + dst,
            sizeContent - base, bytesRemaining);
        bytesRemaining = 0;
    }
    if (ptr[uclength] != 0xCC)
    {
        lit_error(ERR_R,
"LZXdecompress overflowed memory. (%02x). \n"\
"This is a serious bug, please report this.\n", ptr[uclength]);

        /* Can't really free, anything may be corrupted at this point */
        return -1;
    }
    if (status) {
        lit_error(ERR_R, "LZXdecompress failed, status = %d.", status);
        free(ptr);
        return E_LIT_LZX_ERROR;
    }

    if (bytesRemaining) {
        lit_error(ERR_R, "Failed to completely decompress section! (%d left).",            bytesRemaining);
        free(ptr);
        return E_LIT_LZX_ERROR;
    }

    if (ppUncompressed) *ppUncompressed = ptr;
    else free(ptr);
    if (psizeUncompressed) *psizeUncompressed = uclength;
    return 0;
}


/*--[lit_i_read_sections]------------------------------------------------------
 |
 | This routine reads the directory of "sections" from the
 | "::DataSpace/NameList" file, and stores them for later. The real work
 | of reading a section occurs in the "cache_section" routine.
 |
 | NameList format:
 |      U16     NUMBER OF SECTIONS
 |      U16     Length of section name 0
 |      U16s    (UTF16) section name
 |      U16     Length of section name 1, and so forth
*/
#define MAX_SECTION_NAME    128
int lit_i_read_sections(lit_file * litfile )
{
    U8          *pNamelist;
    int         status;
    int         nbytes, num_sections, size, section, idx;

    status = lit_get_file(litfile, namelist_string, &pNamelist,&nbytes);
    if (status) return status;

    idx = 2;
    size = 2;
    if ((idx + size) > nbytes) goto bad;
    num_sections = READ_U16(pNamelist+idx);
    idx += 2;

    litfile->sections = malloc(num_sections * sizeof(section_type));
    if (!litfile->sections) {
        lit_error(ERR_R|ERR_LIBC,"malloc(%d) failed!\n",
            num_sections*sizeof(section_type));
        free(pNamelist);
        return E_LIT_OUT_OF_MEMORY;
    }
    memset(litfile->sections, 0, num_sections * sizeof(section_type));

    litfile->num_sections = num_sections;

    for (section = 0; section < num_sections; section++) {
        int j;

        size = 2;
        if ((idx + size) > nbytes) goto bad;
        size = READ_U16(pNamelist + idx);
        idx += 2;

        if (size > MAX_SECTION_NAME) {
            lit_error(ERR_R,
"litlib.c cannot handle a %d byte section name, fix MAX_SECTION_NAME (%d)\n",
                size, MAX_SECTION_NAME);
            return -1;
        }
        size = (size * 2) + 2;

        if ((idx + size) > nbytes) goto bad;

        for (j = 0; j < size; j+=2) {
            litfile->sections[section].name[j/2] = pNamelist[idx + j];
        }
        litfile->sections[section].name[j/2] = '\0';
        idx += size;
    }
    free(pNamelist);
    return 0;
bad:
    lit_error(ERR_R,
        "Invalid or corrupt ::DataSpace/Namelist!\n"
        "\nTried to read %d bytes past length of (%ld)\n", size,nbytes);
    if (pNamelist) free(pNamelist);
    return -1;
}
