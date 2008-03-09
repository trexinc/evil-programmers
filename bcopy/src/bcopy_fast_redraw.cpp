#include <stdio.h>
#include "../../plugin.hpp"
#include "../../farkeys.hpp"
#include "bcplugin.h"
#include "bcopy_fast_redraw.h"
#include "dm_macro.hpp"

HANDLE OpenConsoleIn(void)
{
  return CreateFileW(L"CONIN$",GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
}

void WriteConsoleKey(HANDLE console,DWORD Key,DWORD State)
{
  INPUT_RECORD event; DWORD written;
  event.EventType=KEY_EVENT;
  event.Event.KeyEvent.bKeyDown=TRUE;
  event.Event.KeyEvent.wRepeatCount=1;
  event.Event.KeyEvent.wVirtualKeyCode=(WORD)Key;
  event.Event.KeyEvent.wVirtualScanCode=0;
  event.Event.KeyEvent.uChar.UnicodeChar=0;
  event.Event.KeyEvent.dwControlKeyState=State;
  WriteConsoleInputW(console,&event,1,&written);
}

void Redraw_Close(ThreadData *thdata)
{
  if(thdata->Process)
    SetEvent(thdata->StopEvent);
}

DWORD WINAPI TimeThread(LPVOID lpvThreadParm)
{
  ThreadData *thdata=(ThreadData *)lpvThreadParm;
  HANDLE console=OpenConsoleIn();
  if(console!=INVALID_HANDLE_VALUE)
  {
    while(true)
    {
      LONG LocalInterval;
      WaitForSingleObject(thdata->DataMutex,INFINITE);
      LocalInterval=thdata->Interval;
      ReleaseMutex(thdata->DataMutex);
      if(WaitForSingleObject(thdata->StopEvent,LocalInterval)!=WAIT_TIMEOUT) break;
      WriteConsoleKey(console,VK_BACK,LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED);
    }
    CloseHandle(console);
  }
  return 0;
}

LONG_PTR FastRedrawDefDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  BasicDialogData *DlgParams=(BasicDialogData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  FILETIME CurrentTimeFT; unsigned long long CurrentTime;
  GetSystemTimeAsFileTime(&CurrentTimeFT);
  memcpy(&CurrentTime,&CurrentTimeFT,sizeof(CurrentTime));
  switch(Msg)
  {
    case DN_INITDIALOG:
      if(DlgParams->Thread->Process) Info.SendDlgMessage(hDlg,DM_SETMOUSEEVENTNOTIFY,1,0);
      break;
    case DN_MOUSEEVENT:
      if(DlgParams->Thread->Process) Info.SendDlgMessage(hDlg,DM_CHECK_STATE,0,0);
      break;
    case DN_ENTERIDLE:
      DlgParams->InMacro=false;
      if(!DlgParams->Thread->Process) Info.SendDlgMessage(hDlg,DN_TIMER,0,0);
      else Info.SendDlgMessage(hDlg,DM_CHECK_STATE,0,0);
      break;
    case DN_DRAWDLGITEM:
      if(DlgParams->Console!=INVALID_HANDLE_VALUE) Info.SendDlgMessage(hDlg,DM_REDRAW_UNICODE,Param1,0);
      break;
    case DN_DRAWDIALOGDONE:
      if(!DlgParams->InRefresh)
      {
        DlgParams->InRefresh=true;
        if(!DlgParams->Thread->Process) Info.SendDlgMessage(hDlg,DM_CHECK_STATE,0,0);
        DlgParams->InRefresh=false;
      }
      break;
    case DM_REDRAW_UNICODE:
      if(DlgParams->Console!=INVALID_HANDLE_VALUE)
      {
        for(int i=0;i<UNICODE_LABEL_COUNT;i++)
        {
          if(DlgParams->UnicodeInicies[i]>=0&&Param1==DlgParams->UnicodeInicies[i])
          {
            DWORD written; COORD coord;
            SMALL_RECT Rect; FarDialogItem DialogItem;
            Info.SendDlgMessage(hDlg,DM_GETDLGRECT,0,(LONG_PTR)&Rect);
            Info.SendDlgMessage(hDlg,DM_GETDLGITEM,DlgParams->UnicodeInicies[i],(LONG_PTR)&DialogItem);
            coord.X=Rect.Left+DialogItem.X1;
            coord.Y=Rect.Top+DialogItem.Y1;
            WriteConsoleOutputCharacterW(DlgParams->Console,&(DlgParams->FileNameW[i][0]),(DWORD)wcslen(&(DlgParams->FileNameW[i][0])),coord,&written);
          }
        }
      }
      break;
    case DN_KEY:
      if(Param2==(KEY_BS|KEY_CTRL|KEY_ALT|KEY_SHIFT))
      {
        DlgParams->StartTime=CurrentTime;
        if(DlgParams->State)
          Info.SendDlgMessage(hDlg,DN_TIMER,0,0);
        else
          Info.SendDlgMessage(hDlg,DM_CHECK_STATE,0,0);
        DlgParams->State=!DlgParams->State;
        return TRUE;
      }
      else if(!DlgParams->InMacro&&run_macro(DlgParams->MacroIndex,(DWORD)Param2))
      {
        DlgParams->InMacro=true;
        return TRUE;
      }
      else if(DlgParams->KeyProc) DlgParams->KeyProc(hDlg,Msg,Param1,Param2);
      if(!DlgParams->InMacro) Info.SendDlgMessage(hDlg,DM_CHECK_STATE,0,0);
      break;
    case DN_MACRO_DISABLE:
      return TRUE;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void InitThreadData(ThreadData *Thread)
{
  if(PlgOpt.RefreshInterval)
  {
    Thread->DataMutex=CreateMutex(NULL,FALSE,NULL);
    if(Thread->DataMutex)
    {
      Thread->StopEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
      if(Thread->StopEvent)
      {
        DWORD ThreadID;
        Thread->Thread=CreateThread(NULL,0,TimeThread,(void *)Thread,CREATE_SUSPENDED,&ThreadID);
        if(Thread->Thread)
        {
          Thread->Process=TRUE;
          ResumeThread(Thread->Thread);
        }
      }
    }
  }
}

void FreeThreadData(ThreadData *Thread)
{
  if(Thread->Thread)
  {
    WaitForSingleObject(Thread->Thread,INFINITE);
    CloseHandle(Thread->Thread);

    HANDLE console=OpenConsoleIn();
    if(console!=INVALID_HANDLE_VALUE)
    {
      WriteConsoleKey(console,VK_CONTROL,0);
      CloseHandle(console);
    }
  }
  if(Thread->DataMutex) CloseHandle(Thread->DataMutex);
  if(Thread->StopEvent) CloseHandle(Thread->StopEvent);
}
