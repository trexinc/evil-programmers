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
#include <windows.h>

void * operator new(size_t size)
{
  return HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size);
}

void operator delete(void *block)
{
  if(block)
    HeapFree(GetProcessHeap(),0,block);
}

void * operator new[](size_t size)
{
  return HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size);
}

void operator delete[](void *block)
{
  if(block)
    HeapFree(GetProcessHeap(),0,block);
}

#ifdef __cplusplus
extern "C"{
#endif
void __cxa_pure_virtual(void);
#ifdef __cplusplus
};
#endif

void __cxa_pure_virtual(void)
{
}
