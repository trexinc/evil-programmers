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
#ifndef __KeySequenceStorage_cpp
#define __KeySequenceStorage_cpp

#include "myrtl.hpp"
#include "KeySequenceStorage.hpp"
#include "kserror.hpp"
#include "syslog.hpp"

extern FarStandardFunctions FSF;

KeySequenceStorage::KeySequenceStorage(const wchar_t *str,bool silent,DWORD total,
                                       BOOL stop)
{
  Init();
  if(str)
  {
    int error;
    strcon unknownKey;
    compile(str,silent,total,stop,error,unknownKey);
  }
}

KeySequenceStorage::KeySequenceStorage(const KeySequenceStorage& Value)
{
  Init();
  Copy(Value);
}

void KeySequenceStorage::Init()
{
  Stop=TRUE;
  Sequence.Sequence=NULL;
  Free();
}

KeySequenceStorage& KeySequenceStorage::operator=(const KeySequenceStorage& Value)
{
  if(this!=&Value)
  {
    Free();
    Copy(Value);
  }
  return *this;
}

void KeySequenceStorage::Free()
{
  if(Sequence.Sequence)
  {
    free(Sequence.Sequence);
    Sequence.Sequence=NULL;
  }
  Sequence.Count=0;
  Sequence.Flags=0;
}

void KeySequenceStorage::Copy(const KeySequenceStorage& Value)
{
  Sequence.Flags=Value.Sequence.Flags;
  if(Value.Sequence.Sequence)
  {
    Sequence.Sequence=static_cast<DWORD *>(malloc((Value.Sequence.Count+1)*sizeof(wchar_t)));

    if(Sequence.Sequence)
    {
      Stop=Value.Stop;
      Sequence.Count=Value.Sequence.Count;
      memcpy(Sequence.Sequence,Value.Sequence.Sequence,(Value.Sequence.Count+1)*sizeof(wchar_t));
    }
  }
}

bool KeySequenceStorage::compile(const wchar_t *BufPtr, bool silent, DWORD total,
                                 BOOL stop,int &Error,strcon &unknownKey)
{
  _D(SysLog(L"compile: start total=%d, (unknownKey='%s') [%s]",
     total, unknownKey.str?unknownKey.str:L"(null)", BufPtr?BufPtr:L"(null)"));
  Free();
  Error=KSE_SUCCESS;
  unknownKey=L"";
  if(!BufPtr || !*BufPtr)
  {
    _D(SysLog(L"compile: error Buf (unknownKey='%s')", unknownKey.str?unknownKey.str:L"(null)"));
    return false;
  }
  if(silent) Sequence.Flags=KSFLAGS_DISABLEOUTPUT;
  Stop=stop;

  DWORD tmpSize=wstrlen(BufPtr);
  Sequence.Count=0;
  Sequence.Sequence=static_cast<DWORD *>(malloc(((tmpSize+1)*total+1)*sizeof(wchar_t)));
  _D(SysLog(L"compile: new size=%d", (tmpSize+1)*total+1));

  if(Sequence.Sequence)
  {
    for(DWORD I=1;I<=total;++I)
    {
      memcpy(reinterpret_cast<wchar_t *>(Sequence.Sequence)+Sequence.Count,BufPtr,tmpSize*sizeof(wchar_t));
      if(I<total)
      {
        *(reinterpret_cast<wchar_t *>(Sequence.Sequence)+Sequence.Count+tmpSize)=0x20;
        ++Sequence.Count;
      }
      Sequence.Count+=tmpSize;
    }
    *(reinterpret_cast<wchar_t *>(Sequence.Sequence)+Sequence.Count)=0;
    _D(SysLog(L"compile: compile newmacro [%s])", Sequence.Sequence));
    return true;
  }

  Free();
  Error=KSE_ALLOC;
  _D(SysLog(L"compile: error newmacro total alloc (unknownKey='%s')", unknownKey.str?unknownKey.str:L"(null)"));
  return false;
}

#endif // __KeySequenceStorage_cpp
