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
#include "plugin.hpp"
#include <math.h>
#include <errno.h>

bool IsEnd(int c)
{
  return c == 0 || c == L')' || c == L'=';
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

bool GetNumber(const wchar_t **p, double *n)
{
  if (**p < L'0' || **p > L'9')
    return false;

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

bool GetAction(const wchar_t **p, int *a)
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

bool GetUnaryActionPrefix(const wchar_t **p, int *a)
{
  switch (**p)
  {
    case L'+':
    case L'-':
      *a = **p;
      break;

    case L's':
    case L'S':
      (*p)++; if (**p!=L'q' && **p!=L'Q') return false;
      (*p)++; if (**p!=L'r' && **p!=L'R') return false;
      (*p)++; if (**p!=L't' && **p!=L'T') return false;
      *a = L's';
      break;

    default: return false;
  }

  (*p)++;

  return true;
}


bool Action(double x, double y, double *r, int a)
{
  switch (a)
  {
    case L'+': *r=x+y; break;
    case L'-': *r=x-y; break;
    case L'*': *r=x*y; break;
    case L'/': if (!y) return false; *r=x/y; break;
    case L'%': if (!y) return false; *r=(int)x%(int)y; break;
    case L'^': errno=0; *r=pow(x,y); if (errno==EDOM) return false; break;

    default: return false;
  }

  return true;
}

bool UnaryAction(double *r, int a)
{
  switch (a)
  {
    case L'+': break;
    case L'-': *r=*r*-1.0; break;
    case L's': errno=0; *r=sqrt(*r); if (errno==EDOM) return false; break;

    default: return false;
  }

  return true;
}

#define HIGHEST_PRECEDENCE (1000000)

int Precedence(int a)
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

struct PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *psi)
{
  Info=*psi;
  FSF=*psi->FSF;
  Info.FSF=&FSF;
}

void WINAPI GetPluginInfoW(struct PluginInfo *pi)
{
  static const wchar_t *MenuStrings[1] = {L"Crapculator"};

  pi->StructSize=sizeof(struct PluginInfo);
  pi->Flags=PF_EDITOR|PF_DISABLEPANELS;
  pi->PluginMenuStrings=MenuStrings;
  pi->PluginMenuStringsNumber=1;
}

HANDLE WINAPI OpenPluginW(int OpenFrom, INT_PTR Item)
{
  int b=0;
  double n=0;
  wchar_t result[200];
  wchar_t error[] = L"Crap";
  wchar_t *r=error;

  struct EditorGetString egs = {-1, NULL, NULL, 0, 0, 0};

  if (Info.EditorControl(ECTL_GETSTRING,(void *)&egs) && egs.StringText)
  {
    if (Expression(&egs.StringText,&n,0,&b) && !b)
    {
      FSF.sprintf(result,L"%f",n);
      r=result;
    }
    Info.EditorControl(ECTL_INSERTTEXT,(void *)r);
  }

  return INVALID_HANDLE_VALUE;
}

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}
#endif
