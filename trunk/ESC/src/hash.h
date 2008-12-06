/*
  Copyright (c) Konstantin Stupnik (aka Xecutor) 2000-2001 <skv@aurorisoft.com>
  You can use, modify, distribute this code or any other part
  only with permissions of author!

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
  pchar szKey;
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
int hashExists(PHash h,const pchar Key);
void hashDelete(PHash h,const pchar Key);
void *hashGet(PHash h,const pchar Key);
PHashLink hashSet(PHash p,const pchar Key,const void* Value);
void hashFirst(PHash h);
int hashNext(PHash h,pchar* pKey,void** pValue);
void* hashGetN(PHash h,const pchar Key,int N);
void* hashGetEx(PHash h,const pchar Key,int keylen);
int hashKeyCount(PHash h,const pchar Key);
PHashLink hashAdd(PHash h,const pchar Key,void* Value);
PHashLink hashEnumKey(PHash h,const pchar Key,PHashLink lnk,void**Value);

#ifdef __cplusplus
}
#endif
#endif
