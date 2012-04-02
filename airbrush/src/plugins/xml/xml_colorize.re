/*
    xml_colorize.re
    Copyright (C) 2012 zg

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
#include "../plugins/xml/abxml.h"

typedef unsigned short UTCHAR;

#define YYCTYPE unsigned long
#define YYCURSOR yycur
#define YYLIMIT yyend
#define YYMARKER yytmp
#define YYCTXMARKER yyctxtmp
#define YYFILL(n)

/*!re2c
any                     = [\U00000001-\U0000FFFF];
Char                    = [\U00000001-\U0000D7FF\U0000E000-\U0000FFFD];
S                       = [\U00000020\U00000009\U0000000D\U0000000A]+;
NameStartChar           = [:A-Z_a-z\U000000C0-\U000000D6\U000000D8-\U000000F6\U000000F8-\U000002FF\U00000370-\U0000037D\U0000037F-\U00001FFF\U0000200C-\U0000200D\U00002070-\U0000218F\U00002C00-\U00002FEF\U00003001\U0000D7FF\U0000F900-\U0000FDCF\U0000FDF0-\U0000FFFD];
NameChar                = NameStartChar|[-.0-9\U000000B7\U00000300-\U0000036F\U0000203F-\U00002040];
Name                    = NameStartChar (NameChar)*;
SystemLiteral           = ("\"" [^"\000]* "\"")|("'" [^'\000]* "'");
*/

void WINAPI Colorize(int index,struct ColorizeParams *params)
{
  const UTCHAR *commentstart;
  const UTCHAR *line;
  int linelen,startcol;
  int lColorize=0;
  int state_data=PARSER_CLEAR;
  int *state=&state_data;
  int state_size=sizeof(state_data);
  const UTCHAR *yycur,*yyend,*yytmp,*yyctxtmp,*yytok=NULL;
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
    if(state[0]==PARSER_PI) goto colorize_pi;
    if(state[0]==PARSER_CDATA) goto colorize_cdata;
    if(state[0]==PARSER_OPENTAG) goto colorize_opentag;
    if(state[0]==PARSER_CLOSETAG) goto colorize_closetag;
    if(state[0]==PARSER_VALUES1) goto colorize_values1;
    if(state[0]==PARSER_VALUES2) goto colorize_values2;
    if(state[0]==PARSER_STRING1) goto colorize_string1;
    if(state[0]==PARSER_STRING2) goto colorize_string2;
    if(state[0]==PARSER_MARKUP) goto colorize_markup;
    if(state[0]==PARSER_SUBSET) goto colorize_subset;
/*!re2c
  /* Comments */
  "<!--"
  {
    state[0]=PARSER_COMMENT;
    commentstart=yytok;
    goto colorize_comment;
  }
  /* Processing Instructions */
  "<?" Name S
  {
    state[0]=PARSER_PI;
    commentstart=yytok;
    goto colorize_pi;
  }
  "<?" Name "?>"
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_PI,EPriorityNormal);
    goto colorize_clear;
  }
  /* CDATA */
  "<![CDATA["
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_CDATA,EPriorityNormal);
    state[0]=PARSER_CDATA;
    goto colorize_cdata;
  }
  /* open tag */
  "<" Name
  {
    state[0]=PARSER_OPENTAG;
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_OPENTAG,EPriorityNormal);
    goto colorize_opentag;
  }
  /* close tag */
  "</" Name
  {
    state[0]=PARSER_CLOSETAG;
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_CLOSETAG,EPriorityNormal);
    goto colorize_closetag;
  }
  /* markup */
  "<!" Name
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_MARKUP,EPriorityNormal);
    state[0]=PARSER_MARKUP;
    goto colorize_markup;
  }
  /* char data */
  [&%] Name ";"
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_REFERENCE,EPriorityNormal); goto colorize_clear; }
  [<&\000]
  {
    if(yytok==yyend) goto colorize_end;
    if(lColorize)
    {
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    }
    goto colorize_clear;
  }
  any
  { goto colorize_clear; }
*/
colorize_comment:
    yytok=yycur;
/*!re2c
  "-->"
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_COMMENT,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  Char\"-"|"-" (Char\"-")
  { goto colorize_comment; }
  any|[\000]
  {
    if(yytok==yyend) goto colorize_end;
    if(lColorize)
    {
      Info.pAddColor(lno,commentstart-line,yytok-commentstart,colors+HC_COMMENT,EPriorityNormal);
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
      commentstart=yycur;
    }
    goto colorize_comment;
  }
*/
colorize_pi:
    yytok=yycur;
/*!re2c
  "?>"
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_PI,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  Char
  { goto colorize_pi; }
  any|[\000]
  {
    if(yytok==yyend) goto colorize_end;
    if(lColorize)
    {
      Info.pAddColor(lno,commentstart-line,yytok-commentstart,colors+HC_PI,EPriorityNormal);
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
      commentstart=yycur;
    }
    goto colorize_pi;
  }
*/
colorize_cdata:
    yytok=yycur;
/*!re2c
  "]]>"
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_CDATA,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  Char
  { goto colorize_cdata; }
  any|[\000]
  {
    if(yytok==yyend) goto colorize_end;
    if(lColorize)
    {
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    }
    goto colorize_cdata;
  }
*/
colorize_opentag:
    yytok=yycur;
/*!re2c
  /*work around start*/
  S
  {
    goto colorize_opentag;
  }
  /*work around end*/
  Name S?
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ATTRNAME,EPriorityNormal);
    state[0]=PARSER_VALUES1;
    goto colorize_values1;
  }
  "/>"
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_CLOSETAG,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  ">"
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_OPENTAG,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  any|[\000]
  {
    if(yytok==yyend) goto colorize_end;
    if(lColorize)
    {
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    }
    goto colorize_opentag;
  }
*/
colorize_values1:
    yytok=yycur;
/*!re2c
  /*work around start*/
  S
  {
    goto colorize_values1;
  }
  /*work around end*/
  "=" S?
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ATTRNAME,EPriorityNormal);
    state[0]=PARSER_VALUES2;
    goto colorize_values2;
  }
  any|[\000]
  {
    if(yytok==yyend) goto colorize_end;
    if(lColorize)
    {
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    }
    state[0]=PARSER_OPENTAG;
    goto colorize_opentag;
  }
*/
colorize_values2:
    yytok=yycur;
/*!re2c
  /*work around start*/
  S
  {
    goto colorize_values2;
  }
  /*work around end*/
  "\""
  {
    state[0]=PARSER_STRING1;
    commentstart=yytok;
    goto colorize_string1;
  }
  "'"
  {
    state[0]=PARSER_STRING2;
    commentstart=yytok;
    goto colorize_string2;
  }
  any|[\000]
  {
    if(yytok==yyend) goto colorize_end;
    if(lColorize)
    {
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    }
    state[0]=PARSER_OPENTAG;
    goto colorize_opentag;
  }
*/
colorize_string1:
    yytok=yycur;
/*!re2c
  "\""
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_ATTRVALUE,EPriorityNormal);
    state[0]=PARSER_OPENTAG;
    goto colorize_opentag;
  }
  [&%] Name ";"
  {
    if(lColorize)
    {
      Info.pAddColor(lno,commentstart-line,yytok-commentstart,colors+HC_ATTRVALUE,EPriorityNormal);
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_REFERENCE,EPriorityNormal);
      commentstart=yycur;
    }
    goto colorize_string1;
  }
  [^<&"\000]
  { goto colorize_string1; }
  any|[\000]
  {
    if(yytok==yyend) goto colorize_end;
    if(lColorize)
    {
      Info.pAddColor(lno,commentstart-line,yytok-commentstart,colors+HC_ATTRVALUE,EPriorityNormal);
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
      commentstart=yycur;
    }
    goto colorize_string1;
  }
*/
colorize_string2:
    yytok=yycur;
/*!re2c
  "'"
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_ATTRVALUE,EPriorityNormal);
    state[0]=PARSER_OPENTAG;
    goto colorize_opentag;
  }
  [&%] Name ";"
  {
    if(lColorize)
    {
      Info.pAddColor(lno,commentstart-line,yytok-commentstart,colors+HC_ATTRVALUE,EPriorityNormal);
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_REFERENCE,EPriorityNormal);
      commentstart=yycur;
    }
    goto colorize_string2;
  }
  [^<&'\000]
  { goto colorize_string2; }
  any|[\000]
  {
    if(yytok==yyend) goto colorize_end;
    if(lColorize)
    {
      Info.pAddColor(lno,commentstart-line,yytok-commentstart,colors+HC_ATTRVALUE,EPriorityNormal);
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
      commentstart=yycur;
    }
    goto colorize_string2;
  }
*/
colorize_closetag:
    yytok=yycur;
/*!re2c
  /*work around start*/
  S
  {
    goto colorize_closetag;
  }
  /*work around end*/
  ">"
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_CLOSETAG,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  any|[\000]
  {
    if(yytok==yyend) goto colorize_end;
    if(lColorize)
    {
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    }
    goto colorize_closetag;
  }
*/
colorize_markup:
    yytok=yycur;
/*!re2c
  (Name S?)|(SystemLiteral S?)|(S)
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_MARKUP,EPriorityNormal);
    goto colorize_markup;
  }
  ">"
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_MARKUP,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  "["
  {
    state[0]=PARSER_SUBSET;
    commentstart=yytok;
    goto colorize_subset;
  }
  any|[\000]
  {
    if(yytok==yyend) goto colorize_end;
    if(lColorize)
    {
      Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    }
    goto colorize_markup;
  }
*/
colorize_subset:
    yytok=yycur;
/*!re2c
  "]"
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_SUBSET,EPriorityNormal);
    state[0]=PARSER_MARKUP;
    goto colorize_markup;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_subset; }
  any
  { goto colorize_subset; }
*/

colorize_end:
    if(state[0]==PARSER_COMMENT)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors+HC_COMMENT,EPriorityNormal);
    if(state[0]==PARSER_PI)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors+HC_PI,EPriorityNormal);
    if((state[0]==PARSER_STRING1)||(state[0]==PARSER_STRING2))
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors+HC_ATTRVALUE,EPriorityNormal);
    if(state[0]==PARSER_SUBSET)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors+HC_SUBSET,EPriorityNormal);
  }
colorize_exit:
  PairStackClear(params->LocalHeap,&hl_state);
}
