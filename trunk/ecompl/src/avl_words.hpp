/*
    avl_words.hpp
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

#ifndef __AVL_WORDS_HPP__
#define __AVL_WORDS_HPP__

#include "avl.hpp"
#include "string.hpp"

class avl_word_data
{
  private:
    void init(void);
    string data;
    unsigned long hash;
    int ref;
  public:
    avl_word_data();
    avl_word_data(const string &Value);
    void inc_ref(void);
    int get_ref(void) const;
    const string &get_data() const;
  friend class avl_word_tree;
};

typedef void (*FOREACH)(void *data,int &counter,avl_word_data &node);

class avl_word_tree: public avl_tree<avl_word_data>
{
  private:
    size_t max_len;
    string partial;
    void recurse_iterate(avl_node<avl_word_data> *node,void *data,int &counter,FOREACH foreach);
  public:
    avl_word_tree();
    ~avl_word_tree();
    void clear(void);
    long compare(avl_node<avl_word_data> *first,avl_word_data *second);
    avl_word_data *insert(avl_word_data *data);
    void iterate(void *data,int &counter,FOREACH foreach);
    size_t get_max_len(void);
    const string &get_partial(void);
};

#endif
