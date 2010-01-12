/*
    AddressBook sub-plugin for FARMail
    Copyright (C) 2002-2004 FARMail Group
    Copyright (C) 1999,2000 Serge Alexandrov

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
#ifndef __REGISTRY_HPP_
#define __REGISTRY_HPP_

void SetRegKey2(HKEY hRoot, const char *root, const char *Key,const char *ValueName,const char *ValueData,DWORD type=REG_SZ);
int GetRegKey2(HKEY hRoot,const char * root, const char *Key,const char *ValueName,char *ValueData,const char *Default,DWORD DataSize);

extern FARSTANDARDFUNCTIONS FSF;
extern const char NULLSTR[];

#endif
