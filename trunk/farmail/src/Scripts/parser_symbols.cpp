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
#include "parser.hpp"

Names& Names::operator=(const Names& Value)
{
  if(this!=&Value)
  {
    name=Value.name;
    value=Value.value;
  }
  return *this;
}

Names& Names::operator=(const char *Value)
{
  name=Value;
  return *this;
}

Names::operator const char *() const
{
  return name;
}

Variant& Names::Value(void)
{
  return value;
}

Symbols::Symbols()
{
  names=NULL;
  count=0;
}

Symbols::~Symbols()
{
  Clear();
}

long Symbols::IndexOf(const char *Value)
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

void Symbols::Clear(void)
{
  if(names)
  {
    delete [] names;
    names=NULL;
    count=0;
  }
}

bool Symbols::Add(const char *Name)
{
  bool result=false;
  if(Name)
  {
    Names *new_names=new Names[count+1];
    if(new_names)
    {
      for(int i=0;i<count;i++)
      {
        new_names[i]=names[i];
      }
      new_names[count++]=Name;
      delete [] names;
      names=new_names;
      result=true;
    }
  }
  return result;
}

long Symbols::GetCount(void)
{
  return count;
}

Names &Symbols::operator[](const char *Name)
{
  long index=IndexOf(Name);
  if(index>=0)
  {
    return names[index];
  }
  else return default_name;
}

Names &Symbols::operator[](long Index)
{
  if(Index>=0&&Index<count) return names[Index];
  else return default_name;
}
