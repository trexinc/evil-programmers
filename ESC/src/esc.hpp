
struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;

char PluginRootKey[NM], XMLFilePath[NM];

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
struct CharTableSet CharSet, tmpCharSet;
char nlsSpace, nlsTab, nlsQuoteSym, nlsMinus;
strcon *nlsStopChars=NULL;
char nlsStop[64];

XMLStrings XMLStr=
{
  "/Esc-Settings/Types","name","type","mask","tabsize","expandtabs",
  "autoindent","eol","cursorbeol","charcodebase","killspace","killemptylines",
  "wrap","justify","on","off","oct","dec","hex","table","p_end","autowrap",
  "quoteeol","p_minuses","lockmode","tablename",
  "smarttab","smartbs","lines","smarthome","skippath","p_quote",
  "stopchar","inherit","include",
  "savepos","minlines","addsymbol",
  "addsym_s","addsym_e","addsym_s_b","addsym_e_b",
  "start", "end", "start_b", "end_b",
  "forcekel",
  "usermacro","macro","enable","sequence","silent","selection","key",
  "lclick","rclick","mclick","rctrl","lctrl","ralt","lalt","shift",
  "stream","column","lockfile","stop",
  "wordsym","alphanum","additional",
  "overload",
  "oncreate","onload","auto",
  "margin","quote","tab","value","smartkey","backspace","home","kill",
};

#include "InitDialogItem.hpp"

FARAPIEDITORCONTROL EditorControl;
FARAPICHARTABLE     CharTable;
FARAPIMESSAGE       FarMessage;
FARSTDEXPANDENVIRONMENTSTR ExpandEnvironmentStr;
FARSTDTRUNCPATHSTR  TruncPathStr;
int ModuleNumber;
