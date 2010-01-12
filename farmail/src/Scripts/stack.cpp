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
#include "stack.hpp"

Stack::Stack()
{
  count=0;
  actual_count=0;
  stack=new Variant[STACK_BUFFER];
  if(stack) actual_count=STACK_BUFFER;
}

Stack::~Stack()
{
  delete [] stack;
}

Variant Stack::Pop(void)
{
  if(stack&&count)
  {
    Variant result;
    result=stack[--count];
    return result;
  }
  else return default_stack;
}

void Stack::Push(const Variant &Value)
{
  if(stack)
  {
    if(count>=actual_count)
    {
      Variant *new_stack=new Variant[actual_count+STACK_BUFFER];
      if(new_stack)
      {
        for(long i=0;i<count;i++)
          new_stack[i]=stack[i];
        delete [] stack;
        stack=new_stack;
        actual_count+=STACK_BUFFER;
      }
    }
    if(count<actual_count)
      stack[count++]=Value;
  }
}

Variant &Stack::Top(void)
{
  if(stack&&count) return stack[count-1];
  else return default_stack;
}

void Stack::Clear(void)
{
  count=0;
}
