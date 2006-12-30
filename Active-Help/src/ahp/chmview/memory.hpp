#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  void *malloc(size_t size);
  void *realloc(void *block, size_t size);
  void free(void *block);
  char *strdup(const char *block);
#ifdef __cplusplus
};
#endif

#endif
