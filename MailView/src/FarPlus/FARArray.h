/* $Header: /cvsroot/farplus/FARPlus/FARArray.h,v 1.3 2002/09/05 06:33:05 yole Exp $
   Lightweight typesafe dynamic array class for use in FAR plugins
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
*/

#ifndef __FARLIST_H
#define __FARLIST_H

#include <stdlib.h>
#include "../plugin.hpp"
#include "FarDbg.h"
#include "FARMemory.h"

#if _MSC_VER >= 1000
#pragma once
#endif

typedef int (__cdecl *BaseCmpFunc) (const void *, const void *);
typedef int (__cdecl *BaseCmpExFunc) (const void *, const void *, void *);

class BaseFarArray
{
private:
	//BaseFarArray (const BaseFarArray &rhs);              // not implemented (for now)
    //BaseFarArray &operator= (const BaseFarArray &rhs);   // not implemented (for now)

protected:
	char *fItems;
  int fCount;
	int fAllocCount;
	int fResizeDelta;
	int fItemSize;

	BaseFarArray (int itemSize, int resizeDelta)
		: fItems (NULL), fCount (0), fAllocCount (0),
		  fResizeDelta (resizeDelta), fItemSize (itemSize)
	{
		far_assert( resizeDelta > 0 );
	};

	int BaseAdd (const void *item);
 	int BaseInsert( int nIndex, const void * item );

	void InternalClear()
	{
		fCount = 0;
		fAllocCount = 0;
		if (fItems)
		{
			free (fItems);
			fItems = NULL;
		}
	}

	int BaseIndexOf (const void *item) const
	{
		for (int i=0; i<fCount; i++)
		{
			if (memcmp (fItems + i * fItemSize, item, fItemSize) == 0)
				return i;
		}
		return -1;
	}

	void InternalSort (BaseCmpFunc cmpFunc);
	void InternalSort (BaseCmpExFunc cmpFunc, void * user);

public:
	int Count() const
	{
		return fCount;
	}

 	void Delete( int nIndex, int nCount = 1 );
};

template <class T>
class FarDataArray: public BaseFarArray
{
public:
	FarDataArray (int resizeDelta = 16)
		: BaseFarArray (sizeof (T), resizeDelta) {};

	~FarDataArray()
	{
		Clear();
	}

	void Clear()
	{
		InternalClear();
	}

	int Add (T &item)
	{
		return BaseAdd (&item);
	}

 	int Insert( int nIndex, T &item )
  	{
 		return BaseInsert( nIndex, &item );
  	}

	int IndexOf (T &item) const
	{
		return BaseIndexOf (&item);
	}

	T &At (int index)
	{
		far_assert( index >= 0 && index < fCount );
		return reinterpret_cast<T *> (fItems) [index];
	}

	T &operator[] (int index)
	{
		return At (index);
	}

	const T &operator[] (int index) const
	{
		far_assert( index >= 0 && index < fCount );
		return reinterpret_cast<const T *> (fItems) [index];
	}

	T *GetItems() const
	{
		return reinterpret_cast<T *> (fItems);
	}
};

template <class T>
class FarArray:public FarDataArray<T *>
{
private:
	typedef int (__cdecl *CmpFunc) (const T **, const T **);
	typedef int (__cdecl *CmpExFunc) (const T **, const T **, void *);

protected:
	bool fOwnsItems;

public:
	FarArray (int resizeDelta = 16)
		: FarDataArray<T *> (resizeDelta), fOwnsItems (true) {};
	~FarArray()
	{
		Clear();
	}

	void Clear()
  {
    if (fOwnsItems)
      for (int i=this->fCount-1; i >= 0; i--)
        delete this->At(i);
    this->InternalClear();
  }

	int Add (const T *item)
	{
		T *addItem = const_cast<T *> (item);
		return FarDataArray<T *>::Add (addItem);
	}

 	int Insert( int nIndex, const T * item )
 	{
 		T * insItem = const_cast<T*>( item );
 		return FarDataArray<T*>::Insert( nIndex, insItem );
 	}

	bool OwnsItems() const
	{
		return fOwnsItems;
	}

	void SetOwnsItems (bool ownsItems)
	{
		// CP-1251
		// не имеен смысла это делать при fCount > 0.
		// избежим лишний геммор, например, с потерей памяти.
		far_assert( this->fCount == 0 );
		fOwnsItems = ownsItems;
	}

	void Sort (CmpFunc cmpFunc)
	{
		this->InternalSort((BaseCmpFunc)cmpFunc);
	}

	void Sort( CmpExFunc cmpFunc, void * user )
	{
		this->InternalSort((BaseCmpExFunc)cmpFunc, user );
	}

	void Remove( T * Item )
	{
		int start = -1;
		int count = 0;
		for ( int i = 0; i < this->fCount; i ++ )
		{
			if ( this->At( i ) == Item )
			{
				if ( start == -1 )
					start = i;
				count ++;
			}
			else
			{
				if ( start != -1 )
				{
					Delete( start, count );
					start = -1;
					count = 0;
				}
			}
		}
		if ( start != -1 )
			Delete( start, count );
	}

	void Pack()
	{
		Remove( NULL );
	}

	void Delete( int nIndex, int nCount = 1 )
	{
		far_assert( nIndex >= 0 && nIndex < Count() );

		if ( nIndex + nCount > this->fCount )
			nCount = this->fCount - nIndex;

		if ( nCount <= 0 )
			return;

		if ( fOwnsItems )
		{
			for ( int i = nIndex; i < nCount; i ++ )
				delete this->At( i );
		}

		BaseFarArray::Delete( nIndex, nCount );
	}
};

class FarStringArray: public FarArray<char>
{
private:
	char *Copy (const char *item, int len = -1 )
	{
		if ( item == NULL )
			len = 0;
		else if ( len == -1 )
			len = strlen( item );
		else
			len = min( len, (int)strlen( item ) );

		char * itemCopy = (char*)memcpy( create char[ len + 1 ], item, len );

		itemCopy[ len ] = '\0';

		return itemCopy;
	}

public:
	FarStringArray (int resizeDelta = 16)
		: FarArray<char> (resizeDelta) {};
	FarStringArray ( bool bOwnItems, int resizeDelta = 16)
		: FarArray<char> (resizeDelta) { SetOwnsItems( bOwnItems ); };
	~FarStringArray()
	{
		Clear();
	}
	int Add (const char *item)
	{
		if (fOwnsItems)
			return FarArray<char>::Add (Copy (item));
		return FarArray<char>::Add (const_cast<char *> (item));
	}
	int Add( const char * szItem, int nLength )
	{
		far_assert( fOwnsItems == true );
		return FarArray<char>::Add( Copy( szItem, nLength ) );
	}
	int Insert( int nIndex, const char * item )
	{
		return FarArray<char>::Insert( nIndex,
			fOwnsItems ? Copy( item ) : const_cast<char*>( item ) );
	}
	int IndexOf( const char * item );
};

class FarIntArray: public FarDataArray<int>
{
public:
	FarIntArray (int resizeDelta = 16)
		: FarDataArray<int> (16)
	{
	};

	int Add (int i)
	{
		return FarDataArray<int>::Add (i);
	}
};

#define FarObjectArray FarArray

class FarPluginPanelItems
{
private:
	PluginPanelItem * m_Items;
	int               m_Count;
	int               m_Capacity;
protected:
	bool OwnsItems() const
	{
		return m_Capacity != -1;
	}
public:
	FarPluginPanelItems( int nInitialAlloc )
		: m_Count( 0 ), m_Capacity( nInitialAlloc )
	{
		far_assert( m_Capacity > 0 );
		m_Items = new PluginPanelItem[ m_Capacity ];
		memset( m_Items, 0, m_Capacity * sizeof( PluginPanelItem ) );
	}
	// attach
	FarPluginPanelItems( PluginPanelItem * Items, int ItemsCount, bool bOwnItems = false )
		: m_Items( Items ), m_Count( ItemsCount )
	{
		far_assert( Items != NULL );
		m_Capacity = bOwnItems ? ItemsCount : -1;
	}
	~FarPluginPanelItems()
	{
		if ( OwnsItems() )
			delete [] m_Items;
	}
	PluginPanelItem &operator[]( int nIndex )
	{
		far_assert( nIndex >= 0 && nIndex < m_Count );
		return m_Items[ nIndex ];
	}
	const PluginPanelItem &operator[]( int nIndex ) const
	{
		far_assert( nIndex >= 0 && nIndex < m_Count );
		return m_Items[ nIndex ];
	}
	int Count() const
	{
		return m_Count;
	}
	PluginPanelItem &Add()
	{
		far_assert( OwnsItems() == true );

		SetCount( m_Count + 1 );

		return m_Items[ m_Count - 1 ];
	}
	void SetCount( int nCount )
	{
		far_assert( nCount >= 0 );
		if ( nCount + 1 > m_Capacity )
		{
			m_Capacity = ( nCount + 16 ) & ~15;
			PluginPanelItem * newItems = new PluginPanelItem[ m_Capacity ];
			memcpy( newItems, m_Items, m_Count * sizeof( PluginPanelItem ) );
			if ( nCount > m_Count )
				memset( newItems + m_Count, 0, (m_Capacity - m_Count) * sizeof( PluginPanelItem ) );
			delete [] m_Items;
			m_Items = newItems;
		}
		m_Count = nCount;
	}
	PluginPanelItem * Detach()
	{
		far_assert( m_Items != NULL );

		PluginPanelItem * Items = m_Items;
		m_Items    = NULL;
		m_Count    = 0;
		m_Capacity = -1;
		return Items;
	}
	PluginPanelItem * GetItems()
	{
		return m_Items;
	}
};

#endif
