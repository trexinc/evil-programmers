/*
    HtmlHelp sub-plugin for Active-Help plugin for FAR Manager
    Copyright (C) 2003 Alex Yaroslavsky

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
#include "../../ahp.hpp"
#include "registry.hpp"
#include "crt.hpp"
#include "htmlhelp.h"

#if defined(__GNUC__)
extern "C"
{
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
};

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}
#endif

extern "C"
{
  void CALLBACK _export RundllEntry(HWND hwnd, HINSTANCE hinst, LPSTR lpCmdLine, int nCmdShow);
};

static void Config(void);

#define sizeofa(array) (sizeof(array)/sizeof(array[0]))
#define WAHMSG_SHOWHELP (WM_USER+1)

static HWND MHWND;
static unsigned int WCounter=0;

static struct PluginStartupInfo Info;
static FARSTANDARDFUNCTIONS FSF;
static struct AHPluginStartupInfo AHInfo;
char PluginRootKey[80];
static int HowShowWindow;
static char PathToViewer[512];
static char OpenContents[512];
static char KeywordSearch[512];
static HANDLE mutex;

enum
{
  MTitle,
  MOpenIn,
  MSW_NORMAL,
  MSW_MAXIMIZED,
  MSW_DEFAULT,
  MErrMSDN,
  MKeyHHTitle,
  MPathToViewer,
  MOpenContents,
  MKeywordSearch,
  MOk,
  MCancel,
};

static int hswa[] =
{
  SW_SHOWNORMAL,
  SW_SHOWMAXIMIZED,
  SW_SHOWDEFAULT
};

typedef HWND (WINAPI *THtmlHelp)(HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData);
static THtmlHelp dHtmlHelp;
static WNDPROC OldHHWndProc=0;
static HANDLE event;

struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1, Y1, X2, Y2;
  unsigned int Flags;
  signed char Data;
};

static char *GetMsg(int MsgNum, char *Str)
{
  AHInfo.GetMsg(AHInfo.MessageName,MsgNum,Str);
  return Str;
}

static void InitDialogItems(const struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber)
{
  int i;
  struct FarDialogItem *PItem = Item;
  const struct InitDialogItem *PInit = Init;

  for (i=0; i<ItemsNumber; i++, PItem++, PInit++)
  {
    PItem->Type = PInit->Type;
    PItem->X1 = PInit->X1;
    PItem->Y1 = PInit->Y1;
    PItem->X2 = PInit->X2;
    PItem->Y2 = PInit->Y2;
    PItem->Focus = 0;
    PItem->Selected = 0;
    PItem->Flags = PInit->Flags;
    PItem->DefaultButton = 0;
    if (PInit->Data==-1)
      *PItem->Data = 0;
    else
      GetMsg(PInit->Data,PItem->Data);
  }
}

LRESULT WINAPI HHWndProc(HWND hwnd,UINT message,UINT wParam,LONG lParam)
{
  LRESULT ret=CallWindowProc(OldHHWndProc,hwnd,message,wParam,lParam);
  if(message==WM_CLOSE)
  {
    if ((--WCounter)<=0)
    {
      CloseHandle(event);
      PostMessage(MHWND,WM_CLOSE,0,0);
      WCounter=0;
    }
  }
  return ret;
}

BOOL ShowHelp(char *FileName, char *Keyword, int SW, char *ErrorMsg)
{
  char szPath[512];
  BOOL ret=TRUE;
  if (!lstrcmpi(FileName,"MSDN"))
  {
    ret=FALSE;
    HKEY hKeyCol;
    if( RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\HTML Help Collections\\Developer Collections",0,KEY_READ,&hKeyCol)==ERROR_SUCCESS )
    {
      DWORD dwType, dwSize=sizeof(szPath);
      if( RegQueryValueEx(hKeyCol,"Language",0,&dwType,(LPBYTE)szPath,&dwSize)==ERROR_SUCCESS )
      {
        dwSize=sizeof(szPath);
        HKEY hKeyLang;
        if( RegOpenKeyEx(hKeyCol,szPath,0,KEY_READ,&hKeyLang)==ERROR_SUCCESS )
        {
          dwSize=sizeof(szPath);
          if( RegQueryValueEx(hKeyLang,"Preferred",0,&dwType,(LPBYTE)szPath,&dwSize)==ERROR_SUCCESS )
          {
            HKEY hKeyLoc;
            if( RegOpenKeyEx(hKeyLang,szPath,0,KEY_READ,&hKeyLoc)==ERROR_SUCCESS )
            {
              dwSize=sizeof(szPath);
              if( RegQueryValueEx(hKeyLoc,"Filename",0,&dwType,(LPBYTE)szPath,&dwSize)==ERROR_SUCCESS )
              {
                ret=TRUE;
                FileName=szPath;
              }
              RegCloseKey(hKeyLoc);
            }
          }
          RegCloseKey(hKeyLang);
        }
      }
      RegCloseKey(hKeyCol);
    }
  }
  if (!ret)
  {
    //Error message is already loaded
    if(OldHHWndProc==0 && WCounter<=0)
    {
      CloseHandle(event);
      PostMessage(MHWND,WM_CLOSE,0,0);
    }
    return ret;
  }
  HWND hwndCaller = GetDesktopWindow();
  HWND hwnd = dHtmlHelp(hwndCaller,FileName,HH_DISPLAY_TOPIC,0);
  if (*Keyword)
  {
    HH_AKLINK link;
    link.cbStruct = sizeof(link);
    link.fReserved = FALSE;
    link.pszKeywords = Keyword;
    link.pszUrl = NULL;
    link.pszMsgText = NULL;
    link.pszMsgTitle = NULL;
    link.pszWindow = NULL;
    link.fIndexOnFail = TRUE;
    DWORD dwData = (DWORD)&link;
    hwnd = dHtmlHelp(hwndCaller, FileName, HH_KEYWORD_LOOKUP, dwData);
    hwnd = dHtmlHelp(hwndCaller, FileName, HH_ALINK_LOOKUP, dwData);
  }
  ret = hwnd==NULL ? FALSE : TRUE;
  if (!ret)
  {
    HH_LAST_ERROR lasterror;
    hwnd = dHtmlHelp(hwndCaller, NULL, HH_GET_LAST_ERROR, (DWORD)&lasterror);
    if (hwnd && (lasterror.hr>=0))
    {
      ret = TRUE;
    }
    else
    {
      WideCharToMultiByte(CP_ACP,0,lasterror.description,-1,ErrorMsg,128,NULL,NULL);
      CharToOem(ErrorMsg,ErrorMsg);
    }
    if(OldHHWndProc==0 && WCounter<=0)
    {
      CloseHandle(event);
      PostMessage(MHWND,WM_CLOSE,0,0);
    }
  }
  else
  {
    LONG l=SetWindowLong(hwnd,GWL_WNDPROC,(LONG)HHWndProc);
    if(l!=(LONG)HHWndProc)
    {
      WCounter++;
      OldHHWndProc=(WNDPROC)l;
    }
    ShowWindow(hwnd, SW_SHOW);
    ShowWindow(hwnd, hswa[SW]);
  }
  return ret;
}

LRESULT WINAPI WndProc(HWND hwnd,UINT message,UINT wParam,LONG lParam)
{
  switch (message)
  {
    case WAHMSG_SHOWHELP:
    {
      HANDLE hMap=OpenFileMapping(FILE_MAP_READ|FILE_MAP_WRITE,FALSE,"htmlhelpFMap");
      int ret=FALSE;
      if (dHtmlHelp)
      {
        char *FileName=(char*)MapViewOfFile(hMap,FILE_MAP_WRITE|FILE_MAP_READ,0,0,0);
        ret=ShowHelp(FileName+128,
                     FileName+128+lstrlen(FileName+128)+1,
                     FileName[128+lstrlen(FileName+128)+1+lstrlen(FileName+128+lstrlen(FileName+128)+1)+1],
                     FileName);
        UnmapViewOfFile(FileName);
      }
      CloseHandle(hMap);
      if (!ret && WCounter<=0)
        PostQuitMessage(0);
      return ret;
    }

    case WM_CLOSE:
      PostQuitMessage(0);
      return 0;

    default:
      return DefWindowProc(hwnd, message, wParam, lParam);
  }
}

void CALLBACK _export RundllEntry(HWND hwnd, HINSTANCE hinst, LPSTR lpCmdLine, int nCmdShow)
{
  (void)hwnd;
  (void)lpCmdLine;
  (void)nCmdShow;
  event=OpenEvent(EVENT_ALL_ACCESS,0,"htmlhelpWEvent");
  HMODULE hLib = LoadLibrary("HHCTRL.OCX");
  dHtmlHelp = (THtmlHelp)GetProcAddress(hLib,(char*)14);

  WNDCLASS wchtmlhelp={0};
  wchtmlhelp.lpfnWndProc = (WNDPROC)WndProc;
  wchtmlhelp.lpszClassName = "htmlhelpWClass";
  wchtmlhelp.hIcon = LoadIcon(hLib,"ICON!HTMLHELP");
  RegisterClass(&wchtmlhelp);
  MHWND = CreateWindow("htmlhelpWClass","", WS_OVERLAPPEDWINDOW, 0,0, 0,0, NULL,NULL,hinst,NULL);

  SetEvent(event);

  MSG msg;
  while(GetMessage(&msg,NULL,0,0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    if (msg.message==WM_CLOSE)
      Sleep(2000);
  }

  if (hLib)
    FreeLibrary(hLib);
}

static void MakeCommandLine(char *CommandLine, const char *Path, const char *Command, const char *FileName, const char *ShortFileName, const char *Keyword)
{
  lstrcpy(CommandLine,Path);
  FSF.QuoteSpaceOnly(CommandLine);
  char *ptr=CommandLine+lstrlen(CommandLine);
  *(ptr++)=' ';
  const char *srcp=Command;
  while (*srcp)
  {
    if (*srcp=='%')
    {
      switch (*(srcp+1))
      {
        case 's':
        case 'S':
        {
          srcp+=2;
          lstrcpy(ptr,ShortFileName);
          ptr+=lstrlen(ptr);
          break;
        }
        case 'f':
        case 'F':
        {
          srcp+=2;
          lstrcpy(ptr,FileName);
          ptr+=lstrlen(ptr);
          break;
        }
        case 'k':
        case 'K':
        {
          srcp+=2;
          lstrcpy(ptr,Keyword);
          ptr+=lstrlen(ptr);
          break;
        }
        case '%':
        {
          srcp+=2;
          *(ptr++)='%';
          break;
        }
        default:
          *(ptr++)=*(srcp++);
      }
    }
    else
      *(ptr++)=*(srcp++);
  }
  *ptr=0;
}

int ExternalHTMLHelp(const char *Keyword, const char *FileName, const char *ShortFileName, char *Error)
{
  char CommandLine[2048];
  if (*Keyword)
    MakeCommandLine(CommandLine,PathToViewer,KeywordSearch,FileName,ShortFileName,Keyword);
  else
    MakeCommandLine(CommandLine,PathToViewer,OpenContents,FileName,ShortFileName,Keyword);
  STARTUPINFO si={0};
  si.cb=sizeof(si);
  PROCESS_INFORMATION pi;
  int ret = CreateProcess(NULL,CommandLine,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);
  if (!ret)
  {
    LPSTR MessageBuffer;
    DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_SYSTEM;
    DWORD dwBufferLength = FormatMessageA(dwFormatFlags,NULL,GetLastError(),MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPSTR)&MessageBuffer,0,NULL);
    lstrcpyn(Error,MessageBuffer,dwBufferLength>128?128:dwBufferLength);
    CharToOem(Error,Error);
  }
  else
  {
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }
  return ret;
}

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct AHPluginStartupInfo *AHInfo)
{
  Info=*FarInfo;
  FSF=*FarInfo->FSF;
  ::AHInfo=*AHInfo;
  FSF.sprintf(PluginRootKey,"%s\\%s",AHInfo->RootKey,"HTMLHelp");
  GetRegKey("HowShowWindow",&HowShowWindow,0);
  HowShowWindow%=sizeofa(hswa);
  GetRegKey("PathToViewer",PathToViewer,"KeyHH",sizeof(PathToViewer));
  GetRegKey("OpenContents",OpenContents,"-%s \"%f\"",sizeof(OpenContents));
  GetRegKey("KeywordSearch",KeywordSearch,"-%s -#klink \"%k\" \"%f\"",sizeof(KeywordSearch));
  mutex = CreateMutex(NULL,FALSE,"htmlhelpWMutex");
  return 0;
}

void WINAPI _export Exit(void)
{
  CloseHandle(mutex);
}

int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData)
{
  switch(Msg)
  {
    case AHMSG_GETINFO:
    {
      GetInfoOutData *data=(GetInfoOutData *)OutData;
      data->TypesNumber=2;
      static struct TypeInfo TypesInfo[2];
      memset(TypesInfo,0,sizeof(TypesInfo));
      for (int i=0; i<2; i++)
      {
        TypesInfo[i].StructSize=sizeof(TypesInfo[0]);
        lstrcpy(TypesInfo[i].Encoding,"WIN");
        lstrcpy(TypesInfo[i].Mask,"*.chm,*.col,*.its");
      }
      lstrcpy(TypesInfo[0].TypeString,"HTMLHelp");
      lstrcat(TypesInfo[0].Mask,",msdn");
      lstrcpy(TypesInfo[0].Encoding,"WIN");
      lstrcpy(TypesInfo[1].TypeString,"ExternalHTMLHelp");
      lstrcpy(TypesInfo[1].Encoding,"OEM");
      data->TypesInfo=TypesInfo;
      GetMsg(MTitle,data->ConfigString);
      data->Flags=AHMSG_CONFIG;
      return TRUE;
    }

    case AHMSG_SHOWHELP:
    {
      HelpInData *data = (HelpInData *)InData;
      HelpOutData *odata = (HelpOutData *)OutData;
      char FileName[512];
      int ret;

      if (data->TypeNumber == 0)
      {
        WaitForSingleObject(mutex,INFINITE);
        HANDLE event=OpenEvent(EVENT_ALL_ACCESS,0,"htmlhelpWEvent");
        if(event==NULL)
        {
          event=CreateEvent(NULL,1,0,"htmlhelpWEvent");
          char shortp[MAX_PATH];
          GetShortPathName(AHInfo.ModuleName,shortp,MAX_PATH);
          char mod[MAX_PATH];
          FSF.sprintf(mod,"%s,RundllEntry",shortp);
          ShellExecute(NULL,"open","rundll32.exe",mod,"",SW_SHOW);
        }
        WaitForSingleObject(event,INFINITE);
        CloseHandle(event);

        {
          DWORD spr = GetShortPathName(data->FileName,FileName,sizeof(FileName));
          if (!(spr>0&&spr<512))
            lstrcpy(FileName,data->FileName);
        }
        int flen=lstrlen(FileName)+1;
        int klen=lstrlen(data->Keyword)+1;

        HANDLE hMap=CreateFileMapping(INVALID_HANDLE_VALUE,0,PAGE_READWRITE,0,128+flen+klen+1,"htmlhelpFMap");
        char *buf=(char*)MapViewOfFile(hMap,FILE_MAP_WRITE|FILE_MAP_READ,0,0,0);
        GetMsg(MErrMSDN,buf);
        memcpy(buf+128,FileName,flen);
        memcpy(buf+flen+128,data->Keyword,klen);
        buf[flen+klen+128]=HowShowWindow;
        UnmapViewOfFile(buf);
        HWND hwnd = FindWindow("htmlhelpWClass",NULL);
        SetForegroundWindow(hwnd);
        ret = SendMessage(hwnd,WAHMSG_SHOWHELP,0,0);
        if (!ret)
        {
          buf=(char*)MapViewOfFile(hMap,FILE_MAP_WRITE|FILE_MAP_READ,0,0,0);
          lstrcpyn(odata->Error,buf,128);
          UnmapViewOfFile(buf);
        }
        CloseHandle(hMap);
        ReleaseMutex(mutex);
      }
      else
      {
        DWORD spr = GetShortPathName(data->FileName,FileName,sizeof(FileName));
        if (spr<=0||spr>=512) //if no short name
          lstrcpy(FileName,FSF.PointToName(data->FileName));
        ret = ExternalHTMLHelp(data->Keyword,data->FileName,FileName,odata->Error);
      }
      return ret;
    }

    case AHMSG_CONFIG:
    {
      Config();
      return TRUE;
    }
  }
  return FALSE;
}

static void Config(void)
{
  static const struct InitDialogItem PreDialogItems[] =
  {
    {DI_DOUBLEBOX  ,3  ,1  ,50 ,15 ,0               ,MTitle},
    {DI_SINGLEBOX  ,4  ,2  ,49 ,5  ,DIF_LEFTTEXT    ,MTitle},
    {DI_TEXT       ,6  ,3  ,0  ,0  ,0               ,MOpenIn},
    {DI_COMBOBOX   ,6  ,4  ,47 ,0  ,DIF_DROPDOWNLIST,-1},
    {DI_SINGLEBOX  ,4  ,6  ,49 ,13 ,DIF_LEFTTEXT    ,MKeyHHTitle},
    {DI_TEXT       ,6  ,7  ,0  ,0  ,0               ,MPathToViewer},
    {DI_EDIT       ,6  ,8  ,47 ,0  ,DIF_EDITEXPAND  ,-1},
    {DI_TEXT       ,6  ,9  ,0  ,0  ,0               ,MOpenContents},
    {DI_EDIT       ,6  ,10 ,47 ,0  ,0               ,-1},
    {DI_TEXT       ,6  ,11 ,0  ,0  ,0               ,MKeywordSearch},
    {DI_EDIT       ,6  ,12 ,47 ,0  ,0               ,-1},
    {DI_BUTTON     ,0  ,14 ,0  ,0  ,DIF_CENTERGROUP ,MOk},
    {DI_BUTTON     ,0  ,14 ,0  ,0  ,DIF_CENTERGROUP ,MCancel},
  };
  struct FarDialogItem DialogItems[sizeofa(PreDialogItems)];

  InitDialogItems(PreDialogItems,DialogItems,sizeofa(PreDialogItems));
  DialogItems[3].Focus = TRUE;
  struct FarListItem li[sizeofa(hswa)];
  memset(li,0,sizeof(li));
  struct FarList fl = {sizeofa(hswa), li};
  li[HowShowWindow].Flags=LIF_SELECTED;
  for (size_t i=0, msg=MSW_NORMAL; i<sizeofa(hswa); i++, msg++)
    GetMsg(msg,li[i].Text);
  DialogItems[3].ListItems = &fl;
  lstrcpy(DialogItems[6].Data,PathToViewer);
  lstrcpy(DialogItems[8].Data,OpenContents);
  lstrcpy(DialogItems[10].Data,KeywordSearch);
  DialogItems[11].DefaultButton = 1;

  if (Info.Dialog(Info.ModuleNumber,-1,-1,54,17,NULL,(struct FarDialogItem *)&DialogItems,sizeofa(DialogItems)) != 11)
    return;

  HowShowWindow = DialogItems[3].ListPos;
  lstrcpy(PathToViewer,DialogItems[6].Data);
  lstrcpy(OpenContents,DialogItems[8].Data);
  lstrcpy(KeywordSearch,DialogItems[10].Data);

  SetRegKey("HowShowWindow",HowShowWindow);
  SetRegKey("PathToViewer",PathToViewer);
  SetRegKey("OpenContents",OpenContents);
  SetRegKey("KeywordSearch",KeywordSearch);
}
