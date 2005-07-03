#include <windows.h>

#if defined(__GNUC__)
static unsigned long long next=0;

int Random(int x)
{
  next=next*0x5deece66dLL+11;
  if (x==0) return 0;
  return ((int)(((next>>16)&0x7fffffff)%x));
}

void Randomize(void)
{
  next=GetTickCount();
}
#else
#undef __seed_t
typedef struct
{
  unsigned lo;
  unsigned hi;
} __seed_t;

static __seed_t Seed = { 1, 0 };

void Randomize(void)
{
  Seed.lo = GetTickCount();
  Seed.hi = 0;
}

int Random(int x)
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
  if (x==0) return 0;
  return((int)(_ESI & 0x7fffffff)%x);
}
#endif

char *strstr(const char * str1, const char * str2 )
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

#if !defined(__GNUC__)
typedef void _cdecl(*constCdecl)  (void *);
typedef void _pascal(*constPascal)  (void *);
typedef void _fastcall(*constFastcall)  (void *);
typedef void _stdcall(*constStdcall)  (void *);

void *_vector_new_ldtc_(void *ptr,     // address of array, 0 means allocate
                        size_t size,   // size of each object
                        unsigned count,// how many objects
                        unsigned mode, // actual type of constructor
                        void *cons,    // addr of the constructor
                        unsigned dtorMode,// actual type of destructor
                        void *dtor     // addr of the destructor
)
/* This routine is used to initialize an array of class type.  If ptr is
   NULL, it allocates the space for the array first.

   The interpretation of the low 3 bits of the mode parameter is:

   reserved        0x00
   _cdecl          0x01
   _pascal         0x02
   _fastcall       0x03
   _fortran        0x04
   _stdcall        0x05
   _syscall        0x06
   reserved        0x07

   The remaining bits of mode are:

   deallocate      0x08
   stored count    0x10
   save dtorcnt    0x200
 */
{
  (void) dtor;
  (void *)dtorMode;

  int eltCount;

  if (ptr == 0)
  {
    // if stored count flag then allocate extra space for count
    ptr = operator new[]((size * count) + ((mode & 0x10) ? sizeof(count) : 0));

    if (ptr == 0)
      return 0;
  }

  if (mode & 0x10)                // if stored count
  {
    *(unsigned *) ptr = count;
    ptr = ((char *) ptr + sizeof(count));
  }

  if (!cons)
    return (ptr);

  eltCount = count;
  for (char *p = (char *)ptr; eltCount-- > 0; p += size)
  {
    unsigned callmode = mode & 0x07;        // strip out all flags except call type

    switch (callmode)
    {
      case 1: (*(constCdecl)    cons) ((void *) p); break;
      case 2: (*(constPascal)   cons) ((void *) p); break;
      case 3: (*(constFastcall) cons) ((void *) p); break;
      case 5: (*(constStdcall)  cons) ((void *) p); break;
      default:
        break;
    }

  }

  return ptr;
}

typedef void _cdecl(*destCdecl) (void *, int);
typedef void _pascal(*destPascal) (int, void *);
typedef void _fastcall(*destFastcall) (void *, int);
typedef void _stdcall(*destStdcall) (void *, int);

void *_vector_delete_ldtc_(void *ptr,       // address of array (always needed)
                           size_t size,     // size of each object
                           unsigned count,  // how many objects
                           unsigned mode,   // How to call
                           void *dest      // destructor address
)
/* This routine is used to destroy an array of class type.  If mode is
   set, it deallocates the space for the array afterwards.

   The interpretation of the low 3 bits of the mode parameter is:

   reserved        0x00
   _cdecl          0x01
   _pascal         0x02
   _fastcall       0x03
   _fortran        0x04    // Not used
   _stdcall        0x05
   _syscall        0x06
   reserved        0x07

   The remaining bits of mode are:

   deallocate      0x08
   stored count    0x10
 */
{
  int dealloc = mode & 0x08;
  int stored_count = mode & 0x10;

  if (ptr == 0)
    return (void *) 0;

  if (stored_count)   // if stored count
    count = *((unsigned *) ptr - 1);

  mode &= 0x07;   // strip out all flags except call type

  for (char *p = (char *)ptr + size * (count - 1); count-- > 0; p -= size)
  {
    switch (mode)
    {
      case 1: (*(destCdecl)    dest) ((void *) p, 2); break;
      case 2: (*(destPascal)   dest) (2, (void *) p); break;
      case 3: (*(destFastcall) dest) ((void *) p, 2); break;
      case 5: (*(destStdcall)  dest) ((void *) p, 2); break;
      default:
        break;
    }
  }

  if (stored_count)
    ptr = (char *) ptr - sizeof(count);

  if (dealloc)
    operator delete[] ((void *) ptr);
  else
    return (void *) ptr;

  return (void *) 0;
}
#endif
