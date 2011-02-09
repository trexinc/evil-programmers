/*
    acmpl.cpp
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

#include "acmpl.hpp"
#include "EditCmpl.hpp"
#include "language.hpp"
#include "guid.hpp"
#include <initguid.h>
// {A28F2DC4-C362-4a82-AB2C-0081161171DD}
DEFINE_GUID(ACmplGuid, 0xa28f2dc4, 0xc362, 0x4a82, 0xab, 0x2c, 0x0, 0x81, 0x16, 0x11, 0x71, 0xdd);

TAutoCompletion::TAutoCompletion(const TCHAR *RegRoot): TCompletion(RegRoot)
{
  WorkInsideWord=false;
  CaseSensitive=true;
  ConsiderDigitAsChar=false;
  MinPreWordLen=2;
  WordsToFindCnt=3;
  PartialCompletion=true;

  AcceptFromMenu=false;
  AcceptKey=-1;
  DeleteKey=-1;
  Color=0x2F;
  AcceptChars[0]=0;
  _tcscat(RegKey,_T("\\AutoCompletion"));
  _tcscpy(ConfigHelpTopic,_T("ConfigAuto"));
  GetOptions();
}

TAutoCompletion::~TAutoCompletion()
{

}

int TAutoCompletion::ProcessEditorInput(const INPUT_RECORD *Rec)
{
  int IgnoreThisEvent=FALSE;
  avl_window_data *Window=GetLocalData();
  if(Window)
  {
    if(Rec->EventType==KEY_EVENT)
    {
      if(Rec->Event.KeyEvent.bKeyDown==TRUE||Rec->Event.KeyEvent.dwControlKeyState==0x4000000)
      {
        if(Window->Active&&Rec->Event.KeyEvent.wVirtualKeyCode!=0&&Rec->Event.KeyEvent.wVirtualScanCode!=0)
        {
          int FarKey=FSF.FarInputRecordToKey(Rec);
          if(FarKey==AcceptKey||(Rec->Event.KeyEvent.uChar.t_KEY_CHAR&&_tcschr(AcceptChars,Rec->Event.KeyEvent.uChar.t_KEY_CHAR)))
          {
            IgnoreThisEvent=AcceptVariant(Window)&&(FarKey==AcceptKey);
          }
          else if(Rec->Event.KeyEvent.wVirtualKeyCode!=VK_CONTROL&&Rec->Event.KeyEvent.wVirtualKeyCode!=VK_SHIFT&&Rec->Event.KeyEvent.wVirtualKeyCode!=VK_MENU)
          {
            DeleteVariant(Window);
            IgnoreThisEvent=(FarKey==DeleteKey);
          }
        }
        UTCHAR c=Rec->Event.KeyEvent.uChar.t_KEY_CHAR;
        DWORD control=Rec->Event.KeyEvent.dwControlKeyState&(LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_ALT_PRESSED|RIGHT_CTRL_PRESSED);
        if(IsAlpha(c)&&(control==0||control==(LEFT_CTRL_PRESSED|RIGHT_ALT_PRESSED)))
        {
          Window->On=true;
          EditorInfo ei;
          if(Info.EditorControl(ECTL_GETINFO,&ei)&&ei.BlockType==BTYPE_STREAM&&(ei.Options&EOPT_PERSISTENTBLOCKS)==0)
          {
            Window->BlockDeleted=true;
          }
        }
      }
    }
    else if((Rec->EventType==MOUSE_EVENT&&Rec->Event.MouseEvent.dwButtonState)||Rec->EventType==FARMACRO_KEY_EVENT)
    {
      DeleteVariant(Window);
    }
  }
  return IgnoreThisEvent;
}

int TAutoCompletion::ProcessEditorEvent(int Event,void *Param)
{
  if(Event==EE_CLOSE)
  {
    avl_window_data Add(*static_cast<int *>(Param));
    windows->remove(&Add);
  }
  else if(Event==EE_READ)
  {
    EditorInfo ei;
    if(Info.EditorControl(ECTL_GETINFO,&ei))
    {
      avl_window_data *Add=new avl_window_data(ei.EditorID);
      Add=windows->insert(Add);
      Add->clear();
    }
  }
  else
  {
    avl_window_data *Window=GetLocalData();
    if(Window)
    {
      if(Event==EE_REDRAW)
      {
        if(Window->On)
        {
          if(Window->BlockDeleted)
          {
            if((int)Param==1) Window->BlockDeleted=false;
          }
          else
          {
            Window->On=false;
            Info.EditorControl(ECTL_REDRAW,0);
            PutVariant(Window);
            Info.EditorControl(ECTL_REDRAW,0);
          }
        }
        else if(Window->Active) Colorize(Color,Window);
      }
      else if(Event==EE_SAVE)
      {
        DeleteVariant(Window);
      }
    }
  }
  return 0;
}

bool TAutoCompletion::CheckText(int Pos,int Row,avl_window_data *Window)
{
  SetCurPos(Pos,Row);
  EditorGetString gs;
  gs.StringNumber=-1; // current string
  Info.EditorControl(ECTL_GETSTRING,&gs);
  if(gs.StringLength>Pos)
  {
    string Line((const UTCHAR*)gs.StringText,gs.StringLength);
    if(!_tcsncmp((const TCHAR *)(const UTCHAR*)Line+Pos,(const TCHAR *)(const UTCHAR*)Window->Inserted,Window->Inserted.length()))
    {
      //set position to word start
      SetCurPos(Pos-Window->OldLen,Row);
      return true;
    }
  }
  return false;
}

void TAutoCompletion::DeleteVariant(avl_window_data *Window)
{
  if(Window->Active)
  {
    bool process=false;
    EditorInfo ei;
    Info.EditorControl(ECTL_GETINFO,&ei);
    Colorize(0,Window);
    Window->Active=false;
    {
      process=CheckText(Window->col,Window->row,Window);
      if(!process) process=CheckText(ei.CurPos,ei.CurLine,Window);
    }
    if(process)
    {
      if(!ei.Overtype)
        for(size_t i=0;i<(Window->AddedLen+Window->OldLen);i++) Info.EditorControl(ECTL_DELETECHAR,NULL);
      Info.EditorControl(ECTL_INSERTTEXT,(void *)Window->Rewrited.get());
    }
    SetCurPos(ei.CurPos,ei.CurLine);
    Window->Rewrited.clear();
    Window->Inserted.clear();
    Window->AddedLen=0;
    Window->OldLen=0;
    Info.EditorControl(ECTL_REDRAW,0);
  }
}

bool TAutoCompletion::AcceptVariant(avl_window_data *Window)
{
  bool Accepted=false;
  if(Window->Active)
  {
    Colorize(0,Window);
    Window->Active=false;
    Window->Rewrited.clear();
    SetCurPos(Window->col+Window->AddedLen,Window->row);
    if(PartialCompletion) Window->On=true;
    Info.EditorControl(ECTL_REDRAW,NULL);
    Accepted=true;
  }
  return Accepted;
}

DWORD WINAPI SearchThread(LPVOID lpvThreadParm)
{
  TAutoCompletion *cmpl=static_cast<TAutoCompletion *>(lpvThreadParm);
  return cmpl->DoSearch();
}

bool TAutoCompletion::PutVariant(avl_window_data *Window)
{
  DWORD SearchOk=FALSE;
  DeleteVariant(Window);
  if(GetPreWord())
  {
    { // start thread
      HANDLE handles[2];
      DWORD ThreadID;
      Stop=FALSE;
      handles[0]=CreateThread(NULL,0,SearchThread,this,CREATE_SUSPENDED,&ThreadID);
      handles[1]=CreateFile(_T("CONIN$"),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
      if(handles[0]&&handles[1])
      {
        ResumeThread(handles[0]);
        {
          bool poll=true;
          INPUT_RECORD *AllEvents=NULL,CurEvent;
          unsigned long AllEventsCount=0,CurEventCount;
          while(poll)
          {
            DWORD wfmo=WaitForMultipleObjects(sizeof(handles)/sizeof(handles[0]),handles,FALSE,INFINITE);
            switch(wfmo)
            {
              case WAIT_OBJECT_0:
                poll=false;
                break;
              case WAIT_OBJECT_0+1:
                ReadConsoleInput(handles[1],&CurEvent,1,&CurEventCount);
                if(CurEventCount==1)
                {
                  if(AllEvents)
                  {
                    INPUT_RECORD *NewAllEvents=(INPUT_RECORD *)HeapReAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,AllEvents,(AllEventsCount+1)*sizeof(INPUT_RECORD));
                    if(NewAllEvents)
                    {
                      AllEvents=NewAllEvents;
                      AllEvents[AllEventsCount++]=CurEvent;
                    }
                  }
                  else
                  {
                    AllEvents=(INPUT_RECORD *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(INPUT_RECORD));
                    if(AllEvents)
                    {
                      AllEvents[0]=CurEvent;
                      AllEventsCount=1;
                    }
                  }
                  if(CurEvent.EventType==KEY_EVENT&&CurEvent.Event.KeyEvent.bKeyDown)
                  {
                    InterlockedIncrement((LONG *)&Stop);
                    WaitForSingleObject(handles[0],INFINITE);
                    poll=false;
                  }
                }
                break;
            }
          }
          if(AllEvents)
          {
            WriteConsoleInput(handles[1],AllEvents,AllEventsCount,&CurEventCount);
            HeapFree(GetProcessHeap(),0,AllEvents);
          }
        }
        if(!GetExitCodeThread(handles[0],&SearchOk)) SearchOk=FALSE;
      }
    }
    if(SearchOk&&WordList.count()<=WordsToFindCnt)
    {
      string NewWord=WordList.get_top()->get_data()->get_data();
      if(PartialCompletion&&(WordList.get_partial().length()>Word.length()))
      {
        NewWord=WordList.get_partial();
      }
      Window->Rewrited=PutWord(NewWord);
      Window->Inserted=string(NewWord.get()+Word.length());
      Window->Active=true;
      Window->AddedLen=NewWord.length()-Word.length()+(AddTrailingSpace?1:0);
      Window->OldLen=Word.length();
    }
  }
  Cleanup();
  return false;
}

void TAutoCompletion::Colorize(int NewColor,avl_window_data *Window)
{
  EditorColor ec;
  ec.StringNumber=Window->row;
  ec.StartPos=Window->col;
  ec.EndPos=Window->col+Window->AddedLen-1;
  ec.Color=NewColor;
  Info.EditorControl(ECTL_ADDCOLOR,&ec);
}

bool TAutoCompletion::CompleteWord(void)
{
  bool WasCompleted=false;
  avl_window_data *Window=GetLocalData();
  if(Window)
  {
    if(Window->Active)
    {
      if(AcceptFromMenu)
      {
        AcceptVariant(Window);
        WasCompleted=true;
      }
      else
        DeleteVariant(Window);
    }
  }
  return WasCompleted;
}

void TAutoCompletion::GetOptions(void)
{
  TCompletion::GetOptions();
  Color=GetRegKey(_T("Color"),Color);
  AcceptKey=GetRegKey(_T("AcceptKey"),_T("Tab"));
  DeleteKey=GetRegKey(_T("DeleteKey"),_T(""));
  AcceptFromMenu=GetRegKey(_T("AcceptFromMenu"),AcceptFromMenu);
  GetRegKey(_T("AcceptChars"),AcceptChars,sizeof(AcceptChars));
}

void TAutoCompletion::SetOptions(void)
{
  TCompletion::SetOptions();
  SetRegKey(_T("Color"),Color);
  SetRegKey(_T("AcceptFromMenu"),AcceptFromMenu);
  SetRegKey(_T("AcceptChars"),AcceptChars);
  {
    TCHAR Key[256];
    if(!t_FarKeyToName(AcceptKey,Key,ArraySize(Key))) Key[0]=0;
    SetRegKey(_T("AcceptKey"),Key);
    if(!t_FarKeyToName(DeleteKey,Key,ArraySize(Key))) Key[0]=0;
    SetRegKey(_T("DeleteKey"),Key);
  }
}

int TAutoCompletion::GetItemCount(void)
{
  return ACMPL_DIALOG_ITEMS;
}

int TAutoCompletion::DialogWidth(void)
{
  return 46;
}

int TAutoCompletion::DialogHeight(void)
{
  return 25;
}

INT_PTR WINAPI GetKey(HANDLE hDlg,int Msg,int Param1,INT_PTR Param2)
{
  if(Msg==DN_INITDIALOG)
  {
    Info.SendDlgMessage(hDlg,DM_SETDLGDATA,0,Param2);
  }
  else if(Msg==DM_KEY)
  {
    TCHAR *KeyName=(TCHAR *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
    t_FarKeyToName(Param2,KeyName,256);
    Info.SendDlgMessage(hDlg,DM_CLOSE,-1,0);
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

INT_PTR TAutoCompletion::DialogProc(HANDLE hDlg,int Msg,int Param1,INT_PTR Param2)
{
  if(Msg==DN_BTNCLICK)
  {
    if(Param1==IAcceptKeyCfg||Param1==IDeleteKeyCfg)
    {
      Info.SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
      TCHAR KeyName[256];
      Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,IAcceptKey,(long)&KeyName);
      FarDialogItem DialogFrame;
      INIT_DLG_DATA(DialogFrame,GetMsg(MPressDesiredKey));
      DialogFrame.Type=DI_DOUBLEBOX;
      DialogFrame.X1=1;
      DialogFrame.Y1=1;
      DialogFrame.X2=_tcslen(DLG_DATA(DialogFrame))+4;
      DialogFrame.Y2=3;
      DialogFrame.Selected=1;
      DialogFrame.Flags=DIF_BOXCOLOR|DIF_FOCUS|DIF_DEFAULTBUTTON;
      int width=_tcslen(DLG_DATA(DialogFrame))+6;
      CFarDialog dialog;
      dialog.Execute(MainGuid,ACmplGuid,-1,-1,width,5,ConfigHelpTopic,&DialogFrame,1,0,0,GetKey,(DWORD)KeyName);
      Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,(Param1==IAcceptKeyCfg)?IAcceptKey:IDeleteKey,(long)&KeyName);
      Info.SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
      return TRUE;
    }
    if(Param1==IAdditional)
    {
      Info.SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
      int bg=(Dialog_Color&0xF0)>>4,fg=Dialog_Color&0x0F;
      if(SelectColor(&fg,&bg)) Dialog_Color=(bg<<4)|fg;
      Info.SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
      return TRUE;
    }
  }
  else if(Msg==DN_INITDIALOG)
  {
    Info.SendDlgMessage(hDlg,DM_SETMAXTEXTLENGTH,IAcceptChars,sizeof(AcceptChars)-1);
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void TAutoCompletion::InitItems(FarDialogItem *DialogItems)
{
  TCompletion::InitItems(DialogItems);
  int Msgs[]=
  {
    MAcceptFromMenu,
    MMinPreWordLen,MMinPreWordLen,
    MAcceptChars,MAcceptChars,
    MAcceptKey,MAcceptKey,
    MKeyCfg,
    MDeleteKey,MDeleteKey,
    MKeyCfg,
  };
  int DialogElements[][4]=
  {
    {DI_CHECKBOX,  3, 14,  0  }, // AcceptFromMenu
    {DI_TEXT,      6, 15,  0  }, //
    {DI_FIXEDIT,   3, 15,  4  }, // MinPreWordLen
    {DI_TEXT,      6, 16,  0  }, //
    {DI_EDIT,      3, 16,  4  }, // AcceptChars
    {DI_TEXT,      3, 18,  0  }, //
    {DI_FIXEDIT,   3, 19, 28  }, // AcceptKey
    {DI_BUTTON,   30, 19,  0  }, // AcceptKeyCfg
    {DI_TEXT,      3, 20,  0  }, //
    {DI_FIXEDIT,   3, 21, 28  }, // DeleteKey
    {DI_BUTTON,   30, 21,  0  }, // DeleteKeyCfg
  };

  for(unsigned int i=0;i<(sizeof(Msgs)/sizeof(Msgs[0]));i++)
  {
    DialogItems[i+CMPL_DIALOG_ITEMS].Type=DialogElements[i][0];
    DialogItems[i+CMPL_DIALOG_ITEMS].X1=DialogElements[i][1];
    DialogItems[i+CMPL_DIALOG_ITEMS].Y1=DialogElements[i][2];
    DialogItems[i+CMPL_DIALOG_ITEMS].X2=DialogElements[i][3];
    DialogItems[i+CMPL_DIALOG_ITEMS].Y2=0;
    DialogItems[i+CMPL_DIALOG_ITEMS].Selected=0;
    DialogItems[i+CMPL_DIALOG_ITEMS].Flags=0;
    INIT_DLG_DATA(DialogItems[i+CMPL_DIALOG_ITEMS],GetMsg(Msgs[i])); // Надписи на эл-тах диалога
  }

  Dialog_Color=Color;

  DialogItems[IAcceptFromMenu].Selected=AcceptFromMenu;

  // Что будет в строках ввода
  DLG_DATA_SPRINTF(DialogItems[IMinPreWordLen],MinPreWordLen,_T("%X"));
  INIT_DLG_DATA(DialogItems[IAcceptChars],AcceptChars);

  DialogItems[IAcceptChars].X2=DialogWidth()-_tcslen(GetMsg(MAcceptChars))-4;
  DialogItems[IAcceptCharsLabel].X1=DialogItems[IAcceptChars].X2+2;

  DLG_DATA_FARKEYTONAME(DialogItems[IAcceptKey],AcceptKey);
  DialogItems[IAcceptKey].X2=DialogWidth()-_tcslen(GetMsg(MKeyCfg))-9;
  DialogItems[IAcceptKeyCfg].X1=DialogWidth()-_tcslen(GetMsg(MKeyCfg))-7;
  DialogItems[IAcceptKeyCfg].Flags=DIF_NOBRACKETS;
  FSF.sprintf(AcceptKeyCfgText,_T("&[ %s ]"),GetMsg(MKeyCfg));
  DialogItems[IAcceptKeyCfg].PtrData=AcceptKeyCfgText;

  DLG_DATA_FARKEYTONAME(DialogItems[IDeleteKey],DeleteKey);
  DialogItems[IDeleteKey].X2=DialogWidth()-_tcslen(GetMsg(MKeyCfg))-9;
  DialogItems[IDeleteKeyCfg].X1=DialogWidth()-_tcslen(GetMsg(MKeyCfg))-7;
  DialogItems[IDeleteKeyCfg].Flags=DIF_NOBRACKETS;
  FSF.sprintf(DeleteKeyCfgText,_T("[ %s &]"),GetMsg(MKeyCfg));
  DialogItems[IDeleteKeyCfg].PtrData=DeleteKeyCfgText;

  INIT_DLG_DATA(DialogItems[ICfg],GetMsg(MAutoCfg)); // Заголовок
  INIT_DLG_DATA(DialogItems[IAdditional],GetMsg(MColor));
  DialogItems[IAdditional].Flags=DIF_CENTERGROUP;
  DialogItems[IAdditional].Y1=DialogHeight()-2;
}

void TAutoCompletion::StoreItems(CFarDialog& Dialog)
{
  TCompletion::StoreItems(Dialog);
  AcceptFromMenu=Dialog.Check(IAcceptFromMenu);
  MinPreWordLen=FSF.atoi(Dialog.Str(IMinPreWordLen));
  Color=Dialog_Color;
  AcceptKey=FSF.FarNameToKey(Dialog.Str(IAcceptKey));
  DeleteKey=FSF.FarNameToKey(Dialog.Str(IDeleteKey));
  _tcscpy(AcceptChars,Dialog.Str(IAcceptChars));
}
