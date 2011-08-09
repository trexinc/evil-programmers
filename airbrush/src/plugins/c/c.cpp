/*
    c.cpp
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

#include <windows.h>
#include <tchar.h>
#include "abplugin.h"
#include "abc.h"

ColorizeInfo Info;
ABColor colors[]=
{
  {ABCF_4BIT,0x03,0,   NULL,false,true },
  {ABCF_4BIT,0x0F,0,   NULL,false,true },
  {ABCF_4BIT,0x0D,0,   NULL,false,true },
  {ABCF_4BIT,0x0E,0,   NULL,false,true },
  {ABCF_4BIT,0x0A,0,   NULL,false,true },
  {ABCF_4BIT,0x0A,0,   NULL,false,true },
  {ABCF_4BIT,0x02,0,   NULL,false,true },
  {ABCF_4BIT,0x00,0x0A,NULL,false,false},
  {ABCF_4BIT,0x0C,0,   NULL,false,true },
  {ABCF_4BIT,0x01,0x0C,NULL,false,false}
};
const TCHAR* colornames[]={_T("Comment"),_T("Keyword"),_T("\";\" Symbol"),_T("String"),_T("Number"),_T("Hex Number"),_T("Preprocessor"),_T("//FIXME"),_T("Pair"),_T("Wrong Pair")};

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
      *param=(const char*)_T("c/c++");
      return true;
    case PAR_GET_PARAMS:
      return PAR_MASK_STORE|PAR_MASK_CACHE|PAR_COLORS_STORE|PAR_SHOW_IN_LIST;
    case PAR_GET_MASK:
      *param=(const char*)_T("*.c,*.h,*.cpp,*.cc,*.m,*.cxx,*.hpp,*.ii");
      return true;
    case PAR_GET_COLOR_COUNT:
      *(int*)param=sizeof(colornames)/sizeof(colornames[0]);
      return true;
    case PAR_GET_COLOR:
      *(const ABColor**)param=colors;
      return true;
    case PAR_GET_COLOR_NAME:
      *param=(const char*)colornames;
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
