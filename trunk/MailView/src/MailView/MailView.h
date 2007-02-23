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


#include "FarPlugin.h"
#include <FarString.h>
#include "LangID.H"
#include "MailboxCfg.H"
#include "FarMultiLang.h"
#include "MailViewConfig.h"

class CMailView : public FarCustomPlugin
{
  friend FarString ToOEMString( const FarString & Str, LPCSTR RfcEncoding );
  friend long SelectCharacterTableMenu( long SelectedTable, bool rfcOnly );
  friend long getCharacterTable( LPCSTR RFCName );
  friend long getCharacterTableNoDefault( LPCSTR RFCName );
  friend FarString getCharsetName( long table );
  friend FarString getDisplayCharsetName( long table );
  friend void getFarCharacterTables( FarStringArray & name, FarIntArray & data, bool rfcOnly );
  friend void getRfcCharacterTables( FarStringArray & name, FarIntArray & data, bool addHotKeys );
private:
  FarFileName    m_ThisRoot;
  FarFileName    m_IniFile;

  CMailboxCfg  * m_MailboxCfg;

  FarMultiLang * m_MLang;

  CMailViewPlugins m_Plugins;
  CMessagesCfg     m_MsgCfg;
  CInetNewsCfg     m_NwsCfg;
  CWebArchivesCfg  m_MhtCfg;
  CViewCfg         m_ViewCfg;

  FarString m_ExcludeMasks;

  int       m_MessageStartPanelMode;
  FarString m_DangerFilesMasks;
  int       m_DangerFilesWarnRule;
  FarString m_VirusChecker;
  int       m_VirusCheckerSuccessCode;

  virtual int GetMinFarVersion() const
  {
    return MAKEFARVERSION( 1, 70, 1282 );
  }

  static int WINAPI PluginsSearch( const LPWIN32_FIND_DATA FindData, LPCSTR FileName, CMailView * pThis );
public:
  CMailView();
  virtual ~CMailView();
  virtual HANDLE Open( const int OpenFrom, int Item );
  virtual HANDLE OpenFile( LPCSTR Name, const BYTE * Data, int DataSize );
  virtual bool Configure( int ItemNumber );

public:

  static CMailView * m_Handle;

  FarString GetRoot() { return m_ThisRoot; }

  FarString GetDangerFilesMasks() { return m_DangerFilesMasks; }
  int GetDangerFilesWarnRule() { return m_DangerFilesWarnRule; }
  FarString GetVirusChecker() { return FarString( m_VirusChecker ).Expand(); }
  int GetVirusCheckerSuccessCode() { return m_VirusCheckerSuccessCode; }

  CViewCfg * GetViewCfg() { return &m_ViewCfg; }

  void SaveSettings( bool bForce = false );
};

long SelectCharacterTableMenu( long SelectedTable, bool rfcOnly = false );
long getCharacterTable( LPCSTR RFCName );
long getCharacterTableNoDefault( LPCSTR RFCName );
FarString getCharsetName( long table );
FarString getDisplayCharsetName( long table );

void getFarCharacterTables( FarStringArray & name, FarIntArray & data, bool rfcOnly );
void getRfcCharacterTables( FarStringArray & name, FarIntArray & data, bool addHotKeys );

extern LPCSTR STR_NotImplemented;

#define Msg_NotImplemented() DbgMsg( Far::GetMsg( MMailView ), STR_NotImplemented )

#ifndef _DEBUG
#define ViewCfg() CMailView::m_Handle->GetViewCfg()
#define ColorCfg() CMailView::m_Handle->GetViewCfg()->GetColors()
#else
inline CViewCfg * ViewCfg() { far_assert( CMailView::m_Handle ); return CMailView::m_Handle->GetViewCfg(); }
inline CColorCfg * ColorCfg() { far_assert( CMailView::m_Handle ); return CMailView::m_Handle->GetViewCfg()->GetColors(); }
#endif

#endif // !defined(___MsgBaseMan_H___)
