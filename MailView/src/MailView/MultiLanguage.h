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
#ifndef ___MultiLanguage_H___
#define ___MultiLanguage_H___

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __PLUGIN_HPP__
#include "../plugin.hpp"
#else
#include <windows.h>
#endif


extern HMODULE LoadLibrary( BSTR, long );

class CMultiLanguage
  {
  typedef void (__cdecl *TOnInit)(const FARSTANDARDFUNCTIONS *);
  typedef void (__cdecl *TOnExit)();
  typedef long (__cdecl *TGetCount)();
  typedef const BSTR (__cdecl *TGetName)( const long );
  typedef const BSTR (__cdecl *TGetMimeName)( const long );
  typedef long (__cdecl *TGetIndex)( const BSTR );
  typedef long (__cdecl *TGetCP)( const long );
  typedef long (__cdecl *TGetFamilyCP)( const long );
  typedef long (__cdecl *TUncToStr)( const DWORD, const BSTR, const long, PSTR, const long );
  typedef long (__cdecl *TStrToUnc)( const DWORD, const PSTR, const long, BSTR, const long );

  private:
    HMODULE      m_hLib;
    TOnInit      m_pOnInit;
    TOnExit      m_pOnExit;
    TGetCount    m_pGetCount;
    TGetName     m_pGetName;
    TGetMimeName m_pGetMimeName;
    TGetIndex    m_pGetIndex;
    TGetCP       m_pGetCP;
    TGetFamilyCP m_pGetFamilyCP;
    TUncToStr    m_pUncToStr;
    TStrToUnc    m_pStrToUnc;
  public:
    CMultiLanguage( const BSTR FileName ) : m_hLib( NULL )
      {
      m_pOnExit = NULL;
      Attach( FileName );
      }
    CMultiLanguage( HMODULE hLib ) : m_hLib( NULL )
      {
      m_pOnExit = NULL;
      Attach( hLib );
      }

    ~CMultiLanguage()
      {
      Detach();
      }
    void Detach()
      {
      if ( m_pOnExit )
        {
        m_pOnExit();
        }
      FreeLibrary( m_hLib );
      m_hLib         = NULL;
      m_pOnInit      = NULL;
      m_pOnExit      = NULL;
      m_pGetCount    = NULL;
      m_pGetName     = NULL;
      m_pGetMimeName = NULL;
      m_pGetIndex    = NULL;
      m_pGetCP       = NULL;
      m_pGetFamilyCP = NULL;
      m_pUncToStr    = NULL;
      m_pStrToUnc    = NULL;
      }

    void Attach( const BSTR FileName )
      {
      Attach( LoadLibrary( FileName, 0 ) );
      }

    void Attach( HMODULE hLib );

    long GetCount()
      {
      if ( m_pGetCount )
        {
        return m_pGetCount();
        }
      return 2;
      }
    BSTR GetDescription( const long Index )
      {
      if ( m_pGetName )
        {
        return m_pGetName( Index );
        }
      wchar_t * CPNames[2] = { L"ANSI", L"OEM" };

      if ( Index == CP_ACP )
        {
        return CPNames[ 0 ];
        }
      if ( Index == CP_OEMCP )
        {
        return CPNames[ 1 ];
        }
      return NULL;
      }
    BSTR GetMimeName( const long Index )
      {
      if ( m_pGetMimeName )
        {
        return m_pGetMimeName( Index );
        }
      return NULL;
      }
    long GetIndex( const BSTR MimeName )
      {
      if ( m_pGetIndex )
        {
        return m_pGetIndex( MimeName );
        }
      return -1;
      }
    DWORD GetCP( const long Index )
      {
      if ( m_pGetCP )
        {
        return m_pGetCP( Index );
        }
      if ( Index == CP_ACP )
        {
        return GetACP();
        }
      if ( Index == CP_OEMCP )
        {
        return GetOEMCP();
        }
      return -1;
      }
    DWORD GetFamilyCP( const long Index )
      {
      if ( m_pGetFamilyCP )
        {
        return m_pGetFamilyCP( Index );
        }
      if ( Index == CP_ACP || Index == CP_OEMCP )
        {
        return GetACP();
        }
      return -1;
      }

/*
    __declspec(property(get=GetCount)) long Count;
    __declspec(property(get=GetDescription)) const BSTR Description[];
    __declspec(property(get=GetMimeName)) const BSTR MimeName[];
    __declspec(property(get=GetIndex)) long Index[];
    __declspec(property(get=GetCP)) DWORD CodePage[];
    __declspec(property(get=GetFamilyCP)) DWORD FamilyCP[];
*/

    long UncToStr( const DWORD CP, const BSTR Src, const long SrcLen, PSTR Dst, const long DstLen )
      {
      if ( m_pUncToStr )
        {
        m_pUncToStr( CP, Src, SrcLen, Dst, DstLen );
        }
      return WideCharToMultiByte( CP, 0, Src, SrcLen, Dst, DstLen, NULL, NULL );
      }

    long StrToUnc( const DWORD CP, const PSTR Src, const long SrcLen, BSTR Dst, const long DstLen )
      {
      if ( m_pStrToUnc )
        {
        return m_pStrToUnc( CP, Src, SrcLen, Dst, DstLen );
        }
      return MultiByteToWideChar( CP, 0, Src, SrcLen, Dst, DstLen );
      }

    PSTR UncToStr( const DWORD CP, const BSTR Src )
      {
      long SrcLen = _StrLenW( Src );
      long DstLen = UncToStr( CP, Src, SrcLen, NULL, 0 );
      PSTR Dst = _StrAllocA( ++DstLen );
      Dst[ UncToStr( CP, Src, SrcLen, Dst, --DstLen ) ] = 0;
      return Dst;
      }

    BSTR StrToUnc( const DWORD CP, const PSTR Src )
      {
      long SrcLen = _StrLenA( Src );
      long DstLen = StrToUnc( CP, Src, SrcLen, NULL, 0 );
      BSTR Dst = _StrAllocW( ++DstLen );
      Dst[ StrToUnc( CP, Src, SrcLen, Dst, --DstLen ) ] = 0;
      return Dst;
      }

    long SelectCodePage( const long InitialCodePage )
      {
      FarMenuItem * pItems = 0;

      UINT cp = GetOEMCP();

      long Count = GetCount();
      pItems = create FarMenuItem[ Count * 2 ];
      long * pCPs = create long[ Count * 2 ];

      UINT fcp = GetFamilyCP(0);
      for ( long i = 0, ItemsCount = 0; i < Count; i ++ )
        {
        DWORD cfcp = GetFamilyCP(i);
        if ( cfcp != fcp )
          {
          fcp = cfcp;
          pItems[ ItemsCount ].Separator = TRUE;
          pItems[ ItemsCount ].Checked   = FALSE;
          pItems[ ItemsCount ].Selected  = FALSE;
          pItems[ ItemsCount ].Text[ 0 ] = 0;
          ItemsCount ++;
          }
        pItems[ ItemsCount ].Separator = FALSE;
        pItems[ ItemsCount ].Checked   = FALSE;
        pItems[ ItemsCount ].Selected  = FALSE;
        pItems[ ItemsCount ].Text[ this->UncToStr( cp, GetDescription(i), -1, pItems[ ItemsCount ].Text, 128 ) ] = 0;

        pCPs[ ItemsCount ] = GetCP(i);

        if ( InitialCodePage == pCPs[ ItemsCount ] )
          {
          pItems[ ItemsCount ].Selected  = TRUE;
          }

        ItemsCount ++;
        }

      int nItem = _Plugin->Menu( CCoord(-1,-1), 0, FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT, "Character Tables", NULL, NULL, NULL, NULL, pItems, ItemsCount );

      if ( nItem != -1 )
        {
        nItem = pCPs[ nItem ];
        }

      delete [] pItems;
      delete [] pCPs;

      return nItem;
      }

  };

#endif // !defined(___MultiLanguage_H___)
