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

static DWORD NormalizeControlState(DWORD State)
{
  DWORD Result=0;
  if(State&(RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED)) Result|=LEFT_ALT_PRESSED;
  if(State&(RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED)) Result|=LEFT_CTRL_PRESSED;
  if(State&SHIFT_PRESSED) Result|=SHIFT_PRESSED;
  if(State&ENHANCED_KEY) Result|=ENHANCED_KEY;
  return Result;
}

static bool CompareKeys(const INPUT_RECORD* One,const INPUT_RECORD* Two)
{
  DWORD oneControl=NormalizeControlState(One->Event.KeyEvent.dwControlKeyState),twoControl=NormalizeControlState(Two->Event.KeyEvent.dwControlKeyState);
  return (One->EventType==KEY_EVENT&&Two->EventType==KEY_EVENT&&One->Event.KeyEvent.bKeyDown==Two->Event.KeyEvent.bKeyDown&&One->Event.KeyEvent.wVirtualKeyCode==Two->Event.KeyEvent.wVirtualKeyCode&&One->Event.KeyEvent.uChar.UnicodeChar==Two->Event.KeyEvent.uChar.UnicodeChar&&oneControl==twoControl);
}

TAutoCompletion::TAutoCompletion()
{
  WorkInsideWord=false;
  CaseSensitive=true;
  ConsiderDigitAsChar=false;
  MinPreWordLen=2;
  WordsToFindCnt=3;
  PartialCompletion=true;

  AcceptFromMenu=false;
  memset(&AcceptKey,0,sizeof(AcceptKey));
  memset(&DeleteKey,0,sizeof(DeleteKey));
  HighliteColor.Flags=FCF_FG_4BIT|FCF_BG_4BIT;
  HighliteColor.ForegroundColor=0xf;
  HighliteColor.BackgroundColor=0x2;
  HighliteColor.Reserved=NULL;
  AcceptChars[0]=0;
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
          //int FarKey=FSF.FarInputRecordToKey(Rec);
          bool accept=CompareKeys(Rec,&AcceptKey);
          if(accept||(Rec->Event.KeyEvent.uChar.UnicodeChar&&_tcschr(AcceptChars,Rec->Event.KeyEvent.uChar.UnicodeChar)))
          {
            IgnoreThisEvent=AcceptVariant(Window)&&accept;
          }
          else if(Rec->Event.KeyEvent.wVirtualKeyCode!=VK_CONTROL&&Rec->Event.KeyEvent.wVirtualKeyCode!=VK_SHIFT&&Rec->Event.KeyEvent.wVirtualKeyCode!=VK_MENU)
          {
            DeleteVariant(Window);
            IgnoreThisEvent=CompareKeys(Rec,&DeleteKey);
          }
        }
        UTCHAR c=Rec->Event.KeyEvent.uChar.UnicodeChar;
        DWORD control=Rec->Event.KeyEvent.dwControlKeyState&(LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_ALT_PRESSED|RIGHT_CTRL_PRESSED);
        if(IsAlpha(c)&&(control==0||control==(LEFT_CTRL_PRESSED|RIGHT_ALT_PRESSED)))
        {
          Window->On=true;
          EditorInfo ei={sizeof(ei)};
          if(Info.EditorControl(-1,ECTL_GETINFO,0,&ei)&&ei.BlockType==BTYPE_STREAM&&(ei.Options&EOPT_PERSISTENTBLOCKS)==0)
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

int TAutoCompletion::ProcessEditorEvent(int Event,void *Param,int EditorID)
{
  if(Event==EE_CLOSE)
  {
    avl_window_data Add(EditorID);
    windows->remove(&Add);
  }
  else if(Event==EE_READ)
  {
    EditorInfo ei={sizeof(ei)};
    if(Info.EditorControl(-1,ECTL_GETINFO,0,&ei))
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
            if((INT_PTR)Param==1) Window->BlockDeleted=false;
          }
          else
          {
            Window->On=false;
            Info.EditorControl(-1,ECTL_REDRAW,0,0);
            PutVariant(Window);
            Info.EditorControl(-1,ECTL_REDRAW,0,0);
          }
        }
        else if(Window->Active) Colorize(HighliteColor,Window);
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
  EditorGetString gs={sizeof(gs)};
  gs.StringNumber=-1; // current string
  Info.EditorControl(-1,ECTL_GETSTRING,0,&gs);
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
    EditorInfo ei={sizeof(ei)};
    Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
    DeColorize(Window);
    Window->Active=false;
    {
      process=CheckText(Window->col,Window->row,Window);
      if(!process) process=CheckText(ei.CurPos,ei.CurLine,Window);
    }
    if(process)
    {
      if(!ei.Overtype)
        for(size_t i=0;i<(Window->AddedLen+Window->OldLen);i++) Info.EditorControl(-1,ECTL_DELETECHAR,0,NULL);
      Info.EditorControl(-1,ECTL_INSERTTEXT,0,Window->Rewrited.get());
    }
    SetCurPos(ei.CurPos,ei.CurLine);
    Window->Rewrited.clear();
    Window->Inserted.clear();
    Window->AddedLen=0;
    Window->OldLen=0;
    Info.EditorControl(-1,ECTL_REDRAW,0,0);
  }
}

bool TAutoCompletion::AcceptVariant(avl_window_data *Window)
{
  bool Accepted=false;
  if(Window->Active)
  {
    DeColorize(Window);
    Window->Active=false;
    Window->Rewrited.clear();
    SetCurPos(Window->col+Window->AddedLen,Window->row);
    if(PartialCompletion) Window->On=true;
    Info.EditorControl(-1,ECTL_REDRAW,0,NULL);
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
          while(poll)
          {
            DWORD wfmo=WaitForMultipleObjects(sizeof(handles)/sizeof(handles[0]),handles,FALSE,INFINITE);
            switch(wfmo)
            {
              case WAIT_OBJECT_0:
                poll=false;
                break;
              case WAIT_OBJECT_0+1:
                InterlockedIncrement((LONG *)&Stop);
                WaitForSingleObject(handles[0],INFINITE);
                poll=false;
                break;
            }
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

void TAutoCompletion::Colorize(FarColor NewColor,avl_window_data *Window)
{
  EditorColor ec;
  ec.StructSize=sizeof(ec);
  ec.StringNumber=Window->row;
  ec.StartPos=Window->col;
  ec.EndPos=Window->col+Window->AddedLen-1;
  ec.Color=NewColor;
  ec.Owner=MainGuid;
  ec.Priority=100;
  Info.EditorControl(-1,ECTL_ADDCOLOR,0,&ec);
}

void TAutoCompletion::DeColorize(avl_window_data *Window)
{
  EditorDeleteColor edc;
  edc.StructSize=sizeof(edc);
  edc.Owner=MainGuid;
  edc.StringNumber=Window->row;
  edc.StartPos=-1;
  Info.EditorControl(-1,ECTL_DELCOLOR,0,&edc);
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

size_t TAutoCompletion::Root(HANDLE Handle)
{
  FarSettingsValue value={sizeof(FarSettingsValue),0,_T("Auto Completion")};
  return Info.SettingsControl(Handle,SCTL_CREATESUBKEY,0,&value);
}

void TAutoCompletion::GetOptions(void)
{
  TCompletion::GetOptions();
  FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
  if(Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
  {
    intptr_t root=Root(settings.Handle);
    GetValue(settings.Handle,root,_T("Color"),&HighliteColor,sizeof(HighliteColor));
    TCHAR Key[256];
    FSF.FarNameToInputRecord(GetValue(settings.Handle,root,_T("AcceptKey"),Key,ArraySize(Key))?Key:_T("CtrlEnd"),&AcceptKey);
    FSF.FarNameToInputRecord(GetValue(settings.Handle,root,_T("DeleteKey"),Key,ArraySize(Key))?Key:_T("Del"),&DeleteKey);
    AcceptFromMenu=GetValue(settings.Handle,root,_T("AcceptFromMenu"),AcceptFromMenu);
    GetValue(settings.Handle,root,_T("AcceptChars"),AcceptChars,ArraySize(AcceptChars));
    Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
  }
}

void TAutoCompletion::SetOptions(void)
{
  TCompletion::SetOptions();
  FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
  if(Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
  {
    intptr_t root=Root(settings.Handle);
    SetValue(settings.Handle,root,_T("Color"),&HighliteColor,sizeof(HighliteColor));
    SetValue(settings.Handle,root,_T("AcceptFromMenu"),AcceptFromMenu);
    SetValue(settings.Handle,root,_T("AcceptChars"),AcceptChars);
    {
      TCHAR Key[256];
      if(!FSF.FarInputRecordToName(&AcceptKey,Key,ArraySize(Key))) Key[0]=0;
      SetValue(settings.Handle,root,_T("AcceptKey"),Key);
      if(!FSF.FarInputRecordToName(&DeleteKey,Key,ArraySize(Key))) Key[0]=0;
      SetValue(settings.Handle,root,_T("DeleteKey"),Key);
    }
    Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
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

intptr_t WINAPI GetKey(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
  if(Msg==DN_INITDIALOG)
  {
    Info.SendDlgMessage(hDlg,DM_SETDLGDATA,0,Param2);
  }
  else if(Msg==DN_CONTROLINPUT)
  {
    const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
    if(record->EventType==KEY_EVENT)
    {
      TCHAR *KeyName=(TCHAR *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
      FSF.FarInputRecordToName(record,KeyName,256);
      Info.SendDlgMessage(hDlg,DM_CLOSE,-1,0);
    }
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

INT_PTR TAutoCompletion::DialogProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
  if(Msg==DN_BTNCLICK)
  {
    if(Param1==IAcceptKeyCfg||Param1==IDeleteKeyCfg)
    {
      Info.SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
      TCHAR KeyName[256]; //FIXME
      FarDialogItemData getdata={sizeof(FarDialogItemData),255,KeyName};
      Info.SendDlgMessage(hDlg,DM_GETTEXT,Param1,&getdata);
      FarDialogItem DialogFrame;
      INIT_DLG_DATA(DialogFrame,GetMsg(MPressDesiredKey));
      DialogFrame.Type=DI_DOUBLEBOX;
      DialogFrame.X1=1;
      DialogFrame.Y1=1;
      DialogFrame.X2=_tcslen(DLG_DATA(DialogFrame))+4;
      DialogFrame.Y2=3;
      DialogFrame.Selected=0;
      DialogFrame.Flags=DIF_BOXCOLOR;
      DialogFrame.History=NULL;
      DialogFrame.Mask=NULL;
      DialogFrame.UserData=0;
      DialogFrame.MaxLength=0;
      int width=_tcslen(DLG_DATA(DialogFrame))+6;
      CFarDialog dialog;
      dialog.Execute(MainGuid,ACmplGuid,-1,-1,width,5,ConfigHelpTopic,&DialogFrame,1,0,0,GetKey,KeyName);
      Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,(Param1==IAcceptKeyCfg)?IAcceptKey:IDeleteKey,KeyName);
      Info.SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
      return TRUE;
    }
    if(Param1==IAdditional)
    {
      Info.SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
      Info.ColorDialog(&MainGuid,CDF_NONE,&Dialog_Color);
      Info.SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
      return TRUE;
    }
  }
  else if(Msg==DN_INITDIALOG)
  {
    Info.SendDlgMessage(hDlg,DM_SETMAXTEXTLENGTH,IAcceptChars,(void*)(sizeof(AcceptChars)-1));
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
    DialogItems[i+CMPL_DIALOG_ITEMS].Type=(FARDIALOGITEMTYPES)DialogElements[i][0];
    DialogItems[i+CMPL_DIALOG_ITEMS].X1=DialogElements[i][1];
    DialogItems[i+CMPL_DIALOG_ITEMS].Y1=DialogElements[i][2];
    DialogItems[i+CMPL_DIALOG_ITEMS].X2=DialogElements[i][3];
    DialogItems[i+CMPL_DIALOG_ITEMS].Y2=0;
    DialogItems[i+CMPL_DIALOG_ITEMS].Selected=0;
    DialogItems[i+CMPL_DIALOG_ITEMS].Flags=0;
    INIT_DLG_DATA(DialogItems[i+CMPL_DIALOG_ITEMS],GetMsg(Msgs[i])); // Надписи на эл-тах диалога
  }

  Dialog_Color=HighliteColor;

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
  DialogItems[IAcceptKeyCfg].Data=AcceptKeyCfgText;

  DLG_DATA_FARKEYTONAME(DialogItems[IDeleteKey],DeleteKey);
  DialogItems[IDeleteKey].X2=DialogWidth()-_tcslen(GetMsg(MKeyCfg))-9;
  DialogItems[IDeleteKeyCfg].X1=DialogWidth()-_tcslen(GetMsg(MKeyCfg))-7;
  DialogItems[IDeleteKeyCfg].Flags=DIF_NOBRACKETS;
  FSF.sprintf(DeleteKeyCfgText,_T("[ %s &]"),GetMsg(MKeyCfg));
  DialogItems[IDeleteKeyCfg].Data=DeleteKeyCfgText;

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
  HighliteColor=Dialog_Color;
  FSF.FarNameToInputRecord(Dialog.Str(IAcceptKey),&AcceptKey);
  FSF.FarNameToInputRecord(Dialog.Str(IDeleteKey),&DeleteKey);
  _tcscpy(AcceptChars,Dialog.Str(IAcceptChars));
}
