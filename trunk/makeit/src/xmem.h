/*
  Copyright (C) 2000 Konstantin Stupnik

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

typedef char xchar;
typedef unsigned  char xuchar;
typedef xchar* pchar;

void* xmemNewPool(int iPageSize);
void xmemFreePool(void* Pool);
void* xalloc(void *Pool,int size);
void* xcalloc(void *Pool,int size);

int xmemUsage(void* Pool);

int xstrlen(pchar str);
pchar xstrdup(void* Pool,const pchar s);
pchar xstrndup(void* Pool,const pchar s,int n);
void xfree(void* p);
int xstrcmp(pchar a,const pchar b);
int xstrncmp(const pchar a,const pchar b,int n);
pchar xstrncpy(pchar dst,const pchar src,int n);
pchar xstrcpy(pchar dst,const pchar src);
pchar xstrchr(const pchar str,int c);

#ifdef __cplusplus
}
#endif

#endif
