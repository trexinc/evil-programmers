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
#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <windows.h>
#include "type.hpp"
#include "custom_string.hpp"
#include "custom_memory.hpp"
#include "variant.hpp"
#include "stack.hpp"
#include "parser_grammar.hpp"
#include "function.hpp"

//opcodes
const long opEXIT     =      0;
const long opPOP      =      1;
const long opPUSHD    =      2;
const long opPUSHV    =      3;
const long opPUSHS    =      4;
const long opMOV      =      5;
const long opMOVS     =      6;
const long opADD      =      7; //арифметические комманды
const long opSUB      =      8;
const long opMUL      =      9;
const long opDIV      =     10;
const long opINC      =     11; //инкременты/декременты
const long opPOSTINC  =     12;
const long opDEC      =     13;
const long opPOSTDEC  =     14;
const long opNEG      =     15;
const long opGT       =     16; //комманды сравнения
const long opGE       =     17;
const long opLT       =     18;
const long opLE       =     19;
const long opEQ       =     20;
const long opNE       =     21;
const long opAND      =     22; //логические комманды
const long opOR       =     23;
const long opNOT      =     24;
const long opJMP      =     25; //комманды управления потоком
const long opJNE      =     26;
const long opJE       =     27;
const long opFUNC     =     28; //функции расширения
const long opCALL     =     29; //подпрограммы
const long opRET      =     30;

#define LEXER_BUFFER (128)
#define YYCTYPE unsigned char
#define YYCURSOR yy_cursor
#define YYLIMIT yy_limit
#define YYMARKER yy_marker
#define YYFILL(n) { if(!fill(n)) return 0; }

struct XRef
{
  long Ref;
  XRef *Next;
};

struct IfFrame
{
  XRef *Other;
  XRef *Follow;
  IfFrame *Next;
};

struct LoopFrame
{
  XRef *Body;
  XRef *Cond;
  XRef *Follow;
  LoopFrame *Next;
};

struct FunctionFrame
{
  long Count;
  FunctionFrame *Next;
};

class Names
{
  private:
    CustomString name;
    Variant value;
  public:
    Names& operator=(const Names& Value);
    Names& operator=(const char *Value);
    operator const char *() const;
    Variant& Value(void);
};

class Symbols
{
  private:
    Names *names;
    Names default_name;
    long count;
  public:
    Symbols();
    ~Symbols();
    long IndexOf(const char *Value);
    void Clear(void);
    bool Add(const char *Name);
    long GetCount(void);
    Names &operator[](const char *Name);
    Names &operator[](long Index);
};

class Parser
{
  private:
    char filename[MAX_PATH];
    HANDLE file;
    HANDLE heap;
    Symbols symbols;
    Functions functions;
    CustomString temp_string;
    int error_index;
    Stack stack;
    //lexer
    YYCTYPE *yy_buffer;
    size_t yy_buffer_size;
    YYCTYPE *yy_eof; //not null if all file readed.
    YYCTYPE *yy_token; //pointer to current token
    YYCTYPE *yy_marker; //YYMARKER
    YYCTYPE *yy_cursor; //YYCURSOR
    YYCTYPE *yy_limit; //YYLIMIT
    YYCTYPE *yy_linestart; //start of line
    long yy_line; //line
    bool yy_line_update;
    long yy_col; //column

    __INT64 GetValue(void);
    char *GetString(void);

    //parser
    CustomMemory code;
    int yylex(YYSTYPE *yylval);
    bool fill(int n);
    int yyparse(void *);
    void yyerror(int msg);

    //write VM
    IfFrame *pIfFrame;
    LoopFrame *pLoopFrame;
    FunctionFrame *pFunctionFrame;

    long AddCode(long Value);
    long AddInt64(__INT64 Value);
    long AddString(const CustomString& Value);

    void AddXRef(XRef **xref,long ref);
    void SolveXRef(XRef *xref,long value);

    void CreateIfFrame(void);
    void SolveIfFrame(long Other,long Follow);

    void CreateLoopFrame(void);
    void SolveLoopFrame(long Body,long Cond,long Follow);

    void CreateFunctionFrame(void);
    void SolveFunctionFrame(void);

    void Cleanup(void);

    //read VM
    long GetCode(void);
    Variant GetData(void);
  public:
    Parser(char *script);
    ~Parser();
    bool Compile(void);
    void Execute(void);
    void Edit(void);
};

#endif
