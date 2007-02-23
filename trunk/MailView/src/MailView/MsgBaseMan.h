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
#ifndef ___MsgBaseMan_H___
#define ___MsgBaseMan_H___

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum TMessageStrings
  {
  };

#include "..\Shared\WPlugin.h"
#include "..\Shared\Wrap.h"

class CMsgBaseMan : public CPlugin
  {
  private:
  public:
    CMsgBaseMan( const PluginStartupInfo * pInfo );
    virtual ~CMsgBaseMan();
    virtual void GetInfo( PluginInfo * pInfo );
    virtual HANDLE Open( const int OpenFrom, const int Item );
    virtual HANDLE OpenFile( const char * Name, const unsigned char * Data, const int DataSize );
    virtual void Close( HANDLE hPlugin );
    virtual void GetOpenInfo( HANDLE hPlugin, OpenPluginInfo * pInfo );
    virtual bool GetFindData( HANDLE hPlugin, PluginPanelItem ** ppPanelItems, int * pItemsNumber, const int OpMode );
    virtual void FreeFindData( HANDLE hPlugin, PluginPanelItem * pPanelItem, int ItemsNumber );
    virtual int GetFiles( HANDLE hPlugin, PluginPanelItem * pPanelItem, const int ItemsNumber, bool bMove, const char * DestPath, const int OpMode );
    virtual bool ProcessKey( HANDLE hPlugin, const int Key, const unsigned int ControlState );
  };

extern CMsgBaseMan * pMsgBaseMan;

#endif // !defined(___MsgBaseMan_H___)
