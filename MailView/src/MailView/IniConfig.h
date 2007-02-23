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
#ifndef ___IniConfig_H___
#define ___IniConfig_H___

#include <FarPlus.h>

class CIniConfig
{
private:
  FarString m_IniFile;
  char * m_temp;
public:
  LPCSTR GetIniFileName() const
  {
    return m_IniFile;
  }
public:
  CIniConfig( const FarString IniFile ) : m_IniFile( IniFile ), m_temp( create char[ 42 ] )
  {
  }
  int ReadInt( LPCSTR Section, LPCSTR Key, int Default )
  {
    return GetPrivateProfileInt( Section, Key, Default, m_IniFile );
  }
  bool ReadBool( LPCSTR Section, LPCSTR Key, bool Default )
  {
    return ReadInt( Section, Key, Default ? TRUE : FALSE ) != FALSE;
  }
  int ReadString( LPCSTR Section, LPCSTR Key, LPCSTR Default, LPSTR Return, int nSize )
  {
    return GetPrivateProfileString( Section, Key, Default, Return, nSize, m_IniFile );
  }
  int ReadData( LPCSTR Section, LPCSTR Key, LPVOID Data, UINT nSize )
  {
    return GetPrivateProfileStruct( Section, Key, Data, nSize, m_IniFile );
  }
  void WriteInt( LPCSTR Section, LPCSTR Key, int Value )
  {
    WriteString( Section, Key, FarSF::itoa( Value, m_temp, 10 ) );
  }
  void WriteBool( LPCSTR Section, LPCSTR Key, bool Value )
  {
    WriteInt( Section, Key, Value ? TRUE : FALSE );
  }
  void WriteString( LPCSTR Section, LPCSTR Key, LPCSTR Value )
  {
    far_assert( Section != NULL && *Section != 0 );
    far_assert( Key != NULL && *Key != 0 );
    if (Value != NULL && strchr(Value, '\x20'))
      WritePrivateProfileString( Section, Key, '\"' + FarString(Value) + '\"', m_IniFile );
    else
      WritePrivateProfileString( Section, Key, Value, m_IniFile );
  }
  void WriteData( LPCSTR Section, LPCSTR Key, LPVOID Data, UINT Size )
  {
    far_assert( Section != NULL && *Section != 0 );
    far_assert( Key != NULL && *Key != 0 );
    WritePrivateProfileStruct( Section, Key, Data, Size, m_IniFile );
  }
  void EraseSection( LPCSTR Section )
  {
    far_assert( Section != NULL && *Section != 0 );
    WritePrivateProfileString( Section, NULL, NULL, m_IniFile );
  }
  void EraseKey( LPCSTR Section, LPCSTR Key )
  {
    far_assert( Section != NULL && *Section != 0 );
    far_assert( Key != NULL && *Key != 0 );
    WritePrivateProfileString( Section, Key, NULL, m_IniFile );
  }
  bool IsEmptySection( LPCSTR Section )
  {
    return GetPrivateProfileSection( Section, m_temp, 32, m_IniFile ) == 0;
  }
  ~CIniConfig()
  {
    delete [] m_temp;
  }
};

#endif // !defined(___IniConfig_H___)
