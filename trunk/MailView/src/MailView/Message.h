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
#ifndef ___Msg_H___
#define ___Msg_H___

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Mailbox.h"
#include <FarPlus.h>
#include "DateTime.h"
#include "Kludges.h"
#include "FarPlugin.h"
#include "FarInetMessage.h"

class CMessage : public FarCustomPanelPlugin
{
private:

  PMessage m_Msg;

  FarString  m_Subject;
  FarString  m_Encoding;
  int        m_FarEncoding;
  CMailbox * m_Mailbox;

  FarString  m_panelTitle;

public:
  CMessage( PMessage Msg, const FarFileName& HostFile );
  virtual ~CMessage();

  bool AllowExecuteFile( PluginPanelItem * Item );
  bool VirusCheckOk( PluginPanelItem * Item );

  static bool SaveToFile( CMailbox * pMailbox, DWORD dwMsgID, LPCSTR FileName );

public:
  virtual void GetOpenInfo( OpenPluginInfo * pInfo );
  virtual bool GetFindData( FarPluginPanelItems *Items, int OpMode );
  virtual void FreeFindData( FarPluginPanelItems *Items );

  virtual int GetFiles( FarPluginPanelItems *Items, bool bMove, LPSTR DestPath, int OpMode );
  virtual bool ProcessKey( int Key, UINT ControlState );
  virtual bool ProcessHostFile( const FarPluginPanelItems *Items, int OpMode );
};



#endif // !defined(___Msg_H___)
