/*
    Active-Help plugin for FAR Manager
    Copyright (C) 2002-2005 Alex Yaroslavsky

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
#include "Active-Help.hpp"
#include "ahp_internal.hpp"

void WINAPI api_get_message(const char *file_name,int index,char *message)
{
  static char language[512];
  static char buffer[32*1024-1];
  *message=0;
  GetLanguage(language,sizeof(language));
  if(GetPrivateProfileSection(language,buffer,sizeof(buffer),file_name))
  {
    char *ptr=buffer;
    while(index>=0&&lstrlen(ptr))
    {
      if(!index) lstrcpy(message,ptr);
      index--;
      ptr=ptr+lstrlen(ptr)+1;
    }
  }
}

long WINAPI api_show_help_dlg_proc(HANDLE hDlg, int Msg,int Param1,long Param2)
{
  static char *ModuleName;

  switch (Msg)
  {
    case DN_INITDIALOG:
      ModuleName = (char *) Param2;
      return FALSE;

    case DN_HELP:
      Info.ShowHelp(ModuleName,(char *) Param2,FHELP_SELFHELP);
      return (long)NULL;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

int WINAPI api_show_help_menu(
  const char *ModuleName,
  int PluginNumber,
  int X, int Y,
  int MaxHeight,
  unsigned int Flags,
  const char *Title,
  const char *Bottom,
  const char *HelpTopic,
  const int *BreakKeys,
  int *BreakCode,
  const struct FarMenuItem *Item,
  int ItemsNumber)
{
  int breakcode;
  const int breakkeys[] = {VK_F1, 0};
  if (!BreakKeys)
  {
    BreakKeys = breakkeys;
    BreakCode = &breakcode;
  }
  int menucode;
  for(;;)
  {
    menucode = Info.Menu(PluginNumber, X, Y, MaxHeight, Flags, Title, Bottom, HelpTopic, BreakKeys, BreakCode, Item, ItemsNumber);
    if (menucode >= 0 && *BreakCode == 0)
    {
      Info.ShowHelp(ModuleName,HelpTopic,FHELP_SELFHELP);
      continue;
    }
    else
      break;
  }
  return menucode;
}

BOOL WINAPI api_encode(char *str, int len, const char *encoding)
{
  return (Encode(str,len,encoding)==true?TRUE:FALSE);
}

BOOL WINAPI api_decode(char *str, int len, const char *encoding)
{
  return (Decode(str,len,encoding)==true?TRUE:FALSE);
}
