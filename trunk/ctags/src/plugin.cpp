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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <memory>

#define FARAPI(type) extern "C" type __declspec(dllexport) WINAPI
#pragma comment(lib,"user32.lib")
#define _FAR_NO_NAMELESS_UNIONS
#include "plugin.hpp"
#include "String.hpp"
#include "List.hpp"
#include "XTools.hpp"
#include "Registry.hpp"
#include "tags.h"

using std::auto_ptr;

static struct PluginStartupInfo I;
static const char* APPNAME="Source Navigator";

static String tagfile;

static String targetFile;

static String rootKey;

static char wordChars[256]={0,};

Config config;

struct SUndoInfo{
  String file;
  int line;
  int pos;
  int top;
  int left;
};

Array<SUndoInfo> UndoArray;

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
  char *msg[]={"","","Ok"};
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

static const char*
GetMsg(int MsgId)
{
  return(I.GetMsg(I.ModuleNumber,MsgId));
}

int Msg(int msgid)
{
  Msg(GetMsg(msgid));
  return 0;
}

struct MI{
  String item;
  int data;
  MI()
  {
    data=-1;
  }
  MI(const char* str,int value):item(str),data(value){}
  MI(int msgid,int value):item(GetMsg(msgid)),data(value){}
};

typedef List<MI> MenuList;

#define MF_LABELS 1
#define MF_FILTER 2
#define MF_SHOWCOUNT 4

int Menu(const char *title,MenuList& lst,int sel,int flags=MF_LABELS,const void* param=NULL)
{
  Vector<FarMenuItem> menu;
  menu.Init(lst.Count());
  static const char labels[]="1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static const int labelsCount=sizeof(labels)-1;
  int i=0;
  int j=0;
  char buf[16];
  ZeroMemory(&menu[0],sizeof(FarMenuItem)*lst.Count());
  if(!(flags&MF_FILTER))
  {
    for(i=0;i<lst.Count();i++)
    {
      if((flags&MF_LABELS))
      {
        if(i<labelsCount)
        {
          sprintf(buf,"&%c ",labels[i]);
          strcpy(menu[i].Text,buf);
        }else
        {
          strcpy(menu[i].Text,"  ");
        }
        strcat(menu[i].Text,lst[i].item.Substr(0,120));
      }else
      {
        strcpy(menu[i].Text,lst[i].item.Substr(0,120));
      }
      if(sel==i)menu[i].Selected=1;
    }
    String cnt;
    cnt.Sprintf(" %s%d ",GetMsg(MItemsCount),lst.Count());
    int res=I.Menu(I.ModuleNumber,-1,-1,0,FMENU_WRAPMODE,title,flags&MF_SHOWCOUNT?cnt.Str():NULL,
                   "content",NULL,NULL,&menu[0],lst.Count());
    return res!=-1?lst[res].data:res;
  }else
  {
    String filter=param?(char*)param:"";
    Vector<int> idx;
    Vector<int> fk;
    static const char *filterkeys="1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_$\\\x08-_=|;':\",./<>? []*&^%#@!~";
    int shift;
    for(i=0;filterkeys[i];i++)
    {
      DWORD k=VkKeyScan(filterkeys[i]);
      if(k==0xffff)
      {
        fk.Push(0);
        continue;
      }
      shift=(k&0xff00)>>8;
      if(shift==1)shift=4;
      else if (shift==2)shift=1;
      else if (shift==4)shift=2;
      k=(k&0xff)|(shift<<16);
      fk.Push(k);
    }
    fk.Push(0);
#ifdef DEBUG
    //DebugBreak();
#endif
    int mode=0;
    for(;;)
    {
      int oldj=j;
      j=0;
      String match="";
      int minit=0;
      int fnd=-1,oldfnd=-1;
      idx.Clean();
      for(i=0;i<lst.Count();i++)
      {
        lst[i].item.SetNoCase(!config.casesens);
        if(filter.Length() && (fnd=lst[i].item.Index(filter))==-1)continue;
        if(!minit && fnd!=-1)
        {
          match=lst[i].item.Substr(fnd);
          minit=1;
        }
        if(oldfnd!=-1 && oldfnd!=fnd)
        {
          oldj=-1;
        }
        if(fnd!=-1 && filter.Length())
        {
          int xfnd=-1;
          while((xfnd=lst[i].item.Index(filter,xfnd+1))!=-1)
          {
            for(int k=0;k<match.Length();k++)
            {
              if(xfnd+k>=lst[i].item.Length() ||
                  (config.casesens && match[k]!=lst[i].item[xfnd+k]) ||
                  (!config.casesens && tolower(match[k])!=tolower(lst[i].item[xfnd+k]))
                )
              {
                match.Delete(k);
                break;
              }
            }
          }
        }
        idx.Push(i);
        strcpy(menu[j].Text,lst[i].item.Substr(0,120));
        if(sel==j)menu[j].Selected=1;
        j++;
        if(fnd!=-1)oldfnd=fnd;
      }
      if((mode==0 && j==0) || (mode==1 && j==oldj))
      {
        if(filter.Length())
        {
          //DebugBreak();
          filter.Delete(-1);
          continue;
        }
      }
      if(sel>j)
      {
        menu[j-1].Selected=1;
      }
      if(match.Length()>filter.Length() && j>1 && mode!=1)
      {
        filter=match;
      }
      String cnt;
      cnt.Sprintf(" %s%d ",GetMsg(MItemsCount),j);
      int bkey;
      String ftitle=title;
      ftitle+=" ["+filter+"]";
      int res=I.Menu(I.ModuleNumber,-1,-1,0,FMENU_WRAPMODE|FMENU_SHOWAMPERSAND,ftitle,
                     flags&MF_SHOWCOUNT?cnt.Str():NULL,"content",&fk[0],&bkey,&menu[0],j);
      if(res==-1 && bkey==-1)return -1;
      if(bkey==-1)
      {
        return lst[idx[res]].data;
      }
      int key=filterkeys[bkey];
      if(key==8)
      {
        filter.Delete(-1);
        mode=1;
        continue;
      }
      filter+=(char)key;
      mode=0;
      sel=res;
    }
  }
}


FARAPI(void) SetStartupInfo(const struct PluginStartupInfo *Info)
{
  RegExp::InitLocale();
  I=*Info;
  rootKey=I.RootKey;
  rootKey+="\\ctags";
}

int isident(int chr)
{
  return wordChars[(unsigned char)chr]!=0;
}

static String GetWord(int offset=0)
{
//  DebugBreak();
  EditorInfo ei;
  I.EditorControl(ECTL_GETINFO,&ei);
  EditorGetString egs;
  egs.StringNumber=-1;
  I.EditorControl(ECTL_GETSTRING,&egs);
  int pos=ei.CurPos-offset;
  if(pos<0)pos=0;
  if(pos>egs.StringLength)return "";
  if(!isident(egs.StringText[pos]))return "";
  int start=pos,end=pos;
  while(start>0 && isident(egs.StringText[start-1]))start--;
  while(end<egs.StringLength-1 && isident(egs.StringText[end+1]))end++;
  if(start==end || (!isident(egs.StringText[start])))return "";
  String rv;
  rv.Set(egs.StringText,start,end-start+1);
  return rv;
}

/*static const char* GetType(char type)
{
  switch(type)
  {
    case 'p':return "proto";
    case 'c':return "class";
    case 'd':return "macro";
    case 'e':return "enum";
    case 'f':return "function";
    case 'g':return "enum name";
    case 'm':return "member";
    case 'n':return "namespace";
    case 's':return "structure";
    case 't':return "typedef";
    case 'u':return "union";
    case 'v':return "variable";
    case 'x':return "extern/forward";
  }
  return "unknown";
}
*/
static void chomp(char* str)
{
  int i=strlen(str)-1;
  while(i>=0 && (unsigned char)str[i]<32)
  {
    str[i]=0;
    i--;
  }
}

int SetPos(const char *filename,int line,int col,int top,int left);

static void NotFound(const char* fn,int line)
{
  const char *msg[4]={APPNAME,GetMsg(MNotFoundAsk)};
  int rc=I.Message(I.ModuleNumber,FMSG_WARNING|FMSG_MB_YESNO,NULL,msg,2,0);
  if(rc==-1 || rc==1)return;
  SetPos(fn,line,0,-1,-1);
}

static void NavigateTo(TagInfo* info)
{
  DWORD ver=I.AdvControl(I.ModuleNumber,ACTL_GETFARVERSION,NULL);
  int build=(ver&0xffff0000)>>16;
  EditorInfo ei;
  I.EditorControl(ECTL_GETINFO,&ei);
  {
    SUndoInfo ui;
    ui.file=ei.FileName;
    ui.line=ei.CurLine;
    ui.pos=ei.CurPos;
    ui.top=ei.TopScreenLine;
    ui.left=ei.LeftPos;
    UndoArray.Push(ui);
  }
  WindowInfo wi;
  int ok=0;

  const char* file=info->file.Str();
  if(build>1500)
  {
    int c=I.AdvControl(I.ModuleNumber,ACTL_GETWINDOWCOUNT,NULL);
    for(int i=0;i<c;i++)
    {
      wi.Pos=i;
      I.AdvControl(I.ModuleNumber,ACTL_GETWINDOWINFO,(void*)&wi);
      if(wi.Type==WTYPE_EDITOR && !lstrcmpi(wi.Name,file))
      {
        I.AdvControl(I.ModuleNumber,ACTL_SETCURRENTWINDOW,(void*)i);
        //if(mode==MODE_EDITOR)I.AdvControl(I.ModuleNumber,ACTL_COMMIT,(void*)i);
        I.AdvControl(I.ModuleNumber,ACTL_COMMIT,(void*)i);
        ok=1;
        break;
      }
    }
  }
  int havere=info->re.Length()>0;
  RegExp re;
  if(havere)re.Compile(info->re);
  if(!ok)
  {
    FILE *f=fopen(file,"rt");
    if(!f)
    {
      Msg(MEFailedToOpen);
      return;
    }
    int line=info->lineno-1;
    int cnt=0;
    char buf[512];
    while(fgets(buf,sizeof(buf),f) && cnt<line)cnt++;
    chomp(buf);
    SMatch m[10];
    int n=10;
    if(line!=-1)
    {
      if(havere && !re.Match(buf,m,n))
      {
        line=-1;
//        Msg("not found in place, searching");
      }
    }
    if(line==-1)
    {
      if(!havere)
      {
        NotFound(file,info->lineno);
        fclose(f);
        return;
      }
      line=0;
      fseek(f,0,SEEK_SET);
      while(fgets(buf,sizeof(buf),f))
      {
        chomp(buf);
        n=10;
        if(re.Match(buf,m,n))
        {
          break;
        }
        line++;
      }
      if(feof(f))
      {
        NotFound(file,info->lineno);
        fclose(f);
        return;
      }
    }
    fclose(f);
    I.Editor(file,"",0,0,-1,-1,build>1500?EF_NONMODAL:0,line+1,1);
    return;
  }
  EditorSetPosition esp;
  I.EditorControl(ECTL_GETINFO,&ei);

  esp.CurPos=-1;
  esp.CurTabPos=-1;
  esp.TopScreenLine=-1;
  esp.LeftPos=-1;
  esp.Overtype=-1;


  int line=info->lineno-1;
  if(line!=-1)
  {
    EditorGetString egs;
    egs.StringNumber=line;
    I.EditorControl(ECTL_GETSTRING,&egs);
    SMatch m[10];
    int n=10;

    if(havere && !re.Match(egs.StringText,egs.StringText+egs.StringLength,m,n))
    {
      line=-1;
    }
  }
  if(line==-1)
  {
    if(!havere)
    {
      esp.CurLine=ei.CurLine;
      esp.TopScreenLine=ei.TopScreenLine;
      I.EditorControl(ECTL_SETPOSITION,&esp);
      NotFound(file,info->lineno);
      return;
    }
    line=0;
    SMatch m[10];
    int n=10;
    EditorGetString egs;
    while(line<ei.TotalLines)
    {
      esp.CurLine=line;
      I.EditorControl(ECTL_SETPOSITION,&esp);
      egs.StringNumber=-1;
      I.EditorControl(ECTL_GETSTRING,&egs);
      n=10;
      if(re.Match(egs.StringText,egs.StringText+egs.StringLength,m,n))
      {
        break;
      }
      line++;
    }
    if(line==ei.TotalLines)
    {
      esp.CurLine=info->lineno==-1?ei.CurLine:info->lineno-1;
      esp.TopScreenLine=ei.TopScreenLine;
      I.EditorControl(ECTL_SETPOSITION,&esp);
      NotFound(file,info->lineno);
      return;
    }
  }

  esp.CurLine=line;
  esp.TopScreenLine=esp.CurLine-1;
  if(esp.TopScreenLine==-1)esp.TopScreenLine=0;
  if(ei.TotalLines<ei.WindowSizeY)esp.TopScreenLine=0;
  esp.LeftPos=0;
  I.EditorControl(ECTL_SETPOSITION,&esp);
  I.EditorControl(ECTL_REDRAW,NULL);
}

int SetPos(const char *filename,int line,int col,int top,int left)
{
  DWORD ver=I.AdvControl(I.ModuleNumber,ACTL_GETFARVERSION,NULL);
  int build=(ver&0xffff0000)>>16;

  WindowInfo wi;
  int ok=0;
  if(build>1500)
  {
    int c=I.AdvControl(I.ModuleNumber,ACTL_GETWINDOWCOUNT,NULL);
    for(int i=0;i<c;i++)
    {
      wi.Pos=i;
      I.AdvControl(I.ModuleNumber,ACTL_GETWINDOWINFO,(void*)&wi);
      if(wi.Type==WTYPE_EDITOR && !lstrcmpi(wi.Name,filename))
      {
        I.AdvControl(I.ModuleNumber,ACTL_SETCURRENTWINDOW,(void*)i);
        //if(mode==MODE_EDITOR)I.AdvControl(I.ModuleNumber,ACTL_COMMIT,(void*)i);
        I.AdvControl(I.ModuleNumber,ACTL_COMMIT,(void*)i);
        ok=1;
        break;
      }
    }
  }
  if(!ok)
  {
    I.Editor(filename,"",0,0,-1,-1,build>1500?EF_NONMODAL:0,line,col);
    return 0;
  }

  EditorInfo ei;
  EditorSetPosition esp;
  I.EditorControl(ECTL_GETINFO,&ei);
  esp.CurLine=line;
  esp.CurPos=col;
  esp.CurTabPos=-1;
  esp.TopScreenLine=top;
  esp.LeftPos=left;
  esp.Overtype=-1;
  I.EditorControl(ECTL_SETPOSITION,&esp);
  I.EditorControl(ECTL_REDRAW,NULL);
  return 1;
}

String TrimFilename(const String& file,int maxlength)
{
  if(file.Length()<=maxlength)return file;
  int ri=file.RIndex("\\")+1;
  if(file.Length()-ri+3>maxlength)
  {
    return "..."+file.Substr(file.Length()-maxlength-3);
  }
  return file.Substr(0,3)+"..."+file.Substr(file.Length()-(maxlength-7));
}

static TagInfo* TagsMenu(PTagArray pta)
{
  EditorInfo ei;
  I.EditorControl(ECTL_GETINFO,&ei);
  MenuList sm;
  String s;
  TagArray& ta=*pta;
  int maxid=0,maxinfo=0;
  int i;
  for(i=0;i<ta.Count();i++)
  {
    TagInfo *ti=ta[i];
    if(ti->name.Length()>maxid)maxid=ti->name.Length();
    if(ti->info.Length()>maxinfo)maxinfo=ti->info.Length();
    //if(ti->file.Length()>maxfile)
  }
  int maxfile=ei.WindowSizeX-8-maxid-maxinfo-1-1-1;
  for(i=0;i<ta.Count();i++)
  {
    TagInfo *ti=ta[i];
    s.Sprintf("%c:%s%*s %s%*s %s",ti->type,ti->name.Str(),maxid-ti->name.Length(),"",
      ti->info.Length()?ti->info.Str():"",maxinfo-ti->info.Length(),"",
      TrimFilename(ti->file,maxfile).Str()
    );
    sm<<MI(s.Str(),i);
  }
  int sel=Menu(GetMsg(MSelectSymbol),sm,0,MF_FILTER|MF_SHOWCOUNT);
  if(sel==-1)return NULL;
  return ta[sel];
}

static void FreeTagsArray(PTagArray ta)
{
  for(int i=0;i<ta->Count();i++)
  {
    delete (*ta)[i];
  }
  delete ta;
}

static void UpdateConfig()
{
  Registry r(HKEY_CURRENT_USER);
  if(!r.Open(rootKey))
  {
    config.exe="ctags.exe";
    config.opt="--c++-types=+px --c-types=+px --fields=+n";
    config.autoload="";
    memset(wordChars,0,sizeof(wordChars));
    for(int i=0;i<256;i++)
    {
      if(isalnum(i) || i=='$' || i=='_' || i=='~')
      {
        wordChars[i]=1;
      }
    }
    return;
  }
  char buf[512];
  if(r.Get("pathtoexe",buf,sizeof(buf)))
  {
    config.exe=buf;
  }
  if(r.Get("commandline",buf,sizeof(buf)))
  {
    config.opt=buf;
  }
  if(r.Get("autoload",buf,sizeof(buf)))
  {
    config.autoload=buf;
  }
  if(r.Get("wordchars",buf,sizeof(buf)))
  {
    memset(wordChars,0,sizeof(wordChars));
    int i=0;
    while(buf[i])
    {
      wordChars[(unsigned char)buf[i]]=1;
      i++;
    }
  }else
  {
    memset(wordChars,0,sizeof(wordChars));
    for(int i=0;i<256;i++)
    {
      if(isalnum(i) || i=='$' || i=='_' || i=='~')
      {
        wordChars[i]=1;
      }
    }
  }
  if(r.Get("casesensfilt",buf,sizeof(buf)))
  {
    config.casesens=!stricmp(buf,"true");
  }else
  {
    config.casesens=true;
  }
}

FARAPI(HANDLE) OpenPlugin(int OpenFrom,int Item)
{
  UpdateConfig();
  if(OpenFrom==OPEN_EDITOR)
  {
    //DebugBreak();
    EditorInfo ei;
    I.EditorControl(ECTL_GETINFO,&ei);
    Autoload(ei.FileName);
    if(Count()==0)
    {
      Msg(MENotLoaded);
      return INVALID_HANDLE_VALUE;
    }
    MenuList ml;
    enum{
      miFindSymbol,miUndo,miResetUndo,
      miComplete,miBrowseFile,miBrowseClass,
    };
    ml<<MI(MFindSymbol,miFindSymbol)
      <<MI(MCompleteSymbol,miComplete)
      <<MI(MUndoNavigation,miUndo)
      <<MI(MResetUndo,miResetUndo)
      <<MI(MBrowseSymbolsInFile,miBrowseFile)
      <<MI(MBrowseClass,miBrowseClass);
    int res=Menu(GetMsg(MPlugin),ml,0);
    if(res==-1)return INVALID_HANDLE_VALUE;
    switch(res)
    {
      case miFindSymbol:
      {
        String word=GetWord();
        if(word.Length()==0)return INVALID_HANDLE_VALUE;
        //Msg(word);
        PTagArray ta=Find(word,ei.FileName);
        if(!ta)
        {
          Msg(GetMsg(MNotFound));
          return INVALID_HANDLE_VALUE;
        }
        TagInfo *ti;
        if(ta->Count()==1)
        {
          ti=(*ta)[0];
        }else
        {
          ti=TagsMenu(ta);
        }
        if(ti)NavigateTo(ti);
        FreeTagsArray(ta);
      }break;
      case miUndo:
      {
        if(UndoArray.Count()==0)return INVALID_HANDLE_VALUE;
        /*char b[32];
        sprintf(b,"%d",ei.CurState);
        Msg(b);*/
        if(ei.CurState==ECSTATE_SAVED)
        {
          I.EditorControl(ECTL_QUIT,NULL);
          I.AdvControl(I.ModuleNumber,ACTL_COMMIT,(void*)-1);
        }
        SUndoInfo ui;
        UndoArray.Pop(ui);
        SetPos(ui.file,ui.line,ui.pos,ui.top,ui.left);
      }break;
      case miResetUndo:
      {
        UndoArray.Clean();
      }break;
      case miComplete:
      {
        EditorInfo ei;
        I.EditorControl(ECTL_GETINFO,&ei);
        String word=GetWord(1);
        if(word.Length()==0)return INVALID_HANDLE_VALUE;
        StrList lst;
        FindParts(ei.FileName,word,lst);
        if(lst.Count()==0)
        {
          Msg(MNothingFound);
          return INVALID_HANDLE_VALUE;
        }
        int res;
        if(lst.Count()>1)
        {
          MenuList ml;
          for(int i=0;i<lst.Count();i++)
          {
            ml<<MI(lst[i],i);
          }
          res=Menu(GetMsg(MSelectSymbol),ml,0,MF_FILTER|MF_SHOWCOUNT,(void*)word.Str());
          if(res==-1)return INVALID_HANDLE_VALUE;
        }else
        {
          res=0;
        }
        EditorGetString egs;
        egs.StringNumber=-1;
        I.EditorControl(ECTL_GETSTRING,&egs);
        while(isident(egs.StringText[ei.CurPos]))ei.CurPos++;
        EditorSetPosition esp;
        esp.CurLine=-1;
        esp.CurPos=ei.CurPos;
        esp.CurTabPos=-1;
        esp.TopScreenLine=-1;
        esp.LeftPos=-1;
        esp.Overtype=-1;
        I.EditorControl(ECTL_SETPOSITION,&esp);
        I.EditorControl(ECTL_INSERTTEXT,(void*)lst[res].Substr(word.Length()).Str());
      }break;
      case miBrowseFile:
      {
        EditorInfo ei;
        I.EditorControl(ECTL_GETINFO,&ei);
        PTagArray ta=FindFileSymbols(ei.FileName);
        if(!ta)
        {
          Msg(MNothingFound);
          return INVALID_HANDLE_VALUE;
        }
        TagInfo *ti=TagsMenu(ta);
        if(ti)NavigateTo(ti);
        FreeTagsArray(ta);
      }break;
      case miBrowseClass:
      {
#ifdef DEBUG
        //DebugBreak();
#endif
        String word=GetWord();
        if(word.Length()==0)
        {
          char buf[256];
          if(!I.InputBox(GetMsg(MBrowseClassTitle),GetMsg(MInputClassToBrowse),NULL,
                      "",buf,sizeof(buf),NULL,0))return INVALID_HANDLE_VALUE;
          word=buf;
        }
        EditorInfo ei;
        I.EditorControl(ECTL_GETINFO,&ei);
        PTagArray ta=FindClassSymbols(ei.FileName,word);
        if(!ta)
        {
          Msg(MNothingFound);
          return INVALID_HANDLE_VALUE;
        }
        TagInfo *ti=TagsMenu(ta);
        if(ti)NavigateTo(ti);
        FreeTagsArray(ta);
      }break;
    }
  }
  else
  {
    int load=OpenFrom==OPEN_COMMANDLINE;
    if(OpenFrom==OPEN_PLUGINSMENU)
    {
      MenuList ml;
      enum {miLoadTagsFile,miUnloadTagsFile,
            miUpdateTagsFile,miCreateTagsFile};
      ml<<MI(MLoadTagsFile,miLoadTagsFile)
        <<MI(MUnloadTagsFile,miUnloadTagsFile)
        <<MI(MCreateTagsFile,miCreateTagsFile)
        <<MI(MUpdateTagsFile,miUpdateTagsFile);
      int rc=Menu(GetMsg(MPlugin),ml,0);
      switch(rc)
      {
        case miLoadTagsFile:
        {
          load=1;
        }break;
        case miUnloadTagsFile:
        {
          ml.Clean();
          ml<<MI(MAll,0);
          StrList l;
          GetFiles(l);
          for(int i=0;i<l.Count();i++)
          {
            ml<<MI(l[i],i+1);
          }
          int rc=Menu(GetMsg(MUnloadTagsFile),ml,0);
          if(rc==-1)return INVALID_HANDLE_VALUE;
          UnloadTags(rc-1);
        }break;
        case miCreateTagsFile:
        {
          HANDLE hScreen=I.SaveScreen(0,0,-1,-1);
          const char *msg[]={GetMsg(MPlugin),GetMsg(MTagingCurrentDirectory)};
          I.Message(I.ModuleNumber,0,NULL,msg,2,0);
          int rc=TagCurrentDir();
          I.RestoreScreen(hScreen);
        }break;
        case miUpdateTagsFile:
        {
          HANDLE hScreen=I.SaveScreen(0,0,-1,-1);
          const char *msg[]={GetMsg(MPlugin),GetMsg(MUpdatingTagsFile)};
          StrList changed;
          PanelInfo pi;
          String file;
          I.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&pi);
          file=pi.CurDir;
          if(file[-1]!='\\')file+="\\";
          file+=pi.PanelItems[pi.CurrentItem].FindData.cFileName;
          I.Message(I.ModuleNumber,0,NULL,msg,2,0);
          if(!UpdateTagsFile(file))
          {
            I.RestoreScreen(hScreen);
            Msg(MUnableToUpdate);
            return INVALID_HANDLE_VALUE;
          }
          I.RestoreScreen(hScreen);
        }break;
      }
    }
    if(load)
    {
      //DebugBreak();
      PanelInfo pi;
      I.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&pi);
      tagfile=pi.CurDir;
      if(tagfile[-1]!='\\')tagfile+="\\";
      if(OpenFrom==OPEN_PLUGINSMENU)
      {
        tagfile+=pi.PanelItems[pi.CurrentItem].FindData.cFileName;
      }else
      if(OpenFrom==OPEN_COMMANDLINE)
      {
        char *cmd=(char*)Item;
        if(cmd[1]==':')
        {
          tagfile=cmd;
        }else
        {
          if(cmd[0]=='\\')
          {
            tagfile.Delete(2);
            tagfile+=cmd;
          }else
          {
            tagfile+=cmd;
          }
        }
      }
      int rc=Load(tagfile,"",true);
      if(rc>1)
      {
        Msg(GetMsg(rc));
        return INVALID_HANDLE_VALUE;
      }
      String msg;
      msg.Sprintf("%s:%d",GetMsg(MLoadOk),Count());
      Msg(msg);
    }
  }
  return INVALID_HANDLE_VALUE;
}

FARAPI(void) GetPluginInfo(struct PluginInfo *pi)
{
  static const char *PluginMenuStrings[1];
  static const char *PluginConfigStrings[1];
  pi->StructSize=sizeof(struct PluginInfo);
  pi->Flags=PF_EDITOR;
  pi->DiskMenuStringsNumber=0;
  PluginMenuStrings[0]=GetMsg(MPlugin);
  pi->PluginMenuStrings=PluginMenuStrings;
  pi->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);

  static const char *ConfigMenuStrings[1];
  ConfigMenuStrings[0]=GetMsg(MPlugin);
  pi->PluginConfigStrings=ConfigMenuStrings;
  pi->PluginConfigStringsNumber=1;
  static const char *pfx="tag";
  pi->CommandPrefix=pfx;
}

struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  unsigned int Selected;
  unsigned int Flags;
  unsigned char DefaultButton;
  char *Data;
};

void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item,
                    int ItemsNumber)
{
  for (int I=0;I<ItemsNumber;I++)
  {
    Item[I].Type=Init[I].Type;
    Item[I].X1=Init[I].X1;
    Item[I].Y1=Init[I].Y1;
    Item[I].X2=Init[I].X2;
    Item[I].Y2=Init[I].Y2;
    Item[I].Focus=Init[I].Focus;
    Item[I].Param.Selected=Init[I].Selected;
    Item[I].Flags=Init[I].Flags;
    Item[I].DefaultButton=Init[I].DefaultButton;
    if ((unsigned int)Init[I].Data<2000)
      strcpy(Item[I].Data.Data,GetMsg((unsigned int)Init[I].Data));
    else
      strcpy(Item[I].Data.Data,Init[I].Data);
  }
}

FARAPI(int) Configure(int item)
{
  struct InitDialogItem InitItems[]={
        /*Type         X1 Y2 X2 Y2  F S           Flags D Data */
/*00*/    DI_DOUBLEBOX, 3, 1,64,13, 0,0,              0,0,(char*)MPlugin,
/*01*/    DI_TEXT,      5, 2, 0, 0, 0,0,              0,0,(char*)MPathToExe,
/*02*/    DI_EDIT,      5, 3,62, 3, 1,0,              0,0,"",
/*03*/    DI_TEXT,      5, 4, 0, 0, 0,0,              0,0,(char*)MCmdLineOptions,
/*04*/    DI_EDIT,      5, 5,62, 5, 1,0,              0,0,"",
/*05*/    DI_TEXT,      5, 6, 0, 0, 0,0,              0,0,(char*)MAutoloadFile,
/*06*/    DI_EDIT,      5, 7,62, 7, 1,0,              0,0,"",
/*07*/    DI_TEXT,      5, 8, 0, 0, 0,0,              0,0,(char*)MWordChars,
/*08*/    DI_EDIT,      5, 9,62, 9, 1,0,              0,0,"",
/*09*/    DI_CHECKBOX,  5, 10,62,10,1,0,              0,0,(char*)MCaseSensFilt,
/*10*/    DI_TEXT,      5,11,62,10, 1,0,DIF_SEPARATOR|DIF_BOXCOLOR,0,"",
/*11*/    DI_BUTTON,    0,12, 0, 0, 0,0,DIF_CENTERGROUP,1,(char *)MOk,
/*12*/    DI_BUTTON,    0,12, 0, 0, 0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  Registry r(HKEY_CURRENT_USER);
  if(!r.Open(rootKey))
  {
    Msg(MRegFailed);
    return FALSE;
  }
  char buf[512];
  if(r.Get("pathtoexe",buf,sizeof(buf)))
  {
    strcpy(DialogItems[2].Data.Data,buf);
  }else
  {
    strcpy(DialogItems[2].Data.Data,"ctags.exe");
  }
  if(r.Get("commandline",buf,sizeof(buf)))
  {
    strcpy(DialogItems[4].Data.Data,buf);
  }else
  {
    wsprintf(DialogItems[4].Data.Data,"%s","--c++-types=+px --c-types=+px --fields=+n -R *");
  }
  if(r.Get("autoload",buf,sizeof(buf)))
  {
    strcpy(DialogItems[6].Data.Data,buf);
  }
  if(r.Get("wordchars",buf,sizeof(buf)))
  {
    strcpy(DialogItems[8].Data.Data,buf);
  }else
  {
    String s;
    for(int i=0;i<256;i++)
    {
      if(isalnum(i) || i=='$' || i=='_' || i=='~')
      {
        s+=(char)i;
      }
    }
    strcpy(DialogItems[8].Data.Data,s.Str());
  }
  if(r.Get("casesensfilt",buf,sizeof(buf)))
  {
    DialogItems[9].Param.Selected=!stricmp(buf,"true");
  }else
  {
    DialogItems[9].Param.Selected=1;
  }
  int ExitCode=I.Dialog(I.ModuleNumber,-1,-1,68,15,"ctagscfg",DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));
  if(ExitCode!=11)return FALSE;
  r.Set("pathtoexe",DialogItems[2].Data.Data);
  r.Set("commandline",DialogItems[4].Data.Data);
  r.Set("autoload",DialogItems[6].Data.Data);
  r.Set("wordchars",DialogItems[8].Data.Data);
  r.Set("casesensfilt",DialogItems[9].Param.Selected?"True":"False");
  return TRUE;
}
