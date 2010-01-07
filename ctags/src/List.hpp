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

  Template class for doubly-connected list.
*/

#ifndef __LIST_HPP__
#define __LIST_HPP__

#ifndef __cplusplus
#error This header is for C++ only
#endif

/*! Template class dor doubly-connected list
*/

template <class T>
class List{
protected:
  typedef struct _tag_Node{
    _tag_Node *_prev,*_next;
    T _value;
  }_Node,*_NodePtr;
  _NodePtr _first,_last,_ptr;
  int _count,_index;
  _NodePtr *_fastindex;
  int _indexed,
  _indexsize;


public:
  /*! Structure used to store position of current element.
    \sa Save
    \sa Restore
  */

  typedef struct _tag_ListSave{
    _NodePtr _ptr;
    int _index;
  } ListSave;
  /*! Default constructor

      Create empty list.
  */
  List()
  {
    _indexed=_indexsize=_count=_index=0;
    _first=_last=_ptr=0;
    _fastindex=0;
  }
  /*! Copy constructor

      Create empty list and Clone \a src list.
      \param src is a source List to copy
      \sa Clone
  */
  List(const List<T>& src)
  {
    _indexed=_indexsize=_count=_index=0;
    _first=_last=_ptr=0;
    _fastindex=0;
    src.Clone(*this);
  }
  /*! Destructor

      Declared as virtual, to avoid problems in child classes
  */
  virtual ~List()
  {
    Clean();
  }

  /*! Clean all elements of list.
  */
  void Clean()
  {
    _ptr=_first;
    while(_ptr)
    {
      _first=_ptr->_next;
      delete _ptr;
      _ptr=_first;
    }
    if(_fastindex)delete [] _fastindex;
    _indexed=_indexsize=_count=_index=0;
    _first=_last=_ptr=0;
    _fastindex=0;
  }

  /*! Create copy of list in a \a dst list
     \param dst is destination List. It will be Cleaned before cloning.
  */
  void Clone(List<T>& dst)
  {
    ((const List<T>*)this)->Clone(dst);
  }
  /*! version of Clone for const this.
  */
  void Clone(List<T>& dst)const
  {
    _NodePtr ptr=_first;
    dst.Clean();
    while(ptr)
    {
      dst<<ptr->_value;
      ptr=ptr->_next;
    }
  }

  /*! Insert item at current position
    \param item is const ref to item object to insert
    \return reference to the inserted item in a list.
  */
  T& Insert(const T& item);

  /*! Insert src list at current lp
    \param src is source list that will be inserted.
    \return reference to *this
  */
  List<T>& Insert(const List<T> src)
  {
    _NodePtr ptr=src._first;
    while(ptr)
    {
      Insert(ptr->_value);
      Next();
      ptr=ptr->_next;
    }
    return *this;
  }
  /*! Append item to the end of list
      \return reference to appended item in a list
      \sa Push
  */
  T& Append(const T& item)
  {
    if(!_count)
    {
      _first=_last=_ptr=new _Node;
      _ptr->_prev=_ptr->_next=0;
    }else
    {
      _ptr=new _Node;
      _last->_next=_ptr;
      _ptr->_prev=_last;
      _ptr->_next=0;
      _last=_ptr;
    }
    _ptr->_value=item;
    _indexed=0;
    _count++;
    return _ptr->_value;
  }

  /*! Append another list to the end.
      \param src is a source list to append
      \return reference to *this
  */

  List<T>& Append(const List<T>& src)
  {
    _NodePtr ptr=src._first;
    while(ptr)
    {
      Append(ptr->_value);
      ptr=ptr->_next;
    }
    return *this;
  }

  /*! Much like Append.

      But return reference to *this
      \param item is an item to insert
      \return reference to *this
      \sa Append
  */
  List<T>& operator<<(const T& item)
  {
    Append(item);
    return *this;
  }

  /*! Exactly like Append for list argument.
  */

  List<T>& operator<<(const List<T>& src)
  {
    return Append(src);
  }

  /*! Exactly like operator<<
      \sa operator<<
  */
  List<T>& Push(const T& item){Append(item);return *this;}
  /*! Retreive last item
      \param item is a reference to the destination object.
      \return reference to *this
  */
  List<T>& Pop(T& item)
  {
    if(_count==0)return *this;
    item=Last().Get();
    _last=_last->_prev;
    delete _ptr;
    _ptr=_last;
    if(_last)_last->_next=0;
    _index--;
    _count--;
    if(!_count)_first=NULL;
    return *this;
  }
  /*! Retreive first item
     \param item is a reference to the destination object
     \return reference to *this
  */

  List<T>& Shift(T& item)
  {
    if(_count==0)return *this;
    item=First().Get();
    _first=_first->_next;
    delete _ptr;
    _ptr=_first;
    if(_first)_first->_prev=0;
    _count--;
    if(!_count)_last=NULL;
    return *this;
  }
  /*! Insert item at the begining of the list
    \param item is an object to insert
    \return reference to *this
  */
  List<T>& Unshift(const T& item)
  {
    First();
    Insert(item);
    return *this;
  }

  /*! Move current element one position forward
  */
  List<T>& operator++(){return Next();}
  /*! Move current element one position backward
  */
  List<T>& operator--(){return Prev();}
  /*! Move current element one position forward
  */
  List<T>& operator++(int){return Next();}
  /*! Move current element one position backward
  */
  List<T>& operator--(int){return Prev();}

  /*! Move current element to the beginning of the list
  */
  List<T>& First()
  {
    _ptr=_first;
    _index=0;
    return *this;
  }
  /*! Move current element to the end of the list
  */
  List<T>& Last()
  {
    _ptr=_last;
    _index=_count-1;
    return *this;
  }

  /*! Move current element one position forward
  */
  List<T>& Next()
  {
    if(_ptr!=_last)
    {
      _ptr=_ptr->_next;
      _index++;
    }
    return *this;
  }
  /*! Move current element one position backward
  */
  List<T>& Prev()
  {
    if(_ptr!=_first)
    {
      _ptr=_ptr->_prev;
      _index--;
    }
    return *this;
  }

  /*! \return Number of elements in a list
  */
  int Count(){return _count;}
  /*! \return position of current element in a list
  */
  int Index(){return _index;}

  /*! Set current element to the specified position
      \param index is desired position of current element
      if index is negative, it is counted from the end of list.
      i.e. -1 is last element in a list
      Index of first element is 0.
      \return reference to *this
  */
  List<T>& Goto(int index);

  /*! \return reference to value of current element of the list
  */
  T& Get(){return _ptr->_value;}
  /*! Get element at given position
     \param index behaves exactly like argument of List<T>::Goto
     \return reference to the specified element
     \sa Goto
     \sa Get
  */
  T& operator[](int index){return Goto(index).Get();}

  /*! Copy list

      Same as src.Clone(*this)
      \sa Clone
  */
  void operator=(const List<T>& src)
  {
    src.Clone(*this);
  }

  /*! Delete current element
  */
  virtual void Delete()
  {
    if(!_count)return;
    if(_ptr==_first)
    {
      _first=_first->_next;
      delete _ptr;
      if(_first)_first->_prev=0;
      _ptr=_first;
    }else
    if(_ptr==_last)
    {
      _last=_last->_prev;
      delete _ptr;
      if(_last)_last->_next=0;
      _ptr=_last;
    }else
    {
      _ptr->_prev->_next=_ptr->_next;
      _ptr->_next->_prev=_ptr->_prev;
      _NodePtr tmp=_ptr->_next;
      delete _ptr;
      _ptr=tmp;
    }
    _count--;
    _indexed=0;
  }

  /*! Delete specified amount of items starting from current position
    \param count is number of elements to delete
  */
  void Delete(int count)
  {
    _indexed=0;
    for(int i=0;i<count;i++)Delete();
  }

  /*! Save current position

    \param save is a reference to the corresponding structure
    \sa Restore
  */
  void Save(ListSave& save)
  {
    save._ptr=_ptr;
    save._index=_index;
  }

  /*! Restore current position from saved.

      \param save is a constant reference to corresponding structure.
  */
  void Restore(const ListSave& save)
  {
    _ptr=save._ptr;
    _index=save._index;
  }

  /*! Create index of list elements.

      If a lot of operation that require repositioning
      of current element without modifications of the list
      are expected, than it is highly recommended to
      call Indexate. With index access to the list
      elements with [] operator perform like an array.
  */
  void Indexate()
  {
    if(_fastindex)
    {
      if(_indexsize<_count)
      {
        delete [] _fastindex;
        _fastindex=new _NodePtr[_count];
        _indexsize=_count;
      }
    }else
    {
      _fastindex=new _NodePtr[_count];
      _indexsize=_count;
    }
    ListSave save;
    Save(save);
    First();
    for(int i=0;i<_count;i++)
    {
      _fastindex[i]=_ptr;
      Next();
    }
    _indexed=1;
  }

  /*! Exchange list items.

      \param index is position of list element
      than need to be exchanged with current.
      \return reference to this*
  */
  virtual List<T>& Exchange(int index=-1);
};

template <class T>
T& List<T>::Insert(const T& item)
{
  _indexed=0;
  if(!_count)
  {
    _ptr=_first=_last=new _Node;
    _ptr->_next=_ptr->_prev=0;
    _ptr->_value=item;
  }else
  {
    _NodePtr tmp=new _Node;
    if(_ptr==_first)
    {
      tmp->_prev=0;
      tmp->_next=_first;
      _first->_prev=tmp;
      _ptr=_first=tmp;
    }else
        {
/*    if(_ptr==_last)
    {
      tmp->_next=0;
      tmp->_prev=_last;
      _last->_next=tmp;
      _last=_ptr=tmp;
    }else
    {*/
      tmp->_prev=_ptr->_prev;
      tmp->_next=_ptr;
      if(_ptr->_prev)_ptr->_prev->_next=tmp;
      _ptr->_prev=tmp;
      _ptr=tmp;
//    }
        }
    tmp->_value=item;
  }
  _count++;
  return _ptr->_value;
}

template <class T>
List<T>& List<T>::Goto(int index)
{
  if(!_count)return *this;
  if(index<0)index+=_count;
  if(index<0)index=0;
  if(index>=_count)index=_count-1;
  if(_indexed)
  {
    _index=index;
    _ptr=_fastindex[index];
    return *this;
  }

  int r1=index-_index;
  if(r1<0)r1=-r1;
  int r2=_count-1-index;
  if(r1<r2)
  {
    if(r1<index)
    {
      if(_index<index)
      {
        while(r1--)Next();
      }else
      {
        while(r1--)Prev();
      }
    }else
    {
      First();
      while(index--)Next();
    }
  }else
  {
    if(r2<index)
    {
      Last();
      while(r2--)Prev();
    }else
    {
      First();
      while(index--)Next();
    }
  }
  return *this;
}

template<class T>
List<T>& List<T>::Exchange(int index)
{
  if(index<0)index+=_count;
  if(index<0)index=0;
  if(index>=_count)index=_count-1;
  if(_index==index)return *this;
  int oldindex=_index;
  _NodePtr tmp=_ptr,next,prev;
  Goto(index);
  if(oldindex>_index)
  {
    _NodePtr tmp2=_ptr;
    _ptr=tmp;
    tmp=tmp2;
    _index=oldindex;
    oldindex=index;
  }
  if(tmp->_next==_ptr)
  {
    if(tmp->_prev)tmp->_prev->_next=_ptr;
    if(_ptr->_next)_ptr->_next->_prev=tmp;
    tmp->_next=_ptr->_next;
    _ptr->_prev=tmp->_prev;
    tmp->_prev=_ptr;
    _ptr->_next=tmp;
  }else
  {
    if(tmp->_prev)tmp->_prev->_next=_ptr;
    tmp->_next->_prev=_ptr;
    if(_ptr->_next)_ptr->_next->_prev=tmp;
    _ptr->_prev->_next=tmp;
    next=tmp->_next;
    prev=tmp->_prev;
    tmp->_next=_ptr->_next;
    tmp->_prev=_ptr->_prev;
    _ptr->_next=next;
    _ptr->_prev=prev;
  }
  if(tmp==_first)_first=_ptr;
  if(_ptr==_last)_last=tmp;
  if(_indexed)
  {
    _fastindex[oldindex]=_ptr;
    _fastindex[_index]=tmp;
  }
  _index=oldindex;
  return *this;
}

/*! This template class designed to hold pointers to objects.

    Both destructor and Delete() method will destroy
    elements.
*/

template <class T>
class PtrList:public List<T>{
public:
  virtual ~PtrList()
  {
    this->_ptr=this->_first;
    while(this->_ptr)
    {
      this->_first=this->_ptr->_next;
      delete this->_ptr->_value;
      delete this->_ptr;
      this->_ptr=this->_first;
    }
    if(this->_fastindex)delete [] this->_fastindex;
    this->_indexed=this->_indexsize=this->_count=this->_index=0;
    this->_first=this->_last=this->_ptr=0;
    this->_fastindex=0;
  }
  virtual void Delete()
  {
    if(!this->_count)return;
    if(this->_ptr==this->_first)
    {
      this->_first=this->_first->_next;
      delete this->_ptr->_value;
      delete this->_ptr;
      if(this->_first)this->_first->_prev=0;
      this->_ptr=this->_first;
    }else
    if(this->_ptr==this->_last)
    {
      this->_last=this->_last->_prev;
      delete this->_ptr->_value;
      delete this->_ptr;
      if(this->_last)this->_last->_next=0;
      this->_ptr=this->_last;
    }else
    {
      this->_ptr->_prev->_next=this->_ptr->_next;
      this->_ptr->_next->_prev=this->_ptr->_prev;
      typename List<T>::_NodePtr tmp=this->_ptr->_next;
      delete this->_ptr->_value;
      delete this->_ptr;
      this->_ptr=tmp;
    }
    this->_count--;
    this->_indexed=0;
  }
  virtual List<T>& Exchange(int index)
  {
    if(index<0)index+=this->_count;
    if(index<0)index=0;
    if(index>=this->_count)index=this->_count-1;
    if(index==this->_index)return *this;
    int oldindex=this->_index;
    typename List<T>::_NodePtr tmp=this->_ptr;
    typename List<T>::ListSave sv;
    Save(sv);
    this->Goto(index);
    T tmpval=tmp->_value;
    tmp->_value=this->_ptr->_value;
    this->_ptr->_value=tmpval;
    Restore(sv);
    return *this;
  }
};

#endif
