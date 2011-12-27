/*
    Crapculator plugin for Far Manager
    Copyright (C) 2009 Alex Yaroslavsky

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <CRT/crt.hpp>
#include "plugin.hpp"
#include "version.hpp"
#include "LogicalExpression.hpp"
#include "NumericalExpression.hpp"

struct PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

// {FD5F213D-6832-44BE-BD92-72177F9EAA9D}
static const GUID MainGuid =
{ 0xfd5f213d, 0x6832, 0x44be, { 0xbd, 0x92, 0x72, 0x17, 0x7f, 0x9e, 0xaa, 0x9d } };

// {A7D97129-920B-4056-9B0C-E747927C13E8}
static const GUID CrapculatorGuid =
{ 0xa7d97129, 0x920b, 0x4056, { 0x9b, 0xc, 0xe7, 0x47, 0x92, 0x7c, 0x13, 0xe8 } };

// {408F5D15-144B-4022-9CA8-28AF8AA4E20E}
static const GUID HackulatorGuid =
{ 0x408f5d15, 0x144b, 0x4022, { 0x9c, 0xa8, 0x28, 0xaf, 0x8a, 0xa4, 0xe2, 0xe } };


void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=PLUGIN_VERSION;
  Info->Guid=MainGuid;
  Info->Title=PLUGIN_NAME;
  Info->Description=PLUGIN_DESC;
  Info->Author=PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *psi)
{
  Info=*psi;
  FSF=*psi->FSF;
  Info.FSF=&FSF;
}

void WINAPI GetPluginInfoW(struct PluginInfo *pi)
{
  static const wchar_t *MenuStrings[2] = {L"Crapculator", L"Hackulator"};
  static const GUID MenuGuids[2] = {CrapculatorGuid, HackulatorGuid};

  pi->StructSize=sizeof(struct PluginInfo);
  pi->Flags=PF_EDITOR|PF_DISABLEPANELS;
  pi->PluginMenu.Guids=MenuGuids;
  pi->PluginMenu.Strings=MenuStrings;
  pi->PluginMenu.Count=ARRAYSIZE(MenuStrings);
}

HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
  wchar_t result[200];
  wchar_t error[] = L"Crap";
  wchar_t *r=error;

  struct EditorGetString egs = {-1, 0, NULL, NULL, 0, 0};

  if (Info.EditorControl(-1,ECTL_GETSTRING,0,&egs) && egs.StringText)
  {
    if (IsEqualGUID(*OInfo->Guid,CrapculatorGuid))
    {
      double d=0;
      NumericalExpression ne;
      if (ne.Calc(&egs.StringText,&d))
      {
        FSF.sprintf(result,L"%f",d);
        size_t i = wcslen(result)-1;
        while (i>0 && result[i] == L'0')
        {
          result[i--] = 0;
        }
        if (result[i] == L'.')
          result[i] = 0;
        r=result;
      }
    }
    else
    {
      long long n=0;
      LogicalExpression le;
      if (le.Calc(&egs.StringText,&n))
      {
        if (*egs.StringText == L'=')
        {
          egs.StringText++;
          if (*egs.StringText == L'd' || *egs.StringText == L'D')
            FSF.sprintf(result,L"%I64d",n);
          else if (*egs.StringText == L'u' || *egs.StringText == L'U')
            FSF.sprintf(result,L"%I64u",n);
          else if (*egs.StringText == L'4')
            FSF.sprintf(result,L"%u.%u.%u.%u", (int)((n&0xFF000000ull)>>24) , (int)((n&0xFF0000ull)>>16), (int)((n&0xFF00ull)>>8), (int)((n&0xFFull)));
          else
            FSF.sprintf(result,L"0x%I64X",n);
        }
        else
        {
          FSF.sprintf(result,L"0x%I64X",n);
        }
        r=result;
      }
    }
    Info.EditorControl(-1,ECTL_INSERTTEXT,0,r);
  }

  return INVALID_HANDLE_VALUE;
}

#if defined(__GNUC__)

#ifdef __cplusplus
extern "C"{
#endif
void __cxa_pure_virtual(void);
#ifdef __cplusplus
};
#endif

void __cxa_pure_virtual(void)
{
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
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}

#endif
