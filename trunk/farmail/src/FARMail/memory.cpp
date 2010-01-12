#include "farmail.hpp"

void SayException( const char *str )
{
  char buf[200];
  MessageBeep( 0xFFFFFFFF );
  FSF.sprintf( buf, "An unhandled exception occured in %s. Program must be terminated.\r\nPlease save your work and restart FAR.", str );
  MessageBox( NULL, buf, "FARMail - Fatal error", MB_OK | MB_ICONSTOP | MB_TOPMOST );
}

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

#if !defined(TDEBUG)
void *__cdecl operator new(size_t size)
{
  size = size ? size : 1;
  return z_malloc(size);
}

void *__cdecl operator new[] (size_t size)
{
  return::operator new(size);
}
/*void *__cdecl operator new(size_t size, void *p)
{
  return p;
}*/
void __cdecl operator delete(void *p)
{
  z_free(p);
}
void __cdecl operator delete[] (void *ptr)
{
  ::operator delete(ptr);
}
#endif
