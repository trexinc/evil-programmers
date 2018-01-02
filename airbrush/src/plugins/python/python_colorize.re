/*
    python_colorize.re
    Copyright (C) 2018 zg

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
#include "../plugins/python/abpython.h"

typedef unsigned short UTCHAR;

#define YYCTYPE unsigned long
#define YYCURSOR yycur
#define YYLIMIT yyend
#define YYMARKER yytmp
#define YYFILL(n)

/*!stags:re2c format='const UTCHAR *@@;';*/
/*!re2c
any=[\U00000001-\U0000ffff];
ESC=[\\] any;
STR='u'|'f'|'b';
STRRAW='r'|'fr'|'rf'|'br'|'rb';
LONGSS=['][']['];
LONGSD=["]["]["];
D=[0-9];
L=[a-zA-Z_];
*/

/*!re2c
nonzerodigit=[1-9];
digit=[0-9];
bindigit=[01];
octdigit=[0-7];
hexdigit=digit|[a-f]|[A-F];
decinteger=nonzerodigit("_"?digit)*|"0"+("_"? "0")*;
bininteger="0"'b'("_"?bindigit)+;
octinteger="0"'o'("_"?octdigit)+;
hexinteger="0"'x'("_"?hexdigit)+;
integer=decinteger|bininteger|octinteger|hexinteger;

digitpart=digit("_"? digit)*;
fraction="."digitpart;
exponent='e'("+"|"-")?digitpart;
pointfloat=digitpart?fraction|digitpart".";
exponentfloat=(digitpart|pointfloat)exponent;
floatnumber=pointfloat|exponentfloat;

imagnumber=(floatnumber|digitpart)'j';
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
  const UTCHAR *yycur,*yyend,*yytmp=NULL,*yytok=NULL,*tagstart,*tagfinish;
  struct PairStack *hl_state=NULL;
  intptr_t hl_row; intptr_t hl_col;
  if(params->data_size>=sizeof(state_data))
  {
    state=(int *)(params->data);
    state_size=params->data_size;
  }
  Info.pGetCursor(params->eid,&hl_row,&hl_col);
  INIT_PAIR;
  for(int lno=params->startline;lno<params->endline;lno++,yytok=NULL)
  {
    startcol=(lno==params->startline)?params->startcolumn:0;
    if(((lno%Info.cachestr)==0)&&(!startcol))
      if(!Info.pAddState(params->eid,lno/Info.cachestr,state_size,(unsigned char *)state)) goto colorize_exit;
    line=(const UTCHAR*)Info.pGetLine(params->eid,lno,&linelen);
    commentstart=line+startcol;
    yycur=line+startcol;
    yyend=line+linelen;
colorize_clear:
    if(yytok) if(params->callback) if(params->callback(0,lno,yytok-line,params->param)) goto colorize_exit;
    yytok=yycur;
    if(params->callback) if(params->callback(1,lno,yytok-line,params->param)) goto colorize_exit;
    if(state[0]==PARSER_STRING_LONG_DOUBLE) goto colorize_string_long_double;
    if(state[0]==PARSER_STRING_LONG_DOUBLE_RAW) goto colorize_string_long_double_raw;
    if(state[0]==PARSER_STRING_LONG_SINGLE) goto colorize_string_long_single;
    if(state[0]==PARSER_STRING_LONG_SINGLE_RAW) goto colorize_string_long_single_raw;
/*!re2c
  "#"
  {
    Info.pAddColor(params,lno,yytok-line,yyend-yytok,colors+HC_COMMENT,EPriorityNormal);
    goto colorize_end;
  }
  integer|floatnumber|imagnumber
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_NUMBER1,EPriorityNormal); goto colorize_clear; }
  "False"|"class"|"finally"|"is"|"return"|
  "None"|"continue"|"for"|"lambda"|"try"|
  "True"|"def"|"from"|"nonlocal"|"while"|
  "and"|"del"|"global"|"not"|"with"|
  "as"|"elif"|"if"|"or"|"yield"|
  "assert"|"else"|"import"|"pass"|
  "break"|"except"|"in"|"raise"|
  [+-*/%@<>=!,:.]
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal); goto colorize_clear; }
  L(L|D)*
  { goto colorize_clear; }
  STR? ["] @tagstart (ESC|(any\["]))* @tagfinish ["]
  {
    PUSH_PAIR_0(PAIR_STRING_SHORT_DOUBLE,HC_STRING1,true,yytok,tagstart);
    Info.pAddColor(params,lno,tagstart-line,tagfinish-tagstart,colors+HC_STRING1,EPriorityNormal);
    POP_PAIR_00(PAIR_STRING_SHORT_DOUBLE,HC_STRING1,true,tagfinish,yycur);
    goto colorize_clear;
  }
  STR? ['] @tagstart (ESC|(any\[']))* @tagfinish [']
  {
    PUSH_PAIR_0(PAIR_STRING_SHORT_SINGLE,HC_STRING1,true,yytok,tagstart);
    Info.pAddColor(params,lno,tagstart-line,tagfinish-tagstart,colors+HC_STRING1,EPriorityNormal);
    POP_PAIR_00(PAIR_STRING_SHORT_SINGLE,HC_STRING1,true,tagfinish,yycur);
    goto colorize_clear;
  }
  STRRAW ["] @tagstart (any\["])* @tagfinish ["]
  {
    PUSH_PAIR_0(PAIR_STRING_SHORT_DOUBLE_RAW,HC_STRING1,true,yytok,tagstart);
    Info.pAddColor(params,lno,tagstart-line,tagfinish-tagstart,colors+HC_STRING1,EPriorityNormal);
    POP_PAIR_00(PAIR_STRING_SHORT_DOUBLE_RAW,HC_STRING1,true,tagfinish,yycur);
    goto colorize_clear;
  }
  STRRAW ['] @tagstart (ESC|(any\[']))* @tagfinish [']
  {
    PUSH_PAIR_0(PAIR_STRING_SHORT_SINGLE_RAW,HC_STRING1,true,yytok,tagstart);
    Info.pAddColor(params,lno,tagstart-line,tagfinish-tagstart,colors+HC_STRING1,EPriorityNormal);
    POP_PAIR_00(PAIR_STRING_SHORT_SINGLE_RAW,HC_STRING1,true,tagfinish,yycur);
    goto colorize_clear;
  }
  STR? LONGSD
  { PUSH_PAIR_S(PAIR_STRING_LONG_DOUBLE); state[0]=PARSER_STRING_LONG_DOUBLE; commentstart=yytok; goto colorize_string_long_double; }
  STRRAW LONGSD
  { PUSH_PAIR_S(PAIR_STRING_LONG_DOUBLE_RAW); state[0]=PARSER_STRING_LONG_DOUBLE_RAW; commentstart=yytok; goto colorize_string_long_double_raw; }
  STR? LONGSS
  { PUSH_PAIR_S(PAIR_STRING_LONG_SINGLE); state[0]=PARSER_STRING_LONG_SINGLE; commentstart=yytok; goto colorize_string_long_single; }
  STRRAW LONGSS
  { PUSH_PAIR_S(PAIR_STRING_LONG_SINGLE_RAW); state[0]=PARSER_STRING_LONG_SINGLE_RAW; commentstart=yytok; goto colorize_string_long_single_raw; }
  "(" {PUSH_PAIR(PAIR_BRACKETS)}
  ")" {POP_PAIR(PAIR_BRACKETS)}
  "[" {PUSH_PAIR(PAIR_SQ_BRACKETS)}
  "]" {POP_PAIR(PAIR_SQ_BRACKETS)}
  "{" {PUSH_PAIR(PAIR_BRACES)}
  "}" {POP_PAIR(PAIR_BRACES)}
  ";" { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD3,EPriorityNormal); goto colorize_clear; }
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

colorize_string_long_double:
    yytok=yycur;
/*!re2c
  ESC
  { goto colorize_string_long_double; }
  LONGSD
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING1,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    POP_PAIR_S(PAIR_STRING_LONG_DOUBLE);
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string_long_double; }
  any
  { goto colorize_string_long_double; }
*/
colorize_string_long_double_raw:
    yytok=yycur;
/*!re2c
  LONGSD
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING1,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    POP_PAIR_S(PAIR_STRING_LONG_DOUBLE_RAW);
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string_long_double_raw; }
  any
  { goto colorize_string_long_double_raw; }
*/
colorize_string_long_single:
    yytok=yycur;
/*!re2c
  ESC
  { goto colorize_string_long_single; }
  LONGSS
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING1,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    POP_PAIR_S(PAIR_STRING_LONG_SINGLE);
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string_long_single; }
  any
  { goto colorize_string_long_single; }
*/
colorize_string_long_single_raw:
    yytok=yycur;
/*!re2c
  LONGSS
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING1,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    POP_PAIR_S(PAIR_STRING_LONG_SINGLE_RAW);
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string_long_single_raw; }
  any
  { goto colorize_string_long_single_raw; }
*/
colorize_end:
    switch(state[0])
    {
      case PARSER_STRING_LONG_SINGLE:
      case PARSER_STRING_LONG_DOUBLE:
      case PARSER_STRING_LONG_SINGLE_RAW:
      case PARSER_STRING_LONG_DOUBLE_RAW:
        Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_STRING1,EPriorityNormal);
        break;
      default:
        break;
    }
  }
colorize_exit:
  PairStackClear(params->LocalHeap,&hl_state);
}
