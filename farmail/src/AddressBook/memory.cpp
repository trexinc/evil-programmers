#include <windows.h>
#include <stddef.h>
#include "memory.hpp"

#define HeapHandle GetProcessHeap()

void *z_calloc(size_t nitems,size_t size)
{
  return HeapAlloc(HeapHandle,HEAP_ZERO_MEMORY,nitems*size);
}

void *z_malloc(size_t size)
{
  return HeapAlloc(HeapHandle,HEAP_ZERO_MEMORY,size);
}

void *z_realloc(void *block,size_t size)
{
  return ((size)?((block)?HeapReAlloc(HeapHandle,HEAP_ZERO_MEMORY,block,size):HeapAlloc(HeapHandle,HEAP_ZERO_MEMORY,size)):(HeapFree(HeapHandle,0,block),(void *)0));
}


void z_free(void *block)
{
  if(block) HeapFree(HeapHandle,0,block);
}

char *z_strdup(const char *block)
{
  char *result=(char*)z_malloc(lstrlen(block)+1);
  if(result==(char*)0) return (char*)0;
  lstrcpy(result,block);
  return result;
}
