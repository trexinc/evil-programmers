#ifndef __CUserMacros_cpp
#define __CUserMacros_cpp

#include "CUserMacros.hpp"

CUserMacros::CUserMacros()
{
  Tree=new CRedBlackTree<OneUserMacro>(CompLT,CompEQ);
}

CUserMacros::CUserMacros(const CUserMacros& rhs)
{
  Tree=new CRedBlackTree<OneUserMacro>(CompLT,CompEQ);
  *this=rhs;
}

CUserMacros& CUserMacros::operator=(const CUserMacros &rhs)
{
   if(this!=&rhs)
     *Tree=*rhs.Tree;
   return *this;
}

BOOL WINAPI CUserMacros::CompLT(const OneUserMacro &a,const OneUserMacro &b)
{
  return a<b;
}

BOOL WINAPI CUserMacros::CompEQ(const OneUserMacro &a,const OneUserMacro &b)
{
  return a==b;
}

bool CUserMacros::InsertMacro(const UserMacroID &ID, const char *Str,
                      bool silent, bool stop, int &Error, strcon &unknownKey)
{
  Node *node=Find(ID);
  if(!node)
  {
    OneUserMacro tmp(ID);
    node=Tree->insertNode(tmp);
  }
  if(node)
  {
    node->data.ID=ID;
    if(node->data.Macro.compile(Str, silent, 1, stop, Error, unknownKey))
      return true;
    Tree->deleteNode(node);
  }
  return false;
}

bool CUserMacros::DeleteMacro(const UserMacroID &ID)
{
  Node *node=Find(ID);
  if(node)
  {
     Tree->deleteNode(node);
     return true;
  }
  return false;
}

Node_<OneUserMacro> *CUserMacros::Find(const OneUserMacro &ID)
{
  OneUserMacro tmp(ID);
  return Tree->findNode(tmp);
}

const KeySequence *CUserMacros::GetMacro(const UserMacroID &ID, BOOL &Stop)
{
  Node *node=Find(ID);
  if(node)
    return &node->data.Macro.GetSequence(Stop);
  return NULL;
}

#endif // __CUserMacros_cpp
