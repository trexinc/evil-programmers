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
  Special hashtable implementation.
  This table can hold several values with one key.
*/

#ifndef __HASH_H__
#define __HASH_H__

#define HASH_DEFAULT_SIZE 4

#define HASH_FLAG_ALLOCKEY

#ifdef __cplusplus
extern "C" {
#endif


typedef struct tag_HashLink{
  wchar_t *szKey;
  void *pValue;
  unsigned  uHashCode;
  struct tag_HashLink *pNext;
} SHashLink,*PHashLink;

typedef struct tag_HashList{
  PHashLink pHead,pTail;
}SHashList,*PHashList;

typedef struct tag_Hash{
  PHashList pBuckets;
  int iBucketsNum;
  int iCount;
  int iIterIndex;
  PHashLink pIterLink;
  void* pPool;
} SHash, *PHash;


PHash hashNew(void* pool);
PHash hashNewEx(void* pool,int cnt);
void hashFree(PHash h);
int hashExists(PHash h,const wchar_t *Key);
void hashDelete(PHash h,const wchar_t *Key);
void *hashGet(PHash h,const wchar_t *Key);
PHashLink hashSet(PHash p,const wchar_t *Key,const void* Value);
void hashFirst(PHash h);
int hashNext(PHash h,wchar_t **pKey,void** pValue);
void* hashGetN(PHash h,const wchar_t *Key,int N);
void* hashGetEx(PHash h,const wchar_t *Key,int keylen);
int hashKeyCount(PHash h,const wchar_t *Key);
PHashLink hashAdd(PHash h,const wchar_t *Key,void* Value);
PHashLink hashEnumKey(PHash h,const wchar_t *Key,PHashLink lnk,void**Value);

#ifdef __cplusplus
}
#endif
#endif
