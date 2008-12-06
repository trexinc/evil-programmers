#ifndef __TArray_hpp
#define __TArray_hpp

// Object ������ ����� ����������� �� ��������� � ��������� ���������
//  bool operator==(const Object &) const
//  bool operator<(const Object &) const
//  const Object& operator=(const Object &)

typedef int __cdecl (*TARRAYCMPFUNC)(const void *el1,const void *el2);

template <class Object>
class TArray
{
  private:
    unsigned int internalCount, Count, Delta;
    Object **items;

  private:
    static int __cdecl CmpItems(const Object **el1, const Object **el2);
    bool deleteItem(unsigned int index);

  public:
    TArray(unsigned int Delta=8);
    ~TArray() { Free(); }
    TArray<Object>(const TArray<Object> &rhs);
    TArray& operator=(const TArray<Object> &rhs);

  public:
    void Free();
    void setDelta(unsigned int newDelta);
    bool setSize(unsigned int newSize);
    Object *setItem(unsigned int index, const Object &newItem);
    Object *getItem(unsigned int index) const;
    int getIndex(const Object &item, int start=-1);
    void Sort(TARRAYCMPFUNC user_cmp_func=NULL); // ������������� ������
    bool Pack(); // ��������� ������ - ������ ���������� ���������� ���������,
                 // ������ ������, �������� ������ ����. ����������, false,
                 // ���� ��������� ������� �� �������������.
                 // ����� Pack() ����� Sort(NULL) �������� � ����������
                 // ����������

  public: // inline
    unsigned int getSize() const { return Count; }
    Object *addItem(const Object &newItem) { return setItem(Count,newItem); }
};

#endif // __TArray_hpp
