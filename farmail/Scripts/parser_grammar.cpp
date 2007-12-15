/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "scripts.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0



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




/* Copy the first part of user declarations.  */
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


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 30 "parser_grammar.y"
{
  __INT64 number; //numbers
  long variable; //variables, functions, custom functions
  long ptr; //internal use
  char *string; //strings
}
/* Line 193 of yacc.c.  */
#line 189 "parser_grammar.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */
#line 54 "parser_grammar.y"

#define YYPARSE_PARAM


/* Line 216 of yacc.c.  */
#line 205 "parser_grammar.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
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
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   329

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  41
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  14
/* YYNRULES -- Number of rules.  */
#define YYNRULES  51
/* YYNRULES -- Number of states.  */
#define YYNSTATES  95

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   284

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    31,    32,    33,    34
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    12,    16,    22,    26,
      29,    32,    36,    39,    43,    45,    46,    48,    50,    52,
      54,    59,    63,    70,    73,    76,    79,    82,    86,    90,
      94,    98,   102,   105,   109,   113,   117,   121,   125,   129,
     133,   137,   140,   145,   146,   149,   154,   157,   158,   160,
     162,   167
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      42,     0,    -1,    -1,    42,    43,    -1,    35,    -1,    46,
      35,    -1,    48,    49,    50,    -1,    48,    51,     8,    52,
       9,    -1,    53,    47,    11,    -1,    13,    35,    -1,    12,
      35,    -1,    54,    47,    16,    -1,    14,     4,    -1,    44,
      36,    46,    -1,    46,    -1,    -1,    37,    -1,     4,    -1,
       3,    -1,     6,    -1,     4,    38,    46,    39,    -1,     4,
      17,    46,    -1,     4,    38,    46,    39,    17,    46,    -1,
      31,     4,    -1,     4,    31,    -1,    32,     4,    -1,     4,
      32,    -1,    37,    46,    40,    -1,    46,    27,    46,    -1,
      46,    28,    46,    -1,    46,    29,    46,    -1,    46,    30,
      46,    -1,    28,    46,    -1,    46,    26,    46,    -1,    46,
      25,    46,    -1,    46,    24,    46,    -1,    46,    23,    46,
      -1,    46,    21,    46,    -1,    46,    20,    46,    -1,    46,
      19,    46,    -1,    46,    18,    46,    -1,    33,    46,    -1,
       5,    45,    44,    40,    -1,    -1,    47,    43,    -1,     7,
      37,    46,    40,    -1,    47,     9,    -1,    -1,    47,    -1,
      47,    -1,    10,    37,    46,    40,    -1,    15,     4,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    60,    60,    61,    65,    66,    67,    68,    69,    76,
      85,    94,    99,   109,   113,   117,   120,   123,   127,   131,
     137,   142,   147,   152,   157,   162,   167,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   197,   198,   201,   210,   218,   221,   229,
     232,   241
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "_NUMBER", "_VAR", "_FUNC", "_STRING",
  "_IF", "_ELSE", "_ENDIF", "_WHILE", "_WEND", "_CONTINUE", "_BREAK",
  "_GOSUB", "_SUB", "_ENDSUB", "'='", "_OR", "_AND", "_NE", "_EQ", "_LQ",
  "_LE", "_LT", "_GE", "_GT", "'+'", "'-'", "'*'", "'/'", "_DEC", "_INC",
  "_NOT", "UNARYMINUS", "'\\n'", "','", "'('", "'['", "']'", "')'",
  "$accept", "list", "stmt", "paramlist", "funcstart", "expr", "stlist",
  "if", "then1", "else1", "then2", "else2", "while", "sub", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,    61,   272,   273,
     274,   275,   276,   277,   278,   279,   280,    43,    45,    42,
      47,   281,   282,   283,   284,    10,    44,    40,    91,    93,
      41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    41,    42,    42,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    44,    44,    44,    45,    46,    46,    46,
      46,    46,    46,    46,    46,    46,    46,    46,    46,    46,
      46,    46,    46,    46,    46,    46,    46,    46,    46,    46,
      46,    46,    46,    47,    47,    48,    49,    50,    51,    52,
      53,    54
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     2,     3,     5,     3,     2,
       2,     3,     2,     3,     1,     0,     1,     1,     1,     1,
       4,     3,     6,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     4,     0,     2,     4,     2,     0,     1,     1,
       4,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,    18,    17,     0,    19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     4,     0,     3,
       0,    43,    43,    43,     0,    24,    26,     0,    16,    15,
       0,     0,    10,     9,    12,    51,    32,    23,    25,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     5,    48,    47,     0,     0,     0,    21,
       0,     0,    14,     0,     0,    27,    40,    39,    38,    37,
      36,    35,    34,    33,    28,    29,    30,    31,    46,    44,
       6,    43,     8,    11,    20,     0,    42,    45,    50,    49,
       0,     0,    13,     7,    22
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,    79,    61,    29,    20,    54,    21,    55,    80,
      56,    90,    22,    23
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -33
static const yytype_int16 yypact[] =
{
     -33,    42,   -33,   -33,   -11,   -30,   -33,   -25,   -24,   -16,
      -9,    11,    36,   195,    37,    39,   195,   -33,   195,   -33,
     286,   -33,   -33,   -33,   195,   -33,   -33,   195,   -33,   195,
     195,   195,   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,
     215,   195,   195,   195,   195,   195,   195,   195,   195,   195,
     195,   195,   195,   -33,    77,   -33,    43,   112,   147,   299,
     269,   -32,   299,   233,   251,   -33,    74,    38,    -5,    -5,
      -5,    -5,    -5,    -5,   -20,   -20,   -33,   -33,   -33,   -33,
     -33,   -33,   -33,   -33,    27,   195,   -33,   -33,   -33,   182,
      41,   195,   299,   -33,   299
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -33,   -33,    52,   -33,   -33,   -13,   -21,   -33,   -33,   -33,
     -33,   -33,   -33,   -33
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      36,    57,    58,    39,    85,    40,    24,    28,    86,    51,
      52,    59,    30,    31,    60,    34,    62,    63,    64,    32,
      25,    26,    49,    50,    51,    52,    33,    27,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      35,    37,     2,    38,    91,     3,     4,     5,     6,     7,
      93,    81,     8,    19,     9,    10,    11,    12,    43,    44,
      89,    45,    46,    47,    48,    49,    50,    51,    52,     0,
      13,     0,    92,    14,    15,    16,     0,    17,    94,    18,
       3,     4,     5,     6,     7,     0,    78,     8,     0,     9,
      10,    11,    12,    42,    43,    44,     0,    45,    46,    47,
      48,    49,    50,    51,    52,    13,     0,     0,    14,    15,
      16,     0,    17,     0,    18,     3,     4,     5,     6,     7,
       0,     0,     8,    82,     9,    10,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,     0,     0,    14,    15,    16,     0,    17,     0,    18,
       3,     4,     5,     6,     7,     0,     0,     8,     0,     9,
      10,    11,    12,    83,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,     0,     0,    14,    15,
      16,     0,    17,     0,    18,     3,     4,     5,     6,     7,
       0,     0,     8,     0,     9,    10,    11,    12,     3,     4,
       5,     6,     0,     0,     0,     0,     0,     0,     0,     0,
      13,     0,     0,    14,    15,    16,     0,    17,     0,    18,
       0,     0,     0,    13,     0,     0,    14,    15,    16,     0,
       0,     0,    18,    41,    42,    43,    44,     0,    45,    46,
      47,    48,    49,    50,    51,    52,     0,     0,     0,     0,
       0,    41,    42,    43,    44,    65,    45,    46,    47,    48,
      49,    50,    51,    52,     0,     0,     0,     0,     0,    41,
      42,    43,    44,    87,    45,    46,    47,    48,    49,    50,
      51,    52,     0,     0,     0,     0,     0,    41,    42,    43,
      44,    88,    45,    46,    47,    48,    49,    50,    51,    52,
       0,     0,     0,     0,    41,    42,    43,    44,    84,    45,
      46,    47,    48,    49,    50,    51,    52,    41,    42,    43,
      44,    53,    45,    46,    47,    48,    49,    50,    51,    52
};

static const yytype_int8 yycheck[] =
{
      13,    22,    23,    16,    36,    18,    17,    37,    40,    29,
      30,    24,    37,    37,    27,     4,    29,    30,    31,    35,
      31,    32,    27,    28,    29,    30,    35,    38,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
       4,     4,     0,     4,    17,     3,     4,     5,     6,     7,
       9,     8,    10,     1,    12,    13,    14,    15,    20,    21,
      81,    23,    24,    25,    26,    27,    28,    29,    30,    -1,
      28,    -1,    85,    31,    32,    33,    -1,    35,    91,    37,
       3,     4,     5,     6,     7,    -1,     9,    10,    -1,    12,
      13,    14,    15,    19,    20,    21,    -1,    23,    24,    25,
      26,    27,    28,    29,    30,    28,    -1,    -1,    31,    32,
      33,    -1,    35,    -1,    37,     3,     4,     5,     6,     7,
      -1,    -1,    10,    11,    12,    13,    14,    15,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    31,    32,    33,    -1,    35,    -1,    37,
       3,     4,     5,     6,     7,    -1,    -1,    10,    -1,    12,
      13,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    31,    32,
      33,    -1,    35,    -1,    37,     3,     4,     5,     6,     7,
      -1,    -1,    10,    -1,    12,    13,    14,    15,     3,     4,
       5,     6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    31,    32,    33,    -1,    35,    -1,    37,
      -1,    -1,    -1,    28,    -1,    -1,    31,    32,    33,    -1,
      -1,    -1,    37,    18,    19,    20,    21,    -1,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    -1,    -1,    -1,
      -1,    18,    19,    20,    21,    40,    23,    24,    25,    26,
      27,    28,    29,    30,    -1,    -1,    -1,    -1,    -1,    18,
      19,    20,    21,    40,    23,    24,    25,    26,    27,    28,
      29,    30,    -1,    -1,    -1,    -1,    -1,    18,    19,    20,
      21,    40,    23,    24,    25,    26,    27,    28,    29,    30,
      -1,    -1,    -1,    -1,    18,    19,    20,    21,    39,    23,
      24,    25,    26,    27,    28,    29,    30,    18,    19,    20,
      21,    35,    23,    24,    25,    26,    27,    28,    29,    30
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    42,     0,     3,     4,     5,     6,     7,    10,    12,
      13,    14,    15,    28,    31,    32,    33,    35,    37,    43,
      46,    48,    53,    54,    17,    31,    32,    38,    37,    45,
      37,    37,    35,    35,     4,     4,    46,     4,     4,    46,
      46,    18,    19,    20,    21,    23,    24,    25,    26,    27,
      28,    29,    30,    35,    47,    49,    51,    47,    47,    46,
      46,    44,    46,    46,    46,    40,    46,    46,    46,    46,
      46,    46,    46,    46,    46,    46,    46,    46,     9,    43,
      50,     8,    11,    16,    39,    36,    40,    40,    40,    47,
      52,    17,    46,     9,    46
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


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
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

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
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
Parser::yyparse (void *YYPARSE_PARAM)
#else
int
Parser::yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  /* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

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

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:
#line 65 "parser_grammar.y"
    { (yyval.ptr)=code.GetPosition(); ;}
    break;

  case 5:
#line 66 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (2)].ptr); AddCode(opPOP); ;}
    break;

  case 6:
#line 67 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (3)].ptr); SolveIfFrame((yyvsp[(3) - (3)].ptr),code.GetPosition()); ;}
    break;

  case 7:
#line 68 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (5)].ptr); SolveIfFrame((yyvsp[(4) - (5)].ptr),code.GetPosition()); ;}
    break;

  case 8:
#line 69 "parser_grammar.y"
    {
                                          (yyval.ptr)=(yyvsp[(1) - (3)].ptr);
                                          AddCode(opJMP);
                                          AddXRef(&pLoopFrame->Cond,code.GetPosition());
                                          AddCode(0);
                                          SolveLoopFrame((yyvsp[(2) - (3)].ptr),(yyvsp[(1) - (3)].ptr),code.GetPosition());
                                        ;}
    break;

  case 9:
#line 76 "parser_grammar.y"
    {
                                          (yyval.ptr)=code.GetPosition();
                                          if(pLoopFrame)
                                          {
                                            AddCode(opJMP);
                                            AddXRef(&pLoopFrame->Follow,code.GetPosition());
                                            AddCode(0);
                                          }
                                        ;}
    break;

  case 10:
#line 85 "parser_grammar.y"
    {
                                          (yyval.ptr)=code.GetPosition();
                                          if(pLoopFrame)
                                          {
                                            AddCode(opJMP);
                                            AddXRef(&pLoopFrame->Cond,code.GetPosition());
                                            AddCode(0);
                                          }
                                        ;}
    break;

  case 11:
#line 94 "parser_grammar.y"
    {
                                          (yyval.ptr)=(yyvsp[(1) - (3)].ptr);
                                          AddCode(opRET);
                                          SolveIfFrame(code.GetPosition(),code.GetPosition());
                                        ;}
    break;

  case 12:
#line 99 "parser_grammar.y"
    {
                                          (yyval.ptr)=code.GetPosition();
                                          if(symbols[(yyvsp[(2) - (2)].variable)].Value().getcodepos())
                                          {
                                            AddCode(opCALL);
                                            AddCode(symbols[(yyvsp[(2) - (2)].variable)].Value().getcodepos());
                                          }
                                        ;}
    break;

  case 13:
#line 109 "parser_grammar.y"
    {
                                          (yyval.ptr)=(yyvsp[(1) - (3)].ptr);
                                          (pFunctionFrame->Count)++;
                                        ;}
    break;

  case 14:
#line 113 "parser_grammar.y"
    {
                                          (yyval.ptr)=(yyvsp[(1) - (1)].ptr);
                                          (pFunctionFrame->Count)++;
                                        ;}
    break;

  case 15:
#line 117 "parser_grammar.y"
    { (yyval.ptr)=code.GetPosition(); ;}
    break;

  case 16:
#line 120 "parser_grammar.y"
    { CreateFunctionFrame(); ;}
    break;

  case 17:
#line 123 "parser_grammar.y"
    {
                                          (yyval.ptr)=AddCode(opPUSHV);
                                          AddCode((yyvsp[(1) - (1)].variable));
                                        ;}
    break;

  case 18:
#line 127 "parser_grammar.y"
    {
                                          (yyval.ptr)=AddCode(opPUSHD);
                                          AddInt64((yyvsp[(1) - (1)].number));
                                        ;}
    break;

  case 19:
#line 131 "parser_grammar.y"
    {
                                          (yyval.ptr)=AddCode(opPUSHD);
                                          temp_string=(yyvsp[(1) - (1)].string);
                                          HeapFree(heap,0,(yyvsp[(1) - (1)].string));
                                          AddString(temp_string);
                                        ;}
    break;

  case 20:
#line 137 "parser_grammar.y"
    {
                                          (yyval.ptr)=(yyvsp[(3) - (4)].ptr);
                                          AddCode(opPUSHS);
                                          AddCode((yyvsp[(1) - (4)].variable));
                                        ;}
    break;

  case 21:
#line 142 "parser_grammar.y"
    {
                                          AddCode(opMOV);
                                          AddCode((yyvsp[(1) - (3)].variable));
                                          (yyval.ptr)=(yyvsp[(3) - (3)].ptr);
                                        ;}
    break;

  case 22:
#line 147 "parser_grammar.y"
    {
                                          (yyval.ptr)=(yyvsp[(3) - (6)].ptr);
                                          AddCode(opMOVS);
                                          AddCode((yyvsp[(1) - (6)].variable));
                                        ;}
    break;

  case 23:
#line 152 "parser_grammar.y"
    {
                                          (yyval.ptr)=code.GetPosition();
                                          AddCode(opDEC);
                                          AddCode((yyvsp[(2) - (2)].variable));
                                        ;}
    break;

  case 24:
#line 157 "parser_grammar.y"
    {
                                          (yyval.ptr)=code.GetPosition();
                                          AddCode(opPOSTDEC);
                                          AddCode((yyvsp[(1) - (2)].variable));
                                        ;}
    break;

  case 25:
#line 162 "parser_grammar.y"
    {
                                          (yyval.ptr)=code.GetPosition();
                                          AddCode(opINC);
                                          AddCode((yyvsp[(2) - (2)].variable));
                                        ;}
    break;

  case 26:
#line 167 "parser_grammar.y"
    {
                                          (yyval.ptr)=code.GetPosition();
                                          AddCode(opPOSTINC);
                                          AddCode((yyvsp[(1) - (2)].variable));
                                        ;}
    break;

  case 27:
#line 172 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(2) - (3)].ptr); ;}
    break;

  case 28:
#line 173 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (3)].ptr); AddCode(opADD); ;}
    break;

  case 29:
#line 174 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (3)].ptr); AddCode(opSUB); ;}
    break;

  case 30:
#line 175 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (3)].ptr); AddCode(opMUL); ;}
    break;

  case 31:
#line 176 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (3)].ptr); AddCode(opDIV); ;}
    break;

  case 32:
#line 177 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(2) - (2)].ptr); AddCode(opNEG);;}
    break;

  case 33:
#line 178 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (3)].ptr); AddCode(opGT); ;}
    break;

  case 34:
#line 179 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (3)].ptr); AddCode(opGE); ;}
    break;

  case 35:
#line 180 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (3)].ptr); AddCode(opLT); ;}
    break;

  case 36:
#line 181 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (3)].ptr); AddCode(opLE); ;}
    break;

  case 37:
#line 182 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (3)].ptr); AddCode(opEQ); ;}
    break;

  case 38:
#line 183 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (3)].ptr); AddCode(opNE); ;}
    break;

  case 39:
#line 184 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (3)].ptr); AddCode(opAND); ;}
    break;

  case 40:
#line 185 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (3)].ptr); AddCode(opOR); ;}
    break;

  case 41:
#line 186 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(2) - (2)].ptr); AddCode(opNOT); ;}
    break;

  case 42:
#line 187 "parser_grammar.y"
    {
                                          (yyval.ptr)=(yyvsp[(3) - (4)].ptr);
                                          AddCode(opPUSHD);
                                          AddInt64(pFunctionFrame->Count);
                                          SolveFunctionFrame();
                                          AddCode(opFUNC);
                                          AddCode((yyvsp[(1) - (4)].variable));
                                        ;}
    break;

  case 43:
#line 197 "parser_grammar.y"
    { (yyval.ptr)=code.GetPosition(); ;}
    break;

  case 45:
#line 201 "parser_grammar.y"
    {
                                          CreateIfFrame();
                                          AddCode(opJE);
                                          AddXRef(&pIfFrame->Other,code.GetPosition());
                                          AddCode(0);
                                          (yyval.ptr)=(yyvsp[(3) - (4)].ptr);
                                        ;}
    break;

  case 46:
#line 210 "parser_grammar.y"
    {
                                          (yyval.ptr)=(yyvsp[(1) - (2)].ptr);
                                          AddCode(opJMP);
                                          AddXRef(&pIfFrame->Follow,code.GetPosition());
                                          AddCode(0);
                                        ;}
    break;

  case 47:
#line 218 "parser_grammar.y"
    { (yyval.ptr)=code.GetPosition(); ;}
    break;

  case 48:
#line 221 "parser_grammar.y"
    {
                                          (yyval.ptr)=(yyvsp[(1) - (1)].ptr);
                                          AddCode(opJMP);
                                          AddXRef(&pIfFrame->Follow,code.GetPosition());
                                          AddCode(0);
                                        ;}
    break;

  case 49:
#line 229 "parser_grammar.y"
    { (yyval.ptr)=(yyvsp[(1) - (1)].ptr); ;}
    break;

  case 50:
#line 232 "parser_grammar.y"
    {
                                          CreateLoopFrame();
                                          AddCode(opJE);
                                          AddXRef(&pLoopFrame->Follow,code.GetPosition());
                                          AddCode(0);
                                          (yyval.ptr)=(yyvsp[(3) - (4)].ptr);
                                        ;}
    break;

  case 51:
#line 241 "parser_grammar.y"
    {
                                          (yyval.ptr)=code.GetPosition();
                                          CreateIfFrame();
                                          AddCode(opJMP);
                                          AddXRef(&pIfFrame->Other,code.GetPosition());
                                          AddCode(0);
                                          symbols[(yyvsp[(2) - (2)].variable)].Value().setcodepos(code.GetPosition());
                                        ;}
    break;


/* Line 1267 of yacc.c.  */
#line 1874 "parser_grammar.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (SErr_Parser);
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

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

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (SErr_Stack);
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 251 "parser_grammar.y"

void Parser::yyerror(int msg)
{
  error_index=msg;
  return;
}

