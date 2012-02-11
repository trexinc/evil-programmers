/*
    far_settings.hpp
    Copyright (C) 2011 zg

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

#ifndef __FAR_SETTINGS_HPP__
#define __FAR_SETTINGS_HPP__

#include <tchar.h>
#include "plugin.hpp"
extern PluginStartupInfo Info;

class CFarSettings
{
  private:
    HANDLE iSettings;
    size_t iRoot;
    CFarSettings();
  public:
    CFarSettings(const GUID& PluginId);
    ~CFarSettings();
    void Change(const wchar_t* aName);
    void Set(const wchar_t* aName,__int64 aValue);
    void Set(const wchar_t* aName,const wchar_t* aValue);
    __int64 Get(const wchar_t* aName,__int64 aDefault=0);
    bool Get(const wchar_t* aName,wchar_t* aValue,size_t aSize);
};

#endif
