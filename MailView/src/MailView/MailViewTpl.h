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
#ifndef ___MailViewTpl_H___
#define ___MailViewTpl_H___

#include <FarPlus.h>
#include "MsgLib/MsgLib.h"
#include "Template.h"

class CMessageTemplate : public CTemplate
{
protected:

  CMessageTemplate() : CTemplate()
  {
  }

public:

  virtual ~CMessageTemplate()
  {
  }

private:

  virtual void processMacro( LPCSTR str, int len, int fill, IWriteSink & f );

protected:

  virtual void writeFormatted( const FarString& str, int len, int fill, IWriteSink & f );

  virtual void processSubject( int len, int fill, IWriteSink & f ) = 0;
  virtual void processFrom( int len, int fill, IWriteSink & f ) = 0;
  virtual void processTo( int len, int fill, IWriteSink & f ) = 0;
  virtual void processDate( int len, int fill, IWriteSink & f ) {}
  virtual void processOrganization( int len, int fill, IWriteSink & f ) {}
  virtual void processMailer( int len, int fill, IWriteSink & f ) {}
  virtual void processOSubject( int len, int fill, IWriteSink & f ) = 0;
  virtual void processMessageId( int len, int fill, IWriteSink & f ) = 0;
  virtual void processFromName( int len, int fill, IWriteSink & f ) = 0;
  virtual void processFromAddr( int len, int fill, IWriteSink & f ) = 0;
  virtual void processToName( int len, int fill, IWriteSink & f ) = 0;
  virtual void processToAddr( int len, int fill, IWriteSink & f ) = 0;
  virtual void processSentDate( int len, int fill, IWriteSink & f ) = 0;
  virtual void processSentTime( int len, int fill, IWriteSink & f ) = 0;
  virtual void processReceivedDate( int len, int fill, IWriteSink & f ) = 0;
  virtual void processReceivedTime( int len, int fill, IWriteSink & f ) = 0;
  virtual void processText( int len, IWriteSink & f ) {}
  virtual void processHeaders( IWriteSink & f ) {}
  virtual void processKludge( LPCSTR kludge, IWriteSink & f ) {}
};

class CMailViewTpl : private CMessageTemplate
{
private:
  PMessage    m_origMsg;
  bool        m_origDel;
  FarFileName m_fileName;
  long        m_defaultEncoding;

  virtual void processSubject( int len, int fill, IWriteSink & f );
  virtual void processFrom( int len, int fill, IWriteSink & f );
  virtual void processTo( int len, int fill, IWriteSink & f );
  virtual void processDate( int len, int fill, IWriteSink & f );
  virtual void processOrganization( int len, int fill, IWriteSink & f );
  virtual void processMailer( int len, int fill, IWriteSink & f );
  virtual void processOSubject( int len, int fill, IWriteSink & f );
  virtual void processMessageId( int len, int fill, IWriteSink & f );
  virtual void processFromName( int len, int fill, IWriteSink & f );
  virtual void processFromAddr( int len, int fill, IWriteSink & f );
  virtual void processToName( int len, int fill, IWriteSink & f );
  virtual void processToAddr( int len, int fill, IWriteSink & f );
  virtual void processSentDate( int len, int fill, IWriteSink & f );
  virtual void processSentTime( int len, int fill, IWriteSink & f );
  virtual void processReceivedDate( int len, int fill, IWriteSink & f );
  virtual void processReceivedTime( int len, int fill, IWriteSink & f );
  virtual void processText( int len, IWriteSink & f );
  virtual void processHeaders( IWriteSink & f );
  virtual void processKludge( LPCSTR kludge, IWriteSink & f );

public:
  CMailViewTpl( const FarFileName& TplFileName, PMessage Msg, long defaultEncoding );
  CMailViewTpl( const FarFileName& TplFileName, LPCSTR MsgFileName, long defaultEncoding );
  ~CMailViewTpl();

  bool View( bool bEditor, bool bLock );

  bool SaveToFile( LPCSTR fileName );
};

#endif //!defined(___MailViewTpl_H___)
