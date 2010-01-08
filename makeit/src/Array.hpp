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

  Template class for quantified array.
*/

#ifndef __ARRAY_HPP__
#define __ARRAY_HPP__

#ifndef __cplusplus
#error This header is for use with C++ only
#endif

#include <new>
#include <string.h>
#include <algorithm>

class ArrayIndexOutOfRange{};

inline int DefaultStorageIncrement(int oldsize)
{
  if(oldsize<16)return oldsize+4;
  if(oldsize<256)return oldsize+16;
  return oldsize+oldsize/2;
}


template <class T,int (*Increment)(int)=DefaultStorageIncrement>
class Vector{
protected:
  T* _data;
  int _count;
  int _size;
  int _empty;
  static const int SZ;

  inline T& Obj(int index=0)
  {
    return _data[index];
  }

  void Resize(int newsize,int emptysize=0)
  {
    if(newsize<=_size)return;
    if(newsize<=_size+_empty && _size+_empty-newsize>_size/32)
    {
      int sh=newsize-_size;
      if(sh<_empty/2)sh=_empty/2;
      _data-=sh;
      //Copy(sh,0,_count);
      memcpy(_data,_data+sh,SZ*_count);
      _size+=sh;
      _empty-=sh;
      return;
    }
    newsize=Increment(newsize);
    emptysize=emptysize>_empty/2?emptysize:_empty/2;
    T* newdata=new T[newsize+emptysize];
    newdata+=emptysize;
    if(_data)memcpy(newdata,_data,SZ*_count);
    _size=newsize;
    if(_data)delete [] (_data-_empty);
    _empty=emptysize;
    _data=newdata;
  }
  void Copy(int from,int to,int count)
  {
    memmove(_data+to,_data+from,SZ*count);
  }
public:
  Vector():_data(0),_count(0),_size(0),_empty(0){}
  explicit Vector(int n):_count(0),_size(n),_empty(0)
  {
    _data=new T[n];
  }
  Vector(const Vector& src)
  {
    _data=new T[src.Count()];
    memcpy(_data,src._data,SZ*src.Count());
    _size=src.Count();
    _count=_size;
    _empty=0;
  }
  virtual ~Vector()
  {
    Clean();
  }

  void operator=(const Vector& src)
  {
    if(this==&src)return;
    Clean();
    _data=new T[src.Count()];
    memcpy(_data,src._data,SZ*src.Count());
    _size=src.Count();
    _count=_size;
  }

  T& operator[](int index)
  {
    if(index<0)index+=_count;
    if(index>=_count)throw ArrayIndexOutOfRange();
    return _data[index];
  }
  const T& operator[](int index)const
  {
    if(index<0)index+=_count;
    if(index>=_count)throw ArrayIndexOutOfRange();
    return _data[index];
  }

  T& Last()
  {
    return _data[_count-1];
  }
  const T& Last()const
  {
    return _data[_count-1];
  }


  int Push(const T& item)
  {
    Resize(_count+1);
    _data[_count]=item;
    _count++;
    return _count;
  }

  int Pop(T& item)
  {
    if(_count==0)return 0;
    _count--;
    item=Obj(_count);
    return _count;
  }

  int Unshift(const T& item)
  {
    if(_empty)
    {
      _data--;
      _empty--;
      _data[0]=item;
      _size++;
      _count++;
    }else
    {
      Resize(_count+1,Increment(_count)-_count);
      if(_empty)
      {
        _data--;
        _empty--;
        _data[0]=item;
        _size++;
        _count++;
      }else
      {
        Copy(0,1,_count);
        _data[0]=item;
        _count++;
      }
    }
    return _count;
  }

  int Shift(T& item)
  {
    if(_count==0)return 0;
    item=Obj();
    _data++;
    _empty++;
    _size--;
    _count--;
    return _count;
  }

  int Insert(int index,const T& item)
  {
    if(index<0)index+=_count;
    if(index<0)return -1;
    if(index>_count)
    {
      Resize(index+1);
      _data[index]=item;
      _count=index+1;
      return _count;
    }
    if(_empty && index<_count/2)
    {
      _data--;
      _empty--;
      _size++;
      Copy(1,0,index);
      _data[index]=item;
      _count++;
      return _count;
    }
    Resize(_count+1,index<_count/2?Increment(_count)-_count:0);
    Copy(index,index+1,_count-index);
    _data[index]=item;
    _count++;
    return _count;
  }

  int Insert(int index,const Vector& src)
  {
    if(index<0)index+=_count;
    if(index<0)return -1;
    if(index>_count)
    {
      Resize(index+src.Count());
      memcpy(_data+index,src._data,SZ*src.Count());
      _count=index+src.Count();
    }else
    {
      Resize(_count+src.Count());
      memcpy(_data+index,src._data,SZ*src.Count());
      _count+=src.Count();
    }
    return _count;
  }

  int Delete(int index,int count=1)
  {
    if(index<0)index+=_count;
    if(index<0)
    {
      if (count==-1)
      {
        count=_count;
        index=0;
      }else
      {
        count-=index;
        index=0;
      }
    }else
    {
      if(count==-1)count=_count-index;
    }
    if(index>=_count)return 0;
    if(index+count>=_count)
    {
      _count=index;
      return _count-index;
    }
    if(index<_count-index-count)
    {
      Copy(0,count,index);
      _data+=count;
      _empty+=count;
      _size-=count;
    }else
    {
      Copy(index+count,index,_count-(index+count));
    }
    _count-=count;
    return count;
  }

  void Swap(Vector& src)
  {
    T* data=_data;
    int count=_count;
    int size=_size;
    int empty=_empty;
    _data=src._data;
    _count=src._count;
    _size=src._size;
    _empty=src._empty;
    src._data=data;
    src._count=count;
    src._size=size;
    src._empty=empty;
  }

  void Swap(int from,int to)
  {
    T t=_data[from];
    from=_data[to];
    _data[to]=t;
  }

  void Reverse()
  {
    int i;
    T t;
    for(i=0;i<_count;i++)
    {
      t=_data[i];
      _data[i]=_data[_count-1-i];
      _data[_count-1-i]=t;
    }
  }

  int Count(){return _count;}
  int Count()const{return _count;}

  void Clean()
  {
    if(_data)
    {
      delete [] (_data-_empty);
    }
    _data=0;
    _size=0;
    _count=0;
    _empty=0;
  }

  int SetSize(int count)
  {
    int oldsize=_size;
    Resize(count);
    return oldsize;
  }

  void Init(int count)
  {
    if(count<=_count)return;
    T *newdata=new T[count];
    memcpy(newdata,_data,SZ*_count);
    delete [] (_data-_empty);
    _count=count;
    _size=_count;
    _data=newdata;
    _empty=0;
  }

  void Fill(int count,const T& value)
  {
    Resize(count);
    _count=count;
    T* ptr=_data;
    for(int i=0;i<count;i++,ptr++)
    {
      *ptr=value;
    }
  }

  int Shrink()
  {
    if(_count==_size)return _size;
    T *newdata=new T[_count];
    memcpy(newdata,_data,SZ*_count);
    delete [] (_data-_empty);
    int oldsize=_size;
    _size=_count;
    _data=newdata;
    _empty=0;
    return oldsize;
  }
  void Sort()
  {
    if(_size==0 || _count==0)return;
    std::sort(_data,_data+_count);
  }
  template <class P>
  void Sort(const P& p)
  {
    if(_size==0 || _count==0)return;
    std::sort(_data,_data+_count,p);
  }

};

template <class T,int (*Increment)(int)>
const int Vector<T,Increment>::SZ=sizeof(T);

template <class T,int (*Increment)(int)=DefaultStorageIncrement>
class Array{
protected:
  T** _data;
  int _count;
  int _size;
  int _empty;

  inline T* Ptr(int index)const
  {
    return _data[index];
  }
  inline T& Obj(int index=0)const
  {
    return *Ptr(index);
  }

  inline void DestroyRange(int from,int to)
  {
    for(int i=from;i<to;i++)delete _data[i];
  }

  inline void InitRange(int from,int to)
  {
    for(int i=from;i<to;i++)_data[i]=new T();
  }

  void Resize(int newsize)
  {
    if(newsize<=_size)return;
    if(newsize<=_size+_empty)
    {
      int sh=newsize-_size;
      _data-=sh;
      Copy(sh,0,_count);
      _size+=sh;
      _empty-=sh;
      return;
    }
    newsize=Increment(newsize);
    T** newdata=new T*[newsize+_empty/2];
    newdata+=_empty/2;
    memcpy(newdata,_data,sizeof(T*)*_count);
    _size=newsize;
    delete [] (_data-_empty);
    _empty/=2;
    _data=newdata;
  }
  void Copy(int from,int to,int count)
  {
    memmove(_data+to,_data+from,sizeof(T*)*count);
  }
public:
  Array():_data(0),_count(0),_size(0),_empty(0){}
  explicit Array(int n):_count(0),_size(n),_empty(0)
  {
    _data=new T*[n];
  }
  Array(const Array& src)
  {
    _data=new T*[src.Count()];
    int i;
    for(i=0;i<src.Count();i++)
    {
      _data[i]=new T(src[i]);
    }
    _size=src.Count();
    _count=_size;
    _empty=0;
  }
  virtual ~Array()
  {
    Clean();
  }

  void operator=(const Array& src)
  {
    if(this==&src)return;
    Clean();
    _data=new T*[src.Count()];
    int i;
    for(i=0;i<src.Count();i++)
    {
      _data[i]=new T(src[i]);
    }
    _size=src.Count();
    _count=_size;
  }

  T& operator[](int index)
  {
    if(index<0)index+=_count;
    if(index<0 || index>=_count)throw ArrayIndexOutOfRange();
    return Obj(index);
  }
  const T& operator[](int index)const
  {
    if(index<0)index+=_count;
    if(index<0 || index>=_count)throw ArrayIndexOutOfRange();
    return Obj(index);
  }

  int Push(const T& item)
  {
    Resize(_count+1);
    _data[_count]=new T(item);
    _count++;
    return _count;
  }

  int Pop(T& item)
  {
    if(_count==0)return 0;
    _count--;
    item=Obj(_count);
    delete _data[_count];
    return _count;
  }

  int Unshift(const T& item)
  {
    if(_empty)
    {
      _data--;
      _empty--;
      _data[0]=new T(item);
      _size++;
      _count++;
    }else
    {
      Resize(_count+1);
      Copy(0,1,_count);
      _data[0]=new T(item);
      _count++;
    }
    return _count;
  }

  int Shift(T& item)
  {
    if(_count==0)return 0;
    item=Obj();
    delete _data[0];
    _data++;
    _empty++;
    _size--;
    _count--;
    return _count;
  }

  T* Extract(int index)
  {
    if(index<0)index+=_count;
    if(index>=_count)return 0;
    T* retval=_data[index];
    if(index>_count/2)
    {
      Copy(index+1,index,_count-(index+1));
    }else
    {
      if(index!=0)Copy(0,1,index);
      _data++;
      _empty++;
      _size--;
    }
    _count--;
    return retval;
  }

  int Insert(int index,const T& item)
  {
    if(index<0)index+=_count;
    if(index<0)return -1;
    if(index>_count)
    {
      Resize(index+1);
      InitRange(_count,index);
      _data[index]=new T(item);
      _count=index+1;
      return _count;
    }
    Resize(_count+1);
    Copy(index,index+1,_count-index);
    _data[index]=new T(item);
    _count++;
    return _count;
  }

  int Insert(int index,const Array& src)
  {
    if(index<0)index+=_count;
    if(index<0)return -1;
    if(index>_count)
    {
      Resize(index+src.Count());
      int i;
      InitRange(_count,index);
      for(i=0;i<src.Count();i++)
      {
        _data[index+i]=new T(src[i]);
      }
      _count=index+src.Count();
    }else
    {
      Resize(_count+src.Count());
      Copy(index,index+src.Count(),src.Count());
      int i;
      for(i=0;i<src.Count();i++)
      {
        _data[index+i]=new T(src[i]);
      }
      _count+=src.Count();
    }
    return _count;
  }

  int Delete(int index,int count=1)
  {
    if(index<0)index+=_count;
    if(index<0)
    {
      if (count==-1)
      {
        count=_count;
        index=0;
      }else
      {
        count-=index;
        index=0;
      }
    }else
    {
      if(count==-1)count=_count-index;
    }
    if(index>=_count)return 0;
    if(index+count>=_count)
    {
      DestroyRange(index,_count);
      _count=index;
      return _count-index;
    }
    DestroyRange(index,index+count);
    Copy(index+count,index,_count-(index+count));
    _count-=count;
    return count;
  }

  void Swap(Array& src)
  {
    char* data=_data;
    int count=_count;
    int size=_size;
    int empty=_empty;
    _data=src._data;
    _count=src._count;
    _size=src._size;
    _empty=src._empty;
    src._data=data;
    src._count=count;
    src._size=size;
    src._empty=empty;
  }

  void Swap(int from,int to)
  {
    T* t;
    t=_data[from];
    _data[from]=_data[to];
    _data[to]=t;
  }

  void Reverse()
  {
    int i;
    T *t;
    for(i=0;i<_count;i++)
    {
      t=_data[i];
      _data[i]=_data[_count-1-i];
      _data[_count-1-i]=t;
    }
  }

  int Count(){return _count;}
  int Count()const{return _count;}

  void Clean()
  {
    if(_data)
    {
      DestroyRange(0,_count);
      delete [] (_data-_empty);
    }
    _data=0;
    _size=0;
    _count=0;
    _empty=0;
  }

  int SetSize(int count)
  {
    int oldsize=_size;
    Resize(count);
    return oldsize;
  }

  void Init(int count)
  {
    if(count<=_count)return;
    Resize(count);
    InitRange(_count,count);
    _count=count;
  }

  int Shrink()
  {
    if(_size==_count)return _size;
    T** newdata=new T*[_count];
    int i;
    for(i=0;i<_count;i++)
    {
      newdata[i]=_data[i];
    }
    delete [] (_data-_empty);
    int oldsize=_size;
    _size=_count;
    _data=newdata;
    _empty=0;
    return oldsize;
  }
  struct ArraySorter{
    bool operator()(T* a,T* b)
    {
      return *a<*b;
    }
  };
  void Sort()
  {
    if(_size==0 || _count==0)return;
    std::sort(_data,_data+_count,ArraySorter());
  }
  template <class P>
  void Sort(const P& p)
  {
    if(_size==0 || _count==0)return;
    std::sort(_data,_data+_count,p);
  }
};


#endif
