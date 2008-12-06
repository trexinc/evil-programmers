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
#ifndef __e_options_cpp
#define __e_options_cpp

#include "e_options.hpp"

NODEDATA::NODEDATA():
     AddSym_S(L""), AddSym_E(L""), AddSym_S_B(L""), AddSym_E_B(L""),
     TabSize(0),WrapPos(0),Table(0),MinLinesNum(0),EOL(0),Options(0),Options2(0),Lines(0),
     AdditionalLetters(L""),
     TableNum(-1), TableName(L""), LockFile(NULL), StopChar(L"")
{
}

NODEDATA::NODEDATA(const NODEDATA& rhs):LockFile(NULL)
{
  *this=rhs;
}

bool NODEDATA::operator==(const NODEDATA &rhs) const
{
  return Name==rhs.Name;
}

bool NODEDATA::operator<(const NODEDATA &rhs) const
{
  return wstricmp(Name.str, rhs.Name.str)<0;
}

NODEDATA& NODEDATA::operator=(const NODEDATA &rhs)
{
  if(this!=&rhs)
  {
    Name=rhs.Name;
    mask=rhs.mask;
    AddSym_S=rhs.AddSym_S;
    AddSym_E=rhs.AddSym_E;
    AddSym_S_B=rhs.AddSym_S_B;
    AddSym_E_B=rhs.AddSym_E_B;
    TabSize=rhs.TabSize;
    WrapPos=rhs.WrapPos;
    Table=rhs.Table;
    MinLinesNum=rhs.MinLinesNum;
    EOL=rhs.EOL;
    Options=rhs.Options;
    Options2=rhs.Options2;
    Lines=rhs.Lines;
    TableNum=rhs.TableNum;
    TableName=rhs.TableName;
    KeyMacros=rhs.KeyMacros;
    MouseMacros=rhs.MouseMacros;
    OnCreateMacros=rhs.OnCreateMacros;
    OnLoadMacros=rhs.OnLoadMacros;
    LockFile=rhs.LockFile;
    StopChar=rhs.StopChar;
    AdditionalLetters=rhs.AdditionalLetters;
  }
  return *this;
}

void NODEDATA::Inherit(NODEDATA &Dest)
{
  Dest.AddSym_S    = AddSym_S;
  Dest.AddSym_E    = AddSym_E;
  Dest.AddSym_S_B  = AddSym_S_B;
  Dest.AddSym_E_B  = AddSym_E_B;
  Dest.TableName   = TableName;
  Dest.TabSize     = TabSize;
  Dest.WrapPos     = WrapPos;
  Dest.Table       = Table;
  Dest.MinLinesNum = MinLinesNum;
  Dest.EOL         = EOL;
  Dest.Options     = Options;
  Dest.Options2    = Options2;
  Dest.Lines       = Lines;
  Dest.KeyMacros   = KeyMacros;
  Dest.MouseMacros = MouseMacros;
  Dest.OnCreateMacros = OnCreateMacros;
  Dest.OnLoadMacros   = OnLoadMacros;
  Dest.StopChar    = StopChar;
  Dest.AdditionalLetters = AdditionalLetters;
}

#endif // __e_options_cpp
