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
#include "function.hpp"

Functions::Functions()
{
  names=NULL;
  func=NULL;
  count=0;
  heap=GetProcessHeap();
}

Functions::~Functions()
{
  Clear();
}

long Functions::IndexOf(const char *Value)
{
  if(names)
  {
    for(long i=0;i<count;i++)
    {
      if((!lstrcmp(names[i],Value)))
        return i;
    }
  }
  return -1;
}

void Functions::Clear(void)
{
  if(names)
  {
    delete [] names;
    names=NULL;
  }
  if(func)
  {
    delete [] func;
    func=NULL;
  }
  count=0;
}

bool Functions::Add(const char *Name,UserFunction Func)
{
  bool result=false;
  if(Name&&Func)
  {
    CustomString *new_names=new CustomString[count+1];
    UserFunction *new_func=new UserFunction[count+1];
    if(new_names&&new_func)
    {
      for(int i=0;i<count;i++)
      {
        new_names[i]=names[i];
        new_func[i]=func[i];
      }
      new_names[count]=Name;
      new_func[count++]=Func;
      delete [] names;
      delete [] func;
      names=new_names;
      func=new_func;
      result=true;
    }
    else
    {
      delete [] new_names;
      delete [] new_func;
    }
  }
  return result;
}

long Functions::GetCount(void)
{
  return count;
}

Variant Functions::Run(long index,long val_count,Variant *values,int *stop,void *ptr)
{
  Variant result;
  if(index>-1&&index<count)
    result=func[index](val_count,values,stop,ptr);
  return result;
}
