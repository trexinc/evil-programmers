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
/*
#include "MultiLng.h"
#include "WChar.h"
#include "../Shared/Wrap.h"

extern bool IsWinNT();
/////////////////////////////////////////////////////////////////////////////
//
LONG RegOpenKeyU( HKEY hKey, LPCWSTR lpSubKey, REGSAM saMask, PHKEY pResult )
  {
  if ( IsWinNT() )
    {
    return RegOpenKeyExW( hKey, lpSubKey, 0, saMask, pResult );
    }

  char * buffer = StrAllocA( 1024 );
  buffer[ UncToStr( CP_ACP, lpSubKey, -1, buffer, 2048 ) ] = 0;

  LONG r = RegOpenKeyExA( hKey, buffer, 0, saMask, pResult );

  StrFreeA( buffer );

  return r;
  }

LONG RegQueryValueU( HKEY hKey, LPCWSTR lpValueName, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData )
  {
  if ( IsWinNT() )
    {
    return RegQueryValueExW( hKey, lpValueName, NULL, lpType, lpData, lpcbData );
    }

  char * buffer = StrAllocA( 1024 );

  buffer[ UncToStr( CP_ACP, lpValueName, -1, buffer, 2048 ) ] = 0;

  DWORD Type;

  LONG r = RegQueryValueExA( hKey, buffer, NULL, &Type, lpData, lpcbData );

  StrFreeA( buffer );

  if ( r == ERROR_SUCCESS && lpData && ( Type == REG_SZ || Type == REG_EXPAND_SZ ) )
    {
    buffer = StrAllocA( *lpcbData );
    buffer[ StrToUnc( CP_ACP, (char*)lpData, -1, (LPWSTR)buffer, *lpcbData ) ] = 0;
    *wcsncpy( (LPWSTR)lpData, (LPWSTR)buffer, *lpcbData ) = 0;
    StrFreeA( buffer );
    }

  if ( lpType )
    {
    *lpType = Type;
    }

  return r;
  }

LONG RegEnumKeyU( HKEY hKey, DWORD dwIndex, LPWSTR lpName, DWORD cbName )
  {
  if ( IsWinNT() )
    {
    return RegEnumKeyW( hKey, dwIndex, lpName, cbName );
    }


  LPSTR buffer = StrAllocA( cbName );

  LONG r = RegEnumKeyA( hKey, dwIndex, buffer, cbName );

  lpName[ StrToUnc( CP_ACP, buffer, -1, lpName, cbName - 1 ) ] = 0;

  StrFreeA( buffer );

  return r;
  }

/////////////////////////////////////////////////////////////////////////////
//
HANDLE OpenEnumCodePages()
  {
  HKEY hKey;

  DWORD dwErr = RegOpenKeyU( HKEY_CLASSES_ROOT, L"Mime\\Database\\CodePage", KEY_READ, &hKey );

  return dwErr != ERROR_SUCCESS ? INVALID_HANDLE_VALUE : hKey;
  }

/////////////////////////////////////////////////////////////////////////////
//
void CloseEnumCodePages( HANDLE hEnum )
  {
  RegCloseKey( (HKEY)hEnum );
  }

/////////////////////////////////////////////////////////////////////////////
//
UINT EnumCodePages( HANDLE hEnum, const DWORD dwIndex )
  {
  WCHAR Value[ 1024 ];

  DWORD dwErr = RegEnumKeyU( (HKEY)hEnum, dwIndex, Value, 1024 );

  if ( dwErr == ERROR_SUCCESS )
    {
    return (UINT)wcstoul( Value, NULL, 0 );
    }

  return 0;
  }

/////////////////////////////////////////////////////////////////////////////
//
DWORD RegQueryDWORD( HKEY hKey, LPCWSTR ValueName, DWORD Default )
  {
  BYTE  szData[ 1024 ];
  DWORD dwSize = 1024;
  DWORD dwType;

  if ( RegQueryValueU( hKey, ValueName, &dwType, szData, &dwSize ) == ERROR_SUCCESS )
    {
    if ( dwType == REG_DWORD || dwType == REG_BINARY )
      {
      return (DWORD)*(LPDWORD*)szData;
      }
    if ( dwType == REG_SZ )
      {
      return (DWORD)wcstoul( (LPCWSTR)szData, NULL, 0 );
      }
    }
  return Default;
  }

/////////////////////////////////////////////////////////////////////////////
//
void RegQueryString( HKEY hKey, LPCWSTR ValueName, LPWSTR Value, const DWORD Size, LPCWSTR Default = L"" )
  {
  DWORD dwType, dwSize = 0;
  if ( RegQueryValueU( hKey, ValueName, &dwType, NULL, &dwSize ) == ERROR_SUCCESS )
    {
    if ( dwType == REG_SZ )
      {
      dwSize = Size * 2;
      if ( RegQueryValueU( hKey, ValueName, NULL, (LPBYTE)Value, &dwSize ) == ERROR_SUCCESS )
        {
        return;
        }
      }
    else if ( dwType == REG_DWORD )
      {
      dwSize = sizeof ( DWORD );
      if ( RegQueryValueU( hKey, ValueName, NULL, (LPBYTE)&dwType, &dwSize ) == ERROR_SUCCESS )
        {
        if ( _ultow( dwType, Value, 10 ) )
          {
          return;
          }
        }
      }

    }

  wcsncpy( Value, Default, Size );
  }

/////////////////////////////////////////////////////////////////////////////
//
UINT GetCodePageA( LPCSTR MimeName )
{
  WCHAR n[ MAX_MIMECSET_NAME ];
  StrToUnc( GetOEMCP(), MimeName, -1, n, MAX_MIMECSET_NAME );
  return GetCodePageW( n );
}

/////////////////////////////////////////////////////////////////////////////
//
UINT GetCodePageW( LPCWSTR MimeName )
{
  static WCHAR PrevMimeName[ MAX_MIMECSET_NAME ] = L"";
  static UINT PrevCP;
  if ( wcsicmp( MimeName, PrevMimeName ) == 0 )
    {
    return PrevCP;
    }

  HKEY hKey;

  DWORD dwErr;

  WCHAR ValueName[ 1024 ];
  swprintf( ValueName, L"Mime\\Database\\Charset\\%s", MimeName );

  dwErr = RegOpenKeyU( HKEY_CLASSES_ROOT, ValueName, KEY_READ, &hKey );

  if ( dwErr != ERROR_SUCCESS )
    {
    return 0;
    }

  WCHAR szData[ 1024 ];
  DWORD dwSize = 1024;

  dwErr = RegQueryValueU( hKey, L"AliasForCharset", NULL, (LPBYTE)szData, &dwSize );

  if ( dwErr == ERROR_SUCCESS )
    {
    RegCloseKey( hKey );
    swprintf( ValueName, L"Mime\\Database\\Charset\\%s", szData );
    dwErr = RegOpenKeyU( HKEY_CLASSES_ROOT, ValueName, KEY_READ, &hKey );
    if ( dwErr != ERROR_SUCCESS )
      {
      return 0;
      }
    }

  dwErr = RegQueryDWORD( hKey, L"InternetEncoding", 0 );

  RegCloseKey( hKey );

  wcsncpy( PrevMimeName, MimeName, MAX_MIMECSET_NAME );
  PrevCP = (UINT)dwErr;

  return PrevCP;
}

/////////////////////////////////////////////////////////////////////////////
//
BOOL GetMimeCPInfo( const UINT CodePage, TMimeCPInfo * pCPInfo )
{
  if ( pCPInfo == NULL )
    {
    return FALSE;
    }

  ZeroMemory( pCPInfo, sizeof( TMimeCPInfo ) );

  HKEY hKey;

  DWORD dwErr;

  WCHAR ValueName[ 1024 ];
  swprintf( ValueName, L"Mime\\Database\\CodePage\\%u", CodePage );

  dwErr = RegOpenKeyU( HKEY_CLASSES_ROOT, ValueName, KEY_READ, &hKey );

  if ( dwErr != ERROR_SUCCESS )
    {
    return FALSE;
    }

  pCPInfo->uiCodePage       = RegQueryDWORD( hKey, L"Encoding", 0 );
  pCPInfo->uiFamilyCodePage = RegQueryDWORD( hKey, L"Family", pCPInfo->uiCodePage );
  RegQueryString( hKey, L"Description", pCPInfo->wszDescription, MAX_MIMECP_NAME );
  RegQueryString( hKey, L"BodyCharset", pCPInfo->wszBodyCharset, MAX_MIMECSET_NAME );
  RegQueryString( hKey, L"HeaderCharset", pCPInfo->wszHeaderCharset, MAX_MIMECSET_NAME, pCPInfo->wszBodyCharset );
  RegQueryString( hKey, L"WebCharset", pCPInfo->wszWebCharset, MAX_MIMECSET_NAME, pCPInfo->wszBodyCharset );

  RegCloseKey( hKey );

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//
int UncToStr( const UINT CP, LPCWSTR Src, const int SrcLen, LPSTR Dst, const int DstLen )
  {
  return WideCharToMultiByte( CP, 0, Src, SrcLen, Dst, DstLen, NULL, NULL );
  }

/////////////////////////////////////////////////////////////////////////////
//
int StrToUnc( const UINT CP, LPCSTR Src, const int SrcLen, LPWSTR Dst, const int DstLen )
  {
  return MultiByteToWideChar( CP, 0, Src, SrcLen, Dst, DstLen );
  }

*/
