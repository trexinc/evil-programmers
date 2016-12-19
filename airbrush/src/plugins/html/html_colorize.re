/*
    html_colorize.re
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
#include "../plugins/html/abhtml.h"
#include <initguid.h>
#include "../plugins/php/guid.h"

typedef unsigned short UTCHAR;

struct CallbackParam
{
  int ok;
  int row;
  int col;
  ColorizeParams* params;
};

static int WINAPI code_callback(int from,int row,int col,void *param)
{
  const TCHAR* line;
  intptr_t linelen;
  line=Info.pGetLine(((CallbackParam *)param)->params->eid,row,&linelen);
  if(from==1)
  {
    if(!_tcsncmp(line+col,_T("?>"),2))
    {
      Info.pAddColor(((CallbackParam *)param)->params,row,col,2,colors+HC_PI,EPriorityNormal);
      ((CallbackParam *)param)->ok=1;
      ((CallbackParam *)param)->row=row;
      ((CallbackParam *)param)->col=col+2;
      return true;
    }
  }
  return false;
}

static void CallParser(ColorizeParams *params,CallbackParam *data)
{
  ColorizeParams code_params;
  code_params.size=sizeof(ColorizeParams);
  code_params.eid=params->eid;
  code_params.startline=data->row;
  code_params.startcolumn=data->col;
  code_params.endline=params->endline;
  code_params.topmargin=params->topmargin;
  code_params.bottommargin=params->bottommargin;
  code_params.margins=params->margins;
  code_params.data_size=params->data_size;
  if(params->data[0]<PARSER_PHP) params->data[0]|=PARSER_PHP;
  code_params.data=params->data;
  code_params.LocalHeap=params->LocalHeap;
  code_params.callback=code_callback;
  code_params.param=data;
  data->ok=0;
  Info.pCallParser(&PhpGUID,&code_params);
  if(data->ok)
  {
    params->data[0]=(unsigned char)(params->data[0]&PARSER_HTML);
    params->startline=data->row;
    params->startcolumn=data->col;
  }
}

#define YYCTYPE unsigned long
#define YYCURSOR yycur
#define YYLIMIT yyend
#define YYMARKER yytmp
#define YYCTXMARKER yyctxtmp
#define YYFILL(n)

/*!re2c
any                     = [\U00000001-\U0000ffff];

Digit                   = [0-9];
LCLetter                = [a-z];
Special                 = ['()_,\-\./:=?];
UCLetter                = [A-Z];

LCNMCHAR                = [\.-];
UCNMCHAR                = [\.-];
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

name                    = (name0start0character) (name0character)*;
number                  = (Digit)+;
number0token            = (Digit) (name0character)*;
name0token              = (name0character)+;
s                       = (SPACE)|(RE)|(RS)|(SEPCHAR);
ps                      = ((SPACE)|(RE)|(RS)|(SEPCHAR))+;
ws                      = ((SPACE)|(RE)|(RS)|(SEPCHAR))*;
reference0end           = (REFC)|(RE);
literal                 = ((LIT)(any\"\"")*(LIT))|((LITA)(any\"'")*(LITA));

PHP                     = "php";
*/

#define CALL_PHP \
Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_PI,EPriorityNormal); \
callback_data.row=lno; \
callback_data.col=yycur-line; \
callback_data.params=params; \
CallParser(params,&callback_data); \
if(!callback_data.ok) goto colorize_exit; \
goto colorize_start;


void WINAPI Colorize(intptr_t index,struct ColorizeParams *params)
{
  (void)index;
  const UTCHAR *commentstart;
  const UTCHAR *line;
  intptr_t linelen;
  int startcol;
  CallbackParam callback_data;
  int state_data=PARSER_CLEAR;
  int *state=&state_data;
  int state_size=sizeof(state_data);
  const UTCHAR *yycur,*yyend,*yytmp,*yyctxtmp,*yytok=NULL;
  struct PairStack *hl_state=NULL;
  intptr_t hl_row; intptr_t hl_col;
  if(params->data_size>=sizeof(state_data))
  {
    state=(int *)(params->data);
    state_size=params->data_size;
  }
  else
  {
    params->data=(unsigned char *)state;
    params->data_size=state_size;
  }
  Info.pGetCursor(params->eid,&hl_row,&hl_col);
  if(state[0]>=PARSER_PHP)
  {
    callback_data.row=params->startline;
    callback_data.col=params->startcolumn;
    callback_data.params=params;
    CallParser(params,&callback_data);
    if(!callback_data.ok) goto colorize_exit;
  }
colorize_start:
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
    if(state[0]==PARSER_COMMENT) goto colorize_comment;
    if(state[0]==PARSER_OPENTAG) goto colorize_opentag;
    if(state[0]==PARSER_CLOSETAG) goto colorize_closetag;
    if(state[0]==PARSER_MARKUP) goto colorize_markup;
    if(state[0]==PARSER_SUBSET) goto colorize_subset;
    if(state[0]==PARSER_PI) goto colorize_pi;
    if(state[0]==PARSER_VALUES) goto colorize_values;
    if(state[0]==PARSER_STRING1) goto colorize_string1;
    if(state[0]==PARSER_STRING2) goto colorize_string2;
/*!re2c
  /*open tag*/
  STAGO name ws
  {
    state[0]=PARSER_OPENTAG;
    commentstart=yytok;
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_OPENTAG,EPriorityNormal);
    goto colorize_opentag;
  }
  STAGO TAGC
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal); goto colorize_clear; }
  /*close tag*/
  ETAGO name ws
  {
    state[0]=PARSER_CLOSETAG;
    commentstart=yytok;
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_CLOSETAG,EPriorityNormal);
    goto colorize_closetag;
  }
  ETAGO TAGC
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal); goto colorize_clear; }
  /*markup delcarations*/
  MDO name ws
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_MARKUP,EPriorityNormal);
    state[0]=PARSER_MARKUP;
    commentstart=yytok;
    goto colorize_markup;
  }
  MDO MDC
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_MARKUP,EPriorityNormal);
    goto colorize_clear;
  }
  /*comment*/
  (MDO)/(COM)
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_MARKUP,EPriorityNormal);
    state[0]=PARSER_MARKUP;
    commentstart=yytok;
    goto colorize_markup;
  }
  /*declaration subset*/
  MDO DSO ws
  { state[0]=PARSER_SUBSET; commentstart=yytok; goto colorize_subset; }
  MSC MDC
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal); goto colorize_clear; }
  /*processing instruction*/
  PIO
  { state[0]=PARSER_PI; commentstart=yytok; goto colorize_pi; }
  /*php*/
  PIO PHP
  {
    CALL_PHP
  }
  /*reference*/
  (CRO number (reference0end)?)|(ERO name (reference0end)?)
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_REFERENCE,EPriorityNormal); goto colorize_clear; }
  (CRO number0token (reference0end)?)|(CRO name (reference0end)?)
  { Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal); goto colorize_clear; }
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
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_COMMENT,EPriorityNormal);
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
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ATTRNAME,EPriorityNormal);
    state[0]=PARSER_VALUES;
    goto colorize_values;
  }
  /*work around start*/
  (name (s)*)/("\000")
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ATTRNAME,EPriorityNormal);
    goto colorize_opentag;
  }
  (s)* "=" ws
  {
    if(!(yytok-line))
    {
      Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ATTRNAME,EPriorityNormal);
      state[0]=PARSER_VALUES;
      goto colorize_values;
    }
    else
    {
      Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
      goto colorize_opentag;
    }
  }
  /*work around end*/
  name ws
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ATTRNAME,EPriorityNormal);
    goto colorize_opentag;
  }
  TAGC
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_OPENTAG,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  NET
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  STAGO
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  /*php*/
  PIO PHP
  {
    CALL_PHP
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_opentag; }
  any
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    goto colorize_opentag;
  }
*/
colorize_closetag:
    yytok=yycur;
/*!re2c
  TAGC
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_CLOSETAG,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  STAGO
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  /*php*/
  PIO PHP
  {
    CALL_PHP
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_closetag; }
  any
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
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
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    goto colorize_markup;
  }
  (number ws)|(name ws)|(literal ws)
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_MARKUP,EPriorityNormal);
    goto colorize_markup;
  }
  MDC
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_MARKUP,EPriorityNormal);
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
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    goto colorize_markup;
  }
*/
colorize_subset:
    yytok=yycur;
/*!re2c
  MSC MDC
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_SUBSET,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  DSC
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_SUBSET,EPriorityNormal);
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
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_PI,EPriorityNormal);
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
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ATTRVALUE,EPriorityNormal);
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
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    state[0]=PARSER_OPENTAG;
    goto colorize_opentag;
  }
  TAGC
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  NET
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  STAGO
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    state[0]=PARSER_CLEAR;
    goto colorize_clear;
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_values; }
  any
  {
    Info.pAddColor(params,lno,yytok-line,yycur-yytok,colors+HC_ERROR,EPriorityNormal);
    goto colorize_opentag;
  }
*/
colorize_string1:
    yytok=yycur;
/*!re2c
  "\"" ws
  {
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_ATTRVALUE,EPriorityNormal);
    state[0]=PARSER_OPENTAG;
    goto colorize_opentag;
  }
  /*php*/
  PIO PHP
  {
    Info.pAddColor(params,lno,commentstart-line,yytok-commentstart,colors+HC_ATTRVALUE,EPriorityNormal);
    CALL_PHP
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
    Info.pAddColor(params,lno,commentstart-line,yycur-commentstart,colors+HC_ATTRVALUE,EPriorityNormal);
    state[0]=PARSER_OPENTAG;
    goto colorize_opentag;
  }
  /*php*/
  PIO PHP
  {
    Info.pAddColor(params,lno,commentstart-line,yytok-commentstart,colors+HC_ATTRVALUE,EPriorityNormal);
    CALL_PHP
  }
  [\000]
  { if(yytok==yyend) goto colorize_end; goto colorize_string2; }
  any
  { goto colorize_string2; }
*/

colorize_end:
    if(state[0]==PARSER_COMMENT)
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_COMMENT,EPriorityNormal);
    if(state[0]==PARSER_SUBSET)
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_SUBSET,EPriorityNormal);
    if(state[0]==PARSER_PI)
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_PI,EPriorityNormal);
    if((state[0]==PARSER_STRING1)||(state[0]==PARSER_STRING2))
      Info.pAddColor(params,lno,commentstart-line,yyend-commentstart,colors+HC_ATTRVALUE,EPriorityNormal);
  }
colorize_exit:
  PairStackClear(params->LocalHeap,&hl_state);
}
