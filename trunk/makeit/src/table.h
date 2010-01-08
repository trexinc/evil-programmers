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

#ifndef __TABLE_H__
#define __TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TABLE_PREALLOC
#define TABLE_PREALLOC 4
#endif

#define TABLE_FLAG_ALLOCNAME  1
#define TABLE_FLAG_ALLOCVALUE 2
#define TABLE_FLAG_ALLOCBOTH (TABLE_FLAG_ALLOCNAME|TABLE_FLAG_ALLOCVALUE)

typedef struct tag_Attr{
  pchar szName;
  pchar szValue;
}SAttr,*PAttr;

typedef struct tag_TablePage{
  PAttr Values;
  int iCount;
  int iSize;
  struct tag_TablePage* pNext;
} STablePage,*PTablePage;

typedef struct tag_Table{
  SAttr aPreAlloc[TABLE_PREALLOC];
  STablePage sFirst;
  PTablePage pFirst;
  PTablePage pLast;
  int iHoles;
  int iCount;
  int iFlags;
  PTablePage pIterPage;
  int iIterPos;
  void* pPool;
}STable,*PTable;

PTable tableNew(void* Pool,int);
void tableFree(PTable t);
int tableAdd(PTable t,const pchar Key,const pchar Value);
pchar tableGet(PTable t,const pchar Key);
int tableDel(PTable t,const pchar Key);
int tableSet(PTable t,const pchar Key,const pchar Value);
void tableFirst(PTable t);
int tableNext(PTable t,pchar* Name,pchar* Value);

#ifdef __cplusplus
}
#endif

#endif
