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
#ifndef __PSITEMS_HPP__
#define __PSITEMS_HPP__

#include "PositionSaver.hpp"
#include "sgmlext.hpp"
#include "tree.hpp"

typedef struct _PSInfo
{
  char *Type;
  char *FileName;
  union
  {
    struct EditorSetPosition *esp;
    #ifdef ALLOW_VIEWER_API
    struct ViewerSetPosition *vsp;
    #endif
  };
} PSInfo;

class PSItems
{
  PSgmlExt base;
  char LastFileName[_MAX_PATH];
  FILETIME LastFileTime;
  PSgmlExt PFindItem(PSInfo Info);
  bool GetItem(PSgmlExt Item, PSInfo Info);
  bool SetItem(PSgmlExt Item, PSInfo Info);
  public:
  PSItems(void);
  ~PSItems(void);
  bool LoadFile(char *FileName, PSTree *tree);
  bool SaveFile(char *FileName, PSTree *tree, bool Load=true);
  bool FindItem(PSInfo Info);
  bool GetItem(PSInfo Info);
  bool SetItem(PSInfo Info);
  bool NewItem(PSInfo Info);
  void DelItem(PSInfo Info);
  int CountItems(const char *type);
  int RemoveOld(PSTree *tree, char *FileName);
  PSgmlExt Base();
};

#endif
