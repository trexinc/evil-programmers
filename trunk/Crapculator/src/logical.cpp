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
#include <string.h>

static long long vars[L'z'-L'a'+1];

static bool IsEnd(int c)
{
  return c == 0 || c == L')' || c == L'=' || c == L';';
}

static void SkipSpace(const wchar_t **p)
{
  while (**p == L' ' || **p == L'\t')
  {
    (*p)++;
  }
}

static int SkipOpenBracket(const wchar_t **p)
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

static int SkipCloseBracket(const wchar_t **p)
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

static bool IsHexDigit(int c)
{
  return (c >= L'0' && c <= L'9') || (c >= L'A' && c <= L'F') || (c >= L'a' && c <= L'f');
}

static long long GetHex(int c)
{
  switch (c)
  {
    case L'0':
    case L'1':
    case L'2':
    case L'3':
    case L'4':
    case L'5':
    case L'6':
    case L'7':
    case L'8':
    case L'9':
      return (c-L'0');

    case L'A':
    case L'B':
    case L'C':
    case L'D':
    case L'E':
    case L'F':
      return (10ll+c-L'A');

    case L'a':
    case L'b':
    case L'c':
    case L'd':
    case L'e':
    case L'f':
      return (10ll+c-L'a');
  }

  return 0ll;
}

static bool IsDigit(int c)
{
  return (c >= L'0' && c <= L'9');
}

static bool GetIPv4(const wchar_t **p, long long *n)
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
      x=x*10+**p-L'0';
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

static bool GetNumber(const wchar_t **p, long long *n)
{
  if (!IsDigit(**p))
    return false;

  *n=0;

  if (**p == L'0' && (*(*p+1) == L'x' || *(*p+1) == L'X'))
  {
    (*p)+=2;
    if (!IsHexDigit(**p))
      return false;

    while (IsHexDigit(**p))
    {
      *n=*n*16ll+GetHex(**p);
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

static bool GetAction(const wchar_t **p, int *a)
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

static bool GetUnaryActionPrefix(const wchar_t **p, int *a)
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


static bool Action(long long x, long long y, long long *r, int a)
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

static bool UnaryAction(long long *r, int a)
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

#define HIGHEST_PRECEDENCE (1000000)

static int Precedence(int a)
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

static bool Expression(const wchar_t **p, long long *n, int pa, int *b)
{
  if (!**p)
    return false;

  int o=SkipOpenBracket(p);
  *b+=o;

start:

  if (**p == L'$')
  {
    (*p)++; if (**p < L'a' || **p > L'z') return false;

    int v=**p-L'a';

    (*p)++;

    SkipSpace(p);

    if (**p == L':')
    {
      (*p)++;
      SkipSpace(p);
      if (**p != L'=') return false;
      (*p)++;

      int bb=0;
      long long nn=0;
      if (!Expression(p, &nn, 0 , &bb) || bb)
        return false;
      (*p)++;
      SkipSpace(p);

      vars[v]=nn;
      goto start;
    }
    else
    {
      *n=vars[v];
    }
  }
  else if (!GetNumber(p, n))
  {
    int u;
    if (!GetUnaryActionPrefix(p,&u))
      return false;

    int d=*b;
    if (!Expression(p, n, HIGHEST_PRECEDENCE , b))
      return false;

    if (!UnaryAction(n, u))
      return false;

    o-=d-*b;
    if (o<0)
      return true;
  }

  SkipSpace(p);

  if (*b)
  {
    int c=SkipCloseBracket(p);
    *b-=c;
    o-=c;
    if (o < 0)
      return true;
  }

  if (*b<0)
    return false;

  while (!IsEnd(**p))
  {
    int a;
    if (pa && o==0)
    {
      const wchar_t *s=*p;

      if (!GetAction(p, &a))
        return false;

      if (pa >= Precedence(a))
      {
        *p = s;
        return true;
      }
    }
    else
    {
      if (!GetAction(p, &a))
        return false;
    }

    long long x;
    int d=*b;
    if (!Expression(p, &x, Precedence(a), b))
      return false;

    if (!Action(*n, x, n, a))
      return false;

    o-=d-*b;
    if (o<0)
      return true;
  }

  *b-=SkipCloseBracket(p);

  return true;
}

bool Logical(const wchar_t **p, long long *n)
{
  int b=0;
  *n=0;

  memset(vars,0,sizeof(vars));

  return Expression(p,n,0,&b) && !b;
}
