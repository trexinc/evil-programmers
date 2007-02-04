/*
    Macro plugin for DialogManager
    Copyright (C) 2004 Vadim Yegorov

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
#include "string.hpp"

#define MEMORY_STEP (64)

void string::init(void)
{
  data=NULL;
  default_data[0]=0;
  current_size=0;
  actual_size=0;
  hash_start=0;
  heap=GetProcessHeap();
}

void string::copy(const string& Value)
{
  if(enlarge(Value.current_size))
  {
    memcpy(data,Value.data,Value.current_size);
    current_size=Value.current_size;
    hash_start=Value.hash_start;
  }
}

void string::copy(const unsigned char *Value)
{
  int len=strlen((const char *)Value)+1;
  if(enlarge(len))
  {
    memcpy(data,Value,len);
    current_size=len;
    hash_start=0;
  }
}

void string::copy(const unsigned char *Value,size_t size)
{
  if(enlarge(size+1))
  {
    memcpy(data,Value,size);
    data[size]=0;
    current_size=size+1;
    hash_start=0;
  }
}

bool string::enlarge(size_t size)
{
  size_t new_actual_size=(size/MEMORY_STEP+((size%MEMORY_STEP)?1:0))*MEMORY_STEP;
  if(actual_size>=new_actual_size) return true;
  void *new_data;
  if(data)
    new_data=HeapReAlloc(heap,HEAP_ZERO_MEMORY,data,new_actual_size);
  else
    new_data=HeapAlloc(heap,HEAP_ZERO_MEMORY,new_actual_size);
  if(new_data)
  {
    data=(unsigned char *)new_data;
    actual_size=new_actual_size;
    return true;
  }
  return false;
}

string::string()
{
  init();
}

string::string(const unsigned char *Value)
{
  init();
  copy(Value);
}

string::string(const unsigned char *Value,size_t size)
{
  init();
  copy(Value,size);
}

string::string(const string& Value)
{
  init();
  copy(Value);
}

string::~string()
{
  if(data) HeapFree(heap,0,data);
}

string &string::operator=(const string& Value)
{
  if(this!=&Value)
    copy(Value);
  return *this;
}

string &string::operator=(const unsigned char *Value)
{
  copy(Value);
  return *this;
}

string &string::operator()(const unsigned char *Value,size_t size)
{
  copy(Value,size);
  return *this;
}

string::operator const unsigned char *() const
{
  if(data) return data;
  return default_data;
}

unsigned char &string::operator[](size_t index)
{
  if(index>=current_size) return default_data[0];
  return data[index];
}

size_t string::length(void) const
{
  return current_size?current_size-1:0;
}

#if !defined(__STRING_REVERSE)
void string::reverse(void)
{
  if(data) _strrev((char *)data);
}
#endif

unsigned long string::hash(void)
{
  return (((*this)[hash_start+0])*256+(*this)[hash_start+1])*256+(*this)[hash_start+2];
}

void string::hash(size_t pos)
{
  hash_start=pos;
}

unsigned char *string::get(void)
{
  if(data) return data;
  return default_data;
}

void string::clear(void)
{
  if(data&&actual_size)
  {
    current_size=1;
    data[0]=0;
  }
}

string& string::operator+=(unsigned char Value)
{
  if(enlarge(current_size+(current_size?1:2)))
  {
    if(!current_size) current_size++;
    data[current_size-1]=Value;
    data[current_size++]=0;
  }
  return *this;
}

int operator==(const string& x,const string& y)
{
  if(x.length()==y.length()) return memcmp((const unsigned char *)x,(const unsigned char *)y,x.length());
  else return x.length()-y.length();
}

#if !defined(__STRING_PLUS)
string operator+(string& x,string& y)
{
  string result;
  if(result.enlarge(x.length()+y.length()+1))
  {
    memcpy(result.data,x.data,x.length());
    memcpy(result.data+x.length(),y.data,y.length()+1);
    result.current_size=x.length()+y.length()+1;
    result.hash_start=0;
  }
  return result;
}
#endif
