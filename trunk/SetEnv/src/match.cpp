#include "stdafx.h"
#include "match.hpp"

BOOL Match(LPCTSTR pat, LPCTSTR text)
{
  switch (*pat)
  {
    case TEXT('\0'):
      return *text == TEXT('\0');

    case TEXT('?'):
      return *text != TEXT('\0') && Match(pat + 1, text + 1);

    case TEXT('*'):
      do
      {
        if (Match(pat + 1, text))
          return TRUE;
      }
      while (*text++);
      return FALSE;

    default:
      return (TCHAR)CharUpper((LPTSTR)*text) == (TCHAR)CharUpper((LPTSTR)*pat)
         && Match(pat + 1, text + 1);
  }
}

