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
#ifndef __FUNCTION_HPP__
#define __FUNCTION_HPP__

#include "variant.hpp"
#include <windows.h>

typedef Variant (WINAPI *UserFunction)(long count,Variant *values,int *stop,void *ptr);

class Functions
{
  private:
    CustomString *names;
    UserFunction *func;
    long count;
    HANDLE heap;
  public:
    Functions();
    ~Functions();
    long IndexOf(const char *Value);
    void Clear(void);
    bool Add(const char *Name,UserFunction Func);
    long GetCount(void);
    Variant Run(long index,long val_count,Variant *values,int *stop,void *ptr);
};

#endif
