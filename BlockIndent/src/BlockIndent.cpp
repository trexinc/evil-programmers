/*
    Block Indent plugin for FAR Manager
    Copyright (C) 2001 Alex Yaroslavsky

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

// {D7302F8C-C73D-4143-9B39-B439CF2E7ACC}
static const GUID MainGuid =
{ 0xd7302f8c, 0xc73d, 0x4143, { 0x9b, 0x39, 0xb4, 0x39, 0xcf, 0x2e, 0x7a, 0xcc } };

// {764D3DB5-0DF3-4E51-8694-7B0DCBADE07D}
static const GUID MenuGuid =
{ 0x764d3db5, 0xdf3, 0x4e51, { 0x86, 0x94, 0x7b, 0xd, 0xcb, 0xad, 0xe0, 0x7d } };


static struct PluginStartupInfo Info;

enum
{
  MTitle,
  MIndentTabLeft,
  MIndentTabRight,
  MIndentSpaceLeft,
  MIndentSpaceRight,
};

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

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
}

const wchar_t *GetMsg(intptr_t MsgId)
{
  return Info.GetMsg(&MainGuid,MsgId);
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
  static const wchar_t *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MTitle);
  Info->PluginMenu.Guids=&MenuGuid;
  Info->PluginMenu.Strings=PluginMenuStrings;
  Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
}

HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
  struct FarMenuItem MenuItems[4] = {};
  {
    for (int i=0; i<4; i++)
    {
      MenuItems[i].Text = GetMsg(MIndentTabLeft+i);
    }
    MenuItems[0].Flags = MIF_SELECTED;
  }

  intptr_t menu;
  if ((menu = Info.Menu(&MainGuid,NULL,-1,-1,0,FMENU_WRAPMODE,GetMsg(MTitle),
      NULL,NULL,NULL,NULL,MenuItems,sizeof(MenuItems)/sizeof(MenuItems[0]))) == -1)
    return NULL;

  struct EditorInfo ei;
  ei.StructSize = sizeof(ei);
  Info.EditorControl(-1,ECTL_GETINFO,0,&ei);

  wchar_t IndentStr[2];
  IndentStr[0] = menu<2?'\t':' ';
  IndentStr[1] = '\0';
  intptr_t IndentSize = menu<2?ei.TabSize:1;

  intptr_t line = ei.CurLine;
  bool loop = false;
  if (ei.BlockType != BTYPE_NONE)
  {
    struct EditorGetString egs;
    egs.StructSize = sizeof(egs);
    egs.StringNumber = -1;
    Info.EditorControl(-1,ECTL_GETSTRING,0,&egs);
    if (egs.SelStart != -1)
    {
      loop = true;
      line = ei.BlockStartLine;
    }
  }

  struct EditorUndoRedo eur;
  eur.StructSize = sizeof(eur);
  eur.Command=EUR_BEGIN;
  Info.EditorControl(-1,ECTL_UNDOREDO,0,&eur);

  do
  {
    struct EditorGetString egs;
    egs.StructSize = sizeof(egs);
    if (line < ei.TotalLines)
    {
      struct EditorSetPosition esp;
      esp.StructSize = sizeof(esp);
      esp.CurLine = line++;
      esp.CurPos = esp.Overtype = 0;
      esp.CurTabPos = esp.TopScreenLine = esp.LeftPos = -1;
      Info.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
      egs.StringNumber = -1;
      Info.EditorControl(-1,ECTL_GETSTRING,0,&egs);
      if (loop && ((egs.SelStart == -1) || (egs.SelStart == egs.SelEnd)))
        break;
    }
    else
      break;
    int j = 0;
    while ((egs.StringText[j]==9 || egs.StringText[j]==32) && (j < egs.StringLength))
      j++;
    if ((j || (menu%2 == 1)) && (j < egs.StringLength))
    {
      intptr_t x;
      {
        struct EditorConvertPos ecp;
        ecp.StructSize = sizeof(ecp);
        ecp.StringNumber = -1;
        ecp.SrcPos = j+1;
        Info.EditorControl(-1,ECTL_REALTOTAB,0,&ecp);
        x = (--ecp.DestPos)/IndentSize;
        if (!(ecp.DestPos%IndentSize) && !(menu%2))
          x--;
        if (menu%2 == 1)
          x++;
      }
      {
        struct EditorSetString ess;
        ess.StructSize = sizeof(ess);
        ess.StringNumber = -1;
        ess.StringText = (wchar_t *)&egs.StringText[j];
        ess.StringEOL = (wchar_t *)egs.StringEOL;
        ess.StringLength = egs.StringLength - j;
        Info.EditorControl(-1,ECTL_SETSTRING,0,&ess);
      }
      if (x)
      {
        for (int i=0; i<x; i++)
          Info.EditorControl(-1,ECTL_INSERTTEXT,0,IndentStr);
      }
    }
  } while (loop);

  {
    struct EditorSetPosition esp;
    esp.StructSize = sizeof(esp);
    esp.CurLine = ei.CurLine;
    esp.CurPos = ei.CurPos;
    esp.TopScreenLine = ei.TopScreenLine;
    esp.LeftPos = ei.LeftPos;
    esp.Overtype = ei.Overtype;
    esp.CurTabPos = -1;
    Info.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
  }

  eur.Command=EUR_END;
  Info.EditorControl(-1,ECTL_UNDOREDO,0,&eur);

  return NULL;
}
