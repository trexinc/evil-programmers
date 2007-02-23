/*
 pkt access plugin for MailView
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
#ifndef ___pkt_H___
#define ___pkt_H___

#include "../FidoSuite.h"

#include <pshpack1.h>

// Packet header structure (FSC-0048.002)
struct TPacketFileHeader
{
  WORD  origNode;           // Origination node number
  WORD  destNode;           // Destination node number
  WORD  Year;               // Year of packet creation (e.g. 1999)
  WORD  Month;              // Month of packet creation (0-11 for Jan-Dec)
  WORD  Day;                // Day of packet creation (1-31)
  WORD  Hour;               // Hour of packet creation (0-23)
  WORD  Minute;             // Minute of packet creation (0-59)
  WORD  Second;             // Second of packet creation (0-59)
  WORD  Baud;               // Max baud rate of orig and dest
  WORD  Type;               // Packet type (PKT_TYPE2)
  WORD  origNet;            // Origination net number
  WORD  destNet;            // Destination net number
  BYTE  prodCodeLow;        // Product code (low)
  BYTE  prodRevMajor;       // Product revision (major)
  CHAR  Password[8];        // Password (null padded)
  WORD  origZone0;          // Origination zone number (ZMail,QMail)
  WORD  destZone0;          // Destination zone number (ZMail,QMail)
  WORD  auxNet;             // Auxiliary net number
  WORD  capWordCopy;        // Byte swapped copy of capability word
  BYTE  prodCodeHigh;       // Product code (high)
  BYTE  prodRevMinor;       // Product revision (minor)
  WORD  capWord;            // Capability word
  WORD  origZone;           // Origination zone number (FrontDoor)
  WORD  destZone;           // Destination zone number (FrontDoor)
  WORD  origPoint;          // Origination point number
  WORD  destPoint;          // Destination point number
  CHAR  achData[4];         // Product specific data
};
typedef TPacketFileHeader * PPacketFileHeader;
// Packet type

#define PKT_TYPE2       0x0002  // Always version 2

// Packet capability word

#define PCAP_TYPE2PLUS  0x0001u // Type-2+ packet
#define PCAP_RFC822     0x8000u // Allows RFC-822

// FMA product code... to be ascquired from FSC

#define FMA_PRODCODE    0x00FE  // None allocated

// Packed message header structure mostly used

struct TPacketMessageHeader
{
  WORD     Type;               // Packed message type  (PMH_TYPE2)
  WORD     origNode;           // Origination node number
  WORD     destNode;           // Destination node number
  WORD     origNet;            // Origination net number
  WORD     destNet;            // Destination net number
  WORD     Attr;               // Message attributes (FMA_)
  WORD     Cost;               // Unit cost charged to send the message
  TFTNDate Date;               // ASCII date information
//  CHAR    achTo[max 36];           // To name (asciiz)
//  CHAR    achBy[max 36];           // From name (asciiz)
//  CHAR    achRe[max 72];           // Message subject (asciiz)
};

typedef TPacketMessageHeader * PPacketMessageHeader;

// Packed message stamp

#define PMH_TYPE2   0x0002      // Always version 2

#include <poppack.h>

#endif //!defined(___pkt_H___)
