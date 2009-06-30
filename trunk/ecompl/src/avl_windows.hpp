/*
    avl_windows.hpp
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

#ifndef __AVL_WINDOWS_HPP__
#define __AVL_WINDOWS_HPP__

#include "avl.hpp"
#include "string.hpp"

class avl_window_data
{
  private:
    int eid; //editor
  public:
    int row; //cursor pos
    int col;
    bool Active; //autotext added
    size_t OldLen; //original word len
    size_t AddedLen; //autotext len
    bool On; //Autocomplete at next redraw
    bool BlockDeleted; //non-persistent strem block - so skip 1st event.
    string Rewrited;
    string Inserted;
    avl_window_data(int value);
    void clear(void);
  friend class avl_window_tree;
};

class avl_window_tree: public avl_tree<avl_window_data>
{
  public:
    avl_window_tree();
    ~avl_window_tree();
    long compare(avl_node<avl_window_data> *first,avl_window_data *second);
    avl_window_data *query(int value);
};

#endif
