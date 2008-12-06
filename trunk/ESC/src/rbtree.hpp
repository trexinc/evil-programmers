#ifndef __rbtree_hpp
#define __rbtree_hpp

/* Red-Black tree description */
typedef enum
{ BLACK, RED }
nodeColor;

template <class Object>
struct Node_
{
  Node_<Object> *left;           /* left child */
  Node_<Object> *right;          /* right child */
  Node_<Object> *parent;         /* parent */
  nodeColor color;               /* node color (BLACK, RED) */
  Object data;                   /* data stoRED in node */
};

#ifdef NIL
#error NIL is already defined
#endif

#define NIL &sentinel           /* all leafs are sentinels */

template <class Object>
class CRedBlackTree
{
private:
  typedef Node_<Object> Node;
  const Node *BaseNode; // 㪠��⥫� �� ⥪�騩 ����� �� �� ��ॡ�� (GetNext)
  BOOL BaseNodeFirst; // TRUE, �᫨ GetNext ��뢠���� � ���� ࠧ ��᫥ First
  Node sentinel;
  Node *root;                   /* root of Red-Black tree */
  typedef BOOL (WINAPI *COMP)(const Object & a, const Object & b);
  COMP compEQ, compLT;
  void DeleteTree(Node * P);
  void deleteFixup(Node * x);   /* maintain Red-Black tree balance after
                                   deleting node x */
  void insertFixup(Node * x);   /* maintain Red-Black tree balance after
                                   inserting node x */
  void rotateRight(Node * x);   /* rotate node x to right */
  void rotateLeft(Node * x);    /* rotate node x to left */
  const Object *internalGetNext(const Node *nil);

public:
  CRedBlackTree(COMP compLT, COMP compEQ);
  ~CRedBlackTree();
  CRedBlackTree<Object>(const CRedBlackTree<Object> &rhs);
  CRedBlackTree& operator=(const CRedBlackTree<Object> &rhs);

public:
  void Empty(); /* 㤠���� �� ������ �� ��ॢ� */
  BOOL IsEmpty() const { return root==NIL; } // "���⮥ �� ��ॢ�?"
  Node *findNode(const Object &data);   /* find node containing data */
  void deleteNode(Node * z);    /* delete node z from tree */
  Node *insertNode(const Object &data); /* allocate node for data and insert in tree */
  void First();      /* ��뢠�� ��। GetNext, ���樠������� ����� */
  Object *GetNext()  /* �����頥� ���� ��।���� ����� ��� NULL */
    { return const_cast<Object*>(internalGetNext(NIL)); }
};

#endif // __rbtree_hpp
