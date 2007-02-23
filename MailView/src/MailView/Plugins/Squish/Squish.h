/*
 squish mailbox access plugin for MailView
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
#ifndef ___Squish_H___
#define ___Squish_H___

#include "../FidoSuite.h"

#include <pshpack1.h>

// Squish base header structure at the beginning of <area>.sqd
struct TSquishFileHeader
{
  DWORD StructSize;   // size of this header (256 bytes)
  DWORD MsgCount;     // number of messages in area
  DWORD MsgHigh;      // highest msg in area. same as MsgCount.

  DWORD SkipMsg;      // number of first messages to keep
  DWORD HighWaterMark;    // hight water mark message unique id
  DWORD NextId;     // next unique message id to use

  char  Base[ 80 ];   // message base path
  DWORD First;      // first frame offset
  DWORD Last;     // last free frame offset
  DWORD FirstFree;    // first free frame offset
  DWORD LastFree;     // last free frame offset
  DWORD EndOfFile;    // eof
  DWORD MaxAreaMsg;   // max number of messages to keep
  WORD  KeepDays;     // number of days to keep message
  WORD  FrameSize;    // size of the squish frame header

  char  Reserved[ 124 ];    // reserved
};

typedef TSquishFileHeader * PSquishFileHeader;

enum TSquishFrameType
{
  sftUsed   = 0x0000,     // Used frame
  sftFree   = 0x0001,     // Free frame
  sftLZSS   = 0x0002,     // N/A
  sftUpdate = 0x0003,     // Frame is being updated
};

#define SFH_SIGNATURE 0xAFAE4453

// Squish frame header which goes before each and every message
struct TSquishFrameHeader
{
  DWORD Signature;      // signature (SFH_SIGNATURE)
  DWORD Next;         // next frame offset
  DWORD Prev;         // previous frame offset

  DWORD Length;       // frame size not including this header
  DWORD MsgLength;      // msg size (msg header, ctrl info and text)
  DWORD CtrlLength;     // message control info size - суммаpная длина кладжей (Cludges)
  WORD  Type;         // frame type (one of TSquishFrameType)
  WORD  Reserved;       // reserved
};

typedef TSquishFrameHeader * PSquishFrameHeader;

// Squish message header structure
struct TSquishMessageHeader
{
  DWORD        Flags;     // message flags

  TFTNName     From;      // отпpавитель письма
  TFTNName     To;      // адpесат письма
  TFTNSubj     Subject;   // тема письма

  TFTNAddr     AddrFrom;    // адpес отпpавителя письма
  TFTNAddr     AddrTo;    // адpес получателя письма

  TFTNDateTime Written;   // written date/time (UTC)
  TFTNDateTime Arrived;   // arrived date/time (UTC)

  WORD         UTCOffset;   // author's UTC offset in minutes
  DWORD        ReplyTo;   // предыдущее письмо в цепочке репли-линков
  DWORD        NextReply[ 9 ];// следующие письма в цепочке репли-линков
  DWORD        MessageId;     // msg uid if (attr & SMA_UMSG)
  TFTNDate     Date;      // obsolete date information
};

typedef TSquishMessageHeader * PSquishMessageHeader;

#include <poppack.h>
#endif //!defined(___Squish_H___)
