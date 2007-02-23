//////////////////////////////////////////////////////////////////////////
//
// FarList.h
//
// Copyright (c) 2002 Dennis Trachuk <dennis.trachuk@nm.ru>
//
// Revision 1.0  [11/12/2002]
// 
#ifndef ___FarList_H___
#define ___FarList_H___

template<class T>
class FarListT
{
protected:
	struct ListItem
	{
		FarListT<T> * m_List;
		ListItem    * m_Prev;
		ListItem    * m_Next;
		T             m_Data;
	};

	ListItem * m_Head;
	ListItem * m_Last;
		
	DWORD      m_Size;

	void Remove( ListItem * Item )
	{
		if ( Item == NULL )
			return;
		
		if ( Item->m_Prev )
			Item->m_Prev->m_Next = Item->m_Next;
		if ( Item->m_Next )
			Item->m_Next->m_Prev = Item->m_Prev;
		
		if ( Item->m_List->m_Last == Item )
			Item->m_List->m_Last = Item->m_Prev;
		if ( Item->m_List->m_Head == Item )
			Item->m_List->m_Head = NULL;
		
		Item->m_List->m_Size --;
		
		delete Item;
	}
	FarListT() : m_Head( NULL ), m_Last( NULL ), m_Size( 0 )
	{
	}
public:
	~FarListT()
	{
		Clear();
	}
	DWORD Size() const
	{
		return m_Size;
	}
	void Append( const T& newVal )
	{
		ListItem * Item = create ListItem;
		
		Item->m_List = this;
		Item->m_Data = newVal;
		Item->m_Prev = m_Last;
		Item->m_Next = NULL;
			
		if ( m_Last == NULL )
			m_Head = Item;
		else
			m_Last->m_Next = Item;

		m_Last = Item;

		m_Size ++;
	}
	void Clear()
	{
		ListItem * Item = m_Head;
		while ( Item )
		{
			ListItem * Next = Item->m_Next;
			delete Item;
			Item = Next;
		}
		m_Head = m_Last = NULL;
	}
	bool IsEmpty() const
	{
		return m_Head != NULL;
	}
};

template<class T>
class FarPtrList : public FarListT<T*>
{
private:
	static ListItem m_EmptyListItem;
public:
	class Iterator
	{
		friend class FarPtrList<T>;
	private:
		ListItem * m_Item;
	protected:
		Iterator( ListItem * Item = NULL ) : m_Item( Item )
		{
		}
	public:
		Iterator( const Iterator& rhs ) : m_Item( rhs.m_Item )
		{
		}
		Iterator& operator=( const Iterator& rhs )
		{
			m_Item = rhs.m_Item;
			return * this;
		}
		~Iterator()
		{
		}
		T * Data() const
		{
			return m_Item ? m_Item->m_Data : NULL;
		}
		void Data( T * Data )
		{
			if ( m_Item )
				m_Item->m_Data = Data;
		}
		Iterator Prev() const
		{
			return m_Item ? Iterator( m_Item->m_Prev ) : Iterator();
		}
		Iterator Next() const
		{
			return m_Item ? Iterator( m_Item->m_Next ) : Iterator();
		}
		bool Valid() const
		{
			return m_Item != NULL;
		}
		bool operator==( const Iterator& rhs ) const
		{
			return m_Item == rhs.m_Item;
		}
		bool operator!=( const Iterator& rhs ) const
		{
			return m_Item != rhs.m_Item
		}
		bool operator!() const
		{
			return !Valid();
		}
		T * operator*() const
		{
			return Data();
		}
		Iterator& operator++()
		{
			if ( m_Item )
				m_Item = m_Item->m_Next;
			return * this;
		}
		Iterator operator++(int)
		{
			ListItem * Item = m_Item;
			++*this;
			return Iterator( Item );
		}
		Iterator& operator--()
		{
			if ( m_Item )
				m_Item = m_Item->m_Prev;
			return * this;
		}
		Iterator operator--(int)
		{
			ListItem * Item = m_Item;
			--*this;
			return Iterator( Item );
		}		
	};
	
public:
	FarPtrList() : FarListT<T*>()
	{
	}
	~FarPtrList()
	{
		Clear();
	}
	void Remove( Iterator& Iter )
	{
		FarListT<T*>::Remove( Iter.m_Item );
		Iter.m_Item = NULL;
	}
	void Pack()
	{
		ListItem * Item = m_Head;
		while ( Item )
		{
			ListItem * Next = Item->m_Next;
			if ( Item->m_Data == NULL )
				FarListT<T*>::Remove( Item );
			Item = Next;
		}
	}
	Iterator Head() const
	{
		return Iterator( m_Head );
	}
	Iterator Last() const
	{
		return Iterator( m_Last );
	}
	Iterator Find( const T * fndVal ) const
	{
		for ( ListItem * Item = m_Head; Item; Item = Item->m_Next )
			if ( Item->m_Data == fndVal )
				break;
		return Iterator( Item );
	}
	Iterator ReverseFind( const T * fndVal ) const
	{
		for ( ListItem * Item = m_Last; Item; Item = Item->m_Prev )
			if ( Item->m_Data == fndVal )
				break;
		return Iterator( Item );
	}
};

#endif // !defined(___FarList_H___)