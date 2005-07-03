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
#ifndef __VARIANT_HPP__
#define __VARIANT_HPP__

#include "type.hpp"
#include "custom_string.hpp"

//types
const long vtNull    = 0;
const long vtInt64   = 1;
const long vtString  = 2;

class Variant
{
  private:
    long type;
    long codepos;
    __INT64 i_value;
    CustomString s_value;
  public:
    Variant();
    Variant(const char *Value);
    Variant(__INT64 Value);
    Variant(bool Value);
#if !defined(__INT64_IS_LONG)
    Variant(long Value);
#endif
    Variant(int Value);
    Variant& operator=(const Variant& Value);
    Variant& operator=(const char *Value);
    Variant& operator=(__INT64 Value);
    Variant& operator=(bool Value);
#if !defined(__INT64_IS_LONG)
    Variant& operator=(long Value);
#endif
    Variant& operator=(int Value);
    operator const char *();
    operator __INT64();
#if !defined(__INT64_IS_LONG)
    operator long();
#endif
    operator int();
    operator bool();
    Variant& operator!();
    Variant& operator-();
    Variant& operator+=(long Value);
    Variant& operator-=(long Value);

    long length(void);
    void set(long index,__INT64 value);
    char *get(void);
    long gettype(void);
    long getcodepos(void);
    void setcodepos(long pos);

    friend Variant operator+(Variant& x,Variant& y);
    friend Variant operator>(Variant& x,Variant& y);
    friend Variant operator<(Variant& x,Variant& y);
    friend Variant operator==(Variant& x,Variant& y);
};

Variant operator-(Variant& x,Variant& y);
Variant operator*(Variant& x,Variant& y);
Variant operator/(Variant& x,Variant& y);
Variant operator>=(Variant& x,Variant& y);
Variant operator<=(Variant& x,Variant& y);
Variant operator!=(Variant& x,Variant& y);
Variant operator&&(Variant& x,Variant& y);
Variant operator||(Variant& x,Variant& y);

#endif
