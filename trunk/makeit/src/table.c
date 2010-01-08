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
  Table that hold key-value pairs.
  Only effecient when amount of items it hold is small enough.
*/

#include "xmem.h"
#include "table.h"
#include <string.h>

static PTablePage tpNew(void* Pool,int size)
{
  PTablePage p;
  p=xalloc(Pool,sizeof(*p));
  p->Values=xalloc(Pool,sizeof(SAttr)*size);
  p->iSize=size;
  p->iCount=0;
  p->pNext=0;
  return p;
}
/*
static void tpFree(PTablePage p)
{
  xfree(p->Values);
  xfree(p);
}*/

PTable tableNew(void* Pool,int Flags)
{
  PTable t;
  t=xcalloc(Pool,sizeof(*t));
  t->pFirst=&t->sFirst;
  t->pFirst->iSize=TABLE_PREALLOC;
  t->pFirst->Values=&t->aPreAlloc[0];
  t->pLast=t->pFirst;
  t->iFlags=Flags;
  t->pPool=Pool;
  return t;
}

void tableFree(PTable t)
{
/*  PTablePage q,p=t->pFirst->pNext;
  if(t->iFlags&TABLE_FLAG_ALLOCBOTH)
  {
    pchar n,*v;
    tableFirst(t);
    while(tableNext(t,&n,&v))
    {
      if(t->iFlags&TABLE_FLAG_ALLOCNAME)xfree(n);
      if(t->iFlags&TABLE_FLAG_ALLOCVALUE)xfree(v);
    }
  }
  while(p)
  {
    q=p->pNext;
    tpFree(p);
    p=q;
  }
  xfree(t);*/
}

int tableAdd(PTable t,const pchar Name,const pchar Value)
{
  PTablePage p;

  if(t->iHoles)
  {
    p=t->pFirst;
    while(p->iCount==p->iSize)
    {
      if(p->pNext)p=p->pNext;else break;
    }
  }else
  {
    p=t->pLast;
  }
  if(p->iCount==p->iSize)
  {
    p->pNext=tpNew(t->pPool,p->iSize*2);
    p=p->pNext;
    t->iHoles=0;
    t->pLast=p;
  }
  if(t->iFlags&TABLE_FLAG_ALLOCNAME)
    p->Values[p->iCount].szName=xstrdup(t->pPool,Name);
  else
    p->Values[p->iCount].szName=(pchar)Name;

  if(t->iFlags&TABLE_FLAG_ALLOCVALUE)
    p->Values[p->iCount].szValue=xstrdup(t->pPool,Value);
  else
    p->Values[p->iCount].szValue=(pchar)Value;
  p->iCount++;
  t->iCount++;
  return t->iCount;
}

static PTablePage tableFind(PTablePage p,int* idx,const pchar Name)
{
  for(;;)
  {
    if(*idx>=p->iCount)
    {
      if(!p->pNext)return NULL;
      p=p->pNext;
      *idx=0;
    }
    if(xstrcmp(p->Values[*idx].szName,Name))return p;
    (*idx)++;
  }
}

pchar tableGet(PTable t,const pchar Name)
{
  PTablePage p;
  int idx=0;
  if(!t)return NULL;
  p=t->pFirst;
  p=tableFind(p,&idx,Name);
  if(p)return p->Values[idx].szValue;else return NULL;
}

int tableDel(PTable t,const pchar Name)
{
  PTablePage p=t->pFirst;
  int idx=0;
  int res=0;
  while(p)
  {
    p=tableFind(p,&idx,Name);
    if(p)
    {
      if(t->iFlags&TABLE_FLAG_ALLOCNAME)xfree(p->Values[idx].szName);
      if(t->iFlags&TABLE_FLAG_ALLOCVALUE)xfree(p->Values[idx].szValue);
      if(idx!=p->iCount-1)memmove(&p->Values[idx],&p->Values[idx+1],sizeof(SAttr)*(p->iCount-idx-1));
      p->iCount--;
      t->iCount--;
      res++;
    }
  }
  if(res)t->iHoles++;
  return res;
}

int tableSet(PTable t,const pchar Name,const pchar Value)
{
  PTablePage p=t->pFirst,q;
  int idx0,idx=0,res=0;
  p=tableFind(p,&idx,Name);
  if(p)
  {
    q=p;idx0=idx;
    while((p=tableFind(p,&idx,Name)))
    {
      if(t->iFlags&TABLE_FLAG_ALLOCNAME)xfree(p->Values[idx].szName);
      if(t->iFlags&TABLE_FLAG_ALLOCVALUE)xfree(p->Values[idx].szValue);
      if(idx!=p->iCount-1)memmove(&p->Values[idx],&p->Values[idx+1],sizeof(SAttr)*(p->iCount-idx-1));
      p->iCount--;
      t->iCount--;
      res++;
    }
    if(res)t->iHoles++;
    if(t->iFlags&TABLE_FLAG_ALLOCVALUE)xfree(q->Values[idx].szValue);
    if(xstrlen(q->Values[idx0].szValue)>=xstrlen(Value))
      xstrcpy(q->Values[idx0].szValue,Value);
    else
      q->Values[idx0].szValue=xstrdup(t->pPool,Value);
    return t->iCount;
  }else return tableAdd(t,Name,Value);
}

void tableFirst(PTable t)
{
  t->pIterPage=t->pFirst;
  t->iIterPos=-1;
}

int tableNext(PTable t,pchar* Name,pchar* Value)
{
  if(!t->pIterPage)return 0;
  t->iIterPos++;
  for(;;)
  {
    if(t->iIterPos>=t->pIterPage->iCount)
    {
      t->iIterPos=0;
      t->pIterPage=t->pIterPage->pNext;
      if(!t->pIterPage)return 0;
    }else break;
  }
  *Name=t->pIterPage->Values[t->iIterPos].szName;
  *Value=t->pIterPage->Values[t->iIterPos].szValue;
  return 1;
}
