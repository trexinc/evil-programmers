/*
    awk_colorize.re
    Copyright (C) 2009 zg

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
#include "../plugins/awk/abawk.h"

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
SIGN = [+-]?;
E = [Ee] SIGN D+;
FS  = [fFlL];
IS  = [uUlL]*;
ESC = [\\] ([abfnrtv?'"\\] | "x" H+ | O+);
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
    if(state[0]==PARSER_STRING) goto colorize_string;
    if(state[0]==PARSER_REGEXP) goto colorize_regexp;
/*!re2c
  "#"
  { commentstart=yytok; goto colorize_comment; }
  ["]
  { state[0]=PARSER_STRING; commentstart=yytok; goto colorize_string; }
  [/]
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_REGEXPS,EPriorityNormal);
    state[0]=PARSER_REGEXP;
    commentstart=yycur;
    goto colorize_regexp;
  }
  "BEGIN"|"END"
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_PATTERNS,EPriorityNormal); goto colorize_clear; }
  "ARGC"|"ARGIND"|"ARGV"|"BINMODE"|"CONVFMT"|"ENVIRON"|"ERRNO"|"FIELDWIDTHS"|"FILENAME"|"FNR"|"FS"|"IGNORECASE"|"LINT"|"NF"|"NR"|"OFMT"|"OFS"|"ORS"|"PROCINFO"|"RLENGTH"|"RS"|"RSTART"|"RT"|"SUBSEP"|"TEXTDOMAIN"
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_KEYWORD2,EPriorityNormal); goto colorize_clear; }
  "break"|"case"|"continue"|"default"|"delete"|"do"|"else"|"exit"|"for"|"if"|"in"|"switch"|"while"|
  "func"|"function"|"return"
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal); goto colorize_clear; }
  "close"|"fflush"|"getline"|"next"|"nextfile"|"prev"|"print"|"system"|"printf"|
  "atan2"|"cos"|"exp"|"int"|"log"|"rand"|"sin"|"sqrt"|"srand"|
  "asort"|"asorti"|"gsub"|"gensub"|"index"|"length"|"match"|"split"|"sprintf"|"strtonum"|"sub"|"substr"|"tolower"|"toupper"|
  "mktime"|"strftime"|"systime"|
  "and"|"compl"|"lshift"|"or"|"rshift"|"xor"|
  "bindtextdomain"|"dcgettext"|"dcngettext"
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_BUILTIN,EPriorityNormal); goto colorize_clear; }
  (SIGN "0"[xX]H+)|(SIGN "0"O+)|(SIGN D+)|(SIGN D+E)|(SIGN D*"."D+E?)|(SIGN D+"."D*E?)
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_NUMBER,EPriorityNormal); goto colorize_clear; }
  (SIGN "0"[xX]H*[^a-fA-F0-9 \t\v\f\000()\[\]{};=+-/*&|^,.:!~><%]+)
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_HL_ERROR,EPriorityNormal); goto colorize_clear; }
  "$" (L|D)+
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_PARAM,EPriorityNormal); goto colorize_clear; }
  "$" (L|D)* [^0-9a-zA-Z_ \t\v\f\000()\[\]{};=+-/*&|^,.:!~><%]*
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_HL_ERROR,EPriorityNormal); goto colorize_clear; }
  L(L|D)*
  { goto colorize_clear; }
  ";"|"="|"+"|"-"|"/"|"*"|"&"|"|"|"^"|","|"."|":"|"!"|"~"|">"|"<"|"%"
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_SYMBOL,EPriorityNormal); goto colorize_clear; }
  "\"" ( "/dev/" ("stdin"|"stdout"|"stderr"|"fd/" D|"pid"|"ppid"|"pgrpid"|"user") | "/inet/" ("tcp"|"udp"|"raw") "/" (D|L)+ "/" [0-9a-zA-Z._-]+ "/" (D|L)+ ) "\""
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_IO,EPriorityNormal); goto colorize_clear; }
  "(" {PUSH_PAIR(0)}
  ")" {POP_PAIR(0,0)}
  "[" {PUSH_PAIR(1)}
  "]" {POP_PAIR(1,1)}
  "{" {PUSH_PAIR(2)}
  "}" {POP_PAIR(2,2)}
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

colorize_comment:
    yytok=yycur;
/*!re2c
  [\000]
  {
    if(yytok==yyend)
    {
      if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_COMMENT,EPriorityNormal);
      goto colorize_end;
    }
    goto colorize_comment;
  }
  any
  { goto colorize_comment; }
*/
colorize_string:
    yytok=yycur;
/*!re2c
  ESC
  { goto colorize_string; }
  ["]
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_STRING,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string; }
  any
  { goto colorize_string; }
*/
colorize_regexp:
    yytok=yycur;
/*!re2c
  ESC
  { goto colorize_regexp; }
  [/]
  {
    if(lColorize)
    {
      Info.pAddColor(lno,commentstart-line,yytok-commentstart,colors+HC_REGEXP,EPriorityNormal);
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_REGEXPS,EPriorityNormal);
    }
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_regexp; }
  any
  { goto colorize_regexp; }
*/
colorize_end:
    if(state[0]==PARSER_STRING)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors+HC_STRING,EPriorityNormal);
    if(state[0]==PARSER_REGEXP)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors+HC_REGEXP,EPriorityNormal);
  }
colorize_exit:
  PairStackClear(params->LocalHeap,&hl_state);
}
