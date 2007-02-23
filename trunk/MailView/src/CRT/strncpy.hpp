#ifndef __STRNCPY_HPP__
#define __STRNCPY_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  char *strncpy(char *dest,const char *src,size_t size);
#ifdef __cplusplus
};
#endif

#endif
