%{
/*
    Scripts sub-plugin for FARMail
    Copyright (C) 2002-2004 FARMail Group

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
#include "language.hpp"

#if defined(__BORLANDC__)
#define malloc(size) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size)
#define free(ptr) ((ptr)?HeapFree(GetProcessHeap(),0,ptr):0)
#endif
%}
%pure_parser
%union
{
  __INT64 number; //numbers
  long variable; //variables, functions, custom functions
  long ptr; //internal use
  char *string; //strings
}

%token <number> _NUMBER
%token <variable> _VAR _FUNC
%token <string> _STRING
%token _IF _ELSE _ENDIF _WHILE _WEND _CONTINUE _BREAK _GOSUB _SUB _ENDSUB

%type <ptr> expr stmt stlist paramlist funcstart
%type <ptr> if then1 else1 then2 else2
%type <ptr> while sub


%right  '='
%left _OR
%left _AND
%left _GT _GE _LT _LE _LQ _EQ _NE
%left '+' '-'
%left '*' '/'
%left UNARYMINUS _NOT _INC _DEC
%{
#define YYPARSE_PARAM
%}

%%

list:
  | list stmt
/*  | list error*/ /*no error recovery*/
  ;
stmt:
  '\n'                                  { $$=code.GetPosition(); }
  | expr '\n'                           { $$=$1; AddCode(opPOP); }
  | if then1 else1                      { $$=$1; SolveIfFrame($3,code.GetPosition()); }
  | if then2 _ELSE else2 _ENDIF         { $$=$1; SolveIfFrame($4,code.GetPosition()); }
  | while stlist _WEND                  {
                                          $$=$1;
                                          AddCode(opJMP);
                                          AddXRef(&pLoopFrame->Cond,code.GetPosition());
                                          AddCode(0);
                                          SolveLoopFrame($2,$1,code.GetPosition());
                                        }
  | _BREAK '\n'                         {
                                          $$=code.GetPosition();
                                          if(pLoopFrame)
                                          {
                                            AddCode(opJMP);
                                            AddXRef(&pLoopFrame->Follow,code.GetPosition());
                                            AddCode(0);
                                          }
                                        }
  | _CONTINUE '\n'                      {
                                          $$=code.GetPosition();
                                          if(pLoopFrame)
                                          {
                                            AddCode(opJMP);
                                            AddXRef(&pLoopFrame->Cond,code.GetPosition());
                                            AddCode(0);
                                          }
                                        }
  | sub stlist _ENDSUB                  {
                                          $$=$1;
                                          AddCode(opRET);
                                          SolveIfFrame(code.GetPosition(),code.GetPosition());
                                        }
  | _GOSUB _VAR                         {
                                          $$=code.GetPosition();
                                          if(symbols[$2].Value().getcodepos())
                                          {
                                            AddCode(opCALL);
                                            AddCode(symbols[$2].Value().getcodepos());
                                          }
                                        }
  ;

paramlist:  paramlist ',' expr           {
                                          $$=$1;
                                          (pFunctionFrame->Count)++;
                                        }
  | expr                                {
                                          $$=$1;
                                          (pFunctionFrame->Count)++;
                                        }
  |                                     { $$=code.GetPosition(); }
  ;

funcstart: '('                          { CreateFunctionFrame(); }
  ;

expr: _VAR                              {
                                          $$=AddCode(opPUSHV);
                                          AddCode($1);
                                        }
  | _NUMBER                             {
                                          $$=AddCode(opPUSHD);
                                          AddInt64($1);
                                        }
  | _STRING                             {
                                          $$=AddCode(opPUSHD);
                                          temp_string=$1;
                                          HeapFree(heap,0,$1);
                                          AddString(temp_string);
                                        }
  | _VAR '[' expr ']'                   {
                                          $$=$3;
                                          AddCode(opPUSHS);
                                          AddCode($1);
                                        }
  | _VAR '=' expr                       {
                                          AddCode(opMOV);
                                          AddCode($1);
                                          $$=$3;
                                        }
  | _VAR '[' expr ']' '=' expr          {
                                          $$=$3;
                                          AddCode(opMOVS);
                                          AddCode($1);
                                        }
  | _DEC _VAR                           {
                                          $$=code.GetPosition();
                                          AddCode(opDEC);
                                          AddCode($2);
                                        }
  | _VAR _DEC                           {
                                          $$=code.GetPosition();
                                          AddCode(opPOSTDEC);
                                          AddCode($1);
                                        }
  | _INC _VAR                           {
                                          $$=code.GetPosition();
                                          AddCode(opINC);
                                          AddCode($2);
                                        }
  | _VAR _INC                           {
                                          $$=code.GetPosition();
                                          AddCode(opPOSTINC);
                                          AddCode($1);
                                        }
  | '(' expr ')'                        { $$=$2; }
  | expr '+' expr                       { $$=$1; AddCode(opADD); }
  | expr '-' expr                       { $$=$1; AddCode(opSUB); }
  | expr '*' expr                       { $$=$1; AddCode(opMUL); }
  | expr '/' expr                       { $$=$1; AddCode(opDIV); }
  | '-' expr %prec UNARYMINUS           { $$=$2; AddCode(opNEG);}
  | expr _GT expr                       { $$=$1; AddCode(opGT); }
  | expr _GE expr                       { $$=$1; AddCode(opGE); }
  | expr _LT expr                       { $$=$1; AddCode(opLT); }
  | expr _LE expr                       { $$=$1; AddCode(opLE); }
  | expr _EQ expr                       { $$=$1; AddCode(opEQ); }
  | expr _NE expr                       { $$=$1; AddCode(opNE); }
  | expr _AND expr                      { $$=$1; AddCode(opAND); }
  | expr _OR expr                       { $$=$1; AddCode(opOR); }
  | _NOT expr                           { $$=$2; AddCode(opNOT); }
  | _FUNC funcstart paramlist ')'       {
                                          $$=$3;
                                          AddCode(opPUSHD);
                                          AddInt64(pFunctionFrame->Count);
                                          SolveFunctionFrame();
                                          AddCode(opFUNC);
                                          AddCode($1);
                                        }
  ;

stlist:                                 { $$=code.GetPosition(); }
  | stlist stmt
  ;

if: _IF '(' expr ')'                    {
                                          CreateIfFrame();
                                          AddCode(opJE);
                                          AddXRef(&pIfFrame->Other,code.GetPosition());
                                          AddCode(0);
                                          $$=$3;
                                        }
  ;

then1: stlist _ENDIF                    {
                                          $$=$1;
                                          AddCode(opJMP);
                                          AddXRef(&pIfFrame->Follow,code.GetPosition());
                                          AddCode(0);
                                        }
  ;

else1:                                  { $$=code.GetPosition(); }
  ;

then2: stlist                           {
                                          $$=$1;
                                          AddCode(opJMP);
                                          AddXRef(&pIfFrame->Follow,code.GetPosition());
                                          AddCode(0);
                                        }
  ;

else2: stlist                           { $$=$1; }
  ;

while: _WHILE '(' expr ')'              {
                                          CreateLoopFrame();
                                          AddCode(opJE);
                                          AddXRef(&pLoopFrame->Follow,code.GetPosition());
                                          AddCode(0);
                                          $$=$3;
                                        }
  ;

sub: _SUB _VAR                          {
                                          $$=code.GetPosition();
                                          CreateIfFrame();
                                          AddCode(opJMP);
                                          AddXRef(&pIfFrame->Other,code.GetPosition());
                                          AddCode(0);
                                          symbols[$2].Value().setcodepos(code.GetPosition());
                                        }
  ;

%%
void Parser::yyerror(int msg)
{
  error_index=msg;
  return;
}
