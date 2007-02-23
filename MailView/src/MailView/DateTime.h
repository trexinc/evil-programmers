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
#ifndef ___DateTime_H___
#define ___DateTime_H___

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _WINDOWS_
#include <windows.h>
#endif

class SystemTime : public SYSTEMTIME
{
public:
  SystemTime()
  {
    memset( this, 0, sizeof( this ) );
  }

  SystemTime( const SYSTEMTIME& st )
  {
    memcpy( this, &st, sizeof( this ) );
  }

  static void now( SYSTEMTIME& st )
  {
    GetSystemTime( &st );
  }

#ifdef __FARPLUS_H
  FarString getDate( LPCSTR fmt = NULL, LCID locale = LOCALE_USER_DEFAULT ) const
  {
    FarString date;

    DWORD size = GetDateFormat( locale, 0, this, fmt, NULL, 0 );

    date.ReleaseBuffer( GetDateFormat( locale, 0, this, fmt, date.GetBuffer( size ), size ) - 1 );

    return AreFileApisANSI() ? date : date.ToOEM();
  }

  FarString getTime( LPCSTR fmt = NULL, LCID locale = LOCALE_USER_DEFAULT ) const
  {
    FarString time;

    DWORD size = GetTimeFormat( locale, 0, this, fmt, NULL, 0 );

    time.ReleaseBuffer( GetTimeFormat( locale, 0, this, fmt, time.GetBuffer( size ), size ) - 1 );

    return AreFileApisANSI() ? time : time.ToOEM();
  }

  FarString toString( LPCSTR dateFmt = NULL, LPCSTR timeFmt = NULL, LCID locale = LOCALE_USER_DEFAULT ) const
  {
    return getDate( dateFmt, locale ) + '\x20' + getTime( timeFmt, locale );
  }
#endif
};

class FileTime : public FILETIME
{
public:
  FileTime()
  {
    *((__int64*)this) = 0ll;
  }

  FileTime( const FILETIME& ft )
  {
    *((__int64*)this) = *((__int64*)&ft);
  }

  static void now( FILETIME& ft )
  {
    GetSystemTimeAsFileTime( &ft );
  }

  bool toLocal( FILETIME& ft ) const
  {
    return FileTimeToLocalFileTime( this, &ft ) != FALSE;
  }

  bool toSystemTime( SYSTEMTIME& st ) const
  {
    return FileTimeToSystemTime( this, &st ) != FALSE;
  }

  void toMailDate( char Date[ 32 ] ) const;

#ifdef __FARPLUS_H
  FarString toMailDate() const
  {
    FarString date;
    toMailDate( date.GetBuffer( 31 ) );
    date.ReleaseBuffer();
    return date;
  }

  FarString getDate( LPCSTR fmt = NULL, LCID locale = LOCALE_USER_DEFAULT )
  {
    FarString date;

    if ( *(__int64*)this != 0ll )
    {
      FileTime ft; toLocal( ft );

      SystemTime st; ft.toSystemTime( st );

      date = st.getDate( fmt, locale );
    }

    return date;
  }


  FarString getTime( LPCSTR fmt = NULL, LCID locale = LOCALE_USER_DEFAULT )
  {
    FarString time;

    if ( *(__int64*)this != 0ll )
    {
      FileTime ft; toLocal( ft );

      SystemTime st; ft.toSystemTime( st );

      time = st.getTime( fmt, locale );
    }

    return time;
  }

  FarString toString( LPCSTR dateFmt = NULL, LPCSTR timeFmt = NULL, LCID locale = LOCALE_USER_DEFAULT )
  {
    if ( *(PINT64)this == 0ll )
      return FarString();

    FileTime ft; toLocal( ft );

    SystemTime st; ft.toSystemTime( st );

    return st.toString( dateFmt, timeFmt, locale );
  }

#endif
};

class UnixTime
{
private:
  // Number of 100 nanosecond units from 01.01.1601 to 01.01.1970
  static const __int64 EPOCH_BIAS;

  long time;

public:
  UnixTime()
  {
    time = 0;
  }

  UnixTime( const long time )
  {
    this->time = time;
  }

  /*static UnixTime now()
  {
    return UnixTime( time( NULL ) );
  }*/

  void toFileTime( FILETIME & ft ) const
  {
    *((PINT64)&ft) = EPOCH_BIAS + time * 10000000ll;
  }

  FileTime toFileTime() const
  {
    __int64 ft = EPOCH_BIAS + time * 10000000ll;
    return FileTime( *(LPFILETIME)&ft );
  }
};

class DateTimeUtils
{
private:
  static const char dayStr[][4];
  static const char monStr[][4];

  static WORD strToWord( LPCSTR ptr, LPCSTR * end );
  static void UnixDateToFileTime( LPCSTR src, FILETIME& dst );
  static void RFCMailDateToFileTime( LPCSTR src, FILETIME& dst );

public:
  static LPCSTR getDayStr( const int nDay );     // 0 - Sun, 1 - Mon, etc.
  static LPCSTR getMonthStr( const int nMonth ); // 0 - Jan, 1 - Feb, etc.
  static int getMonthNum( LPCSTR szMonth );      // 0 - Jan, 1 - Feb, etc.

  static void MailDateToFileTime( LPCSTR src, FILETIME& dst );

  static FileTime MailDateToFileTime( LPCSTR src )
  {
    FileTime ft;
    MailDateToFileTime( src, ft );
    return ft;
  }
};

class Ticks
{
private:
  DWORD ticks;

public:
  Ticks() : ticks( 0 )
  {
  }

  Ticks( const DWORD _ticks ) : ticks( _ticks )
  {
  }

  operator DWORD() const
  {
    return ticks;
  }

  static DWORD now()
  {
    return GetTickCount();
  }

  void toFileTime( FILETIME& ft ) const
  {
    __int64 tft = ticks * 10000ll;
    ft = *(LPFILETIME)&tft;
  }

  FileTime toFileTime() const
  {
    __int64 tft = ticks * 10000ll;
    return FileTime( *(LPFILETIME)&tft );
  }
};


#endif //!defined(___DateTime_H___)
