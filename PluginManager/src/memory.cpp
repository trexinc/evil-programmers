#include "memory.hpp"
#include <windows.h>

#define heapNEW GetProcessHeap()

void * __cdecl malloc(size_t size)
{
  return HeapAlloc(heapNEW, HEAP_ZERO_MEMORY, size);
}

void * __cdecl realloc(void *block, size_t size)
{
  if (block)
    return HeapReAlloc(heapNEW,HEAP_ZERO_MEMORY,block,size);
  else
    return HeapAlloc(heapNEW,HEAP_ZERO_MEMORY, size);
}

void __cdecl free(void *block)
{
  if (block)
    HeapFree(heapNEW,0,block);
}
/*
void *calloc(size_t nitems, size_t size)
{
  return malloc(nitems*size);
}

char *strdup(const char *block)
{
  char *result=(char*)malloc(lstrlen(block)+1);
  if(result==(char*)0) return (char*)0;
  lstrcpy(result,block);
  return result;
}
*/
#ifdef __cplusplus
void * __cdecl operator new(size_t size)
{
  return malloc(size);
}

void __cdecl operator delete(void *block)
{
  free(block);
}
/*
void *__cdecl operator new[] (size_t size)
{
  return ::operator new(size);
}

void __cdecl operator delete[] (void *ptr)
{
  ::operator delete(ptr);
}
*/
#endif
