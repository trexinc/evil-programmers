/*
    avl_words.cpp
    Copyright (C) 2002-2008 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <windows.h>
#include "avl_words.hpp"
#include "plugin.hpp"
#include "EditCmpl.hpp"

void avl_word_data::init(void)
{
  hash=0; ref=0; data.clear();
}

avl_word_data::avl_word_data()
{
  init();
}

avl_word_data::avl_word_data(const string &Value)
{
  init();
  data=Value;
  hash=data.hash();
}

void avl_word_data::inc_ref(void)
{
  ref++;
}

int avl_word_data::get_ref(void) const
{
  return ref;
}

const string &avl_word_data::get_data() const
{
  return data;
}

avl_word_tree::avl_word_tree()
{
  clear();
}

avl_word_tree::~avl_word_tree()
{
  clear();
}

void avl_word_tree::clear(void)
{
  avl_tree<avl_word_data>::clear();
  max_len=0;
}

long avl_word_tree::compare(avl_node<avl_word_data> *first,avl_word_data *second)
{
  long diff=second->hash-first->data->hash;
  if(!diff) diff=(second->data==first->data->data);
  return diff;
}

void avl_word_tree::recurse_iterate(avl_node<avl_word_data> *node,void *data,int &counter,FOREACH foreach)
{
  if(node->left) recurse_iterate(node->left,data,counter,foreach);
  foreach(data,counter,*node->data);
  if(node->right) recurse_iterate(node->right,data,counter,foreach);
}

void avl_word_tree::iterate(void *data,int &counter,FOREACH foreach)
{
  if(root) recurse_iterate(root,data,counter,foreach);
}

size_t avl_word_tree::get_max_len(void)
{
  return max_len;
}

avl_word_data *avl_word_tree::insert(avl_word_data *data)
{
  if(count())
  {
    size_t len=((data->data.length()<partial.length())?data->data.length():partial.length()),reslen=len;
    for(size_t i=0;i<len;i++)
    {
      if(FSF.LUpper(data->data[i])!=FSF.LUpper(partial[i])) { reslen=i; break; }
    }
    partial((const UTCHAR *)partial,reslen);
  }
  else
  {
    partial=data->data;
  }
  avl_word_data *result=avl_tree<avl_word_data>::insert(data);
  if(result) if(result->data.length()>max_len) max_len=result->data.length();
  return result;
}

const string &avl_word_tree::get_partial(void)
{
  return partial;
}
