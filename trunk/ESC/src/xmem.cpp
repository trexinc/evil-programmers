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

// #include <stdlib.h>
// #include <string.h>
#include "myrtl.hpp"
#include "xmem.h"

//#define MEMDEBUG

typedef struct tag_MemPoolPage{
  char *pMem;
  int iSize;
  int iAlloc;
  struct tag_MemPoolPage *pNext;
}SMemPoolPage,*PMemPoolPage;

typedef struct tag_MemPool{
  PMemPoolPage pFirst;
  PMemPoolPage pLast;
  int iPageSize;
}SMemPool,*PMemPool;

void* xmemNewPool(int iPageSize)
{
  PMemPool pl;
  PMemPoolPage pg;
  char *m;
  m=(char *)malloc(iPageSize+sizeof(*pg)+sizeof(*pl));
  pg=(PMemPoolPage)m;
  pl=(PMemPool)(m+sizeof(*pg));
  pl->iPageSize=iPageSize;
  pg->iSize=iPageSize;
  pg->pMem=m;
  pg->iAlloc=sizeof(*pg)+sizeof(*pl);
  pg->pNext=0;
  pl->pFirst=pg;
  pl->pLast=pg;
  return pl;
}


void xmemFreePool(void* Pool)
{
  PMemPool pl=(PMemPool)Pool;
  PMemPoolPage q,p=pl->pFirst;
  while(p)
  {
    q=p->pNext;
    free(p->pMem);
    p=q;
  }
}

int xmemUsage(void* Pool)
{
  PMemPoolPage p=((PMemPool)Pool)->pFirst;
  int size=0;
  while(p)
  {
    size+=p->iAlloc;
    p=p->pNext;
  }
  return size;
}

static void newMemPoolPage(PMemPool pl,int minsize)
{
  int size=pl->iPageSize;
  PMemPoolPage p;
  char *m;
  if(size<minsize)size=minsize;
  size+=sizeof(SMemPoolPage);
  m=(char *)malloc(size);
  p=(PMemPoolPage)m;
  p->pMem=m;
  p->iSize=size;
  p->iAlloc=sizeof(*p);
  p->pNext=0;
  pl->pLast->pNext=p;
  pl->pLast=p;
}


void* xalloc(void *Pool,int size)
{
  char *p;
  PMemPool pl=(PMemPool)Pool;
  PMemPoolPage l=pl->pLast;
  if(l->iAlloc+size<l->iSize)
  {
    p=l->pMem+l->iAlloc;
    l->iAlloc+=size;
  }else
  {
    newMemPoolPage(pl,size);
    l=pl->pLast;
    p=l->pMem+l->iAlloc;
    l->iAlloc+=size;
  }
  return p;
}

void* xcalloc(void *Pool,int size)
{
  void *p=xalloc(Pool,size);
  memset(p,0,size);
  return p;
}

void xfree(void* p)
{
}

int xstrlen(const wchar_t *str)
{
  if(!str)return 0;
  return wstrlen(str);
}

wchar_t *xstrdup(void* Pool,const wchar_t *s)
{
  wchar_t *d;
  if(s==NULL)return NULL;
  d=(wchar_t *)xalloc(Pool,(xstrlen(s)+1)*sizeof(wchar_t));
  wstrcpy(d,s);
  return d;
}

wchar_t *xstrndup(void* Pool,const wchar_t *s,int n)
{
  wchar_t *d;
  if(s==NULL || n<0)return NULL;
  d=(wchar_t *)xalloc(Pool,(n+1)*sizeof(wchar_t));
  wstrncpy(d,s,n);
  d[n]=0;
  return d;
}

int xstrcmp(const wchar_t *a,const wchar_t *b)
{
  if(!a || !b)return 0;
  return !wstrcmp(a,b);
}

int xstrncmp(const wchar_t *a,const wchar_t *b,int n)
{
  if(!a || !b || n<=0)return 0;
  return !wstrncmp(a,b,n);
}

wchar_t *xstrncpy(wchar_t *dst,const wchar_t *src,int n)
{
  if(!src || n<=0)
  {
    dst[0]=0;
    return dst;
  }
  wstrncpy(dst,src,n);
  dst[n]=0;
  return dst;
}

wchar_t *xstrcpy(wchar_t *dst,const wchar_t *src)
{
  if(!src)
  {
    dst[0]=0;
    return dst;
  }
  wstrcpy(dst,src);
  return dst;
}

const wchar_t *xstrchr(const wchar_t *str,wchar_t c)
{
  if(!str)return NULL;
  return wstrchr(str,c);
}
