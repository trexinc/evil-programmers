/*
    bci.cpp
    Copyright (C) 2005 WhiteDragon
    Copyright (C) 2009 DrKnS

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

#define UNICODE
#define _UNICODE
#define _WIN32_IE 0x0600

#include <windows.h>
#include <shellapi.h>
#include <mmsystem.h>

#include "bci.h"

HMENU menu;
HINSTANCE hin;
LPWSTR wav;
bool z;
UINT lst_size=0;

PVOID __cdecl memset(PVOID dst,int val, size_t count)
{
  PVOID start=dst;
  while(count--)
  {
    *(LPSTR)dst=(CHAR)val;
    dst=(LPSTR)dst+1;
  }
  return start;
}

bool GetJobList(DWORD *size,struct SmallInfoRec **receive)
{
  *size=0; *receive=NULL;
  bool res=false;
  DWORD send[2]={OPERATION_INFO,INFOFLAG_ALL};
  DWORD dwBytesRead,dwBytesWritten,rec_size;
  HANDLE hPipe=CreateFile(PIPE_NAME,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
  if(hPipe!=INVALID_HANDLE_VALUE)
  {
    if(WriteFile(hPipe,send,sizeof(send),&dwBytesWritten,NULL))
      if(ReadFile(hPipe,size,sizeof(*size),&dwBytesRead,NULL)&&ReadFile(hPipe,&rec_size,sizeof(rec_size),&dwBytesRead,NULL))
      {
        res=true;
      if(*size)
      {
        *receive=(SmallInfoRec *)HeapAlloc(GetProcessHeap(),0,sizeof(SmallInfoRec)*(*size));
        if((!(*receive))||(!ReadFile(hPipe,*receive,sizeof(struct SmallInfoRec)*(*size),&dwBytesRead,NULL)))
        {
          *size=0;
          HeapFree(GetProcessHeap(),0,*receive);
          *receive=NULL;
          res=false;
        }
      }
    }
    CloseHandle(hPipe);
  }
  return res;
}

void SendCommand(DWORD ThreadId,DWORD Command)
{
  DWORD send[3]={OPERATION_INFO,Command,ThreadId};
  HANDLE hPipe=CreateFile(PIPE_NAME,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
  if(hPipe!=INVALID_HANDLE_VALUE)
  {
    DWORD Transfered;
    WriteFile(hPipe,send,sizeof(send),&Transfered,NULL);
    CloseHandle(hPipe);
  }
}

bool GetInfo(DWORD ThreadId,struct InfoRec *receive)
{
  bool res=false;
  DWORD dwBytesRead,dwBytesWritten,RecSize;
  DWORD send[3]={OPERATION_INFO,INFOFLAG_BYHANDLE,ThreadId};
  receive->info.type=INFOTYPE_INVALID;
  HANDLE hPipe=CreateFile(PIPE_NAME,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
  if(hPipe!=INVALID_HANDLE_VALUE)
  {
    WriteFile(hPipe,send,sizeof(send),&dwBytesWritten,NULL);
    if(ReadFile(hPipe,&RecSize,sizeof(RecSize),&dwBytesRead,NULL)) //FIXME
      ReadFile(hPipe,receive,sizeof(struct InfoRec),&dwBytesRead,NULL);
    CloseHandle(hPipe);
  }
  else if(GetLastError()==ERROR_PIPE_BUSY) res=true;
  return res;
}

BOOL systray(HWND hwnd,UINT uID,HICON hIcon,LPWSTR lpszTip)
{
  static HICON hPrevIcon=NULL;
  if(hPrevIcon)
    DestroyIcon(hPrevIcon);
  hPrevIcon=hIcon;
  NOTIFYICONDATAW tnid;
  ZeroMemory(&tnid,sizeof(tnid));
  DWORD Message=NIM_DELETE;
  tnid.cbSize=sizeof(NOTIFYICONDATAW);
  tnid.hWnd=hwnd;
  tnid.uID=uID;
  if(hIcon)
  {
    tnid.uFlags=NIF_ICON|NIF_TIP|NIF_MESSAGE/*|NIF_INFO*/;
    tnid.uCallbackMessage=WM_TASKICON;
    tnid.hIcon=hIcon;
    Message=NIM_MODIFY;
  }
  lstrcpynW(tnid.szTip,lpszTip,sizeofa(tnid.szTip)-1);
  //lstrcpynW(tnid.szInfo,lpszTip,sizeofa(tnid.szTip)-1);

  BOOL res=Shell_NotifyIcon(Message,&tnid);
  if(!res)
    res=Shell_NotifyIconW(NIM_ADD,&tnid);
  return res;
}

int proc(HWND h,UINT m,WPARAM w,LPARAM l)
{
  POINT cur;
  DWORD size;
  struct SmallInfoRec *list;
  WCHAR file[500],hint[500];
  switch(m)
  {
    case WM_TASKICON:
      switch((UINT)l)
      {
        case WM_LBUTTONDOWN:
          SendCommand((UINT)w,INFOFLAG_PAUSE);
          break;
        case WM_RBUTTONDOWN:
          GetCursorPos(&cur);
          SetForegroundWindow(h);
          switch(TrackPopupMenu(menu,TPM_RIGHTALIGN|TPM_RETURNCMD|TPM_NONOTIFY,cur.x,cur.y,0,h,0))
          {
            case 1:
              SendCommand((UINT)w,INFOFLAG_STOP);
              break;
            case 2:
              SendCommand((UINT)w,INFOFLAG_PAUSE);
              break;
            case 3:
              PostQuitMessage(0);
          }
      }
      break;
    case WM_TIMER:
    {
      if(GetJobList(&size,&list))
      {
        for(UINT i=0;i<size;i++)
        {
          int found=0;
          for(UINT j=0;j<lst_size;j++)
            if(list[i].ThreadId==lst[j].id)
            {
              if((lst[i].pr!=list[i].percent+128*list[i].pause))
              {
                lst[i].pr=(BYTE)list[i].percent+128*list[i].pause;
                int o=list[i].type-1;
                ZeroMemory(x,sizeof(x));
                x[12]=(0xffff>>(list[i].percent*145/1000+1))&0x7ffe;
                swb(x[12]);
                x[13]=x[12];
                a[0]=a[1]=a[2]=a[3]=0xffff-0x1b00*list[i].pause;
                for(int k=0;k<4;k++)
                  a[k]-=t[o][k];
                for(int k=0;k<5;k++)
                {
                  a[k+5]=0xffff-n[list[i].percent/10][k]*0x1000-n[list[i].percent%10][k]*0x80-n[10][k];
                  swb(a[k+5]);
                }
                if(z)
                  for(int k=0;k<16;k++)
                    x[k]=~(a[k]|x[k]);
                lstrcpyW(file,list[i].Src);
                wsprintfW(hint,L"%s %s\nВыполнено %i%%",O[o],file,list[i].percent);
                HBITMAP ba=CreateBitmap(16, 16, 1, 1, a),bx=CreateBitmap(16, 16, 1, 1, x);
                ICONINFO ii={TRUE,0,0,ba,bx};
                HICON hi = CreateIconIndirect(&ii);
                DeleteObject(ba);
                DeleteObject(bx);
                systray(h,list[i].ThreadId,hi,hint);
              }
              found=1;
            }
            if(!found)
            {
              JobItem ji;
              ji.id=list[i].ThreadId;
              ji.pr=111;
              lst[lst_size++]=ji;
              /*wsprintfW(hint,L"Выполнено %i%%",list[i].percent);
              x[12]=x[13]=0xfe7f;
              a[0]=a[1]=a[2]=a[3]=a[5]=a[6]=a[7]=a[8]=a[9]=0xffff;
              HBITMAP ba=CreateBitmap(16, 16, 1, 1, a),bx=CreateBitmap(16, 16, 1, 1, x);
              ICONINFO ii={TRUE,0,0,ba,bx};
              HICON hi = CreateIconIndirect(&ii);
              DeleteObject(ba);
              DeleteObject(bx);
              systray(h,list[i].ThreadId,hi,hint);*/
            }
          }
          for(UINT j=0;j<lst_size;j++)
          {
            int found=0;
            for(UINT i=0;i<size;i++)
              if(list[i].ThreadId==lst[j].id)
              {
                found=1;
                break;
              }
              if(!found)
              {
                systray(h,lst[j].id,0,0);
                lst[j]=lst[--lst_size];
                PlaySound(wav,NULL,SND_FILENAME);
              }
          }
          HeapFree(GetProcessHeap(),0,list);
        }
      }
      break;
    }
    return DefWindowProc(h,m,w,l);
}

template <typename T, size_t N> inline size_t arraySize(const T (&)[N]){return N;}

int APIENTRY wWinMain(HINSTANCE,HINSTANCE,LPWSTR,int)
{
  a[4]=a[10]=a[15]=0xffff;
  HINSTANCE I=(HINSTANCE)GetModuleHandleW(NULL);
  hin=I;
  if(!FindWindowW(0,wNm))
  {
    menu=CreatePopupMenu();
    AppendMenuW(menu,0,1,L"Остановить");
    AppendMenuW(menu,0,2,L"Пауза/Продолжить");
    AppendMenuW(menu,MF_SEPARATOR,0,NULL);
    AppendMenuW(menu,0,3,L"Выход");

    WNDCLASS c;
    ZeroMemory(&c,sizeof(WNDCLASS));
    c.lpfnWndProc=(WNDPROC)proc;
    c.hInstance = I;
    c.lpszClassName=wCls;
    RegisterClass(&c);
    HWND w=CreateWindow(wCls,wNm,0,-1,-1,0,0,NULL,NULL,I,NULL);
    LPCWSTR cmdline=GetCommandLineW();
    int argc=0;
    LPWSTR* argw=CommandLineToArgvW(cmdline,&argc);
    wchar_t a[MAX_PATH];
    wav=argc>1?argw[1]:L"";
    DWORD s=GetModuleFileNameW(NULL,a,sizeof(a));
    z=(a[s-5]==L'w'||a[s-5]==L'W');
    SetTimer(w,0,100,0);
    MSG m;
    while(GetMessage(&m,NULL,0,0))
    {
      TranslateMessage(&m);
      DispatchMessage(&m);
    }
    for(UINT i=0;i<lst_size;i++)
      systray(w,lst[i].id,0,0);
    LocalFree(argw);
  }
  return 0;
}
