/*
 thebat! mailbox access plugin for MailView
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
#ifndef ___TheBat_H___
#define ___TheBat_H___

/*
  format of thebat! message base (messages.tbb):

  TBBFileHeader FileHdr;

        TBBMsgHeader  MsgHdr1;
        char          MsgBody1[ MsgHdr1.MsgSize ];

        TBBMsgHeader  MsgHdr2;
        char          MsgBody2[ MsgHdr.MsgSize ];

        ...
        ...

        TBBMsgHeader  MsgHdrN;
        char          MsgBodyN[ MsgHdrN.MsgSize ];
*/

struct TBBFileHeader
{
  DWORD Signature;         // signature (TBFH_SIGNATURE)
  DWORD StructSize;        // = 0x0C08 = 3080 bytes
  BYTE  Reserved[ 0xC00 ]; //
};

struct TBBMsgHeader
{
  DWORD Signature;         // signature (TBMH_SIGNATURE)
  DWORD StructSize;        // = 0x30 = 48 bytes
  DWORD Unknown1;          // ?? possible CRC ??
  DWORD Received;          // received date and time in unix form
  DWORD Id;                // number of message in message base
  DWORD Flags;             // flags (TBF_xxx)
  DWORD Encoding;          // internal thebat! encoding number
  DWORD ColorGrp;          // color group
  DWORD Priority;          // priority (one of TBP_xxx)
  DWORD MsgSize;           // message size
  DWORD Reserved[ 0x002 ]; // must be 0L
};

#define TBFH_SIGNATURE   0x19790620
#define TBMH_SIGNATURE   0x19700921

// Flags
#define TBF_DELETED      (1<<0)
#define TBF_READED       (1<<1)
#define TBF_REPLIED      (1<<2)
#define TBF_PARKED       (1<<3)
#define TBF_UNKNOWN1     (1<<4)
#define TBF_UNKNOWN2     (1<<5)
#define TBF_FLAGGED      (1<<6)
#define TBF_FORWARDED    (1<<7)

// Priority
#define TBP_NORMAL       0x00000000
#define TBP_HIGH         0x00000005
#define TBP_LOW          0xFFFFFFFB // -5

#endif //!defined(___TheBat_H___)
