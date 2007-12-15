/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     _NUMBER = 258,
     _VAR = 259,
     _FUNC = 260,
     _STRING = 261,
     _IF = 262,
     _ELSE = 263,
     _ENDIF = 264,
     _WHILE = 265,
     _WEND = 266,
     _CONTINUE = 267,
     _BREAK = 268,
     _GOSUB = 269,
     _SUB = 270,
     _ENDSUB = 271,
     _OR = 272,
     _AND = 273,
     _NE = 274,
     _EQ = 275,
     _LQ = 276,
     _LE = 277,
     _LT = 278,
     _GE = 279,
     _GT = 280,
     _DEC = 281,
     _INC = 282,
     _NOT = 283,
     UNARYMINUS = 284
   };
#endif
/* Tokens.  */
#define _NUMBER 258
#define _VAR 259
#define _FUNC 260
#define _STRING 261
#define _IF 262
#define _ELSE 263
#define _ENDIF 264
#define _WHILE 265
#define _WEND 266
#define _CONTINUE 267
#define _BREAK 268
#define _GOSUB 269
#define _SUB 270
#define _ENDSUB 271
#define _OR 272
#define _AND 273
#define _NE 274
#define _EQ 275
#define _LQ 276
#define _LE 277
#define _LT 278
#define _GE 279
#define _GT 280
#define _DEC 281
#define _INC 282
#define _NOT 283
#define UNARYMINUS 284




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 30 "parser_grammar.y"
{
  __INT64 number; //numbers
  long variable; //variables, functions, custom functions
  long ptr; //internal use
  char *string; //strings
}
/* Line 1529 of yacc.c.  */
#line 114 "parser_grammar.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



