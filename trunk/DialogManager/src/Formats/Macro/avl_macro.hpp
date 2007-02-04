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
#ifndef __AVL_MACRO_HPP__
#define __AVL_MACRO_HPP__

#include "avl.hpp"
#include "string.hpp"

#define MACRO_IGNORE_SELECTION -1
#define MACRO_SELECTION 1
#define MACRO_NO_SELECTION 0


class avl_macro_data
{
  public:
    DWORD key;
    string sequence;
    DWORD flags;
    int selection;
    int type;
    avl_macro_data(DWORD new_key,const unsigned char *new_sequence=(const unsigned char *)"",DWORD new_flags=0,bool new_selection=false,bool new_noselection=false,int new_type=-1);
  friend class avl_macro_tree;
};

class avl_macro_tree: public avl_tree<avl_macro_data>
{
  public:
    avl_macro_tree();
    ~avl_macro_tree();
    long compare(avl_node<avl_macro_data> *first,avl_macro_data *second);
    avl_macro_data *query(DWORD q_key,int q_selection,int q_type);
};

#endif
