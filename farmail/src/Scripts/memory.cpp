#include <windows.h>

void * operator new(size_t size)
{
  return HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size);
}

void operator delete(void *block)
{
  if(block) HeapFree(GetProcessHeap(),0,block);
}

void * operator new[](size_t size)
{
  return HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size);
}

void operator delete[](void *block)
{
  if(block) HeapFree(GetProcessHeap(),0,block);
}
