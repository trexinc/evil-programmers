/*
    Block Indent plugin for FAR Manager
    Copyright (C) 2001-2004 Alex Yaroslavsky

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


static struct PluginStartupInfo Info;
//static FARSTANDARDFUNCTIONS FSF;

enum
{
  MTitle,
  MIndentTabLeft,
  MIndentTabRight,
  MIndentSpaceLeft,
  MIndentSpaceRight,
};

void *memcpy(void *dst, const void *src, size_t count)
{
  void *ret = dst;

  while (count--)
  {
    *(char *)dst = *(char *)src;
    dst = (char *)dst + 1;
    src = (char *)src + 1;
  }
  return (ret);
}

char *strcpy(char *dst, const char *src)
{
  char *cp = dst;

  while(*cp++ = *src++)
    ;

  return (dst);
}

/*
inline int IsWhite(unsigned char c)
{
  return ((c == 32) || (c == 9))?TRUE:FALSE;
}
*/

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *psi)
{
  Info=*psi;
  //FSF=*psi->FSF;
  //Info.FSF=&FSF;
}

void WINAPI _export GetPluginInfo(struct PluginInfo *pi)
{
  static char *PluginMenuStrings[1];

  pi->StructSize=sizeof(struct PluginInfo);
  pi->Flags=PF_EDITOR|PF_DISABLEPANELS;
  PluginMenuStrings[0]=Info.GetMsg(Info.ModuleNumber,MTitle);
  pi->PluginMenuStrings=PluginMenuStrings;
  pi->PluginMenuStringsNumber=1;
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom, int Item)
{
  struct FarMenuItem MenuItems[4];
  {
    //char s[3];
    //s[0] = '%'; s[1] = 's'; s[2] = 0;
    for (int i=0; i<4; i++)
    {
      MenuItems[i].Checked = 0;
      MenuItems[i].Separator = 0;
      MenuItems[i].Selected = 0;
      //FSF.sprintf(MenuItems[i].Text,s,Info.GetMsg(Info.ModuleNumber,MIndentTabLeft+i));
      strcpy(MenuItems[i].Text,Info.GetMsg(Info.ModuleNumber,MIndentTabLeft+i));
    }
    MenuItems[0].Selected = 1;
  }

  int menu;
  if ((menu = Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_WRAPMODE,Info.GetMsg(Info.ModuleNumber,MTitle),
      NULL,NULL,NULL,NULL,MenuItems,sizeof(MenuItems)/sizeof(MenuItems[0]))) == -1)
    return INVALID_HANDLE_VALUE;

  struct EditorInfo ei;
  Info.EditorControl(ECTL_GETINFO,&ei);

  /*
  unsigned char Space = 32;
  unsigned char Tab = 9;
  if (ei.TableNum != -1)
  {
    struct CharTableSet cts;
    if (Info.CharTable(ei.TableNum,(const char *)&cts,sizeof(struct CharTableSet)) != -1) {
        Space = cts.EncodeTable[Space];
        Tab = cts.EncodeTable[Tab];
    }
  }
  */

  char IndentStr[2];
  IndentStr[0] = menu<2?9:32;
  IndentStr[1] = '\0';
  int IndentSize = menu<2?ei.TabSize:1;

  int line = ei.CurLine;
  int loop = FALSE;
  if (ei.BlockType != BTYPE_NONE)
  {
    struct EditorGetString egs;
    egs.StringNumber = -1;
    Info.EditorControl(ECTL_GETSTRING,&egs);
    if (egs.SelStart != -1)
    {
      loop = TRUE;
      line = ei.BlockStartLine;
    }
  }

  do
  {
    struct EditorGetString egs;
    if (line < ei.TotalLines)
    {
      struct EditorSetPosition esp;
      esp.CurLine = line++;
      esp.CurPos = esp.Overtype = 0;
      esp.CurTabPos = esp.TopScreenLine = esp.LeftPos = -1;
      Info.EditorControl(ECTL_SETPOSITION,&esp);
      egs.StringNumber = -1;
      Info.EditorControl(ECTL_GETSTRING,&egs);
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
      int x;
      {
        struct EditorConvertPos ecp;
        ecp.StringNumber = -1;
        ecp.SrcPos = j+1;
        Info.EditorControl(ECTL_REALTOTAB,&ecp);
        x = (--ecp.DestPos)/IndentSize;
        if (!(ecp.DestPos%IndentSize) && !(menu%2))
          x--;
        if (menu%2 == 1)
          x++;
      }
      {
        struct EditorSetString ess;
        ess.StringNumber = -1;
        ess.StringText = &egs.StringText[j];
        ess.StringEOL = egs.StringEOL;
        ess.StringLength = egs.StringLength - j;
        Info.EditorControl(ECTL_SETSTRING,&ess);
      }
      if (x)
      {
        for (int i=0; i<x; i++)
          Info.EditorControl(ECTL_INSERTTEXT,IndentStr);
      }
    }
  } while (loop);

  {
    struct EditorSetPosition esp;
    esp.CurLine = ei.CurLine;
    esp.CurPos = ei.CurPos;
    esp.TopScreenLine = ei.TopScreenLine;
    esp.LeftPos = ei.LeftPos;
    esp.Overtype = ei.Overtype;
    esp.CurTabPos = -1;
    Info.EditorControl(ECTL_SETPOSITION,&esp);
  }

  return INVALID_HANDLE_VALUE;
}
