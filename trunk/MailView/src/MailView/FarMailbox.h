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
#ifndef ___FarMailbox_H___
#define ___FarMailbox_H___

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MailView.h"

#include <FarFile.h>

#include "MailViewPlugin.h"

#include <FarString.h>
#include "Person.h"
#include "References.h"
#include "MailboxCache.h"
#include "MsgLib/MsgLib.h"
#include "Mailbox.h"
#include "File.h"

//////////////////////////////////////////////////////////////////////////
// Mailbox Cache file structure
//
// char[4]  Signature = "!mbc"
// WORD         Version   = MAKEWORD( 1, 0 );
// FILETIME Last modified mailbox time
// DWORD  Mailbox file size
// DWORD  CRC32 of Mailbox
// DWORD  Items Count
// TMessage     Array of
//

class CFarMailbox : public FarCustomPanelPlugin
{
private:
  CMailbox  m_mailbox;

  FarString m_formatName;
  FarString m_panelTitle;

  FarString m_msgExt;

  FarString m_noneSubj;

  TMailboxCache m_Cache;
  CThread     * m_CurRef;
  CThreads    * m_RefMap;
  FarStringArray m_dirList;

  void MakeRefs();
//  PluginPanelItem * InsertItems( CThread * Ref, PluginPanelItem * CurItem );
  PluginPanelItem * InsertTreeItems( CThread * Ref, PluginPanelItem * CurItem, int level );
  void SetItem( PluginPanelItem * item, TCacheEntry * ce );
  void SetItem( PluginPanelItem * item, TCacheEntry * ce, const FarString& graph );

  void LoadCache();
  void SaveCache();

  CMailboxCfg * m_Config;

  void (*KillRe)(LPSTR);

  static int SpeedSearchSelect( LPCSTR Mask, PluginPanelItem * Items, int ItemsCount, int nStart );

  PMessage CreateFarMessage();

  static LPSTR getKbTitleMsg( int msgId )
  {
    LPCSTR msg = Far::GetMsg( msgId );
    return *msg == '\b' ? NULL : (LPSTR)msg;
  }

  static int sortBySubject( const TCacheEntry ** ce1, const TCacheEntry ** ce2, void * user );
  static int sortByFrom( const TCacheEntry ** ce1, const TCacheEntry ** ce2, void * user );
  static int sortByTo( const TCacheEntry ** ce1, const TCacheEntry ** ce2, void * user );
  static int sortBySize( const TCacheEntry ** ce1, const TCacheEntry ** ce2, void * user );
  static int sortBySent( const TCacheEntry ** ce1, const TCacheEntry ** ce2, void * user );
  static int sortByReceived( const TCacheEntry ** ce1, const TCacheEntry ** ce2, void * user );
  static int sortByAccessed( const TCacheEntry ** ce1, const TCacheEntry ** ce2, void * user );
  static int sortByHandle( const TCacheEntry ** ce1, const TCacheEntry ** ce2, void * user );
  static int sortByCacheIndex( const TCacheEntry ** ppce1, const TCacheEntry ** ppce2 );

  bool deleteFiles( FarPluginPanelItems *items, int opMode, bool markOnly );
  bool undeleteFiles( FarPluginPanelItems *items, int opMode = 0 );
  bool setAttributes();
  void purge();
  void needMore();

  void changeSortMode( CMailboxCfg::TSortMode newMode );

  bool addCacheItem( CMailbox * mailbox, DWORD msgId, PMessage Msg, long defaultEncoding );

  static void call_purge( CFarMailbox * object )
  {
    far_assert( object != NULL );
    object->purge();
  }
  static void call_needMore( CFarMailbox * object )
  {
    far_assert( object != NULL );
    object->needMore();
  }

  FarFileName getCacheFileName() const
  {
    FarFileName path = m_HostFileName;
    if ( *m_Config->GetCacheExt() != '.' )
      path.SetExt( '.' + FarFileName(m_Config->GetCacheExt()) );
    else
      path += m_Config->GetCacheExt();

    if ( m_Config->GetUseNTFSStreams() && !File::exists( path ) )
    {
      char fileSystemName[ 5 ];
      if ( GetVolumeInformation( path.GetDrive() + '\\',
        NULL, 0, NULL, NULL, NULL, fileSystemName, sizeof( fileSystemName ) ) &&
        FarSF::LStricmp( fileSystemName, "NTFS" ) == 0 )
      {
        path = m_HostFileName + ":mailview.mailbox.cache";
      }
    }

    return path;
  }

  bool confirm( const char * title, const char * text = NULL );

  bool confirm( int title, int text )
  {
    return confirm( Far::GetMsg( title ), Far::GetMsg( text ) );
  }

  bool confirm( int title )
  {
    return confirm( Far::GetMsg( title ) );
  }

public:

  CFarMailbox( HMODULE hPluginDLL, const FarFileName& FileName, CMailboxCfg * GlobalCfg );
  virtual ~CFarMailbox();

  void Close( bool saveCache = true );
  bool Open();

  LPCSTR GetDefAttachDir()
  {
    return m_Config->GetAttachDir();
  }

  virtual void GetOpenInfo( OpenPluginInfo * pInfo );
  virtual bool GetFindData( FarPluginPanelItems *Items, int OpMode );
  virtual void FreeFindData( FarPluginPanelItems *Items );
  virtual int GetFiles( FarPluginPanelItems *Items, bool bMove, LPSTR DestPath, int OpMode );
  virtual bool ProcessKey( int Key, UINT ControlState );
  virtual int Compare( const PluginPanelItem * Item1, const PluginPanelItem * Item2, UINT Mode );
  virtual bool SetDirectory( LPCSTR Dir, int OpMode );
  virtual bool ProcessHostFile( const FarPluginPanelItems *Items, int OpMode );
  virtual bool DeleteFiles( FarPluginPanelItems *Items, int OpMode );
  virtual bool PutFiles( const FarPluginPanelItems *Items, bool bMove, int OpMode );
};

#endif // !deifned(___FarMailbox_H___)
