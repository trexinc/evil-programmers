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
#include "Decoder.h"
#include "FarPlugin.h"
#include "StrPtr.h"
#include <ctype.h>


//////////////////////////////////////////////////////////////////////
//
CMimeDecoder::CMimeDecoder()/* : m_Data( NULL ), m_DataCapacity( NULL )*/
{
}

//////////////////////////////////////////////////////////////////////
//
CMimeDecoder::~CMimeDecoder()
{
//  if ( m_Data )
//    delete m_Data;
}

//////////////////////////////////////////////////////////////////////
//
/*
void CMimeDecoder::SetDataSize( const DWORD newSize )
{
  if ( m_Data )
  {
    if ( newSize > m_DataCapacity )
    {
      delete [] m_Data;
      m_Data = create BYTE[ newSize ];
      m_DataCapacity = newSize;
    }
  }
  else
  {
    m_Data = create BYTE[ newSize ];
    m_DataCapacity = newSize;
  }
}

//////////////////////////////////////////////////////////////////////
//
DWORD CMimeDecoder::Decode( LPCSTR EncodedStr )
{
  far_assert( EncodedStr != NULL );

  DWORD Size = strlen( EncodedStr );

  SetDataSize( Size + 2 );

  memcpy( m_Data, EncodedStr, Size );

  m_Data[ Size++ ] = '\r';
  m_Data[ Size++ ] = '\n';

  return Size;
}
//////////////////////////////////////////////////////////////////////
//
DWORD CMimeDecoder::GetSize( const DWORD EncodedStr )
{
  return EncodedStr + 2;
}

//////////////////////////////////////////////////////////////////////
//
DWORD CMimeDecoder::GetSize( LPCSTR EncodedStr )
{
  far_assert( EncodedStr != NULL );
  return GetSize( strlen( EncodedStr ) );
}

//////////////////////////////////////////////////////////////////////
//
DWORD CMimeDecoder::GetSize( FarStringArray * EncodedStrings )
{
  far_assert( EncodedStrings != NULL );
  DWORD Size = 0;
  for ( int i = 0; i < EncodedStrings->Count(); i ++ )
  {
    DWORD Cur = GetSize( EncodedStrings->At( i ) );
    if ( Cur == (DWORD)-1 )
      break;
    Size += Cur;
  }
  return Size;
}

//////////////////////////////////////////////////////////////////////
//
void CMimeDecoder::DecodeToFile( FarStringArray * EncodedStrings, HANDLE hFile )
{
  far_assert( EncodedStrings != NULL );
  far_assert( hFile != INVALID_HANDLE_VALUE );
  for ( int i = 0; i < EncodedStrings->Count(); i ++ )
  {
    DWORD Size = Decode( EncodedStrings->At( i ) );
    if ( Size == (DWORD)-1 ) break;
    WriteFile( hFile, m_Data, Size, &Size, NULL );
  }
}

//////////////////////////////////////////////////////////////////////
//
void CQuotedPrintableDecoder::DecodeToFile( FarStringArray * EncodedStrings, HANDLE hFile )
{
  far_assert( EncodedStrings != NULL );
  far_assert( hFile != INVALID_HANDLE_VALUE );

  for ( int i = 0; i < EncodedStrings->Count(); i ++ )
  {
    DWORD Size = Decode( EncodedStrings->At( i ) );
    WriteFile( hFile, m_Data, Size, &Size, NULL );
    if ( bSoftCRLF == false )
      WriteFile( hFile, "\r\n", 2, &Size, NULL );
  }
}

//////////////////////////////////////////////////////////////////////
//
FarString CMimeDecoder::DecodeToString( LPCSTR EncodedStr )
{
  far_assert( EncodedStr != NULL );
  DWORD Size = Decode( EncodedStr );
  return FarString( (LPCSTR)m_Data, Size );
}
*/
//////////////////////////////////////////////////////////////////////
//
bool CMimeDecoder::isCoded( LPCSTR TransferEncoding )
{
  far_assert( TransferEncoding != NULL );
  return FarSF::LStricmp( TransferEncoding, "base64" ) == 0 ||
    FarSF::LStricmp( TransferEncoding, "quoted-printable" ) == 0;
}

//////////////////////////////////////////////////////////////////////
//
CMimeDecoder * CMimeDecoder::Create( LPCSTR TransferEncoding )
{
  far_assert( TransferEncoding != NULL );
  if ( FarSF::LStrnicmp( TransferEncoding, "base64", 6 ) == 0 )
    return create CBase64Decoder;
  if ( FarSF::LStrnicmp( TransferEncoding, "quoted-printable", 16 ) == 0 )
    return create CQuotedPrintableDecoder;
  if ( FarSF::LStrnicmp( TransferEncoding, "x-uue", 5 ) == 0 ||
    FarSF::LStrnicmp( TransferEncoding, "x-uuencode", 10 ) == 0 )
    return create CUUEDecoder;
  return create CMimeDecoder;
}

//////////////////////////////////////////////////////////////////////
//
#define QPIsHexOctetDigit(c) ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f' ))
DWORD CQuotedPrintableDecoder::getSize(PBYTE iData, DWORD dwSize)
{
  if (iData == NULL)
    return 0;

  PBYTE eData = iData + dwSize;

  dwSize = 0;

  while ( iData < eData )
  {
    if ( *iData == '=' )
      if ( *(iData+1) == '\r' || *(iData+1) == '\n' )
      {
        iData ++;
        if ( *iData == '\r' && *(++iData) == '\n' )
          iData ++;
        continue;
      }
      else if ( QPIsHexOctetDigit( *(iData+1) ) && QPIsHexOctetDigit( *(iData+2) ) )
      {
        iData += 2;
      }

    dwSize ++;

    iData ++;
  }

  return dwSize;
}

//////////////////////////////////////////////////////////////////////
//
#define QPToHexDigit(c) ( ( ( c >= '0' && c <= '9' ) ? c - '0' : c - 'A' + 10 ) & 0x0F )
DWORD CQuotedPrintableDecoder::decode(PBYTE iData, DWORD dwSize)
{
  if ( iData == NULL )
    return 0;

  PBYTE oData = iData;
  PBYTE eData = iData + dwSize;

  char c;
  DWORD Size = 0;

  //bSoftCRLF = false;

  while ( iData < eData )
  {
    if ( *iData == '=' )
    {
      iData ++;

      if ( *iData == '\r' || *iData == '\n' )
      {
        if ( *iData == '\n' || (*iData == '\r' && *(++iData) == '\n') )
          iData ++;
        continue;
      }
      else if ( QPIsHexOctetDigit( *iData ) && QPIsHexOctetDigit( *(iData+1) ) )
      {
        c = QPToHexDigit( *iData ) << 4;
        iData ++;
        c |= QPToHexDigit( *iData );
      }
      else
      {
        c = *iData;
      }
    }
    else
    {
      c = *iData;
    }

    oData[ Size++ ] = c;

    iData ++;
  }

  return Size;
}

DWORD CQuotedPrintableSubjectDecoder::decode(PBYTE iData, DWORD dwSize)
{
  if ( iData == NULL )
    return 0;

  PBYTE oData = iData;
  PBYTE eData = iData + dwSize;

  char c;
  DWORD Size = 0;

  //bSoftCRLF = false;

  while ( iData < eData )
  {
    if ( *iData == '=' )
    {
      iData ++;

      if ( *iData == '\r' || *iData == '\n' )
      {
        if ( *iData == '\n' || (*iData == '\r' && *(++iData) == '\n') )
          iData ++;
        continue;
      }
      else if ( QPIsHexOctetDigit( *iData ) && QPIsHexOctetDigit( *(iData+1) ) )
      {
        c = QPToHexDigit( *iData ) << 4;
        iData ++;
        c |= QPToHexDigit( *iData );
      }
      else
      {
        c = *iData;
      }
    }
    else if ( *iData == '_' )
    {
      c = '\x20';
    }
    else
    {
      c = *iData;
    }

    oData[ Size++ ] = c;

    iData ++;
  }

  return Size;
}

//////////////////////////////////////////////////////////////////////
//
const int CBase64Decoder::m_Index64[ 128 ] =
{
  -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
  -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
  -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
  52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
  -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
  15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
  -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
  41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
};

CBase64Decoder::CBase64Decoder() : CMimeDecoder(), m_TmpData( NULL ), m_TmpSize( 0 )
{
}

//////////////////////////////////////////////////////////////////////
//
CBase64Decoder::~CBase64Decoder()
{
  if ( m_TmpData )
    delete m_TmpData;
}

//////////////////////////////////////////////////////////////////////
//
DWORD CBase64Decoder::getSize(PBYTE iData, DWORD dwSize)
{
  if ( iData == NULL )
    return 0;

  if ( dwSize <= 0 )
    return -1;

  PBYTE eData = iData + dwSize;

  while ( iData < eData && isspace( (unsigned char)*iData ) )
    iData ++;

  dwSize = 0;

  register BYTE d1, d2, d3, d4;

  do
  {
    if ( *iData == '\r' )
      iData ++;
    if ( *iData == '\n' )
      iData ++;

    if ( iData >= eData || *iData == '\r' || *iData == '\n' )
      break;

    d1 = iData[ 0 ];
    if ( d1 > 127 || m_Index64[ d1 ] == -1 )
      return -1;

    d2 = iData[ 1 ];
    if ( d2 > 127 || m_Index64[ d2 ] == -1 )
      return -1;

    d3 = iData[ 2 ];
    if ( d3 != '\0' )
    {
      if ( d3 > 127 || ( ( d3 != '=' ) && ( m_Index64[ d3 ] == -1 ) ) )
        return -1;

      d4 = iData[ 3 ];
      if ( d4 != '\0' )
      {
        if ( d4 > 127 || ( ( d4 != '=' ) && ( m_Index64[ d4 ] == -1 ) ) )
          return -1;
      }
      else
      {
        d4 = '=';
      }
    }
    else
    {
      d3 = d4 = '=';
    }

    iData += 4;

    dwSize++;
    if ( d3 != '=' )
    {
      dwSize++;
      if ( d4 != '=' )
      {
        dwSize++;
      }
    }
  } while ( d4 != '=' );

  return dwSize;

//  return dwSize / 4 * 3 + ( dwSize - dwSize / 4 * 4 );
}

//////////////////////////////////////////////////////////////////////
//
DWORD CBase64Decoder::decode(PBYTE iData, DWORD dwSize)
{
  if ( iData == NULL )
    return 0;
  //SetDataSize( dwSize );

  if ( dwSize <= 0 )
    return -1;

  PBYTE oData = iData;
  PBYTE eData = iData + dwSize;

  while ( iData < eData && isspace( (unsigned char)*iData ) )
    iData ++;

  dwSize = 0;

  register BYTE d1, d2, d3, d4;

  do
  {
    if ( *iData == '\r' )
      iData ++;
    if ( *iData == '\n' )
      iData ++;

    if ( iData >= eData || *iData == '\r' || *iData == '\n' )
      break;

    d1 = iData[ 0 ];
    if ( d1 > 127 || m_Index64[ d1 ] == -1 )
      return -1;

    d2 = iData[ 1 ];
    if ( d2 > 127 || m_Index64[ d2 ] == -1 )
      return -1;

    d3 = iData[ 2 ];
    if ( d3 != '\0' )
    {
      if ( d3 > 127 || ( ( d3 != '=' ) && ( m_Index64[ d3 ] == -1 ) ) )
        return -1;

      d4 = iData[ 3 ];
      if ( d4 != '\0' )
      {
        if ( d4 > 127 || ( ( d4 != '=' ) && ( m_Index64[ d4 ] == -1 ) ) )
          return -1;
      }
      else
      {
        d4 = '=';
      }
    }
    else
    {
      d3 = d4 = '=';
    }

    iData += 4;

    /* digits are already sanity-checked */
    *oData++ = ( m_Index64[ d1 ] << 2 )|(( m_Index64[ d2 ] >> 4 ) & 0x03 );
    dwSize++;
    if ( d3 != '=' )
    {
      *oData++ = ( ( m_Index64[ d2 ] << 4 ) & 0xF0 )|( m_Index64[ d3 ] >> 2 );
      dwSize++;
      if ( d4 != '=' )
      {
        *oData++ = ( ( m_Index64[ d3 ] << 6 ) & 0xC0 )|m_Index64[ d4 ];
        dwSize++;
      }
    }
  } while ( /* *iData &&*/ d4 != '=' );

  return dwSize;
}
/*
void CUUEDecoder::DecodeToFile( FarStringArray * EncodedStrings, HANDLE hFile )
{
  far_assert( EncodedStrings != NULL );
  far_assert( hFile != INVALID_HANDLE_VALUE );
  for ( int i = 0; i < EncodedStrings->Count(); i ++ )
  {
    LPCSTR str = EncodedStrings->At( i );
    if ( *str == '\0' || FarSF::LStrnicmp( str, "begin ", 6 ) == 0 )
      continue;

    if ( FarSF::LStrnicmp( str, "end", 3 ) == 0 )
      break;

    DWORD Size = Decode( str );

    if ( Size == (DWORD)-1 )
      break;

    WriteFile( hFile, m_Data, Size, &Size, NULL );
  }
}
*/
#define UUDECODE( c ) ( c=='`' ? 0 : c - '\x20' )
#define N64( i ) ( i & ~63 )

const char uudigit[ 64 ] =
{
  '`', '!', '"', '#', '$', '%', '&','\'',
  '(', ')', '*', '+', ',', '-', '.', '/',
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', ':', ';', '<', '=', '>', '?',
  '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
  'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
  'X', 'Y', 'Z', '[','\\', ']', '^', '_'
};

DWORD fromuutobits( PBYTE out, LPCSTR in/*, DWORD dwSize*/ )
{
/*  if ( dwSize == 0 )
    return -1;
*/
  int outlen = UUDECODE( in[ 0 ] );
  in += 1;
  if ( outlen < 0 || outlen > 45 )
    return /*-2*/-1;

  if ( outlen == 0 )
    return 0;

  int inlen = ( outlen * 4 + 2 ) / 3;

  /*if ( inlen > (int)dwSize )
    inlen = dwSize - dwSize % 3;*/

  register BYTE digit1;
  register BYTE digit2;

  int len;
  for ( len = 0; inlen > 0; inlen -= 4 )
  {
    digit1 = UUDECODE( in[ 0 ] );
    if ( N64( digit1 ) )
      return -1;

    digit2 = UUDECODE( in[ 1 ] );
    if ( N64( digit2 ) )
      return -1;

    out[ len++ ] = ( digit1 << 2 ) | ( digit2 >> 4 );

    if ( inlen > 2 )
    {
      digit1 = UUDECODE( in[2] );
      if ( N64( digit1 ) )
        return -1;

      out[ len++ ] = ( digit2 << 4 ) | ( digit1 >> 2 );
      if ( inlen > 3 )
      {
        digit2 = UUDECODE( in[ 3 ] );
        if ( N64( digit2 ) )
          return -1;

        out[ len++ ] = ( digit1 << 6 ) | digit2;
      }
    }
    in += 4;
  }

  return len == outlen ? len : -1;
}

DWORD CUUEDecoder::decode(PBYTE Src, DWORD dwSize)
{
  if (Src == NULL)
    return 0;

  PBYTE Dst = Src;

  PBYTE Lst = Src + dwSize;

  dwSize = 0;

  while ( Src < Lst )
  {
    while ( Src < Lst && isspace( (unsigned char)*Src ) )
      Src ++;

    if ( Src >= Lst || *Src == '`' || FarSF::LStrnicmp( (LPSTR)Src, "end", 3 ) == 0 )
      break;

    if ( FarSF::LStrnicmp( (LPSTR)Src, "begin ", 6 ) == 0 )
    {
      Src += 6;
      while ( Src < Lst && *Src != '\r' && *Src != '\n' )
        Src ++;
      continue;
    }

    int Len = fromuutobits( Dst, (LPSTR)Src );

    if ( Len <= 0 )
      break;

    dwSize += Len;
    Dst += Len;
    Src += Len;
    while ( Src < Lst && *Src != '\r' && *Src != '\n' )
      Src ++;
  }

  return dwSize;
}

DWORD CUUEDecoder::getSize(PBYTE Data, DWORD dwSize)
{
  if (Data == NULL)
    return 0;

  PBYTE Last = Data + dwSize;

  StrPtr sp = { NULL, 0, (LPSTR)Data };

  dwSize = 0;

  while ( sp.Nxt )
  {
    GetStrPtr( (LPSTR)sp.Nxt, &sp, (LPSTR)Last );

    if ( sp.Len == 0 )
      break;

    if ( FarSF::LStrnicmp( (LPSTR)sp.Str, "begin ", 6 ) == 0 )
      continue;

    DWORD Size = UUDECODE( *sp.Str );

    if ( Size <= 0 || Size > 45 )
      break;

    dwSize += Size;
  }

  return dwSize;
}
