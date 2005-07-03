#include "crt.hpp"
#include <stddef.h>
#include "memory.hpp"
#include <windows.h>

#if !defined (_MSC_VER)

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
#endif

int strncmp(const char * first, const char * last, size_t count)
{
  if (!count)
    return(0);

  while (--count && *first && *first == *last)
  {
    first++;
    last++;
  }

  return( *(unsigned char *)first - *(unsigned char *)last );
}

char *strchr(register const char *s,int c)
{
  do
  {
    if(*s==c)
    {
      return (char*)s;
    }
  } while (*s++);
  return (0);
}

char *strstr(const char *str1, const char *str2)
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

static int DotFound(char *pB)
{
  if (*(pB-1) == '.')
    pB--;
  switch (*--pB)
  {
    case ':'  :
      if (*(pB-2) != '\0')
        break;
    case '/'  :
    case '\\' :
    case '\0' :
      return 1;
  }
  return 0;
}

char *strtok(char *string, const char *control)
{
  unsigned char *str;
  const unsigned char *ctrl = (const unsigned char *) control;

  unsigned char map[32];
  int count;

  static unsigned char *nextoken;

  for (count = 0; count < 32; count++)
    map[count] = 0;

  do
  {
    map[*ctrl >> 3] |= (1 << (*ctrl & 7));
  } while (*ctrl++);

  if (string)
    str = (unsigned char *) string;
  else
    str = nextoken;

  while ((map[*str >> 3] & (1 << (*str & 7))) && *str)
          str++;

  string = (char *) str;

  for (; *str; str++)
    if (map[*str >> 3] & (1 << (*str & 7)))
    {
      *str++ = '\0';
      break;
    }

  nextoken = str;

  if (string == (char *)str)
    return NULL;
  else
    return string;
}

static void CopyIt(char *dst, const char *src, unsigned maxlen)
{
  if (dst)
  {
    if((unsigned)lstrlen(src) >= maxlen)
      lstrcpyn(dst, src, maxlen+1);
    else
      lstrcpy(dst, src);
  }
}

int isspace(int c)
{
  return (c==32 || c==9 ? 1 : 0);
}

int isdigit(int c)
{
  return (c>='0' && c<='9' ? 1 : 0);
}

long atol(const char *nptr)
{
  int c;              /* current char */
  long total;         /* current total */
  int sign;           /* if '-', then negative, otherwise positive */

  /* skip whitespace */
  while ( isspace((int)(unsigned char)*nptr) )
    ++nptr;

  c = (int)(unsigned char)*nptr++;
  sign = c;           /* save sign indication */
  if (c == '-' || c == '+')
    c = (int)(unsigned char)*nptr++;    /* skip sign */

  total = 0;

  while (isdigit(c))
  {
    total = 10 * total + (c - '0');     /* accumulate digit */
    c = (int)(unsigned char)*nptr++;    /* get next char */
  }

  if (sign == '-')
    return -total;
  else
    return total;   /* return result, negated if necessary */
}

#if defined(__GNUC__) || defined (_MSC_VER)
static unsigned long long next=0;

long _lrand(void)
{
  next=next*0x5deece66dLL+11;
  return (long)((next>>16)&0x7fffffff);
}

void srand(unsigned seed)
{
  next=seed;
}
#else
#undef __seed_t
typedef struct
{
  unsigned lo;
  unsigned hi;
} __seed_t;

static __seed_t Seed = { 1, 0 };

void srand(unsigned seed)
{
  Seed.lo = seed;
  Seed.hi = 0;
}

long _lrand(void)
{
//      Seed = MULTIPLIER * Seed + INCREMENT;
//      return((int)(Seed & 0x7fffffffL));
  _EBX = Seed.hi;
  _ESI = Seed.lo;
  asm
  {
    mov eax, ebx
    mov ecx, 015Ah
    mov ebx, 4E35h


    test  eax, eax
    jz  nohi1
    mul ebx

  nohi1:
    xchg  ecx, eax
    mul esi
    add eax, ecx

    xchg  eax, esi
    mul ebx
    add edx, esi

    add eax, 1
    adc edx, 0
    mov ebx, eax
    mov esi, edx
  }
  Seed.lo = _EBX;
  Seed.hi = _ESI;
  return _ESI & 0x7fffffff;
}
#endif

#if defined(TDEBUG) && defined(__GNUC__)
extern "C"
{
  size_t write(int _fd, const void *_buf, size_t _nbyte);
  void* malloc(size_t _size);
  void free(void* _ptr);
  void _assert(const char *msg, const char *file, int line);
  void abort();
  int strcmp(const char *s1, const char *s2);
};

#if __GNUC__==2
size_t write(int _fd, const void *_buf, size_t _nbyte)
{
  (void) _fd;
  (void *) _buf;
  return _nbyte;
}
#endif

void* malloc(size_t _size)
{
  return(z_malloc(_size));
}

void free(void* _ptr)
{
  z_free(_ptr);
}

void _assert(const char *msg, const char *file, int line)
{
  (void *) msg;
  (void *) file;
  (void) line;
}

void abort()
{
  for(;;);
}

int strcmp(const char *s1, const char *s2)
{
  return lstrcmp(s1,s2);
}
#endif
