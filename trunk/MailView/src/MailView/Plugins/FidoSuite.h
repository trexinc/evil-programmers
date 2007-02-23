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
#ifndef ___FidoSuite_H___
#define ___FidoSuite_H___

#include <pshpack1.h>

struct TFTNAddr
{
  WORD Zone;
  WORD Net;
  WORD Node;
  WORD Point;
  TFTNAddr &operator = ( const TFTNAddr& src )
  {
    memcpy( this, &src, sizeof( TFTNAddr ) );
    return * this;
  }
};

#define BAD_FTNADDR_FIELD (WORD)-1

struct TFTNDateTime
{
  UINT    da:5;     // Day of the month (1-31)
  UINT    mo:4;     // Month (1 = January, 2 = February, and so on)
  UINT    yr:7;     // Year offset from 1980 (add 1980 to get actual year)
  UINT    ss:5;     // Second divided by 2
  UINT    mm:6;     // Minute (0-59)
  UINT    hh:5;     // Hour (0-23 on a 24-hour clock)
  bool Check()
  {
    if ( !( hh <= 23 ) )
      return false;
    if ( !( mm <= 59 ) )
      return false;
    if ( !( ss <= 29 ) )
      return false;

    if ( !( da >= 1 && da <= 31 ) )
      return false;
    if ( !( mo >= 1 && mo <= 12 ) )
      return false;
    if ( !( yr <= 99 ) )
      return false;

    return true;
  }
  void ToSystemTime( LPSYSTEMTIME st )
  {
    st->wDay    = da;
    st->wMonth  = mo;
    st->wYear   = yr + 1980;
    st->wSecond = ss;
    st->wMinute = mm;
    st->wHour   = hh;

    st->wDayOfWeek    = 0;
    st->wMilliseconds = 0;
  }
  void ToFileTime( LPFILETIME ft )
  {
    SYSTEMTIME st;
    ToSystemTime( &st );
    if ( !SystemTimeToFileTime( &st, ft ) )
      *(PINT64)ft = 0ll;
  }
};

typedef char TFTNName[ 36 ];
typedef char TFTNSubj[ 72 ];
typedef char TFTNDate[ 20 ];

struct TFidoMessageHeader   // Opus
{
  TFTNName     From;      // From name
  TFTNName     To;      // To name
  TFTNSubj     Subject;   // Message subject
  TFTNDate     Date;      // ASCII date information
  WORD         timesRead;   // Number of times read
  WORD         destNode;    // Destination node number
  WORD         origNode;    // Origination node number
  WORD         cost;      // Unit cost charged to send the message
  WORD         origNet;   // Origination net number
  WORD         destNet;   // Destination net number
  TFTNDateTime Written;   // Message written date/time stamp
  TFTNDateTime Arrived;   // Message arrived date/time stamp
  WORD         replyTo;   // Msg is a reply to this msg number
  WORD         attr;      // Message attributes (FMA_)
  WORD         nextReply;   // Next message in the reply thread
};

typedef TFidoMessageHeader * PFidoMessageHeader;

struct TFidoMessageHeader2    // FTS001.16
{
  TFTNName     From;      // From name
  TFTNName     To;      // To name
  TFTNSubj     Subject;   // Message subject
  TFTNDate     Date;      // ASCII date information
  WORD         timesRead;   // Number of times read
  WORD         destNode;    // Destination node number
  WORD         origNode;    // Origination node number
  WORD         cost;      // Unit cost charged to send the message
  WORD         origNet;   // Origination net number
  WORD         destNet;   // Destination net number
  WORD         destZone;    // Destination zone number
  WORD         origZone;    // Origination zone number
  WORD         destPoint;   // Destination point number
  WORD         origPoint;   // Origination point number
  WORD         replyTo;   // Msg is a reply to this msg number
  WORD         attr;      // Message attributes (FMA_)
  WORD         nextReply;   // Next message in the reply thread
};

typedef TFidoMessageHeader2 * PFidoMessageHeader2;

#include <poppack.h>

#define FMH_SIGNATURE 0x19792301

struct TFTNMessageHeader
{
  DWORD    Signature;   // FMH_SIGNATURE
  DWORD    StructSize;
  TFTNName From;
  TFTNName To;
  TFTNSubj Subject;
  TFTNAddr AddrFrom;
  TFTNAddr AddrTo;
  DWORD    MessageId;
  DWORD    InReplyTo;
};

typedef TFTNMessageHeader * PFTNMessageHeader;

#endif //!defined(___FidoSuite_H___)
