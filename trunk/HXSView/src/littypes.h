//some changes made to make it work with HXS
//AY

/*****************************************************************************/
/*--[littypes.h]---------------------------------------------------------------
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
#ifndef LITTYPES_H
#define LITTYPES_H

typedef     unsigned char       U8;
typedef     unsigned short int  U16;
typedef     unsigned long int   U32;
#ifdef _MSC_VER
typedef     unsigned __int64    U64;
#else
typedef     unsigned long long  U64;
#endif

#define READ_U32(pv) (  (U32)*((U8 *)(pv))                + \
                        ((U32)(*((U8 *)(pv) + 1)) <<  8)  + \
                        ((U32)(*((U8 *)(pv) + 2)) << 16)  + \
                        ((U32)(*((U8 *)(pv) + 3)) << 24)  )

#define READ_INT32(pv)  ((int)(READ_U32(pv)&0x7FFFFFFF))

#define READ_U16(pv) (int)((U16)*((U8 *)(pv))                + \
                           ((U16)(*((U8 *)(pv) + 1)) <<  8)  )

#endif
