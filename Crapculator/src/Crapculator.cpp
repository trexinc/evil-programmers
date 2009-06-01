/*
    Crapculator plugin for FAR Manager
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

#include "plugin.hpp"

bool Expression(const wchar_t **p, double *n, int pa, int *b);
bool LExpression(const wchar_t **p, long long *n, int pa, int *b);

struct PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *psi)
{
  Info=*psi;
  FSF=*psi->FSF;
  Info.FSF=&FSF;
}

void WINAPI GetPluginInfoW(struct PluginInfo *pi)
{
  static const wchar_t *MenuStrings[2] = {L"Crapculator", L"Hackulator"};

  pi->StructSize=sizeof(struct PluginInfo);
  pi->Flags=PF_EDITOR|PF_DISABLEPANELS;
  pi->PluginMenuStrings=MenuStrings;
  pi->PluginMenuStringsNumber=2;
}

HANDLE WINAPI OpenPluginW(int OpenFrom, INT_PTR Item)
{
  int b=0;
  double d=0;
  long long n=0;
  wchar_t result[200];
  wchar_t error[] = L"Crap";
  wchar_t *r=error;

  struct EditorGetString egs = {-1, NULL, NULL, 0, 0, 0};

  if (Info.EditorControl(ECTL_GETSTRING,(void *)&egs) && egs.StringText)
  {
    if (Item == 0)
    {
      if (Expression(&egs.StringText,&d,0,&b) && !b)
      {
        FSF.sprintf(result,L"%f",d);
        r=result;
      }
    }
    else
    {
      if (LExpression(&egs.StringText,&n,0,&b) && !b)
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
    Info.EditorControl(ECTL_INSERTTEXT,(void *)r);
  }

  return INVALID_HANDLE_VALUE;
}

#if defined(__GNUC__)
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
