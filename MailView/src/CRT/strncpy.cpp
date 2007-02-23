#include "strncpy.hpp"

char *strncpy(char *dest,const char *src,size_t size)
{
  char *start = dest;

  while (size && (*dest++ = *src++))
    size--;

  if (size)
    while (--size)
      *dest++ = '\0';

  return (start);
}
