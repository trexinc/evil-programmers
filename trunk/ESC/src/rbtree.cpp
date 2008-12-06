#ifndef __rbtree_cpp
#define __rbtree_cpp

#include "myrtl.hpp"
#include "rbtree.hpp"
#include "syslog.hpp"

//#define compLT(a,b) (a < b)
//#define compEQ(a,b) (a == b)

template <class Object>
CRedBlackTree<Object>::CRedBlackTree(COMP complt, COMP compeq)
{
  compLT=complt;
  compEQ=compeq;
  sentinel.left = sentinel.right = NIL;
  sentinel.parent = 0;
  sentinel.color = BLACK;
  root = NIL;
  BaseNode = NIL;
  BaseNodeFirst = TRUE;
}

template <class Object>
CRedBlackTree<Object>::CRedBlackTree(const CRedBlackTree<Object> &rhs)
{
  compLT=rhs.compLT;
  compEQ=rhs.compEQ;
  sentinel.left = sentinel.right = NIL;
  sentinel.parent = 0;
  sentinel.color = BLACK;
  root = NIL;
  BaseNode = NIL;
  BaseNodeFirst = TRUE;
  *this=rhs;
}

template <class Object>
CRedBlackTree<Object>& CRedBlackTree<Object>::operator=(const CRedBlackTree<Object> &rhs)
{
  if(this!=&rhs)
  {
    Empty();
    compLT=rhs.compLT;
    compEQ=rhs.compEQ;
    if(!rhs.IsEmpty())
    {
      const Object *link;
      BaseNode=rhs.root;
      const Node *nil=&rhs.sentinel;
      while(BaseNode->left!=nil) BaseNode=BaseNode->left;
      BaseNodeFirst=TRUE;
      while(NULL!=(link=internalGetNext(nil)))
        insertNode(*link);
    }
    BaseNode = NIL;
    BaseNodeFirst = TRUE;
  }
  return *this;
}

template <class Object>
CRedBlackTree<Object>::~CRedBlackTree()
{
  Empty();
}

template <class Object>
void CRedBlackTree<Object>::Empty()
{
  DeleteTree(root);
  root = NIL;
  BaseNode = NIL;
  BaseNodeFirst = TRUE;
}

template <class Object>
void CRedBlackTree<Object>::DeleteTree(Node * P)
{
  if (P == NIL)
    return;
  DeleteTree(P->left);
  P->left = NIL;
  DeleteTree(P->right);
  P->right = NIL;
  delete P;
}

template <class Object>
void CRedBlackTree<Object>::First()
{
   BaseNode=root;
   while(BaseNode->left!=NIL) BaseNode=BaseNode->left;
   BaseNodeFirst=TRUE;
}

template <class Object>
const Object *CRedBlackTree<Object>::internalGetNext(const Node *nil)
{
   if(BaseNode==nil) return NULL;

   if(BaseNodeFirst)
   {
     BaseNodeFirst=FALSE;
     return &BaseNode->data;
   }

   if (BaseNode->right != nil)
   {
      BaseNode = BaseNode->right;
      while (BaseNode->left != nil)
        BaseNode = BaseNode->left;
   }
   else
   {
      Node *tmpNode = BaseNode->parent;
      while (tmpNode && BaseNode == tmpNode->right)
        {
           BaseNode = tmpNode;
           tmpNode = tmpNode->parent;
        }

      if(tmpNode==NULL)
         BaseNode = nil;
      else if (BaseNode->right != tmpNode)
         BaseNode = tmpNode;
   }

   return BaseNode==nil?NULL:&BaseNode->data;
}

template <class Object>
void CRedBlackTree<Object>::rotateLeft(Node * x)
{

   /**************************
    *  rotate node x to left *
    **************************/

  Node *y = x->right;

  /* establish x->right link */
  x->right = y->left;
  if (y->left != NIL)
    y->left->parent = x;

  /* establish y->parent link */
  if (y != NIL)
    y->parent = x->parent;
  if (x->parent)
    {
      if (x == x->parent->left)
        x->parent->left = y;
      else
        x->parent->right = y;
    }
  else
    {
      root = y;
    }

  /* link x and y */
  y->left = x;
  if (x != NIL)
    x->parent = y;
}

template <class Object>
void CRedBlackTree<Object>::rotateRight(Node * x)
{

   /****************************
    *  rotate node x to right  *
    ****************************/

  Node *y = x->left;

  /* establish x->left link */
  x->left = y->right;
  if (y->right != NIL)
    y->right->parent = x;

  /* establish y->parent link */
  if (y != NIL)
    y->parent = x->parent;
  if (x->parent)
    {
      if (x == x->parent->right)
        x->parent->right = y;
      else
        x->parent->left = y;
    }
  else
    {
      root = y;
    }

  /* link x and y */
  y->right = x;
  if (x != NIL)
    x->parent = y;
}

template <class Object>
void CRedBlackTree<Object>::insertFixup(Node * x)
{

   /*************************************
    *  maintain Red-Black tree balance  *
    *  after inserting node x           *
    *************************************/
  /* check Red-Black properties */
  while (x != root && x->parent->color == RED)
    {
      /* we have a violation */
      if (x->parent == x->parent->parent->left)
        {
          Node *y = x->parent->parent->right;

          if (y->color == RED)
            {

              /* uncle is RED */
              x->parent->color = BLACK;
              y->color = BLACK;
              x->parent->parent->color = RED;
              x = x->parent->parent;
            }
          else
            {

              /* uncle is BLACK */
              if (x == x->parent->right)
                {
                  /* make x a left child */
                  x = x->parent;
                  rotateLeft(x);
                }

              /* recolor and rotate */
              x->parent->color = BLACK;
              x->parent->parent->color = RED;
              rotateRight(x->parent->parent);
            }
        }
      else
        {

          /* mirror image of above code */
          Node *y = x->parent->parent->left;

          if (y->color == RED)
            {

              /* uncle is RED */
              x->parent->color = BLACK;
              y->color = BLACK;
              x->parent->parent->color = RED;
              x = x->parent->parent;
            }
          else
            {

              /* uncle is BLACK */
              if (x == x->parent->left)
                {
                  x = x->parent;
                  rotateRight(x);
                }
              x->parent->color = BLACK;
              x->parent->parent->color = RED;
              rotateLeft(x->parent->parent);
            }
        }
    }
  root->color = BLACK;
}

template <class Object>
Node_<Object> *CRedBlackTree<Object>::insertNode(const Object &data)
{
  Node *current, *parent, *x;

   /***********************************************
    *  allocate node for data and insert in tree  *
    ***********************************************/

  /* find where node belongs */
  current = root;
  parent = 0;
  while (current != NIL)
    {
      if (compEQ(data, current->data))
        return (current);
      parent = current;
      current = compLT(data, current->data) ? current->left : current->right;
    }

  /* setup new node */
  if ((x = new Node) == 0)
    {
//        printf ("insufficient memory (insertNode)\n");
//        exit(1);
      return NULL;
    }
  x->data=data;
  x->parent = parent;
  x->left = NIL;
  x->right = NIL;
  x->color = RED;

  /* insert node in tree */
  if (parent)
    {
      if (compLT(data, parent->data))
        parent->left = x;
      else
        parent->right = x;
    }
  else
    {
      root = x;
    }

  insertFixup(x);
  return (x);
}

template <class Object>
void CRedBlackTree<Object>::deleteFixup(Node * x)
{

   /*************************************
    *  maintain Red-Black tree balance  *
    *  after deleting node x            *
    *************************************/

  while (x != root && x->color == BLACK)
    {
      if (x == x->parent->left)
        {
          Node *w = x->parent->right;

          if (w->color == RED)
            {
              w->color = BLACK;
              x->parent->color = RED;
              rotateLeft(x->parent);
              w = x->parent->right;
            }
          if (w->left->color == BLACK && w->right->color == BLACK)
            {
              w->color = RED;
              x = x->parent;
            }
          else
            {
              if (w->right->color == BLACK)
                {
                  w->left->color = BLACK;
                  w->color = RED;
                  rotateRight(w);
                  w = x->parent->right;
                }
              w->color = x->parent->color;
              x->parent->color = BLACK;
              w->right->color = BLACK;
              rotateLeft(x->parent);
              x = root;
            }
        }
      else
        {
          Node *w = x->parent->left;

          if (w->color == RED)
            {
              w->color = BLACK;
              x->parent->color = RED;
              rotateRight(x->parent);
              w = x->parent->left;
            }
          if (w->right->color == BLACK && w->left->color == BLACK)
            {
              w->color = RED;
              x = x->parent;
            }
          else
            {
              if (w->left->color == BLACK)
                {
                  w->right->color = BLACK;
                  w->color = RED;
                  rotateLeft(w);
                  w = x->parent->left;
                }
              w->color = x->parent->color;
              x->parent->color = BLACK;
              w->left->color = BLACK;
              rotateRight(x->parent);
              x = root;
            }
        }
    }
  x->color = BLACK;
}

template <class Object>
void CRedBlackTree<Object>::deleteNode(Node * z)
{
  Node *x, *y;

   /*****************************
    *  delete node z from tree  *
    *****************************/

  if (!z || z == NIL)
    return;

  if (z->left == NIL || z->right == NIL)
    {
      /* y has a NIL node as a child */
      y = z;
    }
  else
    {
      /* find tree successor with a NIL node as a child */
      y = z->right;
      while (y->left != NIL)
        y = y->left;
    }

  /* x is y's only child */
  if (y->left != NIL)
    x = y->left;
  else
    x = y->right;

  /* remove y from the parent chain */
  x->parent = y->parent;
  if (y->parent)
    if (y == y->parent->left)
      y->parent->left = x;
    else
      y->parent->right = x;
  else
    root = x;

  if (y != z)
    z->data=y->data;

  if (y->color == BLACK)
    deleteFixup(x);

  delete y;
}

template <class Object>
Node_<Object> *CRedBlackTree<Object>::findNode(const Object &data)
{
  /*******************************
   *  find node containing data  *
   *******************************/
  Node *current = root;
  while (current != NIL)
    if (compEQ(data, current->data))
      return (current);
    else
      current = compLT(data, current->data) ? current->left : current->right;
  return (0);
}

#endif //__rbtree_cpp
