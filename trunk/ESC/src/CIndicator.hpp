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
  void StartTimer(DWORD ms);    // ��뢠���� ᠬ� ����
  void SetParams(const char *Title, int total); // ��뢠���� ����

private:
  CIndicator& operator=(const CIndicator& rhs); /* �⮡� �� */
  CIndicator(const CIndicator& rhs); /* �����஢����� �� 㬮�砭�� */

};

#endif // __CIndicator_hpp
