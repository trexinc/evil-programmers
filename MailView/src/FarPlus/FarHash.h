//////////////////////////////////////////////////////////////////////////
//
// FarHash.H
//
// Copyright (c) 2002 Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>
//

#ifndef ___FarHash_H___
#define ___FarHash_H___

#if _MSC_VER >= 1000
#pragma once
#endif

#include "FarDbg.h"
#include <windows.h>

class FarHashT
{
private:
	struct TElement
	{
		void     * Data;
		TElement * Next;
	};
	typedef TElement * PElement;

	PElement * m_Table;
	int        m_Size;

protected:
	virtual DWORD HashKey( void * ) const;
	virtual void  Destroy( void * );
	virtual int   Compare( void *, void * ) const;

	FarHashT( int nTableSize );
	virtual ~FarHashT();

	bool Insert( void *, void ** );
	bool Remove( void * );

	void * Find( void * ) const;

public:
	void RemoveAll();
};

// пример использования FarHashT
// ! надо бы сделать нормальный шаблон
class FarStringHash : public FarHashT
{
private:
	bool m_bOwnKeys;
	int (__cdecl *m_Compare)(const char*,const char*);

	virtual DWORD HashKey( void * ) const;
	virtual void  Destroy( void * );
	virtual int   Compare( void *, void * ) const;

public:
	FarStringHash( int nTableSize, bool bOwnKeys = true, bool bIgnoreCase = false );
	virtual ~FarStringHash();

	bool Insert( LPCSTR Key, DWORD Data );
	bool Remove( LPCSTR Key );

	DWORD Find( LPCSTR Key ) const;

	DWORD& operator[]( LPCSTR Key ); // lookup and add if not there
};

#endif //!defined(___FarHash_H___)
