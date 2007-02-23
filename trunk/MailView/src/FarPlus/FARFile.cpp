/* $Header: /cvsroot/farplus/FARPlus/FARFile.cpp,v 1.6 2002/05/30 06:27:40 yole Exp $
   FAR+Plus: lightweight file class implementation
   (c) 2002 Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>
   Portions Copyright (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
*/

#include "FARFile.h"

FarFile::FarFile()
  :	fHandle    ( INVALID_HANDLE_VALUE ),
	fLastError ( ERROR_SUCCESS )
{
}

FarFile::FarFile( LPCSTR FileName, DWORD AccessMode, DWORD ShareMode, DWORD HowToCreate )
  :	fHandle    ( INVALID_HANDLE_VALUE ), 
	fLastError ( ERROR_SUCCESS )
{
	Open( FileName, AccessMode, ShareMode, HowToCreate );
}

FarFile::~FarFile()
{
	Close();
}

bool FarFile::Open( LPCSTR FileName, DWORD AccessMode, DWORD ShareMode, DWORD HowToCreate,
				   DWORD Attribute /* = FILE_ATTRIBUTE_NORMAL */) 
{ 
	Close();
	
	far_assert( FileName );

	fHandle = CreateFile( FileName, AccessMode, ShareMode, NULL, HowToCreate, 
		Attribute, NULL );
	
	if ( fHandle == INVALID_HANDLE_VALUE )
	{
		fLastError = GetLastError();
		if ( fLastError == 0 )                    
			fLastError = ERROR_FILE_NOT_FOUND;
		return false;
	}

	return true;
}

void FarFile::Close()
{
	if ( fHandle != INVALID_HANDLE_VALUE )
	{
		CloseHandle( fHandle );
		fHandle = INVALID_HANDLE_VALUE;
	}
}

//////////////////////////////////////////////////////////////////////////

FarMemoryMappedFile::FarMemoryMappedFile()
  :	FarFile(),
	fMapping ( NULL ),
	fMapPtr  ( NULL ),
	fCurPtr  ( NULL ),
	fSize    ( 0 )
{
}

FarMemoryMappedFile::FarMemoryMappedFile( LPCSTR FileName, DWORD Access, DWORD ShareMode, DWORD HowToCreate )
  :	FarFile  ( FileName, Access, ShareMode, HowToCreate ),
	fMapping ( NULL ),
	fMapPtr  ( NULL ),
	fCurPtr  ( NULL ),
	fSize    ( 0 )  
{
	Open( FileName, Access, ShareMode, HowToCreate );
}

FarMemoryMappedFile::~FarMemoryMappedFile()
{
	Close();
}

bool FarMemoryMappedFile::Open( LPCSTR FileName, DWORD AccessMode, DWORD ShareMode, DWORD HowToCreate, 
							   DWORD Attribute) 
{
	if ( FarFile::Open( FileName, AccessMode, ShareMode, HowToCreate, Attribute) == false )
	{
		return false;
	}
	
	fMapping = CreateFileMapping( GetHandle(), NULL,
		( AccessMode & GENERIC_WRITE ) ? PAGE_READWRITE : PAGE_READONLY, 0, 0, NULL );
	
	if ( fMapping == NULL )
	{
		fLastError = GetLastError();
		FarFile::Close();
		return false;
	}
	
	fMapPtr = MapViewOfFile( fMapping, 
		( AccessMode & GENERIC_WRITE ) ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0 );
	
	if ( fMapPtr == NULL )
	{
		fLastError = GetLastError();
		CloseHandle (fMapping );
		fMapping = NULL;
		FarFile::Close();
		return false;
	}
	
	fCurPtr = fMapPtr;
	fSize   = FarFile::GetSize();
	
	return true;
}

void FarMemoryMappedFile::Close()
{
	if ( fMapPtr )
	{
		UnmapViewOfFile( fMapPtr );
		fMapPtr = NULL;
	}
	if ( fMapping )
	{
		CloseHandle( fMapping );
		fMapping = NULL;
	}
	FarFile::Close();
}

DWORD FarMemoryMappedFile::Seek( LONG Offset, DWORD Orign, PLONG lpOffsetHigh )
{
	// big files support is not implemented yet.
	if ( lpOffsetHigh )
		*lpOffsetHigh = 0;

	switch ( Orign )
	{
	case FILE_CURRENT:
		{
			fCurPtr = (LPBYTE)fCurPtr + Offset;
			break;
		}
	case FILE_END:
		{
			fCurPtr = (LPBYTE)fMapPtr + fSize + Offset;
			break;
		}
	case FILE_BEGIN:
		{
			fCurPtr = (LPBYTE)fMapPtr + Offset;
			break;
		}
	default:
		{
			return (DWORD)-1;
		}
	}
	
	if ( fCurPtr < fMapPtr )
		fCurPtr = fMapPtr;
	else if (fCurPtr > ((LPBYTE) fMapPtr) + fSize)
		fCurPtr = (LPBYTE) fMapPtr + fSize;
	
	return (LPBYTE)fCurPtr - (LPBYTE)fMapPtr;
}

DWORD FarMemoryMappedFile::Read( LPVOID Buffer, DWORD Size )
{
	far_assert( Size >= 0 );
	far_assert( Buffer != 0 );

	DWORD Pos = GetPosition();
	if ( Pos + Size >= fSize )
		Size = fSize - Pos;

	memcpy (Buffer, fCurPtr, Size );

	return Size;
}

DWORD FarMemoryMappedFile::Write( LPCVOID Buffer, DWORD Size )
{
	return 0; // !!! not implemented
}

// -- FarTextFile implementation ---------------------------------------------

FarString FarTextFile::ReadLine()
{
	LPSTR MapPtr = (LPSTR)GetMemory();
	LPSTR CurPtr = (LPSTR)fCurPtr;
	DWORD MaxLen =  MapPtr + fSize - CurPtr;
	
	if ( MaxLen == 0 )
		return FarString();
	
	DWORD Len    = 0;
	LPSTR pStart = CurPtr;
	
	if ( *CurPtr == 0x0A )
	{
		CurPtr++;
	}
	else 
	{
		while ( *CurPtr != 0x0D && *CurPtr != 0x0A && *CurPtr != '\0' && Len < MaxLen )
		{
			CurPtr++;
			Len++;
		}
		
		if ( *CurPtr == 0x0D )
			CurPtr++;
		
		if ( *CurPtr == 0x0A )
			CurPtr++;

		if ( *CurPtr == '\0' )
			CurPtr++;
	}
	
	
	fCurPtr = CurPtr;
	
	return FarString(pStart, Len);
}

// -- FarFileInfo implementation ---------------------------------------------

bool FarFileInfo::FileExists (const char *fileName)
{
    WIN32_FIND_DATA FindData;
	return GetInfo (fileName, FindData);
}

bool FarFileInfo::IsDirectory (const char *fileName)
{
	DWORD dwAttr = GetFileAttributes (fileName);
	return dwAttr != (DWORD) -1 && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool FarFileInfo::GetInfo (const char *fileName, WIN32_FIND_DATA &findData)
{
    HANDLE hFind;
    if ((hFind=FindFirstFile (fileName, &findData)) != INVALID_HANDLE_VALUE) 
	{
		FindClose (hFind);
		return true;
	}
	return false;
}

DWORD FarFileInfo::GetFileSize (const char *fileName, LPDWORD lpFileSizeHigh )
{
	WIN32_FIND_DATA findData;
	if (!GetInfo (fileName, findData))
		return -1;
	//return findData.nFileSizeLow | (findData.nFileSizeHigh) << 32;
	if ( lpFileSizeHigh )
		*lpFileSizeHigh = findData.nFileSizeHigh;
	return findData.nFileSizeLow;
}

ULONGLONG FarFileInfo::GetFileSize( const char *fileName )
{
	WIN32_FIND_DATA findData;
	if (!GetInfo (fileName, findData))
		return -1;
	return findData.nFileSizeLow | (findData.nFileSizeHigh) << 32;
}
