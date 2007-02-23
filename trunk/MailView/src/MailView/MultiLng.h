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
#ifndef ___MultiLng_H___
#define ___MultiLng_H___

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/*
#ifndef MAX_MIMECP_NAME
#define MAX_MIMECP_NAME ( 64 )
#endif
#ifndef MAX_MIMECSET_NAME
#define MAX_MIMECSET_NAME ( 50 )
#endif

struct TMimeCPInfo
    {
  UINT  uiCodePage;
  UINT  uiFamilyCodePage;
  WCHAR wszDescription[ MAX_MIMECP_NAME ];
  WCHAR wszWebCharset[ MAX_MIMECSET_NAME ];
  WCHAR wszHeaderCharset[ MAX_MIMECSET_NAME ];
  WCHAR wszBodyCharset[ MAX_MIMECSET_NAME ];
    };


HANDLE OpenEnumCodePages();
void CloseEnumCodePages( HANDLE hEnum );
UINT EnumCodePages( HANDLE hEnum, const DWORD dwIndex );

UINT GetCodePageA( LPCSTR MimeName );
UINT GetCodePageW( LPCWSTR MimeName );
BOOL GetMimeCPInfo( const UINT CodePage, TMimeCPInfo * pCPInfo );

//LPWSTR StrToUnc( LPCSTR Src, UINT Encoding );
//LPSTR UncToStr( LPCWSTR Src, UINT Encoding );

int UncToStr( const UINT CP, LPCWSTR Src, const int SrcLen, LPSTR Dst, const int DstLen );
int StrToUnc( const UINT CP, LPCSTR Src, const int SrcLen, LPWSTR Dst, const int DstLen );
*/

#endif // !defined(___MultiLng_H___)
