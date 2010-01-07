/*
  Copyright (C) 2000 Konstantin Stupnik

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

  This is collection of auxilary functions that use almost
  all 'Classes'.
*/

#ifndef __TOOLS_HPP__
#define __TOOLS_HPP__

#include "String.hpp"
#include "Array.hpp"
#include "Hash.hpp"
#include "List.hpp"
#include "RegExp.hpp"

namespace XClasses{

class StrList:public List<String>{
public:
  int LoadFromFile(const char* filename);
  int SaveToFile(const char* filename);
  void Sort(StrList& dest);
  bool Find(const String& str);
};

int Split(const String& source,const String& pattern,StrList& dest,int maxnum=-1);

int RegScanf(const String& str,const char* rx,StrList& dst);

}//namespace XClasses

#endif
