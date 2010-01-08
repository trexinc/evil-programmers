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

#include <windows.h>
#include <stdio.h>
#include "xmem.h"
#include "xmlite.h"
#include "makeit.h"
#include "regexp.hpp"

static void *Pool=NULL;
static PXMLNode xconfig=NULL;


struct SFileType{
  RegExp *filetype;
  CCmdList commands;
};

typedef List<SFileType*> CFileTypes;

CFileTypes types;

struct SParserType{
  RegExp *re;
  int file;
  int line;
  int col;
//  int message;
  SParserType()
  {
    re=NULL;
    file=-1;
    line=-1;
    col=-1;
//    message=-1;
  }
};

typedef List<SParserType> CParserTypesList;

struct SParser{
  CParserTypesList errors,warnings;
};

typedef Hash<SParser*> CParsers;

CParsers parsers;

struct SPrefix{
  SFileType *type;
  SCommand* cmd;
};

typedef Hash<SPrefix> CPrefixes;

CPrefixes prefixes;
String sprefixes;


int read_config(const char *filename)
{
  //DebugBreak();
  FILE *f=fopen(filename,"rb");
  if(!f)return 0;
  fseek(f,0,SEEK_END);
  int size=ftell(f);
  fseek(f,0,SEEK_SET);
  char *buf=new char[size+1];
  fread(buf,size,1,f);
  fclose(f);
  buf[size]=0;
  Pool=xmemNewPool(size+size/2);
  xconfig=xmlNew(Pool);
  if(!xmlParse(Pool,xconfig,buf,0))
  {
    xmemFreePool(Pool);
    Pool=NULL;
    delete buf;
    return 0;
  }
  return 1;
}

static int init_parser_type(PXMLNode q,SParserType& t)
{
  const char *tmp;
  tmp=xmlGetItemAttr(q,"pattern");
  if(!tmp)return 0;
  t.re=new RegExp;
  if(!t.re->Compile(tmp))return 0;
  tmp=xmlGetItemAttr(q,"file");
  if(!tmp)return 0;
  if(*tmp)
    t.file=atoi(tmp);
  else
    t.file=-1;
  tmp=xmlGetItemAttr(q,"line");
  if(!tmp)return 0;
  t.line=atoi(tmp);
  tmp=xmlGetItemAttr(q,"pos");
  if(tmp)t.col=atoi(tmp);
//  tmp=xmlGetItemAttr(q,"message");
//  if(tmp)t.message=atoi(tmp);
  return 1;
}

int init_config(SColors *clr,SOptions* opt)
{
  PXMLNode p,q,x;
  p=xmlGetItem(xconfig,"/makeit-config/colors");
  if(p)
  {
    char *val;
    val=xmlGetItemAttr(p,"background");
    if(val)clr->bg=atoi(val);
    val=xmlGetItemAttr(p,"error");
    if(val)clr->error=atoi(val);
    val=xmlGetItemAttr(p,"warning");
    if(val)clr->warning=atoi(val);
    val=xmlGetItemAttr(p,"text");
    if(val)clr->text=atoi(val);
    val=xmlGetItemAttr(p,"numbers");
    if(val)clr->number=atoi(val);
  }
  p=xmlGetItem(xconfig,"/makeit-config/options");
  if(p)
  {
    char *val;
    val=xmlGetItemAttr(p,"shownumbers");
    opt->shownum=(val && !stricmp(val,"yes"));
    val=xmlGetItemAttr(p,"autodelete");
    opt->autodelete=(val && !stricmp(val,"yes"));
    val=xmlGetItemAttr(p,"beep");
    opt->beep=(val && !stricmp(val,"yes"));
    val=xmlGetItemAttr(p,"beeptime");
    if(val)opt->beeptime=atoi(val);
    val=xmlGetItemAttr(p,"wavefile");
    if(val)opt->wave=val;
    val=xmlGetItemAttr(p,"autosave");
    if(val)opt->autosave=(val && !stricmp(val,"yes"));
  }
  p=xmlGetItem(xconfig,"/makeit-config/parsers");
  if(!p || !p->hChildren)return 0;
  x=p->pChildren;
  while(x)
  {
    SParser *prs=new SParser;
    parsers.Insert(x->szName,prs);
    //prs=&parsers[x->szName];
    PHashLink e=NULL;
    while(e=xmlEnumNode(x,"error",e,&q))
    {
      SParserType pt;
      if(!init_parser_type(q,pt))
      {
        const char *m=GetMsg(MInvalidParser);
        char *buf=new char[strlen(m)+strlen(x->szName)+16];
        sprintf(buf,m,x->szName);
        Msg(buf);
        delete buf;
        parsers.Delete(x->szName);
      }else
      {
        prs->errors<<pt;
      }
    }
    while(e=xmlEnumNode(x,"warning",e,&q))
    {
      SParserType pt;
      if(!init_parser_type(q,pt))
      {
        const char *m=GetMsg(MInvalidParser);
        char *buf=new char[strlen(m)+strlen(x->szName)+16];
        sprintf(buf,m,x->szName);
        Msg(buf);
        delete buf;
        parsers.Delete(x->szName);
      }else
      {
        prs->warnings<<pt;
      }
    }
    x=x->pNext;
  }
  p=xmlGetItem(xconfig,"/makeit-config/types");
  if(!p || !p->hChildren)
  {
    Msg(GetMsg(MNoTypes));
    return 0;
  }
  PHashLink e=NULL;
  while(e=xmlEnumNode(p,"filetype",e,&q))
  {
    SFileType *t=new SFileType;
    char *tmp;
    t->filetype=new RegExp;
    tmp=xmlGetItemAttr(q,"filename");
    if(!tmp)
    {
      delete t;
      continue;
    }
    if(!t->filetype->Compile(tmp))
    {
      const char *m=GetMsg(MInvalidRegexp);
      char *buf=new char[strlen(tmp)+strlen(m)+16];
      sprintf(buf,m,tmp);
      Msg(buf);
      delete buf;
      delete t;
      continue;
    }
    types.Push(t);
    PHashLink n=NULL;
    while(n=xmlEnumNode(q,"command",n,&x))
    {
      SCommand *cmd=new SCommand;
      tmp=xmlGetItemAttr(x,"name");
      if(!tmp)
      {
        delete cmd;
        Msg(GetMsg(MIncompleteCommand));
        continue;
      }
      cmd->name=tmp;
      tmp=xmlGetItemAttr(x,"parser");
      if(!tmp)
      {
        delete cmd;
        Msg(GetMsg(MIncompleteCommand));
        continue;
      }
      cmd->parser=tmp;
      tmp=xmlGetItemAttr(x,"prefix");
      if(tmp)
      {
        SPrefix pfx;
        pfx.type=t;
        pfx.cmd=cmd;
        prefixes[tmp]=pfx;
        sprefixes+=":";
        sprefixes+=tmp;
      }
      cmd->command=x->szCont?x->szCont:"";
      x=x->pChildren;
      while(x)
      {
        if(x->eType!=xmlComment)
        {
          cmd->command+=x->szCont?x->szCont:"";
        }
        x=x->pNext;
      }
      if(!cmd->command)
      {
        delete cmd;
        Msg(GetMsg(MIncompleteCommand));
        continue;
      }
      t->commands.Push(cmd);
      for(int i=0;cmd->command[i];i++)
      {
        if((unsigned char)cmd->command[i]<32)cmd->command[i]=32;
      }
    }
  }
  return 1;
}

void free_config()
{
  if(Pool)xmemFreePool(Pool);
  char *k;
  SParser *p;
  parsers.First();
  while(parsers.Next(k,p))
  {
    int j;
    for(j=0;j<p->errors.Count();j++)
    {
      if(p->errors[j].re)delete p->errors[j].re;
    }
    for(j=0;j<p->warnings.Count();j++)
    {
      if(p->warnings[j].re)delete p->warnings[j].re;
    }
    delete p;
  }
  parsers.Empty();
  for(int i=0;i<types.Count();i++)
  {
    if(types[i]->filetype)delete types[i]->filetype;
    for(int j=0;j<types[i]->commands.Count();j++)
    {
      delete types[i]->commands[j];
    }
  }
  types.Clean();
  prefixes.Empty();
  sprefixes="";
}

static bool expand_cmd(SCommand& cmd,const String& c,SMatch* m)
{
  String s;
  int i=0;
  for(;cmd.command[i];i++)
  {
    if(cmd.command[i]=='$')
    {
      if(cmd.command[i+1]=='$')
      {
        s<<'$';
        i++;
      }else
      if(cmd.command[i+1]>='0' && cmd.command[i+1]<='9')
      {
        int n=cmd.command[i+1]-'0';
        i++;
        if(m[n].start!=-1)
        {
          s<<c.Substr(m[n].start,m[n].end-m[n].start);
        }
      }else s<<cmd.command[i];
    }else
    {
      s<<cmd.command[i];
    }
  }
  cmd.command=s;
  return true;
}

int get_commands(const String&c,CCmdList& cmds)
{
  types.First();
  for(int i=0;i<types.Count();i++)
  {
    SMatch m[10];
    int n=10;
    SFileType *ft=types[i];
    if(ft->filetype->Match(c,m,n))
    {
      ft->commands.First();
      for(int j=0;j<ft->commands.Count();j++)
      {
        SCommand cmd=*ft->commands[j];
        if(expand_cmd(cmd,c,m))
        {
          cmds.Append(new SCommand(cmd));
        }
      }
    }
  }
  return cmds.Count();
}

int match_line(const char *parser,const String& line,SLineInfo *li)
{
  if(!parsers.Exists(parser))return 0;
  SParser *p=parsers[parser];
  SParserType *pt;
  SMatch m[10];
  int n=10;
  int res=0;
  int i;
  for(i=0;i<p->errors.Count();i++)
  if(p->errors[i].re && p->errors[i].re->Match(line,m,n))
  {
    res=1;
    pt=&p->errors[i];
    break;
  }
  if(!res)
  {
    for(i=0;i<p->warnings.Count();i++)
    if(p->warnings[i].re && p->warnings[i].re->Match(line,m,n))
    {
      res=2;
      pt=&p->warnings[i];
      break;
    }
  }
  if(res)
  {
    li->error=res;
    if(pt->file!=-1)
      li->file=line.Substr(m[pt->file].start,m[pt->file].end-m[pt->file].start);
    else
      li->file="";
    li->line=atoi(line.Str()+m[pt->line].start);
    if(pt->col!=-1)
      li->col=atoi(line.Str()+m[pt->col].start);
    else
      li->col=-1;
  }
  return res;
}

void get_parsers(StrList& lst)
{
  parsers.First();
  char *k;
  SParser *s;
  while(parsers.Next(k,s))
  {
    lst<<k;
  }
}

String get_prefixes()
{
  if(sprefixes==":")return "";
  else return sprefixes;
}

int set_prefix(const char *pfx,const char *cmd,SCommand& scmd)
{
  if(!prefixes.Exists(pfx))return 0;
  SPrefix* p=&prefixes[pfx];
  SMatch m[10];
  int n=10;
  String c=cmd;
  scmd=*p->cmd;
  if(!p->type->filetype->Match(c,m,n))return 0;
  expand_cmd(scmd,c,m);
  return 1;
}
