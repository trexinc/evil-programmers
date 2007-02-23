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
// FarMultiLang.h: interface for the FarMultiLang class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ___FarMLang_h___
#define ___FarMLang_h___

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _COM_NO_STANDARD_GUIDS_
//#include <ComDef.h>
#include <objbase.h>
#include "MLang.h"
#include <FarPlus.h>
#include <FarRegistry.h>
#include <FarHash.h>
#include "IniConfig.h"

#define FCT__INVALID   -1
#define FCT_DEFAULT    (FCT_DETECT|(1<<0))
#define FCT_OEM        (FCT_DETECT|(1<<1))
#define FCT_ANSI       (FCT_DETECT|(1<<2))
//#define FCT_UTF7       (FCT_DETECT|(1<<3))
//#define FCT_UTF8       (FCT_DETECT|(1<<4))
#define FCT_AUTODETECT FCT_DETECT

#define IS_RFC_CP(Table) (Table&0x80000000)
#define IS_FAR_CP(Table) (!IS_RFC_CP(Table))


#define _USE_FARRFCCHARSET

class FarMultiLang
{
private:
  IMultiLanguage * m_MLang;
#ifndef _USE_FARRFCCHARSET
  int CalcHash( LPCSTR IniSectionContent, int IniSectionContentLen, IEnumCodePage * EnumCP );
#else
  int CalcHash( FarRegistry *Reg, IEnumCodePage * EnumCP );
#endif
  FarStringHash * m_MimeNames;
  //long ShowRFCMenu( long SelectedTable );
  long GetCPFromMimeDB( LPCSTR RfcEncoding );

public:
#ifndef _USE_FARRFCCHARSET
  FarMultiLang( CIniConfig & ini );
#else
  FarMultiLang();
#endif
  virtual ~FarMultiLang();

  long SelectCharacterTableMenu( long SelectedTable, bool rfcOnly = false );

  long getCharacterTable( LPCSTR RFCName );
  long getCharacterTable( LPCSTR RFCName, LPCSTR Default );

  FarString getDisplayCharsetName( long table );
  FarString getCharsetName( long table );

  void getFarCharacterTables( FarStringArray & name, FarIntArray & data, bool rfcOnly );
  void getRfcCharacterTables( FarStringArray & name, FarIntArray & data, bool addHotKeys );
};

FarString ToOEMString( const FarString & Str, long FarEncoding );
FarString ToOEMString( const FarString & Str, LPCSTR RfcEncoding );

#endif // !defined(___FarMLang_h___)
