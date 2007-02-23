#include "memory.hpp"
#include <windows.h>

#define HeapHandle GetProcessHeap()

void *malloc(size_t size)
{
  return HeapAlloc(HeapHandle,HEAP_ZERO_MEMORY,size);
}

void *realloc(void *block,size_t size)
{
  return ((size)?((block)?HeapReAlloc(HeapHandle,HEAP_ZERO_MEMORY,block,size):HeapAlloc(HeapHandle,HEAP_ZERO_MEMORY,size)):(HeapFree(HeapHandle,0,block),(void *)0));
}

void free(void *block)
{
  if(block) HeapFree(HeapHandle,0,block);
}

void *__cdecl operator new(size_t size)
{
  size = size ? size : 1;
  return malloc(size);
}

/*void *__cdecl operator new[] (size_t size)
{
  return::operator new(size);
}*/

/*void *__cdecl operator new(size_t size, void *p)
{
  return p;
}*/

void __cdecl operator delete(void *p)
{
  free(p);
}

/*void __cdecl operator delete[] (void *ptr)
{
  ::operator delete(ptr);
}*/
