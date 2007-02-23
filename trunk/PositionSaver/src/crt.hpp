#ifndef __CRT_HPP__
#define __CRT_HPP__
#include <stddef.h>

extern "C"
{
  void *memcpy(void * dst, const void * src, size_t count);
  void *memset(void *dst, int val, size_t count);
  int strncmp(const char * first, const char * last, size_t count);
};

#endif
