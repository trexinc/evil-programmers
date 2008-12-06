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
#ifndef __myrtl_cpp
#define __myrtl_cpp

#include "myrtl.hpp"
#include "plugin.hpp"
#include "syslog.hpp"

extern FarStandardFunctions FSF;

HANDLE heapNew_ESC = NULL;

#ifdef _check_mem      // проверка и подсчет выделенной памяти

DWORD _check_mem_DAT;  // счетчик, должен инициализироваться нулем при старте

void *malloc(size_t size)
{
  void *p = NULL;

  if (heapNew_ESC)
    {
      p = HeapAlloc(heapNew_ESC, HEAP_ZERO_MEMORY, size);
      if (p)
        _check_mem_DAT += HeapSize(heapNew_ESC, 0, p);
    }
  return p;
}

void *realloc(void *block, size_t size)
{
  void *p = NULL;

  if (heapNew_ESC != NULL)
    {
      if (!size)
        {
          if(block)
          {
            _check_mem_DAT -= HeapSize(heapNew_ESC, 0, block);
            HeapFree(heapNew_ESC, 0, block);
          }
        }
      else
        {
          if (block == NULL)
            p = HeapAlloc(heapNew_ESC, HEAP_ZERO_MEMORY, size);
          else
            {
              _check_mem_DAT -= HeapSize(heapNew_ESC, 0, block);
              p = HeapReAlloc(heapNew_ESC, HEAP_ZERO_MEMORY, block, size);
            }
          if (p)
            _check_mem_DAT += HeapSize(heapNew_ESC, 0, p);
        }
    }
  return p;
}

void free(void *block)
{
  if (block != NULL && heapNew_ESC)
    {
      _check_mem_DAT -= HeapSize(heapNew_ESC, 0, block);
      HeapFree(heapNew_ESC, 0, block);
    }
}
#else // обычный режим без проверки
void *malloc(size_t size)
{
  if (heapNew_ESC)
    return HeapAlloc(heapNew_ESC, HEAP_ZERO_MEMORY, size);
  return NULL;
}

void *realloc(void *block, size_t size)
{
  void *p = NULL;

  if (heapNew_ESC != NULL)
    {
      if (!size)
      {
        if(block) HeapFree(heapNew_ESC, 0, block);
      }
      else
        {
          if (block == NULL)
            p = HeapAlloc(heapNew_ESC, HEAP_ZERO_MEMORY, size);
          else
            p = HeapReAlloc(heapNew_ESC, HEAP_ZERO_MEMORY, block, size);
        }
    }
  return p;
}

void free(void *block)
{
  if (block != NULL && heapNew_ESC)
    HeapFree(heapNew_ESC, 0, block);
}
#endif // _check_mem

void qsort(void *base, size_t nelem, size_t width,
    int (_cdecl *fcmp)(const void *, const void *))
{
  FSF.qsort(base, nelem, width, fcmp);
}

void *memcpy(void *dest, const void *src, size_t n)
{
  BYTE *d = (BYTE *) dest, *s = (BYTE *) src;

  while (n--)
    {
      *d = *s;
      ++d;
      ++s;
    }
  return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
  if (n && (dest != src))
    {
      BYTE *s = (BYTE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, n);

      if (s)
        {
          memcpy(s, src, n);
          memcpy(dest, s, n);
          HeapFree(GetProcessHeap(), 0, (void *) s);
        }
    }
  return dest;
}

void *memset(void *s, int c, size_t n)
{
  BYTE *dst = (BYTE *) s;

  while (n--)
    {
      *dst = (BYTE) (unsigned) c;
      ++dst;
    }
  return s;
}

wchar_t *wwmemset(wchar_t *s, wchar_t c, size_t n)
{
  wchar_t *dst = s;

  while (n--)
    {
      *dst = c;
      ++dst;
    }
  return s;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
  if (!n)
    return (0);
  BYTE *b1 = (BYTE *) s1, *b2 = (BYTE *) s2;

  while (--n && *b1 == *b2)
    {
      ++b1;
      ++b2;
    }
  return (*b1 - *b2);
}

wchar_t *wstrchr(const wchar_t *s, wchar_t c)
{
  while (*s)
    {
      if (*s == c)
        return const_cast<wchar_t*>(s);
      ++s;
    }
  return NULL;
}

wchar_t *wstrrchr(const wchar_t *s, wchar_t c)
{
  register const wchar_t *ss;
  register size_t i;

  for (i = wstrlen(s) + 1, ss = s + i; i; --i)
    {
      if (*(--ss) == (wchar_t) c)
        return ((wchar_t *) ss);
    }
  return NULL;
}

wchar_t *wstrncpy(wchar_t *dest, const wchar_t *source, size_t n)
{
  wchar_t *src = dest;

  while (n && 0 != (*dest++ = *source++))
    --n;
//  if (n)
  *dest = 0;
  return (src);
}

int wstrcmp(const wchar_t *s1, const wchar_t *s2)
{
  while (*s1 && *s1 == *s2)
    {
      ++s1;
      ++s2;
    }
  return (*s1 - *s2);
}

int wstrncmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{
  if (!n)
    return (0);
  while (--n && *s1 && *s1 == *s2)
    {
      ++s1;
      ++s2;
    }
  return (*s1 - *s2);
}

wchar_t *wstrdup(const wchar_t *s)
{
  wchar_t *p=NULL;
  if(s)
  {
    int len=wstrlen(s)+1;
    p=static_cast<wchar_t*>(malloc(len*sizeof(wchar_t)));
    if(p)
      memcpy(p, s, len*sizeof(wchar_t));
  }
  return p;
}

size_t wstrlen (const wchar_t * str)
{
        const wchar_t *eos = str;

        while( *eos++ ) ;

        return( (int)(eos - str - 1) );
}

wchar_t * wstrcat (wchar_t * dst, const wchar_t * src)
{
        wchar_t * cp = dst;

        while( *cp )
                cp++;                   /* find end of dst */

        while( (*cp++ = *src++) != 0 ) ;       /* Copy src to end of dst */

        return( dst );                  /* return dst */

}

wchar_t *wstrcpy(wchar_t * dst, const wchar_t * src)
{
        wchar_t * cp = dst;

        while( (*cp++ = *src++) != 0 )
                ;               /* Copy src over dst */

        return( dst );
}

int wstricmp(const wchar_t *s1, const wchar_t *s2)
{
  return FSF.LStricmp(s1, s2);
}

wchar_t *wstrpbrk (const wchar_t * string, const wchar_t * control)
{
  for ( ; *string; string++)
    for (const wchar_t *wcset = control; *wcset; wcset++)
      if (*wcset == *string) return((wchar_t *)string);
  return(NULL);
}

FILE *wfopen(const wchar_t *filename, const wchar_t *mode)
{
   if(!mode || !filename || !*filename)
      return NULL;
   //_D(SysLog("fopen: filename[%s], mode[%s] strpbrk%c=NULL",
   //   filename, mode, strpbrk(mode, "rwa")?'!':'='));
   if(wstrpbrk(mode, L"rwa")==NULL)
      return NULL;
   DWORD dwDesiredAccess=0, dwCreationDistribution=0;
   BOOL R, W, A;
   R=W=A=FALSE;
/*
r   Open for reading only.
w   Create for writing. If a file by that name already exists, it will be
    overwritten.
a   Append; open for writing at end-of-file or create for writing if the file
    does not exist.
r+  Open an existing file for update (reading and writing).
w+  Create a new file for update (reading and writing). If a file by that name
    already exists, it will be overwritten.
a+  Open for append; open (or create if the file does not exist) for update at
    the end of the file.
*/
   if(wstrchr(mode, L'r'))
   {
     //_D(SysLog("r - GENERIC_READ"));
     dwDesiredAccess|=GENERIC_READ;
     dwCreationDistribution=OPEN_EXISTING;
     R=TRUE;
   }
   if(wstrchr(mode, L'w'))
   {
     //_D(SysLog("w - GENERIC_WRITE"));
     dwDesiredAccess|=GENERIC_WRITE;
     dwCreationDistribution=CREATE_ALWAYS;
     W=TRUE;
   }
   if(wstrchr(mode, L'a'))
   {
     //_D(SysLog("a - GENERIC_WRITE"));
     dwDesiredAccess|=GENERIC_WRITE;
     dwCreationDistribution=CREATE_ALWAYS;
     A=TRUE;
   }
   if(wstrchr(mode, L'+'))
   {
     //_D(SysLog("+++"));
     dwDesiredAccess|=GENERIC_WRITE;
     dwCreationDistribution=CREATE_ALWAYS;
     if(R) dwDesiredAccess|=GENERIC_WRITE;
     if(W) dwDesiredAccess|=GENERIC_READ;
   }

   HANDLE hFile=CreateFile(filename, dwDesiredAccess, FILE_SHARE_READ, NULL,
                           dwCreationDistribution, 0, NULL);
   //_D(SysLog("hFile%c=INVALID_HANDLE_VALUE", hFile==INVALID_HANDLE_VALUE?'=':'!'));
   if(hFile==INVALID_HANDLE_VALUE) return NULL;
   if(A) fseek(hFile, 0, SEEK_END);
   return hFile;
};

int fseek(FILE *stream, long offset, int whence)
{
   return SetFilePointer(stream, offset, NULL, whence)==0xFFFFFFFF;
}

long int ftell(FILE *stream)
{
   return SetFilePointer(stream, 0, NULL, FILE_CURRENT);
}

int fclose(FILE *stream)
{
   return (stream!=NULL && stream!=INVALID_HANDLE_VALUE)?
          CloseHandle(stream):TRUE;
}

size_t fread(void *ptr, size_t size, size_t n, FILE *stream)
{
   DWORD NumberOfBytesToRead=size*n, NumberOfBytesRead;
   if(!ptr || !size || !n) return 0;
   BOOL r=ReadFile(stream, ptr, NumberOfBytesToRead, &NumberOfBytesRead, NULL);
   return (!r || NumberOfBytesRead!=NumberOfBytesToRead)?0:n;
}

size_t fwrite(const void *ptr, size_t size, size_t n, FILE *stream)
{
   DWORD NumberOfBytesToWrite=n*size, NumberOfBytesWritten;
   if(!ptr || !size || !n) return 0;
   BOOL w=WriteFile(stream, ptr, NumberOfBytesToWrite, &NumberOfBytesWritten,
                    NULL);
   return (!w || NumberOfBytesWritten!=NumberOfBytesToWrite)?0:n;
}

inline bool __isDecimal(wchar_t B)
{
  return (B>=L'0' && B<=L'9');
}

bool __isHex(wchar_t B)
{
  return __isDecimal(B) || (B>=L'a' && B<=L'f') || (B>=L'A' && B<=L'F');
}

BYTE __CharToHex(wchar_t Char)
{
  const BYTE dec1=L'A'-10, dec2=L'a'-10;
  if(!__isHex(Char)) return 0;
  return Char-((Char<L'A')?L'0':((Char<L'a')?dec1:dec2));
}

long wstrtol(const wchar_t *s, wchar_t **endptr, int radix)
{
   long result=0;
   int size, c=1;
   if(!s) return 0;
   if(endptr) *endptr=(wchar_t *)s;
   if(radix!=10 && radix!=16) return 0;
   const wchar_t *start=s, *end;
   while(0x20==*start || 0x09==*start) ++start; // пропустим пробелы
   while('0'==*start) ++start;                  // пропустим нули
   if('x'==FSF.LLower((BYTE)*start))
   {
    if(radix!=16)
      return 0;
    else
      ++start;
   }
   end=start;

   if(radix==16)
   {
     while(__isHex(*end)) ++end;
     size=end-start;
     if(size)
      for(int f=size-1;f>=0;--f)
      {
        result+=__CharToHex(start[f])*c;
        c*=16;
      }
   }
   else if(radix==10)
   {
     while(__isDecimal(*end)) ++end;
     size=end-start;
     if(size)
      for(int f=size-1;f>=0;--f)
      {
        result+=__CharToHex(start[f])*c;
        c*=10;
      }
   }

   if(endptr) *endptr=(wchar_t *)end;
   return result;
/*
Description

Converts a string to a long value.
strtol converts a character string, s, to a long integer value. s is a sequence
of characters that can be interpreted as a long value; the characters must
match this generic format:

   [ws] [sn] [0] [x] [ddd]

where:

[ws]    =   optional whitespace
[sn]    =   optional sign (+ or -)
[0] =   optional zero (0)
[x] =   optional x or X
[ddd]   =   optional digits

strtol stops reading the string at the first character it doesn't recognize.

Any invalid value for radix causes the result to be 0 and sets the next
character pointer *endptr to the starting string pointer.

If the value in s is meant to be interpreted as decimal, any character other
than 0 to 9 will be unrecognized.

If the value in s is meant to be interpreted as a number in any other base,
then only the numerals and letters used to represent numbers in that base will
be recognized. (For example, if radix equals 5, only 0 to 4 will be recognized;
if radix equals 20, only 0 to 9 and A to J will be recognized.)

If endptr is not null, strtol sets *endptr to point to the character that
stopped the scan (*endptr = &stopper).

Return Value

strtol returns the value of the converted string, or 0 on error.
*/
}

div_t div (int numer, int denom)
{
  div_t result;
  result.quot=numer/denom;
  result.rem=numer-result.quot*denom;
  return result;
}

#endif //__myrtl_cpp
