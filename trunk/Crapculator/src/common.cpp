/*
    Crapculator plugin for FAR Manager
    Copyright (C) 2009 Alex Yaroslavsky

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

bool IsEnd(int c)
{
  return c == 0 || c == L')' || c == L'=' || c == L';';
}

void SkipSpace(const wchar_t **p)
{
  while (**p == L' ' || **p == L'\t')
  {
    (*p)++;
  }
}

int SkipOpenBracket(const wchar_t **p)
{
  int c=0;
  SkipSpace(p);
  while (**p == L'(')
  {
    (*p)++;
    c++;
    SkipSpace(p);
  }
  return c;
}

int SkipCloseBracket(const wchar_t **p)
{
  int c=0;
  SkipSpace(p);
  while (**p == L')')
  {
    (*p)++;
    c++;
    SkipSpace(p);
  }
  return c;
}

bool IsDigit(int c)
{
  return (c >= L'0' && c <= L'9');
}

bool IsHexDigit(int c)
{
  return (IsDigit(c) || ((c|0x20) >= L'a' && (c|0x20) <= L'f'));
}
