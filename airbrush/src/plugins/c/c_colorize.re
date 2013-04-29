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
#include <tchar.h>
#include "abplugin.h"
#include "../abpairs.h"
#include "../plugins/c/abc.h"

typedef unsigned short UTCHAR;

#define YYCTYPE unsigned long
#define YYCURSOR yycur
#define YYLIMIT yyend
#define YYMARKER yytmp
#define YYFILL(n)

/*!re2c
any = [\U00000001-\U0000ffff];
O = [0-7];
D = [0-9];
L = [a-zA-Z_];
H = [a-fA-F0-9];
E = [Ee] [+-]? D+;
FS  = [fFlL];
IS  = [uUlL]*;
ESC = [\\] ([abfnrtv?'"\\] | "x" H+ | O+);
STR = ("L"|"u8"|"u"|"U");
*/

void WINAPI Colorize(int index,struct ColorizeParams *params)
{
  (void)index;
  const UTCHAR *commentstart;
  const UTCHAR *line;
  int linelen,startcol;
  int lColorize=0;
  int state_data=PARSER_CLEAR;
  int *state=&state_data;
  int state_size=sizeof(state_data);
  const UTCHAR *yycur,*yyend,*yytmp=NULL,*yytok=NULL;
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
    if(lColorize&&(!startcol)) Info.pDelColor(lno);
    line=(const UTCHAR*)Info.pGetLine(lno,&linelen);
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
  "alignas"|"alignof"|"and"|"and_eq"|"asm"|"auto"|
  "bitand"|"bitor"|"bool"|"break"|
  "case"|"catch"|"char"|"char16_t"|"char32_t"|"class"|"compl"|"const"|"constexpr"|"const_cast"|"continue"|
  "decltype"|"default"|"delete"|"do"|"double"|"dynamic_cast"|
  "else"|"enum"|"explicit"|"export"|"extern"|
  "false"|"float"|"for"|"friend"|
  "goto"|"if"|"inline"|"int"|"long"|"mutable"|
  "namespace"|"new"|"noexcept"|"not"|"not_eq"|"nullptr"|
  "operator"|"or"|"or_eq"|"private"|"protected"|"public"|
  "register"|"reinterpret_cast"|"return"|
  "short"|"signed"|"sizeof"|"static"|"static_assert"|"static_cast"|"struct"|"switch"|
  "template"|"this"|"thread_local"|"throw"|"true"|"try"|"typedef"|"typeid"|"typename"|
  "union"|"unsigned"|"using"|
  "virtual"|"void"|"volatile"|
  "wchar_t"|"while"|"xor"|"xor_eq"|
  "__int64"|"size_t"
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal); goto colorize_clear; }
  L(L|D)*
  { goto colorize_clear; }
  ("0"[xX]H+IS?)|("0"O+IS?)|(D+IS?)|
  (D+E FS?)|(D*"."D+E?FS?)|(D+"."D*E?FS?)
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_NUMBER1,EPriorityNormal); goto colorize_clear; }
  STR?["]
  { state[0]=PARSER_STRING; commentstart=yytok; goto colorize_string; }
  (STR?['] (ESC|any\[\\'])* ['])
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_STRING1,EPriorityNormal); goto colorize_clear; }
  "..."|">>="|"<<="|"+="|"-="|"*="|"/="|"%="|"&="|"^="|"|="|">>"|"<<"|"++"|"--"|"->"|"&&"|"||"|
  "<="|">="|"=="|"!="|","|":"|"="|"."|"&"|"!"|"~"|
  "-"|"+"|"*"|"/"|"%"|"<"|">"|"^"|"|"|"?"
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal);
    goto colorize_clear;
  }
  "(" {PUSH_PAIR(0)}
  ")" {POP_PAIR(0,0)}
  "[" {PUSH_PAIR(1)}
  "]" {POP_PAIR(1,1)}
  "{" {PUSH_PAIR(2)}
  "}" {POP_PAIR(2,2)}
  ";" { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_KEYWORD3,EPriorityNormal); goto colorize_clear; }
  [ \t]*"#"[ \t]*[a-zA-Z]+
  {
    if((yytok==line)&&lColorize)
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_DEFINE,EPriorityNormal); goto colorize_clear;
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
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_COMMENT,EPriorityNormal);
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
      if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_COMMENT,EPriorityNormal);
      goto colorize_end;
    }
    goto colorize_comment2;
  }
  [Ff][Ii][Xx][Mm][Ee]
  {
    if(lColorize)
    {
      Info.pAddColor(lno,commentstart-line,yytok-commentstart,colors+HC_COMMENT,EPriorityNormal);
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_FIXME,EPriorityNormal);
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
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_STRING1,EPriorityNormal);
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
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors+HC_COMMENT,EPriorityNormal);
    if(state[0]==PARSER_STRING)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors+HC_STRING1,EPriorityNormal);
  }
colorize_exit:
  PairStackClear(params->LocalHeap,&hl_state);
}
