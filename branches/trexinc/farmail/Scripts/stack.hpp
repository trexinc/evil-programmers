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
#ifndef __STACK_HPP__
#define __STACK_HPP__

#include "variant.hpp"

#define STACK_BUFFER (1024)

class Stack
{
  private:
    Variant *stack;
    long count;
    long actual_count;
    Variant default_stack;
  public:
    Stack();
    ~Stack();
    Variant Pop(void);
    void Push(const Variant &Value);
    Variant &Top(void);
    void Clear(void);
};

#endif
