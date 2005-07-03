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
#ifndef __CUSTOM_MEMORY_HPP__
#define __CUSTOM_MEMORY_HPP__

#include <stddef.h>
#include <windows.h>

class CustomMemory
{
  private:
    char *data;
    long position;
    long current_size;
    long actual_size;
    HANDLE heap;
    bool IncreaseData(long size);
  public:
    CustomMemory();
    ~CustomMemory();
    long GetPosition(void) { return position; }
    bool SetPosition(long new_position) { if(new_position<=current_size) { position=new_position; return true; } else return false; }
    long GetSize(void) { return current_size; }
    long Read(void *ptr,long size);
    bool Write(const void *ptr,long size);
    bool WriteRandom(long offset,const void *ptr,long size);
    void Clear(void);
};

#endif
