/*
    cmpl.cpp
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

#include "MatPat.h"
#include "EditCmpl.hpp"
#include "cmpl.hpp"
#include "string.hpp"
#include "language.hpp"
#include "guid.hpp"
#include <initguid.h>
// {9EAE0BDB-8FBA-4fe8-A710-C0FDD58A046A}
DEFINE_GUID(CmplGuid, 0x9eae0bdb, 0x8fba, 0x4fe8, 0xa7, 0x10, 0xc0, 0xfd, 0xd5, 0x8a, 0x4, 0x6a);

TCompletion::TCompletion(const TCHAR *RegRoot)
{
  Stop=FALSE;

  WorkInsideWord=true;
  BrowseDownward=true;
  CaseSensitive=false;
  ConsiderDigitAsChar=true;
  PartialCompletion=false;
  AddTrailingSpace=false;

  MinPreWordLen=1;
  MinWordLen=2;
  BrowseLineCnt=1000;
  WordsToFindCnt=20;

  AdditionalLetters[0]='_';
  AdditionalLetters[1]=0;

  AsteriskSymbol=0;

  _tcscpy(RegKey,RegRoot);
  _tcscat(RegKey,_T("\\EditCompletion"));

  ConfigHelpTopic[0]=0;
}

TCompletion::~TCompletion()
{

}

void TCompletion::Cleanup(void)
{
  WordList.clear();
  Word.clear();
}

int TCompletion::GetPreWord(void)
{
  EditorInfo ei;
  EditorGetString gs;

  Info.EditorControl(ECTL_GETINFO,&ei);
  if(ei.CurState&ECSTATE_LOCKED) return 0;
  avl_window_data *Window=windows->query(ei.EditorID);
  if(!Window) return 0;

  Window->row=ei.CurLine;
  Window->col=ei.CurPos;

  Word.clear();

  gs.StringNumber=-1; // current string
  Info.EditorControl(ECTL_GETSTRING,&gs);
  // � ��砫� � � ���� ��ப� �᪠�� ��祣�!!!
  if(ei.CurPos>0&&ei.CurPos<=gs.StringLength)
  {
    string Line((const UTCHAR *)gs.StringText,gs.StringLength);
    WordPos=ei.CurPos;

    // �᫨ WorkInsideWord==FALSE, � ��� ����஬ �� ������ ���� �㪢�
    if(!IsAlpha(Line[(size_t)WordPos])||WorkInsideWord)
    {
      // �饬 ��砫� ᫮��
      while((WordPos&&IsAlpha(Line[(size_t)WordPos-1]))||(WordPos&&AsteriskSymbol&&Line[(size_t)WordPos-1]==AsteriskSymbol)) WordPos--;
      if(WordPos<ei.CurPos)
      {
        //࠭�� ����� �뫠 �஢�ઠ �� �㪢� � ��砫� ᫮�� � �� �� �ॢ�襭�� ����� ᫮�� ����� ��ப�
        Word(Line+WordPos,ei.CurPos-WordPos);
      }
    }
  }
  if(Word.length()<(size_t)MinPreWordLen) Word.clear();
  return Word.length();
}

int TCompletion::DoSearch(void)
{
  if(!Word.length()) return 0;
  EditorGetString gs;
  EditorInfo ei;
  EditorSetPosition sp;

  int Line2Browse[2]; // ����쪮 ��ப �㤥� ��ᬠ�ਢ��� ���। � �����

  WordList.clear();
  Info.EditorControl(ECTL_GETINFO,&ei); //SEELATER use stored data for currsor position and check eid
  gs.StringNumber=-1; // current string
  Info.EditorControl(ECTL_GETSTRING,&gs);

  string Line((const UTCHAR *)gs.StringText,gs.StringLength);
  if(Line.length()==(size_t)gs.StringLength)
  {
    // ��ᬠ�ਢ��� ��砫� ⥪�饩 ��ப�
    AddWords(Line,ei.CurPos,0);
    // � ⥯��� - ����� (��� Word)
    if(BrowseDownward) AddWords(Line.get()+ei.CurPos,gs.StringLength-ei.CurPos,1);
  }
  Line2Browse[0]=MIN(ei.CurLine,BrowseLineCnt); // ᢥ���
  Line2Browse[1]=MIN(ei.TotalLines-ei.CurLine-1,BrowseLineCnt); // ᭨��
  int LinesCount=MAX(Line2Browse[0], Line2Browse[1]); // ��� ����᪠ �᭮����� 横��
  gs.StringNumber=-1; // ⥪. ��ப�
  sp.CurPos=-1;
  sp.CurTabPos=-1;
  sp.TopScreenLine=-1;
  sp.LeftPos=-1;
  sp.Overtype=-1;
  for(int i=1;(i<=LinesCount)&&(WordList.count()<WordsToFindCnt);i++)
  {
    for(int j=0;j<1+BrowseDownward;j++)
    {
      if(Line2Browse[j]<i) continue;
      sp.CurLine=ei.CurLine+(j?i:-i); //����塞 ⥪. ��ப�
      Info.EditorControl(ECTL_SETPOSITION,&sp);
      Info.EditorControl(ECTL_GETSTRING,&gs);
      Line((const UTCHAR *)gs.StringText,gs.StringLength);
      AddWords(Line,Line.length(),j);
      if(WordList.count()>=WordsToFindCnt) break;
    }
    if(Stop) break;
  }
  sp.CurLine=ei.CurLine;
  sp.CurPos=ei.CurPos;
  sp.TopScreenLine=ei.TopScreenLine;
  sp.LeftPos=ei.LeftPos;
  Info.EditorControl(ECTL_SETPOSITION,&sp);
  return WordList.count();
}

void TCompletion::AddWords(const UTCHAR *Line,int Len,int Direction)
{
  bool IsInsideWord=false;
  UTCHAR CurChar;
  string TmpWord;
  TmpWord.hash(Word.length());
  for(int i=0;i<Len;i++)
  {
    CurChar=Line[(Direction>0)?i:(Len-i-1)];
    if(IsInsideWord)
    {
      if(!IsAlpha(CurChar))
      {
        IsInsideWord=false;
        if(TmpWord.length())
        {
          if(Direction<=0) TmpWord.reverse();
          AddWord(TmpWord);
          TmpWord.clear();
        }
      }
      else
      {
        TmpWord+=CurChar;
      }
    }
    else if(IsAlpha(CurChar))
    {
      IsInsideWord=true;
      TmpWord+=CurChar;
    }
  }
  if(TmpWord.length())
  {
    if(Direction<=0) TmpWord.reverse();
    AddWord(TmpWord);
  }
}

// � �����⭮ �஢�ઠ �� ᮢ�������
// �ந�������� �����
void TCompletion::AddWord(const string &NewWord)
{
  if(NewWord.length()&&(int)WordList.count()<WordsToFindCnt&&NewWord.length()>Word.length()&&(int)NewWord.length()>=MinWordLen)
  {
    if(AsteriskSymbol&&_tcschr((const TCHAR*)(const UTCHAR*)Word,(int)AsteriskSymbol))
    {
      string TmpWord(Word),TmpNewWord(NewWord);
      if(!CaseSensitive)
      {
        FSF.LUpperBuf((TCHAR*)TmpWord.get(),TmpWord.length());
        FSF.LUpperBuf((TCHAR*)TmpNewWord.get(),TmpNewWord.length());
      }
      if(MatchPattern(TmpNewWord.get(),TmpWord.get(),AsteriskSymbol)) InsertWordIntoList(NewWord);
    }
    else
    {
      if(CaseSensitive)
      {
        if(!_tcsncmp((const TCHAR*)(const UTCHAR*)NewWord,(const TCHAR*)(const UTCHAR*)Word,Word.length())) InsertWordIntoList(NewWord);
      }
      else
      {
        if(!FSF.LStrnicmp((const TCHAR*)(const UTCHAR*)NewWord,(const TCHAR*)(const UTCHAR*)Word,Word.length())) InsertWordIntoList(NewWord);
      }
    }
  }
}

void TCompletion::InsertWordIntoList(const string &NewWord)
{
  avl_word_data *Add=new avl_word_data(NewWord);
  Add=WordList.insert(Add);
  if(Add) Add->inc_ref();
}

bool TCompletion::IsAlpha(unsigned int c)
{
  bool ret=false;
  if(c)
  {
    if(ConsiderDigitAsChar)
      ret=FSF.LIsAlphanum(c);
    else
      ret=FSF.LIsAlpha(c);
    if(!ret)
      if(_tcschr(AdditionalLetters,c))
        ret=true;
  }
  return ret;
}

avl_window_data *TCompletion::GetLocalData(void)
{
  EditorInfo ei;
  Info.EditorControl(ECTL_GETINFO,&ei);
  return windows->query(ei.EditorID);
}

string TCompletion::PutWord(string NewWord)
{
  //SEELATER check editor before insert
  EditorInfo ei; EditorGetString gs;
  int OldPos;
  string OverwritedText;
  Info.EditorControl(ECTL_GETINFO,&ei);
  OldPos=ei.CurPos;
  if(AddTrailingSpace) NewWord+=' ';
  SetCurPos(WordPos);

  gs.StringNumber=-1;
  Info.EditorControl(ECTL_GETSTRING,&gs);
  if(!ei.Overtype)
  {
    if(gs.StringLength>WordPos) OverwritedText((const UTCHAR *)&gs.StringText[WordPos],MIN(ei.CurPos-WordPos,gs.StringLength-WordPos));
    for(int i=WordPos;i<ei.CurPos;i++) Info.EditorControl(ECTL_DELETECHAR,NULL);
    //workaround
    Info.EditorControl(ECTL_GETINFO,&ei);
    if(ei.BlockType==BTYPE_STREAM&&ei.BlockStartLine==ei.CurLine)
    {
      gs.StringNumber=-1;
      Info.EditorControl(ECTL_GETSTRING,&gs);
      if(gs.SelStart==-1)
      {
        EditorSelect es={BTYPE_NONE,0,0,0,0};
        Info.EditorControl(ECTL_SELECT,&es);
      }
    }
  }
  else
  {
    if(gs.StringLength>WordPos) OverwritedText((const UTCHAR *)&gs.StringText[WordPos],MIN(NewWord.length(),(size_t)(gs.StringLength-WordPos)));
  }

  Info.EditorControl(ECTL_INSERTTEXT,(void *)NewWord.get());
  SetCurPos(OldPos);
  return OverwritedText;
}

void TCompletion::SetCurPos(int NewPos,int NewRow)
{
  struct EditorSetPosition sp;
  sp.CurLine=NewRow;
  sp.CurPos=NewPos;
  sp.CurTabPos=-1;
  sp.TopScreenLine=-1;
  sp.LeftPos=-1;
  sp.Overtype=-1;
  Info.EditorControl(ECTL_SETPOSITION,&sp);
}

DWORD TCompletion::GetRegKey(const TCHAR* ValueName,DWORD Default)
{
  DWORD result=Default;
  HKEY hKey; DWORD Type; DWORD DataSize=0;
  if((RegOpenKeyEx(HKEY_CURRENT_USER,RegKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
  {
    DataSize=sizeof(result);
    RegQueryValueEx(hKey,ValueName,0,&Type,(LPBYTE)&result,&DataSize);
    RegCloseKey(hKey);
  }
  return result;
}

void TCompletion::SetRegKey(const TCHAR* ValueName,DWORD Value)
{
  HKEY hKey;
  DWORD Disposition;
  if((RegCreateKeyEx(HKEY_CURRENT_USER,RegKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
  {
    RegSetValueEx(hKey,ValueName,0,REG_DWORD,(LPBYTE)&Value,sizeof(Value));
    RegCloseKey(hKey);
  }
}

void TCompletion::GetRegKey(const TCHAR* ValueName,TCHAR* buffer,DWORD size)
{
  HKEY hKey; DWORD Type;
  if((RegOpenKeyEx(HKEY_CURRENT_USER,RegKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
  {
    RegQueryValueEx(hKey,ValueName,0,&Type,(LPBYTE)buffer,&size);
    RegCloseKey(hKey);
  }
}

void TCompletion::SetRegKey(const TCHAR* ValueName,TCHAR* buffer)
{
  HKEY hKey;
  DWORD Disposition;
  if((RegCreateKeyEx(HKEY_CURRENT_USER,RegKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
  {
    RegSetValueEx(hKey,ValueName,0,REG_SZ,(LPBYTE)buffer,(_tcslen(buffer)+1)*sizeof(TCHAR));
    RegCloseKey(hKey);
  }
}

int TCompletion::GetRegKey(const TCHAR* ValueName,const TCHAR* Default)
{
  int result=-1;
  Key Key; HKEY hKey; DWORD Type; DWORD DataSize=0;
  if((RegOpenKeyEx(HKEY_CURRENT_USER,RegKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
  {
    DataSize=sizeof(Key);
    LONG res=RegQueryValueEx(hKey,ValueName,0,&Type,(LPBYTE)&Key,&DataSize);
    if(res!=ERROR_SUCCESS||(Type!=REG_DWORD&&Type!=REG_SZ))
    {
      Type=REG_SZ;
      _tcscpy(Key.KeyName,Default);
    }
    if(Type==REG_DWORD) result=Key.KeyCode;
    if(Type==REG_SZ) result=FSF.FarNameToKey(Key.KeyName);
    RegCloseKey(hKey);
  }
  return result;
}

void TCompletion::GetOptions(void)
{
  WorkInsideWord=GetRegKey(_T("WorkInsideWord"),WorkInsideWord);
  CaseSensitive=GetRegKey(_T("CaseSensitive"),CaseSensitive);
  BrowseDownward=GetRegKey(_T("BrowseDownward"),BrowseDownward);
  MinPreWordLen=GetRegKey(_T("MinPreWordLen"),MinPreWordLen);
  MinWordLen=GetRegKey(_T("MinWordLen"),MinWordLen);
  BrowseLineCnt=GetRegKey(_T("BrowseLineCnt"),BrowseLineCnt);
  WordsToFindCnt=GetRegKey(_T("WordsToFindCnt"),WordsToFindCnt);
  ConsiderDigitAsChar=GetRegKey(_T("ConsiderDigitAsChar"),ConsiderDigitAsChar);
  PartialCompletion=GetRegKey(_T("PartialCompletion"),PartialCompletion);
  AddTrailingSpace=GetRegKey(_T("AddTrailingSpace"),AddTrailingSpace);
  GetRegKey(_T("AdditionalLetters"),AdditionalLetters,sizeof(AdditionalLetters));
}

void TCompletion::SetOptions(void)
{
  SetRegKey(_T("WorkInsideWord"),WorkInsideWord);
  SetRegKey(_T("CaseSensitive"),CaseSensitive);
  SetRegKey(_T("BrowseDownward"),BrowseDownward);
  SetRegKey(_T("BrowseLineCnt"),BrowseLineCnt);
  SetRegKey(_T("MinPreWordLen"),MinPreWordLen);
  SetRegKey(_T("MinWordLen"),MinWordLen);
  SetRegKey(_T("WordsToFindCnt"),WordsToFindCnt);
  SetRegKey(_T("ConsiderDigitAsChar"),ConsiderDigitAsChar);
  SetRegKey(_T("PartialCompletion"),PartialCompletion);
  SetRegKey(_T("AddTrailingSpace"),AddTrailingSpace);
  SetRegKey(_T("AdditionalLetters"),AdditionalLetters);
}

void TCompletion::InitItems(FarDialogItem *DialogItems)
{
  int Msgs[]=
  {
    MOk,MOk,MCancel,MOk,
    MWorkInsideWord,
    MCaseSensitive,
    MConsiderDigitAsChar,
    MAdditionalLetters,MAdditionalLetters,
    MBrowseDownward,
    MBrowseLineCnt,MBrowseLineCnt,
    MWordsToFindCnt,MWordsToFindCnt,
    MMinWordLen,MMinWordLen,
    MPartialCompletion,
    MAddTrailingSpace,
  };
  int DialogElements[][4]=
  {
    {DI_DOUBLEBOX, 1,  0,  0               },
    {DI_BUTTON,    0,  0,  0               },
    {DI_BUTTON,    0,  0,  0               },
    {DI_BUTTON,    0,  0,  0               },
    {DI_CHECKBOX,  3,  2,  0               }, // WorkInsideWord
    {DI_CHECKBOX,  3,  3,  0               }, // CaseSensitive
    {DI_CHECKBOX,  3,  4,  0               }, // ConsiderDigitAsChar
    {DI_TEXT,      3,  5,  0               }, //
    {DI_EDIT,      3,  6,  DialogWidth()-4 }, // AdditionalLetters
    {DI_CHECKBOX,  3,  7,  0               }, // BrowseDownward
    {DI_TEXT,      8,  8,  0               }, //
    {DI_FIXEDIT,   3,  8,  6               }, // BrowseLineCnt
    {DI_TEXT,      6,  9,  0               }, //
    {DI_FIXEDIT,   3,  9,  4               }, // WordsToFindCnt
    {DI_TEXT,      6, 10,  0               }, //
    {DI_FIXEDIT,   3, 10,  4               }, // MinWordLen
    {DI_CHECKBOX,  3, 11,  0               }, // PartialCompletion
    {DI_CHECKBOX,  3, 12,  0               }, // AddTrailingSpace
  };
  for(unsigned int i=0;i<(sizeof(Msgs)/sizeof(Msgs[0]));i++)
  {
    DialogItems[i].Type=DialogElements[i][0];
    DialogItems[i].X1=DialogElements[i][1];
    DialogItems[i].Y1=DialogElements[i][2];
    DialogItems[i].X2=DialogElements[i][3];
    DialogItems[i].Y2=0;
    DialogItems[i].Selected=0;
    DialogItems[i].Flags=0;
    DialogItems[i].History=NULL;
    DialogItems[i].Mask=NULL;
    DialogItems[i].UserParam=0;
    INIT_DLG_DATA(DialogItems[i],GetMsg(Msgs[i])); // ������ �� �-�� �������
  }

  DialogItems[ICfg].X2=DialogWidth()-2;
  DialogItems[ICfg].Y2=DialogHeight()-1;

  // ��࠭
  DialogItems[IWorkInsideWord].Flags|=DIF_FOCUS;
  // ������
  DialogItems[IOk].Flags=DIF_CENTERGROUP|DIF_DEFAULTBUTTON;
  DialogItems[ICancel].Flags=DIF_CENTERGROUP;
  DialogItems[IAdditional].Flags=DIF_HIDDEN;

  DialogItems[IOk].Y1=DialogHeight()-2;
  DialogItems[ICancel].Y1=DialogHeight()-2;

  DialogItems[IWorkInsideWord].Selected=WorkInsideWord;
  DialogItems[ICaseSensitive].Selected=CaseSensitive;
  DialogItems[IBrowseDownward].Selected=BrowseDownward;
  DialogItems[IConsiderDigitAsChar].Selected=ConsiderDigitAsChar;
  DialogItems[IPartialCompletion].Selected=PartialCompletion;
  DialogItems[IAddTrailingSpace].Selected=AddTrailingSpace;

  // �� �㤥� � ��ப�� �����
  DLG_DATA_ITOA(DialogItems[IBrowseLineCnt],BrowseLineCnt);
  DLG_DATA_ITOA(DialogItems[IWordsToFindCnt],WordsToFindCnt);
  DLG_DATA_ITOA(DialogItems[IMinWordLen],MinWordLen);
  INIT_DLG_DATA(DialogItems[IAdditionalLetters],AdditionalLetters);
}

void TCompletion::StoreItems(CFarDialog& Dialog)
{
  WorkInsideWord=Dialog.Check(IWorkInsideWord);
  CaseSensitive=Dialog.Check(ICaseSensitive);
  BrowseDownward=Dialog.Check(IBrowseDownward);
  ConsiderDigitAsChar=Dialog.Check(IConsiderDigitAsChar);
  PartialCompletion=Dialog.Check(IPartialCompletion);
  AddTrailingSpace=Dialog.Check(IAddTrailingSpace);
  BrowseLineCnt=FSF.atoi(Dialog.Str(IBrowseLineCnt));
  WordsToFindCnt=FSF.atoi(Dialog.Str(IWordsToFindCnt));
  MinWordLen=FSF.atoi(Dialog.Str(IMinWordLen));
  FSF.sprintf(AdditionalLetters,_T("%s"),Dialog.Str(IAdditionalLetters));
}

INT_PTR WINAPI ConfigDialogProc(HANDLE hDlg,int Msg,int Param1,INT_PTR Param2)
{
  TCompletion *sender;
  if(Msg==DN_INITDIALOG)
  {
    Info.SendDlgMessage(hDlg,DM_SETDLGDATA,0,Param2);
    sender=(TCompletion *)Param2;
    Info.SendDlgMessage(hDlg,DM_SETMAXTEXTLENGTH,IAdditionalLetters,sizeof(sender->AdditionalLetters)-1);
  }
  else sender=(TCompletion *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  return sender->DialogProc(hDlg,Msg,Param1,Param2);
}

void TCompletion::ShowDialog()
{
  FarDialogItem *DialogItems=(FarDialogItem *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,GetItemCount()*sizeof(FarDialogItem));
  if(DialogItems)
  {
    InitItems(DialogItems);
    CFarDialog dialog;
    int DlgCode=dialog.Execute(MainGuid,CmplGuid,-1,-1,DialogWidth(),DialogHeight(),ConfigHelpTopic,DialogItems,GetItemCount(),0,0,ConfigDialogProc,(DWORD)this);
    if(DlgCode==IOk)
    {
      StoreItems(dialog);
      SetOptions();
    }
    HeapFree(GetProcessHeap(),0,DialogItems);
  }
}
