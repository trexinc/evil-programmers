#include <stdio.h>
#include "../../plugin.hpp"
#include "../../farkeys.hpp"
#include "../../farcolor.hpp"
#include "bcplugin.h"
#include "bcopy_fast_redraw.h"

struct OverwriteData
{
  BasicDialogData CommonData;
  InfoRec *receive;
};

enum
{
  LINK_BORDER=0,
  LINK_FILE,
  LINK_SEP1,
  LINK_ALL,
  LINK_COPY,
  LINK_LINK,
  LINK_SKIP,
  LINK_ABORT,
  LINK_CLOSE,
  LINK_IGNORE,
};

static long WINAPI LinkDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  OverwriteData *DlgParams=(OverwriteData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_CTLCOLORDIALOG:
      return Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGTEXT);
    case DN_CTLCOLORDLGITEM:
      switch(Param1)
      {
        case LINK_BORDER:
          return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOX)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTBOXTITLE)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOXTITLE));
        case LINK_FILE:
        case LINK_SEP1:
        case LINK_ALL:
          return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOX)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTTEXT)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGTEXT));
        case LINK_COPY:
        case LINK_LINK:
        case LINK_SKIP:
        case LINK_ABORT:
        case LINK_CLOSE:
        case LINK_IGNORE:
          if(Info.SendDlgMessage(hDlg,DM_GETFOCUS,0,0)==Param1)
            return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGSELECTEDBUTTON));
          else
            return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTBUTTON)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBUTTON));
      }
      break;
    case DN_TIMER:
      {
        Info.SendDlgMessage(hDlg,DM_REDRAW_UNICODE,DlgParams->CommonData.UnicodeInicies[0],0);
        Info.SendDlgMessage(hDlg,DM_CHECK_STATE,0,0);
      }
      break;
    case DM_CHECK_STATE:
      {
        InfoRec receive,*old_receive=DlgParams->receive;
        if(GetInfo(old_receive->info.ThreadId,&receive)) break;
        if(receive.info.type==INFOTYPE_INVALID||memcmp(&(old_receive->AskID),&(receive.AskID),sizeof(receive.AskID)))
          Info.SendDlgMessage(hDlg,DM_CLOSE,LINK_IGNORE,0);
      }
      break;
  }
  return FastRedrawDefDlgProc(hDlg,Msg,Param1,Param2);
}

static bool ShowLink(InfoRec *receive,HANDLE Console,ThreadData *Thread)
{
  static struct InitDialogItem InitItems[]={
  /* 0*/  {DI_DOUBLEBOX,3,1,74,7,0,0,0,0,(char *)mExistLinkFound},
  /* 1*/  {DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /* 2*/  {DI_TEXT,-1,3,0,0,0,0,DIF_SEPARATOR,0,""},
  /* 3*/  {DI_CHECKBOX,5,4,0,0,1,0,0,0,(char *)mExistAcceptChoice},
  /* 4*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,1,(char *)mExistCopy},
  /* 5*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char *)mExistLink},
  /* 6*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char *)mExistSkip},
  /* 7*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char *)mExistAbort},
  /* 8*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char *)mExistClose},
  /* 9*/  {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char *)mExistIgnore},
  };
  struct FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));
  WideCharToMultiByte(CP_OEMCP,0,receive->Src,-1,DialogItems[LINK_FILE].Data,sizeof(DialogItems[LINK_FILE].Data),NULL,NULL);
  FSF.TruncPathStr(DialogItems[LINK_FILE].Data,68);
  if(!PlgOpt.IgnoreButton) DialogItems[LINK_IGNORE].Flags|=DIF_HIDDEN;
  OverwriteData data={{Thread,0,{LINK_FILE,-1},{L"",L""},Console,false,NULL,MACRO_LINK,true,false},receive};
  if(Console!=INVALID_HANDLE_VALUE)
  {
    wcscpy(&(data.CommonData.FileNameW[0][0]),receive->Src);
    TruncPathStrW(&(data.CommonData.FileNameW[0][0]),68);
  }
  int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,78,9,NULL,DialogItems,sizeofa(DialogItems),0,0,LinkDlgProc,(DWORD)&data);
  if(DlgCode==-1) DlgCode=LINK_CLOSE;
  if(DlgCode<LINK_CLOSE)
  {
    DWORD send[3]={OPERATION_ANSWER,ASKTYPE_CANCEL,receive->info.ThreadId};
    DWORD Transfered;
    HANDLE hPipe=CreateFile(PIPE_NAME,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
    if(hPipe!=INVALID_HANDLE_VALUE)
    {
      switch(DlgCode)
      {
        case LINK_COPY:
          send[1]=ASKTYPE_COPY;
          break;
        case LINK_LINK:
          send[1]=ASKTYPE_LINK;
          break;
        case LINK_SKIP:
          send[1]=ASKTYPE_SKIP;
          break;
      }
      if(DialogItems[LINK_ALL].Selected) send[1]|=ASKFLAG_ALL;
      if(WriteFile(hPipe,send,sizeof(send),&Transfered,NULL))
        WriteFile(hPipe,&receive->AskID,sizeof(receive->AskID),&Transfered,NULL);
      CloseHandle(hPipe);
    }
  }
  return (DlgCode==LINK_CLOSE);
}

enum
{
  RETRY_BORDER=0,
  RETRY_SEP1,
  RETRY_SEP2,
  RETRY_RETRY,
  RETRY_SKIP,
  RETRY_SKIPALL,
  RETRY_ABORT,
  RETRY_CLOSE,
  RETRY_IGNORE,
  RETRY_FILE,
  RETRY_ERROR,
};

static long WINAPI RetryDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  OverwriteData *DlgParams=(OverwriteData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_CTLCOLORDIALOG:
      return Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGTEXT);
    case DN_CTLCOLORDLGITEM:
      switch(Param1)
      {
        case RETRY_BORDER:
          return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOX)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTBOXTITLE)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOXTITLE));
        case RETRY_RETRY:
        case RETRY_SKIP:
        case RETRY_SKIPALL:
        case RETRY_ABORT:
        case RETRY_CLOSE:
        case RETRY_IGNORE:
          if(Info.SendDlgMessage(hDlg,DM_GETFOCUS,0,0)==Param1)
            return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGSELECTEDBUTTON));
          else
            return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTBUTTON)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBUTTON));
        default:
          return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOX)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTTEXT)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGTEXT));
      }
      break;
    case DN_TIMER:
      {
        Info.SendDlgMessage(hDlg,DM_REDRAW_UNICODE,DlgParams->CommonData.UnicodeInicies[0],0);
        Info.SendDlgMessage(hDlg,DM_CHECK_STATE,0,0);
      }
      break;
    case DM_CHECK_STATE:
      {
        InfoRec receive,*old_receive=DlgParams->receive;
        if(GetInfo(old_receive->info.ThreadId,&receive)) break;
        if(receive.info.type==INFOTYPE_INVALID||memcmp(&(old_receive->AskID),&(receive.AskID),sizeof(receive.AskID)))
          Info.SendDlgMessage(hDlg,DM_CLOSE,RETRY_IGNORE,0);
      }
      break;
  }
  return FastRedrawDefDlgProc(hDlg,Msg,Param1,Param2);
}

static bool ShowRetry(InfoRec *receive,HANDLE Console,ThreadData *Thread)
{
  static struct InitDialogItem InitItems[]={
  /* 0*/  {DI_DOUBLEBOX,3,1,74,6,0,0,0,0,(char *)mError},
  /* 1*/  {DI_TEXT,-1,2,0,0,0,0,DIF_SEPARATOR,0,""},
  /* 2*/  {DI_TEXT,-1,4,0,0,0,0,DIF_SEPARATOR,0,""},
  /* 3*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,1,(char *)mExistRetry},
  /* 4*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char *)mExistSkip},
  /* 5*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char *)mExistSkipAll},
  /* 6*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char *)mExistAbort},
  /* 7*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char *)mExistClose},
  /* 8*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char *)mExistIgnore},
  /* 9*/  {DI_TEXT,5,3,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /*10*/  {DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /*11*/  {DI_TEXT,5,3,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /*12*/  {DI_TEXT,5,4,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /*13*/  {DI_TEXT,5,5,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /*14*/  {DI_TEXT,5,6,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /*15*/  {DI_TEXT,5,7,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /*16*/  {DI_TEXT,5,8,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /*17*/  {DI_TEXT,5,9,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /*18*/  {DI_TEXT,5,10,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /*19*/  {DI_TEXT,5,11,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /*20*/  {DI_TEXT,5,12,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /*21*/  {DI_TEXT,5,13,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /*22*/  {DI_TEXT,5,14,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /*23*/  {DI_TEXT,5,15,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /*24*/  {DI_TEXT,5,16,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  };
  int width=InitItems[RETRY_BORDER].X2-InitItems[RETRY_BORDER].X1-3,height=0,count=sizeofa(InitItems)-RETRY_ERROR;
  FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));
  WideCharToMultiByte(CP_OEMCP,0,receive->Src,-1,DialogItems[RETRY_ERROR].Data,sizeof(DialogItems[RETRY_ERROR].Data),NULL,NULL);
  NormalizeName(width,mExistCannotProcess,DialogItems[RETRY_ERROR].Data,DialogItems[RETRY_FILE].Data);
  {
    char *msg;
    if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ARGUMENT_ARRAY|width,NULL,receive->Error,LANG_NEUTRAL,(char *)&msg,512,NULL))
    {
      CharToOem(msg,msg);
      char *line_cur=msg,*line_end;
      for(int i=0;i<count;i++)
      {
        line_end=strchr(line_cur,0x0d);
        if(line_end)
        {
          strncpy(DialogItems[RETRY_ERROR+height].Data,line_cur,line_end-line_cur);
          DialogItems[RETRY_ERROR+height].Data[line_end-line_cur]=0;
          while(*line_end==0x0d||*line_end==0x0a) line_end++;
          line_cur=line_end;
          height++;
        }
        else
        {
          strncpy(DialogItems[RETRY_ERROR+height].Data,line_cur,width);
          DialogItems[RETRY_ERROR+height].Data[width]=0;
          height++;
          break;
        }
      }
      //adjust height
      DialogItems[RETRY_BORDER].Y2+=height;
      for(int i=RETRY_BORDER+1;i<RETRY_ERROR;i++)
        DialogItems[i].Y1+=height;
      LocalFree(msg);
    }
    //center
    for(int i=RETRY_FILE;i<(RETRY_ERROR+height);i++)
      DialogItems[i].X1+=(width-strlen(DialogItems[i].Data))/2;
  }
  if(!PlgOpt.IgnoreButton) DialogItems[RETRY_IGNORE].Flags|=DIF_HIDDEN;
  OverwriteData data={{Thread,0,{RETRY_FILE,-1},{L"",L""},Console,false,NULL,MACRO_RETRY,true,false},receive};
  if(Console!=INVALID_HANDLE_VALUE) NormalizeNameW(width,mExistCannotProcess,receive->Src,&(data.CommonData.FileNameW[0][0]));
  int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,78,8+height,NULL,DialogItems,RETRY_ERROR+height,0,0,RetryDlgProc,(DWORD)&data);
  if(DlgCode==-1) DlgCode=RETRY_CLOSE;
  if(DlgCode<RETRY_CLOSE)
  {
    DWORD send[3]={OPERATION_ANSWER,ASKTYPE_CANCEL,receive->info.ThreadId};
    DWORD Transfered;
    HANDLE hPipe=CreateFile(PIPE_NAME,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
    if(hPipe!=INVALID_HANDLE_VALUE)
    {
      switch(DlgCode)
      {
        case RETRY_RETRY:
          send[1]=ASKTYPE_RETRY;
          break;
        case RETRY_SKIP:
          send[1]=ASKTYPE_SKIP;
          break;
        case RETRY_SKIPALL:
          send[1]=ASKTYPE_SKIPALL;
          break;
      }
      if(WriteFile(hPipe,send,sizeof(send),&Transfered,NULL))
        WriteFile(hPipe,&receive->AskID,sizeof(receive->AskID),&Transfered,NULL);
      CloseHandle(hPipe);
    }
  }
  return (DlgCode==RETRY_CLOSE);
}

enum
{
  OVERWRITE_BORDER=0,
  OVERWRITE_FILE,
  OVERWRITE_SEP1,
  OVERWRITE_SRC,
  OVERWRITE_DST,
  OVERWRITE_SEP2,
  OVERWRITE_ALL,
  OVERWRITE_OVERWRITE,
  OVERWRITE_SKIP,
  OVERWRITE_APPEND,
  OVERWRITE_ABORT,
  OVERWRITE_CLOSE,
  OVERWRITE_IGNORE,
};

static void GetFileAttr(wchar_t *file,char *buffer,const char *format)
{
  unsigned long long size=0;
  SYSTEMTIME mod;
  memset(&mod,0,sizeof(SYSTEMTIME));
  buffer[0]=0;
  WIN32_FIND_DATAW find; HANDLE hFind;
  hFind=FindFirstFileW(file,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
    size=(unsigned long long)find.nFileSizeLow+(unsigned long long)find.nFileSizeHigh*4294967296ULL;
    FILETIME local;
    FileTimeToLocalFileTime(&find.ftLastWriteTime,&local);
    FileTimeToSystemTime(&local,&mod);
    sprintf(buffer,format,size,mod.wDay,mod.wMonth,mod.wYear,mod.wHour,mod.wMinute,mod.wSecond);
  }
}

static long WINAPI OverwriteDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  OverwriteData *DlgParams=(OverwriteData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_CTLCOLORDIALOG:
      return Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGTEXT);
    case DN_CTLCOLORDLGITEM:
      switch(Param1)
      {
        case OVERWRITE_BORDER:
          return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOX)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTBOXTITLE)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOXTITLE));
        case OVERWRITE_FILE:
        case OVERWRITE_SEP1:
        case OVERWRITE_SRC:
        case OVERWRITE_DST:
        case OVERWRITE_SEP2:
        case OVERWRITE_ALL:
          return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOX)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTTEXT)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGTEXT));
        case OVERWRITE_OVERWRITE:
        case OVERWRITE_SKIP:
        case OVERWRITE_APPEND:
        case OVERWRITE_ABORT:
        case OVERWRITE_CLOSE:
        case OVERWRITE_IGNORE:
          if(Info.SendDlgMessage(hDlg,DM_GETFOCUS,0,0)==Param1)
            return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGSELECTEDBUTTON));
          else
            return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTBUTTON)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBUTTON));
      }
      break;
    case DN_TIMER:
      {
        Info.SendDlgMessage(hDlg,DM_REDRAW_UNICODE,DlgParams->CommonData.UnicodeInicies[0],0);
        Info.SendDlgMessage(hDlg,DM_CHECK_STATE,0,0);
      }
      break;
    case DM_CHECK_STATE:
      {
        InfoRec receive,*old_receive=DlgParams->receive;
        if(GetInfo(old_receive->info.ThreadId,&receive)) break;
        if(receive.info.type==INFOTYPE_INVALID||memcmp(&(old_receive->AskID),&(receive.AskID),sizeof(receive.AskID)))
          Info.SendDlgMessage(hDlg,DM_CLOSE,OVERWRITE_IGNORE,0);
      }
      break;
  }
  return FastRedrawDefDlgProc(hDlg,Msg,Param1,Param2);
}

static bool ShowOverwrite(InfoRec *receive,HANDLE Console,ThreadData *Thread)
{
  static struct InitDialogItem InitItems[]={
  /* 0*/  {DI_DOUBLEBOX,3,1,74,10,0,0,0,0,(char *)mExistDestAlreadyExists},
  /* 1*/  {DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
  /* 2*/  {DI_TEXT,-1,3,0,0,0,0,DIF_SEPARATOR,0,""},
  /* 3*/  {DI_TEXT,5,4,0,0,0,0,0,0,""},
  /* 4*/  {DI_TEXT,5,5,0,0,0,0,0,0,""},
  /* 5*/  {DI_TEXT,-1,6,0,0,0,0,DIF_SEPARATOR,0,""},
  /* 6*/  {DI_CHECKBOX,5,7,0,0,1,0,0,0,(char *)mExistAcceptChoice},
  /* 7*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,1,(char *)mExistOverwrite},
  /* 8*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char *)mExistSkip},
  /* 9*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char *)mExistAppend},
  /*10*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char *)mExistAbort},
  /*11*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char *)mExistClose},
  /*12*/  {DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(char *)mExistIgnore},
  };
  struct FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));
  WideCharToMultiByte(CP_OEMCP,0,receive->Dest,-1,DialogItems[OVERWRITE_FILE].Data,sizeof(DialogItems[OVERWRITE_FILE].Data),NULL,NULL);
  FSF.TruncPathStr(DialogItems[OVERWRITE_FILE].Data,68);
  GetFileAttr(receive->Src,DialogItems[OVERWRITE_SRC].Data,GetMsg(mExistSource));
  GetFileAttr(receive->Dest,DialogItems[OVERWRITE_DST].Data,GetMsg(mExistDestination));
  if(!PlgOpt.IgnoreButton) DialogItems[OVERWRITE_IGNORE].Flags|=DIF_HIDDEN;
  OverwriteData data={{Thread,0,{OVERWRITE_FILE,-1},{L"",L""},Console,false,NULL,MACRO_OVERWRITE,true,false},receive};
  if(Console!=INVALID_HANDLE_VALUE)
  {
    wcscpy(&(data.CommonData.FileNameW[0][0]),receive->Dest);
    TruncPathStrW(&(data.CommonData.FileNameW[0][0]),68);
  }
  int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,78,12,NULL,DialogItems,sizeofa(DialogItems),0,0,OverwriteDlgProc,(DWORD)&data);
  if(DlgCode==-1) DlgCode=OVERWRITE_CLOSE;
  if(DlgCode<OVERWRITE_CLOSE)
  {
    DWORD send[3]={OPERATION_ANSWER,ASKTYPE_CANCEL,receive->info.ThreadId};
    DWORD Transfered;
    HANDLE hPipe=CreateFile(PIPE_NAME,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
    if(hPipe!=INVALID_HANDLE_VALUE)
    {
      switch(DlgCode)
      {
        case OVERWRITE_OVERWRITE:
          send[1]=ASKTYPE_OVERWRITE;
          break;
        case OVERWRITE_SKIP:
          send[1]=ASKTYPE_SKIP;
          break;
        case OVERWRITE_APPEND:
          send[1]=ASKTYPE_APPEND;
          break;
      }
      if(DialogItems[OVERWRITE_ALL].Selected) send[1]|=ASKFLAG_ALL;
      if(WriteFile(hPipe,send,sizeof(send),&Transfered,NULL))
        WriteFile(hPipe,&receive->AskID,sizeof(receive->AskID),&Transfered,NULL);
      CloseHandle(hPipe);
    }
  }
  return (DlgCode==OVERWRITE_CLOSE);
}

enum
{
  getinfoSuccess,
  getinfoError,
  getinfoBusy,
};

static inline void normalize_2(WORD &value)
{
  if(value>99) value=0;
}

static inline void normalize_4(WORD &value)
{
  if(value>9999) value=0;
}

struct InfoDlgParams
{
  BasicDialogData CommonData;
  DWORD Id;
  BOOL Time;
  BOOL Format;
  DWORD Error;
};

static long WINAPI InfoDialogKeyProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  (void)Param1;
  InfoDlgParams *DlgParams=(InfoDlgParams *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  if(Msg==DN_KEY)
  {
    if((Param2==KEY_ADD)||(Param2==KEY_SUBTRACT))
    {
      if(Param2==KEY_ADD)
        DlgParams->Time=!DlgParams->Time;
      else
        DlgParams->Format=!DlgParams->Format;
    }
  }
  return 0;
}

static long WINAPI InfoDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  InfoDlgParams *DlgParams=(InfoDlgParams *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_BTNCLICK:
      if(Param1==3) SendCommand(DlgParams->Id,INFOFLAG_PAUSE);
      break;
    case DN_CTLCOLORDLGITEM:
      if(DlgParams->Error&&(Param1==(int)DlgParams->Error)&&PlgOpt.ErrorColor)
        return PlgOpt.ErrorColor|(PlgOpt.ErrorColor<<8);
      break;
    case DN_TIMER:
      InfoRec receive;
      if(GetInfo(DlgParams->Id,&receive)) break;
      if(receive.info.type>INFOTYPE_INVALID)
      {
        const unsigned int shifts[]={0,2,2,0,0,0};
        unsigned int shift=shifts[receive.info.type];
        char SrcA[2*MAX_PATH];
        FarDialogItemData Caption;
        if(receive.info.pause)
          Caption.PtrData=(char *)GetMsg(mInfoDlgContinue);
        else
          Caption.PtrData=(char *)GetMsg(mInfoDlgPause);
        Caption.PtrLength=strlen(Caption.PtrData);
        Info.SendDlgMessage(hDlg,DM_SETTEXT,3,(long)&Caption);
        wcscpy(&(DlgParams->CommonData.FileNameW[0][0]),receive.Src); TruncPathStrW(&(DlgParams->CommonData.FileNameW[0][0]),66);
        WideCharToMultiByte(CP_OEMCP,0,receive.Src,-1,SrcA,sizeofa(SrcA),NULL,NULL);
        Caption.PtrData=FSF.TruncPathStr(SrcA,66);
        Caption.PtrLength=strlen(Caption.PtrData);
        Info.SendDlgMessage(hDlg,DM_SETTEXT,5,(long)&Caption);
        if(receive.info.type<INFOTYPE_WIPE)
        {
          wcscpy(&(DlgParams->CommonData.FileNameW[1][0]),receive.Dest); TruncPathStrW(&(DlgParams->CommonData.FileNameW[1][0]),66);
          WideCharToMultiByte(CP_OEMCP,0,receive.Dest,-1,SrcA,sizeofa(SrcA),NULL,NULL);
          Caption.PtrData=FSF.TruncPathStr(SrcA,66);
          Caption.PtrLength=strlen(Caption.PtrData);
          Info.SendDlgMessage(hDlg,DM_SETTEXT,8,(long)&Caption);
        }
        if((receive.info.InfoEx)&&(receive.TotalSize))
        {
          DlgParams->Error=(receive.Errors)?(10+shift):0;
          FILETIME start,current,finish; SYSTEMTIME CurrTime,FinishTime;
          unsigned long long llstart,llcurrent,llfinish; unsigned int percent;
          GetLocalTime(&CurrTime);
          SystemTimeToFileTime(&receive.StartTime,&start);
          SystemTimeToFileTime(&CurrTime,&current);
          memcpy(&llstart,&start,sizeof(llstart));
          memcpy(&llcurrent,&current,sizeof(llcurrent));
          llfinish=llstart;
          if(llcurrent<(llstart+receive.PauseTime)) llcurrent=llstart+receive.PauseTime;
          if((receive.CurrentSize+receive.CurrentSizeAdd)!=0LL)
          {
            double relation=receive.TotalSize;
            relation/=(receive.CurrentSize+receive.CurrentSizeAdd);
            llfinish+=(unsigned long long)(relation*(llcurrent-llstart-receive.PauseTime))+receive.PauseTime;
          }
          memcpy(&finish,&llfinish,sizeof(llfinish));
          FileTimeToSystemTime(&finish,&FinishTime);
          if(DlgParams->Format&&(!receive.SizeType))
          {
            unsigned long long cur_size=receive.CurrentSize+receive.CurrentSizeAdd,total_size=receive.TotalSize;
            int cur_unit=mUnits1,total_unit=mUnits1;
            while((cur_size>=10000)&&(cur_unit<=mUnits6))
            {
              cur_size/=1024;
              cur_unit++;
            }
            while((total_size>=10000)&&(total_unit<=mUnits6))
            {
              total_size/=1024;
              total_unit++;
            }
            sprintf(SrcA,GetMsg(mInfoDlgStatus1a),cur_size,GetMsg(cur_unit),total_size,GetMsg(total_unit));
          }
          else
            sprintf(SrcA,GetMsg(mInfoDlgStatus1),receive.CurrentSize+receive.CurrentSizeAdd,receive.TotalSize);
          Caption.PtrData=SrcA;
          Caption.PtrLength=strlen(Caption.PtrData);
          Info.SendDlgMessage(hDlg,DM_SETTEXT,8+shift,(long)&Caption);
          { //normalize finish time
            normalize_2(FinishTime.wDay);
            normalize_2(FinishTime.wMonth);
            normalize_4(FinishTime.wYear);
            normalize_2(FinishTime.wHour);
            normalize_2(FinishTime.wMinute);
            normalize_2(FinishTime.wSecond);
          }
          if(DlgParams->Time)
            sprintf(SrcA,GetMsg(mInfoDlgStatus2a),CurrTime.wDay,CurrTime.wMonth,CurrTime.wYear,CurrTime.wHour,CurrTime.wMinute,CurrTime.wSecond,FinishTime.wDay,FinishTime.wMonth,FinishTime.wYear,FinishTime.wHour,FinishTime.wMinute,FinishTime.wSecond);
          else
            sprintf(SrcA,GetMsg(mInfoDlgStatus2),receive.StartTime.wDay,receive.StartTime.wMonth,receive.StartTime.wYear,receive.StartTime.wHour,receive.StartTime.wMinute,receive.StartTime.wSecond,FinishTime.wDay,FinishTime.wMonth,FinishTime.wYear,FinishTime.wHour,FinishTime.wMinute,FinishTime.wSecond);
          Caption.PtrData=SrcA;
          Caption.PtrLength=strlen(Caption.PtrData);
          Info.SendDlgMessage(hDlg,DM_SETTEXT,9+shift,(long)&Caption);
          double fCPS=((llcurrent-llstart-receive.PauseTime)/10000000.0);
          if(fCPS>0.0)
            fCPS=(receive.CurrentSize+receive.CurrentSizeAdd)/fCPS;
          else
            fCPS=0.0;
          unsigned long long CPS=(unsigned long long)fCPS;
          if(DlgParams->Format&&(!receive.SizeType))
          {
            unsigned long long cps_norm=CPS;
            int cps_unit=mUnits1;
            while((cps_norm>=10000)&&(cps_unit<=mUnits6))
            {
              cps_norm/=1024;
              cps_unit++;
            }
            sprintf(SrcA,GetMsg(mInfoDlgStatus3a),receive.Errors,cps_norm,GetMsg(cps_unit));
          }
          else
            sprintf(SrcA,GetMsg((receive.SizeType)?mInfoDlgStatus3b:mInfoDlgStatus3),receive.Errors,CPS);
          Caption.PtrData=SrcA;
          Caption.PtrLength=strlen(Caption.PtrData);
          Info.SendDlgMessage(hDlg,DM_SETTEXT,10+shift,(long)&Caption);

          char *Fill=(char *)GetMsg(mInfoDlgFill);
          char Gauge[62];
          percent=0;
          if(receive.TotalSize!=0LL)
            percent=(receive.CurrentSize+receive.CurrentSizeAdd)*61/receive.TotalSize;
          for(unsigned int i=0;i<percent;i++)
            Gauge[i]=Fill[0];
          for(unsigned int i=percent;i<61;i++)
            Gauge[i]=Fill[1];
          Gauge[61]=0;
          percent=0;
          if(receive.TotalSize!=0LL)
            percent=(receive.CurrentSize+receive.CurrentSizeAdd)*100/receive.TotalSize;
          if(percent>100) percent=100;
          sprintf(SrcA,GetMsg(mInfoDlgStatus4),percent,Gauge);
          Caption.PtrData=SrcA;
          Caption.PtrLength=strlen(Caption.PtrData);
          Info.SendDlgMessage(hDlg,DM_SETTEXT,11+shift,(long)&Caption);
        }
        else if(receive.info.InfoEx)
        {
          Caption.PtrData=(char *)GetMsg(mInfoDlgWait);
          Caption.PtrLength=strlen(Caption.PtrData);
          Info.SendDlgMessage(hDlg,DM_SETTEXT,8+shift,(long)&Caption);
        }
        //refresh
        Info.SendDlgMessage(hDlg,DM_SETREDRAW,0,0);
        Info.SendDlgMessage(hDlg,DM_CHECK_STATE,0,0);
      }
      else
        Info.SendDlgMessage(hDlg,DM_CLOSE,1,0); //Ok button
      break;
    case DM_CHECK_STATE:
      {
        InfoRec receive;
        if(GetInfo(DlgParams->Id,&receive)) break;
        if(receive.info.type>INFOTYPE_INVALID)
        {
          switch(receive.info.Ask)
          {
            case ASKGROUP_OVERWRITE:
              if(ShowOverwrite(&receive,DlgParams->CommonData.Console,DlgParams->CommonData.Thread)) Info.SendDlgMessage(hDlg,DM_CLOSE,1,0); //Ok button
              break;
            case ASKGROUP_RETRY:
              if(ShowRetry(&receive,DlgParams->CommonData.Console,DlgParams->CommonData.Thread)) Info.SendDlgMessage(hDlg,DM_CLOSE,1,0); //Ok button
              break;
            case ASKGROUP_RETRYONLY:
              break;
            case ASKGROUP_LINK:
              if(ShowLink(&receive,DlgParams->CommonData.Console,DlgParams->CommonData.Thread)) Info.SendDlgMessage(hDlg,DM_CLOSE,1,0); //Ok button
              break;
          }
        }
      }
      break;
    case DN_CLOSE:
      Redraw_Close(DlgParams->CommonData.Thread);
      break;
  }
  return FastRedrawDefDlgProc(hDlg,Msg,Param1,Param2);
}

void ShowInfoDialog(SmallInfoRec *receive)
{
  //Show dialog
  /*
    0000000000111111111122222222223333333333444444444455555555556666666666777777
    0123456789012345678901234567890123456789012345678901234567890123456789012345
  00                                                                            00
  01   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ Copy ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»   01
  02   º Copying the file                                                   º   02
  03   º C:\Program Files\Adobe\Illustrator 9.0\Illustrator.exe             º   03
  04   º to                                                                 º   04
  05   º C:\TEMP\Illustrator 9.0\Illustrator.exe                            º   05
  06   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   06
  07   º Copied/Total:          19223372036854775807 / 19223372036854775807 º   07
  08   º Start/Finish:           20.01.2001 12.01.03 /  20.01.2001 12.01.03 º   08
  09   º Average CPS:                                  19223372036854775807 º   09
  10   º 100% ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ º   10
  11   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   11
  12   º                 [ OK ]    [ Pause ]     [ Stop ]                   º   12
  13   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   13
  14                                                                            14
    0000000000111111111122222222223333333333444444444455555555556666666666777777
    0123456789012345678901234567890123456789012345678901234567890123456789012345
  */
  struct FarDialogItem DialogItems[14];
  memset(DialogItems,0,sizeof(DialogItems));
  unsigned int i=0,height=8;
  const unsigned int height_add[]={0,2,2,0,0,0},last_row[]={0,11,11,9,9,9},info_add=5;
  height+=height_add[receive->type];
  if(receive->InfoEx)
    height+=info_add;
  //border
  DialogItems[i].Type=DI_DOUBLEBOX; DialogItems[i].X1=3; DialogItems[i].Y1=1;
  DialogItems[i].X2=72; DialogItems[i].Y2=height-2; i++;
  //Ok button
  DialogItems[i].Type=DI_BUTTON; DialogItems[i].Y1=height-3; DialogItems[i].Focus=1; DialogItems[i].DefaultButton=1;
  DialogItems[i].Flags=DIF_CENTERGROUP; strcpy(DialogItems[i].Data,GetMsg(mInfoDlgOk)); i++;
  //Stop button
  DialogItems[i].Type=DI_BUTTON; DialogItems[i].Y1=height-3;
  DialogItems[i].Flags=DIF_CENTERGROUP; strcpy(DialogItems[i].Data,GetMsg(mInfoDlgStop)); i++;
  //Continue button
  DialogItems[i].Type=DI_BUTTON; DialogItems[i].Y1=height-3;
  DialogItems[i].Flags=DIF_CENTERGROUP|DIF_BTNNOCLOSE; strcpy(DialogItems[i].Data,GetMsg(mInfoDlgPause)); i++;

  DialogItems[i].Type=DI_TEXT; DialogItems[i].X1=5; DialogItems[i].Y1=2;
  switch(receive->type)
  {
    case INFOTYPE_COPY:
      strcpy(DialogItems[i-4].Data,GetMsg(mCpyDlgCopyTitle));
      strcpy(DialogItems[i].Data,GetMsg(mInfoDlgCopy));
      break;
    case INFOTYPE_MOVE:
      strcpy(DialogItems[i-4].Data,GetMsg(mCpyDlgMoveTitle));
      strcpy(DialogItems[i].Data,GetMsg(mInfoDlgMove));
      break;
    case INFOTYPE_WIPE:
      strcpy(DialogItems[i-4].Data,GetMsg(mWpeDlgTitle));
      strcpy(DialogItems[i].Data,GetMsg(mInfoDlgWipe));
      break;
    case INFOTYPE_DELETE:
      strcpy(DialogItems[i-4].Data,GetMsg(mDelDlgTitle));
      strcpy(DialogItems[i].Data,GetMsg(mInfoDlgDelete));
      break;
    case INFOTYPE_ATTR:
      strcpy(DialogItems[i-4].Data,GetMsg(mAttrDlgTitle));
      strcpy(DialogItems[i].Data,GetMsg(mInfoDlgAttr));
      break;
  }
  i++;
  DialogItems[i].Type=DI_TEXT; DialogItems[i].X1=5; DialogItems[i].Y1=3;
  DialogItems[i].Flags=DIF_SHOWAMPERSAND; i++;
  DialogItems[i].Type=DI_TEXT; DialogItems[i].Y1=height-4;
  DialogItems[i].Flags=DIF_BOXCOLOR|DIF_SEPARATOR; i++;

  unsigned int j=4; //i=6
  if(receive->type<INFOTYPE_WIPE)
  {
    DialogItems[i].Type=DI_TEXT; DialogItems[i].X1=5; DialogItems[i].Y1=j;
    strcpy(DialogItems[i].Data,GetMsg(mInfoDlgTo)); i++; j++;
    DialogItems[i].Type=DI_TEXT; DialogItems[i].X1=5; DialogItems[i].Y1=j;
    DialogItems[i].Flags=DIF_SHOWAMPERSAND; i++; j++;
  }
  if(receive->InfoEx) //i=8 or 6
  {
    DialogItems[i].Flags=DIF_BOXCOLOR|DIF_SEPARATOR;
    for(;j<last_row[receive->type];i++,j++)
    {
      DialogItems[i].Type=DI_TEXT; DialogItems[i].X1=5; DialogItems[i].Y1=j;
    }
  }
  ThreadData Thread={FALSE,NULL,NULL,NULL,PlgOpt.RefreshInterval};
  InitThreadData(&Thread);
  InfoDlgParams DlgParams={{&Thread,0,{5,(receive->type<INFOTYPE_WIPE)?8:-1},{L"",L""},INVALID_HANDLE_VALUE,false,InfoDialogKeyProc,MACRO_INFO,true,false},receive->ThreadId,PlgOpt.CurrentTime,PlgOpt.FormatSize,0};
  if(PlgOpt.ShowUnicode) DlgParams.CommonData.Console=CreateFileW(L"CONOUT$",GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,76,height,"Info",DialogItems,i,0,0,InfoDialogProc,(DWORD)&DlgParams);
  PlgOpt.CurrentTime=DlgParams.Time; PlgOpt.FormatSize=DlgParams.Format;
  if(DlgParams.CommonData.Console!=INVALID_HANDLE_VALUE) CloseHandle(DlgParams.CommonData.Console);
  FreeThreadData(&Thread);
  if((DlgCode==2)||(DlgCode==-1)) AbortThread(receive->ThreadId);
  if(PlgOpt.AutoRefresh)
  {
    char left[MAX_PATH],right[MAX_PATH],from[MAX_PATH],to[MAX_PATH];
    char *panels[]={left,right};
    int get[]={FCTL_GETPANELSHORTINFO,FCTL_GETANOTHERPANELSHORTINFO};
    int update[]={FCTL_UPDATEPANEL,FCTL_UPDATEANOTHERPANEL};
    int redraw[]={FCTL_REDRAWPANEL,FCTL_REDRAWANOTHERPANEL};
    PanelInfo PInfo;
    for(unsigned int i=0;i<sizeofa(panels);i++)
    {
      Info.Control(INVALID_HANDLE_VALUE,get[i],&PInfo);
      strcpy(panels[i],"|");
      if(PInfo.PanelType==PTYPE_FILEPANEL)
      {
        strcpy(panels[i],PInfo.CurDir);
        UNCPath(panels[i]);
        FSF.AddEndSlash(panels[i]);
      }
    }
    WideCharToMultiByte(CP_OEMCP,0,receive->SrcDir,-1,from,sizeofa(from),NULL,NULL);
    WideCharToMultiByte(CP_OEMCP,0,receive->DestDir,-1,to,sizeofa(to),NULL,NULL);
    if(receive->type>INFOTYPE_MOVE) strcpy(to,"*");
    if(receive->type==INFOTYPE_COPY||receive->type==INFOTYPE_ATTR) strcpy(from,"*");
    for(unsigned int i=0;i<sizeofa(panels);i++)
    {
      if(!_stricmp(panels[i],from)||!_stricmp(panels[i],to))
      {
        Info.Control(INVALID_HANDLE_VALUE,update[i],(void *)1);
        Info.Control(INVALID_HANDLE_VALUE,redraw[i],NULL);
      }
    }
  }
}

void SendCommand(DWORD ThreadId,DWORD Command)
{
  DWORD send[3]={OPERATION_INFO,Command,ThreadId};
  DWORD Transfered;
  HANDLE hPipe=CreateFile(PIPE_NAME,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
  if(hPipe!=INVALID_HANDLE_VALUE)
  {
    WriteFile(hPipe,send,sizeof(send),&Transfered,NULL);
    CloseHandle(hPipe);
  }
}

bool GetInfo(DWORD ThreadId,InfoRec *receive)
{
  bool res=false;
  DWORD dwBytesRead,dwBytesWritten,RecSize;
  DWORD send[3]={OPERATION_INFO,INFOFLAG_BYHANDLE};
  send[2]=ThreadId;
  receive->info.type=INFOTYPE_INVALID;
  HANDLE hPipe=CreateFile(PIPE_NAME,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
  if(hPipe!=INVALID_HANDLE_VALUE)
  {
    WriteFile(hPipe,send,sizeof(send),&dwBytesWritten,NULL);
    if(ReadFile(hPipe,&RecSize,sizeof(RecSize),&dwBytesRead,NULL)) //FIXME: get size from pipe
      ReadFile(hPipe,receive,sizeof(InfoRec),&dwBytesRead,NULL);
    CloseHandle(hPipe);
  }
  else if(GetLastError()==ERROR_PIPE_BUSY) res=true;
  return res;
}

void AbortThread(DWORD ThreadId)
{
  bool process=true;
  if(PlgOpt.ConfirmAbort)
  {
    InfoRec receive_big;
    bool busy=GetInfo(ThreadId,&receive_big);
    if((receive_big.info.type>INFOTYPE_INVALID)||(busy))
    {
      BOOL pause_old=receive_big.info.pause;
      if((!busy)&&(!pause_old)) SendCommand(ThreadId,INFOFLAG_PAUSE);
      const char *MsgItems[]={GetMsg(mName),GetMsg(mAbortText),GetMsg(mInfoDlgStop),GetMsg(mInfoDlgContinue)};
      if(Info.Message(Info.ModuleNumber,FMSG_WARNING,"Abort",MsgItems,sizeofa(MsgItems),2)) process=false;
      if((!busy)&&(!pause_old))
      {
        GetInfo(ThreadId,&receive_big);
        if((receive_big.info.type>INFOTYPE_INVALID)&&(receive_big.info.pause))
          SendCommand(ThreadId,INFOFLAG_PAUSE);
      }
    }
  }
  if(process) SendCommand(ThreadId,INFOFLAG_STOP);
}
