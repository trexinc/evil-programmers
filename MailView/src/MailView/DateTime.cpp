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
#include "stdafx.h"
#include <string.h>
#include <FARPlus.h>
#include "DateTime.h"
#include <ctype.h>

const __int64 UnixTime::EPOCH_BIAS = 116444736000000000ll;

const char DateTimeUtils::dayStr[][4] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
const char DateTimeUtils::monStr[][4] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };

LPCSTR DateTimeUtils::getDayStr( const int nDay )
{
  return dayStr[ nDay ];
}

LPCSTR DateTimeUtils::getMonthStr( const int nMonth )
{
  return monStr[ nMonth ];
}

int DateTimeUtils::getMonthNum( LPCSTR szMonth )
{
  const char monthStr[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

  char * p = strstr( monthStr, szMonth );

  if ( p != NULL )
    return ( p - monthStr ) / 3;

  return 0; // bad month
}

WORD DateTimeUtils::strToWord( LPCSTR ptr, LPCSTR * end )
{
  WORD total = 0;
  bool minus = false;

  while ( isspace( *ptr ) ) ptr ++;

  if ( *ptr == '-' )
  {
    ptr ++;
    minus = true;
  }
  else if ( *ptr == '+' )
  {
    ptr ++;
  }

  while ( isdigit( *ptr ) )
    total = 10 * total + ( *ptr++ - '0' );

  *end = ptr;

  if ( minus )
    total = -total;
  return total;
}

void DateTimeUtils::RFCMailDateToFileTime( LPCSTR src, FILETIME& dst )
{
  const char tzoneStr[] = "PSTPDTMSTMDTCSTCDTESTEDT";
  const char GMTStr  [] = "GMT";
  const char UTStr   [] = "UT";
  const char ZonesStr[] = "MLKIHGFEDCBA.NOPQRSTUVWXY";

  char mbuf[ 4 ] = "\0\0\0";
  char zbuf[ 6 ] = "\0\0\0\0\0";

  LPCSTR p = src;

  SYSTEMTIME sys;

  sys.wDay = strToWord( p, &p );
  while ( isspace( *p ) != 0 ) p++; // skip all spaces
  strncpy( mbuf, p, 3 );
  while ( isspace( *p ) == 0 ) p++; // skip non spaces
  sys.wYear = strToWord( p, &p );
  sys.wHour = strToWord( p, &p );
  if ( *p == ':' )
  {
    sys.wMinute = strToWord( ++p, &p );
    if ( *p == ':' )
      sys.wSecond = strToWord( ++p, &p );
    else
      sys.wSecond = 0;
    while ( isspace( *p ) != 0 ) p++; // skip all spaces
    if ( isdigit( *p ) || *p == '-' || *p == '+' )
    {
      strncpy( zbuf, p, 5 );
      zbuf[ 5 ] = 0;
    }
  }
  else
  {
    sys.wMinute = sys.wSecond = 0;
  }

  if ( sys.wYear < 100 )
  {
    if ( sys.wYear < 70 )
      sys.wYear += 2000;
    else
      sys.wYear += 1900;
  }

  sys.wMonth = DateTimeUtils::getMonthNum( mbuf ) + 1;

  sys.wDayOfWeek    = 0;
  sys.wMilliseconds = 0;

  SystemTimeToFileTime( &sys, &dst );

  long bias;

  if ( *zbuf == '+' || *zbuf == '-' )
  {
    bias = 0;
    p = zbuf + 1;
    while ( isdigit( *p ) )
    {
      bias = 10 * bias + ( *p - '0' );
      p ++;
    }

    bias = bias / 100 * 60 + bias % 100; // minutes
    if ( *zbuf == '-' )
      bias = -bias;
  }
  else if ( strncmp( zbuf, UTStr, 2 ) == 0 || strncmp( zbuf, GMTStr, 2 ) == 0 )
  {
    bias = 0;
  }
  else
  {
    zbuf[ 4 ] = 0;
    if ( strlen( zbuf ) == 3 && (p = strstr( tzoneStr, zbuf )) != NULL && ( p - tzoneStr ) % 3 == 0 )
    {
      bias = ( ( ( p - tzoneStr ) / 3 + 1 ) / 2 - 8 ) * 60; // minutes
    }
    else if ( zbuf[ 1 ] == 0 )
    {
      p = strchr( ZonesStr, *zbuf );
      if ( p )
        bias = ( p - ZonesStr - 12 ) * 60; // minutes
      else
        bias = 0;
    }
  }

  if ( bias != 0 )
    *(__int64*)&dst -= bias * 10000000ll * 60; // seconds + 10000000
}

void DateTimeUtils::UnixDateToFileTime( LPCSTR src, FILETIME& dst )
{
  while ( *src != '\0' && isspace( *src ) == 0 ) src ++; // day of week
  while ( *src != '\0' && isspace( *src ) != 0 ) src ++; // spaces

  char mbuf[ 4 ] = "\0\0\0";
  strncpy( mbuf, src, 3 );

  SYSTEMTIME sys;

  sys.wMonth = DateTimeUtils::getMonthNum( mbuf ) + 1;

  while ( *src != '\0' && isspace( *src ) == 0 ) src ++; // month

  sys.wDay  = strToWord( src, &src );
  sys.wHour = strToWord( src, &src );
  if ( *src == ':' )
  {
    sys.wMinute = strToWord( src+1, &src );
    if ( *src == ':' )
      sys.wSecond = strToWord( src+1, &src );
  }

  while ( *src != '\0' && isspace( *src ) != 0 ) src ++; // spaces

  sys.wYear = strToWord( src, &src );

  if ( sys.wYear < 100 )
  {
    if ( sys.wYear < 70 )
      sys.wYear += 2000;
    else
      sys.wYear += 1900;
  }

  sys.wDayOfWeek    = 0;
  sys.wMilliseconds = 0;

  SystemTimeToFileTime( &sys, &dst );
}

void DateTimeUtils::MailDateToFileTime( LPCSTR src, FILETIME& dst )
{
  if ( src == NULL || *src == '\0' )
  {
    *(__int64*)&dst = 0;
    return;
  }

  LPCSTR p = strchr( src, ',' );
  if ( p )
    RFCMailDateToFileTime( p+1, dst );
  else
  {
    while ( *src != '\0' && isspace( *src ) != 0 ) src ++; // skip spaces
    if ( isdigit( *src ) )
      RFCMailDateToFileTime( src, dst );
    else
      UnixDateToFileTime( src, dst );
  }

}

void FileTime::toMailDate( char Date[32] ) const
{
  int bias = 0;

/*  TIME_ZONE_INFORMATION ZoneInfo;
  DWORD tzState = GetTimeZoneInformation( &ZoneInfo );

  if ( tzState != 0xFFFFFFFF )
  {
    Bias = ZoneInfo.Bias;
    if ( tzState == TIME_ZONE_ID_STANDARD )
      Bias += ZoneInfo.StandardBias;
    else if ( tzState == TIME_ZONE_ID_DAYLIGHT )
      Bias += ZoneInfo.DaylightBias;
  }

  FILETIME ftLocal;
  if ( FileTimeToLocalFileTime( utcTime, &ftLocal ) == FALSE )
    *(unsigned __int64*)&ftLocal = *(unsigned __int64*)utcTime;
*/
  SYSTEMTIME stLocal;
  if ( !toSystemTime( stLocal ) )
    SystemTime::now( stLocal );

  bias = -(bias / 60 * 100 + bias % 60);

  FarSF::sprintf( Date, "%s, %d %s %d %02d:%02d:%02d %+05d",
    DateTimeUtils::getDayStr( stLocal.wDayOfWeek ),
    stLocal.wDay, DateTimeUtils::getMonthStr( stLocal.wMonth - 1 ), stLocal.wYear,
    stLocal.wHour, stLocal.wMinute, stLocal.wSecond, bias );
}
