#ifndef __LOGICALEXPRESSION_HPP__
#define __LOGICALEXPRESSION_HPP__
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
#include "BasicExpression.hpp"

class LogicalExpression: public BasicExpression<long long>
{
  protected:
    bool GetUnaryActionPrefix(const wchar_t **p, int *a);
    bool UnaryAction(long long *r, int a);

    bool GetAction(const wchar_t **p, int *a);
    bool Action(long long x, long long y, long long *r, int a);

    int Precedence(int a);

    bool IsHexDigit(int c) { return (IsDigit(c) || ((c|0x20) >= L'a' && (c|0x20) <= L'f')); }
    long long GetHex(int c);
    bool GetIPv4(const wchar_t **p, long long *n);
    bool GetNumber(const wchar_t **p, long long *n);

  public:

    ~LogicalExpression() { }
};

#endif // __LOGICALEXPRESSION_HPP__
