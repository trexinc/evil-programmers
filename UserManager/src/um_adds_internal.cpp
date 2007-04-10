/*
    um_adds_internal.cpp
    Copyright (C) 2003-2007 zg

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "..\..\plugin.hpp"
#include "umplugin.h"
#include "memory.h"

bool AddOwnerInternal(UserManager *panel,PSID user)
{
  bool res=false;
  PSECURITY_DESCRIPTOR SD=NULL;
  SD=(PSECURITY_DESCRIPTOR)malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
  if(SD)
  {
    if(InitializeSecurityDescriptor(SD,SECURITY_DESCRIPTOR_REVISION))
      if(SetSecurityDescriptorOwner(SD,user,FALSE))
        if(set_security_descriptor[panel->level])
          res=set_security_descriptor[panel->level](panel,OWNER_SECURITY_INFORMATION,SD);
    free(SD);
  }
  return res;
}
