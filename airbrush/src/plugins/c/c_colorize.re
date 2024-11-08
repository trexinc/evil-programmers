/*
    c_colorize.re
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
#include "../plugins/c/abc.h"

#define YYCTYPE unsigned long
#define YYCURSOR yycur
#define YYLIMIT yyend
#define YYMARKER yytmp
#define YYFILL(n)

/*!re2c
re2c:tags=1;
any=[\U00000001-\U0000ffff];
B=[0-1];
O=[0-7];
D=[0-9];
L=[a-zA-Z_];
H=[a-fA-F0-9];
SEP="'";
DD=D(SEP*);
EXP=[eE][+-]?(SEP*)DD+;
FLOAT=[fFlL];
INT=[uUlL]*;
ESC=[\\] ([abfnrtv?'"\\] | "x" H+ | O+ | "u" H H H H );
STR=("L"|"u8"|"u"|"U");
DCHAR=[!"#$%&'()*+,.0-9:;<=>?@A-Z_`a-z{|}~]|"-"|"["|"]"|"\\"|"^";
*/

void WINAPI Colorize(intptr_t index,struct ColorizeParams *params)
{
  (void)index;
  const UTCHAR *commentstart;
  const UTCHAR *line;
  intptr_t linelen;
  int startcol;
  CState state_data={PARSER_CLEAR,{},0};
  CState *state=&state_data;
  int state_size=sizeof(state_data);
  const UTCHAR *yycur,*yyend,*yytmp=NULL,*yytok=NULL;
  struct PairStack *hl_state=NULL;
  intptr_t hl_row; intptr_t hl_col;
  const UTCHAR *t1, *t2;
  /*!stags:re2c format = 'const UTCHAR *@@;\n'; */
  if(params->data_size>=sizeof(state_data))
  {
    state=reinterpret_cast<CState*>(params->data);
    state_size=params->data_size;
  }
  Info.pGetCursor(params->eid,&hl_row,&hl_col);
  INIT_PAIR;
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
    if(state[0].State==PARSER_COMMENT1) goto colorize_comment1;
    if(state[0].State==PARSER_STRING) goto colorize_string;
    if(state[0].State==PARSER_RAWSTRING) goto colorize_rawstring;
/*!re2c
  "/*"
  { state[0].State=PARSER_COMMENT1; commentstart=yytok; goto colorize_comment1; }
  "//"
  { commentstart=yytok; goto colorize_comment2; }
  "alignas"|"alignof"|"and"|"and_eq"|"asm"|"auto"|
  "bitand"|"bitor"|"bool"|"break"|
  "case"|"catch"|"char"|"char16_t"|"char32_t"|"class"|"compl"|"const"|"constexpr"|"const_cast"|"continue"|
  "decltype"|"default"|"delete"|"do"|"double"|"dynamic_cast"|
  "else"|"enum"|"explicit"|"export"|"extern"|
  "false"|"float"|"for"|"friend"|
  "goto"|"if"|"inline"|"int"|"long"|"mutable"|
  "namespace"|"new"|"noexcept"|"not"|"not_eq"|"nullptr"|
  "operator"|"or"|"or_eq"|"private"|"protected"|"public"|
  "register"|"reinterpret_cast"|"return"|
  "short"|"signed"|"sizeof"|"static"|"static_assert"|"static_cast"|"struct"|"switch"|
  "template"|"this"|"thread_local"|"throw"|"true"|"try"|"typedef"|"typeid"|"typename"|
  "union"|"unsigned"|"using"|
  "virtual"|"void"|"volatile"|
  "wchar_t"|"while"|"xor"|"xor_eq"|
  "override"|"final"|
  "__int64"|"size_t"
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal); goto colorize_clear; }
  L(L|D)*
  { goto colorize_clear; }
  ((DD+|("0x" SEP*(H("_"*))+)|("0" SEP*(O(SEP*))+)|("0b" SEP*(B(SEP*))+)) INT?)|((DD+(("." DD*EXP?)|EXP)) FLOAT?)|(DD+ FLOAT)
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_NUMBER1,EPriorityNormal); goto colorize_clear; }
  STR?["]
  { PUSH_PAIR_S(3); state[0].State=PARSER_STRING; commentstart=yytok; goto colorize_string; }
  STR?"R"["] @t1 DCHAR{0,16} @t2 "("
  { PUSH_PAIR_S(4); state[0].State=PARSER_RAWSTRING; state[0].Len=t2-t1; memcpy(state[0].Raw,t1,state[0].Len*sizeof(UTCHAR)); commentstart=yytok; goto colorize_rawstring; }
  (STR?['] (ESC|any\[\\'])* ['])
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_STRING1,EPriorityNormal); goto colorize_clear; }
  "..."|">>="|"<<="|"+="|"-="|"*="|"/="|"%="|"&="|"^="|"|="|">>"|"<<"|"++"|"--"|"->"|"&&"|"||"|
  "<="|">="|"=="|"!="|","|":"|"="|"."|"&"|"!"|"~"|
  "-"|"+"|"*"|"/"|"%"|"<"|">"|"^"|"|"|"?"
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD1,EPriorityNormal);
    goto colorize_clear;
  }
  "(" {PUSH_PAIR(0)}
  ")" {POP_PAIR(0)}
  "[" {PUSH_PAIR(1)}
  "]" {POP_PAIR(1)}
  "{" {PUSH_PAIR(2)}
  "}" {POP_PAIR(2)}
  ";" { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_KEYWORD3,EPriorityNormal); goto colorize_clear; }
  [ \t]*"#"[ \t]*[a-zA-Z]+
  {
    if(yytok==line)
      Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_DEFINE,EPriorityNormal); goto colorize_clear;
  }
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
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_COMMENT1,EPriorityNormal);
    state[0].State=PARSER_CLEAR;
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
      Info.pAddColor(params,lno,commentstart-line,yycur-commentstart-1,colors+HC_COMMENT1,EPriorityNormal);
      goto colorize_end;
    }
    goto colorize_comment2;
  }
  [Ff][Ii][Xx][Mm][Ee]
  {
    Info.pAddColor(params,lno,commentstart-line,yytok-commentstart,colors+HC_COMMENT1,EPriorityNormal);
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
  ESC
  { goto colorize_string; }
  ["]
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING1,EPriorityNormal);
    state[0].State=PARSER_CLEAR;
    POP_PAIR_S(3);
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string; }
  any
  { goto colorize_string; }
*/
colorize_rawstring:
    yytok=yycur;
/*!re2c
  ")" @t1 DCHAR{0,16} @t2 ["]
  {
    if((t2-t1)==state[0].Len&&0==memcmp(t1,state[0].Raw,(t2-t1)*sizeof(UTCHAR)))
    {
      Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_STRING1,EPriorityNormal);
      state[0].State=PARSER_CLEAR;
      POP_PAIR_S(4);
      goto colorize_clear;
    }
    yycur=yytok+1;
    goto colorize_rawstring;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_rawstring; }
  any
  { goto colorize_rawstring; }
*/
colorize_end:
    if(state[0].State==PARSER_COMMENT1)
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_COMMENT1,EPriorityNormal);
    if(state[0].State==PARSER_STRING)
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_STRING1,EPriorityNormal);
    if(state[0].State==PARSER_RAWSTRING)
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_STRING1,EPriorityNormal);
  }
colorize_exit:
  PairStackClear(params->LocalHeap,&hl_state);
}
