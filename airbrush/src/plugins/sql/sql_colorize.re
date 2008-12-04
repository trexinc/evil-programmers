/*
    sql_colorize.re
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
#include "../plugins/sql/absql.h"

#ifdef UNICODE
typedef unsigned short UTCHAR;
#else
typedef unsigned char UTCHAR;
#endif

#define YYCTYPE unsigned long
#define YYCURSOR yycur
#define YYLIMIT yyend
#define YYMARKER yytmp
#define YYFILL(n)

/*!re2c
any = [\U00000001-\U0000ffff];
D = [0-9];
L = [a-zA-Z_];
L2 = [_$a-zA-Z0-9];
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
  const UTCHAR *yycur,*yyend,*yytmp,*yytok=NULL;
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
    line=(const UTCHAR*)Info.pGetLine(lno,&linelen);
    commentstart=line+startcol;
    yycur=line+startcol;
    yyend=line+linelen;
colorize_clear:
    if(yytok) if(params->callback) if(params->callback(0,lno,yytok-line,params->param)) goto colorize_exit;
    yytok=yycur;
    if(params->callback) if(params->callback(1,lno,yytok-line,params->param)) goto colorize_exit;
    if(state[0]==PARSER_COMMENT) goto colorize_comment;
    if(state[0]==PARSER_STRING) goto colorize_string;
/*!re2c
  "/*" { state[0]=PARSER_COMMENT; commentstart=yytok; goto colorize_comment; }
  [ \t]*"--"
  {
    if((yytok==line)&&lColorize) Info.pAddColor(lno,yytok-line,yyend-yytok,colors[HC_COMMENT],colors[HC_COMMENT+1]);
    goto colorize_end;
  }
  ['] { state[0]=PARSER_STRING; commentstart=yytok; goto colorize_string; }
  (["] (any\["])* ["])
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_STRING1],colors[HC_STRING1+1]); goto colorize_clear; }
  ([Aa][Cc][Tt][Ii][Oo][Nn])|([Aa][Cc][Tt][Ii][Vv][Ee])|([Aa][Dd][Dd])|([Aa][Dd][Mm][Ii][Nn])|([Aa][Ff][Tt][Ee][Rr])|
  ([Aa][Ll][Ll])|([Aa][Ll][Tt][Ee][Rr])|([Aa][Nn][Dd])|([Aa][Nn][Yy])|([Aa][Ss])|([Aa][Ss][Cc])|
  ([Aa][Ss][Cc][Ee][Nn][Dd][Ii][Nn][Gg])|([Aa][Tt])|([Aa][Uu][Tt][Oo])|([Aa][Uu][Tt][Oo][Dd][Dd][Ll])|
  ([Aa][Vv][Gg])|([Bb][Aa][Ss][Ee][Dd])|([Bb][Aa][Ss][Ee][Nn][Aa][Mm][Ee])|
  ([Bb][Aa][Ss][Ee]"_"[Nn][Aa][Mm][Ee])|([Bb][Ee][Ff][Oo][Rr][Ee])|
  ([Bb][Ee][Tt][Ww][Ee][Ee][Nn])|([Bb][Ll][Oo][Bb])|([Bb][Ll][Oo][Bb][Ee][Dd][Ii][Tt])|
  ([Bb][Rr][Ee][Aa][Kk])|([Bb][Uu][Ff][Ff][Ee][Rr])|([Bb][Yy])|([Cc][Aa][Cc][Hh][Ee])|
  ([Cc][Aa][Ss][Cc][Aa][Dd][Ee])|([Cc][Aa][Ss][Tt])|([Cc][Hh][Aa][Rr])|
  ([Cc][Hh][Aa][Rr][Aa][Cc][Tt][Ee][Rr])|([Cc][Hh][Aa][Rr][Aa][Cc][Tt][Ee][Rr]"_"[Ll][Ee][Nn][Gg][Tt][Hh])|
  ([Cc][Hh][Aa][Rr]"_"[Ll][Ee][Nn][Gg][Tt][Hh])|([Cc][Hh][Ee][Cc][Kk])|
  ([Cc][Hh][Ee][Cc][Kk]"_"[Pp][Oo][Ii][Nn][Tt]"_"[Ll][Ee][Nn])|
  ([Cc][Hh][Ee][Cc][Kk]"_"[Pp][Oo][Ii][Nn][Tt]"_"[Ll][Ee][Nn][Gg][Tt][Hh])|
  ([Cc][Oo][Ll][Ll][Aa][Tt][Ee])|([Cc][Oo][Ll][Ll][Aa][Tt][Ii][Oo][Nn])|([Cc][Oo][Ll][Uu][Mm][Nn])|
  ([Cc][Oo][Mm][Mm][Ii][Tt])|([Cc][Oo][Mm][Mm][Ii][Tt][Tt][Ee][Dd])|([Cc][Oo][Mm][Pp][Ii][Ll][Ee][Tt][Ii][Mm][Ee])|
  ([Cc][Oo][Mm][Pp][Uu][Tt][Ee][Dd])|([Cc][Ll][Oo][Ss][Ee])|([Cc][Oo][Nn][Dd][Ii][Tt][Ii][Oo][Nn][Aa][Ll])|
  ([Cc][Oo][Nn][Nn][Ee][Cc][Tt])|([Cc][Oo][Nn][Ss][Tt][Rr][Aa][Ii][Nn][Tt])|([Cc][Oo][Nn][Tt][Aa][Ii][Nn][Ii][Nn][Gg])|
  ([Cc][Oo][Nn][Tt][Ii][Nn][Uu][Ee])|([Cc][Oo][Uu][Nn][Tt])|([Cc][Rr][Ee][Aa][Tt][Ee])|([Cc][Ss][Tt][Rr][Ii][Nn][Gg])|
  ([Cc][Uu][Rr][Rr][Ee][Nn][Tt])|([Cc][Uu][Rr][Rr][Ee][Nn][Tt]"_"[Dd][Aa][Tt][Ee])|
  ([Cc][Uu][Rr][Rr][Ee][Nn][Tt]"_"[Tt][Ii][Mm][Ee])|([Cc][Uu][Rr][Rr][Ee][Nn][Tt]"_"[Tt][Ii][Mm][Ee][Ss][Tt][Aa][Mm][Pp])|
  ([Cc][Uu][Rr][Ss][Oo][Rr])|([Dd][Aa][Tt][Aa][Bb][Aa][Ss][Ee])|([Dd][Aa][Tt][Ee])|([Dd][Aa][Yy])|([Dd][Bb]"_"[Kk][Ee][Yy])|
  ([Dd][Ee][Bb][Uu][Gg])|([Dd][Ee][Cc])|([Dd][Ee][Cc][Ii][Mm][Aa][Ll])|([Dd][Ee][Cc][Ll][Aa][Rr][Ee])|
  ([Dd][Ee][Ff][Aa][Uu][Ll][Tt])|([Dd][Ee][Ll][Ee][Tt][Ee])|([Dd][Ee][Ss][Cc])|
  ([Dd][Ee][Ss][Cc][Ee][Nn][Dd][Ii][Nn][Gg])|([Dd][Ee][Ss][Cc][Rr][Ii][Bb][Ee])|
  ([Dd][Ee][Ss][Cc][Rr][Ii][Pp][Tt][Oo][Rr])|([Dd][Ii][Ss][Cc][Oo][Nn][Nn][Ee][Cc][Tt])|
  ([Dd][Ii][Ss][Pp][Ll][Aa][Yy])|([Dd][Ii][Ss][Tt][Ii][Nn][Cc][Tt])|([Dd][Oo])|([Dd][Oo][Mm][Aa][Ii][Nn])|
  ([Dd][Oo][Uu][Bb][Ll][Ee])|([Dd][Rr][Oo][Pp])|([Ee][Cc][Hh][Oo])|([Ee][Dd][Ii][Tt])|([Ee][Ll][Ss][Ee])|
  ([Ee][Nn][Tt][Rr][Yy]"_"[Pp][Oo][Ii][Nn][Tt])|([Ee][Ss][Cc][Aa][Pp][Ee])|
  ([Ee][Vv][Ee][Nn][Tt])|([Ee][Xx][Cc][Ee][Pp][Tt][Ii][Oo][Nn])|([Ee][Xx][Ee][Cc][Uu][Tt][Ee])|
  ([Ee][Xx][Ii][Ss][Tt][Ss])|([Ee][Xx][Ii][Tt])|([Ee][Xx][Tt][Ee][Rr][Nn])|([Ee][Xx][Tt][Ee][Rr][Nn][Aa][Ll])|
  ([Ee][Xx][Tt][Rr][Aa][Cc][Tt])|([Ff][Ee][Tt][Cc][Hh])|([Ff][Ii][Ll][Ee])|([Ff][Ii][Ll][Tt][Ee][Rr])|([Ff][Ii][Rr][Ss][Tt])|
  ([Ff][Ll][Oo][Aa][Tt])|([Ff][Oo][Rr])|([Ff][Oo][Rr][Ee][Ii][Gg][Nn])|([Ff][Oo][Uu][Nn][Dd])|
  ([Ff][Rr][Ee][Ee]"_"[Ii][Tt])|([Ff][Rr][Oo][Mm])|([Ff][Uu][Ll][Ll])|([Ff][Uu][Nn][Cc][Tt][Ii][Oo][Nn])|
  ([Gg][Dd][Ss][Cc][Oo][Dd][Ee])|([Gg][Ee][Nn][Ee][Rr][Aa][Tt][Oo][Rr])|
  ([Gg][Ee][Nn]"_"[Ii][Dd])|([Gg][Ll][Oo][Bb][Aa][Ll])|([Gg][Oo][Tt][Oo])|([Gg][Rr][Aa][Nn][Tt])|
  ([Gg][Rr][Oo][Uu][Pp])|([Gg][Rr][Oo][Uu][Pp]"_"[Cc][Oo][Mm][Mm][Ii][Tt]"_"[Ww][Aa][Ii][Tt])|
  ([Gg][Rr][Oo][Uu][Pp]"_"[Cc][Oo][Mm][Mm][Ii][Tt]"_")|([Ww][Aa][Ii][Tt]"_"[Tt][Ii][Mm][Ee])|
  ([Hh][Aa][Vv][Ii][Nn][Gg])|([Hh][Ee][Ll][Pp])|([Hh][Oo][Uu][Rr])|([Ii][Mm][Mm][Ee][Dd][Ii][Aa][Tt][Ee])|
  ([Ii][Nn])|([Ii][Nn][Aa][Cc][Tt][Ii][Vv][Ee])|([Ii][Nn][Dd][Ee][Xx])|([Ii][Nn][Dd][Ii][Cc][Aa][Tt][Oo][Rr])|
  ([Ii][Nn][Ii][Tt])|([Ii][Nn][Nn][Ee][Rr])|([Ii][Nn][Pp][Uu][Tt])|([Ii][Nn][Pp][Uu][Tt]"_"[Tt][Yy][Pp][Ee])|
  ([Ii][Nn][Ss][Ee][Rr][Tt])|([Ii][Nn][Tt])|([Ii][Nn][Tt][Ee][Gg][Ee][Rr])|
  ([Ii][Nn][Tt][Oo])|([Ii][Ss])|([Ii][Ss][Oo][Ll][Aa][Tt][Ii][Oo][Nn])|([Ii][Ss][Qq][Ll])|([Jj][Oo][Ii][Nn])|
  ([Kk][Ee][Yy])|([Ll][Cc]"_"[Mm][Ee][Ss][Ss][Aa][Gg][Ee][Ss])|([Ll][Cc]"_"[Tt][Yy][Pp][Ee])|([Ll][Ee][Ff][Tt])|
  ([Ll][Ee][Nn][Gg][Tt][Hh])|([Ll][Ee][Vv])|([Ll][Ee][Vv][Ee][Ll])|([Ll][Ii][Kk][Ee])|([Ll][Ii][Mm][Ii][Tt])|([Ll][Oo][Gg][Ff][Ii][Ll][Ee])|
  ([Ll][Oo][Gg]"_"[Bb][Uu][Ff][Ff][Ee][Rr]"_"[Ss][Ii][Zz][Ee])|([Ll][Oo][Gg]"_"[Bb][Uu][Ff]"_"[Ss][Ii][Zz][Ee])|
  ([Ll][Oo][Nn][Gg])|([Mm][Aa][Nn][Uu][Aa][Ll])|([Mm][Aa][Xx])|([Mm][Aa][Xx][Ii][Mm][Uu][Mm])|
  ([Mm][Aa][Xx][Ii][Mm][Uu][Mm]"_"[Ss][Ee][Gg][Mm][Ee][Nn][Tt])|([Mm][Aa][Xx]"_"[Ss][Ee][Gg][Mm][Ee][Nn][Tt])|
  ([Mm][Ee][Rr][Gg][Ee])|([Mm][Ee][Ss][Ss][Aa][Gg][Ee])|([Mm][Ii][Nn])|([Mm][Ii][Nn][Ii][Mm][Uu][Mm])|
  ([Mm][Ii][Nn][Uu][Tt][Ee])|([Mm][Oo][Dd][Uu][Ll][Ee]"_"[Nn][Aa][Mm][Ee])|([Mm][Oo][Nn][Tt][Hh])|
  ([Nn][Aa][Mm][Ee][Ss])|([Nn][Aa][Tt][Ii][Oo][Nn][Aa][Ll])|([Nn][Aa][Tt][Uu][Rr][Aa][Ll])|
  ([Nn][Cc][Hh][Aa][Rr])|([Nn][Oo])|([Nn][Oo][Aa][Uu][Tt][Oo])|([Nn][Oo][Tt])|
  ([Nn][Uu][Ll][Ll])|([Nn][Uu][Mm][Ee][Rr][Ii][Cc])|([Nn][Uu][Mm]"_"[Ll][Oo][Gg]"_"[Bb][Uu][Ff][Ss])|
  ([Nn][Uu][Mm]"_"[Ll][Oo][Gg]"_"[Bb][Uu][Ff][Ff][Ee][Rr][Ss])|([Oo][Cc][Tt][Ee][Tt]"_"[Ll][Ee][Nn][Gg][Tt][Hh])|
  ([Oo][Ff])|([Oo][Nn])|([Oo][Nn][Ll][Yy])|([Oo][Pp][Ee][Nn])|
  ([Oo][Pp][Tt][Ii][Oo][Nn])|([Oo][Rr])|([Oo][Rr][Dd][Ee][Rr])|([Oo][Uu][Tt][Ee][Rr])|
  ([Oo][Uu][Tt][Pp][Uu][Tt])|([Oo][Uu][Tt][Pp][Uu][Tt]"_"[Tt][Yy][Pp][Ee])|([Oo][Vv][Ee][Rr][Ff][Ll][Oo][Ww])|
  ([Pp][Aa][Gg][Ee])|([Pp][Aa][Gg][Ee][Ll][Ee][Nn][Gg][Tt][Hh])|([Pp][Aa][Gg][Ee][Ss])|
  ([Pp][Aa][Gg][Ee]"_"[Ss][Ii][Zz][Ee])|([Pp][Aa][Rr][Aa][Mm][Ee][Tt][Ee][Rr])|([Pp][Aa][Ss][Ss][Ww][Oo][Rr][Dd])|
  ([Pp][Ll][Aa][Nn])|([Pp][Oo][Ss][Ii][Tt][Ii][Oo][Nn])|([Pp][Oo][Ss][Tt]"_"[Ee][Vv][Ee][Nn][Tt])|
  ([Pp][Rr][Ee][Cc][Ii][Ss][Ii][Oo][Nn])|([Pp][Rr][Ee][Pp][Aa][Rr][Ee])|([Pp][Rr][Oo][Cc][Ee][Dd][Uu][Rr][Ee])|
  ([Pp][Rr][Oo][Tt][Ee][Cc][Tt][Ee][Dd])|([Pp][Rr][Ii][Mm][Aa][Rr][Yy])|([Pp][Rr][Ii][Vv][Ii][Ll][Ee][Gg][Ee][Ss])|
  ([Pp][Uu][Bb][Ll][Ii][Cc])|([Qq][Uu][Ii][Tt])|([Rr][Aa][Ww]"_"[Pp][Aa][Rr][Tt][Ii][Tt][Ii][Oo][Nn][Ss])|
  ([Rr][Dd][Bb]"$"[Dd][Bb]"_"[Kk][Ee][Yy])|([Rr][Ee][Aa][Dd])|([Rr][Ee][Aa][Ll])|
  ([Rr][Ee][Cc][Oo][Rr][Dd]"_"[Vv][Ee][Rr][Ss][Ii][Oo][Nn])|([Rr][Ee][Ff][Ee][Rr][Ee][Nn][Cc][Ee][Ss])|
  ([Rr][Ee][Ll][Ee][Aa][Ss][Ee])|([Rr][Ee][Ss][Ee][Rr][Vv])|([Rr][Ee][Ss][Ee][Rr][Vv][Ii][Nn][Gg])|
  ([Rr][Ee][Ss][Tt][Rr][Ii][Cc][Tt])|([Rr][Ee][Tt][Aa][Ii][Nn])|([Rr][Ee][Tt][Uu][Rr][Nn])|
  ([Rr][Ee][Tt][Uu][Rr][Nn][Ii][Nn][Gg]"_"[Vv][Aa][Ll][Uu][Ee][Ss])|([Rr][Ee][Tt][Uu][Rr][Nn][Ss])|([Rr][Ee][Vv][Oo][Kk][Ee])|
  ([Rr][Ii][Gg][Hh][Tt])|([Rr][Oo][Ll][Ee])|([Rr][Oo][Ll][Ll][Bb][Aa][Cc][Kk])|([Rr][Uu][Nn][Tt][Ii][Mm][Ee])|
  ([Ss][Cc][Hh][Ee][Mm][Aa])|([Ss][Ee][Cc][Oo][Nn][Dd])|([Ss][Ee][Gg][Mm][Ee][Nn][Tt])|([Ss][Ee][Ll][Ee][Cc][Tt])|
  ([Ss][Ee][Tt])|([Ss][Hh][Aa][Dd][Oo][Ww])|([Ss][Hh][Aa][Rr][Ee][Dd])|([Ss][Hh][Ee][Ll][Ll])|([Ss][Hh][Oo][Ww])|
  ([Ss][Ii][Nn][Gg][Uu][Ll][Aa][Rr])|([Ss][Ii][Zz][Ee])|([Ss][Mm][Aa][Ll][Ll][Ii][Nn][Tt])|
  ([Ss][Nn][Aa][Pp][Ss][Hh][Oo][Tt])|([Ss][Oo][Mm][Ee])|([Ss][Oo][Rr][Tt])|([Ss][Qq][Ll][Cc][Oo][Dd][Ee])|
  ([Ss][Qq][Ll][Ee][Rr][Rr][Oo][Rr])|([Ss][Qq][Ll][Ww][Aa][Rr][Nn][Ii][Nn][Gg])|
  ([Ss][Tt][Aa][Bb][Ii][Ll][Ii][Tt][Yy])|([Ss][Tt][Aa][Rr][Tt][Ii][Nn][Gg])|
  ([Ss][Tt][Aa][Rr][Tt][Ss])|([Ss][Tt][Aa][Tt][Ee][Mm][Ee][Nn][Tt])|([Ss][Tt][Aa][Tt][Ii][Cc])|
  ([Ss][Tt][Aa][Tt][Ii][Ss][Tt][Ii][Cc][Ss])|([Ss][Uu][Bb]"_"[Tt][Yy][Pp][Ee])|([Ss][Uu][Mm])|
  ([Ss][Uu][Ss][Pp][Ee][Nn][Dd])|([Tt][Aa][Bb][Ll][Ee])|([Tt][Ee][Rr][Mm][Ii][Nn][Aa][Tt][Oo][Rr])|
  ([Tt][Ii][Mm][Ee])|([Tt][Ii][Mm][Ee][Ss][Tt][Aa][Mm][Pp])|([Tt][Oo])|([Tt][Oo][Pp])|([Tt][Rr][Aa][Nn][Ss][Aa][Cc][Tt][Ii][Oo][Nn])|
  ([Tt][Rr][Aa][Nn][Ss][Ll][Aa][Tt][Ee])|([Tt][Rr][Aa][Nn][Ss][Ll][Aa][Tt][Ii][Oo][Nn])|([Tt][Rr][Ii][Gg][Gg][Ee][Rr])|
  ([Tt][Rr][Ii][Mm])|([Tt][Yy][Pp][Ee])|([Uu][Nn][Cc][Oo][Mm][Mm][Ii][Tt][Tt][Ee][Dd])|([Uu][Nn][Ii][Oo][Nn])|
  ([Uu][Nn][Ii][Qq][Uu][Ee])|([Uu][Pp][Dd][Aa][Tt][Ee])|([Uu][Pp][Pp][Ee][Rr])|([Uu][Ss][Ee][Rr])|
  ([Uu][Ss][Ii][Nn][Gg])|([Vv][Aa][Ll][Uu][Ee])|([Vv][Aa][Ll][Uu][Ee][Ss])|([Vv][Aa][Rr][Cc][Hh][Aa][Rr])|
  ([Vv][Aa][Rr][Ii][Aa][Bb][Ll][Ee])|([Vv][Aa][Rr][Yy][Ii][Nn][Gg])|([Vv][Ee][Rr][Ss][Ii][Oo][Nn])|
  ([Vv][Ii][Ee][Ww])|([Ww][Aa][Ii][Tt])|([Ww][Ee][Ee][Kk][Dd][Aa][Yy])|([Ww][Hh][Ee][Nn])|
  ([Ww][Hh][Ee][Nn][Ee][Vv][Ee][Rr])|([Ww][Hh][Ee][Rr][Ee])|([Ww][Hh][Ii][Ll][Ee])|([Ww][Ii][Tt][Hh])|
  ([Ww][Oo][Rr][Kk])|([Ww][Rr][Ii][Tt][Ee])|([Yy][Ee][Aa][Rr])|([Yy][Ee][Aa][Rr][Dd][Aa][Yy])
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_KEYWORD1],colors[HC_KEYWORD1+1]); goto colorize_clear; }
  [Bb][Ee][Gg][Ii][Nn] {PUSH_PAIR(1)}
  [Ee][Nn][Dd] {POP_PAIR(1,1)}
  [Ii][Ff] {PUSH_PAIR(2)}
  [Tt][Hh][Ee][Nn] {POP_PAIR(2,2)}
  ":"L L2*
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_KEYWORD2],colors[HC_KEYWORD2+1]); goto colorize_clear; }
  L L2*
  { goto colorize_clear; }
  D+("." D+)?
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_NUMBER1],colors[HC_NUMBER1+1]); goto colorize_clear; }
  "||"|[\-=+*/^<>,]
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_KEYWORD1],colors[HC_KEYWORD1+1]);
    goto colorize_clear;
  }
  "(" {PUSH_PAIR(0)}
  ")" {POP_PAIR(0,0)}
  ";" { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors[HC_KEYWORD3],colors[HC_KEYWORD3+1]); goto colorize_clear; }
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
  "*/"
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors[HC_COMMENT],colors[HC_COMMENT+1]);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_comment; }
  any
  { goto colorize_comment; }
*/
colorize_string:
    yytok=yycur;
/*!re2c
  [']
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
    if(state[0]==PARSER_COMMENT)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors[HC_COMMENT],colors[HC_COMMENT+1]);
    if(state[0]==PARSER_STRING)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors[HC_STRING1],colors[HC_STRING1+1]);
  }
colorize_exit:
  PairStackClear(params->LocalHeap,&hl_state);
}
