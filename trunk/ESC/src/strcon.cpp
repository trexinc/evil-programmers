/*
    [ESC] Editor's settings changer plugin for FAR Manager
    Copyright (C) 2001 Ivan Sintyurin
    Copyright (C) 2008 Alex Yaroslavsky

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
#ifndef __strcon_cpp
#define __strcon_cpp

#include <CRT/crt.hpp>
#include <stdarg.h>
#include <windows.h>
#include "strcon.hpp"

strcon::strcon(const wchar_t *ptr,int size):str(NULL)
{
  setStr(ptr,size);
}

strcon::strcon(const strcon &rhs):str(NULL)
{
  setStr(rhs.str);
}

strcon::strcon():str(NULL)
{
  setStr(L"");
}

strcon::~strcon()
{
  Free();
}

bool strcon::operator==(const strcon &ptr) const
{
  return (str && ptr.str)?!lstrcmp(str,ptr.str):0;
}

bool strcon::operator==(const wchar_t * ptr) const
{
  return (str && ptr)?!lstrcmp(str,ptr):0;
}

strcon& strcon::operator=(const wchar_t *rhs)
{
  if(rhs<str || rhs>(str+Len))
    setStr(rhs);
  return *this;
}

strcon& strcon::operator=(const strcon &rhs)
{
  if(this!=&rhs) setStr(rhs.str);
  return *this;
}

const strcon& strcon::AddStr(const strcon &s)
{
  return AddStr(s.str,s.Len);
}

const strcon& strcon::AddStrings(const wchar_t *firstStr, ...)
{
  AddStr(firstStr);
  va_list l;
  const wchar_t *arg;
  va_start(l, firstStr);
  while ((arg = va_arg(l,const wchar_t*)) != NULL)
    AddStr(arg);
  va_end(l);
  return *this;
}

const strcon& strcon::AddStr(const wchar_t *s, int size)
{
  if(s)
  {
    if(size<0)
      size=lstrlen(s);

    if(s>str && s<(str+Len))
    {
      strcon tmp(s, size);
      return AddStr(tmp.str);
    }

    int newLen=size+Len;
    wchar_t *newstr=static_cast<wchar_t*>(malloc((1+newLen)*sizeof(wchar_t)));
    if(newstr)
    {
      memcpy(newstr, str, Len*sizeof(wchar_t));
      memcpy(newstr+Len, s, size*sizeof(wchar_t));
      newstr[newLen]=0;
      Free();
      str=newstr;
      Len=newLen;
    }
  }
  return *this;
}

const strcon& strcon::AddChar(wchar_t Chr)
{
  return AddStr(&Chr,1);
}

bool strcon::setLength(unsigned newLen)
{
  if(Len>=newLen)
  {
    Len=newLen;
    str[Len]=0;
    return true;
  }
  return false;
}

wchar_t *strcon::setStr(const wchar_t *s,int size)
{
  if(s)
  {
    unsigned int newLen=lstrlen(s);
    if(size>-1 && static_cast<unsigned int>(size)<newLen)
      newLen=size;
    wchar_t *newstr=static_cast<wchar_t*>(malloc((1+newLen)*sizeof(wchar_t)));
    if(newstr)
    {
      memcpy(newstr, s, newLen*sizeof(wchar_t));
      newstr[newLen]=0;
      Free();
      str=newstr;
      Len=newLen;
    }
    else
      Free();
  }
  else
    Free();
  return str;
}

void strcon::Free()
{
  if(str)
  {
    free(str);
    str=NULL;
  }
  Len=0;
}

#endif //__strcon_cpp
