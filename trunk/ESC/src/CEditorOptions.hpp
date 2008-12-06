/*
    [ESC] Editor's settings changer plugin for FAR Manager
    Copyright (C) 2001 Ivan Sintyurin
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
*/
#ifndef __CEditorOptions_hpp
#define __CEditorOptions_hpp

#include "plugin.hpp"
#include "e_options.hpp"

class CEditorOptions
{
private:
  NODEDATA Data;
  EditorSetParameter ESPar;

public:
  CEditorOptions(const NODEDATA &Data);
   ~CEditorOptions();
  int ApplyOption(EDITOR_SETPARAMETER_TYPES type);
  void ApplyAllOptions();

private:
  static void MakeWordDiv(bool alphanum, const wchar_t *additional, wchar_t *dest);

private:
  CEditorOptions& operator=(const CEditorOptions& rhs); /* чтобы не */
  CEditorOptions(const CEditorOptions& rhs); /* генерировалось по умолчанию */
};

#endif // __CEditorOptions_hpp
