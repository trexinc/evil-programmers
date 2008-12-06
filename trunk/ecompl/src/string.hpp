/*
    string.hpp
    Copyright (C) 2002-2008 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __STRING_HPP__
#define __STRING_HPP__

#include <windows.h>
#include <stddef.h>

class string
{
  private:
    size_t current_size;
    size_t actual_size;
    size_t hash_start;
    unsigned char *data;
    unsigned char default_data[1];
    HANDLE heap;
    void init(void);
    void copy(const string& Value);
    void copy(const unsigned char *Value);
    void copy(const unsigned char *Value,size_t size);
    bool enlarge(size_t size);
  public:
    string();
    string(const unsigned char *Value);
    string(const unsigned char *Value,size_t size);
    string(const string& Value);
    ~string();
    string &operator=(const string& Value);
    string &operator=(const unsigned char *Value);
    string &operator()(const unsigned char *Value,size_t size);
    operator const unsigned char *() const;
    unsigned char &operator[](size_t index);
    size_t length(void) const;
#if !defined(__STRING_REVERSE)
    void reverse(void);
#endif
    unsigned long hash(void);
    void hash(size_t pos);
    unsigned char *get(void);
    void clear(void);
    string& operator+=(unsigned char Value);

#if !defined(__STRING_PLUS)
    friend string operator+(string& x,string& y);
#endif
};

int operator==(const string& x,const string& y);

#endif
