#ifndef __CRT_HPP__
#define __CRT_HPP__

#include <stddef.h>

extern "C"
{
  void *memcpy(void * dst, const void * src, size_t count);
  void *memset(void *dst, int val, size_t count);
  int strncmp(const char * first, const char * last, size_t count);
  char *strchr(register const char *s,int c);
  char *strstr(const char *str1, const char *str2);
  int isspace(int c);
  int isdigit(int c);
  long atol(const char *nptr);
  void srand(unsigned x);
  long _lrand(void);
  char *strtok(char *string, const char *control);
#if __GNUC__>=4 && !defined(TDEBUG)
  int atexit(void(*)(void));
#endif
};

#endif
