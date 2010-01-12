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
#ifndef __CUSTOM_STRING_HPP__
#define __CUSTOM_STRING_HPP__

#include <windows.h>
#include <stddef.h>

class CustomString
{
  private:
    long size;
    char *string;
    char default_string[2];
    HANDLE heap;
    void init(void);
    void free(void);
    void copy(const CustomString& Value);
  public:
    CustomString();
    CustomString(const char *Value);
    CustomString(const CustomString& Value);
    ~CustomString();
    CustomString &operator=(const CustomString& Value);
    operator const char *() const;
    char &operator[](long index);
    CustomString &operator()(const char *Value1,const char *Value2);
    long length(void) const;
    void set(long index,long value);
    char *get(void);
};

#endif
