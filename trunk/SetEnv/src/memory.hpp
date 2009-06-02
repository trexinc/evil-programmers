#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__

#define hHeap GetProcessHeap()

PVOID __cdecl malloc(size_t size);
PVOID __cdecl realloc(PVOID block, size_t size);
VOID __cdecl free(PVOID block);
DWORD memsize(LPCVOID lpMem);

#ifdef __cplusplus
PVOID __cdecl operator new(size_t size);
VOID __cdecl operator delete(PVOID block);
#endif

VOID _pure_error_ ();

#endif /* __MEMORY_HPP__ */