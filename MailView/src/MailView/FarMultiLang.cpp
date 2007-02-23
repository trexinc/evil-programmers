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
// FarMultiLang.cpp: implementation of the FarMultiLang class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <FarMenu.h>
#include "FarMultiLang.h"
#include "LangID.h"

#define CP_X_USER_DEFINED 50000

#ifdef __cplusplus
extern "C"{
#endif


const IID LIBID_MultiLanguage = {0x275c23e0,0x3747,0x11d0,{0x9f,0xea,0x00,0xaa,0x00,0x3f,0x86,0x46}};


const IID IID_IMLangStringBufW = {0xD24ACD21,0xBA72,0x11D0,{0xB1,0x88,0x00,0xAA,0x00,0x38,0xC9,0x69}};


const IID IID_IMLangStringBufA = {0xD24ACD23,0xBA72,0x11D0,{0xB1,0x88,0x00,0xAA,0x00,0x38,0xC9,0x69}};


const IID IID_IMLangString = {0xC04D65CE,0xB70D,0x11D0,{0xB1,0x88,0x00,0xAA,0x00,0x38,0xC9,0x69}};


const IID IID_IMLangStringWStr = {0xC04D65D0,0xB70D,0x11D0,{0xB1,0x88,0x00,0xAA,0x00,0x38,0xC9,0x69}};


const IID IID_IMLangStringAStr = {0xC04D65D2,0xB70D,0x11D0,{0xB1,0x88,0x00,0xAA,0x00,0x38,0xC9,0x69}};


const CLSID CLSID_CMLangString = {0xC04D65CF,0xB70D,0x11D0,{0xB1,0x88,0x00,0xAA,0x00,0x38,0xC9,0x69}};


const IID IID_IMLangLineBreakConsole = {0xF5BE2EE1,0xBFD7,0x11D0,{0xB1,0x88,0x00,0xAA,0x00,0x38,0xC9,0x69}};


const IID IID_IEnumCodePage = {0x275c23e3,0x3747,0x11d0,{0x9f,0xea,0x00,0xaa,0x00,0x3f,0x86,0x46}};


const IID IID_IEnumRfc1766 = {0x3dc39d1d,0xc030,0x11d0,{0xb8,0x1b,0x00,0xc0,0x4f,0xc9,0xb3,0x1f}};


const IID IID_IMLangConvertCharset = {0xd66d6f98,0xcdaa,0x11d0,{0xb8,0x22,0x00,0xc0,0x4f,0xc9,0xb3,0x1f}};


const CLSID CLSID_CMLangConvertCharset = {0xd66d6f99,0xcdaa,0x11d0,{0xb8,0x22,0x00,0xc0,0x4f,0xc9,0xb3,0x1f}};


const IID IID_IMultiLanguage = {0x275c23e1,0x3747,0x11d0,{0x9f,0xea,0x00,0xaa,0x00,0x3f,0x86,0x46}};


const IID IID_IMLangCodePages = {0x359F3443,0xBD4A,0x11D0,{0xB1,0x88,0x00,0xAA,0x00,0x38,0xC9,0x69}};


const IID IID_IMLangFontLink = {0x359F3441,0xBD4A,0x11D0,{0xB1,0x88,0x00,0xAA,0x00,0x38,0xC9,0x69}};


const CLSID CLSID_CMultiLanguage = {0x275c23e2,0x3747,0x11d0,{0x9f,0xea,0x00,0xaa,0x00,0x3f,0x86,0x46}};


#ifdef __cplusplus
}
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FarString GetFarKey()
{
  FarString FarKey( "Software\\Far\\" );
  FarString FarUser = FarSF::GetEnvironmentVariable( "FARUSER" );
  if ( !FarUser.IsEmpty() )
    FarKey += "Users\\" + FarUser;
  return FarKey;
}
#ifndef _USE_FARRFCCHARSET
FarMultiLang::FarMultiLang( CIniConfig & ini ) : m_MLang( NULL )
#else
FarMultiLang::FarMultiLang() : m_MLang( NULL )
#endif
{
  CoInitialize( 0 );

  IEnumCodePage  * EnumCP = NULL;
  if ( SUCCEEDED( CoCreateInstance( CLSID_CMultiLanguage, NULL, CLSCTX_SERVER,
    IID_IMultiLanguage, (void**)&m_MLang ) ) && m_MLang != NULL )
    m_MLang->EnumCodePages( MIMECONTF_MAILNEWS|MIMECONTF_VALID, &EnumCP );

  //////////////////////////////////////////////////////////////////////////
#ifndef _USE_FARRFCCHARSET
  char * Keys  = create char[ 0x8000 ];
  char * Value = create char[ 0x0080 ];

  int Len = ini.ReadString( "RFC Charset", NULL, STR_EmptyStr, Keys, 0x8000 );

  m_MimeNames = create FarStringHash( CalcHash( Keys, Len, EnumCP ), true, true );
  CharTableSet ctSet;

  char * p = Keys;
  for ( int i = 0; i < Len; i += strlen( p ) + 1 )
  {
    p = Keys + i;

    int Len = ini.ReadString( "RFC Charset", p, STR_EmptyStr, Value, 0x80 );

    if ( Len > 0 ) for ( int j = 0; Far::GetCharTable( j, &ctSet ) != FCT__INVALID; j++ )
      if ( FarSF::LStrnicmp( Value, ctSet.TableName, Len ) == 0 )
      {
        Len = strlen( p ) + 1;
        char * RFCName = (char*)memcpy( create char[ Len ], p, Len );
        m_MimeNames->Insert( RFCName, j + 1 );
        delete [] RFCName;
        break;
      }
  }
  delete [] Value;
  delete [] Keys;
#else

  CharTableSet ctSet;
  FarRegistry Reg( GetFarKey() );

  m_MimeNames = create FarStringHash( CalcHash( &Reg, EnumCP ), true, true );

  FarRegistry::KeyIterator *Iter = Reg.EnumKeys( "CodeTables\\" );
  FarString Key, Val;
  while ( Iter->NextKey( Key ) )
  {
    Val = Reg.GetRegStr( "CodeTables\\" + Key, "RFCCharset", STR_EmptyStr );

    bool bDoneKey = false;

    FarStringTokenizer tokenizer( Val );

    while ( !bDoneKey && tokenizer.HasNext() )
    {
      FarString Tkn = tokenizer.NextToken();
      if ( !Tkn.IsEmpty() && m_MimeNames->Find( Val.c_str() ) == 0 )
      {
        FarString TableName = Reg.GetRegStr( "CodeTables\\" + Key, "TableName", Tkn );
        for ( int i = 0; Far::GetCharTable( i, &ctSet ) != FCT__INVALID; i++ )
        {
          if ( TableName.CompareNoCase( ctSet.TableName ) == 0 )
          {
            tokenizer.Attach( Val );
            while ( tokenizer.HasNext() )
              m_MimeNames->Insert( tokenizer.NextToken(), i + 1 );
            bDoneKey = true;
            break;
          }
        }
      }
    }
  }
  delete Iter;

#endif
  //////////////////////////////////////////////////////////////////////////

  if ( m_MLang && EnumCP )
  {
    MIMECPINFO cpInfo;

    EnumCP->Reset();
    while ( EnumCP->Next( 1, &cpInfo, NULL ) == S_OK )
    {
      if ( m_MLang->IsConvertible( cpInfo.uiCodePage, CP_OEMCP ) != S_OK )
        continue;
      int RFCLen = WideCharToMultiByte( CP_OEMCP, 0, cpInfo.wszWebCharset, -1, NULL, 0, NULL, NULL );
      char * RFCName = create char[ RFCLen + 1 ];
      WideCharToMultiByte( CP_OEMCP, 0, cpInfo.wszWebCharset, -1, RFCName, RFCLen, NULL, NULL );
      m_MimeNames->Insert( RFCName, (cpInfo.uiCodePage|0x80000000) + 1 );
      delete [] RFCName;
    }
  }

  if( EnumCP )
    EnumCP->Release();
}

FarMultiLang::~FarMultiLang()
{
  if ( m_MimeNames )
    delete m_MimeNames;
  if ( m_MLang )
    m_MLang->Release();
  CoUninitialize();
}

#ifndef _USE_FARRFCCHARSET
int FarMultiLang::CalcHash( LPCSTR IniSectionContent, int IniSectionContentLen, IEnumCodePage * EnumCP )
{
  int result = 0;
  for ( int i = 0; i < IniSectionContentLen; i ++ ) if ( IniSectionContent[ i ] == 0 )
    result ++;
#else
int FarMultiLang::CalcHash( FarRegistry *Reg, IEnumCodePage * EnumCP )
{
  int result = 0;
  FarRegistry::KeyIterator *Iter = Reg->EnumKeys( "CodeTables\\" );
  FarString Key, Val;
  while ( Iter->NextKey( Key ) )
  {
    Val = Reg->GetRegStr( "CodeTables\\" + Key, "RFCCharset", STR_EmptyStr );
    if ( !Val.IsEmpty() )
    {
      FarStringTokenizer tokenizer( Val );
      while ( tokenizer.HasNext() )
      {
        tokenizer.NextToken();
        result ++;
      }
    }
  }
  delete Iter;
#endif
  if ( EnumCP != NULL )
  {
    EnumCP->Reset();
    MIMECPINFO cpInfo;
    while ( EnumCP->Next( 1, &cpInfo, NULL ) == S_OK )
      result ++;
  }
  return result;
}

/*long FarMultiLang::ShowRFCMenu( long SelectedTable )
{
  if ( m_MLang == NULL )
    return FCT__INVALID;

  FarMenu mnu( MTables );
  FarMenu::PItem Item;
  mnu.SetFlags( FMENU_WRAPMODE );

  FarStringArray name;
  FarIntArray data;
  getRfcCharacterTables( name, data, true );

  for ( int i = 0; i < name.Count(); i ++ )
  {
    long enc = data.At( i );
    if ( enc == FCT__INVALID )
    {
      mnu.AddSeparator();
    }
    else
    {
      Item = mnu.AddItem( name.At( i ) );
      Item->SetData( enc );
      if ( SelectedTable == enc )
        mnu.SelectItem( Item );
    }
  }

  Item = mnu.Show();

  return Item ? ( Item->GetData() | 0x80000000 ) : FCT__INVALID;
}
*/

void FarMultiLang::getFarCharacterTables( FarStringArray & name, FarIntArray & data, bool rfcOnly )
{
  name.Add( Far::GetMsg( MtblDefault ) );
  data.Add( FCT_DEFAULT );

  name.Add( Far::GetMsg( MtblDos ) );
  data.Add( FCT_OEM );

  name.Add( Far::GetMsg( MtblWindows ) );
  data.Add( FCT_ANSI );

  name.Add( STR_EmptyStr );
  data.Add( FCT__INVALID );

  int cnt = data.Count();

  CharTableSet ctSet;

  FarRegistry reg( GetFarKey() );
  for ( int i = 0; Far::GetCharTable( i, &ctSet ) != FCT__INVALID; i++ )
  {
    if ( rfcOnly )
    {
      FarString val;
      if ( Far::GetBuildNumber() > 1280 )
      {
        FarRegistry::KeyIterator *iter = reg.EnumKeys( "CodeTables\\" );
        FarString key;
        while ( iter->NextKey( key ) )
        {
          FarString tableName = reg.GetRegStr( "CodeTables\\" + key, "TableName", STR_EmptyStr );
          if ( tableName.CompareNoCase( ctSet.TableName ) == 0 )
          {
            val = reg.GetRegStr( "CodeTables\\" + key, "RFCCharset", STR_EmptyStr );
            break;
          }
        }
        delete iter;
      }
      else
      {
        val = reg.GetRegStr( "CodeTables\\" + FarString( ctSet.TableName ), "RFCCharset", STR_EmptyStr );
      }

      if ( val.IsEmpty() )
        continue;

    }

    name.Add( ctSet.TableName );
    data.Add( i );
  }

  if ( data.Count() > cnt )
  {
    name.Add( STR_EmptyStr );
    data.Add( FCT__INVALID );
  }

  name.Add( Far::GetMsg( MtblAutodetect ) );
  data.Add( FCT_AUTODETECT );
}

void FarMultiLang::getRfcCharacterTables( FarStringArray & name, FarIntArray & data, bool addHotKeys )
{
  if ( m_MLang == NULL )
    return ;

  IEnumCodePage  * EnumCP = NULL;
  if ( SUCCEEDED( m_MLang->EnumCodePages( MIMECONTF_MAILNEWS|MIMECONTF_VALID,
    &EnumCP ) && EnumCP != NULL ) )
  {
    EnumCP->Reset();
    MIMECPINFO cpInfo;
    DWORD FamilyCP = FCT__INVALID;
    int Id = '0';
    while ( EnumCP->Next( 1, &cpInfo, NULL ) == S_OK )
    {
      if (cpInfo.uiCodePage == CP_X_USER_DEFINED ) // "x-user-defined"
        continue;
      int Len = WideCharToMultiByte( CP_OEMCP, 0, cpInfo.wszDescription, -1, NULL, 0, NULL, NULL );
      char * Name = create char[ Len + 5 ];

      if ( addHotKeys )
      {
        Name[ 0 ] = '&';
        Name[ 1 ] = Id;
        Name[ 2 ] = '.';
        Name[ 3 ] = '\x20';
      }
      WideCharToMultiByte( CP_OEMCP, 0, cpInfo.wszDescription, -1, Name+(addHotKeys?4:0), Len+1, NULL, NULL );
      if ( FamilyCP != cpInfo.uiFamilyCodePage && FamilyCP != FCT__INVALID )
      {
        name.Add( STR_EmptyStr );
        data.Add( FCT__INVALID );
      }
      name.Add( addHotKeys ? ( Id == 'x' ? Name + 1: Name ) : Name  );
      data.Add( cpInfo.uiCodePage | 0x80000000 );

      delete [] Name;
      FamilyCP = cpInfo.uiFamilyCodePage;

      if ( Id >= '9' )
      {
        if ( Id != 'x' )
        {
          if ( Id < 'A' )
            Id = 'A';
          else if ( Id < 'X' )
            Id ++;
          else
            Id = 'x';
        }
      }
      else
        Id ++;
    }
    EnumCP->Release();
  }
}

long FarMultiLang::SelectCharacterTableMenu( long SelectedTable, bool rfcOnly )
{
  FarMenu mnu( MTables );

  FarStringArray name;
  FarIntArray data;
  getFarCharacterTables( name, data, rfcOnly );

  FarMenu::PItem item;

  for ( int i = 0; i < name.Count(); i ++ )
  {
    long enc = data.At( i );
    if ( enc == FCT__INVALID )
    {
      mnu.AddSeparator();
    }
    else
    {
      item = mnu.AddItem( name.At( i ) );
      item->SetData( enc );
      if ( SelectedTable == enc )
        mnu.SelectItem( item );
    }
  }

  if ( m_MLang )
  {
    mnu.AddSeparator();

    if ( IS_RFC_CP( SelectedTable ) && m_MLang )
    {
      MIMECPINFO cpInfo;
      if ( SUCCEEDED( m_MLang->GetCodePageInfo( SelectedTable & ~0x80000000, &cpInfo ) ) )
      {
        int Len = WideCharToMultiByte( CP_OEMCP, 0, cpInfo.wszDescription, -1, NULL, 0, NULL, NULL );
        char * Name = create char[ Len + 1 ];
        WideCharToMultiByte( CP_OEMCP, 0, cpInfo.wszDescription, -1, Name, Len, NULL, NULL );
        item = mnu.AddItem( Name );
        item->SetData( cpInfo.uiCodePage | 0x80000000 );
        if ( (SelectedTable & ~0x80000000) == cpInfo.uiCodePage )
          mnu.SelectItem( item );
        delete [] Name;
      }
    }

    item = mnu.AddItem( MtblMore );
    item->SetSubMenu();
    far_assert( item->GetSubMenu() != NULL );

    FarMenu& sub = *item->GetSubMenu();
    sub.SetTitle( MTables );

    name.Clear();
    data.Clear();

    getRfcCharacterTables( name, data, true );

    for ( int i = 0; i < name.Count(); i ++ )
    {
      long enc = data.At( i );
      if ( enc == FCT__INVALID )
      {
        sub.AddSeparator();
      }
      else
      {
        item = sub.AddItem( name.At( i ) );
        item->SetData( enc );
        if ( SelectedTable == enc )
          sub.SelectItem( item );
      }
    }
  }

  item = mnu.Show();

  return item ? item->GetData() : FCT__INVALID;
}

FarString FarMultiLang::getDisplayCharsetName( long table )
{
  if ( table == FCT_DEFAULT )
    return Far::GetMsg( MtblDefault );
  if ( table == FCT_OEM )
    return Far::GetMsg( MtblDos );
  if ( table == FCT_ANSI )
    return Far::GetMsg( MtblWindows );
  if ( table == FCT_AUTODETECT )
    return Far::GetMsg( MtblAutodetect );

  CharTableSet ctSet;

  if ( IS_RFC_CP( table ) && m_MLang )
  {
    MIMECPINFO cpInfo;
    if ( SUCCEEDED( m_MLang->GetCodePageInfo( table & ~0x80000000, &cpInfo ) ) )
    {
      int Len = WideCharToMultiByte( CP_OEMCP, 0, cpInfo.wszDescription, -1, NULL, 0, NULL, NULL );
      FarString result;
      //char * Name = create char[ Len + 1 ];
      WideCharToMultiByte( CP_OEMCP, 0, cpInfo.wszDescription, -1,
        result.GetBuffer( Len ), Len, NULL, NULL );
      result.ReleaseBuffer( Len );
      return result;
    }
  }
  else if ( Far::GetCharTable( table, &ctSet ) == table )
    return ctSet.TableName;

  return FarString( "UNKNOWN" );
}

FarString FarMultiLang::getCharsetName( long table )
{
  if ( table == FCT_DEFAULT )
    return FarString( ".none" );
  if ( table == FCT_OEM )
    return FarString( ".dos" );
  if ( table == FCT_ANSI )
    return FarString( ".win" );
  if ( table == FCT_AUTODETECT )
    return FarString( ".auto" );

  if ( IS_RFC_CP( table ) && m_MLang )
  {
    MIMECPINFO cpInfo;
    if ( SUCCEEDED( m_MLang->GetCodePageInfo( table & ~0x80000000, &cpInfo ) ) )
    {
      int Len = WideCharToMultiByte( CP_OEMCP, 0, cpInfo.wszWebCharset, -1, NULL, 0, NULL, NULL );
      FarString result;
      //char * Name = create char[ Len + 1 ];
      WideCharToMultiByte( CP_OEMCP, 0, cpInfo.wszWebCharset, -1,
        result.GetBuffer( Len ), Len, NULL, NULL );
      result.ReleaseBuffer( Len );
      return result;
    }
  }

  CharTableSet ctSet;
  if ( Far::GetCharTable( table, &ctSet ) == table )
  {
    // TODO: пробежаться в реестре по фаровсим таблицам
    FarRegistry reg( GetFarKey() );

    FarString val;
    if ( Far::GetBuildNumber() > 1280 )
    {
      FarRegistry::KeyIterator *iter = reg.EnumKeys( "CodeTables\\" );
      FarString key;
      while ( iter->NextKey( key ) )
      {
        FarString tableName = reg.GetRegStr( "CodeTables\\" + key, "TableName", STR_EmptyStr );
        if ( tableName.CompareNoCase( ctSet.TableName ) == 0 )
        {
          val = reg.GetRegStr( "CodeTables\\" + key, "RFCCharset", STR_EmptyStr );
          break;
        }
      }
      delete iter;
    }
    else
    {
      val = reg.GetRegStr( "CodeTables\\" + FarString( ctSet.TableName ), "RFCCharset", STR_EmptyStr );
    }

    return val;
  }

  return FarString( ".none" );
}

long FarMultiLang::getCharacterTable( LPCSTR RFCName )
{
  if ( *RFCName == '.' )
  {
    RFCName ++;

    if ( FarSF::LStricmp( RFCName, "dos" ) == 0 )
      return FCT_OEM;
    if ( FarSF::LStricmp( RFCName, "win" ) == 0 )
      return FCT_ANSI;
    if ( FarSF::LStricmp( RFCName, "auto" ) == 0 )
      return FCT_DETECT;
    if ( FarSF::LStricmp( RFCName, "none" ) == 0 )
      return FCT_DEFAULT;

    return FCT__INVALID;
  }

  return m_MimeNames->Find( RFCName ) - 1;
}

long FarMultiLang::getCharacterTable( LPCSTR RFCName, LPCSTR Default )
{
  long result = getCharacterTable( RFCName );

  if ( result == FCT__INVALID )
  {
    result = GetCPFromMimeDB( RFCName );
    if ( result != FCT__INVALID )
      m_MimeNames->Insert( RFCName, result + 1 );
    else
    {
      result = getCharacterTable( Default );
      if ( result == FCT__INVALID )
      {
        result = GetCPFromMimeDB( Default );
        if ( result != FCT__INVALID )
          m_MimeNames->Insert( Default, result + 1 );
      }
    }
  }

  return result;
}

FarString ToOEMString( const FarString & Str, long Encoding )
{
  if ( Str.IsEmpty() )
    return FarString();

  if ( Encoding == FCT_DETECT &&
    ( Encoding = Far::DetectCharTable( Str, Str.Length() ) ) == FCT__INVALID )
    return Str;

  if ( IS_FAR_CP( Encoding ) )
  {
    if ( Encoding == FCT_ANSI )
      return Str.ToOEM();

    FarString result = Str;

    CharTableSet ctSet;
    Far::GetCharTable( Encoding, &ctSet );
    for ( int i = 0; i < result.Length(); i ++ )
      result[ i ] = ctSet.DecodeTable[ (BYTE)result[ i ] ];

    return result;
  }
  else
  {
    UINT cp = Encoding & ~0x80000000/*0x7FFFFFF*/;
    if ( cp == GetOEMCP() )
      return Str;

    FarString result;

    int Len = MultiByteToWideChar( cp, 0, Str.c_str(), Str.Length(), NULL, 0 );
    wchar_t * Tmp = create wchar_t[ Len + 1 ];
    if ( MultiByteToWideChar( cp, 0, Str.c_str(), Str.Length(),
      Tmp, Len+1 ) != 0 )
    {
      WideCharToMultiByte( CP_OEMCP, 0, Tmp,
        Len, result.GetBuffer( Str.Length() ), Str.Length(), NULL, NULL );
      result.ReleaseBuffer( Len );
    }
    else
      result = Str;
    delete [] Tmp;

    return result;
  }

  return Str;
}

long FarMultiLang::GetCPFromMimeDB( LPCSTR RfcEncoding )
{
  far_assert( RfcEncoding != NULL );

  FarRegistry Reg( "Mime\\Database\\Charset\\" );

  FarString Val = Reg.GetRegStr( RfcEncoding, "AliasForCharset", STR_EmptyStr, HKEY_CLASSES_ROOT );

  long Res = FCT__INVALID;

  if ( !Val.IsEmpty() )
    Res = Reg.GetRegKey( Val, "InternetEncoding", FCT__INVALID, HKEY_CLASSES_ROOT );
  else
    Res = Reg.GetRegKey( RfcEncoding, "InternetEncoding", FCT__INVALID, HKEY_CLASSES_ROOT );

  if ( Res != FCT__INVALID )
    Res |= 0x80000000;

  return Res;
}
