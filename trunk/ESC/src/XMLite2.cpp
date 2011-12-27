/*
  [ESC] Editor's settings changer plugin for FAR Manager
  Copyright (C) 2000 Konstantin Stupnik
  Copyright (C) 2008 Alex Yaroslavsky

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

  Non validating XML parser for well-formed canonical XML.
  Can be used to read/write config files stored in XML format.
  Very fast. Very memory effecient.
*/

#include <CRT/crt.hpp>
#include "XMLite.hpp"

bool CXMLQuery::Next(CXMLNode& res)
{
  if(!node)return false;
  res=*node;
  res.pNode=xmlQueryNext(node->pNode,&query);
  return res.pNode!=NULL;
}
