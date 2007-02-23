#include <windows.h>
#include <stdio.h>

#include "File.h"

static const GUID CLSID_OutlookMessageDatabase = 
	{ 0xFE12ADCF, 0xFDC5, 0x6F74, { 0x66, 0xE3, 0xD1, 0x11, 0x9A, 0x4E, 0x00, 0xC0 } };

static const GUID CLSID_OutlookFoldersDatabase = 
	{ 0xFE12ADCF, 0xFDC6, 0x6F74, { 0x66, 0xE3, 0xD1, 0x11, 0x9A, 0x4E, 0x00, 0xC0 } };


	
#define _countof( x ) sizeof( x ) / sizeof( x[ 0 ] )

void printGUID( REFGUID guid )
{
	CoInitialize( 0 );
/*	wchar_t buf[ 48 ];
	int result = StringFromGUID2( guid, buf, _countof( buf ) );
	if ( result > 0 )
	{
		//buf[ result ] = 0;
		printf( "%S\n", buf );
	}
	else
	{
		printf ( "invalid guid, result id %d\n", result );
	
	}
	CoUninitialize();
*/
///*
	printf( "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		guid.Data1, guid.Data4, guid.Data3, 
		guid.Data4[ 0 ], guid.Data4[ 1 ], 
		guid.Data4[ 2 ], guid.Data4[ 3 ], 
		guid.Data4[ 4 ], guid.Data4[ 5 ], 
		guid.Data4[ 6 ], guid.Data4[ 7 ] );
//*/		
/*
	printf( "{ 0x%08X, 0x%04X, 0x%04X, { 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X } }",
		guid.Data1, guid.Data2, guid.Data3, 
		guid.Data4[ 0 ], guid.Data4[ 1 ], 
		guid.Data4[ 2 ], guid.Data4[ 3 ], 
		guid.Data4[ 4 ], guid.Data4[ 5 ], 
		guid.Data4[ 6 ], guid.Data4[ 7 ] );
*/
}

class Dbx
{
public:

	Dbx()
	{
	}

	~Dbx()
	{
	}
	
	static int main( int argc, char ** argv )
	{
		if ( argc < 1 )
		{
			printf ( "invalid args\n" );
			return 1;
		}

		return Dbx().run( argv[ 1 ] );
	}
	
	int run( const char * fileName )
	{
		File f;
	
		if ( !f.open( fileName, "rb" ) )
		{
			printf ( "invalid filename\n" );
			return 1;
		}
	
		GUID guid; memset( &guid, 0, sizeof( guid ) );
	
		if ( !f.read( &guid, sizeof( guid ) ) )
		{
			printf ( "invalid file\n" );
			return 1;
		}
	
		if ( guid == CLSID_OutlookMessageDatabase )
		{
			return displayMessageDatabase( f );
		}
		
		if ( guid == CLSID_OutlookFoldersDatabase )
		{
			return displayFoldersDatabase( f );
		}
		
		printf(  "invalid file\n" );
		
		return 1;
	}
	
	class DbxFileHeader
	{
	public:
		enum { entriesInTreePtr = 0x00C4, entriesTreePtr = 0x00E4 };

		DbxFileHeader() 
		{
			memset( buffer, 0, sizeof( buffer ) );
		}
		
		DbxFileHeader( File & f ) { read( f ); }
		
		~DbxFileHeader()
		{
		}
		
		void read( File & f )
		{
			f.seekBegin();
			f.read( buffer, sizeof( buffer ) );
		}
		
		DWORD getDwordValue( int id )
		{
			return *(LPDWORD)(buffer + id);
		}
		
		DWORD getEntriesCount()
		{
			return getDwordValue( entriesInTreePtr );
		}

	private:
		enum { fileHeaderSize = 0x24BC };
		
		BYTE buffer[ fileHeaderSize ];
	};

	class DbxTree
	{
	private:
		LPDWORD items;
		DWORD   count;
		
		enum { treeNodeSize = 0x27C };
		
		bool read( File & f, DWORD parent, DWORD address, DWORD position, DWORD count )
		{
			if ( position + count > this->count )
				return false;
				
			f.setPosition( address );
			
			DWORD treeNode[ treeNodeSize >> 2 ]; 
			memset( treeNode, 0, sizeof( treeNode ) );
			
			f.read( treeNode, sizeof( treeNode ) );
			
			if ( address != treeNode[ 0 ] )
				return false;
			if ( parent != treeNode[ 1 ] )
				return false;
				
			BYTE entries = (BYTE)((treeNode[ 4 ] >> 8 ) & 0xFF);
			if ( entries > 0x33 )
				return false;
				
			DWORD n = 0;
			
			if( treeNode[ 2 ] != 0 )
			{
				read( f, address, treeNode[ 2 ], position, treeNode[ 5 ] );
				n += treeNode[ 5 ];
			}
			
			for ( BYTE i = 0; i < entries; ++i )
			{
				LPDWORD pos = treeNode + 6 + i * 3;
				
				if( pos[ 0 ] != 0 )
				{
					DWORD value = position + (++n);
					
					if ( count > this->count )
						return false;
						
					items[ count - 1 ] = pos[ 0 ];
				}
				
				if ( pos[ 1 ] != 0 )
				{
					read( f, address, pos[ 1 ], position + n, pos[ 2 ] );
					n += pos[ 2 ];
				}
			}
			
			return n == count;				
		}
		
	public:
		DbxTree() : items( NULL ), count( 0 ) 
		{
		}

		DbxTree( File & f, DbxFileHeader & h ) : items( NULL ), count( 0 ) 
		{	
			read( f, h );
		}
		
		~DbxTree() 
		{
			if ( items )
				delete [] items;
		}
		
		void read( File & f, DbxFileHeader & h )
		{
			count = h.getEntriesCount();
			if ( items )
				delete [] items;
			items = new DWORD[ count ];
			memset( items, 0, count * sizeof( DWORD ) );
			
			DWORD addr = h.getDwordValue( DbxFileHeader::entriesTreePtr );
			
			read( f, 0, addr, 0, count );			
		}
		
		DWORD getValue( DWORD index )
		{
			return index < count ? items[ index ] : (DWORD)-1;
		}
	};
	
	class DbxIndexedInfo   
	{
	protected:
		enum { maxIndex = 0x20 };
	
	public: 
		enum DataType { dtNone = 0, dtDword = 1, dtFileTime = 2, dtString = 4, dtData = 8 };  

		DbxIndexedInfo( File & f, DWORD address )
		{
			init();
			this->address = address;
			readIndexedInfo( f );	
		}
		
		virtual ~DbxIndexedInfo()
		{
			if ( buffer )
				delete [] buffer;
		}

		DWORD getAddress() const { return address; }    
		DWORD getBodyLength() const { return bodyLength; }    
		BYTE getEntries() const { return entries; }
		BYTE getCounter() const { return counter; }    
		DWORD getIndexes() const { return indexes; }    
		bool isIndexed( BYTE index ) const { return indexes & ( 1<<index ); }  

		/*virtual const char * getIndexText( BYTE index ) const   
		{
			return NULL;
		}*/
		
		virtual DataType getIndexDataType( BYTE index ) const   
		{
			return dtNone;
		}

		BYTE * getValue( BYTE index, LPDWORD length ) const
		{
			if ( index >= maxIndex )
				return NULL;
			*length = this->length[ index ];
			
			return begin[ index ];		
		}
		
		const char * getString( BYTE index ) const
		{
			if ( index >= maxIndex )
				return NULL;
			return (const char*)begin[ index ];
		}

		DWORD getValue( BYTE index ) const
		{
			DWORD length;
			LPBYTE data = getValue( index, &length );
			DWORD value = 0;
			if ( data )
			{
				value = *((LPDWORD)data); // length>4 ignored
			        if ( length < 4 )
			        	value &= (1<<(length<<3))-1;
			}
			return value;
		}

	private: 
		void init()
		{
			bodyLength   = 0;
			objectLength = 0;
			entries      = 0;
			counter      = 0;
			buffer       = 0;
			indexes      = 0;
			for ( BYTE i = 0; i < maxIndex; ++i )
			{
				begin[ i ] = 0;
				length[ i ] = 0;
			}		
		}
		
		bool readIndexedInfo( File & f )
		{
			DWORD temp[ 3 ];
			
			f.setPosition( address );
			if ( !f.read( &temp, sizeof( temp ) ) )
				return false;

			if ( address != temp[ 0 ] )
				return false;

			bodyLength   = temp[ 1 ];
			objectLength = *((LPWORD)(temp+2));       //temp[2]& 0xffff;
			entries      = *(((LPBYTE)(temp+2))+2);       //(temp[2]>>16) & 0xff;
			counter      = *(((LPBYTE)(temp+2))+3);       //(temp[2]>>24) & 0xff;
			
			if ( buffer )
				delete [] buffer;

			buffer = new BYTE[ bodyLength ];
			
			if ( !f.read( buffer, bodyLength ) );
				return false;

			bool isIndirect = false;
			BYTE lastIndirect = 0;
			LPBYTE data = buffer + (entries << 2);

			for ( BYTE i = 0; i < entries; ++i )
			{
				DWORD value = ((LPDWORD)buffer)[ i ];
				bool isDirect = value & 0x80;            // Data stored direct
				BYTE index = (BYTE)(value & 0x7f);   // Low byte is the index
				value >>= 8;                            // Value the rest
				if ( index >= maxIndex )
					return false;
				
				if ( isDirect )
					setIndex( index, buffer + ( i << 2 ) + 1, 3 );
				else
				{
					setIndex( index, data + value );
					if ( isIndirect )
						setEnd( lastIndirect, data + value );
					isIndirect = true;
					lastIndirect = index;
				}
				
				indexes |= 1<<index;
			}
			
			if ( isIndirect )
				setEnd( lastIndirect, buffer + bodyLength );
		
		}

		void setIndex( BYTE index, LPBYTE begin, DWORD length = 0 )
		{
			if( index < maxIndex)
			{
				this->begin[ index ] = begin;
				this->length[index] = length;
			}
		}
		void setEnd( BYTE index, LPBYTE end )
		{
			if ( index < maxIndex )
				length[ index ] = end - begin[ index ];
		}

		// message info data
		DWORD address, bodyLength;
		WORD objectLength;
		BYTE entries, counter, * buffer;

		DWORD indexes;            // Bit field for the used indexes (maxIndexe bits)

		LPBYTE begin[ maxIndex ];
		DWORD length[ maxIndex ];
	};
	
	class DbxMessageInfo : public DbxIndexedInfo   
	{
	public: 
		DbxMessageInfo( File & f, DWORD address ) 
			: DbxIndexedInfo( f, address )
		{
		} 
		virtual ~DbxMessageInfo()
		{
		}

		//virtual const char * getIndexText( BYTE index ) const;
		virtual DataType getIndexDataType( BYTE index ) const
		{
			DataType dataType[ maxIndex ] = 
			{
			      dtDword , dtDword , dtFileTime, dtDword , dtDword , dtString,dtFileTime, dtString,
			      dtString, dtString, dtString  , dtString, dtString, dtString, dtString , dtNone  ,
			      dtDword , dtDword , dtFileTime, dtString, dtString, dtNone  , dtDword  , dtNone  ,
			      dtDword , dtDword , dtString  , dtString, dtData  , dtNone  , dtNone   , dtNone  
      			};

  			return index < maxIndex ? dataType[ index ] : dtNone;
		}
	};

	class DbxMessage
	{
	public:
		DbxMessage( File & f, DWORD address )
		{
			this->address = address;
			init();
			
			readMessageText( f );		
		}
		virtual ~DbxMessage()
		{
			if ( text )
				delete [] text;
		}

		DWORD getLength() const { return length; }
		
		const char * getText() const { return text; }
		
		void convert();
		
		void analyze( DWORD & headerLines, DWORD & bodyLines) const;

	private:
		void init()
		{
			length = 0;
			text   = 0;
		}
		bool readMessageText( File & f )
		{
			DWORD address = this->address, header[ 4 ];
			char * pos;
			
			while ( address )
			{
				f.setPosition( address );
				if ( !f.read( &header, 0x10 ) || address != header[ 0 ] )
					return false;
				
				length += header[ 2 ];
				address = header[ 3 ];
			}
			
			if ( length == 0 )
				return false;

			if ( text )
				delete [] text;
				
			pos = text = new char[ length + 1 ];  // +1 to terminate the text with 0x00
			
			address = this->address;
			
			while ( address )
			{
			
				f.setPosition( address );
				if ( !f.read( &header, 0x10 ) || address != header[ 2 ] )
					return false;
				
				address = header[ 3 ];
				pos += header[ 2 ];
			}
			
			*pos = 0;                           // terminate the text with 0x00
		}

		// Stores the address, the length and the text of the message
		DWORD address, length;
		char * text;
	};
	
	int displayMessageDatabase( File & f )
	{
		const BYTE miiIndex = 0x00, miiFlags = 0x01, miiMessageAddress = 0x04, miiSubject = 0x08;

		printf ( "message database:\n" );
		
		DbxFileHeader hdr( f );		

		DWORD count = hdr.getEntriesCount();
		
		printf( "count: %d\n", count );
		
		DbxTree tree( f, hdr );
		
		for ( int i = 0; i < count; i ++ )
		{
           		DbxMessageInfo messageInfo( f, tree.getValue( i ) );

			DWORD indexes = messageInfo.getIndexes();

			if ( indexes & (1<<miiMessageAddress) ) // Only if a message is stored
			{
				DbxMessage message( f, messageInfo.getValue( miiMessageAddress ) );

				printf( "message: %d\n%s\n", i, message.getText() );
			}
		}		
		
		return 0;
	}
	
	int displayFoldersDatabase( File & f )
	{
		printf ( "folder database:\n" );
		return 0;
	}
};


int main( int argc, char ** argv )
{
	return Dbx::main( argc, argv );
}