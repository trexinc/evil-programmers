/*
    Helper sub-plugin for FARMail
    Copyright (C) 2002-2004 FARMail Group

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
#include "helper.hpp"

void *memset(void *s,int c,size_t n)
{
  BYTE *dst=(BYTE *)s;
  while(n--)
  {
    *dst=(BYTE)(unsigned)c;
    ++dst;
  }
  return s;
}

char *strchr(register const char *s,int c)
{
  do
  {
    if(*s==c)
    {
      return (char*)s;
    }
  } while (*s++);
  return (0);
}

int get_line_count(void)
{
  EditorInfo ei;
  FarInfo.EditorControl(ECTL_GETINFO,&ei);
  return ei.TotalLines;
}

int get_current_line(void)
{
  EditorInfo ei;
  FarInfo.EditorControl(ECTL_GETINFO,&ei);
  return ei.CurLine;
}

void set_current_line(int row,int col)
{
  EditorSetPosition esp;
  esp.CurLine=row;
  esp.CurPos=col;
  esp.TopScreenLine=-1;
  esp.LeftPos=-1;
  esp.CurTabPos=-1;
  esp.Overtype=-1;
  FarInfo.EditorControl(ECTL_SETPOSITION,&esp);
}

void add_blank(void)
{
  int count=get_line_count(),len;
  if(get_line(count-1,NULL,&len))
  {
    set_current_line(count-1,len);
    FarInfo.EditorControl(ECTL_INSERTSTRING,NULL);
  }
}

void insert_string(const char *string,int row)
{
  int count=get_line_count();
  if(row<count)
  {
    set_current_line(row,0);
    FarInfo.EditorControl(ECTL_INSERTSTRING,NULL);
  }
  else
    for(int i=0;i<=(row-count);i++)
      add_blank();
  set_current_line(row,0);
  FarInfo.EditorControl(ECTL_INSERTTEXT,(void *)string);
}

bool get_line(int lno,const char **str,int *len)
{
  if(lno>=get_line_count()) return false;
  EditorGetString egs;
  EditorSetPosition esp;
  esp.CurPos=-1; esp.CurTabPos=-1; esp.TopScreenLine=-1; esp.LeftPos=-1; esp.Overtype=-1;
  egs.StringNumber=-1;
  esp.CurLine=lno;
  if(!FarInfo.EditorControl(ECTL_SETPOSITION,&esp)) return false;
  if(!FarInfo.EditorControl(ECTL_GETSTRING,&egs)) return false;
  if(len) *len=egs.StringLength;
  if(str) *str=egs.StringText;
  return true;
}
