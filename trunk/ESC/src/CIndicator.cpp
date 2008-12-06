#ifndef __CIndicator_cpp
#define __CIndicator_cpp

#ifdef BarSize
#error BarSize
#endif

#include "myrtl.hpp"
#include "plugin.hpp"
#include "CIndicator.hpp"

extern PluginStartupInfo Info;
extern FARAPIMESSAGE FarMessage;
extern int ModuleNumber;

CIndicator::CIndicator(const char *BarTitle)
{
  if(BarTitle)
  {
    Items=3;
    MsgData[0] = BarTitle;
    MsgData[1] = "";
    MsgData[2] = Bar;
  }
  else
  {
    Items=2;
    MsgData[0] = "";
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
          *ptr = 219;           //'Û'
          ++ptr;
          ++Counter;
        }
      FarMessage(ModuleNumber, FMSG_LEFTALIGN, NULL, MsgData, Items, 0);
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
              *ptr = 219;       //'Û'
              ++ptr;
              ++Counter;
            }
          CurPos = Counter;
          FarMessage(ModuleNumber, FMSG_LEFTALIGN, NULL, MsgData, Items,
                       0);
        }
    }
}

void CIndicator::SetParams(const char *Title, int total)
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
      *ptr = 176;               //'°'
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
