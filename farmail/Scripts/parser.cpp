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
#include "language.hpp"
#include "bltins.hpp"

Parser::Parser(char *script)
{
  lstrcpy(filename,script);
  file=INVALID_HANDLE_VALUE;
  pIfFrame=NULL;
  pLoopFrame=NULL;
  pFunctionFrame=NULL;
  heap=GetProcessHeap();
  yy_buffer=NULL;
  yy_buffer_size=0;
  functions.Add("nlines",blt_nlines);
  functions.Add("line",blt_line);
  functions.Add("strlen",blt_strlen);
  functions.Add("setline",blt_setline);
  functions.Add("delline",blt_delline);
  functions.Add("insline",blt_insline);
  functions.Add("string",blt_string);
  functions.Add("integer",blt_integer);
  functions.Add("message",blt_message);
  functions.Add("char",blt_char);
  functions.Add("substr",blt_substr);
  functions.Add("strlwr",blt_strlwr);
  functions.Add("strupr",blt_strupr);
  functions.Add("strstr",blt_strstr);
  functions.Add("random",blt_random);
  functions.Add("date",blt_date);
  functions.Add("time",blt_time);
  functions.Add("boundary",blt_boundary);
  functions.Add("fileline",blt_fileline);
  functions.Add("filecount",blt_filecount);
  functions.Add("blktype",blt_blktype);
  functions.Add("blkstart",blt_blkstart);
  functions.Add("selstart",blt_selstart);
  functions.Add("selend",blt_selend);
  functions.Add("setsel",blt_setsel);
  functions.Add("usrinput",blt_usrinput);
  functions.Add("redirect",blt_redirect);
  functions.Add("version",blt_version);
  functions.Add("setpos",blt_setpos);
  functions.Add("header",blt_header);
  functions.Add("usrmenu",blt_usrmenu);
  functions.Add("winampstate",blt_winampstate);
  functions.Add("winampsong",blt_winampsong);
  functions.Add("addressbook",blt_addressbook);
  functions.Add("exit",blt_exit);
  functions.Add("sprintf",blt_sprintf);
  functions.Add("encodeheader",blt_encodeheader);
  functions.Add("getini",blt_getini);
  functions.Add("setini",blt_setini);
  functions.Add("editorstate",blt_editorstate);
}

Parser::~Parser()
{
}

bool Parser::Compile(void)
{
  bool result=false;
  code.Clear();
  file=CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if(file!=INVALID_HANDLE_VALUE)
  {
    yy_buffer_size=LEXER_BUFFER;
    yy_buffer=(YYCTYPE *)HeapAlloc(heap,HEAP_ZERO_MEMORY,yy_buffer_size);
    if(yy_buffer)
    {
      yy_line=0;
      yy_line_update=false;
      yy_col=0;
      yy_eof=NULL;
      yy_cursor=yy_buffer;
      yy_limit=yy_buffer;
      yy_linestart=yy_buffer;
      error_index=MesError;
      result=yyparse(NULL);
      AddCode(opEXIT);
      HeapFree(heap,0,yy_buffer);
      yy_buffer=NULL;
    }
    yy_buffer_size=0;
    CloseHandle(file);
    file=INVALID_HANDLE_VALUE;
    Cleanup();
  }
  return result;
}

void Parser::Cleanup(void)
{
  IfFrame *curr_frame1;
  LoopFrame *curr_frame2;
  FunctionFrame *curr_frame3;
  while(pIfFrame)
  {
    curr_frame1=pIfFrame->Next;
    HeapFree(heap,0,pIfFrame);
    pIfFrame=curr_frame1;
  }
  while(pLoopFrame)
  {
    curr_frame2=pLoopFrame->Next;
    HeapFree(heap,0,pLoopFrame);
    pLoopFrame=curr_frame2;
  }
  while(pFunctionFrame)
  {
    curr_frame3=pFunctionFrame->Next;
    HeapFree(heap,0,pFunctionFrame);
    pFunctionFrame=curr_frame3;
  }
}

void Parser::Execute(void)
{
  Variant A, B;
  long JmpInc,Code;
  int stop=FALSE;
  code.SetPosition(0);
  stack.Clear();
  while(code.GetPosition()<code.GetSize()&&(!stop))
  {
    long OpCode=GetCode();
    switch(OpCode)
    {
      case opEXIT:
        stop=TRUE;
        break;
      case opPOP:
        stack.Pop();
        break;
      case opPUSHD:
        stack.Push(GetData());
        break;
      case opPUSHV:
        stack.Push(symbols[GetCode()].Value());
        break;
      case opPUSHS:
        {
          A=stack.Pop();
          B=symbols[GetCode()].Value();
          const char *string=B;
          __INT64 index=A;
          if(index>=0&&index<B.length())
            A=(__INT64)string[index];
          else
            A=(__INT64)0;
          stack.Push(A);
        }
        break;
      case opMOV:
        Code=GetCode();
        symbols[Code].Value()=stack.Top();
        break;
      case opMOVS:
        {

          B=stack.Pop(); //value
          A=stack.Pop(); //index
          __INT64 index=A;
          symbols[GetCode()].Value().set(index,B);
          stack.Push(B);
        }
        break;
      case opADD:
        B=stack.Pop();
        A=stack.Pop();
        stack.Push(A+B);
        break;
      case opSUB:
        B=stack.Pop();
        A=stack.Pop();
        stack.Push(A-B);
        break;
      case opMUL:
        B=stack.Pop();
        A=stack.Pop();
        stack.Push(A*B);
        break;
      case opDIV:
        B=stack.Pop();
        A=stack.Pop();
        if((__INT64)B==0)
        {
          SayError(ERR_DIVZERO,"/");
          stop=TRUE;
        }
        else stack.Push(A/B);
        break;
      case opINC:
        Code=GetCode();
        symbols[Code].Value()+=+1;
        stack.Push(symbols[Code].Value());
        break;
      case opPOSTINC:
        Code=GetCode();
        stack.Push(symbols[Code].Value());
        symbols[Code].Value()+=1;
        break;
      case opDEC:
        Code=GetCode();
        symbols[Code].Value()-=1;
        stack.Push(symbols[Code].Value());
        break;
      case opPOSTDEC:
        Code=GetCode();
        stack.Push(symbols[Code].Value());
        symbols[Code].Value()-=1;
        break;
      case opNEG:
        A=stack.Pop();
        A=-A;
        stack.Push(A);
        break;
      case opGT :
        B=stack.Pop();
        A=stack.Pop();
        stack.Push(A>B);
        break;
      case opGE :
        B=stack.Pop();
        A=stack.Pop();
        stack.Push(A>=B);
        break;
      case opLT :
        B=stack.Pop();
        A=stack.Pop();
        stack.Push(A<B);
        break;
      case opLE :
        B=stack.Pop();
        A=stack.Pop();
        stack.Push(A<=B);
        break;
      case opEQ :
        B=stack.Pop();
        A=stack.Pop();
        stack.Push(A==B);
        break;
      case opNE :
        B=stack.Pop();
        A=stack.Pop();
        stack.Push(A!=B);
        break;
      case opAND:
        B=stack.Pop();
        A=stack.Pop();
        stack.Push(A&&B);
        break;
      case opOR :
        B=stack.Pop();
        A=stack.Pop();
        stack.Push(A||B);
        break;
      case opNOT:
        A=stack.Pop();
        A=!A;
        stack.Push(A);
        break;
      case opJMP:
        JmpInc=GetCode()-sizeof(long);
        code.SetPosition(code.GetPosition()+JmpInc);
        break;
      case opJNE:
        A=stack.Pop();
        JmpInc=GetCode()-sizeof(long);
        if(A) code.SetPosition(code.GetPosition()+JmpInc);
        break;
      case opJE:
        A=stack.Pop();
        JmpInc=GetCode()-sizeof(long);
        if(!A)
        {
          code.SetPosition(code.GetPosition()+JmpInc);
        }
        break;
      case opFUNC:
        {
          __INT64 FunctionCount=stack.Pop();
          Variant *FunctionParams=new Variant[FunctionCount];
          if(FunctionParams)
          {
            for(long i=0;i<FunctionCount;i++)
            {
              A=stack.Pop();
              FunctionParams[FunctionCount-1-i]=A;
            }
            stack.Push(functions.Run(GetCode(),FunctionCount,FunctionParams,&stop,this));
            delete [] FunctionParams;
          }
        }
        break;
      case opCALL:
        JmpInc=GetCode();
        stack.Push(code.GetPosition());
        code.SetPosition(JmpInc);
        break;
      case opRET:
        A=stack.Pop();
        code.SetPosition(A);
        break;
    }
  }
}

long Parser::AddCode(long Value)
{
  long result=code.GetPosition();
  code.Write(&Value,sizeof(Value));
  return result;
}

long Parser::AddInt64(__INT64 Value)
{
  long result=code.GetPosition();
  unsigned char type=vtInt64;
  code.Write(&type,sizeof(type));
  code.Write(&Value,sizeof(Value));
  return result;
}

long Parser::AddString(const CustomString& Value)
{
  long result=code.GetPosition(),size=Value.length();
  unsigned char type=vtString;
  code.Write(&type,sizeof(type));
  code.Write(&size,sizeof(size));
  code.Write((const char *)Value,size);
  return result;
}

long Parser::GetCode(void)
{
  long result;
  code.Read(&result,sizeof(result));
  return result;
}

Variant Parser::GetData(void)
{
  Variant result;
  unsigned char type;
  code.Read(&type,sizeof(type));
  switch(type)
  {
    case vtInt64:
      {
        __INT64 value=0;
        code.Read(&value,sizeof(value));
        result=value;
      }
      break;
    case vtString:
      {
        char *value;//=NULL;
        long size=0;
        code.Read(&size,sizeof(size));
        if(size)
        {
          value=(char *)HeapAlloc(heap,HEAP_ZERO_MEMORY,size+1);
          if(value) code.Read(value,size);
          result=value;
        }
        else result="";
      }
      break;
  }
  return result;
}

void Parser::AddXRef(XRef **xref,long ref)
{
  XRef *new_xref=(XRef *)HeapAlloc(heap,HEAP_ZERO_MEMORY,sizeof(XRef));
  if(new_xref)
  {
    new_xref->Ref=ref;
    new_xref->Next=*xref;
    *xref=new_xref;
  }
}

void Parser::SolveXRef(XRef *xref,long value)
{
  XRef *curr_xref;
  while(xref)
  {
    long shift=value-xref->Ref;
    curr_xref=xref->Next;
    code.WriteRandom(xref->Ref,&shift,sizeof(shift));
    HeapFree(heap,0,xref);
    xref=curr_xref;
  }
}

void Parser::CreateIfFrame(void)
{
  IfFrame *new_frame=(IfFrame *)HeapAlloc(heap,HEAP_ZERO_MEMORY,sizeof(IfFrame));
  if(new_frame)
  {
    new_frame->Other=NULL;
    new_frame->Follow=NULL;
    new_frame->Next=pIfFrame;
    pIfFrame=new_frame;
  }
}

void Parser::SolveIfFrame(long Other,long Follow)
{
  if(pIfFrame)
  {
    IfFrame *curr_frame=pIfFrame->Next;
    SolveXRef(pIfFrame->Other,Other);
    SolveXRef(pIfFrame->Follow,Follow);
    HeapFree(heap,0,pIfFrame);
    pIfFrame=curr_frame;
  }
}

void Parser::CreateLoopFrame(void)
{
  LoopFrame *new_frame=(LoopFrame *)HeapAlloc(heap,HEAP_ZERO_MEMORY,sizeof(LoopFrame));
  if(new_frame)
  {
    new_frame->Body=NULL;
    new_frame->Cond=NULL;
    new_frame->Follow=NULL;
    new_frame->Next=pLoopFrame;
    pLoopFrame=new_frame;
  }
}

void Parser::SolveLoopFrame(long Body,long Cond,long Follow)
{
  if(pLoopFrame)
  {
    LoopFrame *curr_frame=pLoopFrame->Next;
    SolveXRef(pLoopFrame->Body,Body);
    SolveXRef(pLoopFrame->Cond,Cond);
    SolveXRef(pLoopFrame->Follow,Follow);
    HeapFree(heap,0,pLoopFrame);
    pLoopFrame=curr_frame;
  }
}

void Parser::CreateFunctionFrame(void)
{
  FunctionFrame *new_frame=(FunctionFrame *)HeapAlloc(heap,HEAP_ZERO_MEMORY,sizeof(FunctionFrame));
  if(new_frame)
  {
    new_frame->Count=0;
    new_frame->Next=pFunctionFrame;
    pFunctionFrame=new_frame;
  }
}

void Parser::SolveFunctionFrame(void)
{
  if(pFunctionFrame)
  {
    FunctionFrame *curr_frame=pFunctionFrame->Next;
    HeapFree(heap,0,pFunctionFrame);
    pFunctionFrame=curr_frame;
  }
}
