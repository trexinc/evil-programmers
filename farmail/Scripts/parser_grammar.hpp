#ifndef BISON_PARSER_GRAMMAR_HPP
# define BISON_PARSER_GRAMMAR_HPP

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


#endif /* not BISON_PARSER_GRAMMAR_HPP */
