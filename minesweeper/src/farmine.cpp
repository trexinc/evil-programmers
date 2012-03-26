#include <limits.h> //for ULONG_MAX
#include "plugin.hpp"
#include "farcolor.hpp"
#include "farmine.hpp"
#include "far_settings.hpp"
#include "bootstrap/fmversion.h"
#include <initguid.h>
#include "guid.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

Options Opt={TRUE,TRUE,FALSE,FALSE,FALSE,1000,100};

enum CHECK_OPEN_PLACE
{
  COP_NONE,
  COP_CHECK_STOP,
  COP_CHECK_ALL
};

const wchar_t* GetMsg(int MsgId)
{
  return(Info.GetMsg(&MainGuid,MsgId));
}

void WINAPI GetGlobalInfoW(struct GlobalInfo* Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=MAKEFARVERSION(VER_MAJOR,VER_MINOR,0,VER_BUILD,VS_ALPHA);
  Info->Guid=MainGuid;
  Info->Title=L"Mine Sweeper";
  Info->Description=L"Mine Sweeper";
  Info->Author=L"Vadim Yegorov";
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo* Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>(int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;
  srand(GetTickCount());
  CFarSettings settings(MainGuid);
  Opt.ShowInConfig=settings.Get(L"ShowInConfig",Opt.ShowInConfig);
  Opt.ShowInPanels=settings.Get(L"ShowInPanels",Opt.ShowInPanels);
  Opt.ShowInEditor=settings.Get(L"ShowInEditor",Opt.ShowInEditor);
  Opt.ShowInViewer=settings.Get(L"ShowInViewer",Opt.ShowInViewer);
  Opt.ShowInDialog=settings.Get(L"ShowInDialog",Opt.ShowInDialog);
  Opt.WonFreq=settings.Get(L"WonFreq",Opt.WonFreq);
  Opt.LoseFreq=settings.Get(L"LoseFreq",Opt.LoseFreq);
}

void WINAPI GetPluginInfoW(struct PluginInfo* Info)
{
  static const wchar_t* PluginMenuStrings[1];
  memset(Info,0,sizeof(*Info));
  Info->StructSize=sizeof(*Info);
  Info->Flags=0;
  if(!Opt.ShowInPanels) Info->Flags|=PF_DISABLEPANELS;
  if(Opt.ShowInEditor) Info->Flags|=PF_EDITOR;
  if(Opt.ShowInViewer) Info->Flags|=PF_VIEWER;
  if(Opt.ShowInDialog) Info->Flags|=PF_DIALOG;
  PluginMenuStrings[0]=GetMsg(mName);
  Info->PluginConfig.Guids=&MenuGuid;
  Info->PluginConfig.Strings=PluginMenuStrings;
  if(Opt.ShowInConfig) Info->PluginConfig.Count=1;
  Info->PluginMenu.Guids=&MenuGuid;
  Info->PluginMenu.Strings=PluginMenuStrings;
  Info->PluginMenu.Count=1;
}

#define STATE_CLOSE  0
#define STATE_OPEN   1
#define STATE_MARKED 2

#define FINISH_NO    0
#define FINISH_WON   1
#define FINISH_LOSE  2


#define FOREGROUND_WHITE FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY
#define FOREGROUND_LIGHT_GRAY FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED

#define FOREGROUND_GRAY FOREGROUND_INTENSITY
#define FOREGROUND_BLACK 0

#define GET_DATA_0(a) (a.UserData&0xff)
#define GET_DATA_1(a) ((a.UserData>>8)&0xff)
#define SET_DATA_0(a,b) a.UserData=((a.UserData&~0xff)|((b)&0xff))
#define SET_DATA_1(a,b) a.UserData=((a.UserData&~0xff00)|(((b)&0xff)<<8))

struct BoardParams
{
  int width,height,mines;
  int curr_row,curr_col;
  unsigned long start_time;
  unsigned long end_time;
  bool started;
  int finished;
  int curr_mines;
};

static int context_colors[]=
{
  FOREGROUND_INTENSITY,
  FOREGROUND_BLUE|FOREGROUND_INTENSITY,
  FOREGROUND_GREEN,
  FOREGROUND_RED|FOREGROUND_INTENSITY,
  FOREGROUND_BLUE,
  FOREGROUND_RED,
  FOREGROUND_BLUE|FOREGROUND_GREEN,
  0,
  FOREGROUND_INTENSITY,
  0,
};

static wchar_t GetChar(int context,int state,int* color)
{
  wchar_t res=L' ';
  *color=0;
  switch(state)
  {
    case STATE_CLOSE:
      res=0x25a0;
      *color=FOREGROUND_INTENSITY;
      break;
    case STATE_OPEN:
      if(context==9) res=L'*';
      else if(context==0) res=L' ';
      else res=(wchar_t)(L'0'+context);
      if(context_colors[context]>-1)
        *color=context_colors[context];
      break;
    case STATE_MARKED:
      res=0x221a;
      *color=FOREGROUND_INTENSITY|FOREGROUND_RED;
      break;
  }
  return res;
}

static int GetBG(int row,int col,int state)
{
  if(row%2==col%2)
    return (state==STATE_OPEN)?FOREGROUND_WHITE:FOREGROUND_BLUE;
  else
    return (state==STATE_OPEN)?FOREGROUND_LIGHT_GRAY:FOREGROUND_BLACK;
}

static void SubmitScore(BoardParams* DlgParams)
{
  __int64 new_time=(DlgParams->end_time-DlgParams->start_time)/1000,old_time=LLONG_MAX,Type,DataSize=0,Disposition;
  wchar_t ScoreKeyName[1024],PlayerKeyName[1024];
  FSF.sprintf(ScoreKeyName,L"Score_%d_%d_%d",DlgParams->width,DlgParams->height,DlgParams->mines);
  FSF.sprintf(PlayerKeyName,L"Player_%d_%d_%d",DlgParams->width,DlgParams->height,DlgParams->mines);
  CFarSettings settings(MainGuid);
  old_time=settings.Get(ScoreKeyName,old_time);
  if(new_time<old_time)
  {
    wchar_t PlayerName[512];
    if(Info.InputBox(&MainGuid,&PlayerGuid,GetMsg(mHighscore),NULL,NULL,NULL,PlayerName,sizeof(PlayerName),NULL,FIB_ENABLEEMPTY|FIB_BUTTONS))
    {
      settings.Set(ScoreKeyName,new_time);
      settings.Set(PlayerKeyName,PlayerName);
    }
  }
}

static void CheckStop(HANDLE hDlg,BoardParams *DlgParams,int row,int col)
{
  if(DlgParams->finished!=FINISH_NO) return;
  FarDialogItem DialogItem;
  Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,DlgParams->width*col+row,&DialogItem);
  if(GET_DATA_1(DialogItem)==STATE_OPEN&&GET_DATA_0(DialogItem)==9)
  {
    Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,DlgParams->width*DlgParams->height,(void*)GetMsg(mLose));
    DlgParams->finished=FINISH_LOSE;
    DlgParams->end_time=GetTickCount();
    Beep((long)Opt.LoseFreq,100);
    Info.SendDlgMessage(hDlg,DM_SHOWTIME,0,NULL);
  }
  else
  {
    FarDialogItem DialogItem; bool won=true;
    for(int i=0;i<DlgParams->height;i++)
    {
      for(int j=0;j<DlgParams->width;j++)
      {
        int k=i*DlgParams->width+j;
        Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,k,&DialogItem);
        if(GET_DATA_1(DialogItem)==STATE_CLOSE&&GET_DATA_0(DialogItem)<9) //FIXME
        {
          won=false;
          break;
        }
        else if(GET_DATA_1(DialogItem)==STATE_MARKED&&GET_DATA_0(DialogItem)<9)
        {
          won=false;
          break;
        }
        else if(GET_DATA_1(DialogItem)==STATE_OPEN&&GET_DATA_0(DialogItem)==9)
        {
          won=false;
          break;
        }
      }
      if(!won) break;
    }
    if(won)
    {
      Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,DlgParams->width*DlgParams->height,(void*)GetMsg(mWon));
      DlgParams->finished=FINISH_WON;
      DlgParams->end_time=GetTickCount();
      DlgParams->curr_mines=DlgParams->mines;
      Beep((long)Opt.WonFreq,100);
      int color;
      for(int i=0;i<DlgParams->height;i++)
        for(int j=0;j<DlgParams->width;j++)
        {
          int k=i*DlgParams->width+j;
          Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,k,&DialogItem);
          if(GET_DATA_1(DialogItem)==STATE_CLOSE)
          {
            SET_DATA_1(DialogItem,STATE_MARKED);
            DialogItem.VBuf[1].Char=GetChar(GET_DATA_0(DialogItem),GET_DATA_1(DialogItem),&color);
            int bgcolor=GetBG(i,j,GET_DATA_1(DialogItem));
            DialogItem.VBuf[0].Attributes.ForegroundColor=DialogItem.VBuf[1].Attributes.ForegroundColor=DialogItem.VBuf[2].Attributes.ForegroundColor=color;
            DialogItem.VBuf[0].Attributes.BackgroundColor=DialogItem.VBuf[1].Attributes.BackgroundColor=DialogItem.VBuf[2].Attributes.BackgroundColor=bgcolor;
            Info.SendDlgMessage(hDlg,DM_SETDLGITEMSHORT,k,&DialogItem);
          }
        }
      Info.SendDlgMessage(hDlg,DM_SHOWTIME,0,0);
      SubmitScore(DlgParams);
    }
  }
}

static int GetMine(int row,int col,HANDLE hDlg,BoardParams *DlgParams,bool flagged)
{
  if(row<0) return 0;
  if(col<0) return 0;
  if(row>=DlgParams->width) return 0;
  if(col>=DlgParams->height) return 0;
  FarDialogItem DialogItem;
  Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,col*DlgParams->width+row,&DialogItem);
  if(flagged)
  {
    if(GET_DATA_1(DialogItem)==STATE_MARKED) return 1;
  }
  else
  {
    if(GET_DATA_0(DialogItem)==9) return 1;
  }
  return 0;
}

static void OpenPlace(HANDLE hDlg,BoardParams *DlgParams,int row,int col,CHECK_OPEN_PLACE check)
{
  FarDialogItem DialogItem; int color;
  if(row<0) return;
  if(col<0) return;
  if(row>=DlgParams->width) return;
  if(col>=DlgParams->height) return;
  Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,DlgParams->width*col+row,&DialogItem);
  if(GET_DATA_1(DialogItem)==STATE_CLOSE)
  {
    SET_DATA_1(DialogItem,STATE_OPEN);
    DialogItem.VBuf[1].Char=GetChar(GET_DATA_0(DialogItem),GET_DATA_1(DialogItem),&color);
    int bgcolor=GetBG(row,col,GET_DATA_1(DialogItem));
    DialogItem.VBuf[0].Attributes.ForegroundColor=DialogItem.VBuf[1].Attributes.ForegroundColor=DialogItem.VBuf[2].Attributes.ForegroundColor=color;
    DialogItem.VBuf[0].Attributes.BackgroundColor=DialogItem.VBuf[1].Attributes.BackgroundColor=DialogItem.VBuf[2].Attributes.BackgroundColor=bgcolor;
    Info.SendDlgMessage(hDlg,DM_SETDLGITEMSHORT,DlgParams->width*col+row,&DialogItem);
    if(!GET_DATA_0(DialogItem))
    {
      OpenPlace(hDlg,DlgParams,row-1,col-1,COP_NONE);
      OpenPlace(hDlg,DlgParams,row-1,col,COP_NONE);
      OpenPlace(hDlg,DlgParams,row-1,col+1,COP_NONE);
      OpenPlace(hDlg,DlgParams,row,col-1,COP_NONE);
      OpenPlace(hDlg,DlgParams,row,col+1,COP_NONE);
      OpenPlace(hDlg,DlgParams,row+1,col-1,COP_NONE);
      OpenPlace(hDlg,DlgParams,row+1,col,COP_NONE);
      OpenPlace(hDlg,DlgParams,row+1,col+1,COP_NONE);
    }
    if(check!=COP_NONE) CheckStop(hDlg,DlgParams,row,col);
  }
  else if(check==COP_CHECK_ALL&&GET_DATA_1(DialogItem)==STATE_OPEN&&GET_DATA_0(DialogItem)>0&&GET_DATA_0(DialogItem)<9)
  {
    int mines=0;
    mines+=GetMine(row-1,col-1,hDlg,DlgParams,true);
    mines+=GetMine(row-1,col,hDlg,DlgParams,true);
    mines+=GetMine(row-1,col+1,hDlg,DlgParams,true);
    mines+=GetMine(row,col-1,hDlg,DlgParams,true);
    mines+=GetMine(row,col+1,hDlg,DlgParams,true);
    mines+=GetMine(row+1,col-1,hDlg,DlgParams,true);
    mines+=GetMine(row+1,col,hDlg,DlgParams,true);
    mines+=GetMine(row+1,col+1,hDlg,DlgParams,true);
    if(mines==GET_DATA_0(DialogItem)) //FIXME
    {
      OpenPlace(hDlg,DlgParams,row-1,col-1,COP_CHECK_STOP);
      OpenPlace(hDlg,DlgParams,row-1,col,COP_CHECK_STOP);
      OpenPlace(hDlg,DlgParams,row-1,col+1,COP_CHECK_STOP);
      OpenPlace(hDlg,DlgParams,row,col-1,COP_CHECK_STOP);
      OpenPlace(hDlg,DlgParams,row,col+1,COP_CHECK_STOP);
      OpenPlace(hDlg,DlgParams,row+1,col-1,COP_CHECK_STOP);
      OpenPlace(hDlg,DlgParams,row+1,col,COP_CHECK_STOP);
      OpenPlace(hDlg,DlgParams,row+1,col+1,COP_CHECK_STOP);

      CheckStop(hDlg,DlgParams,row,col);
    }
  }
}

static void RedrawPlace(HANDLE hDlg,BoardParams *DlgParams,bool focused)
{
  int color;
  FarDialogItem DialogItem;
  Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,DlgParams->width*DlgParams->curr_col+DlgParams->curr_row,&DialogItem);
  DialogItem.VBuf[1].Char=GetChar(GET_DATA_0(DialogItem),GET_DATA_1(DialogItem),&color);
  if(focused&&color==(FOREGROUND_RED|FOREGROUND_INTENSITY)) color=0;
  int bgcolor=focused?(FOREGROUND_RED|FOREGROUND_INTENSITY):(GetBG(DlgParams->curr_row,DlgParams->curr_col,GET_DATA_1(DialogItem)));
  DialogItem.VBuf[0].Attributes.ForegroundColor=DialogItem.VBuf[1].Attributes.ForegroundColor=DialogItem.VBuf[2].Attributes.ForegroundColor=color;
  DialogItem.VBuf[0].Attributes.BackgroundColor=DialogItem.VBuf[1].Attributes.BackgroundColor=DialogItem.VBuf[2].Attributes.BackgroundColor=bgcolor;
}

static void SetFocusUser(HANDLE hDlg,BoardParams *DlgParams,bool real)
{
  if(real)
    Info.SendDlgMessage(hDlg,DM_SETFOCUS,DlgParams->width*DlgParams->curr_col+DlgParams->curr_row,0);
  else
  {
    int curr_elem=(int)Info.SendDlgMessage(hDlg,DM_GETFOCUS,0,0);
    DlgParams->curr_row=curr_elem%DlgParams->width;
    DlgParams->curr_col=curr_elem/DlgParams->width;
  }
  RedrawPlace(hDlg,DlgParams,true);
}

#define SetFocus(real) SetFocusUser(hDlg,DlgParams,real)
#define KillFocus() RedrawPlace(hDlg,DlgParams,false)

static void RunKey(const wchar_t* Key)
{
  MacroSendMacroText macro;
  memset(&macro,0,sizeof(macro));
  macro.StructSize=sizeof(MacroSendMacroText);
  macro.SequenceText=Key;
  Info.MacroControl(&MainGuid,MCTL_SENDSTRING,MSSC_POST,&macro);
}

static INT_PTR WINAPI MainDialogProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
  BoardParams* DlgParams=(BoardParams*)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  //if(Msg==DN_KEY&&Param2==KEY_ENTER) return TRUE;
  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_RESETBOARD,0,0);
      SetFocus(true);
      break;
    case DN_CONTROLINPUT:
      {
        const INPUT_RECORD* record=(const INPUT_RECORD*)Param2;
        if(record->EventType==KEY_EVENT&&record->Event.KeyEvent.bKeyDown)
        {
          KillFocus();
          if(IsNone(record))
          {
            switch(record->Event.KeyEvent.wVirtualKeyCode)
            {
              case VK_UP:
                DlgParams->curr_col--;
                break;
              case VK_DOWN:
                DlgParams->curr_col++;
                break;
              case VK_LEFT:
                DlgParams->curr_row--;
                break;
              case VK_RIGHT:
                DlgParams->curr_row++;
                break;
              case VK_HOME:
                DlgParams->curr_row=0;
                break;
              case VK_END:
                DlgParams->curr_row=DlgParams->width-1;
                break;
              case VK_PRIOR:
                DlgParams->curr_col=0;
                break;
              case VK_NEXT:
                DlgParams->curr_col=DlgParams->height-1;
                break;
              case L' ':
                {
                  Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
                  if(!DlgParams->finished)
                  {
                    if(!DlgParams->started) Info.SendDlgMessage(hDlg,DM_START_GAME,0,0);
                    OpenPlace(hDlg,DlgParams,DlgParams->curr_row,DlgParams->curr_col,COP_CHECK_ALL);
                  }
                  Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
                }
                break;
              case VK_DELETE:
                {
                  Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
                  if(!DlgParams->finished)
                  {
                    FarDialogItem DialogItem;
                    Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,DlgParams->width*DlgParams->curr_col+DlgParams->curr_row,&DialogItem);
                    switch(GET_DATA_1(DialogItem))
                    {
                      case STATE_CLOSE:
                        SET_DATA_1(DialogItem,STATE_MARKED);
                        DlgParams->curr_mines++;
                        break;
                      case STATE_OPEN:
                        break;
                      case STATE_MARKED:
                        SET_DATA_1(DialogItem,STATE_CLOSE);
                        DlgParams->curr_mines--;
                        break;
                    }
                    Info.SendDlgMessage(hDlg,DM_SETDLGITEMSHORT,DlgParams->width*DlgParams->curr_col+DlgParams->curr_row,&DialogItem);
                    Info.SendDlgMessage(hDlg,DM_SHOWTIME,0,0);
                  }
                  Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
                }
                break;
              case VK_F2:
                DlgParams->started=false;
                DlgParams->finished=FINISH_NO;
                Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,DlgParams->width*DlgParams->height,(void*)GetMsg(mStart));
                Info.SendDlgMessage(hDlg,DM_RESETBOARD,0,0);
                break;
              case VK_F3:
                {
                  wchar_t ScoreKeyName[1024],PlayerKeyName[1024];
                  FSF.sprintf(ScoreKeyName,L"Score_%d_%d_%d",DlgParams->width,DlgParams->height,DlgParams->mines);
                  FSF.sprintf(PlayerKeyName,L"Player_%d_%d_%d",DlgParams->width,DlgParams->height,DlgParams->mines);
                  CFarSettings settings(MainGuid);
                  __int64 Score; wchar_t Name[512];
                  Score=settings.Get(ScoreKeyName,-1);
                  if(!settings.Get(PlayerKeyName,Name,sizeofa(Name))) Name[0]=0;
                  if(Score>=0&&Name[0])
                  {
                    wchar_t buffer[1024];
                    FSF.sprintf(buffer,GetMsg(mHighscoreFormat),Name,(long)Score);
                    const wchar_t* MsgItems[]={GetMsg(mHighscoreTitle),buffer,GetMsg(mOk)};
                    Info.Message(&MainGuid,&HiscoreGuid,0,NULL,MsgItems,sizeofa(MsgItems),1);
                  }
                }
                break;
            }
          }
          if(DlgParams->curr_row>=DlgParams->width) DlgParams->curr_row=0;
          if(DlgParams->curr_row<0) DlgParams->curr_row=DlgParams->width-1;
          if(DlgParams->curr_col>=DlgParams->height) DlgParams->curr_col=0;
          if(DlgParams->curr_col<0) DlgParams->curr_col=DlgParams->height-1;
          SetFocus(true);
          Info.SendDlgMessage(hDlg,DM_REDRAW,0,0);
        }
        else if(record->EventType==MOUSE_EVENT)
        {
          DWORD Buttons=record->Event.MouseEvent.dwButtonState;
          if(Buttons&FROM_LEFT_1ST_BUTTON_PRESSED) RunKey(L"Space");
          else if(Buttons&RIGHTMOST_BUTTON_PRESSED) RunKey(L"Del");
        }
      }
      break;
    case DN_GOTFOCUS:
      SetFocus(false);
      break;
    case DN_KILLFOCUS:
      KillFocus();
      break;
    case DM_RESETBOARD:
      {
        int curr_mines=DlgParams->mines,color;
        FarDialogItem DialogItem;
        Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
        for(int i=0;i<DlgParams->height;i++)
          for(int j=0;j<DlgParams->width;j++)
          {
            int k=i*DlgParams->width+j;
            Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,k,&DialogItem);
            SET_DATA_0(DialogItem,0);
            SET_DATA_1(DialogItem,0);
            color=0;
            DialogItem.VBuf[1].Char=GetChar(GET_DATA_0(DialogItem),GET_DATA_1(DialogItem),&color);
            int bgcolor=GetBG(i,j,STATE_CLOSE);
            DialogItem.VBuf[0].Attributes.ForegroundColor=DialogItem.VBuf[1].Attributes.ForegroundColor=DialogItem.VBuf[2].Attributes.ForegroundColor=color;
            DialogItem.VBuf[0].Attributes.BackgroundColor=DialogItem.VBuf[1].Attributes.BackgroundColor=DialogItem.VBuf[2].Attributes.BackgroundColor=bgcolor;
            Info.SendDlgMessage(hDlg,DM_SETDLGITEMSHORT,k,&DialogItem);
          }
        //set mines
        while(curr_mines)
          for(int i=0;i<DlgParams->height;i++)
            for(int j=0;j<DlgParams->width;j++)
            {
              int k=i*DlgParams->width+j;
              Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,k,&DialogItem);
              if(curr_mines&&(!GET_DATA_0(DialogItem)))
              {
                if(rand()<=(RAND_MAX*DlgParams->mines/(DlgParams->width*DlgParams->height)))
                {
                  curr_mines--;
                  SET_DATA_0(DialogItem,9);
                  Info.SendDlgMessage(hDlg,DM_SETDLGITEMSHORT,k,&DialogItem);
                }
              }
            }
        //set places
        for(int i=0;i<DlgParams->height;i++)
          for(int j=0;j<DlgParams->width;j++)
          {
            int k=i*DlgParams->width+j;
            Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,k,&DialogItem);
            if(GET_DATA_0(DialogItem)==0)
            {
              SET_DATA_0(DialogItem,GET_DATA_0(DialogItem)+GetMine(j-1,i-1,hDlg,DlgParams,false));
              SET_DATA_0(DialogItem,GET_DATA_0(DialogItem)+GetMine(j-1,i,hDlg,DlgParams,false));
              SET_DATA_0(DialogItem,GET_DATA_0(DialogItem)+GetMine(j-1,i+1,hDlg,DlgParams,false));
              SET_DATA_0(DialogItem,GET_DATA_0(DialogItem)+GetMine(j,i-1,hDlg,DlgParams,false));
              SET_DATA_0(DialogItem,GET_DATA_0(DialogItem)+GetMine(j,i+1,hDlg,DlgParams,false));
              SET_DATA_0(DialogItem,GET_DATA_0(DialogItem)+GetMine(j+1,i-1,hDlg,DlgParams,false));
              SET_DATA_0(DialogItem,GET_DATA_0(DialogItem)+GetMine(j+1,i,hDlg,DlgParams,false));
              SET_DATA_0(DialogItem,GET_DATA_0(DialogItem)+GetMine(j+1,i+1,hDlg,DlgParams,false));
              Info.SendDlgMessage(hDlg,DM_SETDLGITEMSHORT,k,&DialogItem);
            }
          }
        DlgParams->curr_mines=0;
        Info.SendDlgMessage(hDlg,DM_SHOWTIME,0,0);
        Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
      }
      break;
    case DN_ENTERIDLE:
      Info.SendDlgMessage(hDlg,DM_SHOWTIME,0,0);
      break;
    case DM_SHOWTIME:
      {
        wchar_t buffer[1024]; DWORD diff_time=DlgParams->started?((DlgParams->finished?DlgParams->end_time:GetTickCount())-DlgParams->start_time)/1000:0;
        FSF.sprintf(buffer,GetMsg(mTime),diff_time,DlgParams->curr_mines);
        Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,DlgParams->width*DlgParams->height+1,buffer);
      }
      break;
    case DM_START_GAME:
      Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,DlgParams->width*DlgParams->height,(void*)GetMsg(mGame));
      DlgParams->started=true;
      DlgParams->start_time=GetTickCount();
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

static int widths[]={9,16,31};
static int heights[]={9,16,16};
static int mines[]={10,40,99};
static int screen_widths[]={29,50,95};
static int screen_heights[]={14,21,21};

HANDLE WINAPI OpenW(const struct OpenInfo* Info)
{
  int screen_width=80,screen_height=25;
  HANDLE console=CreateFile(L"CONOUT$",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  if(console!=INVALID_HANDLE_VALUE)
  {
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    if(GetConsoleScreenBufferInfo(console,&console_info))
    {
      screen_width=console_info.dwSize.X;
      screen_height=console_info.dwSize.Y;
    }
    CloseHandle(console);
  }
  FarMenuItem MenuItems[3];
  memset(MenuItems,0,sizeof(MenuItems));
  for(unsigned int i=0;i<sizeofa(MenuItems);i++)
  {
    MenuItems[i].Flags=MIF_DISABLE;
    if(screen_widths[i]<=screen_width&&screen_heights[i]<=screen_height) MenuItems[i].Flags=0;
    MenuItems[i].Text=GetMsg(mLev1+i);
  }
  MenuItems[0].Flags|=MIF_SELECTED;
  int MenuCode=::Info.Menu(&MainGuid,&MainMenuGuid,-1,-1,0,FMENU_WRAPMODE,GetMsg(mName),NULL,L"Contents",NULL,NULL,MenuItems,sizeofa(MenuItems));
  if(MenuCode>=0)
  {
    BoardParams bp; int curr_color;
    bp.width=widths[MenuCode]; bp.height=heights[MenuCode]; bp.mines=mines[MenuCode];
    bp.curr_row=0; bp.curr_col=0;
    bp.started=false; bp.finished=FINISH_NO;
    FarDialogItem* DialogItems=(FarDialogItem*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(bp.width*bp.height+4)*sizeof(FarDialogItem));
    FAR_CHAR_INFO* VirtualBuffer=(FAR_CHAR_INFO*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,bp.width*bp.height*sizeof(FAR_CHAR_INFO)*3);
    wchar_t* Separator=(wchar_t*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(bp.width*3+1)*sizeof(wchar_t));
    if(DialogItems&&VirtualBuffer&&Separator)
    {
      for(int i=0;i<bp.height;i++)
        for(int j=0;j<bp.width;j++)
        {
          int k=i*bp.width+j;
          DialogItems[k].Type=DI_USERCONTROL;
          DialogItems[k].X1=1+j*3;
          DialogItems[k].X2=1+j*3+2;
          DialogItems[k].Y1=1+i;
          DialogItems[k].Y2=1+i;
          DialogItems[k].VBuf=VirtualBuffer+k*3;
          DialogItems[k].VBuf[0].Char=L' ';
          DialogItems[k].VBuf[1].Char=0x25a0;
          DialogItems[k].VBuf[2].Char=L' ';
          curr_color=GetBG(j,i,STATE_CLOSE);
          for(int l=0;l<3;l++)
          {
            DialogItems[k].VBuf[l].Attributes.Flags=FCF_4BITMASK;
            DialogItems[k].VBuf[l].Attributes.BackgroundColor=curr_color;
            DialogItems[k].VBuf[l].Attributes.ForegroundColor=FOREGROUND_INTENSITY;
          }
        }
      //Status
      {
        int k=bp.width*bp.height;
        DialogItems[k].Type=DI_TEXT;
        DialogItems[k].X1=1;
        DialogItems[k].X2=0;
        DialogItems[k].Y1=bp.height+2;
        DialogItems[k].Y2=0;
        DialogItems[k].Data=GetMsg(mStart);
      }
      //time
      {
        int k=bp.width*bp.height+1;
        DialogItems[k].Type=DI_TEXT;
        DialogItems[k].X1=1;
        DialogItems[k].X2=0;
        DialogItems[k].Y1=bp.height+3;
        DialogItems[k].Y2=0;
      }
      //delimiter
      {
        int k=bp.width*bp.height+2;
        DialogItems[k].Type=DI_TEXT;
        DialogItems[k].X1=1;
        DialogItems[k].X2=0;
        DialogItems[k].Y1=bp.height+1;
        DialogItems[k].Y2=0;
        DialogItems[k].Data=Separator;
        for(int ii=0;ii<bp.width*3;++ii) Separator[ii]=0x2500;
      }
      //border
      {
        int k=bp.width*bp.height+3;
        DialogItems[k].Type=DI_DOUBLEBOX;
        DialogItems[k].X1=0;
        DialogItems[k].X2=bp.width*3+1;
        DialogItems[k].Y1=0;
        DialogItems[k].Y2=bp.height+4;
        DialogItems[k].Data=GetMsg(mName);
      }
      HANDLE dlg=::Info.DialogInit(&MainGuid,&MainDialogGuid,-1,-1,bp.width*3+2,bp.height+5,L"Contents",DialogItems,bp.width*bp.height+4,0,0,MainDialogProc,&bp);
      if(dlg!=INVALID_HANDLE_VALUE)
      {
        ::Info.DialogRun(dlg);
        ::Info.DialogFree(dlg);
      }
    }
    if(DialogItems) HeapFree(GetProcessHeap(),0,DialogItems);
    if(VirtualBuffer) HeapFree(GetProcessHeap(),0,VirtualBuffer);
    if(Separator) HeapFree(GetProcessHeap(),0,Separator);
  }
  return NULL;
}

int WINAPI ConfigureW(const struct ConfigureInfo* anInfo)
{
  return(Config());
}

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void)hDll;
  (void)dwReason;
  (void)lpReserved;
  return TRUE;
}
