/*
    memory.hpp
    Copyright (C) 2012 zg

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

#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__

#define malloc(size) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size)
#define free(ptr) ((ptr)?HeapFree(GetProcessHeap(),0,ptr):0)
#define realloc(ptr,size) ((size)?((ptr)?HeapReAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,ptr,size):HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size)):(HeapFree(GetProcessHeap(),0,ptr),(void *)0))

#endif
