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
#ifndef __SaveAndLoadConfig
#define __SaveAndLoadConfig

#include <CRT/crt.hpp>
#include <windows.h>
#include "SaveAndLoadConfig.hpp"
#include "options.hpp"
#include "plugin.hpp"
#include "syslog.hpp"
#include "guid.h"

extern OPTIONS Opt;
extern PluginStartupInfo Info;
const wchar_t *REGStrOptions=L"Options";

enum
{
  O_TurnOnPluginModule          = 0x00000001,
  O_ReloadSettingsAutomatically = 0x00000002,
  O_ShowFileMaskInMenu          = 0x00000004,
};

void LoadGlobalConfig() // грузится здесь, а сохраняется в настройках
{
  __int64 Options=O_TurnOnPluginModule|O_ReloadSettingsAutomatically|O_ShowFileMaskInMenu;
  FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
  HANDLE Settings=Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings)?settings.Handle:0;
  if(Settings)
  {
    FarSettingsItem item={sizeof(FarSettingsItem),0,REGStrOptions,FST_QWORD,{0}};
    if(Info.SettingsControl(Settings,SCTL_GET,0,&item))
    {
      Options=item.Number;
    }
    Info.SettingsControl(Settings,SCTL_FREE,0,0);
  }
  Opt.TurnOnPluginModule =(Options&O_TurnOnPluginModule)?TRUE:FALSE;
  Opt.ReloadSettingsAutomatically=(Options&O_ReloadSettingsAutomatically)?TRUE:FALSE;
  Opt.ShowFileMaskInMenu=(Options&O_ShowFileMaskInMenu)?TRUE:FALSE;
}

void SaveGlobalConfig()
{
  __int64 Options=0;
  if(Opt.TurnOnPluginModule)
     Options|=O_TurnOnPluginModule;
  if(Opt.ReloadSettingsAutomatically)
     Options|=O_ReloadSettingsAutomatically;
  if(Opt.ShowFileMaskInMenu)
     Options|=O_ShowFileMaskInMenu;
  FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
  HANDLE Settings=Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings)?settings.Handle:0;
  if(Settings)
  {
    FarSettingsItem item={sizeof(FarSettingsItem),0,REGStrOptions,FST_QWORD,{0}};
    item.Number=Options;
    Info.SettingsControl(Settings,SCTL_SET,0,&item);
    Info.SettingsControl(Settings,SCTL_FREE,0,0);
  }
}

#endif //__SaveAndLoadConfig
