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
  const Node *BaseNode; // указатель на текущий элемент при их переборе (GetNext)
  BOOL BaseNodeFirst; // TRUE, если GetNext вызывается в первый раз после First
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
  void Empty(); /* удалить все элементы из дерева */
  BOOL IsEmpty() const { return root==NIL; } // "пустое ли дерево?"
  Node *findNode(const Object &data);   /* find node containing data */
  void deleteNode(Node * z);    /* delete node z from tree */
  Node *insertNode(const Object &data); /* allocate node for data and insert in tree */
  void First();      /* вызывать перед GetNext, инициализирует итератор */
  Object *GetNext()  /* возвращает адрес очередного элемента или NULL */
    { return const_cast<Object*>(internalGetNext(NIL)); }
};

#endif // __rbtree_hpp
