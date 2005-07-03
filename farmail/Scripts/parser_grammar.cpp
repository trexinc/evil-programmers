/* A Bison parser, made from parser_grammar.y
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

# define	_NUMBER	257
# define	_VAR	258
# define	_FUNC	259
# define	_STRING	260
# define	_IF	261
# define	_ELSE	262
# define	_ENDIF	263
# define	_WHILE	264
# define	_WEND	265
# define	_CONTINUE	266
# define	_BREAK	267
# define	_GOSUB	268
# define	_SUB	269
# define	_ENDSUB	270
# define	_OR	271
# define	_AND	272
# define	_GT	273
# define	_GE	274
# define	_LT	275
# define	_LE	276
# define	_LQ	277
# define	_EQ	278
# define	_NE	279
# define	UNARYMINUS	280
# define	_NOT	281
# define	_INC	282
# define	_DEC	283

#line 1 "parser_grammar.y"

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

#line 29 "parser_grammar.y"
#ifndef YYSTYPE
typedef union
{
  __INT64 number; //numbers
  long variable; //variables, functions, custom functions
  long ptr; //internal use
  char *string; //strings
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
#line 54 "parser_grammar.y"

#define YYPARSE_PARAM
#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		95
#define	YYFLAG		-32768
#define	YYNTBASE	41

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 283 ? yytranslate[x] : 54)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      35,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      37,    40,    29,    27,    36,    28,     2,    30,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    17,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    38,     2,    39,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      31,    32,    33,    34
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     1,     4,     6,     9,    13,    19,    23,    26,
      29,    33,    36,    40,    42,    43,    45,    47,    49,    51,
      56,    60,    67,    70,    73,    76,    79,    83,    87,    91,
      95,    99,   102,   106,   110,   114,   118,   122,   126,   130,
     134,   137,   142,   143,   146,   151,   154,   155,   157,   159,
     164
};
static const short yyrhs[] =
{
      -1,    41,    42,     0,    35,     0,    45,    35,     0,    47,
      48,    49,     0,    47,    50,     8,    51,     9,     0,    52,
      46,    11,     0,    13,    35,     0,    12,    35,     0,    53,
      46,    16,     0,    14,     4,     0,    43,    36,    45,     0,
      45,     0,     0,    37,     0,     4,     0,     3,     0,     6,
       0,     4,    38,    45,    39,     0,     4,    17,    45,     0,
       4,    38,    45,    39,    17,    45,     0,    34,     4,     0,
       4,    34,     0,    33,     4,     0,     4,    33,     0,    37,
      45,    40,     0,    45,    27,    45,     0,    45,    28,    45,
       0,    45,    29,    45,     0,    45,    30,    45,     0,    28,
      45,     0,    45,    20,    45,     0,    45,    21,    45,     0,
      45,    22,    45,     0,    45,    23,    45,     0,    45,    25,
      45,     0,    45,    26,    45,     0,    45,    19,    45,     0,
      45,    18,    45,     0,    32,    45,     0,     5,    44,    43,
      40,     0,     0,    46,    42,     0,     7,    37,    45,    40,
       0,    46,     9,     0,     0,    46,     0,    46,     0,    10,
      37,    45,    40,     0,    15,     4,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,    60,    61,    64,    66,    67,    68,    69,    76,    85,
      94,    99,   109,   113,   117,   120,   123,   127,   131,   137,
     142,   147,   152,   157,   162,   167,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   197,   198,   201,   210,   218,   221,   229,   232,
     241
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "_NUMBER", "_VAR", "_FUNC", "_STRING", "_IF", 
  "_ELSE", "_ENDIF", "_WHILE", "_WEND", "_CONTINUE", "_BREAK", "_GOSUB", 
  "_SUB", "_ENDSUB", "'='", "_OR", "_AND", "_GT", "_GE", "_LT", "_LE", 
  "_LQ", "_EQ", "_NE", "'+'", "'-'", "'*'", "'/'", "UNARYMINUS", "_NOT", 
  "_INC", "_DEC", "'\\n'", "','", "'('", "'['", "']'", "')'", "list", 
  "stmt", "paramlist", "funcstart", "expr", "stlist", "if", "then1", 
  "else1", "then2", "else2", "while", "sub", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    41,    41,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    43,    43,    43,    44,    45,    45,    45,    45,
      45,    45,    45,    45,    45,    45,    45,    45,    45,    45,
      45,    45,    45,    45,    45,    45,    45,    45,    45,    45,
      45,    45,    46,    46,    47,    48,    49,    50,    51,    52,
      53
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     0,     2,     1,     2,     3,     5,     3,     2,     2,
       3,     2,     3,     1,     0,     1,     1,     1,     1,     4,
       3,     6,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     4,     0,     2,     4,     2,     0,     1,     1,     4,
       2
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       1,     0,    17,    16,     0,    18,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     3,     0,     2,     0,
      42,    42,    42,     0,    25,    23,     0,    15,    14,     0,
       0,     9,     8,    11,    50,    31,    40,    24,    22,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     4,    47,    46,     0,     0,     0,    20,     0,
       0,    13,     0,     0,    26,    39,    38,    32,    33,    34,
      35,    36,    37,    27,    28,    29,    30,    45,    43,     5,
      42,     7,    10,    19,     0,    41,    44,    49,    48,     0,
       0,    12,     6,    21,     0,     0
};

static const short yydefgoto[] =
{
       1,    78,    60,    28,    19,    53,    20,    54,    79,    55,
      89,    21,    22
};

static const short yypact[] =
{
  -32768,    42,-32768,   -13,   -28,-32768,   -25,   -24,   -20,   -16,
      18,    19,   195,   195,    20,    22,-32768,   195,-32768,   278,
  -32768,-32768,-32768,   195,-32768,-32768,   195,-32768,   195,   195,
     195,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   215,
     195,   195,   195,   195,   195,   195,   195,   195,   195,   195,
     195,   195,-32768,    77,-32768,    32,   112,   147,   296,   263,
     -30,   296,   231,   247,-32768,    74,   108,    35,    35,    35,
      35,    35,    35,   -22,   -22,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,    10,   195,-32768,-32768,-32768,   182,    34,
     195,   296,-32768,   296,    41,-32768
};

static const short yypgoto[] =
{
  -32768,    43,-32768,-32768,   -12,   -19,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768
};


#define	YYLAST		326


static const short yytable[] =
{
      35,    36,    56,    57,    23,    39,    84,    50,    51,    27,
      85,    58,    29,    30,    59,    31,    61,    62,    63,    32,
      24,    25,    33,    34,    37,    26,    38,    90,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      80,    95,    94,    92,    18,     2,     3,     4,     5,     6,
       0,     0,     7,     0,     8,     9,    10,    11,     0,     0,
       0,    88,    48,    49,    50,    51,     0,     0,     0,     0,
      12,     0,    91,     0,    13,    14,    15,    16,    93,    17,
       2,     3,     4,     5,     6,     0,    77,     7,     0,     8,
       9,    10,    11,    41,    42,    43,    44,    45,     0,    46,
      47,    48,    49,    50,    51,    12,     0,     0,     0,    13,
      14,    15,    16,     0,    17,     2,     3,     4,     5,     6,
       0,     0,     7,    81,     8,     9,    10,    11,    42,    43,
      44,    45,     0,    46,    47,    48,    49,    50,    51,     0,
      12,     0,     0,     0,    13,    14,    15,    16,     0,    17,
       2,     3,     4,     5,     6,     0,     0,     7,     0,     8,
       9,    10,    11,    82,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,     0,     0,     0,    13,
      14,    15,    16,     0,    17,     2,     3,     4,     5,     6,
       0,     0,     7,     0,     8,     9,    10,    11,     2,     3,
       4,     5,     0,     0,     0,     0,     0,     0,     0,     0,
      12,     0,     0,     0,    13,    14,    15,    16,     0,    17,
       0,     0,     0,    12,     0,     0,     0,    13,    14,    15,
       0,     0,    17,    40,    41,    42,    43,    44,    45,     0,
      46,    47,    48,    49,    50,    51,     0,     0,     0,    40,
      41,    42,    43,    44,    45,    64,    46,    47,    48,    49,
      50,    51,     0,     0,     0,    40,    41,    42,    43,    44,
      45,    86,    46,    47,    48,    49,    50,    51,     0,     0,
       0,    40,    41,    42,    43,    44,    45,    87,    46,    47,
      48,    49,    50,    51,     0,     0,    40,    41,    42,    43,
      44,    45,    83,    46,    47,    48,    49,    50,    51,     0,
       0,     0,     0,    52,    40,    41,    42,    43,    44,    45,
       0,    46,    47,    48,    49,    50,    51
};

static const short yycheck[] =
{
      12,    13,    21,    22,    17,    17,    36,    29,    30,    37,
      40,    23,    37,    37,    26,    35,    28,    29,    30,    35,
      33,    34,     4,     4,     4,    38,     4,    17,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
       8,     0,     0,     9,     1,     3,     4,     5,     6,     7,
      -1,    -1,    10,    -1,    12,    13,    14,    15,    -1,    -1,
      -1,    80,    27,    28,    29,    30,    -1,    -1,    -1,    -1,
      28,    -1,    84,    -1,    32,    33,    34,    35,    90,    37,
       3,     4,     5,     6,     7,    -1,     9,    10,    -1,    12,
      13,    14,    15,    19,    20,    21,    22,    23,    -1,    25,
      26,    27,    28,    29,    30,    28,    -1,    -1,    -1,    32,
      33,    34,    35,    -1,    37,     3,     4,     5,     6,     7,
      -1,    -1,    10,    11,    12,    13,    14,    15,    20,    21,
      22,    23,    -1,    25,    26,    27,    28,    29,    30,    -1,
      28,    -1,    -1,    -1,    32,    33,    34,    35,    -1,    37,
       3,     4,     5,     6,     7,    -1,    -1,    10,    -1,    12,
      13,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    32,
      33,    34,    35,    -1,    37,     3,     4,     5,     6,     7,
      -1,    -1,    10,    -1,    12,    13,    14,    15,     3,     4,
       5,     6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    32,    33,    34,    35,    -1,    37,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    32,    33,    34,
      -1,    -1,    37,    18,    19,    20,    21,    22,    23,    -1,
      25,    26,    27,    28,    29,    30,    -1,    -1,    -1,    18,
      19,    20,    21,    22,    23,    40,    25,    26,    27,    28,
      29,    30,    -1,    -1,    -1,    18,    19,    20,    21,    22,
      23,    40,    25,    26,    27,    28,    29,    30,    -1,    -1,
      -1,    18,    19,    20,    21,    22,    23,    40,    25,    26,
      27,    28,    29,    30,    -1,    -1,    18,    19,    20,    21,
      22,    23,    39,    25,    26,    27,    28,    29,    30,    -1,
      -1,    -1,    -1,    35,    18,    19,    20,    21,    22,    23,
      -1,    25,    26,    27,    28,    29,    30
};
#define YYPURE 1

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "bison.scripts"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif

#line 315 "bison.scripts"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
Parser::yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 3:
#line 65 "parser_grammar.y"
{ yyval.ptr=code.GetPosition(); ;
    break;}
case 4:
#line 66 "parser_grammar.y"
{ yyval.ptr=yyvsp[-1].ptr; AddCode(opPOP); ;
    break;}
case 5:
#line 67 "parser_grammar.y"
{ yyval.ptr=yyvsp[-2].ptr; SolveIfFrame(yyvsp[0].ptr,code.GetPosition()); ;
    break;}
case 6:
#line 68 "parser_grammar.y"
{ yyval.ptr=yyvsp[-4].ptr; SolveIfFrame(yyvsp[-1].ptr,code.GetPosition()); ;
    break;}
case 7:
#line 69 "parser_grammar.y"
{
                                          yyval.ptr=yyvsp[-2].ptr;
                                          AddCode(opJMP);
                                          AddXRef(&pLoopFrame->Cond,code.GetPosition());
                                          AddCode(0);
                                          SolveLoopFrame(yyvsp[-1].ptr,yyvsp[-2].ptr,code.GetPosition());
                                        ;
    break;}
case 8:
#line 76 "parser_grammar.y"
{
                                          yyval.ptr=code.GetPosition();
                                          if(pLoopFrame)
                                          {
                                            AddCode(opJMP);
                                            AddXRef(&pLoopFrame->Follow,code.GetPosition());
                                            AddCode(0);
                                          }
                                        ;
    break;}
case 9:
#line 85 "parser_grammar.y"
{
                                          yyval.ptr=code.GetPosition();
                                          if(pLoopFrame)
                                          {
                                            AddCode(opJMP);
                                            AddXRef(&pLoopFrame->Cond,code.GetPosition());
                                            AddCode(0);
                                          }
                                        ;
    break;}
case 10:
#line 94 "parser_grammar.y"
{
                                          yyval.ptr=yyvsp[-2].ptr;
                                          AddCode(opRET);
                                          SolveIfFrame(code.GetPosition(),code.GetPosition());
                                        ;
    break;}
case 11:
#line 99 "parser_grammar.y"
{
                                          yyval.ptr=code.GetPosition();
                                          if(symbols[yyvsp[0].variable].Value().getcodepos())
                                          {
                                            AddCode(opCALL);
                                            AddCode(symbols[yyvsp[0].variable].Value().getcodepos());
                                          }
                                        ;
    break;}
case 12:
#line 109 "parser_grammar.y"
{
                                          yyval.ptr=yyvsp[-2].ptr;
                                          (pFunctionFrame->Count)++;
                                        ;
    break;}
case 13:
#line 113 "parser_grammar.y"
{
                                          yyval.ptr=yyvsp[0].ptr;
                                          (pFunctionFrame->Count)++;
                                        ;
    break;}
case 14:
#line 117 "parser_grammar.y"
{ yyval.ptr=code.GetPosition(); ;
    break;}
case 15:
#line 120 "parser_grammar.y"
{ CreateFunctionFrame(); ;
    break;}
case 16:
#line 123 "parser_grammar.y"
{
                                          yyval.ptr=AddCode(opPUSHV);
                                          AddCode(yyvsp[0].variable);
                                        ;
    break;}
case 17:
#line 127 "parser_grammar.y"
{
                                          yyval.ptr=AddCode(opPUSHD);
                                          AddInt64(yyvsp[0].number);
                                        ;
    break;}
case 18:
#line 131 "parser_grammar.y"
{
                                          yyval.ptr=AddCode(opPUSHD);
                                          temp_string=yyvsp[0].string;
                                          HeapFree(heap,0,yyvsp[0].string);
                                          AddString(temp_string);
                                        ;
    break;}
case 19:
#line 137 "parser_grammar.y"
{
                                          yyval.ptr=yyvsp[-1].ptr;
                                          AddCode(opPUSHS);
                                          AddCode(yyvsp[-3].variable);
                                        ;
    break;}
case 20:
#line 142 "parser_grammar.y"
{
                                          AddCode(opMOV);
                                          AddCode(yyvsp[-2].variable);
                                          yyval.ptr=yyvsp[0].ptr;
                                        ;
    break;}
case 21:
#line 147 "parser_grammar.y"
{
                                          yyval.ptr=yyvsp[-3].ptr;
                                          AddCode(opMOVS);
                                          AddCode(yyvsp[-5].variable);
                                        ;
    break;}
case 22:
#line 152 "parser_grammar.y"
{
                                          yyval.ptr=code.GetPosition();
                                          AddCode(opDEC);
                                          AddCode(yyvsp[0].variable);
                                        ;
    break;}
case 23:
#line 157 "parser_grammar.y"
{
                                          yyval.ptr=code.GetPosition();
                                          AddCode(opPOSTDEC);
                                          AddCode(yyvsp[-1].variable);
                                        ;
    break;}
case 24:
#line 162 "parser_grammar.y"
{
                                          yyval.ptr=code.GetPosition();
                                          AddCode(opINC);
                                          AddCode(yyvsp[0].variable);
                                        ;
    break;}
case 25:
#line 167 "parser_grammar.y"
{
                                          yyval.ptr=code.GetPosition();
                                          AddCode(opPOSTINC);
                                          AddCode(yyvsp[-1].variable);
                                        ;
    break;}
case 26:
#line 172 "parser_grammar.y"
{ yyval.ptr=yyvsp[-1].ptr; ;
    break;}
case 27:
#line 173 "parser_grammar.y"
{ yyval.ptr=yyvsp[-2].ptr; AddCode(opADD); ;
    break;}
case 28:
#line 174 "parser_grammar.y"
{ yyval.ptr=yyvsp[-2].ptr; AddCode(opSUB); ;
    break;}
case 29:
#line 175 "parser_grammar.y"
{ yyval.ptr=yyvsp[-2].ptr; AddCode(opMUL); ;
    break;}
case 30:
#line 176 "parser_grammar.y"
{ yyval.ptr=yyvsp[-2].ptr; AddCode(opDIV); ;
    break;}
case 31:
#line 177 "parser_grammar.y"
{ yyval.ptr=yyvsp[0].ptr; AddCode(opNEG);;
    break;}
case 32:
#line 178 "parser_grammar.y"
{ yyval.ptr=yyvsp[-2].ptr; AddCode(opGT); ;
    break;}
case 33:
#line 179 "parser_grammar.y"
{ yyval.ptr=yyvsp[-2].ptr; AddCode(opGE); ;
    break;}
case 34:
#line 180 "parser_grammar.y"
{ yyval.ptr=yyvsp[-2].ptr; AddCode(opLT); ;
    break;}
case 35:
#line 181 "parser_grammar.y"
{ yyval.ptr=yyvsp[-2].ptr; AddCode(opLE); ;
    break;}
case 36:
#line 182 "parser_grammar.y"
{ yyval.ptr=yyvsp[-2].ptr; AddCode(opEQ); ;
    break;}
case 37:
#line 183 "parser_grammar.y"
{ yyval.ptr=yyvsp[-2].ptr; AddCode(opNE); ;
    break;}
case 38:
#line 184 "parser_grammar.y"
{ yyval.ptr=yyvsp[-2].ptr; AddCode(opAND); ;
    break;}
case 39:
#line 185 "parser_grammar.y"
{ yyval.ptr=yyvsp[-2].ptr; AddCode(opOR); ;
    break;}
case 40:
#line 186 "parser_grammar.y"
{ yyval.ptr=yyvsp[0].ptr; AddCode(opNOT); ;
    break;}
case 41:
#line 187 "parser_grammar.y"
{
                                          yyval.ptr=yyvsp[-1].ptr;
                                          AddCode(opPUSHD);
                                          AddInt64(pFunctionFrame->Count);
                                          SolveFunctionFrame();
                                          AddCode(opFUNC);
                                          AddCode(yyvsp[-3].variable);
                                        ;
    break;}
case 42:
#line 197 "parser_grammar.y"
{ yyval.ptr=code.GetPosition(); ;
    break;}
case 44:
#line 201 "parser_grammar.y"
{
                                          CreateIfFrame();
                                          AddCode(opJE);
                                          AddXRef(&pIfFrame->Other,code.GetPosition());
                                          AddCode(0);
                                          yyval.ptr=yyvsp[-1].ptr;
                                        ;
    break;}
case 45:
#line 210 "parser_grammar.y"
{
                                          yyval.ptr=yyvsp[-1].ptr;
                                          AddCode(opJMP);
                                          AddXRef(&pIfFrame->Follow,code.GetPosition());
                                          AddCode(0);
                                        ;
    break;}
case 46:
#line 218 "parser_grammar.y"
{ yyval.ptr=code.GetPosition(); ;
    break;}
case 47:
#line 221 "parser_grammar.y"
{
                                          yyval.ptr=yyvsp[0].ptr;
                                          AddCode(opJMP);
                                          AddXRef(&pIfFrame->Follow,code.GetPosition());
                                          AddCode(0);
                                        ;
    break;}
case 48:
#line 229 "parser_grammar.y"
{ yyval.ptr=yyvsp[0].ptr; ;
    break;}
case 49:
#line 232 "parser_grammar.y"
{
                                          CreateLoopFrame();
                                          AddCode(opJE);
                                          AddXRef(&pLoopFrame->Follow,code.GetPosition());
                                          AddCode(0);
                                          yyval.ptr=yyvsp[-1].ptr;
                                        ;
    break;}
case 50:
#line 241 "parser_grammar.y"
{
                                          yyval.ptr=code.GetPosition();
                                          CreateIfFrame();
                                          AddCode(opJMP);
                                          AddXRef(&pIfFrame->Other,code.GetPosition());
                                          AddCode(0);
                                          symbols[yyvsp[0].variable].Value().setcodepos(code.GetPosition());
                                        ;
    break;}
}

#line 705 "bison.scripts"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror (SErr_Parser);
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
yyerrhandle:
  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror (SErr_Stack);
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 251 "parser_grammar.y"

void Parser::yyerror(int msg)
{
  error_index=msg;
  return;
}
