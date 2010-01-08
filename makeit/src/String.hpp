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

  Yet another C++ string implementation.
  Key feature - this string can hold zero characters.
  Feature that can be considered as benefit in some cases and
  as drawback in other - quantified memory allocation.
*/

#ifndef __STRING_HPP__
#define __STRING_HPP__

#ifndef __cplusplus
#error This header for C++ only
#endif

#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>


/*! String class
*/

class String{
protected:
  enum flags{
    nocase=1,
    temp=nocase<<1,
    reused=temp<<1,
  };

  char *_str;
  int  _length;
  int  _size;
  mutable int _flags;

  static inline void xmemcpy(void* dst,const void* src,int size)
  {
    if(!dst || !src)return;
    memcpy(dst,src,size);
  }


  void SetSize(int newsize)
  {
    if(newsize<_size)return;
    delete [] _str;
    _size=newsize+newsize+1;
    _str=new char[_size];
    _length=0;
    _str[0]=0;
  }

  int _Index(const char *substr,int substrlength,int pos)const
  {
  //  int substrlength=strlen(substr);
    if(pos<0)pos+=_length;
    if(pos<0 || pos>_length)return -1;
    int i=pos,j;
    if(_flags&nocase)
    {
      while(i+substrlength<=_length)
      {
        if(tolower(_str[i])==tolower(substr[0]))
        {
          if(substrlength==1)return i;
          j=1;
          while(tolower(_str[i+j])==tolower(substr[j]))
          {
            j++;
            if(j==substrlength)return i;
          }
        }
        i++;
      }
    }else
    {
      while(i+substrlength<=_length)
      {
        if(_str[i]==substr[0])
        {
          if(substrlength==1)return i;
          j=1;
          while(_str[i+j]==substr[j])
          {
            j++;
            if(j==substrlength)return i;
          }
        }
        i++;
      }
    }
    return -1;
  }
  int _RIndex(const char *substr,int substrlength,int pos)const
  {
  //  int substrlength=strlen(substr);
    if(pos<0)pos+=_length;
    if(pos<0 || pos>_length)return -1;
    int i=pos,j;
    if((_flags&nocase))
    {
      while(i>=0)
      {
        if(tolower(_str[i])==tolower(substr[0]))
        {
          if(substrlength==1)return i;
          j=1;
          while(tolower(_str[i+j])==tolower(substr[j]))
          {
            j++;
            if(j==substrlength)return i;
          }
        }
        i--;
      }
    }else
    {
      while(i>=0)
      {
        if(_str[i]==substr[0])
        {
          if(substrlength==1)return i;
          j=1;
          while(_str[i+j]==substr[j])
          {
            j++;
            if(j==substrlength)return i;
          }
        }
        i--;
      }
    }
    return -1;
  }
public:
  //! Empty string constructor
  String()
  {
    _str=0;
    _length=0;
    _size=0;
    _flags=0;
  }

  //! Copy constructor
  String(const String& src)
  {
    _str=0;
    _length=0;
    _size=0;
    _flags=0;
    if(src._flags&temp)
    {
      _str=src._str;
      src._flags|=reused;
      src._flags&=~temp;
    }else
    {
      SetSize(src.Length());
      xmemcpy(_str,(const char*)src,src.Length()+1);
    }
    _length=src.Length();
  }
  //! Constructor from ASCIIZ string
  String(const char* src)
  {
    _flags=0;
    _length=strlen(src);
    _size=_length+16;
    _str=new char[_size];
    xmemcpy(_str,src,_length+1);
  }

  /*! This constructor will preallocate {num} bytes
      \param num - amount of bytes to preallocate
  */
  explicit String(int num)
  {
    _flags=0;
    _length=0;
    _str=new char[num+1];
    _str[0]=0;
    _size=num+1;
  }
  //! Destructor :)
  ~String()
  {
    Clean();
  }

  //! assignment operator for String
  void operator=(const String& src)
  {
    SetSize(src.Length());
    xmemcpy(_str,(const char*)src,src.Length()+1);
    _length=src.Length();
  }
  //! assignment operator for ASCIIZ string
  void operator=(const char* src)
  {
    int len=strlen(src);
    SetSize(len);
    _length=len;
    xmemcpy(_str,src,_length+1);
  }

  //! assignment operator for single char
  void operator=(char src)
  {
    SetSize(4);
    _length=1;
    _str[0]=src;
    _str[1]=0;
  }



  bool operator==(const String& cmp)const
  {
    int i;
    if((_flags&nocase))return CmpNoCase(cmp)!=0;
    if((const char*)cmp==0)return _str==0;
    if(!_str)return !(const char*)cmp;
    if(_length!=cmp.Length())return 0;
    for(i=0;i<_length;i++)
    {
      if(_str[i]!=((const char*)cmp)[i])return 0;
    }
    return 1;
  }
  bool operator==(const char* cmp)const
  {
    int i;
    if((_flags&nocase))return CmpNoCase(cmp)!=0;
    if(!cmp)return _str==0;
    if(!_str)return !cmp;
    for(i=0;i<_length;i++)
    {
      if(_str[i]!=cmp[i])return 0;
    }
    if(cmp[i]!=0)return 0;
    return 1;
  }

  bool operator!=(const String& cmp)const
  {
    return !(*this==cmp);
  }
  bool operator!=(const char* cmp)const
  {
    return !(*this==cmp);
  }

  bool operator<(const String& cmp)const
  {
    int res=memcmp(_str,cmp._str,_length<cmp._length?_length:cmp._length);
    return res<0 || (res==0 && _length<cmp._length);
  }
  bool operator<(const char* cmp)const
  {
    if((_flags&nocase))
    {
      return stricmp(_str,cmp)<0;
    }else
    {
      return strcmp(_str,cmp)<0;
    }
  }

  bool operator>(const String& cmp)const
  {
    int res=memcmp(_str,cmp._str,_length<cmp._length?_length:cmp._length);
    return res>0 || (res==0 && _length>cmp._length);
  }
  bool operator>(const char* cmp)const
  {
    if((_flags&nocase))
    {
      return stricmp(_str,cmp)>0;
    }else
    {
      return strcmp(_str,cmp)>0;
    }
  }

  bool operator<=(const String& cmp)const
  {
    int res=memcmp(_str,cmp._str,_length<cmp._length?_length:cmp._length);
    return res<0 || (res==0 && _length<=cmp._length);
  }
  bool operator<=(const char* cmp)const
  {
    if((_flags&nocase))
    {
      return stricmp(_str,cmp)<=0;
    }else
    {
      return strcmp(_str,cmp)<=0;
    }
  }

  bool operator>=(const String& cmp)const
  {
    int res=memcmp(_str,cmp._str,_length<cmp._length?_length:cmp._length);
    return res>0 || (res==0 && _length>=cmp._length);
  }
  bool operator>=(const char* cmp)const
  {
    if((_flags&nocase))
    {
      return stricmp(_str,cmp)>=0;
    }else
    {
      return strcmp(_str,cmp)>=0;
    }
  }

  String operator+(const String& src)const
  {
    String tmp(*this);
    tmp._flags|=temp;
    tmp+=src;
    return tmp;
  }
  String operator+(const char* src)const
  {
    String tmp(*this);
    tmp._flags|=temp;
    tmp+=src;
    return tmp;
  }
  String operator+(const char src)const
  {
    String tmp(*this);
    tmp._flags|=temp;
    tmp+=src;
    return tmp;
  }
  /*! This will add numeric representation of integer
    \param src - integer number that will be added to string
  */
  String operator+(int src)const
  {
    String tmp(*this);
    tmp._flags|=temp;
    tmp+=src;
    return tmp;
  }

  /*!
    Operator that will enable expressions like this:<br>
    str="hello"+str2;
  */
  friend String operator+(const char* str1,const String& str2)
  {
    String tmp;
    tmp._flags|=temp;
    int len=strlen(str1);
    tmp.SetSize(len+str2.Length());
    xmemcpy(tmp._str,str1,len);
    xmemcpy(tmp._str+len,str2._str,str2._length);
    tmp._length=len+str2._length;
    tmp._str[tmp._length]=0;
    return tmp;
  }

  friend String operator+(char c,const String& str)
  {
    String tmp;
    tmp._flags|=temp;
    tmp.SetSize(1+str.Length());
    tmp._str[0]=c;
    xmemcpy(tmp._str+1,str._str,str._length);
    tmp._length=1+str._length;
    tmp._str[tmp._length]=0;
    return tmp;
  }

  friend String operator+(int x,const String& str)
  {
    char buf[64];
    int len=sprintf(buf,"%d",x);
    String tmp;
    tmp._flags|=temp;
    tmp.SetSize(len+str.Length());
    xmemcpy(tmp._str,buf,len);
    xmemcpy(tmp._str+len,str._str,str._length);
    tmp._length=len+str._length;
    tmp._str[tmp._length]=0;
    return tmp;
  }

/*!
  Operator that will enable expressions like this:<br>
  if("hello"==world){}
*/
  friend bool operator==(const char* str1,const String& str2)
  {
    return str2==str1;
  }


  void operator+=(const String& src)
  {
    Resize(_length+src.Length());
    xmemcpy(_str+_length,(const char*)src,src.Length()+1);
    _length+=src.Length();
  }
  void operator+=(const char* src)
  {
    int srclength=strlen(src);
    Resize(_length+srclength);
    xmemcpy(_str+_length,src,srclength+1);
    _length+=srclength;
  }
  void operator+=(const char src)
  {
    Resize(_length+1);
    _str[_length]=src;
    _length++;
    _str[_length]=0;
  }
  //! This will append numeric representation of integer
  void operator+=(int src)
  {
    char buf[64];
    sprintf(buf,"%d",src);
    (*this)+=buf;
  }

  //! The same as +=
  String& operator<<(const String& src)
  {
    (*this)+=src;
    return *this;
  }
  //! The same as +=
  String& operator<<(const char* src)
  {
    (*this)+=src;
    return *this;
  }
  //! The same as +=
  String& operator<<(char src)
  {
    (*this)+=src;
    return *this;
  }
  //! The same as +=
  String& operator<<(int src)
  {
    (*this)+=src;
    return *this;
  }

  //! constant version of char* typecast operator
  operator const char*()const{return _str?_str:"";}

  /*! single char indexing operator
    \param index - index of char to retrieve
    if index < 0 that chars from the end of string are indexed
    i.e. char with index -1 is last char of string,
    with index -2 is the one before the last, etc
  */
  char& operator[](int index)
  {
    if(index<0)index+=_length;
    if(index>_length)index=_length-1;
    return _str[index];
  }

  //! constant version of char& operator[](int index)
  const char operator[](int index)const
  {
    if(index<0)index+=_length;
    if(index>_length)index=_length-1;
    return _str[index];
  }

  //! Will return ASCIIZ pointer to string
  const char* Str()const{return _str;}

  //! Return string concatenated with self num times
  String operator*(int num)
  {
    String tmp;
    tmp._flags|=temp;
    tmp.SetSize(_length*num);
    int i;
    for(i=0;i<num;i++)tmp+=*this;
    return tmp;
  }
  //! Selfconcatenate string num times
  void operator*=(int num)
  {
    int newlength=_length*num;
    Resize(newlength);
    int i,pos=_length;
    for(i=1;i<num;i++)
    {
      memcpy(_str+pos,_str,_length);
      pos+=_length;
    }
    _length=newlength;
    _str[_length]=0;
  }

  /*! Reallocate string buffer, preserving content
      \param newsize - new size of string buffer.
  */
  int Resize(int newsize)
  {
    if(newsize<_size)return 0;
    char *tmpstr=new char[newsize+newsize+1];
    if(!tmpstr)return 0;
    if(_str)xmemcpy(tmpstr,_str,_length);
    delete [] _str;
    _str=tmpstr;
    _size=newsize+newsize+1;
    _str[_length]=0;
    return 1;
  }

  //! return length of the string
  int Length()const{return _length;}
  //! Shrink or enlarge string to the length - newlength
  void SetLength(int newlength)
  {
    Resize(newlength);
    _length=newlength;
    _str[newlength]=0;
  }
  /*! Set 'ignorecase' mode
      \param newnocase - new mode. 0 - disable, 1 - enable
      \return previous mode
  */
  int SetNoCase(int newnocase)
  {
    int oldcase=(_flags&nocase);
    if(newnocase)
    {
      _flags|=nocase;
    }else
    {
      _flags&=~nocase;
    }
    return oldcase;
  }
  //! Compare string ignoring case with ASCIIZ string
  int CmpNoCase(const char * cmp)const
  {
    int i;
    if(!cmp)return _str==0;
    if(!_str)return !cmp;
    for(i=0;cmp[i];i++)
    {
      if(tolower(_str[i])!=tolower(cmp[i]))
      {
        return 0;
      }
    }
    if(i!=_length)return 0;
    return 1;
  }

  /*! Get substring of the string
      \param index - offset of the first char of new string
      \param count - number of chars to get, or -1 if need substring
      from the index till the end.
      \return new string/
  */
  String Substr(int index,int count=-1)const
  {
    String tmp;
    tmp._flags|=temp;
    if(count>_length)count=_length;
    tmp.Set(_str,index,count);
    return tmp;
  }

  /*! Set string content from the ASCIIZ string
      \param src - source ASCIIZ string
      \param index - offset of the first char
      \param count - number of chars to copy
  */
  void Set(const char* src,int index=0,int count=-1)
  {
    int len=-1;
    if(index<0)
    {
      len=strlen(src);
      index+=len;
    }
    if(index<0)return;
    if(count==-1)
    {
      count=(len==-1?strlen(src):len)-index;
    }
    if(count<0)return;
    SetSize(count);
    memcpy(_str,src+index,count);
    _length=count;
    _str[_length]=0;
  }

  /*! Set string content from the ASCIIZ string
      \param src - source ASCIIZ string
      \param index - offset of the first char
      \param count - number of chars to copy
  */
  void Set(const String& src,int index=0,int count=-1)
  {
    if(index<0)
    {
      index+=src.Length();
    }
    if(index<0)return;
    if(count==-1)
    {
      count=src.Length()-index;
    }
    if(count<0)return;
    SetSize(count);
    memcpy(_str,src._str+index,count);
    _length=count;
    _str[_length]=0;
  }
  /*! Concatenate string with part of ASCIIZ string
      \param src - source ASCIIZ string
      \param index - offset of first char to concatenate
      \param count - number of chars to concatenate
  */
  void Concat(const char* src,int index=0,int count=-1)
  {
    if(count<0)return;
    Resize(_length+count);
    memcpy(_str+_length,src+index,count);
    _length+=count;
    _str[_length]=0;
  }

  /*! Find substring
      \param substr - substring to search
      \param pos - offset where to start search
      \return - offset of substring if found, or -1 otherwise
  */
  int Index(const String& substr,int pos=0)const
  {
    return _Index((const char*)substr,substr.Length(),pos);
  }
  //! Same as int Index(const String& substr,int pos=0)const;, but for ASCIIZ string
  int Index(const char* substr,int pos=0)const
  {
    return _Index(substr,strlen(substr),pos);
  }
  //! Same as int Index(const String& substr,int pos=0)const;, but search from the right
  int RIndex(const String& substr,int pos=-1)const
  {
    return _RIndex(substr,substr.Length(),pos);
  }

  //! Same as int RIndex(const String& substr,int pos=-1)const;, but for ASCIIZ string
  int RIndex(const char* substr,int pos=-1)const
  {
    return _RIndex(substr,strlen(substr),pos);
  }

  /*! Insert string
      \param index - offset where to insert string. if index<0 offset
      calculated from the end of the string
      \param ins - string to insert
  */
  String& Insert(int index,const String& ins,int fromIndex=0,int count=-1)
  {
    if(count==-1)
    {
      count=ins.Length()-fromIndex;
    }
    if(fromIndex<0)fromIndex=fromIndex+ins.Length();
    if(fromIndex<0)return *this;
    if(fromIndex+count>ins.Length())count=ins.Length()-fromIndex;
    int newlength=_length+count;
    if(index<0)index+=_length;
    if(index>_length)
    {
      (*this)+=ins;
      return *this;
    }
    Resize(newlength);
    memmove(_str+index+count,_str+index,_length-index+1);
    xmemcpy(_str+index,ins._str+fromIndex,count);
    _length=newlength;
    return *this;
  }
  //! Same as void Insert(int index,const String& ins);, but for ASCIIZ string
  String& Insert(int index,const char* ins,int count=-1)
  {
    if(count==-1)count=strlen(ins);
    int newlength=_length+count;
    if(index<0)index+=_length;
    if(index>_length)
    {
      (*this)+=ins;
      return *this;
    }
    Resize(newlength);
    memmove(_str+index+count,_str+index,_length-index+1);
    xmemcpy(_str+index,ins,count);
    _length=newlength;
    return *this;
  }

  /*! Delete substring
      \param index - offset of first char to delete. If index <0 then
      chars counted from the end of string.
      \param count - number of chars to delete, or -1 to delete till the end
  */
  void Delete(int index,int count=-1)
  {
    if(index<0)index+=_length;
    if(index<0)return;
    if(count<0 || index+count>_length)
    {
      _length=index;
      _str[_length]=0;
      return;
    }
    memmove(_str+index,_str+index+count,_length-index-count+1);
    _length-=count;
  }

  /*! Replace substring
      \param src - substring that will be replaced
      \param dst - substring will replace src
  */
  int Replace(const String& src,const String& dst)
  {
    String tmp;
    int i=0,j=0,cnt=0;
    while((i=Index(src,i))!=-1)
    {
      tmp.Concat(_str,j,i-j);
      tmp<<dst;
      i+=src.Length();
      j=i;
      cnt++;
    }
    if(j<_length)tmp.Concat(_str,j,_length-j);
  //  if(
    (*this)=tmp;
    return cnt;
  }
  //! Same as void Replace(const String& src,const String& dst);, but for ASCIIZ src
  int Replace(const char* src,const char* dst)
  {
    String tmp;
    int srclength=strlen(src);
    int i=0,j=0,cnt=0;
    while((i=Index(src,i))!=-1)
    {
      tmp.Concat(_str,j,i-j);
      tmp<<dst;
      i+=srclength;
      j=i;
      cnt++;
    }
    if(j<_length)tmp.Concat(_str,j,_length-j);
    (*this)=tmp;
    return cnt;
  }

  /*! Replace chars
    \param from - what char to replace
    \param to - to which char to replace
  */
  int Replace(char from,char to)
  {
    int cnt=0;
    for(int i=0;i<_length;i++)
    {
      if(_str[i]==from)
      {
        _str[i]=to;
        cnt++;
      }
    }
    return cnt;
  }

  /*! Check if string start with specified substring
    \param str - string to check
  */
  bool StartWith(const char* str)const
  {
    if(!str || !*str)
    {
      return true;
    }
    int len=strlen(str);
    if(len>_length)return false;
    if((_flags&nocase))
    {
      return !memicmp(_str,str,len);
    }else
    {
      return !memcmp(_str,str,len);
    }
  }
  bool StartWith(const String& str)const
  {
    if(str.Length()==0)
    {
      return true;
    }
    if(str.Length()>_length)return false;
    if((_flags&nocase))
    {
      return !memicmp(_str,str._str,str.Length());
    }else
    {
      return !memcmp(_str,str._str,str.Length());
    }
  }

  /*! Fill string with char
      \param src - char that will fill the string
      \param num - new length of the string
  */
  void Fill(char src,int num)
  {
    SetSize(num);
    memset(_str,src,num);
    _length=num;
    _str[_length]=0;
  }

  /*! Prepare internal buffer of specified size
      and return its pointer to be filler by
      program.
    \param size - minimum buffer length.
  */
  char* Prepare(int size)
  {
    SetSize(size);
    _length=size;
    _str[_length]=0;
    return _str;
  }

  /*! Format string
      \sa sprintf from libc
  */
  String& Sprintf(const char *fmt,...)
  {
    SetSize(strlen(fmt)*4);
    va_list arglist;
    int res;
    do{
      va_start(arglist,fmt);
  #ifdef _WIN32
      res=_vsnprintf( _str, _size,fmt,arglist);
  #else
      res=vsnprintf( _str, _size,fmt,arglist);
  #endif
      va_end(arglist);
      if(res<0)
      {
        SetSize(_size*2);
      }
    }while(res<0);
    _length=res;
    _str[_length]=0;
    return *this;
  }

  String& VASprintf(const char *fmt,va_list va)
  {
    SetSize(strlen(fmt)*4);
    int res;
    do{
  #ifdef _WIN32
      res=_vsnprintf( _str, _size,fmt,va);
  #else
      res=vsnprintf( _str, _size,fmt,va);
  #endif
      if(res<0)
      {
        SetSize(_size-res);
      }
    }while(res<0);
    _length=res;
    _str[_length]=0;
    return *this;
  }


  /*! Append formated string
      \sa sprintf from libc
  */
  String& AppendSprintf(const char *fmt,...)
  {
    Resize(_length+strlen(fmt)*4);
    va_list arglist;
    int res;
    do{
      va_start(arglist,fmt);
      res=_vsnprintf(_str+_length,_size-_length,fmt,arglist);
      va_end(arglist);
      if(res<0)
      {
        Resize(_size-res);
      }
    }while(res<0);
    _length+=res;
    return *this;
  }

  //! Free memory allocated by string
  void Clean()
  {
    if(!(_flags&reused))
    {
      delete [] _str;
    }
    _str=0;
    _size=0;
    _length=0;
  }

  //! Delete trailing spaces
  String& RTrim()
  {
    while(_length>0 && isspace(_str[_length-1]))_length--;
    _str[_length]=0;
    return *this;
  }
  //! Delete leading spaces
  String& LTrim()
  {
    int i=0;
    while(i<_length && isspace(_str[i]))i++;
    if(i==_length)
    {
      _length=0;
      _str[0]=0;
    }else
    {
      memmove(_str,_str+i,_length-i);
      _length-=i;
      _str[_length]=0;
    }
    return *this;
  }
  //! Delete leading and trailing spaces
  String& Trim(){RTrim();LTrim();return *this;}

//! use by String::Align
enum AlignPosition{
  apLeft,apCenter,apRight,apWords,
};

  /*! Align string
      \param where - if set to apRight, fill with spaces to left till new size
      if set to apCenter, fill with spaces from the left and right equally till the new size,
      if set to apLeft, fill with spaces from the left till the new size,
      if set to apWords, try to fill with spaces between words equally, till the new size.
      \param size - new size of string (with spaces).

  */
  void Align(int where,int size)
  {
    if(size<_length)return;
    Resize(size);
    switch(where)
    {
      case apLeft:
      {
        memset(_str+_length,' ',size-_length);
        break;
      }
      case apCenter:
      {
        int left=(size-_length)/2;
        int right=size-_length-left;
        memmove(_str+left,_str,_length);
        memset(_str,' ',left);
        memset(_str+left+_length,' ',right);
        break;
      }
      case apRight:
      {
        int fill=size-_length;
        memmove(_str+fill,_str,_length);
        memset(_str,' ',fill);
        break;
      }
      case apWords:
      {
        int i=0;
        Trim();
        //while(isspace(_str[i]) && i<_length)i++;
        if(i==_length)return;
        int words=0;
        int spaces=0;
        while(i<_length)
        {
          while(!isspace(_str[i]) && i<_length)i++;
          while(isspace(_str[i]) && i<_length)
          {
            i++;
            spaces++;
          }
          words++;
        }
        if(words==1)
        {
          return;
        }
        int fill=size-_length+spaces;
        int perword=fill/(words-1);
        int rest=fill-perword*(words-1);
        int moreevery=rest?(words-1)/rest:-1;
        String old=*this;
        _length=0;
        i=0;
        int j,k=0,pad;
        while(i<old._length)
        {
          j=i;
          while(!isspace(old._str[i]) && i<old._length)i++;
          if(i==old._length)
          {
            memmove(_str+size-(i-j),old._str+j,i-j);
            break;
          }
          memmove(_str+_length,old._str+j,i-j);
          _length+=i-j;
          k++;
          pad=perword;
          if(k==moreevery)
          {
            pad++;
            k=0;
          }
          memset(_str+_length,' ',pad);
          _length+=pad;
          while(isspace(old._str[i]) && i<old._length)i++;
        }
        break;
      }
    }
    _length=size;
    _str[_length]=0;
  }

  String& ToLower(int from=0,int count=-1)
  {
    if(from<0)from=_length+from;
    if(from>_length)return *this;
    if(count==-1)count=_length-from;
    for(int i=from;i<from+count;i++)_str[i]=tolower(_str[i]);
    return *this;
  }
  String& ToUpper(int from=0,int count=-1)
  {
    if(from<0)from=_length+from;
    if(from>_length)return *this;
    if(count==-1)count=_length-from;
    for(int i=from;i<from+count;i++)_str[i]=toupper(_str[i]);
    return *this;
  }

  int ToInt()const
  {
    int rv=-1;
    sscanf(_str,"%d",&rv);
    return rv;
  }


  //! Load string from file
  int LoadFromFile(const char* filename)
  {
    FILE *f=fopen(filename,"rb");
    if(!f)return 0;
    fseek(f,0,SEEK_END);
    size_t size=ftell(f);
    fseek(f,0,SEEK_SET);
    SetSize(size);
    if(fread(_str,1,size,f)!=size)
    {
      fclose(f);
      return 0;
    };
    fclose(f);
    _str[size]=0;
    _length=size;
    return 1;
  }
  //! Save string to file
  int SaveToFile(const char* filename)
  {
    FILE *f=fopen(filename,"wb");
    if(!f)return 0;
    fwrite(_str,1,_length,f);
    fclose(f);
    return 1;
  }
};



#endif
