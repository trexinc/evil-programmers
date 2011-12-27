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
#ifndef __KeySequenceStorage_hpp
#define __KeySequenceStorage_hpp

#include "plugin.hpp"
#include "strcon.hpp"

enum KeySequenceStorageFlags
{
  KSSF_SELECTION_ON     = 0x01,
  KSSF_SELECTION_OFF    = 0x02,
  KSSF_SELECTION_STREAM = 0x04,
  KSSF_SELECTION_COLUMN = 0x08,
};

struct KeySequence
{
  unsigned __int64 Flags;
  int Count;
  wchar_t* Sequence;
};

class KeySequenceStorage
{
  private:
    enum { KEY_MACRODATE = 0x00800001 };
    KeySequence Sequence;
    BOOL Stop;
    void Copy(const KeySequenceStorage& Value);
    void Init();
    inline int IsSpace(wchar_t x) { return x==0x20||x==L'\t'||x==L'\r'||x==L'\n';}
  public:
    KeySequenceStorage(const wchar_t *str=NULL, bool silent=true, DWORD total=1, BOOL Stop=FALSE);
    KeySequenceStorage(const KeySequenceStorage& Value);
    ~KeySequenceStorage() { Free(); }
    const KeySequence& GetSequence(BOOL &stop) const
    {
      stop=Stop;
      return Sequence;
    }
    bool compile(const wchar_t *str, bool silent,DWORD total,BOOL stop,
                 int &Error, strcon &unknownKey);
    bool IsOK() const { return Sequence.Sequence!=NULL; }
    void Free();
    KeySequenceStorage& operator=(const KeySequenceStorage& Value);
};

#endif // __KeySequenceStorage_hpp
