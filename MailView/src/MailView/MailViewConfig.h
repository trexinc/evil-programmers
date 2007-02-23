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
#ifndef ___MailViewConfig_H___
#define ___MailViewConfig_H___

#include <FarString.h>
#include "IniConfig.h"

#define INI_GENERAL  "General"
#define INI_MAILVIEW "MailView"
#define INI_MESSAGE  "Message"

class CMailViewCfg
{
protected:
  void WriteInt( CIniConfig& ini, LPCSTR Section, LPCSTR Key, int nValue, int nDefault )
  {
    if ( nValue == nDefault )
      ini.EraseKey( Section, Key );
    else
      ini.WriteInt( Section, Key, nValue );
  }
  void WriteBool( CIniConfig& ini, LPCSTR Section, LPCSTR Key, bool bValue, bool bDefault )
  {
    if ( bValue == bDefault )
      ini.EraseKey( Section, Key );
    else
      ini.WriteBool( Section, Key, bValue );
  }
  void WriteString( CIniConfig& ini, LPCSTR Section, LPCSTR Key, const FarString& szValue, LPCSTR szDefault, bool bIgnoreCase = true )
  {
    if ( bIgnoreCase )
    {
      if ( szValue.CompareNoCase( szDefault ) == 0 )
        ini.EraseKey( Section, Key );
      else
        ini.WriteString( Section, Key, szValue );
    }
    else
    {
      if ( szValue.Compare( szDefault ) == 0 )
        ini.EraseKey( Section, Key );
      else
        ini.WriteString( Section, Key, szValue );
    }
  }
public:
  CMailViewCfg()
  {
  }
  virtual ~CMailViewCfg()
  {
  }

  virtual void LoadSettings(CIniConfig& ini) = 0;
  virtual void SaveSettings(CIniConfig& ini) = 0;

  virtual void Configure() = 0;
};

class CMessagesCfg : public CMailViewCfg
{
protected:
  FarString m_FilesMasks;
  virtual LPCSTR GetDefaultFilesMasks() const;
  virtual LPCSTR GetConfigDialogTitle() const;
public:
  CMessagesCfg() : CMailViewCfg()
  {
  }
  virtual ~CMessagesCfg()
  {
  }

  virtual void LoadSettings( CIniConfig& ini );
  virtual void SaveSettings( CIniConfig& ini );

  virtual void Configure();

  LPCSTR GetFilesMasks() const;

  virtual LPCSTR GetIniKeyName() const;
};

class CInetNewsCfg : public CMessagesCfg
{
protected:
  virtual LPCSTR GetDefaultFilesMasks() const;
  virtual LPCSTR GetConfigDialogTitle() const;
public:
  CInetNewsCfg() : CMessagesCfg()
  {
  }
  virtual ~CInetNewsCfg()
  {
  }
  virtual LPCSTR GetIniKeyName() const;
};

class CWebArchivesCfg : public CMessagesCfg
{
protected:
  virtual LPCSTR GetDefaultFilesMasks() const;
  virtual LPCSTR GetConfigDialogTitle() const;
public:
  CWebArchivesCfg() : CMessagesCfg()
  {
  }
  virtual ~CWebArchivesCfg()
  {
  }
  virtual LPCSTR GetIniKeyName() const;
};

class CMailViewPlugins : public CMailViewCfg
{
private:
  FarStringArray m_Names;
  FarStringArray m_Masks;
  FarIntArray    m_Libs;
  static int WINAPI PluginsSearch( const WIN32_FIND_DATA * FindData, LPCSTR FileName, LPVOID Param );
  FarString MakeKeyName( LPCSTR Root, LPCSTR FileName );
public:
  CMailViewPlugins() : CMailViewCfg()
  {
  }
  virtual ~CMailViewPlugins();

  void Search( LPCSTR SearchPath );
  virtual void Configure();

  virtual void LoadSettings( CIniConfig& ini );
  virtual void SaveSettings( CIniConfig& ini );

  int Count() const
  {
    return m_Libs.Count();
  }

  HMODULE GetLibrary( int nIndex ) const;
  FarString GetFilesMasks( int nIndex ) const;
};

class CColorCfg : public CMailViewCfg
{
  friend class CViewCfg;
private:
  static BYTE m_Default[];
  LPBYTE m_Palette;
  CColorCfg();
public:
  virtual ~CColorCfg();

  virtual void Configure();

  virtual void LoadSettings( CIniConfig& ini );
  virtual void SaveSettings( CIniConfig& ini );

  BYTE GetColor( int nIndex )
  {
    far_assert( nIndex >= 0 && nIndex < icl_Last );
    return m_Palette[ nIndex ];
  }

  enum
  {
    iclNormal = 0,
    iclBold,
    iclItalic,
    iclBoldItalic,
    iclUnderline,
    iclBoldUnderline,
    iclItalicUnderline,
    iclBoldItalicUnderline,
    iclReverse,
    iclReverseBold,
    iclReverseItalic,
    iclReverseBoldItalic,
    iclReverseUnderline,
    iclReverseBoldUnderline,
    iclReverseItalicUnderline,
    iclReverseBoldItalicUnderline,

    iclQuotesEven,
    iclQuotesOdd,
    iclSignature,
    iclKludges,

    iclBorder,
    iclBorderTitle,
    iclHeaderLine,
    iclHeaderEdit,
    iclHeaderEditSel,
    iclDateTime,

    iclOrigin,
    iclTearline,
    iclTagline,

    iclHyperlink,
    iclHyperlinkSel,

    iclReserved1,
    iclReserved2,
    iclReserved3,
    iclReserved4,
    iclReserved5,
    iclReserved6,
    iclReserved7,
    iclReserved8,
    iclReserved9,
    iclReserved10,
    iclReserved11,
    iclReserved12,
    iclReserved13,
    iclReserved14,
    iclReserved15,
    iclReserved16,
    iclReserved17,

    icl_Last
  };
};

class CHTMLFilterCfg : public CMailViewCfg
{
private:
  bool m_Enabled;
  FarFileName m_hvLib, m_hvCfg;
  static BOOL validate( char * text, void * data );

public:
  CHTMLFilterCfg() : CMailViewCfg(), m_Enabled( true )
  {
  }
  virtual ~CHTMLFilterCfg()
  {
  }

  virtual void Configure();

  virtual void LoadSettings( CIniConfig& ini );
  virtual void SaveSettings( CIniConfig& ini );

  FarFileName GetHVLib() const
  {
    FarFileName lib;
    if ( m_Enabled )
      lib = m_hvLib;
    return lib.Expand();
  }
  FarFileName GetHVCfg() const { return FarFileName( m_hvCfg ).Expand(); }

  bool getEnabled() const { return m_Enabled; }
  void setEnabled( bool newVal ) { m_Enabled = newVal; }
};

class CTemplatesCfg : public CMailViewCfg
{
private:
  FarFileName m_eTpl, m_qTpl, m_vTpl, m_root;
  FarFileName FixTpl( const FarFileName& name ) const;

public:
  CTemplatesCfg() : CMailViewCfg()
  {
  }
  virtual ~CTemplatesCfg()
  {
  }

  virtual void Configure();

  virtual void LoadSettings( CIniConfig& ini );
  virtual void SaveSettings( CIniConfig& ini );

  FarFileName GetQTpl() const { return FixTpl( m_qTpl ); }
  FarFileName GetETpl() const { return FixTpl( m_eTpl ); }
  FarFileName GetVTpl() const { return FixTpl( m_vTpl ); }
};

class CViewCfg : public CMailViewCfg
{
private:
  enum
  {
    vcsFullSceen        = 1<<0,
    vcsWordWrap         = 1<<1,
    vcsNoHideStylePunct = 1<<2,
  };

  bool m_bUseLastSignature, m_bLockView;

  int m_State;

  CColorCfg      m_Colors;
  CHTMLFilterCfg m_HTMLFilter;
  CTemplatesCfg  m_Templates;

  FarString m_StyleCodePunct;
  FarString m_StyleCodeStops;
  FarString m_DateFormat;
  FarString m_TimeFormat;

public:
  CViewCfg();

  virtual ~CViewCfg()
  {
  }

  virtual void Configure();

  virtual void LoadSettings( CIniConfig& ini );
  virtual void SaveSettings( CIniConfig& ini );

  FarFileName GetQTpl() const { return m_Templates.GetQTpl(); }
  FarFileName GetETpl() const { return m_Templates.GetETpl(); }
  FarFileName GetVTpl() const { return m_Templates.GetVTpl(); }

  FarFileName GetHVLib() const { return m_HTMLFilter.GetHVLib(); }
  FarFileName GetHVCfg() const { return m_HTMLFilter.GetHVCfg(); }

  FarString getStyleCodePunct() const { return m_StyleCodePunct; }
  FarString getStyleCodeStops() const { return m_StyleCodeStops; }

  FarString getDateFormat() const { return m_DateFormat; }
  FarString getTimeFormat() const { return m_TimeFormat; }

  bool GetLockView() const { return m_bLockView; }

  bool GetUseLastSignature() const { return m_bUseLastSignature; }

  CColorCfg * GetColors() { return &m_Colors; }

  bool GetWordWrap() const
  {
    return (m_State & vcsWordWrap) != 0;
  }
  void SetWordWrap( bool bValue )
  {
    m_State = bValue ? m_State | vcsWordWrap : m_State & ~vcsWordWrap;
  }

  bool GetFullScreen() const
  {
    return (m_State & vcsFullSceen) != 0 ;
  }
  void SetFullScreen( bool bValue )
  {
    m_State = bValue ? m_State | vcsFullSceen : m_State & ~vcsFullSceen;
  }

  bool GetHideStylePunct() const
  {
    return (m_State & vcsNoHideStylePunct) == 0;
  }
  void SetHideStylePunct( bool value )
  {
    m_State = value ? (m_State & ~vcsNoHideStylePunct) : (m_State|vcsNoHideStylePunct);
  }
};

#endif //!defined(___MailViewConfig_H___)
