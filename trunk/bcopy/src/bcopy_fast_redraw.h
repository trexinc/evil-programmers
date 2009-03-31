/*
    bcopy_fast_redraw.h
    Copyright (C) 2000-2009 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

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
extern LONG_PTR FastRedrawDefDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);
extern void InitThreadData(ThreadData *Thread);
extern void FreeThreadData(ThreadData *Thread);

#endif
