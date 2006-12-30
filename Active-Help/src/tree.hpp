/*
    Active-Help plugin for FAR Manager
    Copyright (C) 2002-2005 Alex Yaroslavsky

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
#ifndef __TREE_HPP__
#define __TREE_HPP__
#include <stddef.h>

typedef class TreeLink *PTreeLink;

class TreeLink
{
  void *data;
  size_t size;
  PTreeLink Parent;
  PTreeLink ChildLeft;
  PTreeLink ChildRight;
  public:
  TreeLink(void);
  ~TreeLink(void);
  size_t GetDataSize(void);
  void GetData(void *user_data);
  PTreeLink GetParent(void);
  PTreeLink GetChildLeft(void);
  PTreeLink GetChildRight(void);
  bool SetData(void *user_data, size_t user_size);
  void SetParent(PTreeLink new_parent);
  void SetChildLeft(PTreeLink new_childleft);
  void SetChildRight(PTreeLink new_childright);
};

typedef struct _AHTrackInfo
{
  int EditorID;
  int SelectedItem;
} AHTrackInfo;

typedef int (*CMPFNC)(AHTrackInfo *info1, AHTrackInfo *info2);

int CmpID(AHTrackInfo *info1, AHTrackInfo *info2);

class PSTree
{
  PTreeLink base;
  void RecDelete(PTreeLink link);
  bool RecInsert(PTreeLink link, AHTrackInfo *info, CMPFNC Cmp);
  PTreeLink RecFind(PTreeLink link, AHTrackInfo *info,CMPFNC Cmp);
  //PTreeLink RecSearch(PTreeLink link, AHTrackInfo *info,CMPFNC Cmp);
  public:
  PSTree(void);
  ~PSTree(void);
  bool Insert(AHTrackInfo *info);
  bool GetID(AHTrackInfo *info);
  void Delete(AHTrackInfo *info);
};

#endif
