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
#include "custom_memory.hpp"

#define MEMORY_STEP (4*1024)

CustomMemory::CustomMemory()
{
  data=NULL;
  position=0;
  current_size=0;
  actual_size=0;
  heap=GetProcessHeap();
}

CustomMemory::~CustomMemory()
{
  if(data)
    HeapFree(heap,0,data);
}

bool CustomMemory::IncreaseData(long size)
{
  long new_actual_size=actual_size+(size/MEMORY_STEP+((size%MEMORY_STEP)?1:0))*MEMORY_STEP;
  void *new_data;
  if(data)
    new_data=HeapReAlloc(heap,HEAP_ZERO_MEMORY,data,new_actual_size);
  else
    new_data=HeapAlloc(heap,HEAP_ZERO_MEMORY,new_actual_size);
  if(new_data)
  {
    data=(char *)new_data;
    actual_size=new_actual_size;
    return true;
  }
  return false;
}

long CustomMemory::Read(void *ptr,long size)
{
  long result=(size<=(current_size-position))?size:current_size-position;
  memcpy(ptr,data+position,result);
  position+=result;
  return result;
}

bool CustomMemory::Write(const void *ptr,long size)
{
  if(size>(actual_size-position))
    if(!IncreaseData(size-(actual_size-position))) return false;
  memcpy(data+position,ptr,size);
  position+=size;
  if(current_size<position) current_size=position;
  return true;
}

bool CustomMemory::WriteRandom(long offset,const void *ptr,long size)
{
  if(offset<0||(offset+size)>actual_size) return false;
  memcpy(data+offset,ptr,size);
  return true;
}

void CustomMemory::Clear(void)
{
  position=0;
  current_size=0;
}
