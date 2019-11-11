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

typedef unsigned short UTCHAR;

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
void WINAPI Colorize(intptr_t index,struct ColorizeParams *params)
{
  (void)index;
  const UTCHAR *commentstart;
  const UTCHAR *line;
  intptr_t linelen;
  int startcol;
  int state_data=PARSER_CLEAR;
  int *state=&state_data;
  int state_size=sizeof(state_data);
  const UTCHAR *yycur,*yyend,*yytmp,*yyctxtmp,*yytok=NULL;
  struct PairStack *hl_state=NULL;
  intptr_t hl_row; intptr_t hl_col;
  if(params->data_size>=sizeof(state_data))
  {
    state=reinterpret_cast<int*>(params->data);
    state_size=params->data_size;
  }
  Info.pGetCursor(params->eid,&hl_row,&hl_col);
  INIT_PAIR;
  for(int lno=params->startline;lno<params->endline;lno++)
  {
    startcol=(lno==params->startline)?params->startcolumn:0;
    if(((lno%Info.cachestr)==0)&&(!startcol))
      if(!Info.pAddState(params->eid,lno/Info.cachestr,state_size,reinterpret_cast<unsigned char*>(state))) goto colorize_exit;
    line=reinterpret_cast<const UTCHAR*>(Info.pGetLine(params->eid,lno,&linelen));
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
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ASM,EPriorityNormal);
    state[0]=PARSER_ASM;
    goto colorize_asm;
  }
  "//"
  {
    Info.pAddColor(params,lno,yytok-line,yyend-yytok,colors+HC_COMMENT1,EPriorityNormal);
    goto colorize_end;
  }
  ['](any\[\n'])*[']
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_STRING1,EPriorityNormal);
    goto colorize_clear;
  }
  ('constructor')|('destructor')|
  ('procedure')|('with')|('of')|
  ('finally')|('except')|('for')|
  ('to')|('downto')|('type')|('interface')|
  ('initialization')|('finalization')|
  ('default')|('private')|('public')|
  ('protected')|('published')|('automated')|
  ('property')|('program')|('read')|('write')|
  ('override')|('nil')|('raise')|('on')|
  ('set')|('xor')|('shr')|('shl')|('args')|('if')|('then')|
  ('else')|('endif')|('goto')|('while')|('do')|('var')|
  ('true')|('false')|('or')|('and')|('not')|('mod')|('unit')|
  ('function')|('uses')|('external')|('const')|
  ('inherited')|('div')|('virtual')|
  ('dynamic')|('abstract')|('array')|('break')|
  ('continue')|('resourcestring')|('exports')
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal);
    goto colorize_clear;
  }
  ('begin') {PUSH_PAIR(2)}
  ('library') {PUSH_PAIR(3)}
  ('implementation') {PUSH_PAIR(4)}
  ('case') {PUSH_PAIR(5)}
  ('record') {PUSH_PAIR(6)}
  ('object') {PUSH_PAIR(7)}
  ('class') {PUSH_PAIR(8)}
  ('try') {PUSH_PAIR(9)}
  ('end') {POP_PAIR_EXT(2,9)}
  ('repeat') {PUSH_PAIR(10)}
  ('until') {POP_PAIR(10)}
  L(L|D)*
  { goto colorize_clear; }
  /*D+
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_NUMBER1);
    goto colorize_clear;
  }*/
  D+".."
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_NUMBER1,EPriorityNormal);
    goto colorize_clear;
  }
  D+(".")?D*([Ee]([+-])?D+)?
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_NUMBER1,EPriorityNormal);
    goto colorize_clear;
  }
  "$"H+
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_NUMBER1,EPriorityNormal);
    goto colorize_clear;
  }
  ("#"D+)|("#$"H+)
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_STRING1,EPriorityNormal);
    goto colorize_clear;
  }

  [\-=+/*^@:\<>\,]
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal);
    goto colorize_clear;
  }
  "(" {PUSH_PAIR(0)}
  ")" {POP_PAIR(0)}
  "[" {PUSH_PAIR(1)}
  "]" {POP_PAIR(1)}
  ";"
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD3,EPriorityNormal); goto colorize_clear; }

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
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_COMMENT1,EPriorityNormal);
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
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_COMMENT1,EPriorityNormal);
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
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ASM,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  "//"
  {
    Info.pAddColor(params,lno,yytok-line,yyend-yytok,colors+HC_COMMENT1,EPriorityNormal);
    goto colorize_end;
  }
  (L|D)*
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ASM,EPriorityNormal);
    goto colorize_asm;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_asm; }
  any
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ASM,EPriorityNormal);
    goto colorize_asm;
  }
*/
colorize_end:
    if((state[0]&0xffff)==PARSER_COMMENT1)
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_COMMENT1,EPriorityNormal);
    if((state[0]&0xffff)==PARSER_COMMENT2)
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_COMMENT2,EPriorityNormal);
  }
colorize_exit:
  PairStackClear(params->LocalHeap,&hl_state);
}
