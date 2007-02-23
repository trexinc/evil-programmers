/*
 MailView plugin for FAR Manager
 Copyright (C) 2005 Alex Yaroslavsky
 Copyright (C) 2002-2003 Dennis Trachuk

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef ___File_H___
#define ___File_H___

#include <windows.h>

class File
{
public:
  typedef FarString String;

private:
  static String getParent( const char * path, int length )
  {
    int index = -1;

    for ( int i = length; i >= 0; --i )
    {
      int c = path[ i ];
      if ( c == '\\' || c == '/' )
      {
        index = i;
        break;
      }
    }

    const int prefixLength = 3; // "C:\"

    if ( index < prefixLength )
    {
      if ( prefixLength > 0 && length > prefixLength )
        return String( path, prefixLength );

      return String();
    }

    return String( path, index );
  }

  static bool getTime( const char * path, LPFILETIME creation, LPFILETIME lastAccess, LPFILETIME lastWrite )
  {
    HANDLE hFile = CreateFile( path, 0, FILE_SHARE_READ,
      NULL, OPEN_EXISTING, 0, NULL );
    if ( hFile != INVALID_HANDLE_VALUE )
    {
      BOOL result = GetFileTime( hFile, creation, lastAccess, lastWrite );

      CloseHandle( hFile );

      return result != FALSE;
    }

    return false;
  }

public:

  static const char * getName( const char * path )
  {
    far_assert( path != NULL );

    const char * p = path;

    while ( *path )
    {
      if ( *path == '\\' || *path == '/' || *path== ':' )
        p = path + 1;

      path ++;
    }
    return p;
  }

  static bool exists( const char * path )
  {
    DWORD attrs = GetFileAttributes( path );
    return attrs != 0xFFFFFFFF/* && attrs != FILE_ATTRIBUTE_DIRECTORY*/;
  }

  static bool mkdir( const char * path )
  {
    //BOOL fileApisAnsi = AreFileApisANSI();

    //if ( fileApisAnsi )
    //  SetFileApisToOEM();

    bool result = CreateDirectory( path, NULL ) != 0;

    //if ( fileApisAnsi )
    //  SetFileApisToANSI();

    return result;
  }

  static bool mkdirs( const char * path )
  {
    if ( exists( path ) )
      return false;

    if ( mkdir( path ) )
      return true;

    FarString parent = getParent( path );

    return parent.Length() > 0 && File::mkdirs( parent ) && mkdir( path );
  }

  static FarString getParent( const FarString& path )
  {
    return getParent( path.c_str(), path.Length() );
  }

  static FarString getParent( const char * path )
  {
    return getParent( path, strlen( path ) );
  }

  static bool deleteit( const char * path )
  {
    return DeleteFile( path ) != 0;
  }

  static bool remove( const char * path )
  {
    return DeleteFile( path ) != 0;
  }

  static bool canRead( const char * path )
  {
    return true;
  }

  static bool canWrite( const char * path )
  {
    DWORD attrs = GetFileAttributes( path );
    return attrs != 0xFFFFFFFF && (attrs & FILE_ATTRIBUTE_READONLY) == 0;
  }

  static bool isDirectory( const char * path )
  {
    DWORD attrs = GetFileAttributes( path );
    return attrs != 0xFFFFFFFF && (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
  }

  static bool isFile( const char * path )
  {
    DWORD attrs = GetFileAttributes( path );
    return attrs != 0xFFFFFFFF && (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
  }

  static bool isHidden( const char * path )
  {
    DWORD attrs = GetFileAttributes( path );
    return attrs != 0xFFFFFFFF && (attrs & FILE_ATTRIBUTE_HIDDEN) != 0;
  }

  static bool setReadOnly( const char * path )
  {
    DWORD attrs = GetFileAttributes( path );
    if ( attrs == 0xFFFFFFFF )
      return false;
    return SetFileAttributes( path, attrs | FILE_ATTRIBUTE_READONLY ) != FALSE;
  }

  static bool lastModified( const char * path, LPFILETIME time )
  {
    return getTime( path, NULL, NULL, time );
    }

  static ULONGLONG length( const char * path )
  {
    DWORD hi, lo = length( path, &hi );

    return lo | (hi) << 32;
  }

    static DWORD length( const char * path, LPDWORD highPart )
  {
    HANDLE hFile = CreateFile( path, 0, FILE_SHARE_READ,
      NULL, OPEN_EXISTING, 0, NULL );
    if ( hFile != INVALID_HANDLE_VALUE )
    {
      DWORD result = GetFileSize( hFile, highPart );

      CloseHandle( hFile );

      if ( result == 0xFFFFFFFF )
        return 0;

      return result;
    }

    return 0;
    }
};


#endif //!defined(___File_H___)
