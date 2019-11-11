/*
    zc_lexer.re
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

#include <ctype.h>
#define YYCTYPE unsigned long
#define YYCURSOR (*yycur)
#define YYLIMIT yyend
#define YYMARKER yytmp
#define YYFILL(n)
#define PARAM_CHAR_CLASS (reinterpret_cast<struct CharacterClassParam*>(cc_param)->char_class->Class)
#define GET_CHAR(res) \
{ \
  res=*cc_cur; \
  cc_cur++; \
  if(res=='\\') \
  { \
    switch(*cc_cur) \
    { \
      case 'n': \
        res='\n'; \
        break; \
      case 'r': \
        res='\r'; \
        break; \
      case 't': \
        res='\t'; \
        break; \
      case 's': \
        res=' '; \
        break; \
      default: \
      { \
        res=-1; \
        if(isdigit(*cc_cur)) \
        { \
          unsigned char *cc_new; \
          int val=strtol(reinterpret_cast<char*>(cc_cur),reinterpret_cast<char**>(&cc_new),8); \
          if((val<256)&&(val>-1)) \
          { \
            cc_cur=cc_new-1; \
            res=val; \
          } \
        } \
        if(res==-1) \
          res=*cc_cur; \
      } \
    } \
    cc_cur++; \
  } \
}

/*!re2c
full = [\000-\377];
any = [\001-\377];
esc = any \ [\\];
cstring = "[" ((esc \ [\]]) | "\\" any)* "]";
digit = [0-9];
*/
int yylex(YYSTYPE *yylval,void *cc_param)
{
  unsigned char *yyend=reinterpret_cast<struct CharacterClassParam*>(cc_param)->end;
  unsigned char **yycur=&(reinterpret_cast<struct CharacterClassParam*>(cc_param)->start);
  unsigned char *yytmp,*yytok=NULL;
lexer_clear:
  yytok=*yycur;
/*!re2c
  cstring
  {
    unsigned char action=1;
    unsigned char *cc_cur=yytok+1,*cc_end=(*yycur)-1;
    int first_char,last_char;
    if(*cc_cur=='^')
    {
      action=0;
      cc_cur++;
    }
    memset(PARAM_CHAR_CLASS,!action,sizeof(PARAM_CHAR_CLASS));
    while(cc_cur<cc_end)
    {
      GET_CHAR(first_char)
      if((*cc_cur=='-')&&((cc_end-cc_cur)>1))
      {
        cc_cur++;
        GET_CHAR(last_char)
        for(int i=first_char;i<=last_char;i++)
          PARAM_CHAR_CLASS[i]=action;
      }
      else
        PARAM_CHAR_CLASS[first_char]=action;
    }
    return CLASS;
  }
  digit+
  {
    yylval->digit=atoi(reinterpret_cast<char*>(yytok));
    return DIGIT;
  }
  [*+?{},] { return *yytok; }
  [ \t\v\f]+ { goto lexer_clear; }
  full
  {
    if(yytok==yyend) goto lexer_end;
    goto lexer_clear;
  }
*/
lexer_end:
  return 0;
}
