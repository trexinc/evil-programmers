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

#ifndef INCLUDED_UTF8_H
#define INCLUDED_UTF8_H

#include <stddef.h>

char *encode_UTF8(char *utf8s, wchar_t *ws);
//int UTF8_strsize(wchar_t *ws);
wchar_t *decode_UTF8(wchar_t *ws, unsigned char *utf8s);
//int UTF8_strlen(char *utf8s);

#endif
