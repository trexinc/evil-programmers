/*
    mcmpl.hpp
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

#ifndef __MCMPL_H__
#define __MCMPL_H__

#include "cmpl.hpp"

class TMenuCompletion: public TCompletion
{
  protected: //menu completion params
    int SingleVariantInMenu;
    int NotFoundSound;
    int SortListCount;
    TCHAR ShortCuts[80];
    int ShortCutsLen;
    TCHAR AcceptChars[256];
#ifdef UNICODE
    TCHAR AsteriskSymbolText[2];
    TCHAR SortListCountText[21];
#endif
  protected:
    bool ShowMenu(string &Selected);
  protected: //options
    void GetOptions(void);
    void SetOptions(void);
    int GetItemCount(void);
    int DialogWidth(void);
    int DialogHeight(void);
    long DialogProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    void InitItems(FarDialogItem *DialogItems);
    void StoreItems(DLG_REFERENCE Dialog);
  public:
    TMenuCompletion(const TCHAR *RegRoot);
    ~TMenuCompletion();
    bool CompleteWord(void);
};

#define MCMPL_FIRST_TEXT (CMPL_DIALOG_ITEMS+3)
#define MCMPL_DIALOG_ITEMS (IMenuAcceptChars+1)

enum
{
  ISingleVariantInMenu=CMPL_DIALOG_ITEMS,       // SingleVariantInMenu
  INotFoundSound,                               // NotFoundSound
  ISortListCount=MCMPL_FIRST_TEXT,              // SortListCount
  IAsteriskSymbol=MCMPL_FIRST_TEXT+2,           // AsteriskSymbol
  IMenuAcceptCharsLabel,
  IMenuAcceptChars,
};

#endif
