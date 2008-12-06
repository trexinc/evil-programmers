/*
    [ESC] Editor's settings changer plugin for FAR Manager
    Copyright (C) 2001 Ivan Sintyurin
    Copyright (C) 2008 Alex Yaroslavsky

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef __myrtl_hpp
#define __myrtl_hpp

/*
Если определено _check_mem, то будет увеличиваться/уменьшаться
переменная _check_mem_DAT при вызове malloc/realloc/free
*/
#ifdef _DEBUG
#define _check_mem
#endif

#include <windows.h>

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
  wchar_t *wwmemset(wchar_t *s, wchar_t c, size_t n);
  int memcmp(const void *s1, const void *s2, size_t n);

/* string.h */
  wchar_t *wstrdup(const wchar_t *s);
  wchar_t *wstrchr(const wchar_t *s, wchar_t c);
  wchar_t *wstrrchr(const wchar_t *s, wchar_t c);
  wchar_t *wstrncpy(wchar_t *dest, const wchar_t *src, size_t n);
  int wstrcmp(const wchar_t *s1, const wchar_t *s2);
  int wstricmp(const wchar_t *s1, const wchar_t *s2);
  int wstrncmp(const wchar_t *s1, const wchar_t *s2, size_t n);
  size_t wstrlen(const wchar_t * str);
  wchar_t *wstrcat(wchar_t * dst, const wchar_t * src);
  wchar_t *wstrcpy(wchar_t * dst, const wchar_t * src);
  wchar_t *wstrpbrk (const wchar_t * string, const wchar_t * control);
  // radix может быть только 10 или 16
  // обрабатывает только положительные числа (для чисел с явно указанным знаком
  // возвращает ошибку - 0)
  long wstrtol(const wchar_t *s, wchar_t **endptr, int radix);

  div_t div(int numer, int denom);

#ifdef __cplusplus
}
#endif

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
  FILE *wfopen(const wchar_t *filename, const wchar_t *mode);
  int fclose(FILE *stream);
  size_t fread(void *ptr, size_t size, size_t n, FILE *stream);
  size_t fwrite(const void *ptr, size_t size, size_t n, FILE *stream);
#ifdef __cplusplus
}
#endif

#endif // __myrtl_hpp
