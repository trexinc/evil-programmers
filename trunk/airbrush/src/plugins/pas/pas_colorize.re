/*
    pas_colorize.re
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
#include "../plugins/pas/abpas.h"

#ifdef UNICODE
typedef unsigned short UTCHAR;
#else
typedef unsigned char UTCHAR;
#endif

#define YYCTYPE unsigned long
#define YYCURSOR yycur
#define YYLIMIT yyend
#define YYMARKER yytmp
#define YYCTXMARKER yyctxtmp
#define YYFILL(n)

/*!re2c
any = [\U00000001-\U0000ffff];
anywzero = [\U00000000-\U0000ffff];
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
  const UTCHAR *commentstart;
  const UTCHAR *line;
  int linelen,startcol;
  int lColorize=0;
  int state_data=PARSER_CLEAR;
  int *state=&state_data;
  int state_size=sizeof(state_data);
  const UTCHAR *yycur,*yyend,*yytmp,*yyctxtmp,*yytok=NULL;
  struct PairStack *hl_state=NULL;
  int hl_row; int hl_col;
  if(params->data_size>=sizeof(state_data))
  {
    state=(int *)(params->data);
    state_size=params->data_size;
  }
  Info.pGetCursor(&hl_row,&hl_col);
  for(int lno=params->startline;lno<params->endline;lno++)
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
    if(state[0]==PARSER_COMMENT2) goto colorize_comment2;
    if(state[0]==PARSER_ASM) goto colorize_asm;
/*!re2c
  "{"
  {
    state[0]=PARSER_COMMENT1|(state[0]<<16);
    commentstart=yytok; goto colorize_comment1;
  }
  "(*"
  {
    state[0]=PARSER_COMMENT2|(state[0]<<16);
    commentstart=yytok; goto colorize_comment2;
  }
  [aA][sS][mM]/anywzero\[a-zA-Z0-9_#$]
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_ASM],colors[HC_ASM+1],EPriorityNormal);
    state[0]=PARSER_ASM;
    goto colorize_asm;
  }
  "//"
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yyend-yytok,colors[HC_COMMENT1],colors[HC_COMMENT1+1],EPriorityNormal);
    goto colorize_end;
  }
  ['](any\[\n'])*[']
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_STRING1],colors[HC_STRING1+1],EPriorityNormal);
    goto colorize_clear;
  }
  ([cC][oO][nN][sS][tT][rR][uU][cC][tT][oO][rR])|([dD][eE][sS][tT][rR][uU][cC][tT][oO][rR])|
  ([pP][rR][oO][cC][eE][dD][uU][rR][eE])|([wW][iI][tT][hH])|([oO][fF])|
  ([fF][iI][nN][aA][lL][lL][yY])|([eE][xX][cC][eE][pP][tT])|([fF][oO][rR])|
  ([tT][oO])|([dD][oO][wW][nN][tT][oO])|([tT][yY][pP][eE])|([iI][nN][tT][eE][rR][fF][aA][cC][eE])|
  ([iI][nN][iI][tT][iI][aA][lL][iI][zZ][aA][tT][iI][oO][nN])|([fF][iI][nN][aA][lL][iI][zZ][aA][tT][iI][oO][nN])|
  ([dD][eE][fF][aA][uU][lL][tT])|([pP][rR][iI][vV][aA][tT][eE])|([pP][uU][bB][lL][iI][cC])|
  ([pP][rR][oO][tT][eE][cC][tT][eE][dD])|([pP][uU][bB][lL][iI][sS][hH][eE][dD])|([aA][uU][tT][oO][mM][aA][tT][eE][dD])|
  ([pP][rR][oO][pP][eE][rR][tT][yY])|([pP][rR][oO][gG][rR][aA][mM])|([rR][eE][aA][dD])|([wW][rR][iI][tT][eE])|
  ([oO][vV][eE][rR][rR][iI][dD][eE])|([nN][iI][lL])|([rR][aA][iI][sS][eE])|([oO][nN])|
  ([sS][eE][tT])|([xX][oO][rR])|([sS][hH][rR])|([sS][hH][lL])|([aA][rR][gG][sS])|([iI][fF])|([tT][hH][eE][nN])|
  ([eE][lL][sS][eE])|([eE][nN][dD][iI][fF])|([gG][oO][tT][oO])|([wW][hH][iI][lL][eE])|([dD][oO])|([vV][aA][rR])|
  ([tT][rR][uU][eE])|([fF][aA][lL][sS][eE])|([oO][rR])|([aA][nN][dD])|([nN][oO][tT])|([mM][oO][dD])|([uU][nN][iI][tT])|
  ([fF][uU][nN][cC][tT][iI][oO][nN])|([uU][sS][eE][sS])|([eE][xX][tT][eE][rR][nN][aA][lL])|([cC][oO][nN][sS][tT])|
  ([iI][nN][hH][eE][rR][iI][tT][eE][dD])|([dD][iI][vV])|([vV][iI][rR][tT][uU][aA][lL])|
  ([dD][yY][nN][aA][mM][iI][cC])|([aA][bB][sS][tT][rR][aA][cC][tT])|([aA][rR][rR][aA][yY])|([bB][rR][eE][aA][kK])|
  ([cC][oO][nN][tT][iI][nN][uU][eE])|([rR][eE][sS][oO][uU][rR][cC][eE][sS][tT][rR][iI][nN][gG])|([eE][xX][pP][oO][rR][tT][sS])
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_KEYWORD1],colors[HC_KEYWORD1+1],EPriorityNormal);
    goto colorize_clear;
  }
  ([bB][eE][gG][iI][nN]) {PUSH_PAIR(2)}
  ([lL][iI][bB][rR][aA][rR][yY]) {PUSH_PAIR(3)}
  ([iI][mM][pP][lL][eE][mM][eE][nN][tT][aA][tT][iI][oO][nN]) {PUSH_PAIR(4)}
  ([cC][aA][sS][eE]) {PUSH_PAIR(5)}
  ([rR][eE][cC][oO][rR][dD]) {PUSH_PAIR(6)}
  ([oO][bB][jJ][eE][cC][tT]) {PUSH_PAIR(7)}
  ([cC][lL][aA][sS][sS]) {PUSH_PAIR(8)}
  ([tT][rR][yY]) {PUSH_PAIR(9)}
  ([eE][nN][dD]) {POP_PAIR(2,9)}
  ([rR][eE][pP][eE][aA][tT]) {PUSH_PAIR(10)}
  ([uU][nN][tT][iI][lL]) {POP_PAIR(10,10)}
  L(L|D)*
  { goto colorize_clear; }
  /*D+
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_NUMBER1],colors[HC_NUMBER1+1]);
    goto colorize_clear;
  }*/
  D+".."
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_NUMBER1],colors[HC_NUMBER1+1],EPriorityNormal);
    goto colorize_clear;
  }
  D+(".")?D*([Ee]([+-])?D+)?
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_NUMBER1],colors[HC_NUMBER1+1],EPriorityNormal);
    goto colorize_clear;
  }
  "$"H+
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_NUMBER1],colors[HC_NUMBER1+1],EPriorityNormal);
    goto colorize_clear;
  }
  ("#"D+)|("#$"H+)
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_STRING1],colors[HC_STRING1+1],EPriorityNormal);
    goto colorize_clear;
  }

  [\-=+/*^@:\<>\,]
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_KEYWORD1],colors[HC_KEYWORD1+1],EPriorityNormal);
    goto colorize_clear;
  }
  "(" {PUSH_PAIR(0)}
  ")" {POP_PAIR(0,0)}
  "[" {PUSH_PAIR(1)}
  "]" {POP_PAIR(1,1)}
  ";"
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_KEYWORD3],colors[HC_KEYWORD3+1],EPriorityNormal); goto colorize_clear; }

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
  "}"
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors[HC_COMMENT1],colors[HC_COMMENT1+1],EPriorityNormal);
    state[0]=state[0]>>16;
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
  "*)"
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors[HC_COMMENT1],colors[HC_COMMENT1+1],EPriorityNormal);
    state[0]=state[0]>>16;
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_comment2; }
  any
  { goto colorize_comment2; }
*/
colorize_asm:
    yytok=yycur;
/*!re2c
  "{"
  {
    state[0]=PARSER_COMMENT1|(state[0]<<16);
    commentstart=yytok; goto colorize_comment1;
  }
  "(*"
  {
    state[0]=PARSER_COMMENT2|(state[0]<<16);
    commentstart=yytok; goto colorize_comment2;
  }
  [eE][nN][dD]/anywzero\[a-zA-Z0-9_#$]
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_ASM],colors[HC_ASM+1],EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  "//"
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yyend-yytok,colors[HC_COMMENT1],colors[HC_COMMENT1+1],EPriorityNormal);
    goto colorize_end;
  }
  (L|D)*
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_ASM],colors[HC_ASM+1],EPriorityNormal);
    goto colorize_asm;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_asm; }
  any
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_ASM],colors[HC_ASM+1],EPriorityNormal);
    goto colorize_asm;
  }
*/
colorize_end:
    if((state[0]&0xffff)==PARSER_COMMENT1)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors[HC_COMMENT1],colors[HC_COMMENT1+1],EPriorityNormal);
    if((state[0]&0xffff)==PARSER_COMMENT2)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors[HC_COMMENT2],colors[HC_COMMENT2+1],EPriorityNormal);
  }
colorize_exit:
  PairStackClear(params->LocalHeap,&hl_state);
}
