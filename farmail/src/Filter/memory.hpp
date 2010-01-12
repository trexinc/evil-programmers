#ifndef __MEMORY_HPP_
#define __MEMORY_HPP_

void * z_calloc ( size_t nitems, size_t size);
void * z_malloc ( size_t size);
void * z_realloc( void *block, size_t size);
void   z_free   ( void *block );
char * z_strdup ( const char *block );

#endif
