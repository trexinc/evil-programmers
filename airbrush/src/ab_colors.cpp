/*
    ab_colors.cpp
    Copyright (C) 2014 zg

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

#include <algorithm>
#include <stdio.h>
#include "far_settings.h"
#include "abplugin.h"
#include "ab_main.h"
#include "guid.h"

class CColors
{
  public:
    template<typename T,typename S> CColors(CFarSettings& Settings,const wchar_t* Name,T Colors,S& Count): settings(Settings)
    {
      settings.Root();
      settings.Change(PLUGIN_COLOR_KEY);
      settings.Change(Name);
      C(Count);
      for(ii=0;ii<Count;++ii)
      {
        _(L"Flags%d",Colors[ii].Flags);
        _(L"ForegroundColor%d",Colors[ii].ForegroundColor);
        _(L"BackgroundColor%d",Colors[ii].BackgroundColor);
        _(L"ForegroundDefault%d",Colors[ii].ForegroundDefault);
        _(L"BackgroundDefault%d",Colors[ii].BackgroundDefault);
      }
    }
  private:
    template<typename T> void _(const wchar_t* Name,T& Value)
    {
      wchar_t keyName[64];
      wsprintfW(keyName,Name,ii);
      P(keyName,Value);
    }
    template<typename T> void P(const wchar_t* Name,T& Value)
    {
      Value=settings.Get(Name,static_cast<__int64>(Value));
    }
    template<typename T> void P(const wchar_t* Name,const T& Value)
    {
      settings.Set(Name,Value);
    }
    template<typename T> void C(T& Count)
    {
      Count=std::min(static_cast<size_t>(settings.Get(L"Count")),Count);
    }
    template<typename T> void C(const T& Count)
    {
      settings.Set(L"Count",Count);
    }
  private:
    CFarSettings& settings;
    size_t ii;
};


void SaveColors(const wchar_t* Name,const ABColor* Colors,const size_t Count)
{
  CFarSettings settings(MainGuid);
  CColors colors(settings,Name,Colors,Count);
}

void LoadColors(CFarSettings& Settings,const wchar_t* Name,ABColor* Colors,size_t Count)
{
  CColors colors(Settings,Name,Colors,Count);
}
