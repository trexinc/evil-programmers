/*
  Copyright (C) 2000 Konstantin Stupnik

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

  Regular expressions support library.
  Syntax and semantics of regexps very close to
  syntax and semantics of perl regexps.
*/

#ifndef REGEXP_HPP
#define REGEXP_HPP

#ifndef __cplusplus
#error "RegExp.hpp is for C++ only"
#endif

#ifdef RE_SPINOZA_MODE
  #define RE_EXTERNAL_CTYPE
  #define NAMEDBRACKETS
  #define RE_NO_STRING_H
  #define RE_NO_NEWARRAY
#endif

#if defined(NAMEDBRACKETS) || defined(RELIB)
#if defined(RE_SPINOZA_MODE)
  #ifdef __BORLANDC__
    #include "CHash/CHash.cpp"
  #else
    #include "../CHash/CHash.cpp"
  #endif
  #define Hash    CHash
  #define SetItem setitem
  #define GetPtr  getitem
#else
  #include "Hash.hpp"
#endif
#ifdef RELIB
#include "List.hpp"
#endif
#endif

namespace XClasses{

#ifdef UNICODE
typedef unsigned short rechar;
typedef rechar* prechar;
typedef unsigned short RECHAR;
#else
//! Used to avoid problems when mapping string chars
typedef unsigned char rechar;
//! Used to avoid problems when mapping string chars
typedef rechar* prechar;
typedef char RECHAR;
#endif

//! Possible compile and runtime errors returned by LastError.
enum REError{
  //! No errors
  errNone=0,
  //! RegExp wasn't even tried to compile
  errNotCompiled,
  //! expression contain syntax error
  errSyntax,
  //! Unbalanced brackets
  errBrackets,
  //! Max recursive brackets level reached. Controled in compile time
  errMaxDepth,
  //! Invalid options combination
  errOptions,
  //! Reference to nonexistent bracket
  errInvalidBackRef,
  //! Invalid escape char
  errInvalidEscape,
  //! Invalid range value
  errInvalidRange,
  //! Quantifier applyed to invalid object. f.e. lookahed assertion
  errInvalidQuantifiersCombination,
  //! Size of match array isn't large enough.
  errNotEnoughMatches,
  //! Attempt to match RegExp with Named Brackets, and no storage class provided.
  errNoStorageForNB,
  //! Reference to undefined named bracket
  errReferenceToUndefinedNamedBracket,
  //! Only fixed length look behind assertions are supported
  errVariableLengthLookBehind,
};

//! Used internally
struct REOpCode;
//! Used internally
typedef REOpCode *PREOpCode;

//! Max brackets depth can be redefined in compile time
#ifndef MAXDEPTH
static const int MAXDEPTH=256;
#endif

/**
  \defgroup options Regular expression compile time options
*/
/*@{*/
//! Match in a case insensetive manner
static const int OP_IGNORECASE   =0x0001;
//! Single line mode, dot metacharacter will match newline symbol
static const int OP_SINGLELINE   =0x0002;
//! Multiline mode, ^ and $ can match line start and line end
static const int OP_MULTILINE    =0x0004;
//! Extended syntax, spaces symbols are ignored unless escaped
static const int OP_XTENDEDSYNTAX=0x0008;
//! Perl style regexp provided. i.e. /expression/imsx
static const int OP_PERLSTYLE    =0x0010;
//! Optimize after compile
static const int OP_OPTIMIZE     =0x0020;
//! Strict escapes - only unrecognized escape will prodce errInvalidEscape error
static const int OP_STRICT       =0x0040;
//! Replace backslash with slash, used
//! when regexp source embeded in c++ sources
static const int OP_CPPMODE      =0x0080;
/*@}*/


/**
 \defgroup localebits Locale Info bits

*/
/*@{*/
//! Digits
static const int TYPE_DIGITCHAR  =0x01;
//! space, newlines tab etc
static const int TYPE_SPACECHAR  =0x02;
//! alphanumeric and _
static const int TYPE_WORDCHAR   =0x04;
//! lowcase symbol
static const int TYPE_LOWCASE    =0x08;
//! upcase symbol
static const int TYPE_UPCASE     =0x10;
//! letter
static const int TYPE_ALPHACHAR  =0x20;
/*@}*/

/**
  \defgroup brhactions Bracket handler actions

*/
/*@{*/
//! Matched Closing bracket
static const int bhMatch=1;
//! Bracket rollback
static const int bhRollBack=2;

//! default preallocated stack size, and stack page size
#ifndef STACK_PAGE_SIZE
static const int STACK_PAGE_SIZE=16;
#endif

#ifdef RE_STATIC_LOCALE
#define LOCALEDEF static
#else
#define LOCALEDEF
#endif

#ifdef RE_SPINOZA_MODE
  #if defined(__BORLANDC__)
    #pragma option -a2
  #elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
    #pragma pack(2)
  #else
    #pragma pack(push,2)
  #endif
#endif // RE_SPINOZA_MODE
//! Structure that contain single bracket info
typedef struct tag_Match{
  int start,end;
}SMatch,*PMatch;
#ifdef RE_SPINOZA_MODE
  #if defined(__BORLANDC__)
    #pragma option -a.
  #elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
    #pragma pack()
  #else
    #pragma pack(pop)
  #endif
#endif // RE_SPINOZA_MODE

//! Add named brackets and named backrefs
#ifdef NAMEDBRACKETS
//! Hash table with match info
typedef Hash<SMatch> MatchHash;
//! Pointer to hash MatchHash - passed to Match and Search methods
typedef MatchHash *PMatchHash;
#endif

//! Highly experimental feature
#ifdef RELIB
class RegExp;
typedef Hash<RegExp*> RELib;
typedef RELib *PRELib;
class MatchList;
struct SMatchListItem{
  int start,end;
  prechar name;
  MatchList *sublist;
};
class MatchList:public List<SMatchListItem>{
public:
  MatchList *parent;
};
typedef MatchList *PMatchList;

struct SCallStackItem{
  prechar name;
  int strpos;
};
typedef List<SCallStackItem> CallStack;
#endif

//! Used internally
typedef struct StateStackItem{
  int op;
  REOpCode* pos;
  prechar savestr;
  prechar startstr;
  int min;
  int cnt;
  int max;
  int forward;
  #ifdef RE_NO_NEWARRAY
    static void OnCreate(void *ptr);
  #endif
}*PStateStackItem;

//! Used internally
typedef struct StateStackPage{
  PStateStackItem stack;
  StateStackPage* prev;
  StateStackPage* next;
}*PStateStackPage;

#ifdef UNICODE
struct UniSet;
#endif

/*! Regular expressions support class.

Expressions must be Compile'ed first,
and than Match string or Search for matching fragment.
*/
class RegExp{
private:
  // code
  PREOpCode code;
#ifdef RE_DEBUG
  prechar resrc;
#endif

  StateStackItem initstack[STACK_PAGE_SIZE];
  StateStackPage initstackpage;

// current stack page and upper stack element
  PStateStackItem stack,st;
  int stackcount;
#ifdef RELIB
  int stackusage;
  int reclevel;
#endif

  char slashChar;
  char backslashChar;

  PStateStackPage firstpage;
  PStateStackPage lastpage;

#ifndef UNICODE
  // locale info
#ifdef RE_EXTERNAL_CTYPE
  LOCALEDEF prechar lc;
  LOCALEDEF prechar uc;
  LOCALEDEF prechar chartypes;
  #ifdef RE_SPINOZA_MODE
    LOCALEDEF prechar charbits;
  #else
    LOCALEDEF rechar charbits[256];
  #endif
#else
  LOCALEDEF rechar lc[256];
  LOCALEDEF rechar uc[256];
  LOCALEDEF rechar chartypes[256];
  LOCALEDEF rechar charbits[256];
#endif
#endif

#ifdef UNICODE
  UniSet *firstptr;
#else
  rechar first[256];
#endif
  int havefirst;
  int havelookahead;

  int minlength;

  // error info
  int errorcode;
  int errorpos;

  // options
  int ignorecase;

  int bracketscount;
  int maxbackref;
#ifdef NAMEDBRACKETS
  int havenamedbrackets;
#endif

  prechar start;
  prechar end;
  prechar trimend;

#ifdef RELIB
  PRELib relib;
  PMatchList matchlist;
#endif

#ifdef RE_NO_NEWARRAY
  typedef void (*ON_CREATE_FUNC)(void *Item);
  typedef void (*ON_DELETE_FUNC)(void *Item);
  static void *CreateArray(const unsigned int size, const unsigned int total,
    ON_CREATE_FUNC Create);
  static void DeleteArray(void **array, ON_DELETE_FUNC Delete);
#endif
  int CalcLength(const prechar src,int srclength);
  int InnerCompile(const prechar src,int srclength,int options);

  int InnerMatch(prechar str,const prechar end,PMatch match,int& matchcount
#ifdef NAMEDBRACKETS
                 ,PMatchHash hmatch
#endif
                 );

  void TrimTail(prechar& end);

  int SetError(int code,int pos){errorcode=code;errorpos=pos;return 0;}

  int GetNum(const prechar src,int& i);

  static inline void SetBit(prechar bitset,int charindex)
  {
    bitset[charindex>>3]|=1<<(charindex&7);
  }
  static inline int GetBit(prechar bitset,int charindex)
  {
    return bitset[charindex>>3]&(1<<(charindex&7));
  }

  void PushState();
  StateStackItem* GetState();
  StateStackItem* FindStateByPos(PREOpCode pos,int op);
  int PopState();


  int StrCmp(prechar& str,prechar start,prechar end);

  void Init(const prechar,int options);
  RegExp(const RegExp& re){};

public:
  //! Default constructor.
  RegExp();
  /*! Create object with compiled expression

     \param expr - source of expression
     \param options - compilation options

     By default expression in perl style expected,
     and will be optimized after compilation.

     Compilation status can be verified with LastError method.
     \sa LastError
  */
  RegExp(const RECHAR* expr,int options=OP_PERLSTYLE|OP_OPTIMIZE
  #ifdef RE_SPINOZA_MODE
    |OP_STRICT
  #endif
  );
  virtual ~RegExp();

#ifndef UNICODE
  /*! Set locale specific information
      \param newlc - table that convert any symbol to it's lowercase state if possible, or left unchanged
      \param newuc - table that convert any symbol to it's uppercase state if possible, or left unchanged
      \param newchartypes - table with locale info bits.

      all tables have 256 elements
      \sa localebits
  */
  #ifndef RE_EXTERNAL_CTYPE
  LOCALEDEF void InitLocale();
  #endif
  LOCALEDEF void SetLocaleInfo(prechar newlc,prechar newuc,prechar newchartypes
  #if defined(RE_EXTERNAL_CTYPE) && defined(RE_SPINOZA_MODE)
     , prechar newcharbits
  #endif
    );
#endif

#ifdef RELIB
  void SetRELib(PRELib newlib){relib=newlib;}
  void SetMatchList(PMatchList newlist){matchlist=newlist;}
  void ResetRecursion(){reclevel=0;stackusage=0;}
  CallStack cs;
#endif

  /*! Compile regular expression
      Generate internall op-codes of expression.

      \param src - source of expression
      \param options - compile options
      \return 1 on success, 0 otherwise

      If compilation fails error code can be obtained with LastError function,
      position of error in a expression can be obtained with ErrorPosition function.
      See error codes in REError enumeration.
      \sa LastError
      \sa REError
      \sa ErrorPosition
      \sa options
  */
  int Compile(const RECHAR* src,int options=OP_PERLSTYLE|OP_OPTIMIZE
  #ifdef RE_SPINOZA_MODE
    |OP_STRICT
  #endif
  );

  /*! Try to optimize regular expression
      Significally speedup Search mode in some cases.
      \return 1 on success, 0 if optimization failed.
  */
  int Optimize();

  /*! Try to match string with regular expression
      \param textstart - start of string to match
      \param textend - point to symbol after last symbols of the string.
      \param match - array of SMatch structures that receive brackets positions.
      \param matchcount - in/out parameter that indicate number of items in
      match array on input, and number of brackets on output.
      \param hmatch - storage of named brackets if NAMEDBRACKETS feature enabled.
      \return 1 on success, 0 if match failed.
      \sa SMatch
  */
  int Match(const RECHAR* textstart,const RECHAR* textend,PMatch match,int& matchcount
#ifdef NAMEDBRACKETS
                 ,PMatchHash hmatch=NULL
#endif
  );
  /*! Same as Match(const char* textstart,const char* textend,...), but for ASCIIZ string.
      textend calculated automatically.
  */
  int Match(const RECHAR* textstart,PMatch match,int& matchcount
#ifdef NAMEDBRACKETS
                 ,PMatchHash hmatch=NULL
#endif
  );
  /*! Advanced version of match. Can be used for multiple matches
      on one string (to imitate /g modifier of perl regexp
  */
  int MatchEx(const RECHAR* datastart,const RECHAR* textstart,const RECHAR* textend,PMatch match,int& matchcount
#ifdef NAMEDBRACKETS
                 ,PMatchHash hmatch=NULL
#endif
  );
  /*! Try to find substring that will match regexp.
      Parameters and return value are the same as for Match.
      It is highly recommended to call Optimize before Search.
  */
  int Search(const RECHAR* textstart,const RECHAR* textend,PMatch match,int& matchcount
#ifdef NAMEDBRACKETS
                 ,PMatchHash hmatch=NULL
#endif
  );
  /*! Same as Search with specified textend, but for ASCIIZ strings only.
      textend calculated automatically.
  */
  int Search(const RECHAR* textstart,PMatch match,int& matchcount
#ifdef NAMEDBRACKETS
                 ,PMatchHash hmatch=NULL
#endif
  );
  /*! Advanced version of search. Can be used for multiple searches
      on one string (to imitate /g modifier of perl regexp
  */
  int SearchEx(const RECHAR* datastart,const RECHAR* textstart,const RECHAR* textend,PMatch match,int& matchcount
#ifdef NAMEDBRACKETS
                 ,PMatchHash hmatch=NULL
#endif
  );

  /*! Clean regexp execution stack.
      After match large string with complex regexp, significant
      amount of memory can be allocated for execution stack.
  */
  void CleanStack();

  /*! Get last error
      \return code of the last error
      Check REError for explanation
      \sa REError
      \sa ErrorPosition
  */
  int LastError() const {return errorcode;}
  /*! Get last error position.
      \return position of the last error in the regexp source.
      \sa LastError
  */
  int ErrorPosition() const {return errorpos;}
  /*! Get number of brackets in expression
      \return number of brackets, excluding brackets of type (:expr)
      and named brackets.
  */
  int GetBracketsCount() const {return bracketscount;}
  typedef bool(*BracketHandler)(void* data,int action,int brindex,int start,int end);
  void SetBracketHandler(BracketHandler bh,void* data)
  {
    brhandler=bh;
    brhdata=data;
  }
protected:
  BracketHandler brhandler;
  void* brhdata;
};

#ifdef RELIB
int RELibMatch(RELib& relib,MatchList& ml,const RECHAR* name,const RECHAR* start);
int RELibMatch(RELib& relib,MatchList& ml,const RECHAR* name,const RECHAR* start,const RECHAR* end);
#endif

} //namespace classses

#endif
