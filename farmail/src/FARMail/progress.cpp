/*
    FARMail plugin for FAR Manager
    Copyright (C) 2002-2005 FARMail Group
    Copyright (C) 1999,2000 Serge Alexandrov

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
#include "farmail.hpp"
#include "progress.hpp"

#define SPACE_PROGR 5
DWORD WINAPI ProgressThread(LPVOID lpvThreadParm)
{
  ((Progress *)lpvThreadParm)->ThreadProc();
  return 0;
}

Progress::Progress(long total,const char *s,long totalshow)
{
  Total=total;
  if(totalshow==-1)
    TotalShow=total;
  else
    TotalShow=totalshow;
  hs=_Info.SaveScreen(0,0,-1,-1);
  FSF.sprintf(head,::GetMsg(MesProgressSend_Title),s);
  mes[0]=head;
  mes[1]=NULLSTR;
  mes[2]=body;
  mes[3]=NULLSTR;
  start=GetCurrentTime();
  Event=Mutex=Thread=NULL;
  Event=CreateEvent(NULL,TRUE,FALSE,NULL);
  Mutex=CreateMutex(NULL,FALSE,NULL);
  if(Event&&Mutex)
  {
    DWORD ThreadID;
    Thread=CreateThread(NULL,0,ProgressThread,this,0,&ThreadID);
  }
}

Progress::~Progress()
{
  if(Thread)
  {
    SetEvent(Event);
    WaitForSingleObject(Thread,INFINITE);
    CloseHandle(Thread);
  }
  if(Event) CloseHandle(Event);
  if(Mutex) CloseHandle(Mutex);
  _Info.RestoreScreen( hs );
}

#if defined(__GNUC__)
long Ceil(long __x, long __y, long __z)
{
  long __value;
  volatile unsigned short int __cw, __cwtmp;
  __asm __volatile ("fnstcw %0" : "=m" (__cw));
  __cwtmp = (__cw & 0xf3ff) | 0x0800; /* rounding up */
  __asm __volatile ("fldcw %0" : : "m" (__cwtmp));

  __asm __volatile ("fildl %0" : : "m" (__x));
  __asm __volatile ("fimull %0" : : "m" (__y));

  __asm __volatile ("fidivl %0" : : "m" (__z));
  __asm __volatile ("fistpl %0" : : "m" (__value));
  __asm __volatile ("fldcw %0" : : "m" (__cw));

  return __value;
}
#elif defined(_MSC_VER)
long Ceil(long __x, long __y, long __z)
{
  register long __value;
  volatile unsigned short int __cw, __cwtmp;

  __asm fnstcw __cw;
  __cwtmp = (__cw & 0xf3ff) | 0x0800; // rounding up
  __asm fldcw __cwtmp;
  __asm fild __x;
  __asm fimul __y;
  __asm fidiv __z;
  __asm fistp __value;
  __asm fldcw __cw;

  return __value;
}
#else
long Ceil(long __x, long __y, long __z)
{
  register long __value;
  volatile unsigned short int __cw, __cwtmp;

  asm fnstcw __cw;
  __cwtmp = (__cw & 0xf3ff) | 0x0800; // rounding up
  asm fldcw __cwtmp;
  asm fild __x;
  asm fimul __y;
  asm fidiv __z;
  asm fistp __value;
  asm fldcw __cw;

  return __value;
}
#endif

int Progress::UseProgress(long cur,long curshow)
{
  if(curshow==-1) curshow=cur;
  if(cur>Total) cur=Total;
  WaitForSingleObject(Mutex,INFINITE);
  Cur=cur;
  CurShow=curshow;
  ReleaseMutex(Mutex);
  return 0;
}

void Progress::ThreadProc(void)
{
  DWORD Stop;
  do
  {
    long cur,curshow;
    WaitForSingleObject(Mutex,INFINITE);
    cur=Cur;
    curshow=CurShow;
    ReleaseMutex(Mutex);

    DWORD now=GetCurrentTime();
    long delta=(now-start);
    long CPS;

    if(delta>0)
    {
      //CPS=ceil(((double)cur)*1000.0/((double)delta));
      CPS=Ceil(cur,1000,delta);
    }
    else
      CPS=0;

    FSF.sprintf(body,::GetMsg(MesProgress_Body),curshow,TotalShow,CPS);
    _Info.Message(_Info.ModuleNumber,0,NULL,mes,4,0);
    Stop=WaitForSingleObject(Event,1000);
  } while(Stop==WAIT_TIMEOUT);
}

ShortMessage::ShortMessage( int num )
{
  hs = _Info.SaveScreen(0,0,-1,-1);
  mes[0] = NULLSTR;
  mes[1] = ::GetMsg( num );
  _Info.Message( _Info.ModuleNumber, 0, NULL, mes, 2, 0 );
}

ShortMessage::~ShortMessage()
{
  _Info.RestoreScreen( hs );
}



int Confirm( int title )
{
  const char *mes[4];

  mes[0] = NULLSTR;
  mes[1] = ::GetMsg(title);
  mes[2] = ::GetMsg(MesYes);
  mes[3] = ::GetMsg(MesNo);

  int answer = _Info.Message( _Info.ModuleNumber, 0, NULL, mes, 4, 2 );

  return answer;
}

Bar::Bar( long total , const char *s , int len )
{
  int i;

  Total = total;
  Len = len;
  hs = _Info.SaveScreen(0,0,-1,-1);
  Head = s;

  FSF.sprintf( head, "%s : %d/%d", s, 0, total );
  Mes[0] = head;
  Mes[1] = NULLSTR;
  Mes[2] = bar;
  Mes[3] = NULLSTR;

  for (i=0 ; i<len; i++ )
      bar[i+SPACE_PROGR] = '±';
  for ( i=0 ; i<SPACE_PROGR; i++ )
  {
    bar[i] = ' ';
    bar[i+len+SPACE_PROGR] = ' ';
  }
  bar[len+SPACE_PROGR*2] = 0;

  _Info.Message( _Info.ModuleNumber, 0, NULL, Mes, 4, 0 );
}

int Bar::UseBar( long cur )
{
  int i;
  int brk = (Len*cur/Total);

  if ( brk > Len ) brk = Len;

  for( i=0 ; i<brk; i++ )
      bar[i+SPACE_PROGR] = 'Û';

  FSF.sprintf( head, "%s : %d/%d", Head, cur, Total );

  _Info.Message( _Info.ModuleNumber, 0, NULL, Mes, 4, 0 );
  return 0;
}

Bar::~Bar()
{
  _Info.RestoreScreen( hs );
}
