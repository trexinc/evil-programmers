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
#include "StdAfx.h"
#include "MailView.h"        // FCT_DEFAULT
#include <FarFile.h>        // FarFile
#include "MailViewPlugin.h" // BAD_MSG_ID

#include "MailboxCache.h"

inline void FarFile_WriteString( FarFile& f, const FarString& s )
{
  f.WriteDword( s.Length() );
  f.Write( s.c_str(), s.Length() );
}

inline void FarFile_ReadString( FarFile& f, FarString& res )
{
  DWORD len = f.ReadDword( 0 );
  f.Read( res.GetBuffer( len ), len );
  res.ReleaseBuffer( len );
}

bool TCacheEntry::Write( FarFile& f )
{
  f.WriteDword( Handle );
  f.WriteDword( Size );

  f.WriteDword( Encoding );

  FarFile_WriteString( f, Subject );
  FarFile_WriteString( f, From );
  FarFile_WriteString( f, To );
  FarFile_WriteString( f, MessageID );

  f.WriteDword( ParentIDs.Count() );
  for ( int i = 0; i < ParentIDs.Count(); i ++ )
  {
    DWORD len = strlen( ParentIDs[ i ] );
    if ( len > 0 )
    {
      f.WriteDword( len );
      f.Write( ParentIDs[ i ], len );
    }
  }

  f.Write( &Info, sizeof( Info ) );

  return true;
}

bool TCacheEntry::Read( FarFile& f )
{
  Handle   = f.ReadDword( BAD_MSG_ID );
  Size     = f.ReadDword( 0 );

  Encoding = f.ReadDword( FCT_DEFAULT );

  FarFile_ReadString( f, Subject );
  FarFile_ReadString( f, From );
  FarFile_ReadString( f, To );
  FarFile_ReadString( f, MessageID );

  DWORD tempSize = 0;
  LPSTR tempData = NULL;

  DWORD count = f.ReadDword( 0 );
  for ( DWORD i = 0; i < count; i++ )
  {
    DWORD len = f.ReadDword( 0 );

    if ( len > 0 )
    {
      if ( tempSize < len )
      {
        if ( tempData )
          delete [] tempData;

        tempData = create char[ len + 1 ];
        tempSize = len;
      }

      f.Read( tempData, len );

      tempData[ len ] = '\0';

      ParentIDs.Add( tempData );
    }
  }

  if ( tempData )
    delete [] tempData;

  if ( f.Read( &Info, sizeof( Info ) ) != sizeof( Info ) )
    memset( &Info, 0, sizeof( Info ) );

  return true;
}

/*
CCacheEntryHash::CCacheEntryHash( int nTableSize )
: FarHashT( gen_prime( nTableSize ) )
{
}

CCacheEntryHash::~CCacheEntryHash()
{
}

DWORD CCacheEntryHash::HashKey( void * Data )
{
  far_assert( Data != NULL );
  LPCSTR Key = ((TCacheEntry*)Data)->MessageID.c_str();

  DWORD Hash = 0;

  while ( *Key )
    Hash = ( Hash << 5 ) + Hash + *Key++;

  return Hash;
}

int CCacheEntryHash::Compare( void * Data1, void * Data2 )
{
  far_assert( Data1 != NULL );
  far_assert( Data2 != NULL );

  FarString & s1 = ((TCacheEntry*)Data1)->MessageID;
  FarString & s2 = ((TCacheEntry*)Data2)->MessageID;

  return s1.CompareNoCase( s2.c_str(), s2.Length() );
}

bool CCacheEntryHash::Insert( TCacheEntry * ce )
{
  return FarHashT::Insert( ce, NULL );
}

bool CCacheEntryHash::Remove( TCacheEntry * ce )
{
  return FarHashT::Remove( ce );
}

TCacheEntry * CCacheEntryHash::Find( TCacheEntry * ce )
{
  return (TCacheEntry*)FarHashT::Find( ce );
}
*/

void TMailboxCache::remove( TCacheEntry * ce )
{
  Items.Remove( ce );
}
