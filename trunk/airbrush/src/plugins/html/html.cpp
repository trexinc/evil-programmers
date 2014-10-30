/*
    html.cpp
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
#include "abhtml.h"

ColorizeInfo Info;
ABColor colors[]=
{
  {ABCF_4BIT,AB_OPAQUE(0x01),AB_OPAQUE(0x0C),NULL,false,false},
  {ABCF_4BIT,AB_OPAQUE(0x03),AB_OPAQUE(0x00),NULL,false,true },
  {ABCF_4BIT,AB_OPAQUE(0x0F),AB_OPAQUE(0x00),NULL,false,true },
  {ABCF_4BIT,AB_OPAQUE(0x0F),AB_OPAQUE(0x00),NULL,false,true },
  {ABCF_4BIT,AB_OPAQUE(0x0A),AB_OPAQUE(0x00),NULL,false,true },
  {ABCF_4BIT,AB_OPAQUE(0x0F),AB_OPAQUE(0x00),NULL,false,true },
  {ABCF_4BIT,AB_OPAQUE(0x0E),AB_OPAQUE(0x00),NULL,false,true },
  {ABCF_4BIT,AB_OPAQUE(0x07),AB_OPAQUE(0x00),NULL,false,true },
  {ABCF_4BIT,AB_OPAQUE(0x03),AB_OPAQUE(0x00),NULL,false,true },
  {ABCF_4BIT,AB_OPAQUE(0x07),AB_OPAQUE(0x00),NULL,false,true }
};
const TCHAR* colornames[]={_T("Error"),_T("Comment"),_T("Open Tag"),_T("Close Tag"),_T("Reference"),_T("Attribute Name"),_T("Attribute Value"),_T("Markup Declarations"),_T("Declaration Subset"),_T("Processing Instruction")};

int WINAPI SetColorizeInfo(ColorizeInfo *AInfo)
{
  if((AInfo->version<AB_VERSION)||(AInfo->api!=AB_API)) return false;
  Info=*AInfo;
  return true;
};

int WINAPI GetParams(intptr_t index,intptr_t command,const char **param)
{
  (void)index;
  switch(command)
  {
    case PAR_GET_NAME:
      *param=(const char*)_T("html");
      return true;
    case PAR_GET_PARAMS:
      return PAR_MASK_STORE|PAR_MASK_CACHE|PAR_COLORS_STORE|PAR_SHOW_IN_LIST|PAR_FILESTART_STORE|PAR_FILESTART_CACHE;
    case PAR_GET_MASK:
      *param=(const char*)_T("*.htm,*.html,*.shtml,*.php");
      return true;
    case PAR_GET_FILESTART:
      *param=(const char*)_T("*<[Hh][Tt][Mm][Ll]*");
      return true;
    case PAR_GET_COLOR_COUNT:
      *(int *)param=sizeof(colornames)/sizeof(colornames[0]);
      return true;
    case PAR_GET_COLOR:
      *(const ABColor**)param=colors;
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
  (void)hDll;
  (void)dwReason;
  (void)lpReserved;
  return true;
}
