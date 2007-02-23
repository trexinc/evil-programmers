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
#ifndef ___References_H___
#define ___References_H___

#include <FarHash.h>

// adds to end of array
void ParseReferences( LPCSTR str, FarStringArray& result, bool in_reply_to );

struct TCacheEntry;

class CThread
{
private:
  CThread     * m_Parent;
  CThread     * m_Child;
  CThread     * m_Next;
  CThread     * m_Prev;

  LPCSTR        m_Key;
  TCacheEntry * m_Data;

public:
  CThread( LPCSTR Key, TCacheEntry * Data, CThread * Parent = NULL );
  ~CThread();

  LPCSTR GetKey() { return m_Key; }
  void SetKey( LPCSTR Key ) { far_assert( Key != NULL ); m_Key = Key; }
  TCacheEntry * GetData() const { return m_Data; }

  void SetChild( CThread * Ref );

  int GetChildCount() const
  {
    int count = 0;
    for ( CThread * ref = m_Child; ref; ref = ref->m_Next )
      count ++;
    return count;
  }

  CThread * GetNext  () { return m_Next; }
  CThread * GetChild () { return m_Child; }
  CThread * GetParent() { return m_Parent; }
};

class CThreads : public FarHashT
{
private:
  virtual DWORD HashKey( void * ) const;
  //virtual void  Destroy( void * );
  virtual int   Compare( void *, void * ) const;

public:
  CThreads( int nTableSize );
  virtual ~CThreads();

  bool Insert( CThread * Data );
  bool Remove( CThread * Data );

  CThread * Find( CThread * Data );

  //TCacheEntry * operator[]( TCacheEntry * ce );
};

#endif //!defined(___References_H___)
