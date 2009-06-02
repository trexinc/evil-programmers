#include "stdafx.h"
#include "find.hpp"

INT GetAllProcessesList(PROCESSENTRY32* ProcessesList)
{
  BOOL StopEnumeration = FALSE;
  HANDLE Snap;
  PROCESSENTRY32 Process;
  INT count = 0;

  Snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (Snap != INVALID_HANDLE_VALUE)
  {
    Process.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(Snap, &Process))
    {
      do
      {
        if (ProcessesList)
          ProcessesList[count] = Process;
        count++;
      }
      while (!StopEnumeration && Process32Next(Snap, &Process));
    }
    CloseHandle(Snap);
  }
  return count;
}