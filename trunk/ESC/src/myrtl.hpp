#ifndef __myrtl_hpp
#define __myrtl_hpp

#ifdef __BORLANDC__
  /* objbase.h */
  #define _OBJBASE_H_
  /* oleauto.h */
  #define _OLEAUTO_H_
  /* oleidl.h */
  #define __oleidl_h__
  /* ole2.h */
  #define _OLE2_H_
  /* stdlib.h */
  #define __STDLIB_H
  /* new.h */
  #define __NEW_H
  /* alloc.h */
  #define __ALLOC_H
  /* memory.h */
  #define __STD_MEMORY
  /* mem.h */
  #define __MEM_H
  /* stddefs.h */
  #define __RWSTDDEFS_H__

  #ifndef INVALID_FILE_ATTRIBUTES
  #define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
  #endif

#endif // __BORLANDC__

/*
Если определено _check_mem, то будет увеличиваться/уменьшаться
переменная _check_mem_DAT при вызове malloc/realloc/free
*/
#ifdef _DEBUG
#define _check_mem
#endif

#ifndef _SIZE_T
  #define _SIZE_T
  typedef unsigned size_t;
#endif

#ifdef __cplusplus
extern "C"
{
#endif
/* stdlib.h */
  void qsort(void *base, size_t nelem, size_t width,
    int (_cdecl *fcmp)(const void *, const void *));
  void *malloc(size_t size);
  void *realloc(void *block, size_t size);
  void free(void *block);

/* mem.h */
  void *memcpy(void *dest, const void *src, size_t n);
  void *memmove(void *dest, const void *src, size_t n);
  void *memset(void *s, int c, size_t n);
  int memcmp(const void *s1, const void *s2, size_t n);

/* string.h */
#define __STRING_H
#define __STDC__
#define __STDIO_H
  char *strdup(const char *s);
  char *strchr(const char *s, int c);
  char *strrchr(const char *s, int c);
  char *strncpy(char *dest, const char *src, size_t n);
  int strcmp(const char *s1, const char *s2);
  int stricmp(const char *s1, const char *s2);
  int strncmp(const char *s1, const char *s2, size_t n);
  size_t strlen(const char * str);
  char *strcat(char * dst, const char * src);
  char *strcpy(char * dst, const char * src);
  char *strpbrk (const char * string, const char * control);
  // radix может быть только 10 или 16
  // обрабатывает только положительные числа (для чисел с явно указанным знаком
  // возвращает ошибку - 0)
  long strtol(char *s, char **endptr, int radix);

  typedef struct {int quot, rem;} div_t;
  div_t div(int numer, int denom);

#ifdef __cplusplus
}
#endif

#include <windows.h>

#ifdef __cplusplus
template <class T>inline const T&Min(const T &a, const T &b) { return a<b?a:b; }
template <class T>inline const T&Max(const T &a, const T &b) { return a>b?a:b; }
#endif

#ifdef __cplusplus
extern "C"
{
#endif
  typedef void FILE;
  int fseek(FILE *stream, long offset, int whence);
  long int ftell(FILE *stream);
  FILE *fopen(const char *filename, const char *mode);
  int fclose(FILE *stream);
  size_t fread(void *ptr, size_t size, size_t n, FILE *stream);
  size_t fwrite(const void *ptr, size_t size, size_t n, FILE *stream);
#ifdef __cplusplus
}
#endif

#endif // __myrtl_hpp
