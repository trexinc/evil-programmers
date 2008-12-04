/*
    re2c.cpp
    Copyright (C) 2000-2008 zg

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

#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include "abplugin.h"
#include "abre2c.h"

ColorizeInfo Info;
int colors[]={0x09,-1,0x03,-1,0x0E,-1,0x0F,-1};
const TCHAR *colornames[]={_T("Re2c"),_T("Comment"),_T("String"),_T("Keyword")};

int WINAPI SetColorizeInfo(ColorizeInfo *AInfo)
{
  if((AInfo->version<AB_VERSION)||(AInfo->api!=AB_API)) return false;
  Info=*AInfo;
  return true;
};

int WINAPI _export GetParams(int index,int command,const char **param)
{
  switch(command)
  {
    case PAR_GET_NAME:
      *param=(const char*)_T("re2c");
      return true;
    case PAR_GET_PARAMS:
      return PAR_MASK_STORE|PAR_MASK_CACHE|PAR_COLORS_STORE|PAR_SHOW_IN_LIST;
    case PAR_GET_MASK:
      *param=(const char*)_T("*.re");
      return true;
    case PAR_GET_COLOR_COUNT:
      *(int *)param=sizeof(colornames)/sizeof(colornames[0]);
      return true;
    case PAR_GET_COLOR:
      *(const int **)param=colors;
      return true;
    case PAR_GET_COLOR_NAME:
      *param=(const char *)colornames;
      return true;
  }
  return false;
}

#ifdef __cplusplus
extern "C"{
#endif
  bool WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

bool WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  return true;
}
