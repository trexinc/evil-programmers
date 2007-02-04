/*
    Macro plugin for DialogManager
    Copyright (C) 2004 Vadim Yegorov

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
#ifndef __AVL_HPP__
#define __AVL_HPP__

#include <stdio.h>

enum AVL_SKEW
{
  AVL_NONE,
  AVL_LEFT,
  AVL_RIGHT
};

enum AVL_RES
{
  AVL_ERROR=0,
  AVL_OK,
  AVL_BALANCE
};

template <class D> class avl_tree;

template <class D> class avl_node
{
  public:
    avl_node<D> *left, *right;
    D *data;
    AVL_SKEW skew;
    virtual void init(void);
  public:
    avl_node() { init(); }
    avl_node(D *value) { init(); data=value; }
    virtual ~avl_node() { delete data; }
    const D *get_data(void) const;
  friend class avl_tree<D>;
};

template <class D> void avl_node<D>::init(void)
{
  data=NULL;
  left=right=NULL;
  skew=AVL_NONE;
}

template <class D> const D *avl_node<D>::get_data(void) const
{
  return data;
}

template <class D> class avl_tree
{
  private:
    int node_count;
    void recurse_delete(avl_node<D> *node)
    {
      if(node->left) recurse_delete(node->left);
      if(node->right) recurse_delete(node->right);
      delete node;
    }
    AVL_RES internal_insert(avl_node<D> **node,D *data,D **result_data);
    AVL_RES internal_remove(avl_node<D> **node,D *data);
    AVL_RES leftgrown(avl_node<D> **node);
    AVL_RES rightgrown(avl_node<D> **node);
    void rotleft(avl_node<D> **node);
    void rotright(avl_node<D> **node);
    AVL_RES leftshrunk(avl_node<D> **node);
    AVL_RES rightshrunk(avl_node<D> **node);
    int findhighest(avl_node<D> *target,avl_node<D> **node,AVL_RES *res);
    int findlowest(avl_node<D> *target,avl_node<D> **node,AVL_RES *res);
    D *internal_query(avl_node<D> *node,D *data);
  protected:
    avl_node<D> *root;
  public:
    virtual ~avl_tree() {}
    int count(void) { return node_count; }
    virtual void clear(void);
    virtual long compare(avl_node<D> *first,D *second)=0;
    D *insert(D *data);
    void remove(D *data);
    D *query(D *data);
    avl_node<D> *get_top(void) { return root; }
};

template <class D> void avl_tree<D>::clear(void)
{
  if(root) recurse_delete(root);
  root=NULL;
  node_count=0;
}

template <class D> D *avl_tree<D>::insert(D *data)
{
  D *result=NULL;
  internal_insert(&root,data,&result);
  return result;
}

template <class D> void avl_tree<D>::remove(D *data)
{
  internal_remove(&root,data);
}

template <class D> D *avl_tree<D>::query(D *data)
{
  return internal_query(root,data);
}

template <class D> AVL_RES avl_tree<D>::internal_insert(avl_node<D> **node,D *data,D **result_data)
{
  AVL_RES result;

  if(!(*node))
  {
    if(!(*node=new avl_node<D>(data)))
    {
      return AVL_ERROR;
    }
    *result_data=(*node)->data;
    node_count++;
    return AVL_BALANCE;
  }
  long diff=compare(*node,data);
  if(diff<0)
  {
    if((result=internal_insert(&(*node)->left,data,result_data))==AVL_BALANCE)
    {
      return leftgrown(node);
    }
    return result;
  }
  if(diff>0)
  {
    if((result=internal_insert(&(*node)->right,data,result_data))==AVL_BALANCE)
    {
      return rightgrown(node);
    }
    return result;
  }
  *result_data=(*node)->data;
  delete data;
  return AVL_ERROR;
}

template <class D> AVL_RES avl_tree<D>::internal_remove(avl_node<D> **node,D *data)
{
  AVL_RES result=AVL_BALANCE;
  if(!(*node)) return AVL_ERROR;
  long diff=compare(*node,data);
  if(diff<0)
  {
    if((result=internal_remove(&(*node)->left,data))==AVL_BALANCE)
    {
      return leftshrunk(node);
    }
    return result;
  }
  if(diff>0)
  {
    if((result=internal_remove(&(*node)->right,data))==AVL_BALANCE)
    {
      return rightshrunk(node);
    }
    return result;
  }
  node_count--;
  if((*node)->left)
  {
    if(findhighest(*node,&((*node)->left),&result))
    {
      if(result==AVL_BALANCE)
      {
        result=leftshrunk(node);
      }
      return result;
    }
  }
  if((*node)->right)
  {
    if(findlowest(*node,&((*node)->right),&result))
    {
      if(result==AVL_BALANCE)
      {
        result=rightshrunk(node);
      }
      return result;
    }
  }
  delete *node;
  *node=NULL;
  return AVL_BALANCE;
}

template <class D> void avl_tree<D>::rotleft(avl_node<D> **node)
{
  avl_node<D> *tmp=*node;
  *node=(*node)->right;
  tmp->right=(*node)->left;
  (*node)->left=tmp;
}

template <class D> void avl_tree<D>::rotright(avl_node<D> **node)
{
  avl_node<D> *tmp=*node;
  *node=(*node)->left;
  tmp->left=(*node)->right;
  (*node)->right=tmp;
}

template <class D> AVL_RES avl_tree<D>::leftgrown(avl_node<D> **node)
{
  switch((*node)->skew)
  {
    case AVL_LEFT:
      if((*node)->left->skew==AVL_LEFT)
      {
        (*node)->skew=(*node)->left->skew=AVL_NONE;
        rotright(node);
      }
      else
      {
        switch((*node)->left->right->skew)
        {
          case AVL_LEFT:
            (*node)->skew=AVL_RIGHT;
            (*node)->left->skew=AVL_NONE;
            break;
          case AVL_RIGHT:
            (*node)->skew=AVL_NONE;
            (*node)->left->skew=AVL_LEFT;
            break;

          default:
            (*node)->skew=AVL_NONE;
            (*node)->left->skew=AVL_NONE;
        }
        (*node)->left->right->skew=AVL_NONE;
        rotleft(&(*node)->left);
        rotright(node);
      }
      return AVL_OK;

    case AVL_RIGHT:
      (*node)->skew=AVL_NONE;
      return AVL_OK;

    default:
      (*node)->skew=AVL_LEFT;
      return AVL_BALANCE;
  }
}

template <class D> AVL_RES avl_tree<D>::rightgrown(avl_node<D> **node)
{
  switch((*node)->skew)
  {
    case AVL_LEFT:
      (*node)->skew=AVL_NONE;
      return AVL_OK;
    case AVL_RIGHT:
      if((*node)->right->skew==AVL_RIGHT)
      {
        (*node)->skew=(*node)->right->skew=AVL_NONE;
        rotleft(node);
      }
      else
      {
        switch((*node)->right->left->skew)
        {
          case AVL_RIGHT:
            (*node)->skew=AVL_LEFT;
            (*node)->right->skew=AVL_NONE;
            break;
          case AVL_LEFT:
            (*node)->skew=AVL_NONE;
            (*node)->right->skew=AVL_RIGHT;
            break;
          default:
            (*node)->skew=AVL_NONE;
            (*node)->right->skew=AVL_NONE;
        }
        (*node)->right->left->skew=AVL_NONE;
        rotright(&(*node)->right);
        rotleft(node);
      }
      return AVL_OK;
    default:
      (*node)->skew=AVL_RIGHT;
      return AVL_BALANCE;
  }
}

template <class D> AVL_RES avl_tree<D>::leftshrunk(avl_node<D> **node)
{
  switch((*node)->skew)
  {
    case AVL_LEFT:
      (*node)->skew=AVL_NONE;
      return AVL_BALANCE;

    case AVL_RIGHT:
      if((*node)->right->skew==AVL_RIGHT)
      {
        (*node)->skew=(*node)->right->skew=AVL_NONE;
        rotleft(node);
        return AVL_BALANCE;
      }
      else if((*node)->right->skew==AVL_NONE)
      {
        (*node)->skew=AVL_RIGHT;
        (*node)->right->skew=AVL_LEFT;
        rotleft(node);
        return AVL_OK;
      }
      else
      {
        switch((*node)->right->left->skew)
        {
          case AVL_LEFT:
            (*node)->skew=AVL_NONE;
            (*node)->right->skew=AVL_RIGHT;
            break;
          case AVL_RIGHT:
            (*node)->skew=AVL_LEFT;
            (*node)->right->skew=AVL_NONE;
            break;
          default:
            (*node)->skew=AVL_NONE;
            (*node)->right->skew=AVL_NONE;
        }
        (*node)->right->left->skew=AVL_NONE;
        rotright(&(*node)->right);
        rotleft(node);
        return AVL_BALANCE;
      }
    default:
      (*node)->skew=AVL_RIGHT;
      return AVL_OK;
  }
}

template <class D> AVL_RES avl_tree<D>::rightshrunk(avl_node<D> **node)
{
  switch((*node)->skew)
  {
    case AVL_RIGHT:
      (*node)->skew=AVL_NONE;
      return AVL_BALANCE;
    case AVL_LEFT:
      if((*node)->left->skew==AVL_LEFT)
      {
        (*node)->skew=(*node)->left->skew=AVL_NONE;
        rotright(node);
        return AVL_BALANCE;
      }
      else if((*node)->left->skew==AVL_NONE)
      {
        (*node)->skew=AVL_LEFT;
        (*node)->left->skew=AVL_RIGHT;
        rotright(node);
        return AVL_OK;
      }
      else
      {
        switch((*node)->left->right->skew)
        {
          case AVL_LEFT:
            (*node)->skew=AVL_RIGHT;
            (*node)->left->skew=AVL_NONE;
            break;
          case AVL_RIGHT:
            (*node)->skew=AVL_NONE;
            (*node)->left->skew=AVL_LEFT;
            break;
          default:
            (*node)->skew=AVL_NONE;
            (*node)->left->skew=AVL_NONE;
        }
        (*node)->left->right->skew=AVL_NONE;
        rotleft(&(*node)->left);
        rotright(node);
        return AVL_BALANCE;
      }
    default:
      (*node)->skew=AVL_LEFT;
      return AVL_OK;
  }
}

template <class D> int avl_tree<D>::findhighest(avl_node<D> *target,avl_node<D> **node,AVL_RES *res)
{
  avl_node<D> *tmp; D *tmp_data;
  *res=AVL_BALANCE;
  if(!(*node)) return 0;
  if((*node)->right)
  {
    if(!findhighest(target,&(*node)->right,res)) return 0;
    if(*res==AVL_BALANCE)
    {
      *res=rightshrunk(node);
    }
    return 1;
  }
  tmp_data=target->data;
  target->data=(*node)->data;
  (*node)->data=tmp_data;
  tmp=*node;
  *node=(*node)->left;
  delete tmp;
  return 1;
}

template <class D> int avl_tree<D>::findlowest(avl_node<D> *target,avl_node<D> **node,AVL_RES *res)
{
  avl_node<D> *tmp; D *tmp_data;
  *res=AVL_BALANCE;
  if(!(*node)) return 0;
  if((*node)->left)
  {
    if(!findlowest(target,&(*node)->left,res)) return 0;
    if(*res==AVL_BALANCE)
    {
      *res=leftshrunk(node);
    }
    return 1;
  }
  tmp_data=target->data;
  target->data=(*node)->data;
  (*node)->data=tmp_data;
  tmp=*node;
  *node=(*node)->right;
  delete tmp;
  return 1;
}

template <class D> D *avl_tree<D>::internal_query(avl_node<D> *node,D *data)
{
  if(!node) return NULL;
  long diff=compare(node,data);
  if(diff<0) return internal_query(node->left,data);
  if(diff>0) return internal_query(node->right,data);
  return node->data;
}

#endif
