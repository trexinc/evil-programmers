/*
 MailView plugin for FAR Manager
 Copyright (C) 2005 Alex Yaroslavsky
 Copyright (C) 2002-2003 Dennis Trachuk

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
#include "stdafx.h"
#include <FarPlus.h>
#include "References.H"

inline LPSTR StrDup( LPCSTR string )
{
  int length = strlen( string ) + 1;
  return (LPSTR)memcpy( create char[ length ], string, length );
}

void ParseReferences( LPCSTR str, FarStringArray& result, bool in_reply_to )
{
  far_assert( str != NULL );

  if ( *str == '\0' )
    return;

  DWORD len = strlen( str ) + 1;
  LPSTR buf = (LPSTR)memcpy( create char[ len ], str, len );
  for ( LPSTR tok = strtok( buf, "\x20\t;" ); tok; tok = strtok( NULL, "\x20\t;" ) )
  {
    if ( *tok == '<' )
    {
      len = strlen( ++tok ) - 1;
      if ( len > 8 && tok[ len ] == '>' && result.IndexOf( tok ) == -1 )
        result.Add( tok, len );
    }
  }

  delete [] buf;

/*  LPSTR d = StrDup( str );
  LPSTR s = d;
  LPSTR o = NULL;
  DWORD m, l = 0;

  while ( ( s = strtok( s, "\x20\t;" ) ) != NULL)
  {
    LPSTR n = NULL;

    if ( *s == '<' )
    {
      l = strlen( ++s );

      if ( s[ l - 1 ] != '>' )
      {
        o = s;
        s = NULL;
        continue;
      }
      else
      {
        s[ l - 1 ] = '\0';
      }

      n = s; // strdup
    }
    else if ( o ) // broken message-id
    {
      m = strlen( s ) - 1;
      if ( s[ m ] == '>' )
      {
        //n = safe_malloc( sizeof(char) * ( l + m + 1 ) );
        //strcpy( n, o );       // __STRCPY_CHECKED__
        //strcpy( n + l, s );   // __STRCPY_CHECKED__
        s[ m ] = '\0';
        strcpy( o + l, s );
        n = o;
      }
    }

    if ( n )
    {
      // make sure that this really does look like a message-id.
      // it should have exactly one @, and if we're looking at
      // an in-reply-to header, make sure that the part before
      // the @ has more than eight characters or it's probably
      // an email address

//      at = strchr( n, '@' );
//      if ( at == NULL || strchr( at + 1, '@' ) || ( in_reply_to && at - n <= 8 ) )
//        safe_free( (void **)&n );
//      else
      {
        if ( result.IndexOf( n ) == -1 )
          result.Add( n );//result.Insert( 0, n );
      }
    }
    o = NULL;
    s = NULL;
  }

  delete [] d;
*/
}

//////////////////////////////////////////////////////////////////////////

CThread::CThread( LPCSTR Key, TCacheEntry * Data, CThread * Parent )
  : m_Key( Key ), m_Data( Data ),
  m_Parent( Parent ), m_Next( NULL ), m_Prev( NULL ), m_Child( NULL )
{
  far_assert( m_Key != NULL );
//  far_assert( m_Data != NULL );
  if ( m_Parent )
  {
    m_Next = m_Parent->m_Child;
    m_Parent->m_Child = this;
    if ( m_Next )
      m_Next->m_Prev = this;
  }
}

CThread::~CThread()
{
  CThread * ref = m_Child;
  while ( ref )
  {
    CThread * nxt = ref->m_Next;
    delete ref;
    ref = nxt;
  }
}

void CThread::SetChild( CThread * Ref )
{
  far_assert( Ref != this );
  if ( Ref )
  {
    if ( Ref->m_Prev )
      Ref->m_Prev->m_Next = Ref->m_Next;
    else if ( Ref->m_Parent )
      Ref->m_Parent->m_Child = Ref->m_Next;
    if ( Ref->m_Next )
      Ref->m_Next->m_Prev = Ref->m_Prev;

    Ref->m_Parent = this;
    Ref->m_Next = m_Child;
    Ref->m_Prev = NULL;
    m_Child = Ref;
  }
}

//////////////////////////////////////////////////////////////////////////
// Генерация простого числа для размера хэша.
int gen_prime( int base )
{
    // Нет смысла искать среди четных чисел, поэтому подгоняем минимум
    // под следующее нечетное или оставляем как есть.
    int atLeast = base + ( base % 2 ? 0 : 1 );

    // Перебираем кандидатов. Редко когда доведется перебрать больше
    // десяти чисел. Собственно, прямой подсчет для все простых меньше
    // 100000 дает среднюю разницу между двумя соседними ~10.4.
    // Правда для миллиона это значение уже ~12.7, но покажите мне такую
    // базу! 8)
  int i = atLeast;
    for ( ; ; i += 2)
  {
        if ( ( i % 3) == 0)
    {
            // Пpовеpяемое число делится на 3 - нет смысла в пеpебоpе делителей.
    continue;
        }
        // Если число и имеет делитель - то он явно меньше корня квадратного
        // этого числа.
  //        int divisor = (int)sqrt( i );
        int divisor = i / 2;
        // Подгоняем делитель под ближайшее сверху значение, которое допустимо
        // для перебора делителей. Допустимо - значит не кратно ни 3, ни 2.
        // Поизгалявшись можно было бы еще и кратные 5 отфутболивать, но "и
        // так неплохо". Как результат divisor может стать чуть больше корня
        // квадратного проверяемого числа, но это уже не страшно.
        divisor = ( divisor / 3 ) * 3 + 2;
        // Шаг подобран как раз такой, чтобы пропускать все, что кратно 2 и 3.
        // Другими словами, для любого не кратного 2 и 3 числа D число меньшее
        // D на произведение на 2 остатка от деления D на 3 - число не кратное
        // 2 и 3. Доказательство оставляется в качестве упражнения. 8)
        for ( ; divisor > 3; divisor -= 2 * ( divisor % 3))
    {
      // Если вдруг проверяемое число разделилось без остатка, значит
      // оно нам не подходит.
      if ( ( i % divisor) == 0)
      {
        break;
      }
        }
        // Все делители перебраны, а число не разделилось нацело; значит -
        // простое.
        if ( divisor < 3 )
    {
      break;
        }
    }
    return i;
}

CThreads::CThreads( int nTableSize ) : FarHashT( gen_prime( nTableSize ) )
{
}

CThreads::~CThreads()
{
}

DWORD CThreads::HashKey( void * Data ) const
{
  far_assert( Data != NULL );
  LPCSTR Key = ((CThread*)Data)->GetKey();

  DWORD Hash = 0;

  while ( *Key )
    Hash = ( Hash << 5 ) + Hash + *Key++;

  return Hash;
}

int CThreads::Compare( void * Data1, void * Data2 ) const
{
  far_assert( Data1 != NULL );
  far_assert( Data2 != NULL );

  LPCSTR s1 = ((CThread*)Data1)->GetKey();
  LPCSTR s2 = ((CThread*)Data2)->GetKey();

  return CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE, s1, -1, s2, -1 ) - 2;
}

bool CThreads::Insert( CThread * Data )
{
  return FarHashT::Insert( Data, NULL );
}

bool CThreads::Remove( CThread * Data )
{
  return FarHashT::Remove( Data );
}

CThread * CThreads::Find( CThread * Data )
{
  return (CThread*)FarHashT::Find( Data );
}
