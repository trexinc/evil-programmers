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
#ifndef __DIALOGS_MACRO_HPP__
#define __DIALOGS_MACRO_HPP__

#include "avl.hpp"
#include "string.hpp"
#include "avl_macro.hpp"

class dialogs_macro
{
  public:
    string title;
    long dialog_item_count;
    avl_macro_tree macros;
    bool global;
    dialogs_macro *next;
    dialogs_macro(const char *registry_key,bool new_global,dialogs_macro *new_next);
    bool check(HANDLE hDlg,int curr_item,long key);
};

#endif
