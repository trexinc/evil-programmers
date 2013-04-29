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

typedef unsigned short UTCHAR;

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
    if(lColorize&&(!startcol)) Info.pDelColor(lno);
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
    if((yytok==line)&&lColorize) Info.pAddColor(lno,yytok-line,yyend-yytok,colors+HC_COMMENT,EPriorityNormal);
    goto colorize_end;
  }
  ['] { state[0]=PARSER_STRING; commentstart=yytok; goto colorize_string; }
  (["] (any\["])* ["])
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_STRING1,EPriorityNormal); goto colorize_clear; }
  ('action')|('active')|('add')|('admin')|('after')|
  ('all')|('alter')|('and')|('any')|('as')|('asc')|
  ('ascending')|('at')|('auto')|('autoddl')|
  ('avg')|('based')|('basename')|
  ('base_name')|('before')|
  ('between')|('blob')|('blobedit')|
  ('break')|('buffer')|('by')|('cache')|
  ('cascade')|('cast')|('char')|
  ('character')|('character_length')|
  ('char_length')|('check')|
  ('check_point_len')|
  ('check_point_length')|
  ('collate')|('collation')|('column')|
  ('commit')|('committed')|('compiletime')|
  ('computed')|('close')|('conditional')|
  ('connect')|('constraint')|('containing')|
  ('continue')|('count')|('create')|('cstring')|
  ('current')|('current_date')|
  ('current_time')|('current_timestamp')|
  ('cursor')|('database')|('date')|('day')|('db_key')|
  ('debug')|('dec')|('decimal')|('declare')|
  ('default')|('delete')|('desc')|
  ('descending')|('describe')|
  ('descriptor')|('disconnect')|
  ('display')|('distinct')|('do')|('domain')|
  ('double')|('drop')|('echo')|('edit')|('else')|
  ('entry_point')|('escape')|
  ('event')|('exception')|('execute')|
  ('exists')|('exit')|('extern')|('external')|
  ('extract')|('fetch')|('file')|('filter')|('first')|
  ('float')|('for')|('foreign')|('found')|
  ('free_it')|('from')|('full')|('function')|
  ('gdscode')|('generator')|
  ('gen_id')|('global')|('goto')|('grant')|
  ('group')|('group_commit_wait')|
  ('group_commit_')|('wait_time')|
  ('having')|('help')|('hour')|('immediate')|
  ('in')|('inactive')|('index')|('indicator')|
  ('init')|('inner')|('input')|('input_type')|
  ('insert')|('int')|('integer')|
  ('into')|('is')|('isolation')|('isql')|('join')|
  ('key')|('lc_messages')|('lc_type')|('left')|
  ('length')|('lev')|('level')|('like')|('limit')|('logfile')|
  ('log_buffer_size')|('log_buf_size')|
  ('long')|('manual')|('max')|('maximum')|
  ('maximum_segment')|('max_segment')|
  ('merge')|('message')|('min')|('minimum')|
  ('minute')|('module_name')|('month')|
  ('names')|('national')|('natural')|
  ('nchar')|('no')|('noauto')|('not')|
  ('null')|('numeric')|('num_log_bufs')|
  ('num_log_buffers')|('octet_length')|
  ('of')|('on')|('only')|('open')|
  ('option')|('or')|('order')|('outer')|
  ('output')|('output_type')|('overflow')|
  ('page')|('pagelength')|('pages')|
  ('page_size')|('parameter')|('password')|
  ('plan')|('position')|('post_event')|
  ('precision')|('prepare')|('procedure')|
  ('protected')|('primary')|('privileges')|
  ('public')|('quit')|('raw_partitions')|
  ('rdb$db_key')|('read')|('real')|
  ('record_version')|('references')|
  ('release')|('reserv')|('reserving')|
  ('restrict')|('retain')|('return')|
  ('returning_values')|('returns')|('revoke')|
  ('right')|('role')|('rollback')|('runtime')|
  ('schema')|('second')|('segment')|('select')|
  ('set')|('shadow')|('shared')|('shell')|('show')|
  ('singular')|('size')|('smallint')|
  ('snapshot')|('some')|('sort')|('sqlcode')|
  ('sqlerror')|('sqlwarning')|
  ('stability')|('starting')|
  ('starts')|('statement')|('static')|
  ('statistics')|('sub_type')|('sum')|
  ('suspend')|('table')|('terminator')|
  ('time')|('timestamp')|('to')|('top')|('transaction')|
  ('translate')|('translation')|('trigger')|
  ('trim')|('type')|('uncommitted')|('union')|
  ('unique')|('update')|('upper')|('user')|('use')|
  ('using')|('value')|('values')|('varchar')|
  ('variable')|('varying')|('version')|
  ('view')|('wait')|('weekday')|('when')|
  ('whenever')|('where')|('while')|('with')|
  ('work')|('write')|('year')|('yearday')
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal); goto colorize_clear; }
  'begin' {PUSH_PAIR(1)}
  'end' {POP_PAIR(1,1)}
  'if' {PUSH_PAIR(2)}
  'then' {POP_PAIR(2,2)}
  ":"L L2*
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_KEYWORD2,EPriorityNormal); goto colorize_clear; }
  L L2*
  { goto colorize_clear; }
  D+("." D+)?
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_NUMBER1,EPriorityNormal); goto colorize_clear; }
  "||"|[\-=+*/^<>,]
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal);
    goto colorize_clear;
  }
  "(" {PUSH_PAIR(0)}
  ")" {POP_PAIR(0,0)}
  ";" { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_KEYWORD3,EPriorityNormal); goto colorize_clear; }
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
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_COMMENT,EPriorityNormal);
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
    if(state[0]==PARSER_COMMENT)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors+HC_COMMENT,EPriorityNormal);
    if(state[0]==PARSER_STRING)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors+HC_STRING1,EPriorityNormal);
  }
colorize_exit:
  PairStackClear(params->LocalHeap,&hl_state);
}
