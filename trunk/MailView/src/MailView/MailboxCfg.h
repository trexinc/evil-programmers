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
// MailboxCfg.h: interface for the CMailboxCfg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAILBOXCFG_H__50F1A08F_69F3_4EBE_9516_8A6328BCA69B__INCLUDED_)
#define AFX_MAILBOXCFG_H__50F1A08F_69F3_4EBE_9516_8A6328BCA69B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <FarPlus.h>
#include "IniConfig.h"

#define INI_MAILBOX "Mailbox"

enum TTheradsViewMode
{
  tvmNone = 0, tvmReferences_Std, tvmSubject, tvmFrom, tvmTo, tvmReferences_Ext
};


class CMailboxCfg : public CIniConfig
{
private:
  CMailboxCfg *   m_Parent;

  PanelMode       m_PanelMode;
  int             m_StartPanelMode;
  int             m_StartSortMode;
  int             m_StartSortOrder;

  int             m_TheradsViewMode;
  int             m_TreeViewMode;
  int             m_TreeViewMode2;

  int             m_MinCacheSize;
  int             m_ProcessSubject;

  int             m_MarkMsgAsReadTime;

  int             m_AllowWriteAccess;

  LPSTR           m_DefaultCharset;
  LPSTR           m_AttachDir;
  LPSTR           m_ConfigExt;
  LPSTR           m_CacheExt;

  FarString       m_emptySubj;

  void ConvertColumnTypesToFarFormat( LPSTR inPtr, LPSTR outPtr );
  LPCSTR GetTitle( LPSTR szPanelMode );

  bool m_UseNTFSStreams;
  bool m_UseAttributeHighlighting;

  bool m_bModified;

  FarString m_copyOutputFormat;

public:
  CMailboxCfg( LPCSTR IniFile, CMailboxCfg * Parent );
  virtual ~CMailboxCfg();

  int ReadInt( LPCSTR Key, int Default )
  {
    return CIniConfig::ReadInt( INI_MAILBOX, Key, Default );
  }
  bool ReadBool( LPCSTR Key, bool bDefault )
  {
    return CIniConfig::ReadBool( INI_MAILBOX, Key, bDefault );
  }
  int ReadString( LPCSTR Key, LPCSTR Default, LPSTR Return, int nSize )
  {
    return CIniConfig::ReadString( INI_MAILBOX, Key, Default, Return, nSize );
  }
  CMailboxCfg * GetParent() const
  {
    return m_Parent;
  }
  LPCSTR GetDefaultCharset() const
  {
    if ( *m_DefaultCharset == 0 && m_Parent )
      return m_Parent->m_DefaultCharset;
    return m_DefaultCharset;
  }
  void SetDefaultCharset( LPCSTR charset );

  enum TSortMode { smSubject = 1, smFrom, smTo, smSize, smNone, smSent, smReceived, smAccessed };

  int GetStartPanelMode() const { return m_StartPanelMode; }
  TSortMode GetStartSortMode() const { return (TSortMode)m_StartSortMode; }
  int GetStartSortOrder() const { return m_StartSortOrder; }
  void SetStartSortMode( TSortMode mode );
  void SetStartSortOrder( int order );
  TTheradsViewMode GetTheradsViewMode() const { return (TTheradsViewMode)m_TheradsViewMode; }
  void SetTheradsViewMode( TTheradsViewMode mode );
  int GetTreeViewMode() { return m_TreeViewMode; }
  int GetTreeViewMode2() { return m_TreeViewMode2; }

  int GetMinCacheSize() { return m_MinCacheSize; }
  int GetProcessSubject() { return m_ProcessSubject; }

  FarString getEmptySubj() { return m_emptySubj; }

  PanelMode * GetMailboxPanelMode() { return &m_PanelMode; }

  int GetMarkMsgAsReadTime() { return m_MarkMsgAsReadTime; }
  int GetAllowWriteAccess() { return m_AllowWriteAccess; }
  LPCSTR GetAttachDir() const { return m_AttachDir; }

  bool GetUseNTFSStreams() const { return m_UseNTFSStreams; }
  bool GetUseAttributeHighlighting() const { return m_UseAttributeHighlighting; }

  LPCSTR GetConfigExt() const { return m_ConfigExt; }
  LPCSTR GetCacheExt() const { return m_CacheExt; }

  FarString getCopyOutputFormat() const { return m_copyOutputFormat; }
  void setCopyOutputFormat( const FarString& value );
};

#define TVM_DRAW_TREE (1<<0)
#define TVM_WIDE_TREE (1<<1)
#define TVM_DRAW_ROOT (1<<2)
#define TVM_INSERT_SP (1<<3)

#endif // !defined(AFX_MAILBOXCFG_H__50F1A08F_69F3_4EBE_9516_8A6328BCA69B__INCLUDED_)
