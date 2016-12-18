/*
    moon_colorize.re
    Copyright (C) 2014 zg

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
#include <stdio.h>
#include <tchar.h>
#include "abplugin.h"
#include "../abpairs.h"
#include "../plugins/moon/abmoon.h"

typedef unsigned short UTCHAR;

#define YYCTYPE unsigned long
#define YYCURSOR yycur
#define YYLIMIT yyend
#define YYMARKER yytmp
#define YYCTXMARKER yyctxtmp
#define YYFILL(n)

/*!re2c
any = [\U00000001-\U0000ffff];
D = [0-9];
L = [a-zA-Z_];
H = [a-fA-F0-9];
E = [Ee] [+-]? D+;
ESC1 = [\\] ([abfnrtv"\\] | D{1,3});
ESC2 = [\\] ([abfnrtv'\\] | D{1,3});
*/

static int GetLevel(const UTCHAR* string)
{
  int result=0;
  while(*string&&*string!=_T('[')) ++string;
  if(*string)
  {
    while(*++string==_T('=')) ++result;
    if(*string!=_T('[')) result=0;
  }
  return result;
}

static bool MatchLevel(const UTCHAR* string,int level)
{
  int count=-1;
  if(*string==_T(']'))
  {
    count=0;
    while(*++string==_T('=')) ++count;
    if(*string!=_T(']')) count=-1;
  }
  return count==level;
}

static const UTCHAR* ParseInterpolation(const UTCHAR* begin,const UTCHAR* end)
{
  MoonState state={PARSER_CLEAR,0};
  const UTCHAR *yycur,*yyend,*yytmp=NULL,*yytok=NULL;
  intptr_t brackets=0;
  yycur=begin;
  yyend=end;
colorize_clear:
  yytok=yycur;
/*!re2c
  "[" "="* "[" { state.State=PARSER_STRING1; state.Level=GetLevel(yytok); goto colorize_string1; }
  ["] { state.State=PARSER_STRING2; goto colorize_string2; }
  ['] { state.State=PARSER_STRING3; goto colorize_string3; }
  "{" { ++brackets; goto colorize_clear; }
  "}" { --brackets; if(brackets==-1) return yycur; goto colorize_clear; }
  [\000] { if(yytok==yyend) goto colorize_end; goto colorize_clear; }
  any { goto colorize_clear; }
*/
colorize_string1:
  yytok=yycur;
/*!re2c
  "]" "="* "]"
  {
    if(MatchLevel(yytok,state.Level))
    {
      state.State=PARSER_CLEAR;
      state.Level=0;
      goto colorize_clear;
    }
    yycur=yytok+1;
    goto colorize_string1;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string1; }
  any
  { goto colorize_string1; }
*/
colorize_string2:
  yytok=yycur;
/*!re2c
  ESC1
  { goto colorize_string2; }
  "#{" /*}*/
  {
    const UTCHAR* newcur=ParseInterpolation(yycur,yyend);
    if(newcur) yycur=newcur;
    goto colorize_string2;
  }
  ["]
  {
    state.State=PARSER_CLEAR;
    goto colorize_clear;
  }
  [\000]
  {
    if(yytok==yyend) goto colorize_end;
    goto colorize_string2;
  }
  any
  { goto colorize_string2; }
*/
colorize_string3:
  yytok=yycur;
/*!re2c
  ESC2
  { goto colorize_string3; }
  [']
  {
    state.State=PARSER_CLEAR;
    goto colorize_clear;
  }
  [\000]
  {
    if(yytok==yyend) goto colorize_end;
    goto colorize_string3;
  }
  any
  { goto colorize_string3; }
*/
colorize_end:
  return NULL;
}

bool ColorizeInternal(intptr_t lno,const UTCHAR* line,intptr_t startcol,intptr_t linelen,intptr_t hl_row,intptr_t hl_col,PairStack* hl_state,MoonState* state,ColorizeParams* params,bool Interpolation)
{
  const UTCHAR* yycur,*yyend,*yytmp=NULL,*yyctxtmp,*yytok=NULL;
  const UTCHAR* commentstart=line+startcol;
  intptr_t brackets=0;
  yycur=line+startcol;
  yyend=line+linelen;
colorize_clear:
  if(yytok) if(params->callback) if(params->callback(0,lno,yytok-line,params->param)) return true;
  yytok=yycur;
  if(params->callback) if(params->callback(1,lno,yytok-line,params->param)) return true;
  if(state[0].State==PARSER_STRING1) goto colorize_string1;
  if(state[0].State==PARSER_STRING2) goto colorize_string2;
  if(state[0].State==PARSER_STRING3) goto colorize_string3;
/*!re2c
  "--"
  { if(Interpolation) { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal); goto colorize_clear; } else { commentstart=yytok; goto colorize_comment2; } }
  ("."|"\\")L(L|D)*
  {
    Info.pAddColor(params,lno,yytok-line,1,colors+HC_KEYWORD1,EPriorityNormal);
    goto colorize_clear;
  }
  "and"|"break"|"class"|"continue"|"do"|"else"|"elseif"|"export"|"extends"|
  "false"|"for"|"from"|"if"|"import"|"in"|"local"|"not"|"or"|"nil"|"return"|
  "self"|"super"|"switch"|"then"|"true"|"unless"|"using"|"when"|"while"|"with"
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal); goto colorize_clear; }
  (L(L|D)*[ \t]*":")|(":"[ \t]*L(L|D)*)|":"
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_FIELD,EPriorityNormal); goto colorize_clear; }
  [@]{3,}L(L|D)*
  { goto colorize_clear; }
  ([@]{1,2}L(L|D)*)|"@"
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_CLASS,EPriorityNormal); goto colorize_clear; }
  L(L|D)*
  { goto colorize_clear; }
  ("0"[xX]H+)
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_NUMBER2,EPriorityNormal); goto colorize_clear; }
  D+/".."
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_NUMBER1,EPriorityNormal); goto colorize_clear; }
  (D+)|(D+E)|(D*"."D+E?)|(D+"."D*E?)
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_NUMBER1,EPriorityNormal); goto colorize_clear; }
  "[" "="* "["
  { state[0].State=PARSER_STRING1; state[0].Level=GetLevel(yytok); commentstart=yytok; goto colorize_string1; }
  ["]
  { state[0].State=PARSER_STRING2; state[0].Level=0; commentstart=yytok; goto colorize_string2; }
  [']
  { state[0].State=PARSER_STRING3; state[0].Level=0; commentstart=yytok; goto colorize_string3; }
  "."{4,}
  { goto colorize_clear; }
  ".."/"."D
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal);
    goto colorize_clear;
  }
  "+"|"-"|"*"|"/"|"%"|"^"|"#"|
  "=="|"~="|"!="|"<="|">="|"<"|">"|"="|
  ";"|","|".."|"..."|
  "->"|"=>"|"!"
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal);
    goto colorize_clear;
  }
  "(" {PUSH_PAIR(0)}
  ")" {POP_PAIR(0,0)}
  "[" {PUSH_PAIR(1)}
  "]" {POP_PAIR(1,1)}
  "{" { ++brackets; PUSH_PAIR(2)}
  "}" {--brackets; if(Interpolation&&brackets==-1) return yycur; POP_PAIR(2,2)}
  [ \t\v\f]+ { goto colorize_clear; }

  [\000]
  {
    if(yytok==yyend) goto colorize_end;
    goto colorize_clear;
  }
  any
  {
    goto colorize_clear;
  }
*/

colorize_comment2:
  yytok=yycur;
/*!re2c
  [\000]
  {
    if(yytok==yyend)
    {
      Info.pAddColor(params,lno,commentstart-line,yycur-commentstart-1,colors+HC_COMMENT,EPriorityNormal);
      goto colorize_end;
    }
    goto colorize_comment2;
  }
  'fixme'
  {
    Info.pAddColor(params,lno,commentstart-line,yytok-commentstart,colors+HC_COMMENT,EPriorityNormal);
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_FIXME,EPriorityNormal);
    commentstart=yycur;
    goto colorize_comment2;
  }
  any
  { goto colorize_comment2; }
*/
colorize_string1:
  yytok=yycur;
/*!re2c
  "]" "="* "]"
  {
    if(MatchLevel(yytok,state[0].Level))
    {
      Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING1,EPriorityNormal);
      state[0].State=PARSER_CLEAR;
      state[0].Level=0;
      goto colorize_clear;
    }
    yycur=yytok+1;
    goto colorize_string1;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string1; }
  any
  { goto colorize_string1; }
*/
colorize_string2:
  yytok=yycur;
/*!re2c
  ESC1
  { goto colorize_string2; }
  "#{" /*}*/
  {
    const UTCHAR* newcur=ParseInterpolation(yycur,yyend);
    if(newcur&&Info.pInterpolation((const wchar_t*)yytok,newcur-yytok))
    {
      Info.pAddColor(params,lno,commentstart-line,yytok-commentstart,colors+HC_STRING1,EPriorityNormal);
      PUSH_PAIR_0(3,HC_INTERPOL)
      MoonState newstate={PARSER_CLEAR,0};
      ColorizeParams newparams=*params;
      newparams.callback=NULL;
      ColorizeInternal(lno,line,yycur-line,linelen,hl_row,hl_col,hl_state,&newstate,&newparams,true);
      yytok=newcur-1;
      yycur=newcur;
      POP_PAIR_0(3,3,HC_INTERPOL)
      commentstart=yycur;
    }
    goto colorize_string2;
  }
  ["]
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING1,EPriorityNormal);
    state[0].State=PARSER_CLEAR;
    state[0].Level=0;
    goto colorize_clear;
  }
  [\000]
  {
    if(yytok==yyend)
    {
      Info.pAddColor(params,lno,commentstart-line,yycur-commentstart-1,colors+HC_STRING1,EPriorityNormal);
      goto colorize_end;
    }
    goto colorize_string2;
  }
  any
  { goto colorize_string2; }
*/
colorize_string3:
  yytok=yycur;
/*!re2c
  ESC2
  { goto colorize_string3; }
  [']
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING1,EPriorityNormal);
    state[0].State=PARSER_CLEAR;
    state[0].Level=0;
    goto colorize_clear;
  }
  [\000]
  {
    if(yytok==yyend)
    {
      Info.pAddColor(params,lno,commentstart-line,yycur-commentstart-1,colors+HC_STRING1,EPriorityNormal);
      goto colorize_end;
    }
    goto colorize_string3;
  }
  any
  { goto colorize_string3; }
*/
colorize_end:
  if(state[0].State==PARSER_STRING1||state[0].State==PARSER_STRING2||state[0].State==PARSER_STRING3)
    Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_STRING1,EPriorityNormal);
  return false;
}

void WINAPI Colorize(intptr_t index,struct ColorizeParams *params)
{
  (void)index;
  const UTCHAR *line;
  intptr_t linelen;
  intptr_t startcol;
  MoonState state_data={PARSER_CLEAR,0};
  MoonState* state=&state_data;
  int state_size=sizeof(state_data);
  struct PairStack *hl_state=NULL;
  intptr_t hl_row; intptr_t hl_col;
  if(params->data_size>=sizeof(state_data))
  {
    state=(MoonState*)(params->data);
    state_size=params->data_size;
  }
  Info.pGetCursor(params->eid,&hl_row,&hl_col);
  for(int lno=params->startline;lno<params->endline;lno++)
  {
    startcol=(lno==params->startline)?params->startcolumn:0;
    if(((lno%Info.cachestr)==0)&&(!startcol))
      if(!Info.pAddState(params->eid,lno/Info.cachestr,state_size,(unsigned char *)state)) break;
    line=(const UTCHAR*)Info.pGetLine(params->eid,lno,&linelen);
    if(ColorizeInternal(lno,line,startcol,linelen,hl_row,hl_col,hl_state,state,params,false)) break;
  }
  PairStackClear(params->LocalHeap,&hl_state);
}
