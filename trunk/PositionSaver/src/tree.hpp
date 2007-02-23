/*
    Position Saver plugin for FAR Manager
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

#include "PositionSaver.hpp"

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

enum
{
  TI_EDITOR,
  #ifdef ALLOW_VIEWER_API
  TI_VIEWER,
  #endif
  TI_UNKNOWN,
};

typedef struct _PSTrackInfo
{
  int Type;
  union
  {
    int EditorID;
    #ifdef ALLOW_VIEWER_API
    int ViewerID;
    #endif
  };
  union
  {
    struct EditorSetPosition unmod;
    #ifdef ALLOW_VIEWER_API
    struct ViewerSetPosition vsp;
    #endif
  };
  struct EditorSetPosition mod, esp;
  int mod_flag;
  char FileName[_MAX_PATH];
} PSTrackInfo;

typedef int (*CMPFNC)(PSTrackInfo *info1, PSTrackInfo *info2);

int CmpID(PSTrackInfo *info1, PSTrackInfo *info2);
int CmpFN(PSTrackInfo *info1, PSTrackInfo *info2);

class PSTree
{
  PTreeLink base;
  void RecDelete(PTreeLink link);
  bool RecInsert(PTreeLink link, PSTrackInfo *info, CMPFNC Cmp);
  PTreeLink RecFind(PTreeLink link, PSTrackInfo *info,CMPFNC Cmp);
  PTreeLink RecSearch(PTreeLink link, PSTrackInfo *info,CMPFNC Cmp);
  public:
  PSTree(void);
  ~PSTree(void);
  bool Insert(PSTrackInfo *info);
  bool GetID(PSTrackInfo *info);
  bool GetFN(PSTrackInfo *info);
  bool FindID(PSTrackInfo *info);
  bool FindFN(PSTrackInfo *info);
  void Delete(PSTrackInfo *info);
};

#endif
