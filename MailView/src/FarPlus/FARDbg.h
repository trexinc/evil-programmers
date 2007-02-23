/*************************************************************************

  FarDbg.h

  (c) 2001-02 Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>

  Revision: 1.22 [04/10/2002]

*************************************************************************/

#ifndef ___FarDbg_H___
#define ___FarDbg_H___

#if _MSC_VER >= 1000
#pragma once
#endif

#ifdef _DEBUG
#define FARPLUS_ASSERT
#define FARPLUS_TRACE
#endif

#ifdef FARPLUS_ASSERT

int far_assert_message( const char *, const int, const char *, bool );

#define far_assert_ex( Expression, bShowWinError )                                       \
do                                                                                       \
{	if ( !( Expression ) )                                                               \
		if ( far_assert_message( __FILE__, __LINE__, #Expression, bShowWinError ) == 1 ) \
			__asm { int 3 }                                                              \
} while( false )


#else

#define far_assert_ex( Expression, bShowWinError )((void)0)

#endif // FARPLUS_ASSERT

#ifdef FARPLUS_TRACE

void far_ods( char const * const Fmt, ... );

#else

inline void far_ods( char const * const Fmt, ... ) {}

#endif // FARPLUS_TRACE


#define far_trace far_ods

#define far_assert( Expression ) far_assert_ex( Expression, false )
#define far_assert_string( str, maxSize ) far_assert_ex( str && !IsBadStringPtr( str, maxSize ), true )
#define far_assert_read_ptr( ptr, size ) far_assert_ex( ptr && !IsBadReadPtr( ptr, size ), true )
#define far_assert_write_ptr( ptr, size ) far_assert_ex( ptr && !IsBadWritePtr( ptr, size ), true )

#define FAR_STACK_DUMP_TARGET_TRACE         0x0001
#define FAR_STACK_DUMP_TARGET_CLIPBOARD     0x0002
#define FAR_STACK_DUMP_TARGET_BOTH          0x0003
#define FAR_STACK_DUMP_TARGET_ODS           0x0004
#ifdef _DEBUG
#define FAR_STACK_DUMP_TARGET_DEFAULT       FAR_STACK_DUMP_TARGET_BOTH
#else
#define FAR_STACK_DUMP_TARGET_DEFAULT       FAR_STACK_DUMP_TARGET_CLIPBOARD
#endif

void FarDumpStack( unsigned long dwTarget = FAR_STACK_DUMP_TARGET_DEFAULT );

#if defined(FARPLUS_DEBUG_MEMORY) && defined(_DEBUG)

__declspec(dllimport) void * __cdecl operator new( unsigned int, int, const char *, int );
inline void * __cdecl operator new[]( unsigned int s, int, const char * f, int l )
{ return ::operator new( s, 1, f, l ); }
inline void* __cdecl operator new( unsigned int s )
{ return ::operator new( s, 1, __FILE__, __LINE__); }
inline void* __cdecl operator new[]( unsigned int s )
{ return ::operator new( s, 1, __FILE__, __LINE__); }

#if _MSC_VER >= 1200
//inline void __cdecl operator delete(void * _P, int, const char *, int)
//        { ::operator delete(_P); }
#endif  /* _MSC_VER >= 1200 */

// если исрользовать create вместо new, то после окончения
// работы плагина, если где-то забыли почистить память, в
// ODS запишется имя файла и строка где все это добро было
// выделено. (recommended, так-сказать :))
#define create new ( 1, __FILE__, __LINE__ )
#else
#define create new
#endif // FARPLUS_DEBUG_MEMORY

#endif //!defined(___FarDbg_H___)
