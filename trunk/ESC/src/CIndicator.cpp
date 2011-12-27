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
#ifndef __CIndicator_cpp
#define __CIndicator_cpp

#ifdef BarSize
#error BarSize
#endif

#include <CRT/crt.hpp>
#include "plugin.hpp"
#include "CIndicator.hpp"
#include "guid.h"

extern PluginStartupInfo Info;
extern FARAPIMESSAGE FarMessage;

CIndicator::CIndicator(const wchar_t *BarTitle)
{
  if(BarTitle)
  {
    Items=3;
    MsgData[0] = BarTitle;
    MsgData[1] = L"";
    MsgData[2] = Bar;
  }
  else
  {
    Items=2;
    MsgData[0] = L"";
    MsgData[1] = Bar;
  }
  Bar[BarSize] = 0;
  StartTimer(0x7FFFFFFF);
}

CIndicator::~CIndicator()
{
}

void CIndicator::ShowFinal(void)
{
  if (CheckTimer())
    {
      ptr = Bar + (Counter = CurPos);
      while (Counter < BarSize)
        {
          *ptr = 0x2588;           //'Û'
          ++ptr;
          ++Counter;
        }
      FarMessage(&MainGuid, &IndicatorGuid, FMSG_LEFTALIGN, NULL, MsgData, Items, 0);
    }
}

void CIndicator::ShowProgress(int Cur)
{
  if (CheckTimer())
    {
      Procent = (Cur + 1) * 100 / (Total + 1);
      NewProcent = Procent * BarSize / 100;
      if (NewProcent > BarSize)
        NewProcent = BarSize;
      if (NewProcent > OldProcent)
        {
          OldProcent = NewProcent;
          ptr = Bar + (Counter = CurPos);
          while (Counter < NewProcent)
            {
              *ptr = 0x2588;       //'Û'
              ++ptr;
              ++Counter;
            }
          CurPos = Counter;
          FarMessage(&MainGuid, &IndicatorGuid, FMSG_LEFTALIGN, NULL, MsgData, Items,
                       0);
        }
    }
}

void CIndicator::SetParams(const wchar_t *Title, int total)
{
  if (Title != NULL)
    MsgData[Items-2] = Title;
  Total = total;
}

void CIndicator::StartTimer(DWORD ms)
{
  Period.b=GetTickCount();
  Period.Period = ms;
  OldProcent = -1;
  CurPos = Counter = 0;
  ptr = Bar;
  while (Counter < BarSize)
    {
      *ptr = 0x2591;               //'°'
      ++ptr;
      ++Counter;
    }
}

BOOL CIndicator::CheckTimer()
{
  BOOL res;

  Period.e=GetTickCount();
  Period.LastDiff = Period.e - Period.b;
  res = (Period.LastDiff >= Period.Period);
  if (res)
    Period.b = Period.e;
  return res;
}

#endif // __CIndicator_cpp
