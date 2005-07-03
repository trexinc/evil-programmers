#ifndef __MEMORY_HPP_
#define __MEMORY_HPP_

/*
 *    Memory allocation functions
 */

#define ZMEM_GUARD_BYTES 8


extern void * z_calloc ( size_t nitems, size_t size);
extern void * z_malloc(size_t size);
extern void * z_realloc( void *block, size_t size);
extern void   z_free   ( void *block );
extern char * z_strdup ( const char *block );
#if !defined(TDEBUG)
extern void *__cdecl operator new(size_t size);
//extern void *__cdecl operator new[] (size_t size);
//extern void *__cdecl operator new(size_t size, void *p);
extern void __cdecl operator delete(void *p);
//extern void __cdecl operator delete[] (void *ptr);
#endif

extern void SayException( const char *str );

#endif
