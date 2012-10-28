/*
    cmpl.hpp
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

#ifndef __CMPL_H__
#define __CMPL_H__

#include "string.hpp"
#include "avl_words.hpp"
#include "avl_windows.hpp"

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

union Key
{
  int KeyCode;
  TCHAR KeyName[256];
};

class TCompletion
{
  protected: //completion params
    bool WorkInsideWord;
    bool BrowseDownward;
    bool CaseSensitive;
    bool ConsiderDigitAsChar;
    bool PartialCompletion;
    bool AddTrailingSpace;

    __int64 MinPreWordLen;
    __int64 MinWordLen;
    __int64 BrowseLineCnt;
    __int64 WordsToFindCnt;

    TCHAR BrowseLineCntText[21];
    TCHAR WordsToFindCntText[21];
    TCHAR MinWordLenText[21];

    TCHAR AdditionalLetters[256];
    TCHAR ConfigHelpTopic[30];

    __int64 AsteriskSymbol;
  protected: //thread flag
    volatile LONG Stop;
  protected: //completion state
    string Word;
    int WordPos;
    avl_word_tree WordList;
    void Cleanup(void);
  protected: //completion functions
    int GetPreWord(void);
    int DoSearch(void);
    void AddWords(const UTCHAR *Line,int Len,int Direction);
    void AddWord(const string &NewWord);
    void InsertWordIntoList(const string &NewWord);
    UTCHAR *FindWordStart(const UTCHAR *Line,int Len);
    string PutWord(string NewWord);
    void SetCurPos(int NewPos,int NewRow=-1);
  protected: //common functions
    bool IsAlpha(unsigned int c);
    avl_window_data *GetLocalData(void);
  protected: //options
    static bool GetValue(HANDLE Handle,size_t Root,const TCHAR* Name,bool Default);
    static __int64 GetValue(HANDLE Handle,size_t Root,const TCHAR* Name,__int64 Default);
    static bool GetValue(HANDLE Handle,size_t Root,const TCHAR* Name,TCHAR* Value,size_t Size);
    static size_t GetValue(HANDLE Handle,size_t Root,const TCHAR* Name,void* Value,size_t Size);
    static void SetValue(HANDLE Handle,size_t Root,const TCHAR* Name,__int64 Value);
    static void SetValue(HANDLE Handle,size_t Root,const TCHAR* Name,TCHAR* Value);
    static void SetValue(HANDLE Handle,size_t Root,const TCHAR* Name,const void* Value,size_t Size);
    void GetOptions(void);
    virtual void SetOptions(void);
    virtual int GetItemCount(void)=0;
    virtual int DialogWidth(void)=0;
    virtual int DialogHeight(void)=0;
    virtual INT_PTR DialogProc(HANDLE hDlg,int Msg,int Param1,void* Param2)=0;
    virtual void InitItems(FarDialogItem *DialogItems);
    virtual void StoreItems(CFarDialog& Dialog);
    virtual size_t Root(HANDLE Handle);
  public:
    TCompletion();
    virtual ~TCompletion();
    void ShowDialog();
  friend intptr_t WINAPI ConfigDialogProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2);
};

#define CMPL_DIALOG_ITEMS (IAddTrailingSpace+1)

enum
{
  ICfg,IOk,ICancel,IAdditional,
  IWorkInsideWord,
  ICaseSensitive,
  IConsiderDigitAsChar,
  IDummy1,
  IAdditionalLetters,
  IBrowseDownward,
  IDummy2,
  IBrowseLineCnt,
  IDummy3,
  IWordsToFindCnt,
  IDummy4,
  IMinWordLen,
  IPartialCompletion,
  IAddTrailingSpace,
};

#endif
