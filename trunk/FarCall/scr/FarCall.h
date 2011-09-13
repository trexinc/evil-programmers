#define malloc __malloc
#define free __free
#define realloc __realloc

#include <windows.h>
#include "plugin.hpp"

#include <initguid.h>
// {CE3FCBE0-5398-4CBE-9C99-3418EC8336B8}
DEFINE_GUID(MainGuid, 0xce3fcbe0, 0x5398, 0x4cbe, 0x9c, 0x99, 0x34, 0x18, 0xec, 0x83, 0x36, 0xb8);
// {F56A8F85-CF6D-4095-8503-BE6E6EF0ED0A}
DEFINE_GUID(PluginMenu, 0xf56a8f85, 0xcf6d, 0x4095, 0x85, 0x3, 0xbe, 0x6e, 0x6e, 0xf0, 0xed, 0xa);

#undef malloc
#undef free
#undef realloc

#ifdef DEBUG
#define DebugString(x) {OutputDebugString( __STR__(__LINE__)": " );OutputDebugString( x );OutputDebugString( "\n" );}
#define __STR2__(x) #x
#define __STR__(x) __STR2__(x)
#define assert( x ) if( !(x) ) DebugString( "Assertion "#x" in line "__STR__(__LINE__) )
#else
#define DebugString(x)
#define assert( x )
#endif

#ifdef ZeroMemory
#undef ZeroMemory
#endif

#define ZeroMemory( ptr, size ) {for( int i = 0; i < size; i++ ) *((char*)(ptr) + i) = 0;}

#define sizeofa( x ) (sizeof( x ) / sizeof( *x ))

static HANDLE heap = 0;

static void* malloc( size_t size )
{
  assert( heap );
  return HeapAlloc( heap, HEAP_ZERO_MEMORY, size );
}

static void free( void* block )
{
  assert( heap );
  if( block )
    HeapFree( heap, 0, block );
  return;
}

static void* realloc( void* block, size_t size )
{
  assert( heap );

  if( block && !size )
  {
    HeapFree( heap, 0, block );
    return 0;
  }
  else if( block && size )
    return HeapReAlloc( heap, HEAP_ZERO_MEMORY, block, size );
  else if( !block && size )
    return HeapAlloc( heap, HEAP_ZERO_MEMORY, size );
  else // !block && !size
    return 0;
}

static char* lstrdup( const char* str )
{
  if( !str )
    return 0;
  char* dup = (char*)malloc( (lstrlen( str ) + 1) * sizeof(*str) );
  if( !dup )
    return 0;
  lstrcpy( dup, str );
  return dup;
}

static wchar_t* lstrdup( const wchar_t* str )
{
  if( !str )
    return 0;
  wchar_t* dup = (wchar_t*)malloc( (lstrlenW( str ) + 1) * sizeof(*str) );
  if( !dup )
    return 0;
  lstrcpyW( dup, str );
  return dup;
}

static int lstrchr( const wchar_t* str, wchar_t sym )
{
  for( int i = 0; str[i]; i++ )
    if( str[i] == sym )
      return i;
  return -1;
}

static void lmemcpy( void* dst, void* src, int len )
{
  if( !dst || !src || !len )
    return;

  char* _src = (char*)src;
  char* _dst = (char*)dst;
  for( int i = 0; i < len; i++, _dst++, _src++ )
    *_dst = *_src;
}

static char* w2a(const char* s, int cp = CP_ACP)
{
  return lstrdup(s);
}

static char* w2a(const wchar_t* s, int cp = CP_ACP)
{
  char* res = 0;
  size_t size = 0;

  if (!s)
    return NULL;

  size = WideCharToMultiByte(cp, 0, s, -1, 0, 0, NULL, NULL) + 1;

  if (size)
  {
    res = (char*)malloc(size * sizeof(*res));
    WideCharToMultiByte(cp, 0, s, -1, res, size, NULL, NULL);
  }

  return res;
}

static wchar_t* a2w(const wchar_t* s, int cp = CP_ACP)
{
  return lstrdup(s);
}

static wchar_t* a2w(const char* s, int cp = CP_ACP)
{
  wchar_t* res = 0;
  size_t size = 0;

  if (!s)
    return NULL;

  size = MultiByteToWideChar(cp, 0, s, -1, 0, 0) + 1;

  if (size)
  {
    res = (wchar_t*)malloc(size * sizeof(*res));
    MultiByteToWideChar(cp, 0, s, -1, res, size);
  }

  return res;
}

class Handle
{
private:
  HANDLE m_Handle;
  bool   m_NoClose;
public:
  Handle() { m_Handle = 0; m_NoClose = false; }
  Handle( HANDLE handle, bool noclose = false )
  { m_Handle = handle; m_NoClose = noclose; }
  ~Handle()
  { if( m_Handle && m_Handle != INVALID_HANDLE_VALUE && ! m_NoClose ) CloseHandle( m_Handle ); m_Handle = 0; }
  operator HANDLE() { return m_Handle; }
  HANDLE* operator &() { return &m_Handle; }
  Handle& operator =(HANDLE src) {m_Handle = src; return *this;}
  bool NoClose( bool noclose ) { return m_NoClose = noclose; }
};