/*
    armgnuasm_colorize.re
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
#include "../plugins/armgnuasm/abarmgnuasm.h"

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
SIGN = [+-]?;
ESC = [\\] ([abfnrtv?'"\\] | "x" H+ | O+);
ARMCONDTHUMB = 'eq'|'ne'|'cs'|'hs'|'cc'|'lo'|'mi'|'pl'|'vs'|'vc'|'hi'|'ls'|'ge'|'lt'|'gt'|'le';
ARMCOND = ARMCONDTHUMB|'al';
THUMBREGSLO = 'r0'|'r1'|'r2'|'r3'|'r4'|'r5'|'r6'|'r7';
THUMBREGSHI = 'r8'|'r9'|'r10'|'r11'|'r12'|'r13'|'r14'|'r15'|'fp'|'ip'|'sp'|'lr'|'pc';
ARMREGSCOMMON = THUMBREGSLO|THUMBREGSHI;
ARMREGS = ARMREGSCOMMON|'cpsr' ('_'[cfsx])?;
ARMMEMORY = 'ia'|'ib'|'da'|'db'|'fd'|'ed'|'fa'|'ea';
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
  const UTCHAR *yycur,*yyend,*yytmp=NULL,*yyctxtmp,*yytok=NULL;
  struct PairStack *hl_state=NULL;
  intptr_t hl_row; intptr_t hl_col;
  if(params->data_size>=sizeof(state_data))
  {
    state=(int *)(params->data);
    state_size=params->data_size;
  }
  Info.pGetCursor(params->eid,&hl_row,&hl_col);
  Info.pSetBracket(params->eid,-1,-1);
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
    if(state[0]==PARSER_COMMENT1||state[0]==PARSER_COMMENT3) goto colorize_comment1;
    if(state[0]==PARSER_STRING1||state[0]==PARSER_STRING2) goto colorize_string1;
    if(state[0]==PARSER_THUMB) goto colorize_thumb;
/*!re2c
  '.thumb' | '.thumb_func' | '.code 16'
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_DEFINE,EPriorityNormal);
    state[0]=PARSER_THUMB;
    goto colorize_thumb;
  }
  "." 'h'? 'word' [ \t]+ SIGN (("0"[xX]H+)|(D+))
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_NUMBER1,EPriorityNormal); goto colorize_clear; }
  '.code 32' | "." (L|D)+
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_DEFINE,EPriorityNormal); goto colorize_clear; }
  "/*"
  { state[0]=PARSER_COMMENT1; commentstart=yytok; goto colorize_comment1; }
  "@"
  { commentstart=yytok; goto colorize_comment2; }
  ('add'|'sub'|'sbc'|'rsb'|'rsc'|'mul'|'mla'|'umull'|'umlal'|'smull'|'smlal') (ARMCOND)? 's'? | ('cmp'|'cmn') (ARMCOND)?
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ARM_ARITHMETIC,EPriorityNormal); goto colorize_clear; }
  'q' ('d')? ('add'|'sub') (ARMCOND)? | ('smul'|'smla') [wWtTbB] [tTbB] (ARMCOND) | 'smlal' [tTbB] [tTbB] (ARMCOND) |
  'clz' (ARMCOND)?
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ARM_ARITHMETIC5,EPriorityNormal); goto colorize_clear; }
  ('tst'|'teq') (ARMCOND)? | ('and'|'eor'|'orr'|'bic') (ARMCOND)? 's'?
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ARM_LOGICAL,EPriorityNormal); goto colorize_clear; }
  ('mov'|'mvn') (ARMCOND)? 's'? | ('mrs'|'msr') (ARMCOND)?
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ARM_MOVE,EPriorityNormal); goto colorize_clear; }
  ARMREGSCOMMON [ \t]* "," [ \t]* ('lsl'|'lsr'|'asr'|'ror') [ \t]* '#' (D)+ | ARMREGSCOMMON [ \t]* "," [ \t]* 'rrx' | ARMREGSCOMMON [ \t]* "," [ \t]* ('lsl'|'lsr'|'asr'|'ror') [ \t]+ ARMREGSCOMMON
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ARM_SHIFT,EPriorityNormal); goto colorize_clear; }
  ('b'|'bl'|'bx') (ARMCOND)? | 'swi' (ARMCOND)? [ \t]+ (('0x'H+)|(D+))
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ARM_BRANCH,EPriorityNormal); goto colorize_clear; }
  'blx'/[ \t]+ (L|[.])(L|D)*
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ARM_BRANCH5,EPriorityNormal); goto colorize_clear; }
  'blx' (ARMCOND)? / [ \t]+ ARMREGSCOMMON
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ARM_BRANCH5,EPriorityNormal); goto colorize_clear; }
  ('ldr'|'str') (ARMCOND)? ('t'|'b'|'bt'|'sb'|'h'|'sh')? | ('ldm'|'stm') (ARMCOND)? (ARMMEMORY) | 'swp' (ARMCOND)? 'b'?
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ARM_MEMORY,EPriorityNormal); goto colorize_clear; }
  'pld' | ('ldr'|'str') (ARMCOND)? 'd'
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ARM_MEMORY5,EPriorityNormal); goto colorize_clear; }
  'adr' 'l'? (ARMCOND)? | 'movl' (ARMCOND)? | ('asr'|'lsl'|'lsr'|'ror'|'rrx') (ARMCOND)? 's'? | 'push' (ARMCOND)? | 'pop' (ARMCOND)? | 'nop'
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ARM_PSEUDO,EPriorityNormal); goto colorize_clear; }
  ARMREGS
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ARM_REGS,EPriorityNormal); goto colorize_clear; }
  L(L|D)*
  { goto colorize_clear; }
  "#" SIGN (("0"[xX]H+)|(D+))
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_NUMBER1,EPriorityNormal); goto colorize_clear; }
  "-"|"+"|","|"!"
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal);
    goto colorize_clear;
  }
  "[" {PUSH_PAIR(1)}
  "]" {POP_PAIR(1,1)}
  "{" {PUSH_PAIR(2)}
  "}" {POP_PAIR(2,2)}
  ["]
  { state[0]=PARSER_STRING1; commentstart=yytok; goto colorize_string1; }
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
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_COMMENT,EPriorityNormal);
    state[0]=state[0]-PARSER_COMMENT1;
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
colorize_string1:
    yytok=yycur;
/*!re2c
  ESC
  { goto colorize_string1; }
  ["]
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING1,EPriorityNormal);
    state[0]=state[0]-PARSER_STRING1;
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string1; }
  any
  { goto colorize_string1; }
*/
colorize_thumb:
    yytok=yycur;
/*!re2c
  '.arm' | '.code 32'
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_DEFINE,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  "." 'h'? 'word' [ \t]+ SIGN (("0"[xX]H+)|(D+))
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_NUMBER1,EPriorityNormal); goto colorize_clear; }
  '.code 16' | "." (L|D)+
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_DEFINE,EPriorityNormal); goto colorize_clear; }
  "/*"
  { state[0]=PARSER_COMMENT3; commentstart=yytok; goto colorize_comment1; }
  "@"
  { commentstart=yytok; goto colorize_comment2; }
  'add' | 'adc' | 'sub' | 'sbc' | 'neg' | 'mul' | 'cmp' | 'cmn'
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_THUMB_ARITHMETIC,EPriorityNormal); goto colorize_clear; }
  'and' | 'eor' | 'orr' | 'bic' | 'tst'
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_THUMB_LOGICAL,EPriorityNormal); goto colorize_clear; }
  'mov' | 'mvn'
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_THUMB_MOVE,EPriorityNormal); goto colorize_clear; }
  'lsl' | 'lsr' | 'asr' | 'ror'
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_THUMB_SHIFT,EPriorityNormal); goto colorize_clear; }
  'b' (ARMCONDTHUMB)? | 'bl' | 'bx' | 'swi' [ \t]+ (('0x'H+)|(D+))
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_THUMB_BRANCH,EPriorityNormal); goto colorize_clear; }
  'blx'
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_THUMB_BRANCH5,EPriorityNormal); goto colorize_clear; }
  ('ldr'|'str') ('b'|'h'|'sh'|'sb')? | 'ldmia' | 'stmia' | 'push' | 'pop'
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_THUMB_MEMORY,EPriorityNormal); goto colorize_clear; }
  'adr' 'l'? | 'movl' | 'nop'
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_THUMB_PSEUDO,EPriorityNormal); goto colorize_clear; }
  THUMBREGSLO
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_THUMB_REGS_LO,EPriorityNormal); goto colorize_clear; }
  THUMBREGSHI
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_THUMB_REGS_HI,EPriorityNormal); goto colorize_clear; }
  L(L|D)*
  { goto colorize_clear; }
  "#" SIGN (("0"[xX]H+)|(D+))
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_NUMBER1,EPriorityNormal); goto colorize_clear; }
  "-"|"+"|","
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal);
    goto colorize_clear;
  }
  "[" {PUSH_PAIR(1)}
  "]" {POP_PAIR(1,1)}
  "{" {PUSH_PAIR(2)}
  "}" {POP_PAIR(2,2)}
  ["]
  { state[0]=PARSER_STRING2; commentstart=yytok; goto colorize_string1; }
  [ \t\v\f]+ { goto colorize_clear; }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_clear; }
  any
  { goto colorize_clear; }
*/
colorize_end:
    if(state[0]==PARSER_COMMENT1||state[0]==PARSER_COMMENT3)
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_COMMENT,EPriorityNormal);
    if(state[0]==PARSER_STRING1||state[0]==PARSER_STRING2)
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_STRING1,EPriorityNormal);
  }
colorize_exit:
  PairStackClear(params->LocalHeap,&hl_state);
}
