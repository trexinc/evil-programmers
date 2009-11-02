/*
    bcn.cpp
    Copyright (C) 2009 zg

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
#include <stdio.h>
#include "../../src/bcsvc_interface.h"

int WINAPI Notify(unsigned long Msg,void *Data)
{
  if(BCSVC_START_JOB==Msg)
  {
    FILE *log=fopen("c:\\plugins.log","at");
    if(log)
    {
      SYSTEMTIME CurrTime;
      GetLocalTime(&CurrTime);
      fprintf(log,"%04d-%02d-%02d,%02d:%02d:%02d,%ls\n",CurrTime.wYear,CurrTime.wMonth,CurrTime.wDay,CurrTime.wHour,CurrTime.wMinute,CurrTime.wSecond,((BCSvcJob*)Data)->JobName);
      fclose(log);
    }
  }
  return 0;
}

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void)hDll;
  (void)dwReason;
  (void)lpReserved;
  return TRUE;
}
