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
#ifndef __CUserMacros_cpp
#define __CUserMacros_cpp

#include <CRT/crt.hpp>
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

bool CUserMacros::InsertMacro(const UserMacroID &ID, const wchar_t *Str,
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
