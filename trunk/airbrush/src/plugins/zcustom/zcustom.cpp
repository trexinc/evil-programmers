/*
    zcustom.cpp
    Copyright (C) 2000-2008 zg

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

#include <windows.h>
#include <ctype.h>
#include <stdio.h>
#include <tchar.h>
#include "../../bootstrap/abplugin.h"
#include "../../memory.h"
#include "abzcustom.h"

ColorizeInfo Info;
Rules *rules=NULL;
int rules_count=0;
CharacterClass *classes=NULL;
int classes_count=0;

static int read_line(HANDLE file,char **line,int *size)
{
  DWORD CurrPos=SetFilePointer(file,0,NULL,FILE_CURRENT),transferred;
  unsigned long len=1024*4,pos; char *buffer=NULL,*pos_ptr,*pos2_ptr;
  *line=NULL; *size=0;
  while(true)
  {
    len*=2;
    free(buffer);
    buffer=(char *)malloc(len);
    memset(buffer,0,len);
    ReadFile(file,buffer,len,&transferred,NULL);
    SetFilePointer(file,CurrPos,NULL,FILE_BEGIN);
    pos_ptr=(char *)memchr(buffer,'\n',len);
    pos2_ptr=(char *)memchr(buffer,'\r',len);
    if(!pos_ptr) pos_ptr=pos2_ptr;
    if(pos_ptr||pos2_ptr)
    {
      if((pos_ptr>pos2_ptr)&&(pos2_ptr))
        pos_ptr=pos2_ptr;
      pos=pos_ptr-buffer;
      int add=0;
      while((buffer[pos+add]=='\n')||(buffer[pos+add]=='\r')) add++;
      SetFilePointer(file,CurrPos+pos+add,NULL,FILE_BEGIN);
      *line=buffer;
      *size=pos;
      return 0;
    }
    else if(transferred<len)
    {
      SetFilePointer(file,0,NULL,FILE_END);
      *line=buffer;
      *size=transferred;
      return 1; //eof
    }
  }
  return 2; //error
}

static int code_index(int value)
{
  int res=0;
  for(int i=0;i<4;i++)
  {
    res|=(((value&0x7f)+32)<<(i*8));
    value>>=7;
  }
  return res;
}

static int decode_index(int value)
{
  int res=0;
  for(int i=0;i<4;i++)
  {
    res|=(((value&0xff)-32)<<(i*7));
    value>>=8;
  }
  return res;
}

static char *get_char_class_end(char *src,int len)
{
  while(len)
  {
    switch(*src)
    {
      case '\\':
        src++; len--;
        break;
      case '/':
        return src;
    }
    if(len) { src++; len--; }
  }
  return NULL;
}

static int add_char_class(char *start,char *end)
{
  int res=0;
  CharacterClass cc;
  CharacterClassParam ccp;
  ccp.char_class=&cc;
  ccp.start=(unsigned char *)start;
  ccp.end=(unsigned char *)end;
  yyparse(&ccp);
  CharacterClass *cc_new=(CharacterClass *)malloc(sizeof(CharacterClass)*(classes_count+1));
  if(cc_new)
  {
    memcpy(cc_new,classes,sizeof(CharacterClass)*classes_count);
    free(classes);
    classes=cc_new;
    memcpy(classes+classes_count,&cc,sizeof(CharacterClass));
    res=classes_count;
    classes_count++;
  }
  return res;
}

static void syntax_strncpy(char *dst,char *src,int len)
{
  char *char_class_end;
  while(len)
  {
    switch(*src)
    {
      case '\\':
        src++; len--; if(!len) break;
        switch (*src)
        {
          case 'n':
            *dst='\n';
            break;
          case 'r':
            *dst='\r';
            break;
          case 't':
            *dst='\t';
            break;
          case 's':
            *dst=' ';
            break;
          default:
            *dst=*src;
            break;
        }
        break;
      case '*':
        *dst='\001';
        break;
      case '+':
        *dst='\002';
        break;
      case '/':
        char_class_end=get_char_class_end(src+1,len-1);
        if((char_class_end)&&((char_class_end-src+1)>4))
        {
          int index=add_char_class(src+1,char_class_end);
          *dst='\003'; dst++; *((int *)dst)=code_index(index); dst++; dst++; dst++;
          while(src<char_class_end) { src++; len--; }
          break;
        }
      default:
        *dst=*src;
        break;
    }
    if(len) { src++; dst++; len--; }
  }
  *dst=0;
}

static int syntax_get_byte(const char *line,int len,int index)
{
  if((index==-1)||(index==len)) return '\n';
  if((index>-1)&&(index<len)) return line[index];
  return 0;
}

/*
  0 - non equal
  1 - equal
*/
static int syntax_strcmp(const char *line,int len,int pos,char *text,char *whole_left,char *whole_right,int line_start,int recursive)
{
  if(!*text)
    return 0;
  int c=syntax_get_byte(line,len,pos-1);
  if(line_start)
    if(c!='\n')
      return 0;
  if(whole_left)
    if(whole_left[c&0xff])
      return 0;
  char *p=text;
  int old_pos=pos,index;
  while(*p)
  {
    switch(*p)
    {
      case '\001':
        p++;
        while(true)
        {
          if(recursive)
          {
            while(true)
            {
              int new_pos=syntax_strcmp(line,len,pos,text,whole_left,whole_right,line_start,recursive);
              if(new_pos) pos=new_pos;
              else break;
            }
          }
          c=syntax_get_byte(line,len,pos);
          if(c==*p)
            break;
          if(c=='\n')
            return 0;
          pos++;
        }
        break;
      case '\002':
        p++;
        while(true)
        {
          if(recursive)
          {
            while(true)
            {
              int new_pos=syntax_strcmp(line,len,pos,text,whole_left,whole_right,line_start,recursive);
              if(new_pos) pos=new_pos;
              else break;
            }
          }
          c=syntax_get_byte(line,len,pos);
          if(c==*p)
            break;
          if((c=='\n')||(c=='\t')||(c==' '))
          {
            if(!*p) break;
            return 0;
          }
          pos++;
        }
        break;
      case '\003':
        p++;
        index=*((int *)p); index=decode_index(index); p++; p++; p++;
        for(unsigned int  i=0;i<classes[index].min;i++,pos++)
        {
          c=syntax_get_byte(line,len,pos);
          if(!classes[index].Class[c&0xff]) return 0;
        }
        for(unsigned int i=classes[index].min;i<classes[index].max;i++,pos++)
        {
          c=syntax_get_byte(line,len,pos);
          if(!classes[index].Class[c&0xff]) goto syntax_strcmp_cc_ok;
        }
        c=syntax_get_byte(line,len,pos);
        if(classes[index].Class[c&0xff]) return 0;
syntax_strcmp_cc_ok:
        pos--;
        break;
      default:
        if(*p!=syntax_get_byte(line,len,pos))
          return 0;
    }
    p++,pos++;
  }
  if(whole_right)
    if(whole_right[syntax_get_byte(line,len,pos)])
      return 0;
  if(pos==old_pos) return 0;
  return pos;
}

static int get_arg(char *line,char ***argv)
{
  char **arr=NULL,*ptr=line; int len=0,size=0;
  { //comment
    if(line[0]=='#')
      line[0]=0;
  }
  while(true)
  {
    for(ptr+=len;(*ptr==' ')||(*ptr=='\t');ptr++);
    if(!(*ptr)) break;
    for(len=1;(*(ptr+len))&&(*(ptr+len)!=' ')&&(*(ptr+len)!='\t');len++);
    arr=(char **)realloc(arr,(size+1)*sizeof(char *));
    if(!arr) {size=0; break;}
    arr[size]=(char *)malloc((len+1)*sizeof(char));
    if(!arr[size]) break;
    arr[size][len]=0;
    syntax_strncpy(arr[size],ptr,len);
    size++;
  }
  *argv=arr;
  return size;
}

static void free_arg(char **argv,int argc)
{
  for(int i=0;i<argc;i++)
    free(argv[i]);
  free(argv);
}

static const char *color_names[]={"black","blue","green","cyan","red","magenta","brown","lightgray","gray","brightblue","brightgreen","brightcyan","brightred","brightmagenta","yellow","white"};

int color_by_name(char *name)
{
  for(unsigned int i=0;i<(sizeof(color_names)/sizeof(color_names[0]));i++)
    if(!_stricmp(name,color_names[i])) return i;
  return -1;
}

static void string_to_whole(char *whole,const char *string)
{
  memset(whole,0,WHOLE_SIZE);
  for(unsigned int i=0;i<strlen(string);i++)
    whole[string[i]&0xff]=1;
}

static void load_syntax_from_file(char *dirname,char *filename,char *whole_chars_left,char *whole_chars_right)
{
  char include_file[MAX_PATH],include_full_file[MAX_PATH];
  HANDLE f=CreateFileA(filename,GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
  if(f!=INVALID_HANDLE_VALUE)
  {
    char *line; int size;
    while(true)
    {
      int res=read_line(f,&line,&size);
      if(res<2)
      {
        line[size]=0;
        char **argv; int argc;
        argc=get_arg(line,&argv);
        free(line);
        if(argc>0)
        {
          if((!_stricmp(argv[0],"include"))&&(argc>1))
          {
            unsigned long res=ExpandEnvironmentStringsA(argv[1],include_file,sizeof(include_file));
            if(!(res&&(res<=sizeof(include_file))))
              strcpy(include_file,argv[1]);
            if((strlen(include_file)>2)&&(((include_file[1]==':')&&((include_file[2]=='\\')||(include_file[2]=='/')))||((include_file[0]=='\\')&&(include_file[1]=='\\'))))
              strcpy(include_full_file,include_file);
            else
            {
              strcpy(include_full_file,dirname);
              strcat(include_full_file,include_file);
            }
            load_syntax_from_file(dirname,include_full_file,whole_chars_left,whole_chars_right);
          }
          if((!_stricmp(argv[0],"file"))&&(argc>2))
          {
            Rules *new_rules=(Rules *)realloc(rules,(rules_count+1)*sizeof(Rules));
            if(!new_rules) goto line_end;
            rules=new_rules;
            rules[rules_count].name=(TCHAR*)malloc((strlen(argv[1])+1)*sizeof(TCHAR));
            if(!rules[rules_count].name) goto line_end;
#ifdef UNICODE
            MultiByteToWideChar(CP_OEMCP,0,argv[1],-1,rules[rules_count].name,strlen(argv[1])+1);
#else
            strcpy(rules[rules_count].name,argv[1]);
#endif
            rules[rules_count].mask=(TCHAR*)malloc((strlen(argv[2])+1)*sizeof(TCHAR));
            if(!rules[rules_count].mask) goto line_end;
#ifdef UNICODE
            MultiByteToWideChar(CP_OEMCP,0,argv[2],-1,rules[rules_count].mask,strlen(argv[2])+1);
#else
            strcpy(rules[rules_count].mask,argv[2]);
#endif
            if(argc>3)
            {
              rules[rules_count].start=(TCHAR*)malloc((strlen(argv[3])+1)*sizeof(TCHAR));
              if(!rules[rules_count].start) goto line_end;
#ifdef UNICODE
              MultiByteToWideChar(CP_OEMCP,0,argv[3],-1,rules[rules_count].start,strlen(argv[3])+1);
#else
              strcpy(rules[rules_count].start,argv[3]);
#endif
            }
            rules[rules_count].contexts=NULL;
            rules[rules_count].contexts_count=0;
            rules_count++;
            string_to_whole(whole_chars_left,DEFAULT_WHOLE);
            string_to_whole(whole_chars_right,DEFAULT_WHOLE);
          }
          else if((!_stricmp(argv[0],"wholechars"))&&(argc>2))
          {
            if(!_stricmp(argv[1],"left"))
              string_to_whole(whole_chars_left,argv[2]);
            else if(!_stricmp(argv[1],"right"))
              string_to_whole(whole_chars_right,argv[2]);
          }
          else if(rules_count)
          {
            if(!_stricmp(argv[0],"context"))
            {
              if(!(rules[rules_count-1].contexts_count))
              {
                if(argc==1) goto line_end;
                if(_stricmp(argv[1],"default")) goto line_end;
                rules[rules_count-1].contexts=(Context *)malloc(sizeof(Context));
                rules[rules_count-1].contexts->left=NULL;
                rules[rules_count-1].contexts->right=NULL;
                rules[rules_count-1].contexts->fg=-1;
                rules[rules_count-1].contexts->bg=-1;
                rules[rules_count-1].contexts->exclusive=0;
                rules[rules_count-1].contexts->exclusive_left=-1;
                rules[rules_count-1].contexts->exclusive_right=-1;
                rules[rules_count-1].contexts->line_start_left=0;
                rules[rules_count-1].contexts->line_start_right=0;
                rules[rules_count-1].contexts->whole_chars_left=NULL;
                rules[rules_count-1].contexts->whole_chars_right=NULL;
                rules[rules_count-1].contexts->keywords=NULL;
                rules[rules_count-1].contexts->keywords_count=0;
                if(argc>2) rules[rules_count-1].contexts->fg=color_by_name(argv[2]);
                if(argc>3) rules[rules_count-1].contexts->bg=color_by_name(argv[3]);
                rules[rules_count-1].contexts_count=1;
              }
              else
              {
                int key_count=1,whole_left=0,whole_right=0,exclusive=0,line_start_left=0,line_start_right=0;
                char *left,*right;
                if(argc<(key_count+2)) goto line_end;
                if(!_stricmp(argv[key_count],"exclusive"))
                {
                  key_count++;
                  exclusive=1;
                }
                if(argc<(key_count+2)) goto line_end;
                if(!_stricmp(argv[key_count],"whole"))
                {
                  key_count++;
                  whole_left=whole_right=1;
                }
                else if(!_stricmp(argv[key_count],"wholeleft"))
                {
                  key_count++;
                  whole_left=1;
                }
                else if(!_stricmp(argv[key_count],"wholeright"))
                {
                  key_count++;
                  whole_right=1;
                }
                if(argc<(key_count+2)) goto line_end;
                if(!_stricmp(argv[key_count],"linestart"))
                {
                  key_count++;
                  line_start_left=1;
                }
                if(argc<(key_count+2)) goto line_end;
                left=argv[key_count++];
                if(!_stricmp(argv[key_count],"linestart"))
                {
                  key_count++;
                  line_start_right=1;
                }
                if(argc<(key_count+1)) goto line_end;
                right=argv[key_count++];
                Context *NewContexts=(Context *)realloc(rules[rules_count-1].contexts,(rules[rules_count-1].contexts_count+1)*sizeof(Context));
                if(NewContexts)
                {
                  rules[rules_count-1].contexts=NewContexts;
                  NewContexts=rules[rules_count-1].contexts+rules[rules_count-1].contexts_count;
                  NewContexts->left=(char *)malloc(strlen(left)+1);
                  if(NewContexts->left)
                    strcpy(NewContexts->left,left);
                  NewContexts->right=(char *)malloc(strlen(right)+1);
                  if(NewContexts->right)
                    strcpy(NewContexts->right,right);
                  NewContexts->fg=rules[rules_count-1].contexts[0].fg;
                  NewContexts->bg=rules[rules_count-1].contexts[0].bg;
                  NewContexts->exclusive=exclusive;
                  NewContexts->exclusive_left=-1;
                  NewContexts->exclusive_right=-1;
                  NewContexts->line_start_left=line_start_left;
                  NewContexts->line_start_right=line_start_right;
                  NewContexts->whole_chars_left=NULL;
                  if(whole_left)
                  {
                    NewContexts->whole_chars_left=(char *)malloc(WHOLE_SIZE);
                    if(NewContexts->whole_chars_left)
                      memcpy(NewContexts->whole_chars_left,whole_chars_left,WHOLE_SIZE);
                  }
                  NewContexts->whole_chars_right=NULL;
                  if(whole_left)
                  {
                    NewContexts->whole_chars_right=(char *)malloc(WHOLE_SIZE);
                    if(NewContexts->whole_chars_right)
                      memcpy(NewContexts->whole_chars_right,whole_chars_right,WHOLE_SIZE);
                  }
                  NewContexts->keywords=NULL;
                  NewContexts->keywords_count=0;
                  if(argc>key_count)
                    NewContexts->fg=color_by_name(argv[key_count++]);
                  if(argc>key_count)
                    NewContexts->bg=color_by_name(argv[key_count++]);
                  rules[rules_count-1].contexts_count++;
                }
              }
            }
            else if(!_stricmp(argv[0],"keyword"))
            {
              if(rules[rules_count-1].contexts_count)
              {
                int key_count=1,whole_left=0,whole_right=0,line_start=0,recursive=0;
                char *keyword;
                if(argc<(key_count+1)) goto line_end;
                if(!_stricmp(argv[key_count],"whole"))
                {
                  key_count++;
                  whole_left=whole_right=1;
                }
                else if(!_stricmp(argv[key_count],"wholeleft"))
                {
                  key_count++;
                  whole_left=1;
                }
                else if(!_stricmp(argv[key_count],"wholeright"))
                {
                  key_count++;
                  whole_right=1;
                }
                if(argc<(key_count+1)) goto line_end;
                if(!_stricmp(argv[key_count],"linestart"))
                {
                  key_count++;
                  line_start=1;
                }
                if(argc<(key_count+1)) goto line_end;
                if(!_stricmp(argv[key_count],"recursive"))
                {
                  key_count++;
                  recursive=1;
                }
                if(argc<(key_count+1)) goto line_end;
                keyword=argv[key_count++];
                Keyword *NewKeyword=(Keyword *)realloc(rules[rules_count-1].contexts[rules[rules_count-1].contexts_count-1].keywords,(rules[rules_count-1].contexts[rules[rules_count-1].contexts_count-1].keywords_count+1)*sizeof(Keyword));
                if(NewKeyword)
                {
                  rules[rules_count-1].contexts[rules[rules_count-1].contexts_count-1].keywords=NewKeyword;
                  NewKeyword=rules[rules_count-1].contexts[rules[rules_count-1].contexts_count-1].keywords+rules[rules_count-1].contexts[rules[rules_count-1].contexts_count-1].keywords_count;
                  NewKeyword->keyword=(char *)malloc(strlen(keyword)+1);
                  if(NewKeyword->keyword)
                    strcpy(NewKeyword->keyword,keyword);
                  NewKeyword->line_start=line_start;
                  NewKeyword->fg=rules[rules_count-1].contexts[rules[rules_count-1].contexts_count-1].fg;
                  NewKeyword->bg=rules[rules_count-1].contexts[rules[rules_count-1].contexts_count-1].bg;
                  NewKeyword->exclusive=0;
                  NewKeyword->recursive=recursive;
                  NewKeyword->whole_chars_left=NULL;
                  if(whole_left)
                  {
                    NewKeyword->whole_chars_left=(char *)malloc(WHOLE_SIZE);
                    if(NewKeyword->whole_chars_left)
                      memcpy(NewKeyword->whole_chars_left,whole_chars_left,WHOLE_SIZE);
                  }
                  NewKeyword->whole_chars_right=NULL;
                  if(whole_right)
                  {
                    NewKeyword->whole_chars_right=(char *)malloc(WHOLE_SIZE);
                    if(NewKeyword->whole_chars_right)
                      memcpy(NewKeyword->whole_chars_right,whole_chars_right,WHOLE_SIZE);
                  }
                  if(argc>key_count)
                    NewKeyword->fg=color_by_name(argv[key_count++]);
                  if(argc>key_count)
                    NewKeyword->bg=color_by_name(argv[key_count++]);
                  rules[rules_count-1].contexts[rules[rules_count-1].contexts_count-1].keywords_count++;
                }
              }
            }
          }
        }
line_end:
        free_arg(argv,argc);
        if(res)
          break;
      }
      else //error
      {
        break;
      }
    }
    CloseHandle(f);
  }
}

static void load_syntax(void)
{
  { //init char classes
    classes=(CharacterClass *)malloc(sizeof(CharacterClass));
    if(classes)
    {
      classes->min=1;
      classes->max=1;
      classes_count++;
    }
  }
  char SyntaxFile[MAX_PATH],SyntaxDir[MAX_PATH];
  char whole_chars_left[WHOLE_SIZE],whole_chars_right[WHOLE_SIZE];
#ifdef UNICODE
  WideCharToMultiByte(CP_OEMCP,0,Info.folder,-1,SyntaxDir,MAX_PATH,NULL,NULL);
  WideCharToMultiByte(CP_OEMCP,0,Info.folder,-1,SyntaxFile,MAX_PATH,NULL,NULL);
#else
  strcpy(SyntaxDir,Info.folder);
  strcpy(SyntaxFile,Info.folder);
#endif
  strcat(SyntaxFile,"syntax");
  load_syntax_from_file(SyntaxDir,SyntaxFile,whole_chars_left,whole_chars_right);
  if(rules)
  {
    for(int i=0;i<rules_count;i++)
      if(rules[i].contexts_count>1)
        for(int j=0;j<rules[i].contexts[0].keywords_count;j++)
          for(int k=1;k<rules[i].contexts_count;k++)
            if(rules[i].contexts[k].exclusive)
            {
              if((rules[i].contexts[k].exclusive_left<0)&&(!strcmp(rules[i].contexts[k].left,rules[i].contexts[0].keywords[j].keyword)))
              {
                rules[i].contexts[0].keywords[j].exclusive=1;
                rules[i].contexts[k].exclusive_left=j;
              }
              if((rules[i].contexts[k].exclusive_right<0)&&(!strcmp(rules[i].contexts[k].right,rules[i].contexts[0].keywords[j].keyword)))
              {
                rules[i].contexts[0].keywords[j].exclusive=1;
                rules[i].contexts[k].exclusive_right=j;
              }
            }
  }
}

static void free_syntax(void)
{
  for(int i=0;i<rules_count;i++)
  {
    for(int j=0;j<rules[i].contexts_count;j++)
    {
      for(int k=0;k<rules[i].contexts[j].keywords_count;k++)
      {
        free(rules[i].contexts[j].keywords[k].keyword);
        free(rules[i].contexts[j].keywords[k].whole_chars_left);
        free(rules[i].contexts[j].keywords[k].whole_chars_right);
      }
      free(rules[i].contexts[j].left);
      free(rules[i].contexts[j].right);
      free(rules[i].contexts[j].whole_chars_left);
      free(rules[i].contexts[j].whole_chars_right);
      free(rules[i].contexts[j].keywords);
    }
    free(rules[i].name);
    free(rules[i].mask);
    free(rules[i].start);
    free(rules[i].contexts);
  }
  free(rules); rules_count=0;
}

int WINAPI SetColorizeInfo(ColorizeInfo *AInfo)
{
  if((AInfo->version<AB_VERSION)||(AInfo->api!=AB_API)) return false;
  Info=*AInfo;
  load_syntax();
  return true;
};

void WINAPI _export Colorize(int index,struct ColorizeParams *params)
{
  if((index>=rules_count)||(index<0)) return;
  if(!rules[index].contexts_count) return;
  int context_start;
  const char *line;
  int linelen,startcol;
  int lColorize=0;
  int state_data=0;
  int *state=&state_data;
  int state_size=sizeof(state_data);
  if(params->data_size>=sizeof(state_data))
  {
    state=(int *)(params->data);
    state_size=params->data_size;
  }
  for(int lno=params->startline;lno<params->endline;lno++)
  {
    startcol=(lno==params->startline)?params->startcolumn:0;
    if(((lno%Info.cachestr)==0)&&(!startcol))
      if(!Info.pAddState(params->eid,lno/Info.cachestr,state_size,(unsigned char *)state)) return;
    context_start=0;
    if(lno==params->topline) lColorize=1;
    if(lColorize&&(!startcol)) Info.pAddColor(lno,-1,1,0,0);
#ifdef UNICODE
    const wchar_t* lineW=Info.pGetLine(lno,&linelen);
    line=(char*)malloc(linelen);
    if(!line) return;
    WideCharToMultiByte(CP_OEMCP,0,lineW,linelen,(char*)line,linelen,NULL,NULL);
#else
    line=Info.pGetLine(lno,&linelen);
#endif
    int pos=startcol,pos_next;
    while(pos<=linelen)
    {
      pos_next=0;
      for(int i=0;i<rules[index].contexts[state[0]].keywords_count;i++)
      {
        pos_next=syntax_strcmp(line,linelen,pos,rules[index].contexts[state[0]].keywords[i].keyword,rules[index].contexts[state[0]].keywords[i].whole_chars_left,rules[index].contexts[state[0]].keywords[i].whole_chars_right,rules[index].contexts[state[0]].keywords[i].line_start,rules[index].contexts[state[0]].keywords[i].recursive);
        if(pos_next)
        {
          if(rules[index].contexts[state[0]].keywords[i].exclusive)
            pos_next=0;
          else
          {
            if(lColorize) Info.pAddColor(lno,context_start,pos-context_start,rules[index].contexts[state[0]].fg,rules[index].contexts[state[0]].bg);
            if(lColorize) Info.pAddColor(lno,pos,pos_next-pos,rules[index].contexts[state[0]].keywords[i].fg,rules[index].contexts[state[0]].keywords[i].bg);
            pos=pos_next-1;
            context_start=pos_next;
          }
          break;
        }
      }
      if(!pos_next)
      {
        if(state[0])
        {
          pos_next=syntax_strcmp(line,linelen,pos,rules[index].contexts[state[0]].right,rules[index].contexts[state[0]].whole_chars_left,rules[index].contexts[state[0]].whole_chars_right,rules[index].contexts[state[0]].line_start_right,0);
          if(pos_next)
          {
            int end_temp=pos_next,start_temp=pos_next;
            if(rules[index].contexts[state[0]].exclusive)
            {
              end_temp=pos;
              if(rules[index].contexts[state[0]].exclusive_right>=0)
              {
                if(lColorize) Info.pAddColor(lno,pos,pos_next-pos,rules[index].contexts[0].keywords[rules[index].contexts[state[0]].exclusive_right].fg,rules[index].contexts[0].keywords[rules[index].contexts[state[0]].exclusive_right].bg);
              }
              else
                start_temp=pos;
            }
            if(lColorize) Info.pAddColor(lno,context_start,end_temp-context_start,rules[index].contexts[state[0]].fg,rules[index].contexts[state[0]].bg);
            context_start=start_temp;
            state[0]=0;
            pos=pos_next-1;
          }
        }
        else
        {
          for(int i=1;i<rules[index].contexts_count;i++)
          {
            pos_next=syntax_strcmp(line,linelen,pos,rules[index].contexts[i].left,rules[index].contexts[i].whole_chars_left,rules[index].contexts[i].whole_chars_right,rules[index].contexts[i].line_start_left,0);
            if(pos_next)
            {
              int end_temp=pos,start_temp=pos;
              if(rules[index].contexts[i].exclusive)
              {
                if(rules[index].contexts[i].exclusive_left>=0)
                {
                  if(lColorize) Info.pAddColor(lno,pos,pos_next-pos,rules[index].contexts[0].keywords[rules[index].contexts[i].exclusive_left].fg,rules[index].contexts[0].keywords[rules[index].contexts[i].exclusive_left].bg);
                }
                else
                  end_temp=pos_next;
                start_temp=pos_next;
              }
              if(lColorize) Info.pAddColor(lno,context_start,end_temp-context_start,rules[index].contexts[state[0]].fg,rules[index].contexts[state[0]].bg);
              context_start=start_temp;
              state[0]=i;
              pos=pos_next-1;
              break;
            }
          }
        }
      }
      if(params->callback)
        if(params->callback(0,lno,pos,params->param))
        {
#ifdef UNICODE
          free((void*)line);
#endif
          return;
        }
      pos++;
    }
    if(lColorize) Info.pAddColor(lno,context_start,linelen-context_start,rules[index].contexts[state[0]].fg,rules[index].contexts[state[0]].bg);
#ifdef UNICODE
    free((void*)line); line=NULL;
#endif
  }
}

unsigned long WINAPI _export GetSyntaxCount(void)
{
  return rules_count;
}

void WINAPI _export Exit(void)
{
  free_syntax();
}

int WINAPI _export GetParams(int index,int command,const char **param)
{
  switch(command)
  {
    case PAR_GET_NAME:
      if((index>=rules_count)||(index<0)) *param=(const char*)_T("");
      else *param=(const char*)rules[index].name;
      return true;
    case PAR_GET_PARAMS:
      return PAR_MASK_CACHE|PAR_SHOW_IN_LIST;
    case PAR_GET_MASK:
      if((index>=rules_count)||(index<0)) *param=(const char*)_T("");
      else *param=(const char*)rules[index].mask;
      return true;
    case PAR_CHECK_FILESTART:
      if((index<rules_count)&&(index>=0)&&rules[index].start)
      {
#ifdef UNICODE
        char* filestart=NULL;
        int res=0;
        size_t filestart_len=wcslen((const wchar_t*)*param),start_len=wcslen(rules[index].start);
        filestart=(char*)malloc(filestart_len+start_len+2);
        if(filestart)
        {
          WideCharToMultiByte(CP_OEMCP,0,(const wchar_t*)*param,filestart_len+1,filestart,filestart_len+1,NULL,NULL);
          WideCharToMultiByte(CP_OEMCP,0,rules[index].start,start_len+1,(filestart+filestart_len+1),start_len+1,NULL,NULL);
          res=syntax_strcmp(filestart,filestart_len,0,(filestart+filestart_len+1),NULL,NULL,true,false);
          free(filestart);
        }
        return res;
#else
        return syntax_strcmp(*param,strlen(*param),0,rules[index].start,NULL,NULL,true,false);
#endif
      }
      break;
  }
  return false;
}

#ifdef __cplusplus
extern "C"{
#endif
  bool WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

bool WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  return true;
}
