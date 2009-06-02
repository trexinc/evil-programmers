#include "stdafx.h"
#include "memory.hpp"

PVOID __cdecl malloc(size_t size)
{
  return HeapAlloc(hHeap, HEAP_ZERO_MEMORY, size);
}

PVOID __cdecl realloc(PVOID block, size_t size)
{
  if (block)
    return HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, block, size);
  else
    return HeapAlloc(hHeap, HEAP_ZERO_MEMORY, size);
}

VOID __cdecl free(PVOID block)
{
  if (block)
    HeapFree(hHeap, 0, block);
}

DWORD memsize(LPCVOID lpMem)
{
  if (lpMem)
    return HeapSize(hHeap, 0, lpMem);
  else
    return(0);
}

#ifdef __cplusplus
PVOID __cdecl operator new(size_t size)
{
  return malloc(size);
}

VOID __cdecl operator delete(PVOID block)
{
  free(block);
}
#endif

void _pure_error_ () {};
