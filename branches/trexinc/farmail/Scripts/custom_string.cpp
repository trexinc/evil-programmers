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
#include "custom_string.hpp"

void CustomString::init(void)
{
  size=0;
  string=NULL;
  heap=GetProcessHeap();
  default_string[0]=0;
  default_string[1]=0;
}

void CustomString::free(void)
{
  if(string) HeapFree(heap,0,string);
  size=0;
}

void CustomString::copy(const CustomString& Value)
{
  string=(char *)HeapAlloc(heap,HEAP_ZERO_MEMORY,Value.size);
  if(string)
  {
    memcpy(string,Value.string,Value.size);
    size=Value.size;
  }
}

CustomString::CustomString()
{
  init();
}

CustomString::CustomString(const char *Value)
{
  int len=lstrlen(Value)+1;
  init();
  string=(char *)HeapAlloc(heap,HEAP_ZERO_MEMORY,len);
  if(string)
  {
    memcpy(string,Value,len);
    size=len;
  }
}

CustomString::CustomString(const CustomString& Value)
{
  init();
  copy(Value);
}

CustomString::~CustomString()
{
  free();
}

CustomString& CustomString::operator=(const CustomString& Value)
{
  if(this!=&Value)
  {
    free();
    copy(Value);
  }
  return *this;
}

CustomString::operator const char *() const
{
  if(string) return string;
  return default_string;
}

char &CustomString::operator[](long index)
{
  if(index<0||index>=size) return default_string[1];
  return string[index];
}

CustomString &CustomString::operator()(const char *Value1,const char *Value2)
{
  int len1=lstrlen(Value1),len2=lstrlen(Value2)+1;
  free();
  string=(char *)HeapAlloc(heap,HEAP_ZERO_MEMORY,len1+len2);
  if(string)
  {
    memcpy(string,Value1,len1);
    memcpy(string+len1,Value2,len2);
    size=len1+len2;
  }
  return *this;
}

long CustomString::length(void) const
{
  return size?size-1:0;
}

void CustomString::set(long index,long value)
{
  if(index>=0&&index<(size-1))
  {
    string[index]=value;
    if(!value) size=index+1;
  }
}

char *CustomString::get(void)
{
  if(string) return string;
  return default_string;
}
