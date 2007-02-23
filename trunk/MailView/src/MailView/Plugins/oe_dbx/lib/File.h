#ifndef ___StdFile_H___
#define ___StdFile_H___

class File
{
private:
	FILE * m_hFile;

public:

	File() : m_hFile( NULL )
	{
	}
    
	~File()
	{
		close();
	}

	enum SeekMethod
	{ 
		SEEK_METHOD_BEGIN = SEEK_SET, SEEK_METHOD_CUR = SEEK_CUR, SEEK_METHOD_END = SEEK_END 
	};

	bool open( const char * fileName, const char * mode )
	{
		close();
		return ( m_hFile = fopen( fileName, mode ) ) != NULL;
	}

	void close()
	{
		if ( m_hFile )
		{
			fclose( m_hFile );
			m_hFile = NULL;
		}
	}	

	long getSize()
	{
		long curpos = getPosition();

		seekEnd();

		long endpos = getPosition();

		setPosition( curpos );

		return endpos;    
	}

	bool seek( long offset, SeekMethod whence )
	{
		if ( m_hFile == NULL )
			return false;
		return fseek( m_hFile, offset, whence ) == 0;
	}

	bool seekBegin()
	{
		return seek( 0, SEEK_METHOD_BEGIN );
	}

	bool seekEnd()
	{
		return seek( 0, SEEK_METHOD_CUR );
	}

	long getPosition()
	{
		if ( m_hFile == NULL )
			return -1;
		return ftell( m_hFile );
	}

	bool setPosition( size_t offset )
	{
		return seek( offset, SEEK_METHOD_BEGIN );
	}

	bool read( void * buffer, size_t size, size_t count = 1 )
	{
		if ( m_hFile == NULL )
			return false;
		return fread( buffer, size, count, m_hFile ) == count;
	}

	bool write( const void * buffer, size_t size, size_t count = 1 )
	{
		if ( m_hFile == NULL )
			return false;
		return fwrite( buffer, size, count, m_hFile ) == count;
	}
	
	/*__int8 readInt8( __int8 defaultValue )
	{
		__int8 result;

		if ( !read( &result, sizeof( result ) )	)
			result = defaultValue;

		return result;	
	}

	__int16 readInt16( __int16 defaultValue )
	{
		__int16 result;

		if ( !read( &result, sizeof( result ) )	)
			result = defaultValue;

		return result;	
	}*/
	
	template <class T> T readValue( T defaultValue )
	{
		T result;

		if ( !read( &result, sizeof( result ) )	)
			result = defaultValue;

		return result;	
	}
	
	void flush()
	{
		if ( m_hFile )
			fflush( m_hFile );
	}
};

#endif //!defined(___StdFile_H___)