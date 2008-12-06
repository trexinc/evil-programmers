/*
    [ESC] Editor's settings changer plugin for FAR Manager
    Copyright (C) 2001 Ivan Sintyurin
    Copyright (C) 2008 Alex Yaroslavsky

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
#ifndef __options_hpp
#define __options_hpp

#include "strcon.hpp"

struct OPTIONS
{
  int TurnOnPluginModule, ReloadSettingsAutomatically, ShowFileMaskInMenu;
};

struct ESCFileInfo
{
  strcon   Name;
  FILETIME Time;
  DWORD    SizeLow, SizeHigh;
  ESCFileInfo():SizeLow(0), SizeHigh(0)
  {
    memset(&Time,0,sizeof(Time));
  }
  ESCFileInfo(const wchar_t *name):Name(name), SizeLow(0), SizeHigh(0)
  {
    memset(&Time,0,sizeof(Time));
  }
};

BOOL WINAPI ESCFICompLT(const ESCFileInfo &a,const ESCFileInfo &b);
BOOL WINAPI ESCFICompEQ(const ESCFileInfo &a,const ESCFileInfo &b);

#endif // __options_hpp
