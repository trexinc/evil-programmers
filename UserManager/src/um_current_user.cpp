/*
    um_current_user.cpp
    Copyright (C) 2002-2007 zg

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

#include <windows.h>
#include "memory.h"

static PSID user=NULL;

void init_current_user(void)
{
  HANDLE token; PTOKEN_USER token_user=NULL;
  if(OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token))
  {
    unsigned long size;
    GetTokenInformation(token,TokenUser,0,0,&size);
    token_user=(PTOKEN_USER)malloc(size);
    if(token_user)
    {
      if(GetTokenInformation(token,TokenUser,token_user,size,&size))
      {
        if((token_user->User.Sid)&&(IsValidSid(token_user->User.Sid)))
        {
          user=(PSID)malloc(GetLengthSid(token_user->User.Sid));
          if(user) CopySid(GetLengthSid(token_user->User.Sid),user,token_user->User.Sid);
        }
      }
      free(token_user);
    }
    CloseHandle(token);
  }
}

bool is_current_user(PSID sid)
{
  if(sid&&IsValidSid(sid)&&user&&IsValidSid(user)&&EqualSid(user,sid)) return true;
  return false;
}

PSID current_user(void)
{
  return user;
}

void free_current_user(void)
{
  free(user);
  user=NULL;
}
