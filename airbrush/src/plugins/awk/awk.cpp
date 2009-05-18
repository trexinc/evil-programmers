/*
    awk.cpp
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
#include <tchar.h>
#include "abplugin.h"
#include "abawk.h"

ColorizeInfo Info;
int colors[]={
  0x03,-1,              // HC_COMMENT  0
  0x0f,-1,              // HC_KEYWORD1 2
  0x0a,-1,              // HC_KEYWORD2 4
  0x0e,-1,              // HC_STRING   6
  0x02,-1,              // HC_REGEXPS  8
  0x06,-1,              // HC_REGEXP   10
  0x0a,-1,              // HC_NUMBER   12
  0x0f,-1,              // HC_BUILTIN  14
  0x0f,-1,              // HC_IO       16
  0x0a,-1,              // HC_PARAM    18
  0x0c,-1,              // HC_HIGHLITE 20
  0x0e,-1,              // HC_SYMBOL   22
  0x0d,-1,              // HC_PATTERNS 24
  0x01,0x0c             // HC_HL_ERROR 26
};
const TCHAR* colornames[]={_T("Comment"),_T("Keyword"),_T("Keyword2"),_T("String"),_T("Regexp '/'"),_T("Regexp body"),_T("Number"),_T("Builtin"),_T("IO"),_T("Param"),_T("Pair"),_T("Symbol"),_T("Patterns"),_T("Wrong Pair")};

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
      *param=(const char*)_T("awk");
      return true;
    case PAR_GET_PARAMS:
      return PAR_MASK_STORE|PAR_MASK_CACHE|PAR_COLORS_STORE|PAR_SHOW_IN_LIST;
    case PAR_GET_MASK:
      *param=(const char*)_T("*.awk");
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
