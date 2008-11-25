/*
    c_colorize.re
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
#include <stdio.h>
#include "abplugin.h"
#include "../abpairs.h"
#include "../plugins/c/abc.h"

#define YYCTYPE unsigned char
#define YYCURSOR yycur
#define YYLIMIT yyend
#define YYMARKER yytmp
#define YYFILL(n)

/*!re2c
any = [\001-\377];
O = [0-7];
D = [0-9];
L = [a-zA-Z_];
H = [a-fA-F0-9];
E = [Ee] [+-]? D+;
FS  = [fFlL];
IS  = [uUlL]*;
ESC = [\\] ([abfnrtv?'"\\] | "x" H+ | O+);
*/

void WINAPI _export Colorize(int index,struct ColorizeParams *params)
{
  const unsigned char *commentstart;
  const unsigned char *line;
  int linelen,startcol;
  int lColorize=0;
  int state_data=PARSER_CLEAR;
  int *state=&state_data;
  int state_size=sizeof(state_data);
  const unsigned char *yycur,*yyend,*yytmp,*yytok=NULL;
  struct PairStack *hl_state=NULL;
  int hl_row; int hl_col;
  if(params->data_size>=sizeof(state_data))
  {
    state=(int *)(params->data);
    state_size=params->data_size;
  }
  Info.pGetCursor(&hl_row,&hl_col);
  for(int lno=params->startline;lno<params->endline;lno++,yytok=NULL)
  {
    startcol=(lno==params->startline)?params->startcolumn:0;
    if(((lno%Info.cachestr)==0)&&(!startcol))
      if(!Info.pAddState(params->eid,lno/Info.cachestr,state_size,(unsigned char *)state)) goto colorize_exit;
    if(lno==params->topline) lColorize=1;
    if(lColorize&&(!startcol)) Info.pAddColor(lno,-1,1,0,0);
    line=(const unsigned char *)Info.pGetLine(lno,&linelen);
    commentstart=line+startcol;
    yycur=line+startcol;
    yyend=line+linelen;
colorize_clear:
    if(yytok) if(params->callback) if(params->callback(0,lno,yytok-line,params->param)) goto colorize_exit;
    yytok=yycur;
    if(params->callback) if(params->callback(1,lno,yytok-line,params->param)) goto colorize_exit;
    if(state[0]==PARSER_COMMENT1) goto colorize_comment1;
    if(state[0]==PARSER_STRING) goto colorize_string;
/*!re2c
  "/*"
  { state[0]=PARSER_COMMENT1; commentstart=yytok; goto colorize_comment1; }
  "//"
  { commentstart=yytok; goto colorize_comment2; }
  "auto"|"break"|"case"|"char"|"const"|"continue"|"default"|"do"|"double"|"else"|"enum"|
  "extern"|"float"|"for"|"goto"|"if"|"int"|"long"|"register"|"return"|"short"|"signed"|
  "sizeof"|"static"|"struct"|"switch"|"typedef"|"union"|"unsigned"|"void"|"volatile"|"while"|
  "asm"|"catch"|"class"|"friend"|"delete"|"inline"|"new"|"operator"|"private"|"protected"|"public"|
  "this"|"throw"|"template"|"try"|"virtual"|"bool"|"const_cast"|"dynamic_cast"|"explicit"|"false"|
  "mutable"|"namespace"|"reinterpret_cast"|"static_cast"|"true"|"typeid"|"typename"|"using"|"wchar_t"
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_KEYWORD1],colors[HC_KEYWORD1+1]); goto colorize_clear; }
  L(L|D)*
  { goto colorize_clear; }
  ("0"[xX]H+IS?)|("0"D+IS?)|(D+IS?)|
  (D+E FS?)|(D*"."D+E?FS?)|(D+"."D*E?FS?)
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_NUMBER1],colors[HC_NUMBER1+1]); goto colorize_clear; }
  ["]
  { state[0]=PARSER_STRING; commentstart=yytok; goto colorize_string; }
  (['] (ESC|any\[\\'])* ['])
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_STRING1],colors[HC_STRING1+1]); goto colorize_clear; }
  "..."|">>="|"<<="|"+="|"-="|"*="|"/="|"%="|"&="|"^="|"|="|">>"|"<<"|"++"|"--"|"->"|"&&"|"||"|
  "<="|">="|"=="|"!="|","|":"|"="|"."|"&"|"!"|"~"|
  "-"|"+"|"*"|"/"|"%"|"<"|">"|"^"|"|"|"?"
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_KEYWORD1],colors[HC_KEYWORD1+1]);
    goto colorize_clear;
  }
  "(" {PUSH_PAIR(0)}
  ")" {POP_PAIR(0,0)}
  "[" {PUSH_PAIR(1)}
  "]" {POP_PAIR(1,1)}
  "{" {PUSH_PAIR(2)}
  "}" {POP_PAIR(2,2)}
  ";" { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_KEYWORD3],colors[HC_KEYWORD3+1]); goto colorize_clear; }
  [ \t]*"#"[ \t]*[a-zA-Z]+
  {
    if((yytok==line)&&lColorize)
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_DEFINE],colors[HC_DEFINE+1]); goto colorize_clear;
  }
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

colorize_comment1:
    yytok=yycur;
/*!re2c
  "*/"
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors[HC_COMMENT],colors[HC_COMMENT+1]);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_comment1; }
  any
  { goto colorize_comment1; }
*/
colorize_comment2:
    yytok=yycur;
/*!re2c
  [\000]
  {
    if(yytok==yyend)
    {
      if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors[HC_COMMENT],colors[HC_COMMENT+1]);
      goto colorize_end;
    }
    goto colorize_comment2;
  }
  [Ff][Ii][Xx][Mm][Ee]
  {
    if(lColorize)
    {
      Info.pAddColor(lno,commentstart-line,yytok-commentstart,colors[HC_COMMENT],colors[HC_COMMENT+1]);
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_FIXME],colors[HC_FIXME+1]);
      commentstart=yycur;
    }
    goto colorize_comment2;
  }
  any
  { goto colorize_comment2; }
*/
colorize_string:
    yytok=yycur;
/*!re2c
  ESC
  { goto colorize_string; }
  ["]
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors[HC_STRING1],colors[HC_STRING1+1]);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string; }
  any
  { goto colorize_string; }
*/
colorize_end:
    if(state[0]==PARSER_COMMENT1)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors[HC_COMMENT],colors[HC_COMMENT+1]);
    if(state[0]==PARSER_STRING)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors[HC_STRING1],colors[HC_STRING1+1]);
  }
colorize_exit:
  PairStackClear(params->LocalHeap,&hl_state);
}
