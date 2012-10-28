/*
    memory.cpp
    Copyright (C) 2002-2008 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
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
