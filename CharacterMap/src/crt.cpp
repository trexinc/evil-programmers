#include "crt.hpp"

void *memcpy(void * dst, const void * src, size_t count)
{
  void * ret = dst;

  while (count--)
  {
    *(char *)dst = *(char *)src;
    dst = (char *)dst + 1;
    src = (char *)src + 1;
  }
  return(ret);
}

void *memset(void *dst, int val, size_t count)
{
  void *start = dst;

  while (count--)
  {
    *(char *)dst = (char)val;
    dst = (char *)dst + 1;
  }
  return(start);
}

/*
char *strstr(const char * str1, const char * str2)
{
  char *cp = (char *) str1;
  char *s1, *s2;

  if ( !*str2 )
    return((char *)str1);

  while (*cp)
  {
    s1 = cp;
    s2 = (char *) str2;

    while ( *s1 && *s2 && !(*s1-*s2) )
      s1++, s2++;

    if (!*s2)
      return(cp);

    cp++;
  }

  return(NULL);
}

void *memchr(const void * buf, int chr, size_t cnt)
{
  while ( cnt && (*(unsigned char *)buf != (unsigned char)chr) )
  {
    buf = (unsigned char *)buf + 1;
    cnt--;
  }
  return(cnt ? (void *)buf : NULL);
}

int memcmp(const void * buf1, const void * buf2, size_t count)
{
  if (!count)
    return(0);

  while ( --count && *(char *)buf1 == *(char *)buf2 )
  {
    buf1 = (char *)buf1 + 1;
    buf2 = (char *)buf2 + 1;
  }

  return( *((unsigned char *)buf1) - *((unsigned char *)buf2) );
}
*/
