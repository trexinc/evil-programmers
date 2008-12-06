/*
    [ESC] Editor's settings changer plugin for FAR Manager
    Copyright (C) 2001 Ivan Sintyurin
    Copyright (C) 2008 Alex Yaroslavsky

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
#ifndef __TArray_cpp
#define __TArray_cpp

#include "TArray.hpp"

template <class Object>
TArray<Object>::TArray(unsigned int delta):
  items(NULL), Count(0), internalCount(0)
{
  setDelta(delta);
}

template <class Object>
TArray<Object>::TArray(const TArray<Object> &rhs):
  items(NULL), Count(0), internalCount(0)
{
  *this=rhs;
}

template <class Object>
TArray<Object>& TArray<Object>::operator=(const TArray<Object> &rhs)
{
  setDelta(rhs.Delta);
  if(setSize(rhs.Count))
  {
    for(unsigned i=0;i<Count;++i)
    {
      if(rhs.items[i])
      {
        if(!items[i])
          items[i]=new Object;
        if(items[i])
          *items[i]=*rhs.items[i];
        else
        {
          Free();
          break;
        }
      }
      else
      {
        delete items[i];
        items[i]=NULL;
      }
    }
  }
  return *this;
}

template <class Object>
void TArray<Object>::Free()
{
  if(items)
  {
    for(unsigned i=0;i<Count;++i)
      delete items[i];
    free(items);
    items=NULL;
  }
  Count=internalCount=0;
}

template <class Object>
void TArray<Object>::setDelta(unsigned int newDelta)
{
  if(newDelta<4)
    newDelta=4;
  Delta=newDelta;
}

template <class Object>
bool TArray<Object>::setSize(unsigned int newSize)
{
  bool rc=false;
  unsigned int i;

  if(newSize < Count)               // уменьшение размера
  {
    for(i=newSize;i<Count;++i)
    {
      delete items[i];
      items[i]=NULL;
    }
    Count=newSize;
    rc=true;
  }
  else if (newSize < internalCount) // увеличение, но в рамках имеющегося
  {
    for(i=Count;i<newSize;++i)
      items[i]=NULL;
    Count=newSize;
    rc=true;
  }
  else                              // увеличение размера
  {
     unsigned int Remainder=newSize%Delta;
     unsigned int newCount=Remainder?(newSize+Delta)-Remainder:
       newSize?newSize:Delta;
     Object **newItems=static_cast<Object**>(malloc(newCount*sizeof(Object*)));
     if(newItems)
     {
       if(items)
       {
         memcpy(newItems,items,Count*sizeof(Object*));
         free(items);
       }
       items=newItems;
       internalCount=newCount;
       for(i=Count;i<newSize;++i)
         items[i]=NULL;
       Count=newSize;
       rc=true;
     }
  }
  return rc;
}

template <class Object>
bool TArray<Object>::deleteItem(unsigned int index)
{
  if(index<Count)
  {
    delete items[index];
    items[index]=NULL;
    return true;
  }
  return false;
}

template <class Object>
Object *TArray<Object>::setItem(unsigned int index, const Object &newItem)
{
  bool set=true;
  if(index<Count)
    deleteItem(index);
  else
    set=setSize(index+(index==Count));

  if(set)
  {
    items[index]=new Object;
    if(items[index])
      *items[index]=newItem;
    return items[index];
  }
  return NULL;
}

template <class Object>
Object *TArray<Object>::getItem(unsigned int index) const
{
  return (index<Count)?items[index]:NULL;
}

template <class Object>
int TArray<Object>::getIndex(const Object &item, int start)
{
  int rc=-1;
  if(start==-1)
    start=0;
  for (unsigned int i=start; i<Count; ++i)
  {
    if(items[i] && item==*items[i])
    {
      rc=i;
      break;
    }
  }
  return rc;
}

template <class Object>
int __cdecl TArray<Object>::CmpItems(const Object **el1, const Object **el2)
{
  if(el1==el2)
    return 0;
  else if(**el1==**el2)
    return 0;
  else if(**el1<**el2)
    return -1;
  else
    return 1;
}

template <class Object>
void TArray<Object>::Sort(TARRAYCMPFUNC user_cmp_func)
{
  if(Count)
  {
    if(!user_cmp_func)
      user_cmp_func=reinterpret_cast<TARRAYCMPFUNC>(CmpItems);
    qsort(reinterpret_cast<wchar_t*>(items),Count,
      sizeof(Object*),user_cmp_func);
  }
}

template <class Object>
bool TArray<Object>::Pack()
{
  bool was_changed=false;
  for (unsigned int index=1; index<Count; ++index)
  {
    if(*items[index-1]==*items[index])
    {
      deleteItem(index);
      was_changed=true;
      --Count;
      if(index<Count)
      {
        memcpy(&items[index], &items[index+1], sizeof(Object*)*(Count-index));
        --index;
      }
    }
  }
  return was_changed;
}

#endif // __TArray_cpp
