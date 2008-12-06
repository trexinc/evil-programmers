#ifndef __e_options_cpp
#define __e_options_cpp

#include "e_options.hpp"

NODEDATA::NODEDATA():
     AddSym_S(""), AddSym_E(""), AddSym_S_B(""), AddSym_E_B(""),
     TabSize(0),WrapPos(0),Table(0),MinLinesNum(0),EOL(0),Options(0),Options2(0),Lines(0),
     AdditionalLetters(""),
     TableNum(-1), AnsiMode(-1), TableName(""), LockFile(NULL), StopChar("")
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
  return stricmp(Name.str, rhs.Name.str)<0;
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
    AnsiMode=rhs.AnsiMode;
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
