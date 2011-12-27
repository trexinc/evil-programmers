/*
    Crapculator plugin for Far Manager
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
#include "LogicalExpression.hpp"

long long LogicalExpression::GetHex(int c)
{
  if (IsDigit(c))
    return (c-L'0');

  if (IsHexDigit(c))
    return (10ll + (c|0x20) - L'a');

  return 0ll;
}

bool LogicalExpression::GetIPv4(const wchar_t **p, long long *n)
{
  const wchar_t *s=*p;

  *n=0;

  for (int c=0; c<4; c++)
  {
    int x=0;
    int i=0;

    if (!IsDigit(**p))
      goto bad;

    while (IsDigit(**p))
    {
      x=x*10 + **p - L'0';
      (*p)++;
      i++;

      if (i>3 || x>255)
        goto bad;
    }

    *n=(*n<<8)|x;

    if (c==3)
      break;

    if (**p != L'.')
      goto bad;

    (*p)++;
  }

  return true;

bad:
  *p=s;
  return false;
}

bool LogicalExpression::GetNumber(const wchar_t **p, long long *n)
{
  if (!IsDigit(**p))
    return false;

  *n=0;

  if (**p == L'0' && ((*(*p+1))|0x20) == L'x')
  {
    (*p)+=2;

    if (!IsHexDigit(**p))
      return false;

    while (IsHexDigit(**p))
    {
      *n=*n*16ll + GetHex(**p);
      (*p)++;
    }
  }
  else if (!GetIPv4(p,n))
  {
    *n=0;
    while (IsDigit(**p))
    {
      *n=*n*10ll+**p-L'0';
      (*p)++;
    }
  }

  return true;
}

bool LogicalExpression::GetAction(const wchar_t **p, int *a)
{
  switch (**p)
  {
    case L'+':
    case L'-':
    case L'*':
    case L'/':
    case L'%':
    case L'&':
    case L'|':
    case L'^':
      *a = **p;
      break;

    case L'<':
    case L'>':
      if ( **p != *(*p+1)) return false;
      *a = **p;
      (*p)++;
      break;

    default: return false;
  }

  (*p)++;

  return true;
}

bool LogicalExpression::GetUnaryActionPrefix(const wchar_t **p, int *a)
{
  switch (**p)
  {
    case L'+':
    case L'-':
    case L'~':
      *a = **p;
      break;

    default: return false;
  }

  (*p)++;

  return true;
}


bool LogicalExpression::Action(long long x, long long y, long long *r, int a)
{
  switch (a)
  {
    case L'+': *r=x+y; break;
    case L'-': *r=x-y; break;
    case L'*': *r=x*y; break;
    case L'/': if (!y) return false; *r=x/y; break;
    case L'%': if (!y) return false; *r=x%y; break;
    case L'&': *r=x&y; break;
    case L'|': *r=x|y; break;
    case L'<': *r=x<<y; break;
    case L'>': *r=x>>y; break;
    case L'^': *r=x^y; break;

    default: return false;
  }

  return true;
}

bool LogicalExpression::UnaryAction(long long *r, int a)
{
  switch (a)
  {
    case L'+': break;
    case L'-': *r=*r*-1ll; break;
    case L'~': *r=~(*r); break;

    default: return false;
  }

  return true;
}

int LogicalExpression::Precedence(int a)
{
  switch (a)
  {
    case L'|':
      return 10;

    case L'^':
      return 25;

    case L'&':
      return 50;

    case L'<':
    case L'>':
      return 100;

    case L'+':
    case L'-':
      return 1000;

    case L'*':
    case L'/':
    case L'%':
      return 10000;

  }

  return 0;
}
