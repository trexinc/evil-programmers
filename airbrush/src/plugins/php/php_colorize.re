/*
    php_colorize.re
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
#include "../plugins/php/abphp.h"

typedef unsigned short UTCHAR;

#define YYCTYPE unsigned long
#define YYCURSOR yycur
#define YYLIMIT yyend
#define YYMARKER yytmp
#define YYCTXMARKER yyctxtmp
#define YYFILL(n)

/*!re2c
any = [\U00000001-\U0000ffff];
O = [0-7];
D = [0-9];
L = [a-zA-Z_];
H = [a-fA-F0-9];
E = [Ee] [+-]? D+;
ESC1 = [\\] ["\\];
ESC2 = [\\] ['\\];
comment =("//")|("#");
*/

#define CURR_STATE (state[0]&PARSER_PHP)
#define SET_CURR_STATE(new_state) (state[0]=(state[0]&(~PARSER_PHP))|new_state)

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
  for(int lno=params->startline;lno<params->endline;lno++,yytok=NULL)
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
    if(CURR_STATE==PARSER_COMMENT1) goto colorize_comment1;
    if(CURR_STATE==PARSER_COMMENT2) goto colorize_comment2;
    if(CURR_STATE==PARSER_STRING1) goto colorize_string1;
    if(CURR_STATE==PARSER_STRING2) goto colorize_string2;
/*!re2c
  "/*"
  { SET_CURR_STATE(PARSER_COMMENT1); commentstart=yytok; goto colorize_comment1; }
  comment/"?>"
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_COMMENT,EPriorityNormal); goto colorize_clear;
  }
  comment
  { SET_CURR_STATE(PARSER_COMMENT2); commentstart=yytok; goto colorize_comment2; }
  ["]
  { SET_CURR_STATE(PARSER_STRING1); commentstart=yytok; goto colorize_string1; }
  [']
  { SET_CURR_STATE(PARSER_STRING2); commentstart=yytok; goto colorize_string2; }
  ("0"[xX]H+)|("0"D+)|(D+)|(D+E)|(D*"."D+E?)|(D+"."D*E?)
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_NUMBER,EPriorityNormal); goto colorize_clear; }
  "$" [a-zA-Z_\177-\377][a-zA-Z0-9_\177-\377]*
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_VAR,EPriorityNormal); goto colorize_clear; }
  ([Tt][Rr][Uu][Ee])|([Ff][Aa][Ll][Ss][Ee])|[Nn][Uu][Ll][Ll]|"if"|"else"|"elseif"|"while"
  |"do"|"for"|"foreach"|"break"|"continue"|"switch"|"declare"|"return"|"require"|"include"
  |"require_once"|"include_once"|"function"|"class"|"extends"|"parent"|"__sleep"|"__wakeup"|"echo"
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal); goto colorize_clear; }
  "+"|"-"|"*"|"/"|"%"|"="|"&"|"|"|"^"|"~"|"<<"|">>"|"=="|"==="|"!="|"<>"|"!=="|"<"|">"|"<="|">="|"@"|"`" (any\"`")* "`"|"++"|"--"|"and"|"or"|"xor"|"!"|"&&"|"||"|"."|"$"|"->"|"::"|"("|")"|"["|"]"|"{"|"}"
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal); goto colorize_clear; }
  [ \t\v\f]+
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_COMMON,EPriorityNormal);
    goto colorize_clear;
  }
  L+
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_COMMON,EPriorityNormal);
    goto colorize_clear;
  }
  [\000]
  {
    if(yytok==yyend) goto colorize_end;
    goto colorize_clear;
  }
  any
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_COMMON,EPriorityNormal);
    goto colorize_clear;
  }
*/

colorize_comment1:
    yytok=yycur;
/*!re2c
  "*/"
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_COMMENT,EPriorityNormal);
    SET_CURR_STATE(PARSER_CLEAR);
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
  any/("?>")
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_COMMENT,EPriorityNormal);
    SET_CURR_STATE(PARSER_CLEAR);
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_comment2; }
  any
  { goto colorize_comment2; }
*/
colorize_string1:
    yytok=yycur;
/*!re2c
  ESC1
  { goto colorize_string1; }
  ["]
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING,EPriorityNormal);
    SET_CURR_STATE(PARSER_CLEAR);
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string1; }
  any
  { goto colorize_string1; }
*/
colorize_string2:
    yytok=yycur;
/*!re2c
  ESC2
  { goto colorize_string2; }
  [']
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING,EPriorityNormal);
    SET_CURR_STATE(PARSER_CLEAR);
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string2; }
  any
  { goto colorize_string2; }
*/
colorize_end:
    if(CURR_STATE==PARSER_COMMENT1||CURR_STATE==PARSER_COMMENT2)
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_COMMENT,EPriorityNormal);
    if((CURR_STATE==PARSER_STRING1)||(CURR_STATE==PARSER_STRING2))
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_STRING,EPriorityNormal);
    if(CURR_STATE==PARSER_COMMENT2) SET_CURR_STATE(PARSER_CLEAR);
  }
colorize_exit:
  PairStackClear(params->LocalHeap,&hl_state);
}
