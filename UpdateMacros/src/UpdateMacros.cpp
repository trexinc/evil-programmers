/*
    UpdateMacros plugin for FAR Manager
    Copyright (C) 2001-2005 Alex Yaroslavsky

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

void *memcpy(void * dst, const void * src, size_t count)
{
  void * ret = dst;

  while (count--)
  {
    *(char *)dst = *(char *)src;
    dst = (char *)dst + 1;
    src = (char *)src + 1;
  }
  return(ret);
}

enum
{
  MTitle,
  MSuccessful,
  MUnsuccessful
};

struct PluginStartupInfo Info;

char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *psi)
{
  Info=*psi;
}

void WINAPI _export GetPluginInfo(struct PluginInfo *pi)
{
  static char *PluginMenuStrings[1];

  pi->StructSize=sizeof(struct PluginInfo);
  pi->Flags=PF_EDITOR|PF_VIEWER;
  PluginMenuStrings[0]=GetMsg(MTitle);
  pi->PluginMenuStrings=PluginMenuStrings;
  pi->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int item)
{
  const char *Msg[2];
  Msg[0]=GetMsg(MTitle);
  struct ActlKeyMacro km;
  km.Command=MCMD_LOADALL;
  km.Reserved[0]=km.Reserved[1]=km.Reserved[2]=0;
  if (Info.AdvControl(Info.ModuleNumber,ACTL_KEYMACRO, &km))
  {
    Msg[1]=GetMsg(MSuccessful);
    Info.Message(Info.ModuleNumber,FMSG_MB_OK,":Messages",Msg,2,0);
  }
  else
  {
    Msg[1]=GetMsg(MUnsuccessful);
    Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_MB_OK,":Messages",Msg,2,0);
  }
  return INVALID_HANDLE_VALUE;
}
