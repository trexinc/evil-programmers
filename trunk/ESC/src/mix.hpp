#ifndef __mix_hpp
#define __mix_hpp

#include "e_options.hpp"
#include "CEditorOptions.hpp"

extern char nlsSpace, nlsTab, nlsQuoteSym, nlsMinus;

extern "C"
{
    int FARPostMacro(const KeySequence *macro);
    inline int atoi(const char *s);
    void EditorUnmarkBlock();
    BOOL EditorPostMacro(CUserMacros &macros, UserMacroID &id,
                         const EditorInfo &ei, BOOL &Stop);
    void TestCfgFiles();
    int IsQuote(const char* pszStr, size_t nLength);
    BOOL ProcessKeyEnter(const EditorInfo &ei, EditorSetPosition &esp,
                     EditorGetString &egs, int nQuote, bool isWrapQuoteEOL);
    BOOL ProcessKeyDelete(struct EditorGetString &egs, int nQuote);
    BOOL CmpWithFileMask(const char *Mask, const char *Name, bool SkipPath);
    int FindCodeTable(const char *Mask);
    inline int IsCSpaceOrTab(char ch) { return nlsSpace==ch || nlsTab==ch; }
    BOOL CheckForEsc(void);
    void InitDialogItems(struct InitDialogItem *Init,
                         struct FarDialogItem *Item,
                         int ItemsNumber);
    const char *GetMsg(int MsgId);
    BOOL ChangeCoordX(const EditorInfo &ei, EditorSetPosition &esp);
    int GetPrevCoordX(EditorInfo &ei, int Lines, const char *StopChars);
    BOOL GotoHome(const EditorInfo &ei, NODEDATA &nodedata);
    void FreeMem();
    BOOL ReloadSettings(BOOL Force);
    int FarAllInOneMessage(const char *Message, unsigned int Flags);
    BOOL UnpackEOL(DWORD EOL, char *Dest);
    void InitESPandEGS(struct EditorSetPosition &esp, struct EditorGetString &egs);
    void KillSpaces();
    void KillSpacesAndChangeEOL();
    void ChangeEOL();
    BOOL FileExists(const char *Name);
    char *TruncFromRigth(char *Str, unsigned int maxsize, BOOL AddSpaces);
    void InitNLS(const EditorInfo &ei, NODEDATA &nodedata);
    int CalcWrapPos(const NODEDATA &Data, const EditorInfo &ei);
    int GetNextCoordX(const EditorInfo &EI, int Lines, const char *StopChars);
    void ApplyEditorOptions(NODEDATA &Settings,const char *FileName);
    void RestorePosition(const EditorInfo &EI);
    BOOL InsertAdditionalSymbol(const EditorInfo &ei,
                            EditorSetPosition &esp,
                            EditorSetString &ess, EditorGetString &egs,
                            const strcon &AddSym_S,
                            const strcon &AddSym_E,
                            BYTE Symbol, BOOL IncreaseCoordX);
}
#endif // __mix_cpp