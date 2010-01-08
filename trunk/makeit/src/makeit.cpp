/*
  Copyright (C) 2000 Konstantin Stupnik

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


#define _WINCON_
#include <windows.h>
#undef _WINCON_
#pragma pack(push,4)
#include <wincon.h>
#pragma pack(pop)

#include "plugin.hpp"
#include <stdio.h>
#include "makeit.h"
#include <string.h>
#include <time.h>

static struct PluginStartupInfo I;

enum{
  MODE_COMMANDLINE,MODE_EDITOR,
};

static int loadedok=0;
static int screenwidth,screenheight;
static char *linebuf;
static int linebufwidth=0;

static int msgwndheight=20;

static int wasswitch=0;

static StrList log;
static CLineList ll;



static int mode=MODE_COMMANDLINE;
static int toppos=0;
static int linepos=0;
static int leftpos=0;
static String parser;
static String cmddir;
static String command;

static String cfgfile;
static FILETIME cfglastwrite;


static String logfile;
static FILETIME loglastwrite;

static SColors colors={0,7,12,10,15};
static SOptions opt;

static String plugdir;


void Scroll();

//int shownumbers=0;

int Msg(const char* err)
{
  const char *msg[3]={APPNAME,"","Ok"};
  msg[1]=err;
  if(!err)
  {
    msg[0] =  "Wrong argument!" ;
    msg[1] =  "Msg" ;
    I.Message(I.ModuleNumber,0,NULL,(char**)msg,3,1);
  }
  else
    I.Message(I.ModuleNumber,0,NULL,msg,3,1);
  return 0;
}
int Msg2(char* header,char* err)
{
  const char *msg[]={"","","Ok"};
  msg[0]=header;
  msg[1]=err;
  if(!err || !header)
  {
    msg[0] =  "Wrong argument!" ;
    msg[1] =  "Msg2" ;
  I.Message(I.ModuleNumber,0,NULL,msg,3,1);
  }
  else
  I.Message(I.ModuleNumber,0,NULL,msg,3,1);
  return 0;
}

int Menu(const char *title,StrList& lst,int sel)
{
  FarMenuItem *menu=new FarMenuItem[lst.Count()];
  int i=0;
  int j=0;
  char buf[16];
  ZeroMemory(menu,sizeof(FarMenuItem)*lst.Count());
  for(i=0;i<lst.Count();i++)
  {
    sprintf(buf,"&%d ",j+1);
    strcpy(menu[j].Text,buf);
    strcat(menu[j].Text,lst[i].Substr(0,120));
    if(sel==j)menu[j].Selected=1;
    j++;
  }
  int res=I.Menu(I.ModuleNumber,-1,-1,20,FMENU_WRAPMODE,title,NULL,"content",NULL,NULL,menu,lst.Count());
  delete [] menu;
  return res;
}

static void InitLineBuf()
{
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&csbi);
  screenwidth=csbi.dwSize.X;
  screenheight=csbi.dwSize.Y;
  if(linebufwidth!=screenwidth*4)
  {
    if(linebuf)delete linebuf;
    linebuf=new char[screenwidth*4+1];
    linebufwidth=screenwidth*4;
  }
}

const char*
GetMsg(int MsgId)
{
  return(I.GetMsg(I.ModuleNumber,MsgId));
}




void GetCfgTime(FILETIME& tm)
{
  HANDLE h=CreateFile(cfgfile,GENERIC_READ,
          FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
  GetFileTime(h,NULL,NULL,&tm);
  CloseHandle(h);
}

FARAPI(void) SetStartupInfo(const struct PluginStartupInfo *Info)
{
  I=*Info;
  char fname[255];
  char * s;
  strcpy(fname,Info->ModuleName);

  s=strrchr(fname,'\\');
  if(s)
  {
    s++;
    *s=0;
    plugdir=fname;
    strcat(fname,"makeit.xml");
    cfgfile=fname;
  }
  if(!read_config(fname))
  {
    Msg(GetMsg(MConfigError));
    return;
  }
  if(!init_config(&colors,&opt))
  {
    return;
  }
  GetCfgTime(cfglastwrite);
  loadedok=1;
}

HANDLE ClearScreen(int save=1)
{
  HANDLE hScr=NULL;
  InitLineBuf();

  FillMemory(linebuf,linebufwidth,' ');
  linebuf[linebufwidth]=0;

  if(save)hScr=I.SaveScreen(0,0,-1,-1);
  for(int i=mode==MODE_COMMANDLINE?0:screenheight-msgwndheight;i<screenheight;i++)
  {
    I.Text(0,i,colors.bg<<4,linebuf);
  };
  I.Text(0,0,0,NULL);
  return hScr;
}

char *CurDir()
{
  static char buf[MAX_PATH];
  GetCurrentDirectory(sizeof(buf),buf);
  return buf;
}

void Draw(int from,int cursor=0);

class HideCur{
  CONSOLE_CURSOR_INFO  ci;
public:
  HideCur(int hide)
  {
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&ci);
    CONSOLE_CURSOR_INFO newci;
    newci.dwSize=25;
    newci.bVisible=0;
    if(hide)
    {
      SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&newci);
    }
  }
  HideCur()
  {
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&ci);
    CONSOLE_CURSOR_INFO newci;
    newci.dwSize=25;
    newci.bVisible=0;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&newci);
  }
  void Restore()
  {
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&ci);
  }
  ~HideCur()
  {
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&ci);
  }
};

void SetStartPos()
{
  int lastl=0;
  for(linepos=0;linepos<ll.Count();linepos++)
  {
    if(ll[linepos] && ll[linepos]->error)lastl=linepos;
    if(!ll[linepos] || ll[linepos]->error!=1)continue;
    break;
  }
  if(linepos>=log.Count())linepos=lastl;
}

void MakeIt(char *cmdline,SCommand* cmd=NULL)
{
  HANDLE hScr;
  int i;

  if(cmdline && cmd==NULL)
  {
    CCmdList cmds;
    String cmd;
    int cnt=get_commands(cmdline,cmds);
    if(cnt==0)
    {
      Msg(GetMsg(MNoFileType));
      return;
    }
    if(cnt>1)
    {
      StrList menu;
      for(i=0;i<cmds.Count();i++)
      {
        menu<<cmds[i]->name;
      }
      int cmdidx=Menu(GetMsg(MCommands),menu,0);
      if(cmdidx==-1)return;
      command=cmds[cmdidx]->command;
      parser=cmds[cmdidx]->parser;
    }else
    {
      command=cmds.Get()->command;
      parser=cmds.Get()->parser;
    }
  }
  else
  {
    if(cmd!=NULL)
    {
      command=cmd->command;
      parser=cmd->parser;
    }
  }
  //Msg(command);
  {
    //DebugBreak();
    WindowInfo wi;
    int i;
    int n=I.AdvControl(I.ModuleNumber,ACTL_GETWINDOWCOUNT,0);
    for(i=0;i<n;i++)
    {
      wi.Pos=i;
      I.AdvControl(I.ModuleNumber,ACTL_GETWINDOWINFO,&wi);
      if(wi.Type!=WTYPE_EDITOR)continue;
      if(wi.Modified)
      {
        I.AdvControl(I.ModuleNumber,ACTL_SETCURRENTWINDOW,(void*)i);
        I.AdvControl(I.ModuleNumber,ACTL_COMMIT,0);
        if(!I.EditorControl(ECTL_SAVEFILE,NULL))return;
      }
    }
  }


  InitLineBuf();

  hScr=ClearScreen();

  log.Clean();
  ll.Clean();
  linebuf[0]=0;
  linepos=0;
  leftpos=0;

  cmddir=CurDir();
  int err;
  {
    HideCur hc;
    int start=time(NULL);
    int len=ExpandEnvironmentStrings(command,NULL,0);
    char *cmd=new char[len+1];
    ExpandEnvironmentStrings(command,cmd,len+1);

    err=pipesrv(cmd);
    delete [] cmd;
    start=time(NULL)-start;
    if(opt.beep && opt.beeptime<start)
    {
      if(opt.wave.Length()==0)
      {
        MessageBeep(MB_OK);
      }else
      {
        String file=opt.wave;
        file.Replace("/","\\");
        if(file[0]!='\\' && file[1]!=':')file=plugdir+"\\"+file;

        PlaySound(file,NULL,SND_FILENAME|SND_ASYNC|SND_APPLICATION);
      }
    }
  }
  toppos=0;
  leftpos=0;
  SetStartPos();
  if(linepos-toppos>screenheight)toppos=linepos-screenheight/2;

  if(err==0)
  {
    SetConsoleTitle(GetMsg(MCompleted));
    for(i=0;i<6;i++)
    {
      Draw(0,i&1);
      Sleep(50);
    }
    Scroll();
  }
  else
    Msg(GetMsg(err));
  I.RestoreScreen(hScr);
  if(wasswitch)
  {
    I.EditorControl(ECTL_REDRAW,NULL);
    wasswitch=0;
  }
}

void ReloadLog()
{
//  Msg(logfile);
  if(logfile=="")return;
  FILETIME tm;
  HANDLE h=CreateFile(logfile,GENERIC_READ,
          FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
  if(h==INVALID_HANDLE_VALUE)return;
  GetFileTime(h,NULL,NULL,&tm);
  CloseHandle(h);
  if(!CompareFileTime(&tm,&loglastwrite))return;
  log.Clean();
  ll.Clean();
  if(!log.LoadFromFile(logfile))
  {
    Msg(GetMsg(MLogFailed));
    return;
  }
  int i;
  SLineInfo li;
  for(i=0;i<log.Count();i++)
  {
    if(match_line(parser,log[i],&li))
    {
      SLineInfo *tmp=new SLineInfo;
      *tmp=li;
      ll<<tmp;
    }else
    {
      ll<<NULL;
    }
  }
  SetStartPos();
}


FARAPI(HANDLE) OpenPlugin(int OpenFrom,int Item)
{
  HANDLE hScr;
  int i;
  int noprefix=0;
  String savedir;
  PanelInfo pi;
//  DebugBreak();
  char filename[MAX_PATH];
  FILETIME tm;
  GetCfgTime(tm);
  if(CompareFileTime(&tm,&cfglastwrite))
  {
    free_config();
    if(!read_config(cfgfile))
    {
      Msg("Failed to load config!");
      return INVALID_HANDLE_VALUE;
    }
    init_config(&colors,&opt);
    cfglastwrite=tm;
  }

  if(OpenFrom==OPEN_EDITOR)
  {
    StrList menu;
    menu<<GetMsg(MShowErrors)<<GetMsg(MMakeLast)<<GetMsg(MMakeCurrent);
    int sel=Menu(GetMsg(MMakeActions),menu,0);
    if(sel==-1)return INVALID_HANDLE_VALUE;
    mode=MODE_EDITOR;
    if(sel==0)
    {
      ReloadLog();
      if(log.Count()==0)return INVALID_HANDLE_VALUE;
      InitLineBuf();
      hScr=ClearScreen();
      Scroll();
      I.RestoreScreen(hScr);
    }else
    if(sel==2)
    {
      logfile="";
      EditorInfo ei;
      I.EditorControl(ECTL_GETINFO,&ei);
      strcpy(filename,ei.FileName);
      if(filename[1]==':' || filename[0]=='\\' || filename[0]=='/')
      {
        cmddir=filename;
        i=cmddir.Length()-1;
        while(i>0 && cmddir[i]!='\\' && cmddir[i]!='/' && cmddir[i]!=':')i--;
        cmddir=cmddir.Substr(0,i+(cmddir[i]==':'?1:0));
      }
      else cmddir=CurDir();
      MakeIt(filename);
    }else
    if(sel==1)
    {
      logfile="";
      savedir=CurDir();
      SetCurrentDirectory(cmddir);
      MakeIt(NULL);
      SetCurrentDirectory(savedir);
    }
  }else if(OpenFrom==OPEN_PLUGINSMENU)
  {
    StrList menu;
    menu<<GetMsg(MShowErrors)<<GetMsg(MMakeLast)<<GetMsg(MMakeAtCur)<<GetMsg(MOpenLog)<<GetMsg(MSaveLog);
    int sel=Menu(GetMsg(MMakeActions),menu,0);
    if(sel==-1)return INVALID_HANDLE_VALUE;
    switch(sel)
    {
      case 1:{
        if(command=="")return INVALID_HANDLE_VALUE;
        logfile="";
        OpenFrom=OPEN_COMMANDLINE;
        Item=0;
        savedir=CurDir();
        SetCurrentDirectory(cmddir);
      }break;
      case 0:{
        if(log.Count()==0)return INVALID_HANDLE_VALUE;
        ReloadLog();
        mode=MODE_COMMANDLINE;
        hScr=ClearScreen();
        Scroll();
        I.RestoreScreen(hScr);
        if(wasswitch)
        {
          I.EditorControl(ECTL_REDRAW,NULL);
          wasswitch=0;
        }
        return INVALID_HANDLE_VALUE;
      }break;
      case 3:{
        //DebugBreak();
        mode=MODE_COMMANDLINE;

        I.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&pi);

        log.Clean();
        ll.Clean();


        if(pi.ItemsNumber<=0)return INVALID_HANDLE_VALUE;
        logfile=CurDir();
        logfile+="\\";
        logfile+=pi.PanelItems[pi.CurrentItem].FindData.cFileName;
        cmddir=CurDir();
        if(!log.LoadFromFile(logfile))
        {
          Msg(GetMsg(MLogFailed));
          return INVALID_HANDLE_VALUE;
        }
        HANDLE h=CreateFile(logfile,GENERIC_READ,
                FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
        GetFileTime(h,NULL,NULL,&loglastwrite);
        CloseHandle(h);

        SLineInfo li;
        StrList pars;
        get_parsers(pars);
        StrList menu;
//        pars.Sort(menu);
        for(i=0;i<pars.Count();i++)
        {
          for(int j=0;j<log.Count();j++)
          {
            if(match_line(pars[i],log[j],&li))
            {
//              Msg(pars[i]);
              menu<<pars[i];
              break;
            }
          }
        }
        if(menu.Count()==0)
        {
          Msg(GetMsg(MNoParsersFound));
          //return INVALID_HANDLE_VALUE;
          pars.Sort(menu);
        }
        if(menu.Count()>1)
        {
//          menu.Sort(menu);
          int sel=Menu(GetMsg(MSelectParser),menu,0);
          if(sel==-1)
          {
            ll.Clean();
            for(i=0;i<log.Count();i++)
            {
              ll<<NULL;
            }
            return INVALID_HANDLE_VALUE;
          }
          parser=menu[sel];
        }else
        {
          parser=menu[0];
        }
        for(i=0;i<log.Count();i++)
        {
          if(match_line(parser,log[i],&li))
          {
            SLineInfo *tmp=new SLineInfo;
            *tmp=li;
            ll<<tmp;
          }else
          {
            ll<<NULL;
          }
        }
        toppos=0;
        linepos=0;
        SetStartPos();
        hScr=ClearScreen();
        Scroll();
        I.RestoreScreen(hScr);
        if(wasswitch)
        {
          I.EditorControl(ECTL_REDRAW,NULL);
          wasswitch=0;
        }
        return INVALID_HANDLE_VALUE;
      }break;
      case 4:{
        char buf[256];
        int res=I.InputBox(GetMsg(MSaveLog),GetMsg(MFilename),NULL,"",buf,sizeof(buf),NULL,FIB_EXPANDENV|FIB_BUTTONS);
        if(!res)return INVALID_HANDLE_VALUE;
        if(!log.SaveToFile(buf))Msg(GetMsg(MSaveFailed));
      }break;
      case 2:{
        logfile="";
        I.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&pi);
        Item=(int)pi.PanelItems[pi.CurrentItem].FindData.cFileName;
        noprefix=1;
        OpenFrom=OPEN_COMMANDLINE;
      }break;
    }
  }

  if(OpenFrom==OPEN_COMMANDLINE)
  {
    logfile="";
    mode=MODE_COMMANDLINE;
    char *cmdline=(char*)Item;
    if(!cmdline || noprefix)
    {
      MakeIt(cmdline);
    }else
    {
      char *cmd=strchr(cmdline,':');
      if(!cmd)return INVALID_HANDLE_VALUE;
      if(_strnicmp(cmdline,"make",4))
      {
        String pfx=cmdline;
        pfx.Delete(cmd-cmdline);
        pfx.ToLower();
        SCommand scmd;
        if(!set_prefix(pfx,cmd+1,scmd))
        {
          Msg("Command doesn't match prefix");
          return INVALID_HANDLE_VALUE;
        }else
        {
          MakeIt(cmd,&scmd);
        }
      }else
      {
        cmd++;
        MakeIt(cmd);
      }
    }
    if(savedir.Length())
    {
      SetCurrentDirectory(savedir);
      savedir="";
    }
  }

  return INVALID_HANDLE_VALUE;
}

FARAPI(void) GetPluginInfo(struct PluginInfo *pi)
{
  static const char *PluginMenuStrings[1];
  static const char *PluginConfigStrings[1];
  pi->StructSize=sizeof(struct PluginInfo);
  pi->Flags=PF_EDITOR|PF_FULLCMDLINE;
  pi->DiskMenuStringsNumber=0;
  PluginMenuStrings[0]=GetMsg(MPlugin);
  pi->PluginMenuStrings=PluginMenuStrings;
  pi->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  PluginConfigStrings[0]=GetMsg(MReloadCfg);
  pi->PluginConfigStrings=PluginConfigStrings;
  pi->PluginConfigStringsNumber=1;
  String prefix="make";
  prefix+=get_prefixes();
  prefix.ToLower();
  static char pfx[1024];
  strcpy(pfx,prefix);
  pi->CommandPrefix=pfx;
}

FARAPI(void) ExitFAR()
{
  free_config();
}


void Draw(int from,int cursor)
{
  int l,c;
//  DebugBreak();
  char *buf=new char[screenwidth+1];
  SLineInfo *li;
  int more=0;
  int drawfrom=mode==MODE_COMMANDLINE?0:screenheight-msgwndheight;
  if(log.Count()==0)return;
  for(int i=0;i<screenheight-drawfrom;i++)
  {
    if(log[from+i].Length()<leftpos)
    {
      if(opt.shownum)
        sprintf(buf,"%03d ",i+from);
      else
        buf[0]=0;
    }
    else
    {
      if(opt.shownum)
        sprintf(buf,"%03d %s",i+from,log[from+i].Substr(leftpos,screenwidth-4).Str());
      else
      strcpy(buf,log[from+i].Substr(leftpos,screenwidth));

    }
    more=log[from+i].Length()-leftpos>screenwidth;
    l=strlen(buf);
    sprintf(buf+l,"%*s",screenwidth-l,"");
    li=ll[from+i];
    c=(colors.bg<<4)|(!li || !li->error?colors.text:li->error==1?colors.error:colors.warning);
    if(cursor && linepos==from+i)c=((c&0xf)<<4)|((c&0xf0)>>4);
    I.Text(0,drawfrom+i,c,buf);
    if(opt.shownum)
    {
      c=(colors.bg<<4)|colors.number;
      if(cursor && linepos==from+i)c=((c&0xf)<<4)|((c&0xf0)>>4);
      buf[4]=0;
      I.Text(0,drawfrom+i,c,buf);
    }
    if(more)I.Text(screenwidth-1,drawfrom+i,c>0x0F?14|c:14,">");
    if(from+i>=log.Count()-1)break;
  }
  I.Text(0,0,0,NULL);
  delete buf;
}

void AddStrings(void* buf, DWORD len)
{
  char*s = (char*)buf;
  int l=strlen(linebuf);
  for(int i=0;i<len;i++)
  {
    if(s[i]==0x0d || s[i]==0x0a || l>=linebufwidth)
    {
      if(s[i]==0x0d && s[i+1]!=0x0a)
      {
        l=0;
        linebuf[0]=0;
        continue;
      }
      for(int j=0;j<l;j++)
      {
        if((unsigned char)linebuf[j]<32)linebuf[j]=32;
      }
      SLineInfo li;
      int m=match_line(parser,linebuf,&li);
      if(m)
      {
        SLineInfo *tmp=new SLineInfo;
        *tmp=li;
        ll<<tmp;
      }else{
        ll<<NULL;
      }
      log<<linebuf;

//      if(linepos==screenheight-(mode==MODE_EDITOR?screenheight-msgwndheight:0))
//      {
      int from=log.Count()-(screenheight-(mode==MODE_EDITOR?screenheight-msgwndheight:0));
      if(from<0)from=0;
      Draw(from);
      if(linepos==screenheight-(mode==MODE_EDITOR?screenheight-msgwndheight:0))
        linepos--;
//      }else
//      {
//        I.Text(0,linepos+(mode==MODE_EDITOR?screenheight-msgwndheight:0),!m?7:m==1?12:4,linebuf);
//        I.Text(0,0,0,NULL);
//      }

      linepos++;
      l=0;
      linebuf[0]=0;
      if(s[i]==0x0d)i++;
      continue;
    }
    linebuf[l]=s[i];
    l++;
    linebuf[l]=0;
  }
};

int screenheight_mode()
{
  if(mode==MODE_EDITOR)return msgwndheight;
  return screenheight;
}

int CmpFileChars(char a,char b)
{
  if(a=='/')a='\\';
  if(b=='/')b='\\';
  return tolower(a)==tolower(b);
}

int cmpfiles(char *f1,char* f2)
{
  //return !lstrcmpi(f1,f2);
  while(*f1 && *f2 && CmpFileChars(*f1,*f2))
  {
    f1++;
    f2++;
  }
  if(*f1 || *f2)return 0;
  return 1;
}

static inline char NormalizeFileChar(char c)
{
  c=tolower(c);
  if(c=='/')c='\\';
  return c;
}

static bool CmpFileNames(const char* fn1,const char* fn2)
{
  while(*fn1 && *fn2 && NormalizeFileChar(*fn1)==NormalizeFileChar(*fn2))
  {
    fn1++;
    fn2++;
  }
  return (!*fn1) && (!*fn2);
}

static int SetPos(const char *filename,int line,int col)
{
  WindowInfo wi;
  int ok=0;
  int c=I.AdvControl(I.ModuleNumber,ACTL_GETWINDOWCOUNT,NULL);
  for(int i=0;i<c;i++)
  {
    wi.Pos=i;
    I.AdvControl(I.ModuleNumber,ACTL_GETWINDOWINFO,(void*)&wi);
    if(wi.Type==WTYPE_EDITOR && CmpFileNames(wi.Name,filename))
    {
      I.AdvControl(I.ModuleNumber,ACTL_SETCURRENTWINDOW,(void*)i);
      //if(mode==MODE_EDITOR)I.AdvControl(I.ModuleNumber,ACTL_COMMIT,(void*)i);
      I.AdvControl(I.ModuleNumber,ACTL_COMMIT,(void*)i);
      ok=1;
      break;
    }
  }
  if(!ok)
  {
    I.Editor(filename,"",0,0,-1,-1,EF_NONMODAL,ll[linepos]->line,ll[linepos]->col==-1?0:ll[linepos]->col);
    return 0;
  }

  EditorInfo ei;
  EditorSetPosition esp;
  I.EditorControl(ECTL_GETINFO,&ei);
  esp.CurLine=line-1;
  esp.CurPos=col==-1?0:col;
  esp.CurTabPos=-1;
  esp.TopScreenLine=esp.CurLine-1;
  if(esp.TopScreenLine==-1)esp.TopScreenLine=0;
  if(ei.TotalLines<ei.WindowSizeY)esp.TopScreenLine=0;
  esp.LeftPos=0;
  esp.Overtype=-1;
  I.EditorControl(ECTL_SETPOSITION,&esp);
  //I.EditorControl(ECTL_REDRAW,NULL);
  wasswitch=1;
  return 1;
}

void DeleteNonErrors(bool errorsonly=false)
{
  for(int i=log.Count()-1;i>=0;i--)
  {
    if(!ll[i] || (errorsonly && ll[i]->error!=1))
    {
      ll.Goto(i);
      ll.Delete();
      log.Goto(i);
      log.Delete();
      if(i<linepos)linepos--;
    }
    //toppos=0;
    //linepos=0;
  }
  if(log.Count()==0){log<<"";ll<<NULL;}
  if(linepos>=log.Count())linepos=log.Count()-1;
  if(linepos<toppos)toppos=linepos;
  ClearScreen(0);
}

void Scroll()
{
  INPUT_RECORD ir;
  DWORD rd;
  HideCur hc(1);
  if(opt.autodelete)DeleteNonErrors();

  if(linepos>log.Count()-1)
  {
    linepos=0;
    toppos=0;
  }
  if(linepos>toppos+screenheight_mode())
  {
    toppos=linepos-screenheight_mode()+1;
  }


  for(;;)
  {
    if(log.Count()==0)
    {
      log<<"";
      ll<<NULL;
    }
    Draw(toppos,1);
    if(ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE),&ir,1,&rd) && rd==1)
    {
      //DebugBreak();
      if(ir.EventType==KEY_EVENT && ir.Event.KeyEvent.bKeyDown)
      {

        WORD vkey=ir.Event.KeyEvent.wVirtualKeyCode;
/*        if((ir.Event.KeyEvent.dwControlKeyState&LEFT_CTRL_PRESSED ||
           ir.Event.KeyEvent.dwControlKeyState&RIGHT_CTRL_PRESSED)&&
           ir.Event.KeyEvent.uChar.AsciiChar=='N')
        {
          shownumbers^=1;
          Msg("shit!");
          continue;
        }*/


        switch(vkey)
        {
          case VK_INSERT:
          {
            opt.shownum^=1;
          }continue;
          case VK_UP:
          {
            linepos--;
            if(linepos<0)linepos=0;
            if(linepos<toppos)
            {
              toppos--;
              if(toppos<0)toppos=0;
            }
          }break;
          case VK_DOWN:
          {
            linepos++;
            if(linepos>=log.Count())linepos--;
            if(linepos-toppos>=screenheight_mode())
            {
              toppos++;
              if(toppos+screenheight_mode()>log.Count())toppos--;
            }
          }break;
          case VK_LEFT:
          {
            if(leftpos>0)leftpos--;
            if(ir.Event.KeyEvent.dwControlKeyState&LEFT_CTRL_PRESSED ||
               ir.Event.KeyEvent.dwControlKeyState&RIGHT_CTRL_PRESSED)leftpos=0;
          }break;
          case VK_RIGHT:
          {
            leftpos++;
            if(ir.Event.KeyEvent.dwControlKeyState&LEFT_CTRL_PRESSED ||
               ir.Event.KeyEvent.dwControlKeyState&RIGHT_CTRL_PRESSED)
            {
              int max=0;
              for(int i=toppos;i<toppos+screenheight_mode();i++)
              {
                if(i>=log.Count())break;
                if(log[i].Length()>max)max=log[i].Length();
              }
              if(max>screenwidth-(opt.shownum?4:0))leftpos=max-screenwidth+1+(opt.shownum?4:0);
            }
          }break;
          case VK_PRIOR:{
            linepos-=screenheight_mode();
            toppos-=screenheight_mode();
            if(linepos<0)linepos=0;
            if(toppos<0)toppos=0;
          }break;
          case VK_NEXT:{
            linepos+=screenheight_mode();
            toppos+=screenheight_mode();
            if(linepos>=log.Count())linepos=log.Count()-1;
            if(toppos>=log.Count()-screenheight_mode())toppos=log.Count()-screenheight_mode();
            if(toppos<0)toppos=0;
          }break;
          case VK_HOME:{
            linepos=0;
            toppos=0;
          }break;
          case VK_END:{
            linepos=log.Count()-1;
            toppos=log.Count()-screenheight_mode();
            if(toppos<0)toppos=0;
          }break;
          case VK_F1:
          {
            I.ShowHelp(I.ModuleName,"Contents",FHELP_SELFHELP);
          }continue;
          case VK_DELETE:{
            //DebugBreak();
            DeleteNonErrors(ir.Event.KeyEvent.dwControlKeyState&LEFT_ALT_PRESSED);
          }continue;
          case VK_F5:
          case VK_F6:
          case VK_F7:
          case VK_F8:
          case VK_F9:
          case VK_F4:{
            int dir=(ir.Event.KeyEvent.dwControlKeyState&SHIFT_PRESSED)?-1:1;
            int save=linepos;
            linepos+=dir;
            if(linepos<0)linepos=0;
            while(!ll[linepos] && linepos<=ll.Count()-1 && linepos>=0)
            {
              linepos+=dir;
            }
            if(linepos<0)linepos=save;
            if(linepos>=log.Count())linepos=save;
            if(linepos<toppos)toppos=linepos;
            if(linepos>=toppos+screenheight_mode())toppos=linepos-screenheight_mode()+1;
            if(toppos<0)toppos=0;
            if(mode==MODE_EDITOR && ll[linepos])
            {
              String file=ll[linepos]->file;
              if(file.Length()==0)
              {
                EditorInfo ei;
                I.EditorControl(ECTL_GETINFO,&ei);
                file=ei.FileName;
              }
              if(file[0]!='\\' && file[0]!='/' && file[1]!=':')
                file=cmddir+"\\"+file;
              if(SetPos(file,ll[linepos]->line,ll[linepos]->col))
              {
                I.EditorControl(ECTL_REDRAW,NULL);
              }
              hc.Restore();
              ClearScreen(0);
              //return;
            }
          }break;
          case VK_RETURN:
          {
            if(!ll[linepos])break;
            String file=ll[linepos]->file;
            if(file.Length()==0)
            {
              if(mode!=MODE_EDITOR)break;
              EditorInfo ei;
              I.EditorControl(ECTL_GETINFO,&ei);
              file=ei.FileName;
            }
            if(file[0]!='\\' && file[0]!='/' && file[1]!=':')
              file=cmddir+"\\"+file;
            hc.Restore();
            if(!SetPos(file,ll[linepos]->line,ll[linepos]->col))wasswitch=0;
            return;
          }break;
          case VK_ESCAPE:
          {
            return;
          }break;
        }
      }
    }

  }
}

/*
FARAPI(int) Configure(int item)
{
//  DebugBreak();
  free_config();
  if(!read_config(cfgfile))
  {
    Msg("Failed to load config!");
    return FALSE;
  }
  init_config(colors);
  Msg(GetMsg(MDone));
  return TRUE;
}
*/