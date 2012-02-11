#include <limits.h> //for ULONG_MAX
#include "../../plugin.hpp"
#include "../../farcolor.hpp"
#include "../../farkeys.hpp"
#include "farmine.hpp"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
char PluginRootKey[80];
bool IsOldFAR=true;

Options Opt={TRUE,TRUE,FALSE,FALSE,FALSE,1000,100};

enum CHECK_OPEN_PLACE
{
  COP_NONE,
  COP_CHECK_STOP,
  COP_CHECK_ALL
};

const char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

void WINAPI SetStartupInfo(const struct PluginStartupInfo *Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>(int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
  if(Info->StructSize>FAR165_INFO_SIZE)
  {
    IsOldFAR=false;
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;
    strcpy(PluginRootKey,Info->RootKey);
    strcat(PluginRootKey,"\\FarMine");
    srand(GetTickCount());
    HKEY hKey;
    if((RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
    {
      DWORD Type,DataSize=sizeof(Opt.ShowInConfig);
      RegQueryValueEx(hKey,"ShowInConfig",0,&Type,(LPBYTE)&Opt.ShowInConfig,&DataSize);
      DataSize=sizeof(Opt.ShowInPanels);
      RegQueryValueEx(hKey,"ShowInPanels",0,&Type,(LPBYTE)&Opt.ShowInPanels,&DataSize);
      DataSize=sizeof(Opt.ShowInEditor);
      RegQueryValueEx(hKey,"ShowInEditor",0,&Type,(LPBYTE)&Opt.ShowInEditor,&DataSize);
      DataSize=sizeof(Opt.ShowInViewer);
      RegQueryValueEx(hKey,"ShowInViewer",0,&Type,(LPBYTE)&Opt.ShowInViewer,&DataSize);
      DataSize=sizeof(Opt.Preload);
      RegQueryValueEx(hKey,"Preload",0,&Type,(LPBYTE)&Opt.Preload,&DataSize);
      DataSize=sizeof(Opt.WonFreq);
      RegQueryValueEx(hKey,"WonFreq",0,&Type,(LPBYTE)&Opt.WonFreq,&DataSize);
      DataSize=sizeof(Opt.LoseFreq);
      RegQueryValueEx(hKey,"LoseFreq",0,&Type,(LPBYTE)&Opt.LoseFreq,&DataSize);
      RegCloseKey(hKey);
    }
  }
}

void WINAPI GetPluginInfo(struct PluginInfo *Info)
{
  if(!IsOldFAR)
  {
    static const char *PluginMenuStrings;
    memset(Info,0,sizeof(*Info));
    Info->Flags=0;
    if(!Opt.ShowInPanels) Info->Flags|=PF_DISABLEPANELS;
    if(Opt.ShowInEditor) Info->Flags|=PF_EDITOR;
    if(Opt.ShowInViewer) Info->Flags|=PF_VIEWER;
    if(Opt.Preload) Info->Flags|=PF_PRELOAD;
    Info->StructSize=sizeof(*Info);
    Info->PluginMenuStringsNumber=1;
    if(Opt.ShowInConfig) Info->PluginConfigStringsNumber=1;
    PluginMenuStrings=GetMsg(mName);
    Info->PluginConfigStrings=&PluginMenuStrings;
    Info->PluginMenuStrings=&PluginMenuStrings;
  }
}

int WINAPI _export GetMinFarVersion(void)
{
  return MAKEFARVERSION(1,70,1282);
}

#define STATE_CLOSE  0
#define STATE_OPEN   1
#define STATE_MARKED 2

#define FINISH_NO    0
#define FINISH_WON   1
#define FINISH_LOSE  2


#define FOREGROUND_WHITE FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY
#define FOREGROUND_LIGHT_GRAY FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED
#define BACKGROUND_WHITE BACKGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_INTENSITY
#define BACKGROUND_LIGHT_GRAY BACKGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_RED

#define BACKGROUND_GRAY BACKGROUND_INTENSITY
#define BACKGROUND_BLACK 0


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

static int GetChar(int context,int state,int *color)
{
  int res=' ';
  *color=0;
  switch(state)
  {
    case STATE_CLOSE:
      res='þ';
      *color=FOREGROUND_INTENSITY;
      break;
    case STATE_OPEN:
      if(context==9) res='*';
      else if(context==0) res=' ';
      else res='0'+context;
      if(context_colors[context]>-1)
        *color=context_colors[context];
      break;
    case STATE_MARKED:
      res='û';
      *color=FOREGROUND_INTENSITY|FOREGROUND_RED;
      break;
  }
  return res;
}

static int GetBG(int row,int col,int state)
{
  if(row%2==col%2)
    return (state==STATE_OPEN)?BACKGROUND_WHITE:BACKGROUND_BLUE;
  else
    return (state==STATE_OPEN)?BACKGROUND_LIGHT_GRAY:BACKGROUND_BLACK;
}

static void SubmitScore(BoardParams *DlgParams)
{
  DWORD new_time=(DlgParams->end_time-DlgParams->start_time)/1000,old_time=ULONG_MAX,Type,DataSize=0,Disposition;
  HKEY hKey;
  char ScoreKeyName[1024],PlayerKeyName[1024];
  FSF.sprintf(ScoreKeyName,"Score_%d_%d_%d",DlgParams->width,DlgParams->height,DlgParams->mines);
  FSF.sprintf(PlayerKeyName,"Player_%d_%d_%d",DlgParams->width,DlgParams->height,DlgParams->mines);
  if(RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_QUERY_VALUE|KEY_WRITE,NULL,&hKey,&Disposition)==ERROR_SUCCESS)
  {
    DataSize=sizeof(old_time);
    RegQueryValueEx(hKey,ScoreKeyName,0,&Type,(LPBYTE)&old_time,&DataSize);
    if(new_time<old_time)
    {
      char PlayerName[512];
      if(Info.InputBox(NULL,GetMsg(mHighscore),NULL,"",PlayerName,sizeof(PlayerName),NULL,FIB_ENABLEEMPTY|FIB_BUTTONS))
      {
        RegSetValueEx(hKey,ScoreKeyName,0,REG_DWORD,(LPBYTE)&new_time,sizeof(new_time));
        RegSetValueEx(hKey,PlayerKeyName,0,REG_SZ,(LPBYTE)PlayerName,strlen(PlayerName)+1);
      }
    }
    RegCloseKey(hKey);
  }
}

static void CheckStop(HANDLE hDlg,BoardParams *DlgParams,int row,int col)
{
  if(DlgParams->finished!=FINISH_NO) return;
  FarDialogItem DialogItem;
  Info.SendDlgMessage(hDlg,DM_GETDLGITEM,DlgParams->width*col+row,(long)&DialogItem);
  if(DialogItem.Data[1]==STATE_OPEN&&DialogItem.Data[0]==9)
  {
    Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,DlgParams->width*DlgParams->height,(long)GetMsg(mLose));
    DlgParams->finished=FINISH_LOSE;
    DlgParams->end_time=GetTickCount();
    Beep(Opt.LoseFreq,100);
    Info.SendDlgMessage(hDlg,DM_SHOWTIME,0,0);
  }
  else
  {
    FarDialogItem DialogItem; bool won=true;
    for(int i=0;i<DlgParams->height;i++)
    {
      for(int j=0;j<DlgParams->width;j++)
      {
        int k=i*DlgParams->width+j;
        Info.SendDlgMessage(hDlg,DM_GETDLGITEM,k,(long)&DialogItem);
        if(DialogItem.Data[1]==STATE_CLOSE&&DialogItem.Data[0]<9) //FIXME
        {
          won=false;
          break;
        }
        else if(DialogItem.Data[1]==STATE_MARKED&&DialogItem.Data[0]<9)
        {
          won=false;
          break;
        }
        else if(DialogItem.Data[1]==STATE_OPEN&&DialogItem.Data[0]==9)
        {
          won=false;
          break;
        }
      }
      if(!won) break;
    }
    if(won)
    {
      Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,DlgParams->width*DlgParams->height,(long)GetMsg(mWon));
      DlgParams->finished=FINISH_WON;
      DlgParams->end_time=GetTickCount();
      DlgParams->curr_mines=DlgParams->mines;
      Beep(Opt.WonFreq,100);
      int color;
      for(int i=0;i<DlgParams->height;i++)
        for(int j=0;j<DlgParams->width;j++)
        {
          int k=i*DlgParams->width+j;
          Info.SendDlgMessage(hDlg,DM_GETDLGITEM,k,(long)&DialogItem);
          if(DialogItem.Data[1]==STATE_CLOSE)
          {
            DialogItem.Data[1]=STATE_MARKED;
            DialogItem.VBuf[1].Char.AsciiChar=GetChar(DialogItem.Data[0],DialogItem.Data[1],&color);
            color|=GetBG(i,j,DialogItem.Data[1]);
            DialogItem.VBuf[0].Attributes=DialogItem.VBuf[1].Attributes=DialogItem.VBuf[2].Attributes=color;
            Info.SendDlgMessage(hDlg,DM_SETDLGITEM,k,(long)&DialogItem);
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
  Info.SendDlgMessage(hDlg,DM_GETDLGITEM,col*DlgParams->width+row,(long)&DialogItem);
  if(flagged)
  {
    if(DialogItem.Data[1]==STATE_MARKED) return 1;
  }
  else
  {
    if(DialogItem.Data[0]==9) return 1;
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
  Info.SendDlgMessage(hDlg,DM_GETDLGITEM,DlgParams->width*col+row,(long)&DialogItem);
  if(DialogItem.Data[1]==STATE_CLOSE)
  {
    DialogItem.Data[1]=STATE_OPEN;
    DialogItem.VBuf[1].Char.AsciiChar=GetChar(DialogItem.Data[0],DialogItem.Data[1],&color);
    color|=GetBG(row,col,DialogItem.Data[1]);
    DialogItem.VBuf[0].Attributes=DialogItem.VBuf[1].Attributes=DialogItem.VBuf[2].Attributes=color;
    Info.SendDlgMessage(hDlg,DM_SETDLGITEM,DlgParams->width*col+row,(long)&DialogItem);
    if(!DialogItem.Data[0])
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
  else if(check==COP_CHECK_ALL&&DialogItem.Data[1]==STATE_OPEN&&DialogItem.Data[0]>0&&DialogItem.Data[0]<9)
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
    if(mines==DialogItem.Data[0]) //FIXME
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
  Info.SendDlgMessage(hDlg,DM_GETDLGITEM,DlgParams->width*DlgParams->curr_col+DlgParams->curr_row,(long)&DialogItem);
  DialogItem.VBuf[1].Char.AsciiChar=GetChar(DialogItem.Data[0],DialogItem.Data[1],&color);
  if(focused&&color==(FOREGROUND_RED|FOREGROUND_INTENSITY)) color=0;
  color|=focused?(BACKGROUND_RED|BACKGROUND_INTENSITY):(GetBG(DlgParams->curr_row,DlgParams->curr_col,DialogItem.Data[1]));
  DialogItem.VBuf[0].Attributes=DialogItem.VBuf[1].Attributes=DialogItem.VBuf[2].Attributes=color;
}

static void SetFocusUser(HANDLE hDlg,BoardParams *DlgParams,bool real)
{
  if(real)
    Info.SendDlgMessage(hDlg,DM_SETFOCUS,DlgParams->width*DlgParams->curr_col+DlgParams->curr_row,0);
  else
  {
    int curr_elem=Info.SendDlgMessage(hDlg,DM_GETFOCUS,0,0);
    DlgParams->curr_row=curr_elem%DlgParams->width;
    DlgParams->curr_col=curr_elem/DlgParams->width;
  }
  RedrawPlace(hDlg,DlgParams,true);
}

#define SetFocus(real) SetFocusUser(hDlg,DlgParams,real)
#define KillFocus() RedrawPlace(hDlg,DlgParams,false)

static void RunKey(DWORD Key)
{
  KeySequence key;
  key.Flags=KSFLAGS_DISABLEOUTPUT;
  key.Count=1;
  key.Sequence=(DWORD *)&Key;
  Info.AdvControl(Info.ModuleNumber,ACTL_POSTKEYSEQUENCE,&key);
}

static long WINAPI MainDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  BoardParams *DlgParams=(BoardParams *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  if(Msg==DN_KEY&&Param2==KEY_ENTER) return TRUE;
  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_RESETBOARD,0,0);
      SetFocus(true);
      break;
    case DN_KEY:
      KillFocus();
      switch(Param2)
      {
        case KEY_UP:
          DlgParams->curr_col--;
          break;
        case KEY_DOWN:
          DlgParams->curr_col++;
          break;
        case KEY_LEFT:
          DlgParams->curr_row--;
          break;
        case KEY_RIGHT:
          DlgParams->curr_row++;
          break;
        case KEY_HOME:
          DlgParams->curr_row=0;
          break;
        case KEY_END:
          DlgParams->curr_row=DlgParams->width-1;
          break;
        case KEY_PGUP:
          DlgParams->curr_col=0;
          break;
        case KEY_PGDN:
          DlgParams->curr_col=DlgParams->height-1;
          break;
        case KEY_SPACE:
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
        case KEY_DEL:
          {
            Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
            if(!DlgParams->finished)
            {
              FarDialogItem DialogItem;
              Info.SendDlgMessage(hDlg,DM_GETDLGITEM,DlgParams->width*DlgParams->curr_col+DlgParams->curr_row,(long)&DialogItem);
              switch(DialogItem.Data[1])
              {
                case STATE_CLOSE:
                  DialogItem.Data[1]=STATE_MARKED;
                  DlgParams->curr_mines++;
                  break;
                case STATE_OPEN:
                  break;
                case STATE_MARKED:
                  DialogItem.Data[1]=STATE_CLOSE;
                  DlgParams->curr_mines--;
                  break;
              }
              Info.SendDlgMessage(hDlg,DM_SETDLGITEM,DlgParams->width*DlgParams->curr_col+DlgParams->curr_row,(long)&DialogItem);
              Info.SendDlgMessage(hDlg,DM_SHOWTIME,0,0);
            }
            Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
          }
          break;
        case KEY_F2:
          DlgParams->started=false;
          DlgParams->finished=FINISH_NO;
          Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,DlgParams->width*DlgParams->height,(long)GetMsg(mStart));
          Info.SendDlgMessage(hDlg,DM_RESETBOARD,0,0);
          break;
        case KEY_F3:
          {
            char ScoreKeyName[1024],PlayerKeyName[1024];
            FSF.sprintf(ScoreKeyName,"Score_%d_%d_%d",DlgParams->width,DlgParams->height,DlgParams->mines);
            FSF.sprintf(PlayerKeyName,"Player_%d_%d_%d",DlgParams->width,DlgParams->height,DlgParams->mines);
            HKEY hKey;
            DWORD Type,DataSize=0;
            if((RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
            {
              DWORD Score; char Name[512];
              DataSize=sizeof(Score);
              if(RegQueryValueEx(hKey,ScoreKeyName,0,&Type,(LPBYTE)&Score,&DataSize)==ERROR_SUCCESS)
              {
                DataSize=sizeof(Name);
                if(RegQueryValueEx(hKey,PlayerKeyName,0,&Type,(LPBYTE)Name,&DataSize)==ERROR_SUCCESS)
                {
                  char buffer[1024];
                  FSF.sprintf(buffer,GetMsg(mHighscoreFormat),Name,Score);
                  const char *MsgItems[]={GetMsg(mHighscoreTitle),buffer,GetMsg(mOk)};
                  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeofa(MsgItems),1);
                }
              }
              RegCloseKey(hKey);
            }
          }
          break;
      }
      if(DlgParams->curr_row>=DlgParams->width) DlgParams->curr_row=0;
      if(DlgParams->curr_row<0) DlgParams->curr_row=DlgParams->width-1;
      if(DlgParams->curr_col>=DlgParams->height) DlgParams->curr_col=0;
      if(DlgParams->curr_col<0) DlgParams->curr_col=DlgParams->height-1;
      SetFocus(true);
      Info.SendDlgMessage(hDlg,DM_REDRAW,0,0);
      break;
    case DN_GOTFOCUS:
      SetFocus(false);
      break;
    case DN_KILLFOCUS:
      KillFocus();
      break;
    case DN_MOUSECLICK:
      {
        DWORD Buttons=((MOUSE_EVENT_RECORD *)Param2)->dwButtonState;
        if(Buttons&FROM_LEFT_1ST_BUTTON_PRESSED) RunKey(KEY_SPACE);
        else if(Buttons&RIGHTMOST_BUTTON_PRESSED) RunKey(KEY_DEL);
      }
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
            Info.SendDlgMessage(hDlg,DM_GETDLGITEM,k,(long)&DialogItem);
            DialogItem.Data[0]=DialogItem.Data[1]=0;
            color=0;
            DialogItem.VBuf[1].Char.AsciiChar=GetChar(DialogItem.Data[0],DialogItem.Data[1],&color);
            color|=GetBG(i,j,STATE_CLOSE);
            DialogItem.VBuf[0].Attributes=DialogItem.VBuf[1].Attributes=DialogItem.VBuf[2].Attributes=color;
            Info.SendDlgMessage(hDlg,DM_SETDLGITEM,k,(long)&DialogItem);
          }
        //set mines
        while(curr_mines)
          for(int i=0;i<DlgParams->height;i++)
            for(int j=0;j<DlgParams->width;j++)
            {
              int k=i*DlgParams->width+j;
              Info.SendDlgMessage(hDlg,DM_GETDLGITEM,k,(long)&DialogItem);
              if(curr_mines&&(!DialogItem.Data[0]))
              {
                if(rand()<=(RAND_MAX*DlgParams->mines/(DlgParams->width*DlgParams->height)))
                {
                  curr_mines--;
                  DialogItem.Data[0]=9;
                  Info.SendDlgMessage(hDlg,DM_SETDLGITEM,k,(long)&DialogItem);
                }
              }
            }
        //set places
        for(int i=0;i<DlgParams->height;i++)
          for(int j=0;j<DlgParams->width;j++)
          {
            int k=i*DlgParams->width+j;
            Info.SendDlgMessage(hDlg,DM_GETDLGITEM,k,(long)&DialogItem);
            if(DialogItem.Data[0]==0)
            {
              DialogItem.Data[0]+=GetMine(j-1,i-1,hDlg,DlgParams,false);
              DialogItem.Data[0]+=GetMine(j-1,i,hDlg,DlgParams,false);
              DialogItem.Data[0]+=GetMine(j-1,i+1,hDlg,DlgParams,false);
              DialogItem.Data[0]+=GetMine(j,i-1,hDlg,DlgParams,false);
              DialogItem.Data[0]+=GetMine(j,i+1,hDlg,DlgParams,false);
              DialogItem.Data[0]+=GetMine(j+1,i-1,hDlg,DlgParams,false);
              DialogItem.Data[0]+=GetMine(j+1,i,hDlg,DlgParams,false);
              DialogItem.Data[0]+=GetMine(j+1,i+1,hDlg,DlgParams,false);
              Info.SendDlgMessage(hDlg,DM_SETDLGITEM,k,(long)&DialogItem);
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
        char buffer[1024]; DWORD diff_time=DlgParams->started?((DlgParams->finished?DlgParams->end_time:GetTickCount())-DlgParams->start_time)/1000:0;
        FSF.sprintf(buffer,GetMsg(mTime),diff_time,DlgParams->curr_mines);
        Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,DlgParams->width*DlgParams->height+1,(long)buffer);
      }
      break;
    case DM_START_GAME:
      Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,DlgParams->width*DlgParams->height,(long)GetMsg(mGame));
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

HANDLE WINAPI OpenPlugin(int OpenFrom,int Item)
{
  (void)OpenFrom;
  (void)Item;
  int screen_width=80,screen_height=25;
  HANDLE console=CreateFile("CONOUT$",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
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
  FarMenuItemEx MenuItems[3];
  memset(MenuItems,0,sizeof(MenuItems));
  for(unsigned int i=0;i<sizeofa(MenuItems);i++)
  {
    MenuItems[i].Flags=MIF_DISABLE;
    if(screen_widths[i]<=screen_width&&screen_heights[i]<=screen_height) MenuItems[i].Flags=0;
    strcpy(MenuItems[i].Text.Text,GetMsg(mLev1+i));
  }
  MenuItems[0].Flags|=MIF_SELECTED;
  int MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_WRAPMODE|FMENU_USEEXT,GetMsg(mName),NULL,"Contents",NULL,NULL,(FarMenuItem *)MenuItems,sizeofa(MenuItems));
  if(MenuCode>=0)
  {
    BoardParams bp; int curr_color;
    bp.width=widths[MenuCode]; bp.height=heights[MenuCode]; bp.mines=mines[MenuCode];
    bp.curr_row=0; bp.curr_col=0;
    bp.started=false; bp.finished=FINISH_NO;
    FarDialogItem *DialogItems=(FarDialogItem *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(bp.width*bp.height+4)*sizeof(FarDialogItem));
    CHAR_INFO *VirtualBuffer=(CHAR_INFO *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,bp.width*bp.height*sizeof(CHAR_INFO)*3);
    if(DialogItems&&VirtualBuffer)
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
          DialogItems[k].VBuf[0].Char.AsciiChar=' ';
          DialogItems[k].VBuf[1].Char.AsciiChar='þ';
          DialogItems[k].VBuf[2].Char.AsciiChar=' ';
          curr_color=GetBG(j,i,STATE_CLOSE);
          for(int l=0;l<3;l++)
            DialogItems[k].VBuf[l].Attributes=curr_color|FOREGROUND_INTENSITY;
        }
      //Status
      {
        int k=bp.width*bp.height;
        DialogItems[k].Type=DI_TEXT;
        DialogItems[k].X1=1;
        DialogItems[k].X2=0;
        DialogItems[k].Y1=bp.height+2;
        DialogItems[k].Y2=0;
        strcpy(DialogItems[k].Data,GetMsg(mStart));
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
        memset(DialogItems[k].Data,'Ä',bp.width*3);
      }
      //border
      {
        int k=bp.width*bp.height+3;
        DialogItems[k].Type=DI_DOUBLEBOX;
        DialogItems[k].X1=0;
        DialogItems[k].X2=bp.width*3+1;
        DialogItems[k].Y1=0;
        DialogItems[k].Y2=bp.height+4;
        strcpy(DialogItems[k].Data,GetMsg(mName));
      }
      int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,bp.width*3+2,bp.height+5,"Contents",DialogItems,bp.width*bp.height+4,0,0,MainDialogProc,(long)&bp);
      if(DlgCode)
      {
      }
    }
    if(DialogItems) HeapFree(GetProcessHeap(),0,DialogItems);
    if(VirtualBuffer) HeapFree(GetProcessHeap(),0,VirtualBuffer);
  }
  return INVALID_HANDLE_VALUE;
}

int WINAPI _export Configure(int ItemNumber)
{
  switch(ItemNumber)
  {
    case 0:
      return(Config());
  }
  return FALSE;
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
