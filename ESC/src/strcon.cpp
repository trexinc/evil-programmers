#ifndef __strcon_cpp
#define __strcon_cpp

#include "myrtl.hpp"
#include "strcon.hpp"

strcon::strcon(const char *ptr,int size):str(NULL)
{
  setStr(ptr,size);
}

strcon::strcon(const unsigned char *ptr, int size):str(NULL)
{
  setStr(reinterpret_cast<const char*>(ptr),size);
}

strcon::strcon(const strcon &rhs):str(NULL)
{
  setStr(rhs.str);
}

strcon::strcon():str(NULL)
{
  setStr("");
}

strcon::~strcon()
{
  Free();
}

bool strcon::operator==(const strcon &ptr) const
{
  return (str && ptr.str)?!strcmp(str,ptr.str):0;
}

bool strcon::operator==(const char * ptr) const
{
  return (str && ptr)?!strcmp(str,ptr):0;
}

strcon& strcon::operator=(const char *rhs)
{
  if(rhs<str || rhs>(str+Len))
    setStr(rhs);
  return *this;
}

strcon& strcon::operator=(const strcon &rhs)
{
  if(this!=&rhs) setStr(rhs.str);
  return *this;
}

const strcon& strcon::AddStr(const strcon &s)
{
  return AddStr(s.str,s.Len);
}

const strcon& strcon::AddStrings(const char *firstStr, ...)
{
  AddStr(firstStr);
  va_list l;
  const char *arg;
  va_start(l, firstStr);
  while ((arg = va_arg(l,const char*)) != NULL)
    AddStr(arg);
  va_end(l);
  return *this;
}

const strcon& strcon::AddStr(const char *s, int size)
{
  if(s)
  {
    if(size<0)
      size=lstrlen(s);

    if(s>str && s<(str+Len))
    {
      strcon tmp(s, size);
      return AddStr(tmp.str);
    }

    int newLen=size+Len;
    char *newstr=static_cast<char*>(malloc(1+newLen));
    if(newstr)
    {
      memcpy(newstr, str, Len);
      memcpy(newstr+Len, s, size);
      newstr[newLen]=0;
      Free();
      str=newstr;
      Len=newLen;
    }
  }
  return *this;
}

const strcon& strcon::AddChar(char Chr)
{
  return AddStr(&Chr,1);
}

bool strcon::setLength(unsigned newLen)
{
  if(Len>=newLen)
  {
    Len=newLen;
    str[Len]=0;
    return true;
  }
  return false;
}

char *strcon::setStr(const char *s,int size)
{
  if(s)
  {
    unsigned int newLen=lstrlen(s);
    if(size>-1 && static_cast<unsigned int>(size)<newLen)
      newLen=size;
    char *newstr=static_cast<char*>(malloc(1+newLen));
    if(newstr)
    {
      memcpy(newstr, s, newLen);
      newstr[newLen]=0;
      Free();
      str=newstr;
      Len=newLen;
    }
    else
      Free();
  }
  else
    Free();
  return str;
}

void strcon::Free()
{
  if(str)
  {
    free(str);
    str=NULL;
  }
  Len=0;
}

#endif //__strcon_cpp
