/* $Header: /cvsroot/farplus/FARPlus/FARArray.cpp,v 1.2 2002/04/14 10:39:29 yole Exp $
   FAR+Plus: Lightweight typesafe dynamic array class implementation
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
*/

#include <string.h>
#include "FARArray.h"
#include "FARPlus.h"

int BaseFarArray::BaseAdd (const void *item)
{
	if (fCount + 1 > fAllocCount)
	{
		fAllocCount += fResizeDelta;
		fItems = (char *) realloc (fItems, fAllocCount * fItemSize);
		memset (fItems + (fCount + 1) * fItemSize, 0, (fAllocCount - fCount - 1) * fItemSize);
	}
	memcpy (fItems + fCount * fItemSize, item, fItemSize);
	return fCount++;
}

// BaseAdd() можно (или нужно) заменить на:
// int BaseFarArray::BaseAdd (const void *item)
// {
//	BaseInsert( fCount, item );
// }
int BaseFarArray::BaseInsert( int nIndex, const void * item )
{
	far_assert( nIndex >=0 && nIndex <= fCount );

	if ( fCount + 1 > fAllocCount )
	{
		fAllocCount += fResizeDelta;
		fItems = (char*)realloc( fItems, fAllocCount * fItemSize );
		memset( fItems + ( fCount + 1 ) * fItemSize, 0, ( fAllocCount - fCount - 1 ) * fItemSize );
	}

	if ( nIndex == fCount )
	{
		memcpy( fItems + fCount * fItemSize, item, fItemSize );
	}
	else
	{
		memmove( fItems + ( nIndex + 1 ) * fItemSize, fItems + nIndex * fItemSize,
			( fCount - nIndex ) * fItemSize );

		memcpy( fItems + nIndex * fItemSize, item, fItemSize );
	}

	return fCount++;
}

void BaseFarArray::InternalSort (BaseCmpFunc cmpFunc)
{
	FarSF::qsort( fItems, fCount, fItemSize, cmpFunc );
}

#include "qsortex.h"
//extern "C" void __cdecl qsortex( void * base, unsigned num, unsigned width,
//	int (__cdecl *comp)(const void *, const void *, void *), void * );

void BaseFarArray::InternalSort( BaseCmpExFunc cmpFunc, void * user )
{
	qsortex( (void *)fItems, fCount, fItemSize, (comp_t*)cmpFunc, (void *)user );
//	FarSF::qsort( fItems, fCount, fItemSize, cmpFunc, user );
}

void BaseFarArray::Delete( int nIndex, int nCount )
{
	far_assert( nIndex >= 0 && nIndex < fCount );

	if ( nIndex + nCount > fCount )
		nCount = fCount - nIndex;

	if ( nCount <= 0 )
		return;

	char * Dst = fItems + nIndex * fItemSize;
	char * Src = Dst + nCount * fItemSize;

	fCount -= nCount;

	memmove( Dst, Src, ( fCount - nIndex ) * fItemSize );
}

// CP-1251
// для сортированного массива эту операцию можно значительно ускорить,
// но придется вводить дополнительный член класса (fSorted).
// плюс если fSorted == true, то Insert делать сразу в нужную позицию.
int FarStringArray::IndexOf( const char * item )
{
	far_assert( item != NULL );
	int len = strlen( item );
	for ( int i = 0; i < fCount; i++ )
	{
		if ( CompareString( LOCALE_USER_DEFAULT, 0, At( i ), -1, item, len ) == CSTR_EQUAL )
			return i;
	}
	return -1;
}
