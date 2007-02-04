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
#include "avl_macro.hpp"

avl_macro_data::avl_macro_data(DWORD new_key,const unsigned char *new_sequence,DWORD new_flags,bool new_selection,bool new_noselection,int new_type)
{
  key=new_key;
  sequence=new_sequence;
  flags=new_flags;
  selection=MACRO_IGNORE_SELECTION;
  if(new_selection)
    selection=MACRO_SELECTION;
  else if(new_noselection)
    selection=MACRO_NO_SELECTION;
  type=new_type;
}

avl_macro_tree::avl_macro_tree()
{
  clear();
}

avl_macro_tree::~avl_macro_tree()
{
  clear();
}

long avl_macro_tree::compare(avl_node<avl_macro_data> *first,avl_macro_data *second)
{
  long result=second->key-first->data->key;
  if(!result)
  {
    result=second->selection-first->data->selection;
    if(!result)
    {
      if(second->type!=-1&&first->data->type!=-1)
        result=second->type-first->data->type;
    }
  }
  return result;
}

avl_macro_data *avl_macro_tree::query(DWORD q_key,int q_selection,int q_type)
{
  avl_macro_data Get(q_key,(const unsigned char *)"",0,q_selection==MACRO_SELECTION,q_selection==MACRO_NO_SELECTION,q_type);
  return avl_tree<avl_macro_data>::query(&Get);
}
