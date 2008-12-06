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
  static void MakeWordDiv(bool alphanum, const char *additional, char *dest);

private:
  CEditorOptions& operator=(const CEditorOptions& rhs); /* чтобы не */
  CEditorOptions(const CEditorOptions& rhs); /* генерировалось по умолчанию */
};

#endif // __CEditorOptions_hpp
