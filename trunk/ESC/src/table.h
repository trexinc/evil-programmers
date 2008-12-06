/*
  Copyright (c) Konstantin Stupnik (aka Xecutor) 2000-2001 <skv@aurorisoft.com>
  You can use, modify, distribute this code or any other part
  only with permissions of author!

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
