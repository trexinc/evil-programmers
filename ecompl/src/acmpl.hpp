/*
    acmpl.hpp
    Copyright (C) 2000-2001 Igor Lyubimov
    Copyright (C) 2002-2008 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ACMPL_H__
#define __ACMPL_H__

#include "cmpl.hpp"
#include "string.hpp"

class TAutoCompletion: public TCompletion
{
  private:
    bool CheckText(int Pos,int Row,avl_window_data *Window);
    int Dialog_Color;
  protected:
    bool AcceptFromMenu;
    int AcceptKey;
    int DeleteKey;
    __int64 Color;
    TCHAR AcceptChars[256];
    TCHAR MinPreWordLenText[21];
    TCHAR AcceptKeyText[21];
    TCHAR AcceptKeyCfgText[21];
    TCHAR DeleteKeyText[21];
    TCHAR DeleteKeyCfgText[21];
  protected:
    void DeleteVariant(avl_window_data *Window);
    bool AcceptVariant(avl_window_data *Window);
    bool PutVariant(avl_window_data *Window);
    void Colorize(int NewColor,avl_window_data *Window);
    void DeColorize(avl_window_data *Window);
  protected: //options
    void GetOptions(void);
    void SetOptions(void);
    int GetItemCount(void);
    int DialogWidth(void);
    int DialogHeight(void);
    INT_PTR DialogProc(HANDLE hDlg,int Msg,int Param1,void* Param2);
    void InitItems(FarDialogItem *DialogItems);
    void StoreItems(CFarDialog& Dialog);
    int Root(HANDLE Handle);
  public:
    TAutoCompletion();
    ~TAutoCompletion();
    int ProcessEditorInput(const INPUT_RECORD *Rec);
    int ProcessEditorEvent(int Event,void *Param);
    bool CompleteWord(void);
  friend DWORD WINAPI SearchThread(LPVOID lpvThreadParm);
};

#define ACMPL_DIALOG_ITEMS (IDeleteKeyCfg+1)

enum
{
  IAcceptFromMenu=CMPL_DIALOG_ITEMS,
  IMinPreWordLenLabel,
  IMinPreWordLen,
  IAcceptCharsLabel,
  IAcceptChars,
  IAcceptKeyLabel,
  IAcceptKey,
  IAcceptKeyCfg,
  IDeleteKeyLabel,
  IDeleteKey,
  IDeleteKeyCfg,
};

extern bool SelectColor(int *fg,int *bg);

#endif
