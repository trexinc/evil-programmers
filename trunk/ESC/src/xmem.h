/*
  [ESC] Editor's settings changer plugin for FAR Manager
  Copyright (C) 2000 Konstantin Stupnik
  Copyright (C) 2008 Alex Yaroslavsky

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

  This unit used internally by xmlite.
  Pooled memory management implementation.
  Used for fastest allocation, optimal memory filing,
  and realy fast large structures freeing.
*/

#ifndef __XMEM_H__
#define __XMEM_H__

#include "myrtl.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

void* xmemNewPool(int iPageSize);
void xmemFreePool(void* Pool);
void* xalloc(void *Pool,int size);
void* xcalloc(void *Pool,int size);

int xmemUsage(void* Pool);

int xstrlen(const wchar_t *str);
wchar_t *xstrdup(void* Pool,const wchar_t *s);
wchar_t *xstrndup(void* Pool,const wchar_t *s,int n);
void xfree(void* p);
int xstrcmp(const wchar_t *a,const wchar_t *b);
int xstrncmp(const wchar_t *a,const wchar_t *b,int n);
wchar_t *xstrncpy(wchar_t *dst,const wchar_t *src,int n);
wchar_t *xstrcpy(wchar_t *dst,const wchar_t *src);
const wchar_t *xstrchr(const wchar_t *str,wchar_t c);

#ifdef __cplusplus
}
#endif

#endif
