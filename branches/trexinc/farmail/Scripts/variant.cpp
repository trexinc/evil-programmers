/*
    Scripts sub-plugin for FARMail
    Copyright (C) 2002-2004 FARMail Group

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
#include "variant.hpp"
#include "scripts.hpp"

Variant::Variant()
{
  type=vtNull;
  i_value=0;
  codepos=0;
}

Variant::Variant(const char *Value)
{
  *this=Value;
}

Variant::Variant(__INT64 Value)
{
  *this=Value;
}

Variant::Variant(bool Value)
{
  *this=Value;
}

#if !defined(__INT64_IS_LONG)
Variant::Variant(long Value)
{
  *this=Value;
}
#endif

Variant::Variant(int Value)
{
  *this=Value;
}

Variant& Variant::operator=(const Variant& Value)
{
  if(this!=&Value)
  {
    type=Value.type;
    i_value=Value.i_value;
    s_value=Value.s_value;
    codepos=Value.codepos;
  }
  return *this;
}

Variant& Variant::operator=(const char *Value)
{
  type=vtString;
  s_value=Value;
  return *this;
}

Variant& Variant::operator=(__INT64 Value)
{
  type=vtInt64;
  i_value=Value;
  return *this;
}

Variant& Variant::operator=(bool Value)
{
  return operator=((__INT64)Value);
}

#if !defined(__INT64_IS_LONG)
Variant& Variant::operator=(long Value)
{
  *this=(__INT64)Value;
  return *this;
}
#endif

Variant& Variant::operator=(int Value)
{
  *this=(__INT64)Value;
  return *this;
}

Variant::operator const char *()
{
  if(type!=vtString)
  {
    char buffer[64];
    s_value=__ITOA(i_value,buffer,10);
  }
  return s_value;
}

Variant::operator __INT64()
{
  if(type==vtString)
    i_value=__ATOI(s_value);
  return i_value;
}

#if !defined(__INT64_IS_LONG)
Variant::operator long()
{
  return (__INT64)*this;
}
#endif

Variant::operator int()
{
  return (__INT64)*this;
}

Variant::operator bool()
{
  return (__INT64)*this;
}

Variant& Variant::operator!()
{
  *this=!((__INT64)*this);
  return *this;
}

Variant& Variant::operator-()
{
  *this=-((__INT64)*this);
  return *this;
}

Variant& Variant::operator+=(long Value)
{
  *this=(__INT64)*this+Value;
  return *this;
}

Variant& Variant::operator-=(long Value)
{
  *this=(__INT64)*this-Value;
  return *this;
}

long Variant::length(void)
{
  (const char *)*this;
  return s_value.length();
}

void Variant::set(long index,__INT64 value)
{
  (const char *)*this;
  s_value.set(index,value);
}

char *Variant::get(void)
{
  (const char *)*this;
  return s_value.get();
}

long Variant::gettype(void)
{
  return type;
}

long Variant::getcodepos(void)
{
  return codepos;
}

void Variant::setcodepos(long pos)
{
  codepos=pos;
}

Variant operator+(Variant& x,Variant& y)
{
  Variant result;
  if(x.type==vtString)
  {
    result.type=vtString;
    result.s_value((const char *)x,(const char *)y);
  }
  else
  {
    result=(__INT64)x+(__INT64)y;
  }
  return result;
}

Variant operator-(Variant& x,Variant& y)
{
  Variant result;
  result=(__INT64)x-(__INT64)y;
  return result;
}

Variant operator*(Variant& x,Variant& y)
{
  Variant result;
  result=(__INT64)x*(__INT64)y;
  return result;
}

Variant operator/(Variant& x,Variant& y)
{
  Variant result;
  __INT64 divider=y;
  if(divider==0)
    result=0;
  else
    result=(__INT64)x/divider;
  return result;
}

Variant operator>(Variant& x,Variant& y)
{
  Variant result;
  if(x.type==vtString)
    result=(lstrcmp(x,y)>0);
  else
    result=((__INT64)x>(__INT64)y);
  return result;
}

Variant operator<(Variant& x,Variant& y)
{
  Variant result;
  if(x.type==vtString)
    result=(lstrcmp(x,y)<0);
  else
    result=((__INT64)x<(__INT64)y);
  return result;
}

Variant operator==(Variant& x,Variant& y)
{
  Variant result;
  if(x.type==vtString)
    result=(lstrcmp(x,y)==0);
  else
    result=((__INT64)x==(__INT64)y);
  return result;
}

Variant operator>=(Variant& x,Variant& y)
{
  return !(x<y);
}

Variant operator<=(Variant& x,Variant& y)
{
  return !(x>y);
}

Variant operator!=(Variant& x,Variant& y)
{
  return !(x==y);
}

Variant operator&&(Variant& x,Variant& y)
{
  Variant result;
  result=(__INT64)x&&(__INT64)y;
  return result;
}

Variant operator||(Variant& x,Variant& y)
{
  Variant result;
  result=(__INT64)x||(__INT64)y;
  return result;
}
