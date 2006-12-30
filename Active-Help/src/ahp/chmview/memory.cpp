#include "memory.hpp"
#include <windows.h>

#define heapNEW GetProcessHeap()

void *malloc(size_t size)
{
  return HeapAlloc(heapNEW,HEAP_ZERO_MEMORY,size);
}

void *realloc(void *block, size_t size)
{
  if (block)
    return HeapReAlloc(heapNEW,HEAP_ZERO_MEMORY,block,size);
  else
    return HeapAlloc(heapNEW,HEAP_ZERO_MEMORY,size);
}

void free(void *block)
{
  if (block)
    HeapFree(heapNEW,0,block);
}

char *strdup(const char *block)
{
  char *result=(char*)malloc(lstrlen(block)+1);
  if (result==(char*)0) return (char*)0;
  lstrcpy(result,block);
  return result;
}
