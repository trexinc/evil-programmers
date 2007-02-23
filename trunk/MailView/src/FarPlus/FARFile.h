/* $Header: /cvsroot/farplus/FARPlus/FARFile.h,v 1.6 2002/05/30 06:27:40 yole Exp $
   FAR+Plus: lightweight file class
   (c) 2002 Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>
   Portions Copyright (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
*/

#ifndef __FARFILE_H
#define __FARFILE_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include <windows.h>
#include "FARDbg.h"
#include "FARString.h"

class FarFile
{
private:
	HANDLE fHandle;
protected:
	DWORD  fLastError;
public:
	FarFile();
	FarFile( LPCSTR FileName, DWORD AccessMode, DWORD ShareMode, DWORD HowToCreate );
	virtual ~FarFile();
	
	HANDLE GetHandle() const { return fHandle; }
	DWORD GetLastError() const { return fLastError; }
	
	virtual bool Open( LPCSTR FileName, DWORD AccessMode, DWORD ShareMode, DWORD HowToCreate,
		DWORD Attribute = FILE_ATTRIBUTE_NORMAL);
	virtual void Close();
	
	virtual DWORD GetSize( LPDWORD lpSizeHigh = NULL ) const;
	virtual DWORD Seek( LONG Offset, DWORD Origin = FILE_BEGIN, PLONG lpOffsetHigh = NULL );
	
	virtual DWORD Read( LPVOID Buffer, DWORD Size ); 
	virtual DWORD Write( LPCVOID Buffer, DWORD Size );
	
	virtual bool FlushBuffers();
	
	//////////////////////////////////////////////////////////////////////////
	
	bool GetTime( LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime );
	bool SetTime( const FILETIME * CreationTime, const FILETIME * LastAccessTime, const FILETIME * LastWriteTime );
	
	//////////////////////////////////////////////////////////////////////////
	
	bool OpenForRead( LPCSTR FileName );
	bool CreateForWrite ( LPCSTR FileName, DWORD Attribute = FILE_ATTRIBUTE_NORMAL );
	bool OpenForWrite( LPCSTR FileName ) ;
	
	bool SetSize( DWORD nNewSize, DWORD nNewSizeHigh = 0 );
	
	DWORD GetPosition( LPDWORD lpPositionHigh = NULL );
	bool SetPosition( DWORD Position, LPDWORD lpPositionHigh = NULL )
	{
		return Seek( Position, FILE_BEGIN, (PLONG)lpPositionHigh ) != -1;
	}
	
	void SeekBegin();
	void SeekEnd();

	DWORD Read( LPVOID Buffer, DWORD Size, DWORD Count );
	DWORD Write( LPCVOID Buffer, DWORD Size, DWORD Count );
	
	bool IsEOF()
	{
		ULARGE_INTEGER Pos, Size;
		Pos.LowPart = GetPosition( &Pos.HighPart );
		Size.LowPart = GetSize( &Size.HighPart );
		return Pos.QuadPart >= Size.QuadPart;
		//return GetPosition() >= GetSize(); 
	}
	
	//////////////////////////////////////////////////////////////////////////
	
	DWORD ReadDword( const DWORD Default );
	__int64 ReadInt64( const __int64 Default );
	bool WriteDword( const DWORD Value );
	bool WriteInt64( const __int64 Value );
};

class FarFileEx : public FarFile
{
public:
	FarFileEx() : FarFile()
	{
	}
	FarFileEx( LPCSTR FileName, DWORD AccessMode, DWORD ShareMode, DWORD HowToCreate )
		: FarFile( FileName, AccessMode, ShareMode, HowToCreate )
	{
	}
	virtual ~FarFileEx()
	{
	}
	ULONGLONG GetSize()
	{
		ULARGE_INTEGER Size;
		Size.LowPart = FarFile::GetSize( &Size.HighPart );
		return Size.QuadPart;
	}
	bool SetSize( ULONGLONG nNewSize )
	{
		ULARGE_INTEGER Size;
		Size.QuadPart = nNewSize;
		return FarFile::SetSize( Size.LowPart, Size.HighPart );
	}
	ULONGLONG Seek( LONGLONG Offset, DWORD Origin = FILE_BEGIN )
	{
		LARGE_INTEGER Pos;
		Pos.QuadPart = Offset;
		Pos.LowPart = FarFile::Seek( Pos.LowPart, Origin, &Pos.HighPart );
		return Pos.QuadPart;
	}
	ULONGLONG GetPosition()
	{
		ULARGE_INTEGER Pos;
		Pos.LowPart = FarFile::GetPosition( &Pos.HighPart );
		return Pos.QuadPart;
	}
};

class FarMemoryMappedFile : public FarFile
{
private:
	HANDLE fMapping;
	LPVOID fMapPtr;
protected:
	LPVOID fCurPtr;
	DWORD  fSize;
public:
	FarMemoryMappedFile();
	FarMemoryMappedFile( LPCSTR FileName, DWORD Access, DWORD ShareMode, DWORD HowToCreate );
	virtual ~FarMemoryMappedFile();

	LPVOID GetMemory() const { return fMapPtr; }
	
	virtual bool Open( LPCSTR FileName, DWORD AccessMode, DWORD ShareMode, DWORD HowToCreate,
		DWORD Attribute = FILE_ATTRIBUTE_NORMAL);
	virtual void Close();
	
	virtual DWORD GetSize( LPDWORD lpSizeHigh = NULL ) const
	{ 
		if ( lpSizeHigh )
			*lpSizeHigh = 0;
		return fSize; 
	}
	virtual DWORD Seek( LONG Offset, DWORD Orign, PLONG lpOffsetHigh );

	virtual DWORD Read( LPVOID Buffer, DWORD Size ); 
	virtual DWORD Write( LPCVOID Buffer, DWORD Size );
};

class FarTextFile : public FarMemoryMappedFile
{
public:
	FarTextFile() : FarMemoryMappedFile() {}
	FarTextFile( LPCSTR FileName, DWORD Access, DWORD ShareMode, DWORD HowToCreate )
		: FarMemoryMappedFile( FileName, Access, ShareMode, HowToCreate ) {}
	virtual ~FarTextFile() {}

	FarString ReadLine();
};

class FarFileInfo
{
public:
	static bool FileExists (const char *FileName);
	static bool IsDirectory (const char *FileName);
	static bool GetInfo (const char *FileName, WIN32_FIND_DATA &findData);
	static DWORD GetFileSize (const char *FileName, LPDWORD lpFileSizeHigh = NULL );
	static ULONGLONG GetFileSize( const char *FileName );
};

// -- FarFile implementation -------------------------------------------------

#define FAR_INLINE inline

FAR_INLINE bool FarFile::OpenForRead( LPCSTR FileName )
{
	return Open( FileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING );
}

FAR_INLINE bool FarFile::CreateForWrite (LPCSTR FileName, DWORD Attribute)
{
	return Open( FileName, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS, Attribute);
}

FAR_INLINE bool FarFile::OpenForWrite( LPCSTR FileName )
{
	return Open( FileName, GENERIC_WRITE, FILE_SHARE_READ, OPEN_EXISTING );
}

FAR_INLINE DWORD FarFile::GetSize( LPDWORD lpSizeHigh ) const
{
	far_assert( fHandle != INVALID_HANDLE_VALUE );
	return GetFileSize( fHandle, lpSizeHigh );
}

FAR_INLINE DWORD FarFile::Seek( LONG Offset, DWORD Origin, PLONG lpOffsetHigh )
{
	far_assert( fHandle != INVALID_HANDLE_VALUE );
	DWORD Pos = SetFilePointer( fHandle, Offset, lpOffsetHigh, Origin );
	if ( Pos == (DWORD)-1 )
	{
		fLastError = GetLastError();
	}
	return Pos;
}

FAR_INLINE DWORD FarFile::Read( LPVOID Buffer, DWORD Size )
{
	far_assert( fHandle != INVALID_HANDLE_VALUE );
	DWORD readBytes;
	if ( ReadFile( fHandle, Buffer, Size, &readBytes, NULL ) == FALSE )
	{
		fLastError = GetLastError();
		readBytes = 0;
	}
	return readBytes;
}

FAR_INLINE DWORD FarFile::Write( LPCVOID Buffer, DWORD Size )
{ 
	far_assert( fHandle != INVALID_HANDLE_VALUE );
	DWORD written;
	if ( WriteFile( fHandle, Buffer, Size, &written, NULL ) == FALSE )
	{
		fLastError = GetLastError();
		written = 0;
	}
	return written;
}

FAR_INLINE bool FarFile::GetTime( LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime )
{
	if ( GetFileTime( fHandle, CreationTime, LastAccessTime, LastWriteTime ) == FALSE )
	{
		fLastError = GetLastError();
		return false;
	}
	return true;
}

FAR_INLINE bool FarFile::SetTime( const FILETIME * CreationTime, const FILETIME * LastAccessTime, const FILETIME * LastWriteTime )
{
	if ( SetFileTime( fHandle, CreationTime, LastAccessTime, LastWriteTime ) == FALSE )
	{
		fLastError = GetLastError();
		return false;
	}
	return true;
}

FAR_INLINE bool FarFile::FlushBuffers()
{
	far_assert( fHandle != INVALID_HANDLE_VALUE );
	if ( FlushFileBuffers( fHandle ) == FALSE )
	{
		fLastError = GetLastError();
		return false;
	}
	return true;
}

FAR_INLINE void FarFile::SeekBegin()
{
	Seek( 0, FILE_BEGIN );
}

FAR_INLINE void FarFile::SeekEnd()
{
	Seek( 0, FILE_END );
}

FAR_INLINE DWORD FarFile::GetPosition( LPDWORD lpPositionHigh )
{
	return Seek( 0, FILE_CURRENT, (PLONG)lpPositionHigh );
}

FAR_INLINE bool FarFile::SetSize( DWORD nNewSize, DWORD nNewSizeHigh )
{
	far_assert( fHandle != INVALID_HANDLE_VALUE );
	DWORD SizeHigh = nNewSizeHigh;	
	if ( Seek( nNewSize, FILE_BEGIN, (PLONG)&SizeHigh ) != nNewSize || SizeHigh != nNewSizeHigh )
		return false;
	if (!SetEndOfFile (fHandle))
	{
		fLastError = GetLastError();
		return false;
	}
	return true;
}

FAR_INLINE DWORD FarFile::Read( LPVOID Buffer , DWORD Size, DWORD Count )
{
	return Read( Buffer, Size * Count );
}

FAR_INLINE DWORD FarFile::Write( LPCVOID Buffer, DWORD Size, DWORD Count )
{
	return Write( Buffer, Size * Count );
}

FAR_INLINE DWORD FarFile::ReadDword( const DWORD Default )
{
	DWORD v;
	return Read( &v, sizeof( DWORD ) ) == sizeof( DWORD ) ? v : Default;
}

FAR_INLINE __int64 FarFile::ReadInt64( const __int64 Default )
{
	__int64 v;
	return Read( &v, sizeof( __int64 ) ) == sizeof ( __int64 ) ? v : Default;
}

FAR_INLINE bool FarFile::WriteDword( const DWORD Value )
{
	return Write( &Value, sizeof( DWORD ) ) == sizeof( DWORD );
}

FAR_INLINE bool FarFile::WriteInt64( const __int64 Value )
{
	return Write( &Value, sizeof( __int64 ) ) == sizeof ( __int64 );
}

//////////////////////////////////////////////////////////////////////////
#endif
