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
#ifndef __CIndicator_hpp
#define __CIndicator_hpp

class CIndicator
{
private:
  enum {BarSize=40};
  struct structPeriod
  {
    DWORD b, e;
    DWORD Period;
    DWORD LastDiff;
  }
  Period;
  const wchar_t *MsgData[3];
  wchar_t Bar[BarSize + 1], *ptr;
  int Procent, NewProcent, OldProcent, Counter, CurPos, Items;
  intptr_t Total;

  BOOL CheckTimer();

public:
    CIndicator(const wchar_t *BarTitle);
   ~CIndicator();
  void ShowProgress(intptr_t Cur);
  void ShowFinal(void);
  void StartTimer(DWORD ms);    // вызывается самым первым
  void SetParams(const wchar_t *Title, intptr_t total); // вызывается вторым

private:
  CIndicator& operator=(const CIndicator& rhs); /* чтобы не */
  CIndicator(const CIndicator& rhs); /* генерировалось по умолчанию */

};

#endif // __CIndicator_hpp
