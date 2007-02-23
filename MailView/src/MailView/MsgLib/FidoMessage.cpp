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
#include "FidoMessage.h"
#include "FidoMsgPart.h"
#include "../Kludges.h"
#include <FarFile.h>


CFidoMessageT::CFidoMessageT() : CMessageT()
{
}

CFidoMessageT::~CFidoMessageT()
{
}

bool CFidoMessageT::IsMessage( LPCSTR FileName )
{
  FarFile f;
  if ( f.OpenForRead( FileName ) == false )
    return false;

  TFidoMessageHeader hdr;

  if ( f.Read( &hdr, sizeof( TFidoMessageHeader ) ) == sizeof( TFidoMessageHeader ) &&
    memchr( (void *)&hdr, 0, sizeof( TFidoMessageHeader ) ) != NULL )
  {
    return true;
  }

  return false;
}

FarString FTNAddrToString( const TFTNAddr& Value )
{
  FarString result;
  result += IntToString( Value.Zone ) + ":";
  result += IntToString( Value.Net );
  if ( Value.Node != 0 && Value.Node != BAD_FTNADDR_FIELD )
  {
    result += '/' + IntToString( Value.Node );
    if ( Value.Point != 0 && Value.Point != BAD_FTNADDR_FIELD )
      result += '.' + IntToString( Value.Point );
  }
  return result;
}

bool CFidoMessageT::Init( long Encoding )
{
  far_assert( m_Data.size > min(sizeof(TFidoMessageHeader), sizeof(TFTNMessageHeader)));


  if ( *(LPDWORD)m_Data.data == FMH_SIGNATURE )
  {
    PFTNMessageHeader hdr = (PFTNMessageHeader)m_Data.data;

    m_Subject      = hdr->Subject;
    m_From.Name    = hdr->From;
    m_To  .Name    = hdr->To;
    m_From.FTNAddr = hdr->AddrFrom;
    m_To  .FTNAddr = hdr->AddrTo;

    if ( hdr->MessageId )
      m_MsgId = FTNAddrToString( m_From.FTNAddr ) + '\x20' + IntToString( hdr->MessageId );
    else
      m_MsgId.Empty();
    //DWORD    InReplyTo;

    m_Part = create CFidoMsgPart( this,
      (LPSTR)m_Data.data + sizeof( TFTNMessageHeader ),
      m_Data.size - sizeof( TFTNMessageHeader ) );

    PPerson psn = CPerson::Create( GetKludge( K_FTN_ReplyAddr ) );
    if ( psn )
    {
      m_To.Name = psn->Name;
      m_To.Addr = psn->Addr;
      delete psn;
    }
  }
  else
  {
    PFidoMessageHeader hdr = (PFidoMessageHeader)m_Data.data;

    m_Subject   = hdr->Subject;
    m_From.Name = hdr->From;
    m_To  .Name = hdr->To;

    if ( !hdr->Written.Check() && !hdr->Arrived.Check() )
    {
      PFidoMessageHeader2 hdr2 = (PFidoMessageHeader2)hdr;
      m_From.FTNAddr.Zone  = hdr2->origZone;
      m_From.FTNAddr.Point = hdr2->origPoint;
      m_To  .FTNAddr.Zone  = hdr2->destZone;
      m_To  .FTNAddr.Point = hdr2->destPoint;

      *(PINT64)&m_Info.Sent     = 0ll;
      *(PINT64)&m_Info.Received = 0ll;
    }
    else
    {
      m_From.FTNAddr.Zone  = BAD_FTNADDR_FIELD;
      m_From.FTNAddr.Point = BAD_FTNADDR_FIELD;
      m_To  .FTNAddr.Zone  = BAD_FTNADDR_FIELD;
      m_To  .FTNAddr.Point = BAD_FTNADDR_FIELD;

      hdr->Written.ToFileTime( &m_Info.Sent );
      hdr->Arrived.ToFileTime( &m_Info.Received );
    }

    m_From.FTNAddr.Net  = hdr->origNet;
    m_From.FTNAddr.Node = hdr->origNode;
    m_To  .FTNAddr.Net  = hdr->destNet;
    m_To  .FTNAddr.Node = hdr->destNode;

    m_Part = create CFidoMsgPart(this, (PSTR)
      m_Data.data + sizeof(TFidoMessageHeader),
      m_Data.size - sizeof(TFidoMessageHeader));
  }

  return SetEncoding( Encoding );
}

PMsgPart CFidoMessageT::GetTextPart()
{
  MakeParts();
  return m_Part;
}

void CFidoMessageT::DecodeContent( PContent Content, LPCSTR TransferEncoding )
{
  return ;
}

DWORD CFidoMessageT::CalculateDecodedContentSize( PContent Content, LPCSTR TransferEncoding )
{
  return Content->size;
}


FarString CFidoMessageT::GetSubject()
{
  return m_Subject;
}

PPerson CFidoMessageT::GetFrom()
{
  if ( m_From.FTNAddr.Point == BAD_FTNADDR_FIELD )
    m_From.FTNAddr.Point = FarSF::atoi( GetKludge( K_FTN_FMPT ) );
  if ( m_From.FTNAddr.Zone == BAD_FTNADDR_FIELD )
    m_From.FTNAddr.Zone = FarSF::atoi( GetKludge( K_FTN_INTL ) );
  return &m_From;
}

PPerson CFidoMessageT::GetTo()
{
  if ( m_To.FTNAddr.Point == BAD_FTNADDR_FIELD )
    m_To.FTNAddr.Point = FarSF::atoi( GetKludge( K_FTN_TOPT ) );
  if ( m_To.FTNAddr.Zone == BAD_FTNADDR_FIELD )
  {
    LPCSTR p = strchr( GetKludge( K_FTN_INTL ), '\x20' );
    m_To.FTNAddr.Zone = p ? FarSF::atoi( p + 1 ) : 0;
  }
  return &m_To;
}

PPerson CFidoMessageT::GetCc()
{
  return &m_Cc;
}

FarString CFidoMessageT::GetArea()
{
  return GetDecodedKludge( K_FTN_Area );
}

FarString CFidoMessageT::GetNewsgroups()
{
  return FarString();
}

void CFidoMessageT::GetSent( LPFILETIME ft )
{
  *(PINT64)ft = *(PINT64)&m_Info.Sent;
}

void CFidoMessageT::GetReceived( LPFILETIME ft )
{
  *(PINT64)ft = *(PINT64)&m_Info.Received;
}

void CFidoMessageT::GetAccessed( LPFILETIME ft )
{
  *(PINT64)ft = 0ll;
}

DWORD CFidoMessageT::GetPriority()
{
#pragma message( "----- WARNING: need implementation of CFidoMessageT::GetPriority()" )
  return EMP_NORMAL;
}

DWORD CFidoMessageT::GetFlags()
{
#pragma message( "----- WARNING: need implementation of CFidoMessageT::GetFlags()" )
  return 0;
}

FarString CFidoMessageT::GetId()
{
  if ( m_MsgId.IsEmpty() )
    m_MsgId = GetKludge( K_FTN_MsgId );
  return m_MsgId;
}

FarString CFidoMessageT::GetDate()
{
  return FarString();
}

FarString CFidoMessageT::GetMailer()
{
  return GetDecodedKludge( K_FTN_PID );
}

FarString CFidoMessageT::GetOrganization()
{
  return FarString();
}

void CFidoMessageT::GetReferences( FarStringArray& strings )
{
  LPCSTR Reply = GetKludge( K_FTN_Reply );
  if ( *Reply )
    strings.Add( Reply );
}
