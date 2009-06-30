/*
    avl_windows.cpp
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

#include "avl_windows.hpp"

void avl_window_data::clear(void)
{
  Active=On=BlockDeleted=false;
  Rewrited.clear();
  Inserted.clear();
}

avl_window_data::avl_window_data(int value)
{
  eid=value;
  clear();
}

avl_window_tree::avl_window_tree()
{
  clear();
}

avl_window_tree::~avl_window_tree()
{
  clear();
}

long avl_window_tree::compare(avl_node<avl_window_data> *first,avl_window_data *second)
{
  return second->eid-first->data->eid;
}

avl_window_data *avl_window_tree::query(int value)
{
  avl_window_data Get(value);
  return avl_tree<avl_window_data>::query(&Get);
}
