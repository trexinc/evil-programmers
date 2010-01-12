/*
    Scripts sub-plugin for FARMail
    Copyright (C) 2002-2006 FARMail Group
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
#include "type.hpp"
#include "variant.hpp"
#include "scripts.hpp"
#include "language.hpp"
#include "farkeys.hpp"
#include <stdio.h>

static const char *win_unknown="Windows";
static const char *win_95="Win95";
static const char *win_95osr2="Win95 OSR2";
static const char *win_98="Win98";
static const char *win_98se="Win98 SE";
static const char *win_me="WinME";
static const char *win_nt="WinNT";
static const char *win_2k="Win2K";
static const char *win_xp="WinXP";
static const char *win_32s="Win32s";

static int SetPos(int n,int eol)
{
  EditorSetPosition ep;
  int curpos=0;
  if(eol)
  {
    EditorGetString es;
    es.StringNumber=n;
    if(FInfo.EditorControl(ECTL_GETSTRING,&es))
    {
      curpos=es.StringLength;
    }
    else SayError(ERR_INTERNALERROR,"ECTL_GETSTRING");
  }
  ep.CurLine=n;
  ep.CurPos=curpos;
  ep.CurTabPos=-1;
  ep.TopScreenLine=-1;
  ep.LeftPos=-1;
  ep.Overtype=-1;
  return FInfo.EditorControl(ECTL_SETPOSITION,&ep);
}

static bool ReadLine(HANDLE file,char *buffer,DWORD len)
{
  DWORD CurrPos=SetFilePointer(file,0,NULL,FILE_CURRENT),transferred;
  bool res=true;
  if(ReadFile(file,buffer,len-1,&transferred,NULL)&&transferred)
  {
    buffer[len-1]=0;
    DWORD len=0; char *ptr=buffer;
    for(size_t i=0;i<transferred;i++,len++,ptr++)
    {
      if(buffer[i]=='\n'||buffer[i]=='\r')
      {
        if((buffer[i+1]=='\n'||buffer[i+1]=='\r')&&buffer[i]!=buffer[i+1])
          len++;
        break;
      }
    }
    *ptr=0;
    SetFilePointer(file,CurrPos+len+1,NULL,FILE_BEGIN);
  }
  else
  {
    buffer[0]=0;
    res=false;
  }
  return res;
}

Variant WINAPI blt_nlines(long count,Variant *values,int *stop,void *ptr)
{
  (void)count;
  (void)values;
  (void)ptr;
  Variant result=0;
  EditorInfo ei;
  ei.TotalLines=0;
  if(FInfo.EditorControl(ECTL_GETINFO,&ei))
  {
    result=ei.TotalLines;
  }
  else
  {
    SayError(ERR_INTERNALERROR,"nlines");
    *stop=TRUE;
  }
  return result;
}

Variant WINAPI blt_line(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  Variant result="";
  EditorGetString egs;
  egs.StringNumber=-1;
  if(count) egs.StringNumber=values[0];
  if(FInfo.EditorControl(ECTL_GETSTRING,&egs))
  {
    result=egs.StringText;
    EditorConvertText ect;
    ect.Text=result.get();
    ect.TextLength=result.length();
    FInfo.EditorControl(ECTL_EDITORTOOEM,&ect);
  }
  else
  {
    SayError(ERR_INTERNALERROR,"line");
    *stop=TRUE;
  }
  return result;
}

Variant WINAPI blt_strlen(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  Variant result=0;
  if(count)
  {
    result=values[0].length();
  }
  return result;
}

Variant WINAPI blt_setline(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  char def[1]="";
  EditorSetString ess;
  EditorConvertText ect;
  ess.StringNumber=-1;
  if(count>1) ess.StringNumber=(long)values[1];
  ess.StringText=ect.Text=def;
  ect.TextLength=ess.StringLength=0;
  if(count)
  {
    ess.StringText=ect.Text=values[0].get();
    ect.TextLength=ess.StringLength=values[0].length();
  }
  ess.StringEOL=(char*)"";
  FInfo.EditorControl(ECTL_OEMTOEDITOR,&ect);
  FInfo.EditorControl(ECTL_SETSTRING,&ess);
  return 0;
}

Variant WINAPI blt_print(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  if (!count) return 0;
  if(count>1)
    SetPos((__INT64)values[1],0);
  FInfo.EditorControl(ECTL_INSERTTEXT,values[0].get());
  return 0;
}

Variant WINAPI blt_delline(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  if (count)
    SetPos((__INT64)values[0],0);
  FInfo.EditorControl(ECTL_DELETESTRING,NULL);
  return 0;
}

Variant WINAPI blt_insline(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  int n=-1,eol=0;
  if(count) n=values[0];
  if(count>1) eol=values[1];
  SetPos(n,eol);
  FInfo.EditorControl(ECTL_INSERTSTRING,NULL);
  return 0;
}

Variant WINAPI blt_string(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  Variant result="";
  if(count) result=(const char *)values[0];
  return result;
}

Variant WINAPI blt_integer(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  Variant result=0;
  if(count) result=(__INT64)values[0];
  return result;
}

Variant WINAPI blt_message(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  const char **err=(const char **)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(const char *)*(3+count));
  if(err)
  {
    char tmp[100];
    err[0]="";
    for(int i=0;i<count;i++)
      err[1+i]=values[i];
    err[1+count]="";
    err[2+count]=GetMsg(MesOk,tmp);
    FInfo.Message(FInfo.ModuleNumber,0,NULL,err,3+count,1);
    HeapFree(GetProcessHeap(),0,err);
  }
  return 0;
}

Variant WINAPI blt_char(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  char buf[2]={0,0};
  if(count) buf[0]=(int)values[0];
  return Variant(buf);
}

Variant WINAPI blt_substr(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  Variant result="";
  if(count>1&&(__INT64)values[1]<values[0].length())
  {
    result=(const char *)values[0]+(__INT64)values[1];
    if(count>2&&(__INT64)values[2]<result.length())
      result.set(values[2],0);
  }
  return result;
}

Variant WINAPI blt_strlwr(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  Variant result="";
  if(count)
  {
    result=values[0];
    FSF.LStrlwr(result.get());
  }
  return result;
}

Variant WINAPI blt_strupr(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  Variant result="";
  if(count)
  {
    result=values[0];
    FSF.LStrupr(result.get());
  }
  return result;
}

Variant WINAPI blt_strstr(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  Variant result=-1;
  if(count>1)
  {
    const char *find=strstr(values[0],values[1]);
    if(find)
      result=find-(const char *)values[0];
  }
  return result;
}

Variant WINAPI blt_random(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  Variant result;
  long seed=(1<<15);
  if(count) seed=values[0];
  result=Random(seed);
  return result;
}

Variant WINAPI blt_date(long count,Variant *values,int *stop,void *ptr)
{
  (void)count;
  (void)values;
  (void)ptr;
  (void)stop;
  SYSTEMTIME st;
  char buf[50];
  GetLocalTime(&st);
  FSF.sprintf(buf,"%02d.%02d.%04d",st.wDay,st.wMonth,st.wYear);
  return Variant(buf);
}

Variant WINAPI blt_time(long count,Variant *values,int *stop,void *ptr)
{
  (void)count;
  (void)values;
  (void)ptr;
  (void)stop;
  SYSTEMTIME st;
  char buf[50];
  GetLocalTime(&st);
  FSF.sprintf(buf,"%02d:%02d:%02d",st.wHour,st.wMinute,st.wSecond);
  return Variant(buf);
}

Variant WINAPI blt_boundary(long count,Variant *values,int *stop,void *ptr)
{
  (void)count;
  (void)values;
  (void)ptr;
  (void)stop;
  char buf[50];
  MInfo.InitBoundary(buf);
  return Variant(buf);
}

Variant WINAPI blt_fileline(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  char buf[1000]; buf[0]=0; //avoid memset call
  if(count>1)
  {
    char filename[MAX_PATH];
    lstrcpy(filename,values[0]);
    ExpandFilename(filename);
    DWORD i=(long)values[1]+1;
    HANDLE fp=CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
    if(fp!=INVALID_HANDLE_VALUE)
    {
      size_t x=0;
      while(x++<i&&ReadLine(fp,buf,1000));
      CloseHandle(fp);
      x=lstrlen(buf);
      while(x>0&&(buf[x-1]=='\r'||buf[x-1]=='\n'))
        buf[--x]='\0';
    }
  }
  return Variant(buf);
}

Variant WINAPI blt_filecount(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  int i=0;
  if(count)
  {
    char filename[MAX_PATH];
    lstrcpy(filename,values[0]);
    ExpandFilename(filename);
    HANDLE fp=CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if(fp!=INVALID_HANDLE_VALUE)
    {
      i=1;
      DWORD transfered;
      for(char c[]="";ReadFile(fp,c,1,&transfered,NULL)&&transfered;(c[0]=='\n'?i++:i)) ;
      CloseHandle(fp);
    }
  }
  return i;
}

Variant WINAPI blt_blktype(long count,Variant *values,int *stop,void *ptr)
{
  (void)count;
  (void)values;
  (void)ptr;
  (void)stop;
  EditorInfo ei;
  FInfo.EditorControl(ECTL_GETINFO,(void *)&ei);
  return (ei.BlockType==BTYPE_NONE?0:(ei.BlockType==BTYPE_STREAM?1:2));
}

Variant WINAPI blt_blkstart(long count,Variant *values,int *stop,void *ptr)
{
  (void)count;
  (void)values;
  (void)ptr;
  (void)stop;
  EditorInfo ei;
  FInfo.EditorControl(ECTL_GETINFO,(void *)&ei);
  return ei.BlockStartLine;
}

Variant WINAPI blt_selstart(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  long num=-1,result=-1;
  if(count) num=values[0];
  EditorGetString egs;
  egs.StringNumber=num;
  if(FInfo.EditorControl(ECTL_GETSTRING,(void *)&egs))
  {
    result=egs.SelStart;
  }
  else
  {
    SayError(ERR_INTERNALERROR,"selstart");
    *stop=TRUE;
  }
  return result;
}

Variant WINAPI blt_selend(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  long num=-1,result=-1;
  if(count) num=values[0];
  EditorGetString egs;
  egs.StringNumber=num;
  if(FInfo.EditorControl(ECTL_GETSTRING, (void *)&egs))
  {
    result=egs.SelEnd;
  }
  else
  {
    SayError(ERR_INTERNALERROR,"selend");
    *stop=TRUE;
  }
  return result;
}

Variant WINAPI blt_setsel(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  if(count==0 || count>4)
  {
    EditorSelect es;
    int type=BTYPE_NONE;
    if (count)
    {
      type=values[0];
      type=(type==0?BTYPE_NONE:(type==1?BTYPE_STREAM:BTYPE_COLUMN));

      es.BlockStartLine=values[1];
      es.BlockStartPos=values[2];
      es.BlockWidth=values[3];
      es.BlockHeight=values[4];
    }
    es.BlockType=type;
    if(!FInfo.EditorControl(ECTL_SELECT, (void *)&es))
    {
      SayError(ERR_INTERNALERROR,"setsel");
      *stop=TRUE;
    }
  }
  return 0;
}

Variant WINAPI blt_usrinput(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;

  const char *UserTitle="";
  const char *UserLabel="Input:";
  const char *UserData="";
  const char *HistoryUserInput="FMPScriptsFuncUsrInput";
  if(count) UserTitle=values[0];
  if(count>1) UserLabel=values[1];
  if(count>2) UserData=values[2];
  if(count>3) HistoryUserInput=values[3];
  InitDialogItem InitItems[]=
  {
    // type,x1,y1,x2,y2,focus,selected,flags,default,data
    {DI_DOUBLEBOX,3,1,57,4,0,0,0,0,UserTitle},
    {DI_TEXT,5,2,0,0,0,0,0,0,UserLabel},
    {DI_EDIT,5,3,55,0,1,(DWORD)HistoryUserInput,DIF_HISTORY,0,UserData},
    {DI_BUTTON,3,5,0,0,0,0,0,1,(char *)MesOk},
  };
  FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));

  int key=FInfo.Dialog(FInfo.ModuleNumber,-1,-1,60,6,NULL,DialogItems,sizeofa(DialogItems));
  return (key==3)?DialogItems[2].Data:"";
}

Variant WINAPI blt_redirect(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  if(count)
  {
    long mode=0;
    if(count>1) mode=values[1];
    char FileName[MAX_PATH];
    if(mode%2==0)
    {
      char dest[MAX_PATH];
      lstrcpy(dest,values[0]);
      FSF.AddEndSlash(dest);
      int i=MInfo.GetFreeNumber(dest);
      if(i>0) FSF.sprintf(FileName,"%s%08ld.%s",dest,i,Opt.EXT);
    }
    else
    {
      lstrcpy(FileName,values[0]);
    }
    bool modal=FInfo.AdvControl(FInfo.ModuleNumber,ACTL_GETWINDOWCOUNT,0)==1;
    HANDLE fp=CreateFile(FileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
    if(fp!=INVALID_HANDLE_VALUE)
    {
      EditorGetString egs;
      EditorSetPosition esp;
      EditorInfo ei;
      FInfo.EditorControl(ECTL_GETINFO,(void *)&ei);
      esp.CurPos=-1;
      esp.TopScreenLine=-1;
      esp.LeftPos=-1;
      esp.CurTabPos=-1;
      esp.Overtype=-1;
      for(esp.CurLine=0,egs.StringNumber=-1;esp.CurLine<ei.TotalLines;esp.CurLine++)
      {
        DWORD transfered;
        FInfo.EditorControl(ECTL_SETPOSITION,&esp);
        FInfo.EditorControl(ECTL_GETSTRING,&egs);
        WriteFile(fp,egs.StringText,egs.StringLength,&transfered,NULL);
        if(*(egs.StringEOL))
          WriteFile(fp,egs.StringEOL,lstrlen(egs.StringEOL),&transfered,NULL);
        else
          if(esp.CurLine<ei.TotalLines-1)
            WriteFile(fp,"\r\n",2,&transfered,NULL);
      }
      CloseHandle(fp);
      esp.CurLine=ei.CurLine;
      esp.CurPos=ei.CurPos;
      esp.TopScreenLine=ei.TopScreenLine;
      esp.LeftPos=ei.LeftPos;
      FInfo.EditorControl(ECTL_SETPOSITION,&esp);
      if(mode<2)
        FInfo.EditorControl(ECTL_QUIT,NULL);
      if(!modal && mode<4)
      {
        FInfo.Editor(FileName,NULL,0,0,-1,-1,EF_NONMODAL|EF_IMMEDIATERETURN|EF_ENABLE_F6,0,1);
        FInfo.EditorControl(ECTL_SETPOSITION,&esp);
      }
      if(modal || mode>3)
      {
        char *ptr=FSF.PointToName(FileName);
        char save=*ptr;
        *ptr=0;
        FInfo.Control(INVALID_HANDLE_VALUE,FCTL_SETANOTHERPANELDIR,FileName);
        *ptr=save;
        PanelInfo pi;
        FInfo.Control(INVALID_HANDLE_VALUE,FCTL_GETANOTHERPANELINFO,&pi);
        PanelRedrawInfo pr={pi.CurrentItem,pi.TopPanelItem};
        for(int i=0;i<pi.ItemsNumber;i++)
          if(!lstrcmpi(ptr,pi.PanelItems[i].FindData.cFileName))
          {
            pr.CurrentItem = i;
            break;
          }
        FInfo.Control(INVALID_HANDLE_VALUE,FCTL_REDRAWANOTHERPANEL,&pr);
        if(modal && mode<2)
        {
          DWORD key[]={KEY_TAB,KEY_F4};
          KeySequence ks={KSFLAGS_DISABLEOUTPUT,2,key};
          FInfo.AdvControl(FInfo.ModuleNumber,ACTL_POSTKEYSEQUENCE,&ks);
          FInfo.AdvControl(FInfo.ModuleNumber,ACTL_COMMIT,0);
        }
        if(modal && mode<4)
          *stop=TRUE;
      }
    }
  }
  return 0;
}

Variant WINAPI blt_version(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  long type=0;
  if(count) type=values[0];
  char buffer[1024];
  *buffer=0;
  switch(type)
  {
    case 0:
      FSF.sprintf(buffer,"FARMail %d.%d build %d",MInfo.GetValue(FMVAL_VERSION_MAJOR),MInfo.GetValue(FMVAL_VERSION_MINOR),MInfo.GetValue(FMVAL_VERSION_BUILD));
      break;
    case 1:
      {
        DWORD ver;
        FInfo.AdvControl(FInfo.ModuleNumber,ACTL_GETFARVERSION,&ver);
        FSF.sprintf(buffer,"FAR %d.%d build %d",HIBYTE(ver),LOBYTE(ver),HIWORD(ver));
      }
      break;
    case 2:
      {
        OSVERSIONINFO ver;
        ver.dwOSVersionInfoSize=sizeof(ver);
        if(GetVersionEx(&ver))
        {
          const char *name; DWORD build=ver.dwBuildNumber&0xFFFF;
          char format[30]; lstrcpy(format,"%s %d.%d build %d"); //avoid memset call
          if (*ver.szCSDVersion)
            lstrcat(format," %s");
          name=win_unknown;
          if(ver.dwPlatformId&VER_PLATFORM_WIN32_NT)
          {
            switch (ver.dwMajorVersion)
            {
              case 3:
              case 4:
                name=win_nt;
                break;
              case 5:
                switch (ver.dwMinorVersion)
                {
                  case 0:
                    name=win_2k;
                    break;
                  case 1:
                    name=win_xp;
                    break;
                }
                break;
            }
            build=ver.dwBuildNumber;
          }
          else if(ver.dwPlatformId&VER_PLATFORM_WIN32_WINDOWS)
          {
            if (ver.dwMajorVersion==4)
            {
              switch (ver.dwMinorVersion)
              {
                case 0:
                  if (!lstrcmp(ver.szCSDVersion,"C"))
                    name=win_95osr2;
                  else
                    name=win_95;
                  break;
                case 10:
                  if (!lstrcmp(ver.szCSDVersion,"A"))
                    name=win_98se;
                  else
                    name=win_98;
                  break;
                case 90:
                  name=win_me;
                  break;
              }
            }
          }
          else
            name=win_32s;
          FSF.sprintf(buffer,format,name,ver.dwMajorVersion,ver.dwMinorVersion,build,ver.szCSDVersion);
        }
      }
      break;
  }
  return Variant(buffer);
}

Variant WINAPI blt_setpos(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  long result=0;
  int ret=0;
  long line=-1,col=-1;
  if(count) line=values[0];
  if(count>1) col=values[1];
  if (count && line==-1 && col==-1)
  {
    EditorInfo ei;
    ret=FInfo.EditorControl(ECTL_GETINFO,&ei);
    if (ret)
      result=(count==1?ei.CurLine:ei.CurPos);
  }
  else
  {
    EditorSetPosition esp={line,col,-1,-1,-1,-1};
    ret=FInfo.EditorControl(ECTL_SETPOSITION,(void *)&esp);
  }
  if(!ret)
  {
    SayError(ERR_INTERNALERROR,"setpos");
    *stop=TRUE;
  }
  return result;
}

Variant WINAPI blt_header(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  Variant result="";
  if(count>1)
  {
    char *buffer=(char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,values[0].length()+1);
    if(buffer)
    {
      if(MInfo.GetHeaderField(values[0],values[1],buffer,values[0].length()+1)) result=buffer;
      HeapFree(GetProcessHeap(),0,buffer);
    }
  }
  return result;
}

Variant WINAPI blt_usrmenu(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  Variant result=-1;
  if(count)
  {
    FarMenuItem *MenuItems=(FarMenuItem *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(FarMenuItem)*count);
    if(MenuItems)
    {
      for(int i=0;i<count;i++)
      {
        lstrcpy(MenuItems[i].Text,values[i]);
        MenuItems[i].Selected=MenuItems[i].Checked=MenuItems[i].Separator=FALSE;
      }
      MenuItems[0].Selected=TRUE;
      result=FInfo.Menu(FInfo.ModuleNumber,-1,-1,0,FMENU_WRAPMODE,NULL,NULL,NULL,NULL,NULL,MenuItems,count);
      HeapFree(GetProcessHeap(),0,MenuItems);
    }
  }
  return result;
}

static bool IfFoobarStopped(char *title)
{
  DWORD *d_title=(DWORD *)title;
  return d_title[0]==0x626f6f66&&d_title[1]==0x30327261&&d_title[2]==0x76203030;
}

/*
  -1 = winamp not found
  0 = stop
  1 = play
  3 = pause
*/
Variant WINAPI blt_winampstate(long count,Variant *values,int *stop,void *ptr)
{
  (void)count;
  (void)values;
  (void)ptr;
  (void)stop;
  Variant result=-1;
  HWND pWnd=FindWindow("Winamp v1.x",NULL);
  if(pWnd)
  {
    result=SendMessage(pWnd,WM_USER,0,104);
  }
  else
  {
    pWnd=FindWindow("{DA7CD0DE-1602-45e6-89A1-C2CA151E008E}",NULL);
    if(pWnd)
    {
      result=1;
      char buffer[1024];
      if(GetWindowText(pWnd,buffer,sizeof(buffer))&&IfFoobarStopped(buffer)) result=0;
    }
  }
  return result;
}

//Apollo consts
#define WM_APOLLO_COMMAND WM_USER+3
#define APOLLO_GETCURRENTLYPLAYEDTITLE 25

//Winamp consts
#define IPC_GETLISTPOS 125
#define IPC_GETPLAYLISTTITLE 212

Variant WINAPI blt_winampsong(long count,Variant *values,int *stop,void *ptr)
{
  (void)count;
  (void)values;
  (void)ptr;
  (void)stop;
  Variant result="";
  char buffer[1024];
  HWND pWnd=FindWindow("Winamp v1.x",NULL);
  if(pWnd)
  {
    unsigned int version=SendMessage(pWnd,WM_USER,0,0);
    if(version>0x2050) //Winamp 2.05+ or others
    {
      if(version>=0xa000) //Apollo
      {
        pWnd=FindWindow("Apollo - Main Window",NULL);
        if(pWnd)
        {
          HWND hTempWnd=(HWND)SendMessage(pWnd,WM_APOLLO_COMMAND,(WPARAM)MAKELONG(APOLLO_GETCURRENTLYPLAYEDTITLE,0),(LPARAM)0);
          SendMessage(hTempWnd,WM_GETTEXT,(WPARAM)sizeof(buffer),(LPARAM)buffer);
          CharToOem(buffer,buffer);
          result=buffer;
        }
      }
      else
      {
        HWND sWnd=FindWindow("STUDIO",NULL);
        if(sWnd) //STP?
        {
          if(GetWindowText(sWnd,buffer,sizeof(buffer)))
          {
            char *p = buffer + lstrlen(buffer);
            while (*p != ' ' && p > buffer)
              --p;
            *p = 0;
            CharToOem(buffer,buffer);
            result=buffer;
          }
          pWnd = sWnd;
        }
        else //Winamp 2.05+
        {
          DWORD dProcessID;
          if(GetWindowThreadProcessId(pWnd,&dProcessID))
          {
            HANDLE hProcess=OpenProcess(PROCESS_VM_READ,0,dProcessID);
            if(hProcess)
            {
              char* pcTitle=(char*)SendMessage(pWnd,WM_USER,SendMessage(pWnd,WM_USER,0,IPC_GETLISTPOS),IPC_GETPLAYLISTTITLE);
              ReadProcessMemory(hProcess,pcTitle,buffer,sizeof(buffer),0);
              CharToOem(buffer,buffer);
              result=buffer;
            }
          }
        }
      }
    }

    if(result=="") //from window title
    {
      if(pWnd)
      {
        if(GetWindowText(pWnd,buffer,sizeof(buffer)))
        {
          if(lstrlen(buffer)>9)
            if(!lstrcmp(buffer+lstrlen(buffer)-8,"- Winamp"))
              buffer[lstrlen(buffer)-9]=0;
          CharToOem(buffer,buffer);
          result=buffer;
        }
      }
    }
  }
  else
  {
    pWnd=FindWindow("{DA7CD0DE-1602-45e6-89A1-C2CA151E008E}",NULL);
    if(pWnd&&GetWindowText(pWnd,buffer,sizeof(buffer))&&!IfFoobarStopped(buffer))
    {
      CharToOem(buffer,buffer);
      result=buffer;
    }
  }
  return result;
}

Variant WINAPI blt_addressbook(long count,Variant *values,int *stop,void *ptr)
{
  (void)count;
  (void)values;
  (void)ptr;
  (void)stop;
  char email[512];
  *email=0;
  MInfo.AddressBook(email);
  return Variant(email);
}

Variant WINAPI blt_exit(long count,Variant *values,int *stop,void *ptr)
{
  (void)count;
  (void)values;
  (void)ptr;

  *stop=TRUE;
  return 0;
}

// Поддержка только long. long long пока мимо.
Variant WINAPI blt_sprintf(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  if(count<1) return "";
  if(count>1)
  {
    char buffer[8*1024];
    char **args=(char **)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(char *)*(count-1));
    if(args)
    {
      for(int i=1;i<count;i++)
      {
        switch(values[i].gettype())
        {
          case vtNull:
            args[i-1]=NULL;
            break;
          case vtInt64:
            args[i-1]=(char *)((long)values[i]);
            break;
          case vtString:
            args[i-1]=values[i].get();
            break;
        }
      }
      wvsprintf(buffer,values[0],(char *)args);
      HeapFree(GetProcessHeap(),0,args);
      return buffer;
    }
  }
  return values[0];
}

Variant WINAPI blt_encodeheader(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  Variant result="";
  if(count>1 && lstrlen(values[0])>0)
  {
    size_t buffer_size;
    MInfo.EncodeHeader(values[0],NULL,&buffer_size,values[1]);
    char *buffer=(char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,buffer_size);
    if(buffer)
    {
      MInfo.EncodeHeader(values[0],buffer,&buffer_size,values[1]);
      result=buffer;
      HeapFree(GetProcessHeap(),0,buffer);
    }
  }
  return result;
}

Variant WINAPI blt_editorstate(long count,Variant *values,int *stop,void *ptr)
{
  (void)count;
  (void)values;
  (void)ptr;
  (void)stop;
  char result[]="000";
  EditorInfo ei;
  FInfo.EditorControl(ECTL_GETINFO,(void *)&ei);
  if (ei.CurState&ECSTATE_MODIFIED)
    result[0]='1';
  if (ei.CurState&ECSTATE_SAVED)
    result[1]='1';
  if (ei.CurState&ECSTATE_LOCKED)
    result[2]='1';
  return result;
}

Variant WINAPI blt_getini(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  Variant result="";
  if(count>3)
  {
    char filename[MAX_PATH];
    lstrcpy(filename,values[3]);
    ExpandFilename(filename);
    if (values[2].gettype()==vtString)
    {
      char temp[32*1024];
      temp[0]=0;
      GetPrivateProfileString(values[0],values[1],values[2],temp,sizeof(temp),filename);
      result=temp;
    }
    else if (values[2].gettype()==vtInt64)
    {
      result=(long)GetPrivateProfileInt(values[0],values[1],(long)values[2],filename);
    }
  }
  return result;
}

Variant WINAPI blt_setini(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  long result=0;
  if(count>1)
  {
    char filename[MAX_PATH];
    lstrcpy(filename,values[0]);
    ExpandFilename(filename);
    result=WritePrivateProfileString(values[1],(count>2?(const char *)values[2]:NULL),(count>3?(const char *)values[3]:NULL),filename);
  }
  return result;
}

Variant WINAPI blt_rtrim(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  Variant result="";
  if(count)
  {
    int n;
    for (n=values[0].length()-1;n>=0;n--)
    {
      if ((values[0][n]!=9)&&(values[0][n]!=32))
      {
        result=(const char *)values[0];
        result.set(n+1,0);
        break;
      }
    }
  }
  return result;
}

Variant WINAPI blt_ltrim(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  Variant result="";
  if(count)
  {
    int n;
    for (n=0;n<values[0].length();n++)
    {
      if ((values[0][n]!=9)&&(values[0][n]!=32))
      {
        result=(const char *)values[0]+n;
        break;
      }
    }
  }
  return result;
}

Variant WINAPI blt_trim(long count,Variant *values,int *stop,void *ptr)
{
  (void)ptr;
  (void)stop;
  Variant result="";
  if(count)
  {
    int n,e;
    for (n=0;n<values[0].length();n++)
    {
      if ((values[0][n]!=9)&&(values[0][n]!=32))
        break;
    }
    for (e=values[0].length()-1;e>=0;e--)
    {
      if ((values[0][e]!=9)&&(values[0][e]!=32))
        break;
    }
    result=(const char *)values[0]+n;
    result.set(e-n+1,0);
  }
  return result;
}
