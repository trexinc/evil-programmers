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
struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;

wchar_t XMLFilePath[MAX_PATH];

BOOL IsOldFar=TRUE, XMLLoadedOK=FALSE;

#include "e_options.hpp"
#include "XMLStrings.hpp"
#include "lang.hpp"
#include "options.hpp"

OPTIONS Opt;

TArray<NODEDATA> *NodeData=NULL;

class EditorSettingsStorage
{
 public:
  int EditorId, index;
  NODEDATA Data;

  EditorSettingsStorage():EditorId(-1), index(-1)
  {
  }

  EditorSettingsStorage(int Id):EditorId(Id), index(-1)
  {
  }

  EditorSettingsStorage(int Id, int ind, const NODEDATA &data):
    EditorId(Id), index(ind), Data(data)
  {
  }

  EditorSettingsStorage(const EditorSettingsStorage& rhs):
    EditorId(rhs.EditorId), index(rhs.index), Data(rhs.Data)
  {
  }

  EditorSettingsStorage& operator=(const EditorSettingsStorage& rhs)
  {
    if(this!=&rhs)
    {
      EditorId=rhs.EditorId;
      index=rhs.index;
      Data=rhs.Data;
    }
    return *this;
  }
};

CRedBlackTree<EditorSettingsStorage> *ESETStorage=NULL;
CRedBlackTree<ESCFileInfo> *FileInfoTree=NULL;

struct EditorInfo ei;
struct EditorGetString egs;
struct EditorSetPosition esp;
struct EditorSetString ess;
wchar_t nlsSpace, nlsTab, nlsQuoteSym, nlsMinus;
strcon *nlsStopChars=NULL;
wchar_t nlsStop[64];

XMLStrings XMLStr=
{
  L"/Esc-Settings/Types",L"name",L"type",L"mask",L"tabsize",L"expandtabs",
  L"autoindent",L"eol",L"cursorbeol",L"charcodebase",L"killspace",L"killemptylines",
  L"wrap",L"justify",L"on",L"off",L"onlynew",L"oct",L"dec",L"hex",L"table",L"p_end",L"autowrap",
  L"quoteeol",L"p_minuses",L"lockmode",L"tablename",
  L"smarttab",L"smartbs",L"lines",L"smarthome",L"skippath",L"p_quote",
  L"stopchar",L"inherit",L"include",
  L"savepos",L"minlines",L"addsymbol",
  L"addsym_s",L"addsym_e",L"addsym_s_b",L"addsym_e_b",
  L"start", L"end", L"start_b", L"end_b",
  L"forcekel",
  L"usermacro",L"macro",L"enable",L"sequence",L"silent",L"selection",L"key",
  L"lclick",L"rclick",L"mclick",L"rctrl",L"lctrl",L"ralt",L"lalt",L"shift",
  L"stream",L"column",L"lockfile",L"stop",
  L"wordsym",L"alphanum",L"additional",
  L"overload",
  L"oncreate",L"onload",L"auto",
  L"margin",L"quote",L"tab",L"value",L"smartkey",L"backspace",L"home",L"kill",L"showwhitespace",L"bom",
};

FARAPIEDITORCONTROL EditorControl;
FARAPIMESSAGE       FarMessage;
FARSTDTRUNCPATHSTR  TruncPathStr;
