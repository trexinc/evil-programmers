/*
    rust_colorize.re
    Copyright (C) 2017 zg

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
#include "../plugins/rust/abrust.h"

#define YYCTYPE unsigned long
#define YYCURSOR yycur
#define YYLIMIT yyend
#define YYMARKER yytmp
#define YYFILL(n)

/*!re2c
any = [\U00000001-\U0000ffff];
D = [0-9];
L = [a-zA-Z_];
H = [a-fA-F0-9];
E = [Ee] [+-]? D+;
ESC1 = [\\] ([nrt\\0] | "x" D{2});
ESC2 = [\\] ['"];
ESC3 = "\\x{" H{1,6} "}";

ESCA = ESC1|ESC2|ESC3;
ESCB = ESC1|ESC2;
*/

static int GetLevel(const wchar_t* string)
{
  int result=0;
  while(*string&&*string!=L'r') ++string;
  if(*string)
  {
    while(*++string==L'#') ++result;
    if(*string!=L'"') result=0;
  }
  return result;
}

static bool MatchLevel(const wchar_t* string,int level)
{
  int count=-1;
  if(*string==L'"')
  {
    count=0;
    while(*++string==L'#') ++count;
  }
  return count==level;
}

void WINAPI Colorize(intptr_t index,struct ColorizeParams *params)
{
  (void)index;
  const wchar_t *commentstart;
  const wchar_t *line;
  intptr_t linelen;
  int startcol;
  RustState state_data={PARSER_CLEAR,0};
  RustState *state=&state_data;
  int state_size=sizeof(state_data);
  const wchar_t *yycur,*yyend,*yytmp=NULL,*yytok=NULL;
  struct PairStack *hl_state=NULL;
  intptr_t hl_row; intptr_t hl_col;
  if(params->data_size>=sizeof(state_data))
  {
    state=(RustState*)(params->data);
    state_size=params->data_size;
  }
  Info.pGetCursor(params->eid,&hl_row,&hl_col);
  INIT_PAIR;
  for(int lno=params->startline;lno<params->endline;lno++,yytok=NULL)
  {
    startcol=(lno==params->startline)?params->startcolumn:0;
    if(((lno%Info.cachestr)==0)&&(!startcol))
      if(!Info.pAddState(params->eid,lno/Info.cachestr,state_size,(unsigned char *)state)) goto colorize_exit;
    line=(const wchar_t*)Info.pGetLine(params->eid,lno,&linelen);
    commentstart=line+startcol;
    yycur=line+startcol;
    yyend=line+linelen;
colorize_clear:
    if(yytok) if(params->callback) if(params->callback(0,lno,yytok-line,params->param)) goto colorize_exit;
    yytok=yycur;
    if(params->callback) if(params->callback(1,lno,yytok-line,params->param)) goto colorize_exit;
    if(state[0].State==PARSER_COMMENT) goto colorize_comment1;
    if(state[0].State==PARSER_STRING) goto colorize_string;
    if(state[0].State==PARSER_STRING_RAW) goto colorize_string_raw;
    if(state[0].State==PARSER_STRING_BYTE) goto colorize_string_byte;
    if(state[0].State==PARSER_STRING_BYTE_RAW) goto colorize_string_byte_raw;
/*!re2c
  "/*"
  { state[0].State=PARSER_COMMENT; state[0].Level=0; commentstart=yytok; goto colorize_comment1; }
  "//"
  { commentstart=yytok; goto colorize_comment2; }
  "break"|"continue"|"fn"|"for"|"if"|"in"|"loop"|"match"|"return"|"where"|"while"
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal); goto colorize_clear; }
  L(L|D)*
  { goto colorize_clear; }
  "#" "!"? "[" (any\[\[\]])* "]"
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ATTRIBUTE,EPriorityNormal); goto colorize_clear; }
  ["]
  { PUSH_PAIR_S(PAIR_STRING); state[0].State=PARSER_STRING; commentstart=yytok; goto colorize_string; }
  "r" "#"* ["]
  { PUSH_PAIR_S(PAIR_STRING_RAW); state[0].State=PARSER_STRING_RAW; state[0].Level=GetLevel(yytok); commentstart=yytok; goto colorize_string_raw; }
  "b"["]
  { PUSH_PAIR_S(PAIR_STRING_BYTE); state[0].State=PARSER_STRING_BYTE; commentstart=yytok; goto colorize_string_byte; }
  "br" "#"* ["]
  { PUSH_PAIR_S(PAIR_STRING_BYTE_RAW); state[0].State=PARSER_STRING_BYTE_RAW; state[0].Level=GetLevel(yytok); commentstart=yytok; goto colorize_string_byte_raw; }
  "'" (ESCA|any\[\\']) "'"
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_STRING1,EPriorityNormal); goto colorize_clear; }
  "b'" (ESCB|any\[\\']) "'"
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_STRING1,EPriorityNormal); goto colorize_clear; }
  "(" {PUSH_PAIR(PAIR_BRACKETS)}
  ")" {POP_PAIR(PAIR_BRACKETS)}
  "[" {PUSH_PAIR(PAIR_SQ_BRACKETS)}
  "]" {POP_PAIR(PAIR_SQ_BRACKETS)}
  "{" {PUSH_PAIR(PAIR_BRACES)}
  "}" {POP_PAIR(PAIR_BRACES)}
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
    if(0==state[0].Level)
    {
      Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_COMMENT,EPriorityNormal);
      state[0].State=PARSER_CLEAR;
      goto colorize_clear;
    }
    --state[0].Level;
    goto colorize_comment1;
  }
  "/*"
  {
    ++state[0].Level;
    goto colorize_comment1;
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
colorize_string:
    yytok=yycur;
/*!re2c
  ESCA
  { goto colorize_string; }
  ["]
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING1,EPriorityNormal);
    state[0].State=PARSER_CLEAR;
    state[0].Level=0;
    POP_PAIR_S(PAIR_STRING);
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string; }
  any
  { goto colorize_string; }
*/
colorize_string_raw:
    yytok=yycur;
/*!re2c
  ["] "#"*
  {
    if(MatchLevel(yytok,state[0].Level))
    {
      Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING1,EPriorityNormal);
      state[0].State=PARSER_CLEAR;
      state[0].Level=0;
      POP_PAIR_S(PAIR_STRING_RAW);
      goto colorize_clear;
    }
    yycur=yytok+1;
    goto colorize_string_raw;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string_raw; }
  any
  { goto colorize_string_raw; }
*/
colorize_string_byte:
    yytok=yycur;
/*!re2c
  ESCB
  { goto colorize_string_byte; }
  ["]
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING1,EPriorityNormal);
    state[0].State=PARSER_CLEAR;
    state[0].Level=0;
    POP_PAIR_S(PAIR_STRING_BYTE);
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string_byte; }
  any
  { goto colorize_string_byte; }
*/
colorize_string_byte_raw:
    yytok=yycur;
/*!re2c
  ["] "#"*
  {
    if(MatchLevel(yytok,state[0].Level))
    {
      Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING1,EPriorityNormal);
      state[0].State=PARSER_CLEAR;
      state[0].Level=0;
      POP_PAIR_S(PAIR_STRING_BYTE_RAW);
      goto colorize_clear;
    }
    yycur=yytok+1;
    goto colorize_string_byte_raw;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string_byte_raw; }
  any
  { goto colorize_string_byte_raw; }
*/
colorize_end:
    if(state[0].State==PARSER_COMMENT)
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_COMMENT,EPriorityNormal);
    if(state[0].State==PARSER_STRING||state[0].State==PARSER_STRING_RAW||state[0].State==PARSER_STRING_BYTE||state[0].State==PARSER_STRING_BYTE_RAW)
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_STRING1,EPriorityNormal);
  }
colorize_exit:
  PairStackClear(params->LocalHeap,&hl_state);
}
