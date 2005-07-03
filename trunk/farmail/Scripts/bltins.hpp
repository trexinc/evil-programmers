/*
    Scripts sub-plugin for FARMail
    Copyright (C) 2002-2005 FARMail Group
    Copyright (C) 1999,2000 Serge Alexandrov

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
#ifndef __BLTINS_HPP__
#define __BLTINS_HPP__

Variant WINAPI blt_strlen(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_nlines(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_line(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_setline(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_delline(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_insline(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_string(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_integer(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_message(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_char(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_substr(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_strlwr(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_strupr(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_strstr(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_random(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_date(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_time(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_boundary(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_fileline(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_filecount(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_blktype(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_blkstart(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_selstart(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_selend(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_setsel(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_usrinput(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_redirect(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_version(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_setpos(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_header(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_usrmenu(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_winampstate(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_winampsong(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_addressbook(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_exit(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_sprintf(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_encodeheader(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_getini(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_setini(long count,Variant *values,int *stop,void *ptr);
Variant WINAPI blt_editorstate(long count,Variant *values,int *stop,void *ptr);

#endif
