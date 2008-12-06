/*
  Copyright (c) Konstantin Stupnik (aka Xecutor) 2000-2001 <skv@aurorisoft.com>
  You can use, modify, distribute this code or any other part
  only with permissions of author!

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
