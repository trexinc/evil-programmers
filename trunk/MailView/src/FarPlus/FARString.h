/* $Header: /cvsroot/farplus/FARPlus/FARString.h,v 1.8 2002/09/03 06:33:01 yole Exp $
   FAR+Plus: A lightweight string class to be used in FAR plugins.
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
   Portions copyright (C) 2002 Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>
*/

#ifndef __FARSTRING_H
#define __FARSTRING_H

#include <string.h>
#include "FARMemory.h"
#include "FARDbg.h"
#include <stdarg.h>

#if _MSC_VER >= 1000
#pragma once
#endif

class FarString
{
private:
  class FarStringData
  {
    friend class FarString;

    char *fText;
    size_t fLength;
    size_t fCapacity;
    int fRefCount;

    FarStringData (const char *text, int length = -1);

    ~FarStringData()
    {
      delete [] fText;
    }

    void AddRef()
    {
      fRefCount++;
    }

    void DecRef()
    {
      fRefCount--;
      if (!fRefCount)
        delete this;
    }

    void SetCapacity (size_t capacity);
    void SetLength (size_t newLength)
    {
      //far_assert (newLength < fCapacity);
      SetCapacity( newLength );
      fLength = newLength;
            fText [fLength] = '\0';
    }
  };


  FarStringData *fData;
  static FarStringData *fEmptyStringData;

  bool IsUnique() const
  {
    return fData->fRefCount == 1;
  }

  void UniqueString()
  {
    if (fData->fRefCount > 1)
    {
      FarStringData *newData = create FarStringData (fData->fText, fData->fLength);
      fData->DecRef();
      fData = newData;
    }
  }

  FarStringData *GetEmptyStringData()
  {
    if (fEmptyStringData == NULL)
      fEmptyStringData = create FarStringData ("");
    fEmptyStringData->AddRef();
    return fEmptyStringData;
  }

  FarStringData *GetStringData (const char *text, size_t length = -1)
  {
    if ((text == NULL || *text == '\0') && length == -1)
      return GetEmptyStringData();
    if (length == 0)
      return GetEmptyStringData();
    return create FarStringData (text, length);
  }

  const FarString &Append (const char *text, int addLen);

public:
  FarString()
  {
    fData = GetEmptyStringData();
  }

  FarString (const char *text)
  {
    fData = GetStringData (text);
  }

  FarString (const char *text, int length)
  {
    fData = GetStringData (text, length);
  }

  FarString (const FarString &str)
    : fData (str.fData)
  {
    fData->AddRef();
  }

  FarString (char c, int nCount)
  {
    fData = GetStringData (NULL, nCount);
    memset( fData->fText, c, nCount );
    fData->fText[ nCount ] = '\0';
    fData->fLength = nCount;
  }

  ~FarString()
  {
    fData->DecRef();
  }

  operator const char*() const
  {
    return fData->fText;
  }

  const char *c_str() const
  {
    return fData->fText;
  }

  const char *data() const
  {
    return fData->fLength == 0 ? NULL : fData->fText;
  }

  char *GetBuffer (int nLength = -1)
  {
    UniqueString();
    if (nLength > 0)
      fData->SetCapacity (nLength);
    return fData->fText;
  }

  void ReleaseBuffer (int newLength = -1)
  {
    UniqueString();

    if (newLength == -1)
      newLength = (fData->fText) ? strlen (fData->fText) : 0;

    fData->SetLength (newLength);
  }

  char operator[] (int index) const
  {
    return fData->fText [index];
  }

  char& operator[] (int index)
  {
    UniqueString();

    return fData->fText [index];
  }

  const FarString &operator= (char ch)
  {
    SetLength (1);
    fData->fText [0] = ch;
    return *this;
  }

  const FarString &operator= (const FarString &rhs)
  {
    if (&rhs != this)
    {
      rhs.fData->AddRef();
      fData->DecRef();
      fData = rhs.fData;
    }
    return *this;
  }

  const FarString &operator= (const char *text)
  {
    if (text != fData->fText)
    {
      FarStringData *newData = GetStringData (text);
      fData->DecRef();
      fData = newData;
    }
    return *this;
  }

  bool operator== (const FarString &rhs) const
  {
    if (fData == rhs.fData)
      return true;
    if (fData->fLength != rhs.fData->fLength)
      return false;
    return memcmp (fData->fText, rhs.fData->fText, fData->fLength) == 0;
  }

  int Compare(const char *Str) const
  {
    if (Str == NULL)
      return 1;   // we're greater than an empty string

    return memcmp (fData->fText, Str, fData->fLength+1);
  }

  int Compare (const char *Str, size_t nLength) const
  {
    if (Str == NULL)
      return 1;   // we're greater than an empty string

    return memcmp (fData->fText, Str, nLength);
  }

  int Compare( const FarString& str ) const
  {
    return memcmp( fData->fText, str.fData->fText, str.fData->fLength );
  }

  int CompareNoCase (const char *Str) const;
  int CompareNoCase (const char *Str, size_t nLength) const;

  void SetText (const char *text, int length)
  {
    fData->DecRef();
    fData = GetStringData (text, length);
  }

  int Length() const
  {
    return fData->fLength;
  }

  bool IsEmpty() const
  {
    return Length() == 0;
  }

  void SetLength (size_t newLength)
  {
    if (newLength != fData->fLength)
    {
      if (newLength < fData->fCapacity && IsUnique())
        fData->SetLength (newLength);
      else
      {
        FarStringData *newData = GetStringData (fData->fText, newLength);
        fData->DecRef();
        fData = newData;
      }
    }
  }

  void Empty()
  {
    SetLength (0);
  }

  const FarString &operator += (const char *s)
  {
    return Append (s, -1);
  }

  const FarString &operator += (const FarString &str)
  {
    return Append (str.fData->fText, str.fData->fLength);
  }

  const FarString &operator += (char c)
  {
    return Append (&c, 1);
  }

  int Insert (int nIndex, const char *Str, size_t nLength);

  int Insert (int nIndex, const char *Str)
  {
    if (Str && *Str)
      return Insert (nIndex, Str, strlen (Str));
    return 0;
  }

  int Insert (int nIndex, const FarString &Str)
  {
    return Insert (nIndex, Str.fData->fText, Str.fData->fLength);
  }

  int Delete (int nIndex, int nCount = 1);

  FarString Mid (int nFirst, int nCount) const;
  FarString Mid (int nFirst) const
  {
    return Mid (nFirst, fData->fLength - nFirst);
  }

  FarString Left(int nCount) const
  {
    return Mid (0, nCount);
  }

  FarString Right (size_t nCount) const
  {
    if (nCount > fData->fLength)
      nCount = fData->fLength;
    return Mid (fData->fLength - nCount, nCount);
  }

  int IndexOf (char c, int startChar = 0) const
  {
    for (unsigned int i=startChar; i < fData->fLength; i++)
      if (fData->fText [i] == c)
        return i;
    return -1;
  }

  int LastIndexOf (char c) const
  {
    for (int i=fData->fLength; i >= 0; i--)
      if (fData->fText [i] == c)
        return i;
    return -1;
  }

  FarString ToOEM() const;
  FarString ToANSI() const;

  FarString& MakeUpper();
  FarString& MakeLower();

  FarString& Trim();
  FarString& TrimLeft();
  FarString& TrimRight();

  FarString& Unquote();

  static void FreeEmptyString()
  {
    if (fEmptyStringData)
    {
      int oldRefCount = fEmptyStringData->fRefCount;
      fEmptyStringData->DecRef();
      if (oldRefCount == 1)
         fEmptyStringData = NULL;
    }
  }

  FarString& Format( const char * Fmt, ... );
  FarString& Format( int nFmtLangId, ... );
  FarString& FormatV( const char * Fmt, va_list argList );

  // заменяет в строке ссылки на переменные окружения на их значения.
  FarString& Expand();
};

// -- FarStringTokenizer -----------------------------------------------------

class FarStringTokenizer
{
private:
  const char *fText;
  char fSeparator;
  const char *fCurPos;
  int fCurIndex;
  bool fIgnoreWhitespace;

  const char *GetTokenEnd (const char *tokenStart) const;
  const char *GetTokenStart (const char *tokenEnd) const;

  FarStringTokenizer (const FarStringTokenizer &rhs); // not implemented
  void operator= (const FarStringTokenizer &rhs);     // not implemented

public:
  FarStringTokenizer()
    : fText (NULL), fSeparator (','), fCurPos (NULL), fCurIndex (-1), fIgnoreWhitespace (true)
  {
  }

  FarStringTokenizer (const char *text, char separator = ',', bool ignoreWhitespace = true)
    : fText (NULL)
  {
    Attach (text, separator, ignoreWhitespace);
  }

  void Attach (const char *text, char separator = ',', bool ignoreWhitespace = true);
  bool HasNext() const
  {
    if (fText == NULL) return false;
    return *fCurPos != '\0';
  }

  int GetCurIndex() const
    { return fCurIndex; }
  FarString NextToken();
  FarString GetToken (int index) const;
};

// -- FarFileName ------------------------------------------------------------

class FarFileName: public FarString
{
public:
  FarFileName()
    : FarString() {};

  FarFileName (const char *text)
    : FarString (text) {};

  FarFileName (const char *text, int length)
    : FarString (text, length) {};

  FarFileName (const FarString &str)
    : FarString (str) {};

  FarFileName& AddEndSlash();
  FarFileName& QuoteSpaceOnly();

  FarFileName GetName() const;
  FarFileName GetExt() const;
  FarFileName GetPath() const;
  FarFileName GetDrive() const;
  FarFileName GetFullName() const;
  FarFileName GetShortName() const;

  void SetPath (const FarFileName& path);
  void SetName (const FarFileName& name);
  void SetExt (const FarFileName& ext);

  static FarFileName MakeName (const FarString &path, const FarString &name);
  static FarFileName MakeTemp( const char * Prefix );
};

// -- FarString implementation -----------------------------------------------

inline FarString operator+ (const FarString &lhs, char c)
{
  FarString result = lhs;
  result += c;
  return result;
}

inline FarString operator+ (char c, const FarString &rhs)
{
  FarString result (&c, 1);
  result += rhs;
  return result;
}

inline FarString operator+ (const FarString &lhs, const FarString &rhs)
{
  FarString result = lhs;
  result += rhs;
  return result;
}

inline FarString operator+ (const char *lhs, const FarString &rhs)
{
  FarString result = lhs;
  result += rhs;
  return result;
}

inline FarString operator+ (const FarString &lhs, const char *rhs)
{
  FarString result = lhs;
  result += rhs;
  return result;
}

inline bool operator==( const FarString& s1, const char * s2 ) { return s1.Compare( s2 ) == 0; }
inline bool operator==( const char * s1, const FarString& s2 ) { return s2.Compare( s1 ) == 0; }
inline bool operator!=( const FarString& s1, const FarString& s2 ) { return s1.Compare( s2 ) != 0; }
inline bool operator!=( const FarString& s1, const char * s2 ) { return s1.Compare( s2 ) != 0; }
inline bool operator!=( const char * s1, const FarString& s2 ) { return s2.Compare( s1 ) != 0; }
inline bool operator<( const FarString& s1, const FarString& s2 ) { return s1.Compare( s2 ) < 0; }
inline bool operator<( const FarString& s1, const char * s2 ) { return s1.Compare( s2 ) < 0; }
inline bool operator<( const char * s1, const FarString& s2 ) { return s2.Compare( s1 ) > 0; }
inline bool operator>( const FarString& s1, const FarString& s2 ) { return s1.Compare( s2 ) > 0; }
inline bool operator>( const FarString& s1, const char * s2 ) { return s1.Compare( s2 ) > 0; }
inline bool operator>( const char * s1, const FarString& s2 ) { return s2.Compare( s1 ) < 0; }
inline bool operator<=( const FarString& s1, const FarString& s2 ){ return s1.Compare( s2 ) <= 0; }
inline bool operator<=( const FarString& s1, const char * s2 ) { return s1.Compare( s2 ) <= 0; }
inline bool operator<=( const char * s1, const FarString& s2 ) { return s2.Compare( s1 ) >= 0; }
inline bool operator>=( const FarString& s1, const FarString& s2 ) { return s1.Compare( s2 ) >= 0; }
inline bool operator>=( const FarString& s1, const char * s2 ) { return s1.Compare( s2 ) >= 0; }
inline bool operator>=( const char * s1, const FarString& s2 ) { return s2.Compare( s1 ) <= 0; }

FarString IntToString( int Value, int Radix = 10 );

#endif //!defined(__FARSTRING_H)
