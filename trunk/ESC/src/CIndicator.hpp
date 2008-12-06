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
  const char *MsgData[3];
  char Bar[BarSize + 1], *ptr;
  int Procent, NewProcent, OldProcent, Counter, Total, CurPos, Items;

  BOOL CheckTimer();

public:
    CIndicator(const char *BarTitle);
   ~CIndicator();
  void ShowProgress(int Cur);
  void ShowFinal(void);
  void StartTimer(DWORD ms);    // вызывается самым первым
  void SetParams(const char *Title, int total); // вызывается вторым

private:
  CIndicator& operator=(const CIndicator& rhs); /* чтобы не */
  CIndicator(const CIndicator& rhs); /* генерировалось по умолчанию */

};

#endif // __CIndicator_hpp
