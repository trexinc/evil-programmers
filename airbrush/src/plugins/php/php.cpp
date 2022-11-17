/*
    php.cpp
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
#include "abphp.h"
#include <initguid.h>
#include "guid.h"

ColorizeInfo Info;
FarColor colors[]=
{
  {            FCF_BG_4BIT,              0,AB_OPAQUE(0),NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,AB_OPAQUE(0x03),AB_OPAQUE(0),NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,AB_OPAQUE(0x0F),AB_OPAQUE(0),NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,AB_OPAQUE(0x0D),AB_OPAQUE(0),NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,AB_OPAQUE(0x0E),AB_OPAQUE(0),NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,AB_OPAQUE(0x0A),AB_OPAQUE(0),NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,AB_OPAQUE(0x08),AB_OPAQUE(0),NULL}
};
const TCHAR* colornames[]={_T("Common"),_T("Comment"),_T("Keyword"),_T("\";\" Symbol"),_T("String"),_T("Number"),_T("Variable")};

int WINAPI SetColorizeInfo(ColorizeInfo *AInfo)
{
  if((AInfo->version<AB_VERSION)||(AInfo->api!=AB_API)) return false;
  Info=*AInfo;
  return true;
};

int WINAPI GetParams(intptr_t index,intptr_t command,const char **param)
{
  (void)index;
  static const ABName name={PhpGUID,_T("php")};
  switch(command)
  {
    case PAR_GET_NAME:
      *param=reinterpret_cast<const char*>(&name);
      return true;
    case PAR_GET_PARAMS:
      return PAR_MASK_STORE|PAR_MASK_CACHE|PAR_COLORS_STORE;
    case PAR_GET_MASK:
      *param=(const char*)_T("");
      return true;
    case PAR_GET_COLOR_COUNT:
      *reinterpret_cast<size_t*>(param)=sizeof(colornames)/sizeof(colornames[0]);
      return true;
    case PAR_GET_COLOR:
      *reinterpret_cast<const FarColor**>(param)=colors;
      return true;
    case PAR_GET_COLOR_NAME:
      *param=reinterpret_cast<const char*>(colornames);
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
  (void)hDll;
  (void)dwReason;
  (void)lpReserved;
  return true;
}
