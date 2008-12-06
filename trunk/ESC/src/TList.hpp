/*
    [ESC] Editor's settings changer plugin for FAR Manager
    Copyright (C) 2001 Ivan Sintyurin
    Copyright (C) 2008 Alex Yaroslavsky

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
#ifndef __TLIST_HPP__
#define __TLIST_HPP__

/*  TList.hpp
    ������� ������ ������ � ���������� ������� / by Spinoza /
    Object ������ ����� ����������� �� ��������� � ��������� ���������:
      const Object& operator=(const Object &)
*/

#if defined(__BORLANDC__)
  #pragma warn -inl
#endif

template <class Object>
class TList
{
  private:
    struct OneItem
    {
      Object  Item;
      OneItem *Prev, *Next;
    }
      *First, *Last, *Current, *Tmp, *SavedPos;

    DWORD Size;

  public:
    TList(const TList& rhs)
    {
      *this=rhs;
    }
    TList(): First(NULL), Last(NULL), Current(NULL), SavedPos(0), Size(0)
      {}
    ~TList() { clear(); }

  public:
    // �������� ���������� ��������� � ������
    DWORD size() const { return Size; }

    // ���������� TRUE, ���� ������ ����
    BOOL empty() const { return !Size; }

    //  �������� ��������� �� ������� ������� ������ (���������� NULL ��� �������)
    const Object *getItem() { return Current?&Current->Item:NULL; }

    // ���������� TRUE, ���� ������� ������� ����������� �� ������ �������
    BOOL isBegin() const { return Current==First; }

    // ���������� TRUE, ���� ������� ������� ����������� �� ��������� �������
    BOOL isEnd() const { return Current==Last; }

    // ���� � ������ ������ � ������� ��������� �� ������ �������
    // ������������ NULL, ���� ������ ����
    const Object *toBegin()
    {
      Current=First;
      return Current?&Current->Item:NULL;
    }

    // ���� � ����� ������ � ������� ��������� �� ��������� �������
    // ������������ NULL, ���� ������ ����
    const Object *toEnd()
    {
      Current=Last;
      return Current?&Current->Item:NULL;
    }

    // ���� � ���������� �������� ������ � ������� ��������� �� ����
    // (���������� NULL, ���� ��������� ����� ������ ��� ������� �������
    // �� ��� ���������)
    const Object *toNext()
    {
      return (Current && NULL!=(Current=Current->Next))?&Current->Item:NULL;
    }

    // ���� � ����������� �������� ������ � ������� ��������� �� ����
    // (���������� NULL, ���� ���������� ������ ������ ��� ������� �������
    // �� ��� ���������)
    const Object *toPrev()
    {
      return (Current && NULL!=(Current=Current->Prev))?&Current->Item:NULL;
    }

    // �������� ������� � ������ ������
    // ������� ������� ��� ������ ��������������� �� ���� �������
    // ��� ������� ������������ FALSE
    BOOL push_front(const Object &Source)
    {
      SavedPos=NULL;
      Tmp=new OneItem;
      if(Tmp) // ���������, ���� �� ����� ����������
      {
        Tmp->Item=Source;
        Tmp->Prev=NULL;
        if(First)
          First->Prev=Tmp;
        Tmp->Next=First;
        First=Current=Tmp;
        if(!Last) // ������ �� �������� ��� ����
          Last=First;
        ++Size;
        return TRUE;
      }
      return FALSE;
    }

    // �������� ������� � ����� ������
    // ������� ������� ��� ������ ��������������� �� ���� �������
    // ��� ������� ������������ FALSE
    BOOL push_back(const Object &Source)
    {
      SavedPos=NULL;
      Tmp=new OneItem;
      if(Tmp) // ���������, ���� �� ����� ����������
      {
        Tmp->Item=Source;
        if(Last)
          Last->Next=Tmp;
        Tmp->Prev=Last;
        Tmp->Next=NULL;
        Last=Current=Tmp;
        if(!First) // ������ �� �������� ��� ����
          First=Last;
        ++Size;
        return TRUE;
      }
      return FALSE;
    }

    // �������� ������� ����� ������� ������� � ������
    // ���� ������� ������� �� ����������, �� ������� ����������� � ����� ������ (=push_back)
    // ������� ������� ��� ������ ��������������� �� ����� �������
    // ��� ������� ������������ FALSE
    BOOL insert(const Object &Source)
    {
      if(!Current)
        return push_back(Source);
      SavedPos=NULL;
      Tmp=new OneItem;
      if(Tmp) // ���������, ���� �� ����� ����������
      {
        if(isEnd())
          Last=Tmp;
        Tmp->Item=Source;
        Tmp->Next=Current->Next;
        Tmp->Prev=Current;
        Current->Next=Tmp;
        Current=Tmp;
        ++Size;
        return TRUE;
      }
      return FALSE;
    }

    // ������� �������, ������� ������� ��������������� �� ��������� �������
    // ���� ������� ������� �� �������� �� ����������, �� ������������ FALSE
    BOOL removeToEnd()
    {
      SavedPos=NULL;
      if (Current)
      {
        if(isEnd())
          Last=Last->Prev;
        Tmp=Current->Next;
        delete Current;
        Current=Tmp;
        --Size;
        if(!Size)
          First=Last=NULL;
        return TRUE;
      }
      return FALSE;
    }

    // ������� removeToEnd
    BOOL erase() { return removeToEnd(); }

    // ������� �������, ������� ������� ��������������� �� ���������� �������
    // ���� ������� ������� �� �������� �� ����������, �� ������������ FALSE
    BOOL removeToBegin()
    {
      SavedPos=NULL;
      if (Current)
      {
        if(isBegin())
          First=First->Next;
        Tmp=Current->Prev;
        delete Current;
        Current=Tmp;
        --Size;
        if(!Size)
          First=Last=NULL;
        return TRUE;
      }
      return FALSE;
    }

    // �������� ������ �� ���������� ������ lst � ��������
    void swap(TList<Object> &lst)
    {
      if(this!=&lst)
      {
        OneItem *newFirst=lst.First, *newLast=lst.Last,
                *newCurrent=lst.Current, *newSavedPos=lst.SavedPos;
        lst.First=First;
        lst.Last=Last;
        lst.Current=Current;
        lst.SavedPos=SavedPos;
        First=newFirst;
        Last=newLast;
        Current=newCurrent;
        SavedPos=newSavedPos;
      }
    }

    // ��������� ������� ������� (��. restorePosition) ��� ������������
    // ��������������. ������������ FALSE, ���� ������� ������� ����
    // ��������������
    BOOL storePosition()
    {
      SavedPos=Current;
      return SavedPos!=NULL;
    }

    // ������������ ������� ������� (��. storePosition) ��� ������������
    // ��������������. ������������ FALSE, ���� ������� ������� �����
    // ��������������
    BOOL restorePosition()
    {
      Current=SavedPos;
      return Current!=NULL;
    }

    // �������� ������
    void clear()
    {
      toBegin();
      while(erase())
        ;
      SavedPos=NULL;
    }

    // �������� ������ �� ���������� ������ rhs, ������ �� ��������
    // (��. ��� ����� swap)
    TList& operator=(const TList& rhs)
    {
      if(this!=&rhs)
      {
        clear();
        OneItem *value=rhs.First, *newCurrent=NULL, *newSave=NULL;
        while(value)
        {
          if(!push_back(value->Item))
            break;
          if(rhs.Current==value)
            newCurrent=Current;
          if(rhs.SavedPos==value)
            newSave=Current;
          value=value->Next;
        }
        Current=newCurrent;
        SavedPos=newSave;
      }
      return *this;
    }
};

#if defined(__BORLANDC__)
  #pragma warn .inl
#endif

#endif // __TLIST_HPP__
