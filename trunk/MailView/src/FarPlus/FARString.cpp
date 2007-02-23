/* $Header: /cvsroot/farplus/FARPlus/FARString.cpp,v 1.8 2002/09/03 06:33:01 yole Exp $
   FAR+Plus: lightweight string class implementation
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
   Portions copyright (C) 2002 Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>
   Portions copyright (C) 2005 Alex Yaroslavsky
*/

#include "FARString.h"
#include "FARPlus.h"
#include <windows.h>

// -- FarStringData ----------------------------------------------------------

FarString::FarStringData *FarString::fEmptyStringData = NULL;

FarString::FarStringData::FarStringData (const char *text, int length /*= -1*/)
  : fText (NULL), fCapacity (0), fRefCount (1)
{

  if (text)
  {
    if (length == -1)
      fLength = strlen (text);
    else
      fLength = length;
  }
  else
  {
    if (length == -1)
      fLength = 0;
    else
      fLength = length;
  }
  SetCapacity (fLength + 1);
  if (text)
  {
    far_assert_string( text, fLength );
    memcpy (fText, text, fLength );
    fText[ fLength ] = '\0';
  }
  else
  {
    *fText = '\0';
    fLength = 0;
  }
}

void FarString::FarStringData::SetCapacity (size_t capacity)
{
  const int memDelta = 16;

  if (capacity < fCapacity)
    return;

  size_t newCapacity = capacity + memDelta;
  char *newText = create char [newCapacity];

  if (fText)
  {
    memcpy (newText, fText, fCapacity/*fLength + 1*/);
    delete [] fText;
    fText = newText;
  }
  else
  {
    fText   = newText;
    *fText  = '\0';
  }

  fCapacity = newCapacity;
}

// -- FarString --------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4018)
#endif
const FarString &FarString::Append (const char *s, int addLen)
{
  if (s != NULL && *s != '\0' && addLen != 0)
  {
    if (addLen == -1)
      addLen = strlen (s);
    int newLength = fData->fLength + addLen;
    if (IsUnique() && fData->fCapacity > newLength)
    {
      strncpy (fData->fText + fData->fLength, s, addLen);
      fData->SetLength (newLength);
    }
    else
    {   // cp-1251
      // Den to yole
      // здесь только выделяем память, иначе можем
      // поломаться при memcpy в конструкторе
      FarStringData * newData = create FarStringData( NULL, newLength );
      memcpy( newData->fText, fData->fText, fData->fLength );
      memcpy( newData->fText + fData->fLength, s, addLen );
            newData->fText [newLength] = '\0';
      newData->fLength = newLength;
      fData->DecRef();
      fData = newData;
    }
  }
  return *this;
}
#ifdef _MSC_VER
#pragma warning(default:4018)
#endif

int FarString::Insert (int nIndex, const char * Str, size_t nLength)
{
  far_assert (nIndex >= 0);

  if (nLength > 0)
  {
    int newLength = fData->fLength;
    UniqueString();

    if (nIndex > newLength)
      nIndex = newLength;

    newLength += nLength;

    fData->SetCapacity (newLength);

    memmove (fData->fText + nIndex + nLength,
      fData->fText + nIndex,
      newLength - nIndex - nLength + 1 );

    memmove (fData->fText + nIndex, Str, nLength);

    fData->fLength = newLength;
  }

  return nLength;
}

int FarString::Delete (int nIndex, int nCount /* = 1 */)
{
  far_assert (nIndex >= 0);

  int nNewLength = fData->fLength;
  if (nIndex + nCount > nNewLength)
    nCount = nNewLength - nIndex;

  if (nCount > 0 && nIndex < nNewLength)
  {
    UniqueString();

    int nBytesToCopy = nNewLength - (nIndex + nCount) + 1;
    memmove (fData->fText + nIndex, fData->fText + nIndex + nCount, nBytesToCopy);

    fData->fLength = nNewLength - nCount;
  }

  return fData->fLength;
}
#ifdef _MSC_VER
#pragma warning(disable:4018)
#endif
FarString FarString::Mid (int nFirst, int nCount) const
{
  if (nFirst < 0)
    nFirst = 0;

  if (nCount < 0)
    nCount = 0;

  if (nFirst + nCount > fData->fLength)
  {
    nCount = fData->fLength - nFirst;
  }
  if (nFirst > fData->fLength)
  {
    nCount = 0;
  }

  far_assert (nFirst >= 0 );
  far_assert (nFirst + nCount <= fData->fLength);

  if (nFirst == 0 && nFirst + nCount == fData->fLength)
    return *this;

  return FarString (fData->fText + nFirst, nCount);
}
#ifdef _MSC_VER
#pragma warning(default:4018)
#endif

FarString FarString::ToOEM() const
{
  FarString result (fData->fText, Length());
  CharToOemBuff (fData->fText, result.GetBuffer(), Length());
  return result;
}

FarString FarString::ToANSI() const
{
  FarString result (fData->fText, Length());
  OemToCharBuff (fData->fText, result.GetBuffer(), Length());
  return result;
}

FarString& FarString::MakeUpper()
{
  UniqueString();
  CharUpperBuff (fData->fText, fData->fLength);
  return * this;
}

FarString& FarString::MakeLower()
{
  UniqueString();
  CharLowerBuff (fData->fText, fData->fLength);
  return * this;
}

FarString& FarString::Trim()
{
  FarSF::Trim (GetBuffer());
  ReleaseBuffer();
  return * this;
}

FarString& FarString::TrimLeft()
{
  FarSF::LTrim (GetBuffer());
  ReleaseBuffer();
  return * this;
}

FarString& FarString::TrimRight()
{
  FarSF::RTrim (GetBuffer());
  ReleaseBuffer();
  return * this;
}

FarString& FarString::Unquote()
{
  FarSF::Unquote( GetBuffer() );
  ReleaseBuffer();
  return * this;
}

int FarString::CompareNoCase (const char *Str) const
{
  return 2-CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE, fData->fText, fData->fLength,
    Str, -1);
}

int FarString::CompareNoCase (const char * Str, size_t nLength) const
{
  if (nLength > fData->fLength+1)
    nLength = fData->fLength+1;
  return 2-CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE, fData->fText, nLength,
    Str, nLength);
}

FarString& FarString::FormatV( const char * Fmt, va_list argList )
{
  int newLength = FarSF::vsnprintf( NULL, 0, Fmt, argList );
  FarStringData * newData = create FarStringData( NULL, newLength );
  newData->fLength = FarSF::vsnprintf( newData->fText, newLength, Fmt, argList );
  newData->fText[ newData->fLength ] = '\0';

  fData->DecRef();
  fData = newData;

  return * this;
}

FarString& FarString::Format( const char * Fmt, ... )
{
  far_assert( Fmt != NULL );

  va_list argList;
  va_start( argList, Fmt );

  FormatV( Fmt, argList );

  va_end( argList );

  return * this;
}

FarString& FarString::Format( int nFmtLangId, ... )
{
  va_list argList;
  va_start( argList, nFmtLangId );

  FormatV( Far::GetMsg( nFmtLangId ), argList );

  va_end( argList );

  return * this;
}

FarString& FarString::Expand()
{
  int newLength = ExpandEnvironmentStrings( fData->fText, NULL, 0 );
  FarStringData * newData = create FarStringData( NULL, newLength );

  newData->fLength = FarSF::m_FSF.ExpandEnvironmentStr( fData->fText, newData->fText, newLength );
  newData->fText[ newData->fLength ] = '\0';

  fData->DecRef();
  fData = newData;
  return * this;
}
// -- FarStringTokenizer -----------------------------------------------------

void FarStringTokenizer::Attach (const char *text, char separator, bool ignoreWhitespace)
{
  fText = text;
  fSeparator = separator;
  fCurIndex = 0;
  fIgnoreWhitespace = ignoreWhitespace;
  fCurPos = GetTokenStart (fText);
}

const char *FarStringTokenizer::GetTokenEnd (const char *tokenStart) const
{
  if (*tokenStart == '\"')
  {
    const char *closeQuote = strchr (tokenStart+1, '\"');
    if (!closeQuote)
      return tokenStart + strlen (tokenStart);
    return closeQuote+1;
  }
  else
  {
    const char *separator = strchr (tokenStart+1, fSeparator);
    if (!separator)
      return tokenStart + strlen (tokenStart);
    return separator;
  }
}

const char *FarStringTokenizer::GetTokenStart (const char *tokenEnd) const
{
  while (*tokenEnd == fSeparator || (fIgnoreWhitespace && (*tokenEnd == ' ' || *tokenEnd == '\t')))
    tokenEnd++;
  return tokenEnd;
}

FarString FarStringTokenizer::NextToken()
{
  // it is not allowed to call NextToken() for a non-attached tokenizer
  far_assert (fText != NULL);
  const char *tokenEnd = GetTokenEnd (fCurPos);
  FarString token (fCurPos, tokenEnd-fCurPos);
  fCurPos = GetTokenStart (tokenEnd);
  fCurIndex++;
  return token;
}

FarString FarStringTokenizer::GetToken (int index) const
{
  // it is not allowed to call GetToken() for a non-attached tokenizer
  far_assert (fText != NULL);
  far_assert (index >= 0);
  const char *curPos = GetTokenStart (fText);
  for (int i=0; i<index; i++)
  {
    curPos = GetTokenEnd (curPos);
    curPos = GetTokenStart (curPos);
    if (*curPos == '\0')
      return FarString();
  }
  const char *tokenEnd = GetTokenEnd (curPos);

  FarString result (curPos, tokenEnd-curPos);
  if (fIgnoreWhitespace)
    result.TrimRight();
  return result;
}

// -- FarFileName ------------------------------------------------------------

FarFileName& FarFileName::AddEndSlash()
{
  if (IsEmpty() || c_str() [Length()-1] != '\\')
        Insert (Length(), "\\");
  return * this;
}

FarFileName& FarFileName::QuoteSpaceOnly()
{
  if (IndexOf (' ') != -1)
  {
    SetLength (Length()+2);
    FarSF::QuoteSpaceOnly (GetBuffer());
    ReleaseBuffer();
  }
  return * this;
}

FarFileName FarFileName::GetName() const
{
  return FarFileName (FarSF::PointToName (c_str()));
}

FarFileName FarFileName::GetExt() const
{
  int p = LastIndexOf ('.');
  if (p == -1)
    return FarFileName (".");
  return FarFileName (Mid (p));
}

FarFileName FarFileName::GetPath() const
{
  const char *text = c_str();
  char *pName = FarSF::PointToName (text);
  return FarFileName (text, pName - text);
}

FarFileName FarFileName::GetDrive() const
{
  LPCSTR text = c_str();
  if ( Length() >= 2 && text[ 1 ] == ':' )
  {
    return FarFileName( text, 2 );
  }

  if ( Length() >= 2 && text[ 0 ] == '\\' && text[ 1 ] == '\\')
  {
    int i = 0, j = 0;

    while ( i < Length() && i < 2 )
    {
      if ( text[ i ] == '\\' )
        j ++;

      if ( j < 2 )
        i ++;
    }

    if ( text[ i ] == '\\' )
      i --;

    return FarFileName( text, i );
  }

  return FarFileName();
}

void FarFileName::SetPath (const FarFileName& path)
{
  *this = FarFileName::MakeName (path.GetPath(), GetName());
}

void FarFileName::SetName (const FarFileName& name)
{
  *this = FarFileName::MakeName (GetPath(), name.GetName());
}

void FarFileName::SetExt (const FarFileName& ext)
{
  FarFileName ffn = ext.GetExt();

  int nPos = LastIndexOf( '.' );
  if ( nPos != -1 )
    Delete( nPos, Length() - nPos );

  operator+=( ffn );
}

FarFileName FarFileName::GetFullName() const
{
  char buf [260];
  char *pName;
  DWORD dwSize = ::GetFullPathName (c_str(), sizeof (buf)-1, buf, &pName);
  if (dwSize <= sizeof (buf)-1)
    return FarFileName (buf);

  FarFileName result;
  result.SetLength (dwSize);
  ::GetFullPathName (c_str(), dwSize, result.GetBuffer(), &pName);
  return result;
}

FarFileName FarFileName::GetShortName() const
{
  char shortName [260];
  FarSF::ConvertNameToShort (c_str(), shortName);
  return shortName;
}

FarFileName FarFileName::MakeName (const FarString &path, const FarString &name)
{
  FarFileName fullName = path;
  fullName.AddEndSlash();
  fullName += name;
  return fullName;
}

FarFileName FarFileName::MakeTemp( const char * Prefix )
{
  FarFileName tmp;
  FarSF::MkTemp( tmp.GetBuffer( GetTempPath( 0, NULL ) + 12 ), Prefix );
  tmp.ReleaseBuffer();
  return tmp;
}

FarString IntToString( int Value, int Radix )
{
  char buf[ 48 ];
  return FarSF::itoa( Value, buf, Radix );
}
