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
any                     = [\U00000001-\U0000ffff];
xmlvalid                = [\U00000009\U00000020-\U0000dfff\U0000e000-\U0000fffd];

Digit                   = [0-9];
LCLetter                = [a-z];
Special                 = ['()_,\-\./:=?];
UCLetter                = [A-Z];

LCNMCHAR                = [\.-];
UCNMCHAR                = [\.-:_];
RE                      = "\n";
RS                      = "\r";
SEPCHAR                 = "\011";
SPACE                   = "\040";

COM                     = "--";
CRO                     = "&#";
DSC                     = "]";
DSO                     = "[";
ERO                     = "&";
ETAGO                   = "</";
LIT                     = "\"";
LITA                    = "'";
MDC                     = ">";
MDO                     = "<!";
MSC                     = "]]";
NET                     = "/";
PERO                    = "%";
PIC                     = "?>";
PIO                     = "<?";
REFC                    = ";";
STAGO                   = "<";
TAGC                    = ">";

name0start0character    = (LCLetter)|(UCLetter);
name0character          = (name0start0character)|(Digit)|(LCNMCHAR)|(UCNMCHAR);

/*
name0start0character    = xmlvalid;
name0character          = xmlvalid;
*/

name                    = (name0start0character) (name0character)*;
number                  = (Digit)+;
number0token            = (Digit) (name0character)*;
name0token              = (name0character)+;
s                       = (SPACE)|(RE)|(RS)|(SEPCHAR);
ps                      = ((SPACE)|(RE)|(RS)|(SEPCHAR))+;
ws                      = ((SPACE)|(RE)|(RS)|(SEPCHAR))*;
reference0end           = (REFC)|(RE);
literal                 = ((LIT)(any\"\"")*(LIT))|((LITA)(any\"'")*(LITA));
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
    if(state[0]==PARSER_OPENTAG) goto colorize_opentag;
    if(state[0]==PARSER_CLOSETAG) goto colorize_closetag;
    if(state[0]==PARSER_MARKUP) goto colorize_markup;
    if(state[0]==PARSER_SUBSET) goto colorize_subset;
    if(state[0]==PARSER_PI) goto colorize_pi;
    if(state[0]==PARSER_VALUES) goto colorize_values;
    if(state[0]==PARSER_STRING1) goto colorize_string1;
    if(state[0]==PARSER_STRING2) goto colorize_string2;
    if(state[0]==PARSER_CDATA) goto colorize_cdata;
/*!re2c
  /*open tag*/
  STAGO name ws
  {
    state[0]=PARSER_OPENTAG;
    commentstart=yytok;
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_OPENTAG,EPriorityNormal);
    goto colorize_opentag;
  }
  STAGO TAGC
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal); goto colorize_clear; }
  /*close tag*/
  ETAGO name ws
  {
    state[0]=PARSER_CLOSETAG;
    commentstart=yytok;
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_CLOSETAG,EPriorityNormal);
    goto colorize_closetag;
  }
  ETAGO TAGC
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal); goto colorize_clear; }
  "<![CDATA["
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_CDATA,EPriorityNormal);
    state[0]=PARSER_CDATA;
    commentstart=yytok;
    goto colorize_cdata;
  }
  /*markup delcarations*/
  MDO name ws
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_MARKUP,EPriorityNormal);
    state[0]=PARSER_MARKUP;
    commentstart=yytok;
    goto colorize_markup;
  }
  MDO MDC
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_MARKUP,EPriorityNormal);
    goto colorize_clear;
  }
  /*comment*/
  (MDO)/(COM)
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_MARKUP,EPriorityNormal);
    state[0]=PARSER_MARKUP;
    commentstart=yytok;
    goto colorize_markup;
  }
  /*declaration subset*/
  MDO DSO ws
  { state[0]=PARSER_SUBSET; commentstart=yytok; goto colorize_subset; }
  MSC MDC
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal); goto colorize_clear; }
  /*processing instruction*/
  PIO
  { state[0]=PARSER_PI; commentstart=yytok; goto colorize_pi; }
  /*reference*/
  (CRO number (reference0end)?)|(ERO name (reference0end)?)
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_REFERENCE,EPriorityNormal); goto colorize_clear; }
  (CRO number0token (reference0end)?)|(CRO name (reference0end)?)
  { if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal); goto colorize_clear; }
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
  COM ws
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_COMMENT,EPriorityNormal);
    state[0]=PARSER_MARKUP;
    goto colorize_markup;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_comment; }
  any
  { goto colorize_comment; }
*/
colorize_opentag:
    yytok=yycur;
/*!re2c
  /*work around start*/
  ps
  {
    goto colorize_opentag;
  }
  /*work around end*/
  name (s)* "=" ws
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ATTRNAME,EPriorityNormal);
    state[0]=PARSER_VALUES;
    goto colorize_values;
  }
  /*work around start*/
  (name (s)*)/("\000")
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ATTRNAME,EPriorityNormal);
    goto colorize_opentag;
  }
  (s)* "=" ws
  {
    if(!(yytok-line))
    {
      if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ATTRNAME,EPriorityNormal);
      state[0]=PARSER_VALUES;
      goto colorize_values;
    }
    else
    {
      if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
      goto colorize_opentag;
    }
  }
  /*work around end*/
  name ws
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ATTRNAME,EPriorityNormal);
    goto colorize_opentag;
  }
  NET TAGC
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_CLOSETAG,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  TAGC
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_OPENTAG,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  NET
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  STAGO
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_opentag; }
  any
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    goto colorize_opentag;
  }
*/
colorize_closetag:
    yytok=yycur;
/*!re2c
  TAGC
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_CLOSETAG,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  STAGO
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_closetag; }
  any
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    goto colorize_opentag;
  }
*/
colorize_markup:
    yytok=yycur;
/*!re2c
  COM
  {
    state[0]=PARSER_COMMENT;
    commentstart=yytok;
    goto colorize_comment;
  }
  PERO name (reference0end)? ws
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    goto colorize_markup;
  }
  (number ws)|(name ws)|(literal ws)|(ps)
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_MARKUP,EPriorityNormal);
    goto colorize_markup;
  }
  MDC
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_MARKUP,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  DSO
  {
    state[0]=PARSER_SUBSET;
    commentstart=yytok;
    goto colorize_subset;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_markup; }
  any
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    goto colorize_markup;
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
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_cdata; }
  any
  {
    goto colorize_cdata;
  }
*/
colorize_subset:
    yytok=yycur;
/*!re2c
  MSC MDC
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_SUBSET,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  DSC
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
colorize_pi:
    yytok=yycur;
/*!re2c
  PIC
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_PI,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_pi; }
  any
  { goto colorize_pi; }
*/
colorize_values:
    yytok=yycur;
/*!re2c
  /*work around start*/
  ps
  {
    goto colorize_values;
  }
  /*work around end*/
  name0token ws
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ATTRVALUE,EPriorityNormal);
    state[0]=PARSER_OPENTAG;
    goto colorize_opentag;
  }
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
  (any\[ '"\t\n>\000])+ ws
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    state[0]=PARSER_OPENTAG;
    goto colorize_opentag;
  }
  TAGC
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  NET
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  STAGO
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_values; }
  any
  {
    if(lColorize) Info.pAddColor(lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    goto colorize_opentag;
  }
*/
colorize_string1:
    yytok=yycur;
/*!re2c
  "\"" ws
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_ATTRVALUE,EPriorityNormal);
    state[0]=PARSER_OPENTAG;
    goto colorize_opentag;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string1; }
  any
  { goto colorize_string1; }
*/
colorize_string2:
    yytok=yycur;
/*!re2c
  "'" ws
  {
    if(lColorize) Info.pAddColor(lno,commentstart-line,yycur-commentstart,colors+HC_ATTRVALUE,EPriorityNormal);
    state[0]=PARSER_OPENTAG;
    goto colorize_opentag;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string2; }
  any
  { goto colorize_string2; }
*/

colorize_end:
    if(state[0]==PARSER_COMMENT)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors+HC_COMMENT,EPriorityNormal);
    if(state[0]==PARSER_SUBSET)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors+HC_SUBSET,EPriorityNormal);
    if(state[0]==PARSER_PI)
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors+HC_PI,EPriorityNormal);
    if((state[0]==PARSER_STRING1)||(state[0]==PARSER_STRING2))
      if(lColorize) Info.pAddColor(lno,commentstart-line,yyend-commentstart,colors+HC_ATTRVALUE,EPriorityNormal);
  }
colorize_exit:
  PairStackClear(params->LocalHeap,&hl_state);
}
