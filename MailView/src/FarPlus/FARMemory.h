/* $Header: /cvsroot/farplus/FARPlus/FARMemory.h,v 1.1.1.1 2002/02/20 18:56:37 yole Exp $
   FAR+Plus: memory functions (replacements for the standard RTL functions)
   (C) 2001-02 Dmitry Jemerov <yole@spb.cityline.ru>
*/

#ifndef __FARMEMORY_H
#define __FARMEMORY_H

#include <stdlib.h>

#if _MSC_VER >= 1000
#pragma once
#pragma check_stack(off)
#endif

// If USE_WINDOWS_HEAP is defined, the RTL functions for heap manipulation
// are not linked in, reducing plugin size

//#define USE_WINDOWS_HEAP

#ifdef USE_WINDOWS_HEAP

void *my_malloc(size_t size);
void *my_realloc(void *block, size_t size);
void my_free(void *block);

#define malloc my_malloc
#define realloc my_realloc
#define free my_free

#endif

#endif

