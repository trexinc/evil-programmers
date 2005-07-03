/*
    Scripts sub-plugin for FARMail
    Copyright (C) 2002-2005 FARMail Group

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "parser.hpp"
#include "scripts.hpp"
#include <stdio.h>

__INT64 Parser::GetValue(void)
{
  char *string=GetString();
  __INT64 result=0;
  if(string)
  {
    result=__ATOI(string);
    HeapFree(heap,0,string);
  }
  return result;
}

char *Parser::GetString(void)
{
  long len=yy_cursor-yy_token+1;
  char *result=(char *)HeapAlloc(heap,HEAP_ZERO_MEMORY,len);
  if(result)
    memcpy(result,yy_token,len-1); //memory zeroed so last simbol is zero.
  return result;
}

bool Parser::fill(int n)
{
  (void)n;
  if(!yy_eof)
  {
    DWORD transferred,tail_len=yy_limit-yy_token,skip_len=yy_token-yy_buffer;
    if((yy_buffer_size-tail_len)<n)
    {
      size_t add_len=n;
      if(add_len<yy_buffer_size) add_len=yy_buffer_size;
      YYCTYPE *yy_buffer_old=yy_buffer;
      yy_buffer=(YYCTYPE *)HeapReAlloc(heap,HEAP_ZERO_MEMORY,yy_buffer,yy_buffer_size+add_len);
      if(yy_buffer)
      {
        yy_buffer_size+=add_len;
        yy_token+=(yy_buffer-yy_buffer_old);
        yy_marker+=(yy_buffer-yy_buffer_old);
        yy_cursor+=(yy_buffer-yy_buffer_old);
        yy_limit+=(yy_buffer-yy_buffer_old);
        yy_linestart+=(yy_buffer-yy_buffer_old);
      }
      else
      {
        yy_buffer=yy_buffer_old;
        return false;
      }
    }
    memcpy(yy_buffer,yy_token,tail_len);
    yy_marker-=skip_len;
    yy_cursor-=skip_len;
    yy_limit-=skip_len;
    yy_linestart-=skip_len;
    yy_token=yy_buffer;
    if(ReadFile(file,yy_limit,yy_buffer_size-tail_len,&transferred,NULL)||(transferred=0,true))
    {
      if(transferred!=(yy_buffer_size-tail_len))
      {
        yy_eof=yy_limit+transferred;
        *(yy_eof)++='\n';
      }
      yy_limit+=transferred;
    }
  }
  return true;
}

/*!re2c
any = [\000-\377];
O = [0-7];
D = [0-9];
L = [a-zA-Z_];
H = [a-fA-F0-9];
ESC = [\\] ([abfnrtv?'"\\] | "x" H H | O O O);
ESC2 = [\\] ([abfnrtv?'"\\]);
*/

static inline int bcd2byte(int value)
{
  int res=value-0x30;
  if(res>9)
  {
    res-=7;
    if(res>15)
      res-=0x20;
  }
  return res;
}


int Parser::yylex(YYSTYPE *yylval)
{
  if(yy_cursor==yy_eof) return 0; //EOF
parse_clear:
  yy_col=yy_cursor-yy_linestart;
  if(yy_line_update)
  {
    yy_line_update=false;
    yy_line++;
  }
  yy_token=yy_cursor;
/*!re2c
  "rem" [ \t] (any\[\n])* / "\n" { goto parse_clear; }
  "//" (any\[\n])* / "\n" { goto parse_clear; }
  "/*" { goto parse_comment; }
  D+
  {
    yylval->number=GetValue();
    return _NUMBER;
  }
  "'" (ESC2|any\[\n\\']) "'"
  {
    if(yy_token[1]!='\\')
      yylval->number=yy_token[1];
    else
    {
      switch(yy_token[2])
      {
        case 'a':
          yylval->number='\a';
          break;
        case 'b':
          yylval->number='\b';
          break;
        case 'f':
          yylval->number='\f';
          break;
        case 'n':
          yylval->number='\n';
          break;
        case 'r':
          yylval->number='\r';
          break;
        case 't':
          yylval->number='\t';
          break;
        case 'v':
          yylval->number='\v';
          break;
        default:
          yylval->number=yy_token[2];
          break;
      }
    }
    return _NUMBER;
  }
  "if" { return _IF; }
  "else" { return _ELSE; }
  "endif" { return _ENDIF; }
  "while" { return _WHILE; }
  "wend" { return _WEND; }
  "break" { return _BREAK; }
  "continue" { return _CONTINUE; }
  "gosub" { return _GOSUB; }
  "sub" { return _SUB; }
  "endsub" { return _ENDSUB; }

  L (L|D)*
  {
    int res=_FUNC;
    char *name=GetString();
    yylval->variable=functions.IndexOf(name);
    if(yylval->variable<0)
    {
      res=_VAR;
      yylval->variable=symbols.IndexOf(name);
      if(yylval->variable<0)
      {
        if(symbols.Add(name))
          yylval->variable=symbols.GetCount()-1;
      }
    }
    if(name) HeapFree(heap,0,name);
    return res;
  }
  (["] (ESC|any\[\n\\"])* ["])
  {
    char *string=GetString(); size_t j=0;
    for(size_t i=1;i<(yy_cursor-yy_token-1);i++)
    {
      if(string[i]!='\\')
        string[j++]=string[i];
      else
      {
        if(string[++i]=='x')
        {
          int hex=bcd2byte(string[++i]);
          hex=hex*16+bcd2byte(string[++i]);
          string[j++]=hex;
        }
        else if(string[i]=='0'||string[i]=='1'||string[i]=='2'||string[i]=='3')
        {
          int oct=string[i]-'0';
          oct=oct*8+(string[++i]-'0');
          oct=oct*8+(string[++i]-'0');
          string[j++]=oct;
        }
        else
        {
          switch(string[i])
          {
            case 'a':
              string[j++]='\a';
              break;
            case 'b':
              string[j++]='\b';
              break;
            case 'f':
              string[j++]='\f';
              break;
            case 'n':
              string[j++]='\n';
              break;
            case 'r':
              string[j++]='\r';
              break;
            case 't':
              string[j++]='\t';
              break;
            case 'v':
              string[j++]='\v';
              break;
            default:
              string[j++]=string[i];
              break;
          }
        }
      }
    }
    string[j]=0;
    yylval->string=string;
    return _STRING;
  }
  ">=" { return _GE; }
  ">"  { return _GT; }
  "<=" { return _LE; }
  "<"  { return _LT; }
  "==" { return _EQ; }
  "!=" { return _NE; }
  "&&" { return _AND; }
  "||" { return _OR; }
  "!"  { return _NOT; }
  "++"  { return _INC; }
  "--"  { return _DEC; }

  [ \t\v\f\r]+    { goto parse_clear; }
  "\n"
  {
    yy_linestart=yy_cursor;
    yy_line_update=true;
    if(yy_col) yy_col--;
    return *yy_token;
  }
  any
  {
    if(!(*yy_token)) return ' ';
    return *yy_token;
  }
*/
parse_comment:
  yy_token=yy_cursor;
/*!re2c
  "*/" { goto parse_clear; }
  "\n"
  {
    yy_linestart=yy_cursor;
    yy_line++;
    if(yy_cursor==yy_eof) return 0; //EOF
    goto parse_comment;
  }
  any { goto parse_comment; }
*/
}
