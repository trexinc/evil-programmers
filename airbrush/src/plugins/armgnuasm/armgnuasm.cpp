/*
    armgnuasm.cpp
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
#include "abarmgnuasm.h"
#include <initguid.h>
#include "guid.h"

ColorizeInfo Info;
FarColor colors[]=
{
  {FCF_FG_4BIT            ,AB_OPAQUE(0x03),              0,NULL},
  {FCF_FG_4BIT            ,AB_OPAQUE(0x0e),              0,NULL},
  {FCF_FG_4BIT            ,AB_OPAQUE(0x0a),              0,NULL},{FCF_FG_4BIT            ,AB_OPAQUE(0x0a),              0,NULL},
  {FCF_FG_4BIT            ,AB_OPAQUE(0x02),              0,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,AB_OPAQUE(0x00),AB_OPAQUE(0x0a),NULL},
  {FCF_FG_4BIT            ,AB_OPAQUE(0x0c),              0,NULL},{FCF_FG_4BIT|FCF_BG_4BIT,AB_OPAQUE(0x01),AB_OPAQUE(0x0c),NULL},
  {FCF_FG_4BIT            ,AB_OPAQUE(0x0f),              0,NULL},
  {FCF_FG_4BIT            ,AB_OPAQUE(0x0f),              0,NULL},{FCF_FG_4BIT            ,AB_OPAQUE(0x0c),              0,NULL},{FCF_FG_4BIT,AB_OPAQUE(0x08),0,NULL},
  {FCF_FG_4BIT            ,AB_OPAQUE(0x0f),              0,NULL},{FCF_FG_4BIT            ,AB_OPAQUE(0x08),              0,NULL},
  {FCF_FG_4BIT            ,AB_OPAQUE(0x0f),              0,NULL},{FCF_FG_4BIT            ,AB_OPAQUE(0x08),              0,NULL},
  {FCF_FG_4BIT            ,AB_OPAQUE(0x0e),              0,NULL},{FCF_FG_4BIT            ,AB_OPAQUE(0x08),              0,NULL},
  {FCF_FG_4BIT            ,AB_OPAQUE(0x0d),              0,NULL},{FCF_FG_4BIT            ,AB_OPAQUE(0x0c),              0,NULL},{FCF_FG_4BIT,AB_OPAQUE(0x05),0,NULL},{FCF_FG_4BIT,AB_OPAQUE(0x0c),0,NULL},
  {FCF_FG_4BIT            ,AB_OPAQUE(0x0f),              0,NULL},{FCF_FG_4BIT            ,AB_OPAQUE(0x0c),              0,NULL},{FCF_FG_4BIT,AB_OPAQUE(0x08),0,NULL},
  {FCF_FG_4BIT            ,AB_OPAQUE(0x0f),              0,NULL},{FCF_FG_4BIT            ,AB_OPAQUE(0x08),              0,NULL},
  {FCF_FG_4BIT            ,AB_OPAQUE(0x0e),              0,NULL},{FCF_FG_4BIT            ,AB_OPAQUE(0x0e),              0,NULL},{FCF_FG_4BIT,AB_OPAQUE(0x06),0,NULL}
};
const TCHAR* colornames[]=
{
  _T("Comment"),
  _T("String"),
  _T("Number"),_T("Hex Number"),
  _T("Preprocessor"),
  _T("//FIXME"),
  _T("Pair"),_T("Wrong Pair"),
  _T("Keyword"),
  _T("ARM Arithmetic"),_T("ARM Arithmetic 5"),_T("THUMB Arithmetic"),
  _T("ARM Logical"),_T("THUMB Logical"),
  _T("ARM Move"),_T("THUMB Move"),
  _T("ARM Shift"),_T("THUMB Shift"),
  _T("ARM Branch"),_T("ARM Branch 5"),_T("THUMB Branch"),_T("THUMB Branch 5"),
  _T("ARM Memory"),_T("ARM Memory 5"),_T("THUMB Memory")
  _T("ARM Pseudo"),_T("THUMB Pseudo")
  _T("ARM Registers"),_T("THUMB Low Registers"),_T("THUMB High Registers")
};

int WINAPI SetColorizeInfo(ColorizeInfo *AInfo)
{
  if((AInfo->version<AB_VERSION)||(AInfo->api!=AB_API)) return false;
  Info=*AInfo;
  return true;
};

int WINAPI GetParams(intptr_t index,intptr_t command,const char **param)
{
  (void)index;
  static const ABName name={ArmGnuAsmGUID,_T("arm gnu asm")};
  switch(command)
  {
    case PAR_GET_NAME:
      *param=reinterpret_cast<const char*>(&name);
      return true;
    case PAR_GET_PARAMS:
      return PAR_MASK_STORE|PAR_MASK_CACHE|PAR_COLORS_STORE|PAR_SHOW_IN_LIST|PAR_BRACKETS;
    case PAR_GET_MASK:
      *param=(const char*)_T("*.s");
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
