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

char *strchr(const char *s, int c)
{
  while (*s)
    {
      if (*s == c)
        return const_cast<char*>(s);
      ++s;
    }
  return NULL;
}

char *strrchr(const char *s, int c)
{
  register const char *ss;
  register size_t i;

  for (i = strlen(s) + 1, ss = s + i; i; --i)
    {
      if (*(--ss) == (char) c)
        return ((char *) ss);
    }
  return NULL;
}

char *strncpy(char *dest, const char *source, size_t n)
{
  char *src = dest;

  while (n && 0 != (*dest++ = *source++))
    --n;
//  if (n)
  *dest = 0;
  return (src);
}

int strcmp(const char *s1, const char *s2)
{
  BYTE *b1 = (BYTE *) s1, *b2 = (BYTE *) s2;

  while (*b1 && *b1 == *b2)
    {
      ++b1;
      ++b2;
    }
  return (*b1 - *b2);
}

int strncmp(const char *s1, const char *s2, size_t n)
{
  if (!n)
    return (0);
  while (--n && *s1 && *s1 == *s2)
    {
      ++s1;
      ++s2;
    }
  return (*(BYTE *) s1 - *(BYTE *) s2);
}

char *strdup(const char *s)
{
  char *p=NULL;
  if(s)
  {
    int len=strlen(s)+1;
    p=static_cast<char*>(malloc(len));
    if(p)
      memcpy(p, s, len);
  }
  return p;
}

size_t strlen (const char * str)
{
        const char *eos = str;

        while( *eos++ ) ;

        return( (int)(eos - str - 1) );
}

char * strcat (char * dst, const char * src)
{
        char * cp = dst;

        while( *cp )
                cp++;                   /* find end of dst */

        while( *cp++ = *src++ ) ;       /* Copy src to end of dst */

        return( dst );                  /* return dst */

}

char *strcpy(char * dst, const char * src)
{
        char * cp = dst;

        while( *cp++ = *src++ )
                ;               /* Copy src over dst */

        return( dst );
}

int stricmp(const char *s1, const char *s2)
{
  return FSF.LStricmp(s1, s2);
}

char *strpbrk (const char * string, const char * control)
{
        const unsigned char *str = string;
        const unsigned char *ctrl = control;

        unsigned char map[32];
        int count;

        /* Clear out bit map */
        for (count=0; count<32; count++)
                map[count] = 0;

        /* Set bits in control map */
        while (*ctrl)
        {
                map[*ctrl >> 3] |= (1 << (*ctrl & 7));
                ctrl++;
        }

        /* 1st char in control map stops search */
        while (*str)
        {
                if (map[*str >> 3] & (1 << (*str & 7)))
                        return((char *)str);
                str++;
        }
        return(NULL);
}

FILE *fopen(const char *filename, const char *mode)
{
   if(!mode || !filename || !*filename)
      return NULL;
   //_D(SysLog("fopen: filename[%s], mode[%s] strpbrk%c=NULL",
   //   filename, mode, strpbrk(mode, "rwa")?'!':'='));
   if(strpbrk(mode, "rwa")==NULL)
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
   if(strchr(mode, 'r'))
   {
     //_D(SysLog("r - GENERIC_READ"));
     dwDesiredAccess|=GENERIC_READ;
     dwCreationDistribution=OPEN_EXISTING;
     R=TRUE;
   }
   if(strchr(mode, 'w'))
   {
     //_D(SysLog("w - GENERIC_WRITE"));
     dwDesiredAccess|=GENERIC_WRITE;
     dwCreationDistribution=CREATE_ALWAYS;
     W=TRUE;
   }
   if(strchr(mode, 'a'))
   {
     //_D(SysLog("a - GENERIC_WRITE"));
     dwDesiredAccess|=GENERIC_WRITE;
     dwCreationDistribution=CREATE_ALWAYS;
     A=TRUE;
   }
   if(strchr(mode, '+'))
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

inline bool __isDecimal(BYTE B)
{
  return (B>='0' && B<='9');
}

bool __isHex(BYTE B)
{
  return __isDecimal(B) || (B>='a' && B<='f') || (B>='A' && B<='F');
}

BYTE __CharToHex(BYTE Char)
{
  const dec1='A'-10, dec2='a'-10;
  if(!__isHex(Char)) return 0;
  return Char-((Char<'A')?'0':((Char<'a')?dec1:dec2));
}

long strtol(char *s, char **endptr, int radix)
{
   long result=0;
   int size, c=1;
   if(!s) return 0;
   if(endptr) *endptr=s;
   if(radix!=10 && radix!=16) return 0;
   char *start=s, *end;
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

   if(endptr) *endptr=end;
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
