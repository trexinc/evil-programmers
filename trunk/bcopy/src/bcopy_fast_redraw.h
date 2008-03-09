#ifndef __BCOPY_FAST_REDRAW_H__
#define __BCOPY_FAST_REDRAW_H__

#define DM_REDRAW_UNICODE DM_USER+1
#define DN_TIMER DM_USER+2
#define DM_CHECK_STATE DM_USER+3

#define UNICODE_LABEL_COUNT 2

struct ThreadData
{
  BOOL Process;
  HANDLE DataMutex;
  HANDLE StopEvent;
  HANDLE Thread;
  LONG Interval;
};

struct BasicDialogData
{
  ThreadData *Thread;
  unsigned long long StartTime;
  int UnicodeInicies[UNICODE_LABEL_COUNT];
  wchar_t FileNameW[UNICODE_LABEL_COUNT][2*MAX_PATH];
  HANDLE Console;
  bool InMacro;
  FARAPIDEFDLGPROC KeyProc;
  int MacroIndex;
  bool State;
  bool InRefresh;
};

extern HANDLE OpenConsoleIn(void);
extern void WriteConsoleKey(HANDLE console,DWORD Key,DWORD State);
extern void Redraw_Close(ThreadData *thdata);
extern DWORD WINAPI TimeThread(LPVOID lpvThreadParm);
extern long FastRedrawDefDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
extern void InitThreadData(ThreadData *Thread);
extern void FreeThreadData(ThreadData *Thread);

#endif
