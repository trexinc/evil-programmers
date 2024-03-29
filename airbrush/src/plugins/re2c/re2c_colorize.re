/*
    re2c_colorize.re
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
#include "../plugins/re2c/abre2c.h"
#include <initguid.h>
#include "../plugins/c/guid.h"
#include "../plugins/c/abc.h"

struct CacheParam
{
  CState c_state;
  int re2c_state;
  char diff;
  int recurse;
};

struct CallbackParam
{
  int ok;
  int row;
  int col;
  ColorizeParams* params;
  CacheParam *cache;
};

static int WINAPI c_callback(int from,int row,int col,void *param)
{
  const TCHAR* line;
  intptr_t linelen;
  line=Info.pGetLine((reinterpret_cast<CallbackParam*>(param))->params->eid,row,&linelen);
  switch((reinterpret_cast<CallbackParam*>(param))->cache->diff)
  {
    case 1:
      if(from==1)
      {
        if(!_tcsncmp(line+col,_T("\057\052!re2c"),7))
        {
          Info.pAddColor((reinterpret_cast<CallbackParam*>(param))->params,row,col,7,colors+HC_RE2C,EPriorityNormal);
          (reinterpret_cast<CallbackParam*>(param))->ok=1;
          (reinterpret_cast<CallbackParam*>(param))->row=row;
          (reinterpret_cast<CallbackParam*>(param))->col=col+7;
          return true;
        }
      }
      break;
    case 2:
      if(from==0)
      {
        if(!_tcsncmp(line+col,_T("{"),1))
          ((reinterpret_cast<CallbackParam*>(param))->cache->recurse)++;
        if(!_tcsncmp(line+col,_T("}"),1))
        {
          ((reinterpret_cast<CallbackParam*>(param))->cache->recurse)--;
          if(!((reinterpret_cast<CallbackParam*>(param))->cache->recurse))
          {
            (reinterpret_cast<CallbackParam*>(param))->ok=1;
            (reinterpret_cast<CallbackParam*>(param))->row=row;
            (reinterpret_cast<CallbackParam*>(param))->col=col+1;
            return true;
          }
        }
      }
      break;
  }
  return false;
}

static void CallParser(ColorizeParams *params,CallbackParam *data)
{
  ColorizeParams c_params;
  c_params.size=sizeof(ColorizeParams);
  c_params.eid=params->eid;
  c_params.startline=data->row;
  c_params.startcolumn=data->col;
  c_params.endline=params->endline;
  c_params.topmargin=params->topmargin;
  c_params.bottommargin=params->bottommargin;
  c_params.margins=params->margins;
  c_params.data_size=params->data_size;
  c_params.data=params->data;
  c_params.LocalHeap=params->LocalHeap;
  c_params.callback=c_callback;
  c_params.param=data;
  data->ok=0;
  Info.pCallParser(&CplusplusGUID,&c_params);
  if(data->ok)
  {
    params->startline=data->row;
    params->startcolumn=data->col;
    data->cache->re2c_state=PARSER_RE2C;
    data->cache->c_state.State=INVALID_C_STATE;
  }
}

#define YYCTYPE unsigned long
#define YYCURSOR yycur
#define YYLIMIT yyend
#define YYMARKER yytmp
#define YYFILL(n)

/*!re2c
any = [\U00000001-\U0000ffff];
dot = any \ [\n];
esc = dot \ [\\];
cstring = "["  ((esc \ [\]]) | "\\" dot)* "]" ;
dstring = "\"" ((esc \ ["] ) | "\\" dot)* "\"";
istring = "'" ((esc \ ['] ) | "\\" dot)* "'";
letter = [a-zA-Z];
digit = [0-9];
*/

void WINAPI Colorize(intptr_t index,struct ColorizeParams *params)
{
  (void)index;
  const UTCHAR *commentstart;
  const UTCHAR *line;
  intptr_t linelen;
  int startcol;
  CallbackParam callback_data;
  CacheParam state_data={{PARSER_CLEAR,{},0},PARSER_RE2C,1,0};
  CacheParam *state=&state_data;
  int state_size=sizeof(state_data);
  const UTCHAR *yycur,*yyend,*yytmp,*yytok;

  if(params->data_size>=sizeof(state_data))
  {
    state=reinterpret_cast<CacheParam *>(params->data);
    state_size=params->data_size;
  }
  else
  {
    params->data=reinterpret_cast<unsigned char*>(state);
    params->data_size=state_size;
  }
  callback_data.cache=state;
  callback_data.params=params;
  if(state[0].c_state.State>INVALID_C_STATE)
  {
    callback_data.row=params->startline;
    callback_data.col=params->startcolumn;
    CallParser(params,&callback_data);
    if(!callback_data.ok) goto colorize_exit;
  }
colorize_start:
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
    yytok=yycur;
    if(state[0].re2c_state==PARSER_COMMENT) goto colorize_comment;
/*!re2c
  "{"
  {
    callback_data.row=lno;
    callback_data.col=yytok-line;
    state[0].c_state.State=PARSER_CLEAR;
    state[0].diff=2;
    state[0].recurse=0;
    CallParser(params,&callback_data);
    if(!callback_data.ok) goto colorize_exit;
    goto colorize_start;
  }
  "/*"
  { state[0].re2c_state=PARSER_COMMENT; commentstart=yytok; goto colorize_comment; }
  "*/"
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_RE2C,EPriorityNormal);
    callback_data.row=lno;
    callback_data.col=yycur-line;
    state[0].c_state.State=PARSER_CLEAR;
    state[0].diff=1;
    CallParser(params,&callback_data);
    if(!callback_data.ok) goto colorize_exit;
    goto colorize_start;
  }
  dstring
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_STRING,EPriorityNormal);
    goto colorize_clear;
  }
  cstring|istring
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_STRING,EPriorityNormal);
    goto colorize_clear;
  }
  [()|=;/\\*+?]
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD,EPriorityNormal);
    goto colorize_clear;
  }
  (letter|"@") (letter|digit)*
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_RE2C,EPriorityNormal);
    goto colorize_clear;
  }
  [ \t]+
  {
    goto colorize_clear;
  }
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
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_COMMENT,EPriorityNormal);
    state[0].re2c_state=PARSER_RE2C;
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_comment; }
  any
  { goto colorize_comment; }
*/
colorize_end:
    if(state[0].re2c_state==PARSER_COMMENT)
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_COMMENT,EPriorityNormal);
  }
colorize_exit:
  return;
}
