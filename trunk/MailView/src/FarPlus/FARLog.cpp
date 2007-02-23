//////////////////////////////////////////////////////////////////////////
//
// FarLog.cpp
//
// Copyright (c) 2002 Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>
//

#include "FarLog.h"

FarLog::FarLog( const char * FileName, int Level )
: m_Level( Level ), m_File( NULL ), m_Tmp( NULL )
{
	if ( Level > 0 )
	{
		m_File = new FarFile( FileName, GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS );
		if ( m_File->GetHandle() != INVALID_HANDLE_VALUE )
			m_File->SeekEnd();
		else
		{
			delete m_File;
			m_File = NULL;
		}
	}
}

FarLog::~FarLog()
{
	if ( m_File )
		delete m_File;

	if ( m_Tmp )
		delete [] m_Tmp;
}

void FarLog::Log( int Level, char const * Fmt, va_list argPtr )
{
	if ( m_File == NULL || Level > m_Level || Fmt == NULL || *Fmt == '\0' )
		return;

	if ( m_Tmp == NULL )
		m_Tmp = create char[ 0x1000 ];

	SYSTEMTIME LocalTime; GetLocalTime( &LocalTime );

	int nLen;

	nLen = GetDateFormat( LOCALE_USER_DEFAULT, 0, &LocalTime, NULL, m_Tmp, 0x1000 ) - 1;
	m_File->Write( m_Tmp, nLen );

	m_File->Write( "\x20", 1 );

	nLen = GetTimeFormat( LOCALE_USER_DEFAULT, 0, &LocalTime, NULL, m_Tmp, 0x1000 ) - 1;
	m_File->Write( m_Tmp, nLen );

	if ( Level == 1 )
		m_File->Write( " Error: ", 8 );
	else if ( Level == 2 )
		m_File->Write( " Warning: ", 10 );
	else
		m_File->Write( "\x20", 1 );

	nLen = wvsprintf( m_Tmp, Fmt, argPtr );

	m_File->Write( m_Tmp, nLen );

	if ( m_Tmp[ nLen - 1 ] != '\n' )
		m_File->Write( "\r\n", 2 );
}

void FarLog::Error( char const * Fmt, ... )
{
	va_list argPtr;
	va_start( argPtr, Fmt );

	Log( 1, Fmt, argPtr );

	va_end( argPtr );
}

void FarLog::Warning( char const * Fmt, ... )
{
	va_list argPtr;
	va_start( argPtr, Fmt );

	Log( 2, Fmt, argPtr );

	va_end( argPtr );
}

void FarLog::Message( char const * Fmt, ... )
{
	va_list argPtr;
	va_start( argPtr, Fmt );

	Log( 3, Fmt, argPtr );

	va_end( argPtr );
}

void FarLog::Message( int Level, char const * Fmt, ... )
{
	va_list argPtr;
	va_start( argPtr, Fmt );

	Log( 3 + Level, Fmt, argPtr );

	va_end( argPtr );
}
