#if defined(__GNUC__)

#if __GNUC__>=3 && defined(TDEBUG)

#include <stdio.h>
#include <windows.h>
#include "memory.hpp"
#include <limits.h>
#include "crt.hpp"

#define STATUS_INVALID_PARAMETER (0xC000000DL)

static unsigned long dummy_iob=0x00474244;
unsigned long *_imp___iob=&dummy_iob;
void *_imp__malloc=(void *)z_malloc;
void *_imp__free=(void *)z_free;
void *_imp__realloc=(void *)z_realloc;
void *_imp__strdup=(void *)z_strdup;
void *_imp__strtol=(void *)strtol;
void *_imp__fprintf=(void *)fprintf;

typedef void (* p_atexit_fn )(void);

#ifdef __cplusplus
extern "C"
{
#endif
  int printf(const char *,...);
  int fprintf(FILE *,const char *,...);
  size_t fwrite(const void *,size_t,size_t,FILE *);
  char *strcpy(char *,const char *);
  size_t strlen(const char *);
  char *strncpy(char *,const char *,size_t);
  long strtol(const char *nptr,char **endptr,int base);
  int *_errno(void);
  int fflush(FILE *);
  p_atexit_fn __dllonexit(p_atexit_fn,p_atexit_fn **,p_atexit_fn **);
#ifdef __cplusplus
};
#endif

int printf(const char *,...)
{
  RaiseException(STATUS_INVALID_PARAMETER,0,0,NULL);
  return 0;
}

int fprintf(FILE *,const char *,...)
{
  RaiseException(STATUS_INVALID_PARAMETER,0,0,NULL);
  return 0;
}

static int isalpha(int c)
{
  return ((c>='A'&&c<='Z')||(c>='a'&&c<='z')?1:0);
}

static int isupper(int c)
{
  return (c>='A'&&c<='Z'?1:0);
}

long strtol(const char *nptr,char **endptr,int base)
{
  const char *s=nptr;
  unsigned long acc;
  int c;
  unsigned long cutoff;
  int neg = 0,any,cutlim;
  do
  {
    c=*s++;
  } while(isspace(c));
  if(c=='-')
  {
    neg=1;
    c=*s++;
  }
  else if(c=='+')
    c=*s++;
  if((base==0||base==16)&&c=='0'&&(*s=='x'||*s=='X'))
  {
    c=s[1];
    s+=2;
    base=16;
  }
  if(base==0)
    base=c=='0'?8:10;

  cutoff=neg?-(unsigned long)LONG_MIN:LONG_MAX;
  cutlim=cutoff % (unsigned long)base;
  cutoff/=(unsigned long)base;
  for(acc=0,any=0;;c=*s++)
  {
    if(isdigit(c))
      c-='0';
    else if(isalpha(c))
      c-=isupper(c)?'A'-10:'a'-10;
    else
      break;
    if(c>=base)
      break;
    if(any<0||acc>cutoff||(acc==cutoff&&c>cutlim))
      any=-1;
    else
    {
      any=1;
      acc*=base;
      acc+=c;
    }
  }
  if(any<0)
    acc=neg?LONG_MIN:LONG_MAX;
  else if(neg)
    acc=-acc;
  if(endptr!=0)
    *endptr=any?(char *)s-1:(char *)nptr;
  return acc;
}

size_t fwrite(const void *,size_t,size_t,FILE *)
{
  RaiseException(STATUS_INVALID_PARAMETER,0,0,NULL);
  return 0;
}

char *strcpy(char *dest,const char *src)
{
  return lstrcpy(dest,src);
}

size_t strlen(const char *src)
{
  return lstrlen(src);
}

char *strncpy(char *dest,const char *src,size_t size)
{
  return lstrcpyn(dest,src,size+1);
}

int *_errno(void)
{
  static int e=0;
  return &e;
}

int fflush(FILE *) //called
{
  return 0;
}

p_atexit_fn __dllonexit(p_atexit_fn,p_atexit_fn **,p_atexit_fn **) //called
{
  return NULL;
}

#endif

#endif
