%{
/*
    zc_parser.y
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
#include <limits.h>
#include <stdlib.h>
#include <windows.h>
#include "abplugin.h"
#include "../plugins/zcustom/abzcustom.h"
#include "../memory.h"
#define YYPARSE_PARAM cc_param
//#define YYPURE TRUE
#define YYLEX_PARAM YYPARSE_PARAM
#define PARAM_MIN (((struct CharacterClassParam *)cc_param)->char_class->min)
#define PARAM_MAX (((struct CharacterClassParam *)cc_param)->char_class->max)
int yyerror(const char *msg);

%}

%pure-parser
%union
{
  unsigned int digit;
}

%token CLASS
%token <digit> DIGIT

%{
int yylex(YYSTYPE *yylval,void *cc_param);
%}

%%

list:                                   {}
        | exp                           {}
;

exp:    CLASS                           { PARAM_MIN=1; PARAM_MAX=1; }
        | CLASS '*'                     { PARAM_MIN=0; PARAM_MAX=INT_MAX; }
        | CLASS '+'                     { PARAM_MIN=1; PARAM_MAX=INT_MAX; }
        | CLASS '?'                     { PARAM_MIN=0; PARAM_MAX=1; }
        | CLASS '{' DIGIT ',' '}'       { PARAM_MIN=$3; PARAM_MAX=INT_MAX; }
        | CLASS '{' DIGIT ',' DIGIT '}' { PARAM_MIN=$3; PARAM_MAX=$5; }
;

%%

int yyerror(const char *msg)
{
  (void)msg;
  return 0;
}

#include "zc_lexer.cpp"
