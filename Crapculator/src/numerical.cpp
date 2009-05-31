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

#include <math.h>
#include <errno.h>

static bool IsEnd(int c)
{
  return c == 0 || c == L')' || c == L'=';
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

static bool GetNumber(const wchar_t **p, double *n)
{
  if (**p < L'0' || **p > L'9')
  {
    if (**p == L'e' || **p == L'E')
    {
      *n=M_E;
      (*p)++;
      return true;

    }
    if (**p == L'p' || **p == L'P')
    {
      (*p)++;
      if (**p == L'i' || **p == L'I')
      {
        *n=M_PI;
        (*p)++;
        return true;
      }
      (*p)--;
    }
    return false;
  }

  *n=0;
  while (**p >= L'0' && **p <= L'9')
  {
    *n=*n*10+**p-L'0';
    (*p)++;
  }

  if (**p != '.')
    return true;

  (*p)++;

  double r=0;
  double c=10.0;
  while (**p >= L'0' && **p <= L'9')
  {
    r+=((double)(**p-L'0'))/c;
    (*p)++;
    c*=10.0;
  }

  *n+=r;

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
    case L'^':
      *a = **p;
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
      *a = **p;
      break;

    case L'c':
    case L'C':
    {
      (*p)++;
      switch (**p)
      {
        case L'o':
        case L'O':
          (*p)++; if (**p!=L's' && **p!=L'S') return false;
          *a = L'c';
          break;

        case L'e':
        case L'E':
          (*p)++; if (**p!=L'i' && **p!=L'I') return false;
          (*p)++; if (**p!=L'l' && **p!=L'L') return false;
          *a = L'e';
          break;

        default: return false;
      }
      break;
    }

    case L'd':
    case L'D':
      (*p)++; if (**p!=L'e' && **p!=L'E') return false;
      (*p)++; if (**p!=L'g' && **p!=L'G') return false;
      *a = L'd';
      break;

    case L'f':
    case L'F':
      (*p)++; if (**p!=L'l' && **p!=L'L') return false;
      (*p)++; if (**p!=L'o' && **p!=L'O') return false;
      (*p)++; if (**p!=L'o' && **p!=L'O') return false;
      (*p)++; if (**p!=L'r' && **p!=L'R') return false;
      *a = L'f';
      break;

    case L'l':
    case L'L':
    {
      (*p)++;
      switch (**p)
      {
        case L'n':
        case L'N':
          *a = L'n';
          break;

        case L'o':
        case L'O':
          (*p)++; if (**p!=L'g' && **p!=L'G') return false;
          *a = L'l';
          break;

        default: return false;
      }
      break;
    }

    case L'r':
    case L'R':
      (*p)++; if (**p!=L'a' && **p!=L'A') return false;
      (*p)++; if (**p!=L'd' && **p!=L'D') return false;
      *a = L'r';
      break;

    case L's':
    case L'S':
    {
      (*p)++;
      switch (**p)
      {
        case L'q':
        case L'Q':
          (*p)++; if (**p!=L'r' && **p!=L'R') return false;
          (*p)++; if (**p!=L't' && **p!=L'T') return false;
          *a = L's';
          break;

        case L'i':
        case L'I':
          (*p)++; if (**p!=L'n' && **p!=L'N') return false;
          *a = L'i';
          break;

        default: return false;
      }
      break;
    }

    case L't':
    case L'T':
      (*p)++; if (**p!=L'a' && **p!=L'A') return false;
      (*p)++; if (**p!=L'n' && **p!=L'N') return false;
      *a = L't';
      break;

    default: return false;
  }

  (*p)++;

  return true;
}


static bool Action(double x, double y, double *r, int a)
{
  switch (a)
  {
    case L'+': *r=x+y; break;
    case L'-': *r=x-y; break;
    case L'*': *r=x*y; break;
    case L'/': if (!y) return false; *r=x/y; break;
    case L'%': if (!y) return false; *r=(int)x%(int)y; break;
    case L'^': errno=0; *r=pow(x,y); if (errno) return false; break;

    default: return false;
  }

  return true;
}

static bool UnaryAction(double *r, int a)
{
  switch (a)
  {
    case L'+': break;
    case L'-': *r=*r*-1.0; break;

    case L's': errno=0; *r=sqrt(*r); if (errno) return false; break;

    case L'i': errno=0; *r=sin(*r); if (errno) return false; break;
    case L'c': errno=0; *r=cos(*r); if (errno) return false; break;
    case L't': errno=0; *r=tan(*r); if (errno) return false; break;
    case L'd': errno=0; *r=*r/(2.0*M_PI)*360.0; if (errno) return false; break;
    case L'r': errno=0; *r=*r/360.0*(2.0*M_PI); if (errno) return false; break;

    case L'n': errno=0; *r=log(*r); if (errno) return false; break;
    case L'l': errno=0; *r=log10(*r); if (errno) return false; break;

    case L'f': errno=0; *r=floor(*r); if (errno) return false; break;
    case L'e': errno=0; *r=ceil(*r); if (errno) return false; break;

    default: return false;
  }

  return true;
}

#define HIGHEST_PRECEDENCE (1000000)

static int Precedence(int a)
{
  switch (a)
  {
    case L'+':
    case L'-':
      return 10;

    case L'*':
    case L'/':
    case L'%':
      return 100;

    case L'^':
      return 1000;
  }

  return 0;
}

bool Expression(const wchar_t **p, double *n, int pa, int *b)
{
  if (!**p)
    return false;

  int o=SkipOpenBracket(p);
  *b+=o;

  if (!GetNumber(p, n))
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

    double x;
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
