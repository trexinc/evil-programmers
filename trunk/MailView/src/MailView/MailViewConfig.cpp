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
#include "stdafx.h"
#include <FarDialog.h>
#include <FarMenu.h>

#include "Mailbox.h"
#include "LangID.h"
#include "MailViewConfig.h"
#include "FarColorDialog.h"
#include "File.h"

extern FarFileName ExtractRelativePath( LPCSTR BaseName, LPCSTR DestName );
extern bool IsNotRelativePath( const char * FileName );
extern void ShowConsoleCursor( CONST BOOL bVisible );

bool ConfigureFileMasksDlg( LPCSTR Title, LPCSTR Default, FarString& Current )
{
  FarDialog dlg( Title );

  dlg.AddText( MFileMasks );
  FarEditCtrl edtFileMasks( &dlg, Current, 0, 66 );
  edtFileMasks.SetNextY();

  dlg.AddSeparator();
  FarButtonCtrl btnOk( &dlg, MOk );
  FarButtonCtrl btnReset( &dlg, MReset, DIF_CENTERGROUP );
  FarButtonCtrl btnCancel( &dlg, MCancel, DIF_CENTERGROUP );

  dlg.SetDefaultControl( &btnOk );
  dlg.SetFocusControl( &edtFileMasks );

  bool bSkipLayout = false;
  while ( true )
  {
    FarControl * Ctl = dlg.Show( bSkipLayout );

    if ( Ctl == &btnOk )
    {
      Current = edtFileMasks.GetText();
      return true;
    }
    else if ( Ctl == &btnReset )
    {
      edtFileMasks.SetText( Default );

      dlg.SetDefaultControl( &btnOk );
      dlg.SetFocusControl( &edtFileMasks );

      bSkipLayout = true;
      continue;
    }

    break;
  }
  return false;
}

LPCSTR CMessagesCfg::GetFilesMasks() const
{
  return m_FilesMasks.IsEmpty() ? GetDefaultFilesMasks() : m_FilesMasks.c_str();
}

void CMessagesCfg::LoadSettings( CIniConfig& ini )
{
  m_FilesMasks.ReleaseBuffer( ini.ReadString( INI_GENERAL, GetIniKeyName(),
    GetDefaultFilesMasks(), m_FilesMasks.GetBuffer( 256 ), 256 ) );
}

void CMessagesCfg::SaveSettings( CIniConfig& ini )
{
  WriteString( ini, INI_GENERAL, GetIniKeyName(), m_FilesMasks, GetDefaultFilesMasks() );
}

LPCSTR CMessagesCfg::GetDefaultFilesMasks() const
{
  static LPCSTR FilesMasks = "*.eml,*.msg";
  return FilesMasks;
}

LPCSTR CInetNewsCfg::GetDefaultFilesMasks() const
{
  static LPCSTR FilesMasks = "*.nws";
  return FilesMasks;
}

LPCSTR CWebArchivesCfg::GetDefaultFilesMasks() const
{
  static LPCSTR FilesMasks = "*.mht,*.mhtml";
  return FilesMasks;
}

LPCSTR CMessagesCfg::GetIniKeyName() const
{
  static LPCSTR KeyName = "MessagesMasks";
  return KeyName;
}

LPCSTR CInetNewsCfg::GetIniKeyName() const
{
  static LPCSTR KeyName = "InetNewsMasks";
  return KeyName;
}

LPCSTR CWebArchivesCfg::GetIniKeyName() const
{
  static LPCSTR KeyName = "WebArchivesMasks";
  return KeyName;
}

LPCSTR CMessagesCfg::GetConfigDialogTitle() const
{
  return Far::GetMsg( MMessages );
}

LPCSTR CInetNewsCfg::GetConfigDialogTitle() const
{
  return Far::GetMsg( MNews );
}

LPCSTR CWebArchivesCfg::GetConfigDialogTitle() const
{
  return Far::GetMsg( MWebArchives );
}

void CMessagesCfg::Configure()
{
  LPCSTR Default = GetDefaultFilesMasks();

  ConfigureFileMasksDlg( GetConfigDialogTitle(), Default, m_FilesMasks );
    /*&& m_FilesMasks.CompareNoCase( Default ) == 0 )
    m_FilesMasks.Empty();*/
}

CMailViewPlugins::~CMailViewPlugins()
{
  for ( int i = 0; i < m_Libs.Count(); i ++ )
    FreeLibrary( (HMODULE)m_Libs[ i ] );
}

int WINAPI CMailViewPlugins::PluginsSearch( const WIN32_FIND_DATA * FindData, LPCSTR FileName, LPVOID Param )
{
  if ( ( FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
    return TRUE;

  if ( FileName == NULL )
    return TRUE;

  HMODULE hLib = LoadLibrary( FileName );

  if ( hLib == NULL )
    return true;

  CMailViewPlugins * mvp = (CMailViewPlugins*)Param;

  mvp->m_Libs.Add( (int)hLib );
  mvp->m_Names.Add( FileName );

  return TRUE;
}

void CMailViewPlugins::Search( LPCSTR SearchPath )
{
  /*LPCSTR FileName = "D:\\Work\\FarPlugins\\MailView\\Plugins\\Unix\\Unix.mvp";
  HMODULE hLib = LoadLibrary( FileName );

  if ( hLib  )
  {
    m_Libs.Add( (int)hLib );
    m_Names.Add( FileName );
  }*/

  FarSF::RecursiveSearch( SearchPath, "*.mvp", PluginsSearch, FRS_RECUR, this );
}

void CMailViewPlugins::Configure()
{
  FarMenu mnu( MMailboxes );

  FarIntArray Indices;
  for ( int i = 0; i < m_Libs.Count(); i ++ )
  {
    FarString Name = CMailbox::getLongName( (HINSTANCE)m_Libs[ i ] );
    if ( Name.IsEmpty() )
      Name = CMailbox::getShortName( (HINSTANCE)m_Libs[ i ] );
    if ( Name.IsEmpty() )
      Name = FarFileName( m_Names[ i ] ).GetName();

    mnu.AddItem( Name )->SetData( Indices.Add( i ) );
  }

  FarMenu::PItem Item;
  while ( ( Item = mnu.Show() ) != NULL )
  {
    int nIdx = Indices[ Item->GetData() ];

    FarString Default = CMailbox::getFilesMasks( (HINSTANCE)m_Libs[ nIdx ] );
    FarString Current = m_Masks[ nIdx ];
    if ( Current.IsEmpty() )
      Current = Default;

    if ( ConfigureFileMasksDlg( Item->GetText(), Default, Current ) )
    {
      m_Masks.Insert( nIdx, Current );
      m_Masks.Delete( nIdx + 1 );
    }

    mnu.SelectItem( Item );
  }
}

FarString CMailViewPlugins::MakeKeyName( LPCSTR Root, LPCSTR FileName )
{
  FarString Key = ExtractRelativePath( Root, FileName );
  Key.Insert( 0, "Mailbox_" );
  Key.SetLength( Key.Length() - 4 );
  Key += "_Masks";

  int nIdx = Key.IndexOf( '\\' );
  while ( nIdx != -1 )
  {
    Key[ nIdx ] = '_';
    nIdx = Key.IndexOf( '\\' );
  }

  return Key;
}

void CMailViewPlugins::LoadSettings( CIniConfig& ini )
{
  FarFileName Root = FarFileName( ini.GetIniFileName() ).GetPath();

  char * Buffer = create char[ 2048 ];
  for ( int i = 0; i < m_Names.Count(); i ++ )
  {
    FarString Key = MakeKeyName( Root, m_Names[ i ] );
    ini.ReadString( INI_GENERAL, Key, STR_EmptyStr, Buffer, 2048 );
    m_Masks.Add( Buffer );
  }
  delete [] Buffer;
}

void CMailViewPlugins::SaveSettings( CIniConfig& ini )
{
  FarFileName Root = FarFileName( ini.GetIniFileName() ).GetPath();

  for ( int i = 0; i < m_Names.Count(); i ++ )
  {
    FarString Key = MakeKeyName( Root, m_Names[ i ] );
    FarString Def = CMailbox::getFilesMasks( (HMODULE)m_Libs[ i ] );
    LPCSTR    Cur = m_Masks[ i ];
    if ( Def.CompareNoCase( Cur ) == 0 || *Cur == '\0' )
      ini.EraseKey( INI_GENERAL, Key );
    else
      ini.WriteString( INI_GENERAL, Key, Cur );
  }
}

FarString CMailViewPlugins::GetFilesMasks( int nIndex ) const
{
  return ( m_Masks.Count() == m_Libs.Count() && *(m_Masks[ nIndex ]) != '\0' ) ?
    m_Masks[ nIndex ] : CMailbox::getFilesMasks( GetLibrary( nIndex ) );
}

HMODULE CMailViewPlugins::GetLibrary( int nIndex ) const
{
  return (HMODULE)m_Libs[ nIndex ];
}

CColorCfg::CColorCfg() : CMailViewCfg()
{
  m_Palette = create BYTE[ icl_Last ];
  memcpy( m_Palette, m_Default, icl_Last );
}

CColorCfg::~CColorCfg()
{
  delete [] m_Palette;
}

BYTE CColorCfg::m_Default[ icl_Last ] =
{
  MAKE_COLOR( clBlack, clLtGray ),                     // iclNormal
  MAKE_COLOR( clBlack, clLtRed ),                      // iclBold
  MAKE_COLOR( clBlack, clLtBlue ),                     // iclItalic
  MAKE_COLOR( clBlack, clLtRed|clLtBlue ),             // iclBoldItalic
  MAKE_COLOR( clBlack, clLtGreen ),                    // iclUnderline
  MAKE_COLOR( clBlack, clLtRed|clLtGreen ),            // iclBoldUnderline
  MAKE_COLOR( clBlack, clLtBlue|clLtGreen ),           // iclItalicUnderline
  MAKE_COLOR( clBlack, clLtRed|clLtBlue|clLtGreen ),   // iclBoldItalicUnderline
  MAKE_COLOR( clLtGray, clBlack ),                     // iclReverse
  MAKE_COLOR( clLtGray, clLtRed ),                     // iclReverseBold
  MAKE_COLOR( clLtGray, clLtBlue ),                    // iclReverseItalic
  MAKE_COLOR( clLtGray, clLtRed|clLtBlue ),            // iclReverseBoldItalic
  MAKE_COLOR( clLtGray, clLtGreen ),                   // iclReverseUnderline
  MAKE_COLOR( clLtGray, clLtRed|clLtGreen ),           // iclReverseBoldUnderline
  MAKE_COLOR( clLtGray, clLtBlue|clLtGreen ),          // iclReverseItalicUnderline
  MAKE_COLOR( clLtGray, clLtRed|clLtBlue|clLtGreen ),  // iclReverseBoldItalicUnderline

  MAKE_COLOR( clBlack, clBrown ),                      // iclQuotesEven
  MAKE_COLOR( clBlack, clGreen ),                      // iclQuotesOdd
  MAKE_COLOR( clBlack, clDkGray ),                     // iclSignature
  MAKE_COLOR( clBlack, clBlue ),                       // iclKludges

  MAKE_COLOR( clBlack, clLtGray ),                     // iclBorder
  MAKE_COLOR( clBlack, clLtGray ),                     // iclBorderTitle
  MAKE_COLOR( clBlack, clYellow ),                     // iclHeaderLine
  MAKE_COLOR( clBlack, clWhite ),                      // iclHeaderEdit
  MAKE_COLOR( clBlue, clWhite ),                       // iclHeaderEditSel
  MAKE_COLOR( clBlack, clLtGreen ),                    // iclDateTime

  MAKE_COLOR( clBlack, clYellow ),                     // iclOrigin
  MAKE_COLOR( clBlack, clMagenta ),                    // iclTearline
  MAKE_COLOR( clBlack, clMagenta ),                    // iclTagline

  MAKE_COLOR( clBlack, clLtBlue ),                     // iclHyperlink
  MAKE_COLOR( clBlack, clMagenta ),                    // iclHyperlinkSel

  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved1
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved2
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved3
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved4
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved5
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved6
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved7
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved8
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved9
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved10
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved11
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved12
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved13
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved14
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved15
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved16
  MAKE_COLOR( clBlack, clBlack ),                      // iclReserved17
};

void CColorCfg::LoadSettings( CIniConfig& ini )
{
  ini.ReadData( INI_MAILVIEW, "Palette", m_Palette, icl_Last );
}

void CColorCfg::SaveSettings( CIniConfig& ini )
{
  if ( memcmp( m_Palette, m_Default, icl_Last ) == 0 )
    ini.EraseKey( INI_MAILVIEW, "Palette" );
  else
    ini.WriteData( INI_MAILVIEW, "Palette", m_Palette, icl_Last );
}

void CColorCfg::Configure()
{
  FarMenu mnu( MColors );

  mnu.AddItem( MCfg_clNormal )->SetData( iclNormal );

  mnu.AddItem( MCfg_clBold )->SetData( iclBold );
  mnu.AddItem( MCfg_clItalic )->SetData( iclItalic );
  mnu.AddItem( MCfg_clUnderline )->SetData( iclUnderline );
  mnu.AddItem( MCfg_clBoldItalic )->SetData( iclBoldItalic );
  mnu.AddItem( MCfg_clBoldUnderline )->SetData( iclBoldUnderline );
  mnu.AddItem( MCfg_clItalicUnderline )->SetData( iclItalicUnderline );
  mnu.AddItem( MCfg_clBoldItalicUnderline )->SetData( iclBoldItalicUnderline );

  mnu.AddItem( MCfg_clReverse )->SetData( iclReverse );

  mnu.AddItem( MCfg_clReverseBold )->SetData( iclReverseBold );
  mnu.AddItem( MCfg_clReverseItalic )->SetData( iclReverseItalic );
  mnu.AddItem( MCfg_clReverseUnderline )->SetData( iclReverseUnderline );

  mnu.AddItem( MCfg_clReverseBoldItalic )->SetData( iclReverseBoldItalic );
  mnu.AddItem( MCfg_clReverseBoldUnderline )->SetData( iclReverseBoldUnderline );
  mnu.AddItem( MCfg_clReverseItalicUnderline )->SetData( iclReverseItalicUnderline );
  mnu.AddItem( MCfg_clReverseBoldItalicUnderline )->SetData( iclReverseBoldItalicUnderline );

  mnu.AddSeparator();

  mnu.AddItem( MCfg_clKludges )->SetDisabled();
  mnu.AddItem( MCfg_clQuotesEven )->SetData( iclQuotesEven );
  mnu.AddItem( MCfg_clQuotesOdd )->SetData( iclQuotesOdd );
  mnu.AddItem( MCfg_clSignature )->SetData( iclSignature );
  mnu.AddItem( MCfg_clOrigin )->SetData( iclOrigin );
  mnu.AddItem( MCfg_clTearline )->SetData( iclTearline );
  mnu.AddItem( MCfg_clTagline )->SetData( iclTagline );
  mnu.AddItem( Mcfg_clHyperlink )->SetData( iclHyperlink );

  mnu.AddSeparator( MCfg__Window );

  mnu.AddItem( MCfg_clTitle )->SetData( iclBorderTitle );
  mnu.AddItem( MCfg_clBorder )->SetData( iclBorder );
  mnu.AddItem( MCfg_clHeaderLines )->SetData( iclHeaderLine );
  mnu.AddItem( MCfg_clHeaderEdit )->SetData( iclHeaderEdit );
  mnu.AddItem( MCfg_clHeaderEditSel )->SetData( iclHeaderEditSel );
  mnu.AddItem( MCfg_clDateTime )->SetData( iclDateTime );

  FarMenu::PItem Item;
  while ( ( Item = mnu.Show() ) != NULL )
  {
    ShowConsoleCursor( TRUE );
    int idx = Item->GetData();
    CFarColorDialog dlg( m_Palette[ idx ] );
    if ( dlg.Execute() )
      m_Palette[ idx ] = dlg.GetColorValue();
    mnu.SelectItem( Item );
  }
}

#define DEFAULT_STYLECODEPUNCT " !?\"$%&()+,.:;<=>@[\\]^`{|}~\n\r\t"
#define DEFAULT_STYLECODESTOPS "/#*"

FarString StringFromCString(const char * src)
{
  FarString dst;
  const char * end = src + strlen(src);
  for (const char * ptr = src; ptr < end; ptr ++)
  {
    if (ptr > src && *(ptr-1) == '\\')
    {
      switch (*ptr)
      {
      case '\\':
        dst += '\\';
        src = ptr+1;
        break;
      case 'r':
        dst += '\r';
        break;
      case 'n':
        dst += '\n';
        break;
      case '\"':
        dst += '\"';
        break;
      case '\'':
        dst += '\'';
        break;
      case 'b':
        dst += '\b';
        break;
      case 't':
        dst += '\t';
        break;
      default:
        break;
      }
    }
    else if (*ptr != '\\')
      dst += *ptr;
  }

  return dst;
}

FarString CStringFromString(const char * src)
{
  FarString dst;
  const char * end = src + strlen(src);
  for (const char * ptr = src; ptr < end; ptr++)
  {
    switch(*ptr)
    {
      case '\\':
        dst += "\\\\";
        break;
      case '\r':
        dst += "\\r";
        break;
      case '\n':
        dst += "\\n";
        break;
      case '\"':
        dst += "\\\"";
        break;
      case '\'':
        dst += "\\\'";
        break;
      case '\b':
        dst += "\\b";
        break;
      case '\t':
        dst += "\\t";
        break;
      default:
        dst += *ptr;
        break;
    }
  }
  return dst;
}

CViewCfg::CViewCfg() : CMailViewCfg()
  , m_bLockView(false)
  , m_bUseLastSignature(false)
  , m_StyleCodePunct(DEFAULT_STYLECODEPUNCT)
  , m_StyleCodeStops(DEFAULT_STYLECODESTOPS)
  , m_DateFormat(STR_DateFmt)
  , m_TimeFormat(STR_TimeFmt)
{
}

void CViewCfg::LoadSettings( CIniConfig& ini )
{
  m_bLockView = ini.ReadBool( INI_MAILVIEW, "LockView", false );
  m_bUseLastSignature = ini.ReadBool( INI_MAILVIEW, "UseLastSignature", false );
  m_State = ini.ReadInt( INI_MAILVIEW, "State", 0 );

  char * Tmp = create char[ 0x8000 ];

  ini.ReadString(INI_MAILVIEW, "StyleCodePunct", DEFAULT_STYLECODEPUNCT, Tmp, 0x8000);
  m_StyleCodePunct = StringFromCString(Tmp);
  ini.ReadString(INI_MAILVIEW, "StyleCodeStops", DEFAULT_STYLECODESTOPS, Tmp, 0x8000);
  m_StyleCodeStops = StringFromCString(Tmp);

  ini.ReadString(INI_MAILVIEW, "DateFormat", STR_DateFmt, Tmp, 0x8000);
  m_DateFormat = Tmp;
  ini.ReadString(INI_MAILVIEW, "TimeFormat", STR_TimeFmt, Tmp, 0x8000);
  m_TimeFormat = Tmp;

  delete [] Tmp;

  m_Templates.LoadSettings( ini );
  m_Colors.LoadSettings( ini );
  m_HTMLFilter.LoadSettings( ini );
}

void CViewCfg::SaveSettings( CIniConfig& ini )
{
  WriteBool( ini, INI_MAILVIEW, "LockView", m_bLockView, false );
  WriteInt( ini, INI_MAILVIEW, "State", m_State, 0 );

//  WriteBool(ini, INI_MAILVIEW, "UseLastSignature", m_bUseLastSignature, false);

//  WriteString(ini, INI_MAILVIEW, "StyleCodePunct",
//    CStringFromString(m_StyleCodePunct), CStringFromString(DEFAULT_STYLECODEPUNCT), false);
//  WriteString(ini, INI_MAILVIEW, "StyleCodeStops",
//    CStringFromString(m_StyleCodeStops), CStringFromString(DEFAULT_STYLECODESTOPS), false);

//  WriteString(ini, INI_MAILVIEW, "DateFormat", m_DateFormat, STR_DateFmt, false);
//  WriteString(ini, INI_MAILVIEW, "TimeFormat", m_TimeFormat, STR_TimeFmt, false);

  m_Templates.SaveSettings( ini );
  m_Colors.SaveSettings( ini );
  m_HTMLFilter.SaveSettings( ini );
}

void CViewCfg::Configure()
{
  FarMenu mnu( MViewer );
  mnu.AddItem( MColors )->SetData( (DWORD)&m_Colors );

  mnu.AddItem( MViewCfg_HyperView )->SetData( (DWORD)&m_HTMLFilter );
  mnu.AddItem( MViewCfg_Templates )->SetData( (DWORD)&m_Templates );

  FarMenu::PItem Item;
  while ((Item = mnu.Show()) != NULL)
  {
    if (Item->GetData())
      ((CMailViewCfg*)Item->GetData())->Configure();
    mnu.SelectItem(Item);
  }
}

void CHTMLFilterCfg::LoadSettings( CIniConfig& ini )
{
  char * Tmp = create char[ 0x8000 ];

  ini.ReadString( INI_MAILVIEW, "HyperViewLib", STR_EmptyStr, Tmp, 0x8000 );
  m_hvLib = Tmp;

  ini.ReadString( INI_MAILVIEW, "HyperViewCfg", STR_EmptyStr, Tmp, 0x8000 );
  m_hvCfg = Tmp;

  m_Enabled = ini.ReadBool( INI_MAILVIEW, "UseHyperView", true );

  delete [] Tmp;
}

void CHTMLFilterCfg::SaveSettings( CIniConfig& ini )
{
  WriteString( ini, INI_MAILVIEW, "HyperViewLib", m_hvLib, STR_EmptyStr );
  WriteString( ini, INI_MAILVIEW, "HyperViewCfg", m_hvCfg, STR_EmptyStr );
  WriteBool( ini, INI_MAILVIEW, "UseHyperView", m_Enabled, true );
}

BOOL CHTMLFilterCfg::validate( char * text, void * data )
{
  if ( text == NULL || *text == '\0' )
    return TRUE;

  FarFileName f( text );

  if ( !File::exists( f.Expand() ) )
  {
    // TODO: локализовать
    DbgMsg( Far::GetMsg( MHyperViewConfiguration ),
      "File does not exists\n%s", f.c_str() );
    return FALSE;
  }

  return TRUE;
}

void CHTMLFilterCfg::Configure()
{
  FarDialog dlg( MHyperViewConfiguration, "HyperViewConfig" );

  dlg.AddText( MLibFullFileName );
  FarEditCtrl edtLibFileName( &dlg, m_hvLib, 0, 66 );
  edtLibFileName.SetNextY();

  dlg.AddText( MCfgFullFileName );
  FarEditCtrl edtCfgFileName( &dlg, m_hvCfg, 0, 66 );
  edtCfgFileName.SetNextY();

  dlg.AddSeparator();
  int bEnabled = getEnabled() ? TRUE : FALSE;
  FarCheckCtrl chkEnabled( &dlg, MEnabled, &bEnabled );

  dlg.AddSeparator();
  FarButtonCtrl btnOk( &dlg, MOk );
  FarButtonCtrl btnCancel( &dlg, MCancel, DIF_CENTERGROUP );

  dlg.SetDefaultControl( &btnOk );
  if ( bEnabled )
    dlg.SetFocusControl( &edtLibFileName );
  else
    dlg.SetFocusControl( &chkEnabled );

  edtLibFileName.SetValidateFunc( validate );
  edtCfgFileName.SetValidateFunc( validate );

  btnOk.EnableValidate( TRUE );

  if ( dlg.Show() == &btnOk )
  {
    m_hvLib = edtLibFileName.GetText();
    m_hvCfg = edtCfgFileName.GetText();
    setEnabled( bEnabled != FALSE );
  }
}

FarFileName CTemplatesCfg::FixTpl( const FarFileName& name ) const
{
  FarFileName result = FarString(name).Expand();
  //if (result.IsEmpty())
  //  result.SetPath(m_root);
  if (name.IsEmpty())
    result.SetName("MailView.tpl");
  if (!IsNotRelativePath(result))
    result.SetPath(m_root);
  return result;
}

void CTemplatesCfg::LoadSettings( CIniConfig& ini )
{
  char * Tmp = create char[ 0x8000 ];

  m_root = FarFileName( ini.GetIniFileName() ).GetPath();

  ini.ReadString(INI_MAILVIEW, "qTpl", STR_EmptyStr, Tmp, 0x8000);
  m_qTpl = Tmp;

  ini.ReadString(INI_MAILVIEW, "eTpl", STR_EmptyStr, Tmp, 0x8000);
  m_eTpl = Tmp;

  ini.ReadString(INI_MAILVIEW, "vTpl", STR_EmptyStr, Tmp, 0x8000);
  m_vTpl = Tmp;

  delete [] Tmp;
}

void CTemplatesCfg::SaveSettings( CIniConfig& ini )
{
  FarFileName Root = FarFileName(ini.GetIniFileName()).GetPath();

  FarFileName Def = FarFileName(ini.GetIniFileName()).GetPath();
  Def.SetName("MailView.tpl");
  Def = ExtractRelativePath(Root, Def);

  WriteString(ini, INI_MAILVIEW, "qTpl", ExtractRelativePath(Root, FixTpl(m_qTpl)), Def);
  WriteString(ini, INI_MAILVIEW, "eTpl", ExtractRelativePath(Root, FixTpl(m_eTpl)), Def);
  WriteString(ini, INI_MAILVIEW, "vTpl", ExtractRelativePath(Root, FixTpl(m_vTpl)), Def);
}

void CTemplatesCfg::Configure()
{
  FarDialog dlg( MViewCfg_Templates, "TemplatesConfig" );

  dlg.AddText( MViewCfg_Templates_Editor );
  FarEditCtrl edtETpl( &dlg, m_eTpl, 0, 66 );
  edtETpl.SetNextY();

  dlg.AddText( MViewCfg_Templates_Viewer );
  FarEditCtrl edtVTpl( &dlg, m_vTpl, 0, 66 );
  edtVTpl.SetNextY();

  FarEditCtrl * edtQTpl = NULL;
  if ( Far::GetBuildNumber() >= 1376 )
  {
    dlg.AddText( MViewCfg_Templates_QuickView );
    edtQTpl = new FarEditCtrl( &dlg, m_qTpl, 0, 66 );
    edtQTpl->SetNextY();
    dlg.AddOwnedControl( edtQTpl );
  }

  dlg.AddSeparator();
  FarButtonCtrl btnOk( &dlg, MOk );
  FarButtonCtrl btnCancel( &dlg, MCancel, DIF_CENTERGROUP );

  dlg.SetDefaultControl( &btnOk );
  dlg.SetFocusControl( &edtETpl );

  dlg.Show();

  m_eTpl = edtETpl.GetText();
  m_vTpl = edtVTpl.GetText();

  if ( edtQTpl )
    m_qTpl = edtQTpl->GetText();
}
