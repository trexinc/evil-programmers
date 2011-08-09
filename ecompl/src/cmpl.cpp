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

TCompletion::TCompletion()
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

  Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
  if(ei.CurState&ECSTATE_LOCKED) return 0;
  avl_window_data *Window=windows->query(ei.EditorID);
  if(!Window) return 0;

  Window->row=ei.CurLine;
  Window->col=ei.CurPos;

  Word.clear();

  gs.StringNumber=-1; // current string
  Info.EditorControl(-1,ECTL_GETSTRING,0,&gs);
  // в начале и в конце строки искать нечего!!!
  if(ei.CurPos>0&&ei.CurPos<=gs.StringLength)
  {
    string Line((const UTCHAR *)gs.StringText,gs.StringLength);
    WordPos=ei.CurPos;

    // Если WorkInsideWord==FALSE, то под курсором не должно быть буквы
    if(!IsAlpha(Line[(size_t)WordPos])||WorkInsideWord)
    {
      // ищем начало слова
      while((WordPos&&IsAlpha(Line[(size_t)WordPos-1]))||(WordPos&&AsteriskSymbol&&Line[(size_t)WordPos-1]==AsteriskSymbol)) WordPos--;
      if(WordPos<ei.CurPos)
      {
        //раньше здесь была проверка на букву в начале слова и на не превышение длины слова длины строки
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

  int Line2Browse[2]; // Сколько строк будем просматривать вперед и назад

  WordList.clear();
  Info.EditorControl(-1,ECTL_GETINFO,0,&ei); //SEELATER use stored data for currsor position and check eid
  gs.StringNumber=-1; // current string
  Info.EditorControl(-1,ECTL_GETSTRING,0,&gs);

  string Line((const UTCHAR *)gs.StringText,gs.StringLength);
  if(Line.length()==(size_t)gs.StringLength)
  {
    // просматриваем начало текущей строки
    AddWords(Line,ei.CurPos,0);
    // а теперь - конец (без Word)
    if(BrowseDownward) AddWords(Line.get()+ei.CurPos,gs.StringLength-ei.CurPos,1);
  }
  Line2Browse[0]=MIN(ei.CurLine,BrowseLineCnt); // сверху
  Line2Browse[1]=MIN(ei.TotalLines-ei.CurLine-1,BrowseLineCnt); // снизу
  int LinesCount=MAX(Line2Browse[0], Line2Browse[1]); // для запуска основного цикла
  gs.StringNumber=-1; // тек. строка
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
      sp.CurLine=ei.CurLine+(j?i:-i); //вычисляем тек. строку
      Info.EditorControl(-1,ECTL_SETPOSITION,0,&sp);
      Info.EditorControl(-1,ECTL_GETSTRING,0,&gs);
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
  Info.EditorControl(-1,ECTL_SETPOSITION,0,&sp);
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

// а конкретно проверка по совпадению
// производится здесь
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
  Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
  return windows->query(ei.EditorID);
}

string TCompletion::PutWord(string NewWord)
{
  //SEELATER check editor before insert
  EditorInfo ei; EditorGetString gs;
  int OldPos;
  string OverwritedText;
  Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
  OldPos=ei.CurPos;
  if(AddTrailingSpace) NewWord+=' ';
  SetCurPos(WordPos);

  gs.StringNumber=-1;
  Info.EditorControl(-1,ECTL_GETSTRING,0,&gs);
  if(!ei.Overtype)
  {
    if(gs.StringLength>WordPos) OverwritedText((const UTCHAR *)&gs.StringText[WordPos],MIN(ei.CurPos-WordPos,gs.StringLength-WordPos));
    for(int i=WordPos;i<ei.CurPos;i++) Info.EditorControl(-1,ECTL_DELETECHAR,0,NULL);
    //workaround
    Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
    if(ei.BlockType==BTYPE_STREAM&&ei.BlockStartLine==ei.CurLine)
    {
      gs.StringNumber=-1;
      Info.EditorControl(-1,ECTL_GETSTRING,0,&gs);
      if(gs.SelStart==-1)
      {
        EditorSelect es={BTYPE_NONE,0,0,0,0};
        Info.EditorControl(-1,ECTL_SELECT,0,&es);
      }
    }
  }
  else
  {
    if(gs.StringLength>WordPos) OverwritedText((const UTCHAR *)&gs.StringText[WordPos],MIN(NewWord.length(),(size_t)(gs.StringLength-WordPos)));
  }

  Info.EditorControl(-1,ECTL_INSERTTEXT,0,NewWord.get());
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
  Info.EditorControl(-1,ECTL_SETPOSITION,0,&sp);
}

bool TCompletion::GetValue(HANDLE Handle,int Root,const TCHAR* Name,bool Default)
{
  bool result=Default;
  FarSettingsItem item={Root,Name,FST_QWORD};
  if(Info.SettingsControl(Handle,SCTL_GET,0,&item))
  {
    result=item.Number?true:false;
  }
  return result;
}

__int64 TCompletion::GetValue(HANDLE Handle,int Root,const TCHAR* Name,__int64 Default)
{
  __int64 result=Default;
  FarSettingsItem item={Root,Name,FST_QWORD};
  if(Info.SettingsControl(Handle,SCTL_GET,0,&item))
  {
    result=item.Number;
  }
  return result;
}

bool TCompletion::GetValue(HANDLE Handle,int Root,const TCHAR* Name,TCHAR* Value,size_t Size)
{
  FarSettingsItem item={Root,Name,FST_STRING};
  if(Info.SettingsControl(Handle,SCTL_GET,0,&item))
  {
    _tcsncpy(Value,item.String,Size-1);
    Value[Size-1]=0;
    return true;
  }
  return false;
}

size_t TCompletion::GetValue(HANDLE Handle,int Root,const TCHAR* Name,void* Value,size_t Size)
{
  FarSettingsItem item={Root,Name,FST_DATA};
  if(Info.SettingsControl(Handle,SCTL_GET,0,&item))
  {
    size_t result=(item.Data.Size>Size)?Size:item.Data.Size;
    memcpy(Value,item.Data.Data,result);
    return result;
  }
  return 0;
}

void TCompletion::SetValue(HANDLE Handle,int Root,const TCHAR* Name,__int64 Value)
{
  FarSettingsItem item={Root,Name,FST_QWORD};
  item.Number=Value;
  Info.SettingsControl(Handle,SCTL_SET,0,&item);
}

void TCompletion::SetValue(HANDLE Handle,int Root,const TCHAR* Name,TCHAR* Value)
{
  FarSettingsItem item={Root,Name,FST_STRING};
  item.String=Value;
  Info.SettingsControl(Handle,SCTL_SET,0,&item);
}

void TCompletion::SetValue(HANDLE Handle,int Root,const TCHAR* Name,const void* Value,size_t Size)
{
  FarSettingsItem item={Root,Name,FST_DATA};
  item.Data.Size=Size;
  item.Data.Data=Value;
  Info.SettingsControl(Handle,SCTL_SET,0,&item);
}

int TCompletion::Root(HANDLE Handle)
{
  return 0;
}

void TCompletion::GetOptions(void)
{
  FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
  if(Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
  {
    int root=Root(settings.Handle);
    WorkInsideWord=GetValue(settings.Handle,root,_T("WorkInsideWord"),WorkInsideWord);
    BrowseDownward=GetValue(settings.Handle,root,_T("BrowseDownward"),BrowseDownward);
    CaseSensitive=GetValue(settings.Handle,root,_T("CaseSensitive"),CaseSensitive);
    ConsiderDigitAsChar=GetValue(settings.Handle,root,_T("ConsiderDigitAsChar"),ConsiderDigitAsChar);
    PartialCompletion=GetValue(settings.Handle,root,_T("PartialCompletion"),PartialCompletion);
    AddTrailingSpace=GetValue(settings.Handle,root,_T("AddTrailingSpace"),AddTrailingSpace);

    MinPreWordLen=GetValue(settings.Handle,root,_T("MinPreWordLen"),MinPreWordLen);
    MinWordLen=GetValue(settings.Handle,root,_T("MinWordLen"),MinWordLen);
    BrowseLineCnt=GetValue(settings.Handle,root,_T("BrowseLineCnt"),BrowseLineCnt);
    WordsToFindCnt=GetValue(settings.Handle,root,_T("WordsToFindCnt"),WordsToFindCnt);

    GetValue(settings.Handle,root,_T("AdditionalLetters"),AdditionalLetters,ArraySize(AdditionalLetters));

    FarSettingsItem item={root,L"1234",FST_SUBKEY};
    Info.SettingsControl(settings.Handle,SCTL_SET,0,&item);


    Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
  }
}

void TCompletion::SetOptions(void)
{
  FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
  if(Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
  {
    int root=Root(settings.Handle);
    SetValue(settings.Handle,root,_T("WorkInsideWord"),WorkInsideWord);
    SetValue(settings.Handle,root,_T("CaseSensitive"),CaseSensitive);
    SetValue(settings.Handle,root,_T("BrowseDownward"),BrowseDownward);
    SetValue(settings.Handle,root,_T("BrowseLineCnt"),BrowseLineCnt);
    SetValue(settings.Handle,root,_T("MinPreWordLen"),MinPreWordLen);
    SetValue(settings.Handle,root,_T("MinWordLen"),MinWordLen);
    SetValue(settings.Handle,root,_T("WordsToFindCnt"),WordsToFindCnt);
    SetValue(settings.Handle,root,_T("ConsiderDigitAsChar"),ConsiderDigitAsChar);
    SetValue(settings.Handle,root,_T("PartialCompletion"),PartialCompletion);
    SetValue(settings.Handle,root,_T("AddTrailingSpace"),AddTrailingSpace);
    SetValue(settings.Handle,root,_T("AdditionalLetters"),AdditionalLetters);
    Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
  }
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
    DialogItems[i].Type=(FARDIALOGITEMTYPES)DialogElements[i][0];
    DialogItems[i].X1=DialogElements[i][1];
    DialogItems[i].Y1=DialogElements[i][2];
    DialogItems[i].X2=DialogElements[i][3];
    DialogItems[i].Y2=0;
    DialogItems[i].Selected=0;
    DialogItems[i].Flags=0;
    DialogItems[i].History=NULL;
    DialogItems[i].Mask=NULL;
    DialogItems[i].UserData=NULL;
    INIT_DLG_DATA(DialogItems[i],GetMsg(Msgs[i])); // Надписи на эл-тах диалога
  }

  DialogItems[ICfg].X2=DialogWidth()-2;
  DialogItems[ICfg].Y2=DialogHeight()-1;

  // выбран
  DialogItems[IWorkInsideWord].Flags|=DIF_FOCUS;
  // кнопки
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

  // Что будет в строках ввода
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

INT_PTR WINAPI ConfigDialogProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
  TCompletion *sender;
  if(Msg==DN_INITDIALOG)
  {
    Info.SendDlgMessage(hDlg,DM_SETDLGDATA,0,Param2);
    sender=(TCompletion *)Param2;
    Info.SendDlgMessage(hDlg,DM_SETMAXTEXTLENGTH,IAdditionalLetters,(void*)(sizeof(sender->AdditionalLetters)-1));
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
    int DlgCode=dialog.Execute(MainGuid,CmplGuid,-1,-1,DialogWidth(),DialogHeight(),ConfigHelpTopic,DialogItems,GetItemCount(),0,0,ConfigDialogProc,this);
    if(DlgCode==IOk)
    {
      StoreItems(dialog);
      SetOptions();
    }
    HeapFree(GetProcessHeap(),0,DialogItems);
  }
}
