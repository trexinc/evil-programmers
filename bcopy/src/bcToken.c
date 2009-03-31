/*
    bcToken.c
    Copyright (C) 2000-2009 zg

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

static void EnablePrivilege(const wchar_t *name)
{
  TOKEN_PRIVILEGES priv;
  HANDLE token;
  priv.PrivilegeCount=1;
  priv.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;

  if(!LookupPrivilegeValueW(0,name,&(priv.Privileges[0].Luid)))
    LogSys(L"LookupPrivilegeValue",NULL,NULL);
  else
  {
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&token))
      LogSys(L"OpenProcessToken",NULL,NULL);
    else
    {
      //AdjustTokenPrivileges(token,FALSE,&priv,sizeof(priv),0,0);
      //if(GetLastError()!=ERROR_SUCCESS)
      if(!AdjustTokenPrivileges(token,FALSE,&priv,sizeof(priv),0,0))
      {
        LogSys(L"AdjustTokenPrivileges",NULL,NULL);
      }
      CloseHandle(token);
    }
  }
}

void *DefaultTokenInformation(HANDLE token,TOKEN_INFORMATION_CLASS tic)
{
  unsigned long size; unsigned char *Result=NULL;
  GetTokenInformation(token,tic,0,0,&size);
  Result=(unsigned char *)malloc(size);
  if(Result)
  {
    if(!GetTokenInformation(token,tic,Result,size,&size))
      LogSys(L"GetTokenInformation",NULL,NULL);
  }
  return Result;
}

static void PipeToThread(HANDLE pipe,HANDLE thread)
{
  HANDLE pipe_token;
  BOOL CanOpenThread;

  if(!ImpersonateNamedPipeClient(pipe))
    LogSys(L"ImpersonateNamedPipeClient",NULL,NULL);
  else
  {
    CanOpenThread=OpenThreadToken(GetCurrentThread(),TOKEN_ALL_ACCESS,FALSE,&pipe_token);
    if(!CanOpenThread)
      LogSys(L"OpenThreadToken",NULL,NULL);
    if(!RevertToSelf())
      LogSys(L"RevertToSelf",NULL,NULL);
    if(CanOpenThread)
    {
      if(!SetThreadToken(&thread,pipe_token))
        LogSys(L"SetThreadToken",NULL,NULL);
      CloseHandle(pipe_token);
    }
  }
}
