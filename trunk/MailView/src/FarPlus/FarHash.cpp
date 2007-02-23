//////////////////////////////////////////////////////////////////////////
//
// FarHash.cpp
//
// Copyright (c) 2002 Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>
//

#include "FarPlus.h"
#include "FarHash.h"

DWORD FarHashT::HashKey( void * Key ) const
{
	// default identity hash - works for most primitive values
	return ((DWORD)Key) >> 4;
}

int FarHashT::Compare( void * Data1, void * Data2 ) const
{
	return Data1 < Data2 ? -1 : Data1 > Data2 ? 1 : 0;
}

void FarHashT::Destroy( void * )
{
}

FarHashT::FarHashT( int nTableSize ) : m_Size( nTableSize )
{
	if ( m_Size < 2 )
		m_Size = 3;
	m_Table = create PElement[ m_Size ];
	ZeroMemory( m_Table, m_Size * sizeof( PElement ) );
}

FarHashT::~FarHashT()
{
	RemoveAll();
	delete [] m_Table;
}

bool FarHashT::Insert( void * Data, void ** Res )
{
	DWORD Hash = HashKey( Data ) % m_Size;

	PElement Last = NULL;
	PElement Elem = m_Table[ Hash ];

	while ( Elem )
	{
		int Cmp = Compare( Data, Elem->Data );

		if ( Cmp == 0 ) // don't allow any dupes
		{
			if ( Res )
				*Res = Elem;
			return false;
		}

		if ( Cmp < 0 )
			break;

		Last = Elem;
		Elem = Elem->Next;
	}


	PElement New = create TElement;

	if ( Last )
		Last->Next = New;
	else
		m_Table[ Hash ] = New;

	New->Data = Data;
	New->Next = Elem;

	if ( Res )
		*Res = New;

	return true;
}

bool FarHashT::Remove( void * Data )
{
	DWORD Hash = HashKey( Data ) % m_Size;

	PElement   Elem = m_Table[ Hash ];
	PElement * Last = &m_Table[ Hash ];

	while ( Elem )
	{
		if ( Compare( Data, Elem->Data ) == 0 )
		{
			*Last = Elem->Next;

			Destroy( Elem->Data );

			delete Elem;

			return true; // coz we don't have a dupes
		}

		Elem = Elem->Next;
		Last = &Elem->Next;
	}

	return false;
}

void FarHashT::RemoveAll()
{
	for ( int i = 0; i < m_Size; i ++ )
	{
		PElement Elem = m_Table[ i ];
		while ( Elem )
		{
			PElement Next = Elem->Next;
			Destroy( Elem->Data );
			delete Elem;
			Elem = Next;
		}
	}

	ZeroMemory( m_Table, m_Size * sizeof( PElement ) );
}

void * FarHashT::Find( void * Data ) const
{
	DWORD Hash = HashKey( Data ) % m_Size;

	PElement Elem = m_Table[ Hash ];

	while ( Elem )
	{
		if ( Compare( Data, Elem->Data ) == 0 )
			return Elem->Data;

		Elem = Elem->Next;
	}

	return NULL;
}

struct FarStringHash_Pair
{
	LPSTR Key;
	DWORD Data;
};


DWORD FarStringHash::HashKey( void * Data ) const
{
	far_assert( Data != NULL );

	LPCSTR Key = ((FarStringHash_Pair*)Data)->Key;

	far_assert( Key != NULL );

	DWORD Hash = 0;

	if ( m_Compare == strcmp )
	{
		while ( *Key )
			Hash = ( Hash << 5 ) + Hash + *Key++;
	}
	else
	{
		while ( *Key )
			Hash = ( Hash << 5 ) + Hash + FarSF::LLower( *Key++ );
	}
	return Hash;
}


int FarStringHash::Compare( void * Data1, void * Data2 ) const
{
	LPCSTR Key1 = ((FarStringHash_Pair*)Data1)->Key;
	LPCSTR Key2 = ((FarStringHash_Pair*)Data2)->Key;
	return m_Compare( Key1, Key2 );
}

void FarStringHash::Destroy( void * Data )
{
	far_assert( Data != NULL );

	FarStringHash_Pair * Pair = (FarStringHash_Pair*)Data;

	if ( m_bOwnKeys )
		delete [] Pair->Key;

	delete Pair;
}

FarStringHash::FarStringHash( int nTableSize, bool bOwnKeys, bool bIgnoreCase )
: FarHashT( nTableSize ), m_bOwnKeys( bOwnKeys )
{
	if ( bIgnoreCase )
		m_Compare = FarSF::LStricmp;//_stricmp;
	else
		m_Compare = strcmp;
}

FarStringHash::~FarStringHash()
{
	RemoveAll();
}

bool FarStringHash::Insert( LPCSTR Key, DWORD Data )
{
	far_assert( Key != NULL );

	FarStringHash_Pair * Pair = create FarStringHash_Pair;

	Pair->Key  = (LPSTR)Key;
	Pair->Data = Data;

	if ( FarHashT::Insert( Pair, NULL ) )
	{
		if ( m_bOwnKeys )
		{
			int Len = strlen( Key ) + 1;
			Pair->Key = (LPSTR)memcpy( create char[ Len ], Key, Len );
		}
		return true;
	}

	delete Pair;

	return false;
}

bool FarStringHash::Remove( LPCSTR Key )
{
	far_assert( Key != NULL );
	FarStringHash_Pair Pair;
	Pair.Key = (LPSTR)Key;
	return FarHashT::Remove( &Pair );
}

DWORD FarStringHash::Find( LPCSTR Key ) const
{
	far_assert( Key != NULL );
	FarStringHash_Pair Pair;
	Pair.Key = (LPSTR)Key;
	FarStringHash_Pair * Res = (FarStringHash_Pair*)FarHashT::Find( &Pair );
	return Res ? Res->Data : 0;
}

DWORD& FarStringHash::operator[]( LPCSTR Key )
{
	far_assert( Key != NULL );

	FarStringHash_Pair * Pair = create FarStringHash_Pair;

	Pair->Key = (LPSTR)Key;

	FarStringHash_Pair * Res;
	if ( FarHashT::Insert( Pair, (void**)&Res ) )
	{
		if ( m_bOwnKeys )
		{
			int Len = strlen( Key ) + 1;
			Pair->Key = (LPSTR)memcpy( create char[ Len ], Key, Len );
		}
		return Pair->Data;
	}

	delete Pair;

	return Res->Data;
}
