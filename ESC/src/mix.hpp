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
#ifndef __mix_hpp
#define __mix_hpp

#include "e_options.hpp"
#include "CEditorOptions.hpp"

extern wchar_t nlsSpace, nlsTab, nlsQuoteSym, nlsMinus;

extern "C"
{
    int FARPostMacro(const KeySequence *macro);
    void EditorUnmarkBlock();
    BOOL EditorPostMacro(CUserMacros &macros, UserMacroID &id,
                         const EditorInfo &ei, BOOL &Stop);
    void TestCfgFiles();
    int IsQuote(const wchar_t* pszStr, size_t nLength);
    BOOL ProcessKeyEnter(const EditorInfo &ei, EditorSetPosition &esp,
                     EditorGetString &egs, int nQuote, bool isWrapQuoteEOL);
    BOOL ProcessKeyDelete(struct EditorGetString &egs, int nQuote);
    BOOL CmpWithFileMask(const wchar_t *Mask, const wchar_t *Name, bool SkipPath);
    int FindCodeTable(const wchar_t *Mask);
    inline int IsCSpaceOrTab(wchar_t ch) { return nlsSpace==ch || nlsTab==ch; }
    BOOL CheckForEsc(void);
    const wchar_t *GetMsg(int MsgId);
    BOOL ChangeCoordX(const EditorInfo &ei, EditorSetPosition &esp);
    int GetPrevCoordX(EditorInfo &ei, int Lines, const wchar_t *StopChars);
    BOOL GotoHome(const EditorInfo &ei, NODEDATA &nodedata);
    void FreeMem();
    BOOL ReloadSettings(BOOL Force);
    int FarAllInOneMessage(const wchar_t *Message, unsigned int Flags);
    BOOL UnpackEOL(DWORD EOL, wchar_t *Dest);
    void InitESPandEGS(struct EditorSetPosition &esp, struct EditorGetString &egs);
    void KillSpaces();
    void KillSpacesAndChangeEOL();
    void ChangeEOL();
    BOOL FileExists(const wchar_t *Name);
    wchar_t *TruncFromRigth(wchar_t *Str, unsigned int maxsize, BOOL AddSpaces);
    void InitNLS(const EditorInfo &ei, NODEDATA &nodedata);
    int CalcWrapPos(const NODEDATA &Data, const EditorInfo &ei);
    int GetNextCoordX(const EditorInfo &EI, int Lines, const wchar_t *StopChars);
    void ApplyEditorOptions(NODEDATA &Settings,const wchar_t *FileName);
    void RestorePosition(const EditorInfo &EI);
    BOOL InsertAdditionalSymbol(const EditorInfo &ei,
                            EditorSetPosition &esp,
                            EditorSetString &ess, EditorGetString &egs,
                            const strcon &AddSym_S,
                            const strcon &AddSym_E,
                            wchar_t Symbol, BOOL IncreaseCoordX);
    DWORD NormalizeControlState(DWORD State);
}
#endif // __mix_cpp