/*
    bcopy_info.cpp
    Copyright (C) 2000-2009 zg

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

#include <stdio.h>
#include "far_helper.h"
#include "farkeys.hpp"
#include "farcolor.hpp"
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

static LONG_PTR WINAPI LinkDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
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
  /* 0*/  {DI_DOUBLEBOX,3,1,74,7,0,0,0,0,(TCHAR *)mExistLinkFound},
  /* 1*/  {DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /* 2*/  {DI_TEXT,-1,3,0,0,0,0,DIF_SEPARATOR,0,_T("")},
  /* 3*/  {DI_CHECKBOX,5,4,0,0,1,0,0,0,(TCHAR *)mExistAcceptChoice},
  /* 4*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,1,(TCHAR *)mExistCopy},
  /* 5*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(TCHAR *)mExistLink},
  /* 6*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(TCHAR *)mExistSkip},
  /* 7*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(TCHAR *)mExistAbort},
  /* 8*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(TCHAR *)mExistClose},
  /* 9*/  {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(TCHAR *)mExistIgnore},
  };
  struct FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));
  TCHAR link_file[512];
  wc2mb(receive->Src,link_file,ArraySize(link_file));
  FSF.TruncPathStr(link_file,68);
  INIT_DLG_DATA(DialogItems[LINK_FILE],link_file);
  if(!PlgOpt.IgnoreButton) DialogItems[LINK_IGNORE].Flags|=DIF_HIDDEN;
  OverwriteData data={{Thread,0,{LINK_FILE,-1},{L"",L""},Console,false,NULL,MACRO_LINK,true,false},receive};
  if(Console!=INVALID_HANDLE_VALUE)
  {
    wcscpy(&(data.CommonData.FileNameW[0][0]),receive->Src);
    TruncPathStrW(&(data.CommonData.FileNameW[0][0]),68);
  }
  CFarDialog dialog;
  int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,78,9,NULL,DialogItems,sizeofa(DialogItems),0,0,LinkDlgProc,(LONG_PTR)&data);
  if(DlgCode==-1) DlgCode=LINK_CLOSE;
  if(DlgCode<LINK_CLOSE)
  {
    DWORD send[3]={OPERATION_ANSWER,ASKTYPE_CANCEL,receive->info.ThreadId};
    DWORD Transfered;
    HANDLE hPipe=CreateFile(PIPE_NAMEP,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
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
      if(dialog.Check(LINK_ALL)) send[1]|=ASKFLAG_ALL;
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

static LONG_PTR WINAPI RetryDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
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
  /* 0*/  {DI_DOUBLEBOX,3,1,74,6,0,0,0,0,(TCHAR *)mError},
  /* 1*/  {DI_TEXT,-1,2,0,0,0,0,DIF_SEPARATOR,0,_T("")},
  /* 2*/  {DI_TEXT,-1,4,0,0,0,0,DIF_SEPARATOR,0,_T("")},
  /* 3*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,1,(TCHAR *)mExistRetry},
  /* 4*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(TCHAR *)mExistSkip},
  /* 5*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(TCHAR *)mExistSkipAll},
  /* 6*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(TCHAR *)mExistAbort},
  /* 7*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(TCHAR *)mExistClose},
  /* 8*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(TCHAR *)mExistIgnore},
  /* 9*/  {DI_TEXT,5,3,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /*10*/  {DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /*11*/  {DI_TEXT,5,3,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /*12*/  {DI_TEXT,5,4,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /*13*/  {DI_TEXT,5,5,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /*14*/  {DI_TEXT,5,6,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /*15*/  {DI_TEXT,5,7,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /*16*/  {DI_TEXT,5,8,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /*17*/  {DI_TEXT,5,9,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /*18*/  {DI_TEXT,5,10,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /*19*/  {DI_TEXT,5,11,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /*20*/  {DI_TEXT,5,12,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /*21*/  {DI_TEXT,5,13,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /*22*/  {DI_TEXT,5,14,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /*23*/  {DI_TEXT,5,15,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /*24*/  {DI_TEXT,5,16,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  };
  int width=InitItems[RETRY_BORDER].X2-InitItems[RETRY_BORDER].X1-3,height=0,count=sizeofa(InitItems)-RETRY_ERROR;
  FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));
  TCHAR retry_error[16][512];
  wc2mb(receive->Src,retry_error[1],ArraySize(retry_error[1]));
  NormalizeName(width,mExistCannotProcess,retry_error[1],retry_error[0]);
  {
    TCHAR *msg;
    if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ARGUMENT_ARRAY|width,NULL,receive->Error,LANG_NEUTRAL,(TCHAR *)&msg,512,NULL))
    {
#ifndef UNICODE
      CharToOem(msg,msg);
#endif
      TCHAR *line_cur=msg,*line_end;
      for(int i=0;i<count;i++)
      {
        line_end=_tcschr(line_cur,0x0d);
        if(line_end)
        {
          _tcsncpy(retry_error[RETRY_ERROR-RETRY_FILE+height],line_cur,line_end-line_cur);
          retry_error[RETRY_ERROR-RETRY_FILE+height][line_end-line_cur]=0;
          while(*line_end==0x0d||*line_end==0x0a) line_end++;
          line_cur=line_end;
          height++;
        }
        else
        {
          _tcsncpy(retry_error[RETRY_ERROR-RETRY_FILE+height],line_cur,width);
          retry_error[RETRY_ERROR-RETRY_FILE+height][width]=0;
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
      DialogItems[i].X1+=(width-(int)_tcslen(retry_error[i-RETRY_FILE]))/2;
  }
  for(size_t ii=0;ii<ArraySize(retry_error);++ii)
    INIT_DLG_DATA(DialogItems[RETRY_FILE+ii],retry_error[ii]);
  if(!PlgOpt.IgnoreButton) DialogItems[RETRY_IGNORE].Flags|=DIF_HIDDEN;
  OverwriteData data={{Thread,0,{RETRY_FILE,-1},{L"",L""},Console,false,NULL,MACRO_RETRY,true,false},receive};
  if(Console!=INVALID_HANDLE_VALUE) NormalizeNameW(width,mExistCannotProcess,receive->Src,&(data.CommonData.FileNameW[0][0]));
  CFarDialog dialog;
  int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,78,8+height,NULL,DialogItems,RETRY_ERROR+height,0,0,RetryDlgProc,(LONG_PTR)&data);
  if(DlgCode==-1) DlgCode=RETRY_CLOSE;
  if(DlgCode<RETRY_CLOSE)
  {
    DWORD send[3]={OPERATION_ANSWER,ASKTYPE_CANCEL,receive->info.ThreadId};
    DWORD Transfered;
    HANDLE hPipe=CreateFile(PIPE_NAMEP,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
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

static void GetFileAttr(wchar_t *file,TCHAR *buffer,const TCHAR *format)
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
    _stprintf(buffer,format,size,mod.wDay,mod.wMonth,mod.wYear,mod.wHour,mod.wMinute,mod.wSecond);
  }
}

static LONG_PTR WINAPI OverwriteDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
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
  /* 0*/  {DI_DOUBLEBOX,3,1,74,10,0,0,0,0,(TCHAR *)mExistDestAlreadyExists},
  /* 1*/  {DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
  /* 2*/  {DI_TEXT,-1,3,0,0,0,0,DIF_SEPARATOR,0,_T("")},
  /* 3*/  {DI_TEXT,5,4,0,0,0,0,0,0,_T("")},
  /* 4*/  {DI_TEXT,5,5,0,0,0,0,0,0,_T("")},
  /* 5*/  {DI_TEXT,-1,6,0,0,0,0,DIF_SEPARATOR,0,_T("")},
  /* 6*/  {DI_CHECKBOX,5,7,0,0,1,0,0,0,(TCHAR *)mExistAcceptChoice},
  /* 7*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,1,(TCHAR *)mExistOverwrite},
  /* 8*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(TCHAR *)mExistSkip},
  /* 9*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(TCHAR *)mExistAppend},
  /*10*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(TCHAR *)mExistAbort},
  /*11*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(TCHAR *)mExistClose},
  /*12*/  {DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,(TCHAR *)mExistIgnore},
  };
  struct FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));
  TCHAR overwrite_file[512],overwrite_src[512],overwrite_dst[512];
  wc2mb(receive->Dest,overwrite_file,ArraySize(overwrite_file));
  FSF.TruncPathStr(overwrite_file,68);
  GetFileAttr(receive->Src,overwrite_src,GetMsg(mExistSource));
  GetFileAttr(receive->Dest,overwrite_dst,GetMsg(mExistDestination));
  INIT_DLG_DATA(DialogItems[OVERWRITE_FILE],overwrite_file);
  INIT_DLG_DATA(DialogItems[OVERWRITE_SRC],overwrite_src);
  INIT_DLG_DATA(DialogItems[OVERWRITE_DST],overwrite_dst);
  if(!PlgOpt.IgnoreButton) DialogItems[OVERWRITE_IGNORE].Flags|=DIF_HIDDEN;
  OverwriteData data={{Thread,0,{OVERWRITE_FILE,-1},{L"",L""},Console,false,NULL,MACRO_OVERWRITE,true,false},receive};
  if(Console!=INVALID_HANDLE_VALUE)
  {
    wcscpy(&(data.CommonData.FileNameW[0][0]),receive->Dest);
    TruncPathStrW(&(data.CommonData.FileNameW[0][0]),68);
  }
  CFarDialog dialog;
  int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,78,12,NULL,DialogItems,sizeofa(DialogItems),0,0,OverwriteDlgProc,(LONG_PTR)&data);
  if(DlgCode==-1) DlgCode=OVERWRITE_CLOSE;
  if(DlgCode<OVERWRITE_CLOSE)
  {
    DWORD send[3]={OPERATION_ANSWER,ASKTYPE_CANCEL,receive->info.ThreadId};
    DWORD Transfered;
    HANDLE hPipe=CreateFile(PIPE_NAMEP,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
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
      if(dialog.Check(OVERWRITE_ALL)) send[1]|=ASKFLAG_ALL;
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
  BOOL Closed;
};

static LONG_PTR WINAPI InfoDialogKeyProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
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

static LONG_PTR WINAPI InfoDialogProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
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
        TCHAR SrcA[2*MAX_PATH];
        FarDialogItemData Caption;
        if(receive.info.pause)
          Caption.PtrData=(TCHAR *)GetMsg(mInfoDlgContinue);
        else
          Caption.PtrData=(TCHAR *)GetMsg(mInfoDlgPause);
        Caption.PtrLength=(int)_tcslen(Caption.PtrData);
        Info.SendDlgMessage(hDlg,DM_SETTEXT,3,(LONG_PTR)&Caption);
        wcscpy(&(DlgParams->CommonData.FileNameW[0][0]),receive.Src); TruncPathStrW(&(DlgParams->CommonData.FileNameW[0][0]),66);
        wc2mb(receive.Src,SrcA,sizeofa(SrcA));
        Caption.PtrData=FSF.TruncPathStr(SrcA,66);
        Caption.PtrLength=(int)_tcslen(Caption.PtrData);
        Info.SendDlgMessage(hDlg,DM_SETTEXT,5,(LONG_PTR)&Caption);
        if(receive.info.type<INFOTYPE_WIPE)
        {
          wcscpy(&(DlgParams->CommonData.FileNameW[1][0]),receive.Dest); TruncPathStrW(&(DlgParams->CommonData.FileNameW[1][0]),66);
          wc2mb(receive.Dest,SrcA,sizeofa(SrcA));
          Caption.PtrData=FSF.TruncPathStr(SrcA,66);
          Caption.PtrLength=(int)_tcslen(Caption.PtrData);
          Info.SendDlgMessage(hDlg,DM_SETTEXT,8,(LONG_PTR)&Caption);
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
            double relation=(double)receive.TotalSize;
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
            _stprintf(SrcA,GetMsg(mInfoDlgStatus1a),cur_size,GetMsg(cur_unit),total_size,GetMsg(total_unit));
          }
          else
            _stprintf(SrcA,GetMsg(mInfoDlgStatus1),receive.CurrentSize+receive.CurrentSizeAdd,receive.TotalSize);
          Caption.PtrData=SrcA;
          Caption.PtrLength=(int)_tcslen(Caption.PtrData);
          Info.SendDlgMessage(hDlg,DM_SETTEXT,8+shift,(LONG_PTR)&Caption);
          { //normalize finish time
            normalize_2(FinishTime.wDay);
            normalize_2(FinishTime.wMonth);
            normalize_4(FinishTime.wYear);
            normalize_2(FinishTime.wHour);
            normalize_2(FinishTime.wMinute);
            normalize_2(FinishTime.wSecond);
          }
          if(DlgParams->Time)
            _stprintf(SrcA,GetMsg(mInfoDlgStatus2a),CurrTime.wDay,CurrTime.wMonth,CurrTime.wYear,CurrTime.wHour,CurrTime.wMinute,CurrTime.wSecond,FinishTime.wDay,FinishTime.wMonth,FinishTime.wYear,FinishTime.wHour,FinishTime.wMinute,FinishTime.wSecond);
          else
            _stprintf(SrcA,GetMsg(mInfoDlgStatus2),receive.StartTime.wDay,receive.StartTime.wMonth,receive.StartTime.wYear,receive.StartTime.wHour,receive.StartTime.wMinute,receive.StartTime.wSecond,FinishTime.wDay,FinishTime.wMonth,FinishTime.wYear,FinishTime.wHour,FinishTime.wMinute,FinishTime.wSecond);
          Caption.PtrData=SrcA;
          Caption.PtrLength=(int)_tcslen(Caption.PtrData);
          Info.SendDlgMessage(hDlg,DM_SETTEXT,9+shift,(LONG_PTR)&Caption);
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
            _stprintf(SrcA,GetMsg(mInfoDlgStatus3a),receive.Errors,cps_norm,GetMsg(cps_unit));
          }
          else
            _stprintf(SrcA,GetMsg((receive.SizeType)?mInfoDlgStatus3b:mInfoDlgStatus3),receive.Errors,CPS);
          Caption.PtrData=SrcA;
          Caption.PtrLength=(int)_tcslen(Caption.PtrData);
          Info.SendDlgMessage(hDlg,DM_SETTEXT,10+shift,(LONG_PTR)&Caption);

          TCHAR *Fill=(TCHAR *)GetMsg(mInfoDlgFill);
          TCHAR Gauge[62];
          percent=0;
          if(receive.TotalSize!=0LL)
            percent=(unsigned int)((receive.CurrentSize+receive.CurrentSizeAdd)*61/receive.TotalSize);
          for(unsigned int i=0;i<percent;i++)
            Gauge[i]=Fill[0];
          for(unsigned int i=percent;i<61;i++)
            Gauge[i]=Fill[1];
          Gauge[61]=0;
          percent=0;
          if(receive.TotalSize!=0LL)
            percent=(unsigned int)((receive.CurrentSize+receive.CurrentSizeAdd)*100/receive.TotalSize);
          if(percent>100) percent=100;
          _stprintf(SrcA,GetMsg(mInfoDlgStatus4),percent,Gauge);
          Caption.PtrData=SrcA;
          Caption.PtrLength=(int)_tcslen(Caption.PtrData);
          Info.SendDlgMessage(hDlg,DM_SETTEXT,11+shift,(LONG_PTR)&Caption);
          const unsigned int titles[]={mCpyDlgCopyTitle,mCpyDlgCopyTitle,mCpyDlgMoveTitle,mWpeDlgTitle,mDelDlgTitle,mAttrDlgTitle};
          _stprintf(SrcA,GetMsg(mInfoDlgStatus4),percent,GetMsg(titles[receive.info.type]));
          Caption.PtrData=SrcA;
          Caption.PtrLength=(int)_tcslen(Caption.PtrData);
          Info.SendDlgMessage(hDlg,DM_SETTEXT,0,(LONG_PTR)&Caption);
        }
        else if(receive.info.InfoEx)
        {
          Caption.PtrData=(TCHAR *)GetMsg(mInfoDlgWait);
          Caption.PtrLength=(int)_tcslen(Caption.PtrData);
          Info.SendDlgMessage(hDlg,DM_SETTEXT,8+shift,(LONG_PTR)&Caption);
        }
        //refresh
        Info.SendDlgMessage(hDlg,DM_SETREDRAW,0,0);
        Info.SendDlgMessage(hDlg,DM_CHECK_STATE,0,0);
      }
      else
        Info.SendDlgMessage(hDlg,DM_CLOSE,1,0); //Ok button
      break;
    case DM_CHECK_STATE:
      if(!DlgParams->Closed)
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
      DlgParams->Closed=TRUE;
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
  DialogItems[i].Flags=DIF_CENTERGROUP; INIT_DLG_DATA(DialogItems[i],GetMsg(mInfoDlgOk)); i++;
  //Stop button
  DialogItems[i].Type=DI_BUTTON; DialogItems[i].Y1=height-3;
  DialogItems[i].Flags=DIF_CENTERGROUP; INIT_DLG_DATA(DialogItems[i],GetMsg(mInfoDlgStop)); i++;
  //Continue button
  DialogItems[i].Type=DI_BUTTON; DialogItems[i].Y1=height-3;
  DialogItems[i].Flags=DIF_CENTERGROUP|DIF_BTNNOCLOSE; INIT_DLG_DATA(DialogItems[i],GetMsg(mInfoDlgPause)); i++;

  DialogItems[i].Type=DI_TEXT; DialogItems[i].X1=5; DialogItems[i].Y1=2;
  switch(receive->type)
  {
    case INFOTYPE_COPY:
      INIT_DLG_DATA(DialogItems[i-4],GetMsg(mCpyDlgCopyTitle));
      INIT_DLG_DATA(DialogItems[i],GetMsg(mInfoDlgCopy));
      break;
    case INFOTYPE_MOVE:
      INIT_DLG_DATA(DialogItems[i-4],GetMsg(mCpyDlgMoveTitle));
      INIT_DLG_DATA(DialogItems[i],GetMsg(mInfoDlgMove));
      break;
    case INFOTYPE_WIPE:
      INIT_DLG_DATA(DialogItems[i-4],GetMsg(mWpeDlgTitle));
      INIT_DLG_DATA(DialogItems[i],GetMsg(mInfoDlgWipe));
      break;
    case INFOTYPE_DELETE:
      INIT_DLG_DATA(DialogItems[i-4],GetMsg(mDelDlgTitle));
      INIT_DLG_DATA(DialogItems[i],GetMsg(mInfoDlgDelete));
      break;
    case INFOTYPE_ATTR:
      INIT_DLG_DATA(DialogItems[i-4],GetMsg(mAttrDlgTitle));
      INIT_DLG_DATA(DialogItems[i],GetMsg(mInfoDlgAttr));
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
    INIT_DLG_DATA(DialogItems[i],GetMsg(mInfoDlgTo)); i++; j++;
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
  InfoDlgParams DlgParams={{&Thread,0,{5,(receive->type<INFOTYPE_WIPE)?8:-1},{L"",L""},INVALID_HANDLE_VALUE,false,InfoDialogKeyProc,MACRO_INFO,true,false},receive->ThreadId,PlgOpt.CurrentTime,PlgOpt.FormatSize,0,FALSE};
  if(PlgOpt.ShowUnicode) DlgParams.CommonData.Console=CreateFileW(L"CONOUT$",GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  CFarDialog dialog;
  int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,76,height,_T("Info"),DialogItems,i,0,0,InfoDialogProc,(LONG_PTR)&DlgParams);
  PlgOpt.CurrentTime=DlgParams.Time; PlgOpt.FormatSize=DlgParams.Format;
  if(DlgParams.CommonData.Console!=INVALID_HANDLE_VALUE) CloseHandle(DlgParams.CommonData.Console);
  FreeThreadData(&Thread);
  if((DlgCode==2)||(DlgCode==-1)) AbortThread(receive->ThreadId);
  if(PlgOpt.AutoRefresh)
  {
    TCHAR left[MAX_PATH],right[MAX_PATH],from[MAX_PATH],to[MAX_PATH];
    TCHAR *panels[]={left,right};
    int get[]={FCTL_GETPANELSHORTINFO,FCTL_GETANOTHERPANELSHORTINFO};
    int update[]={FCTL_UPDATEPANEL,FCTL_UPDATEANOTHERPANEL};
    int redraw[]={FCTL_REDRAWPANEL,FCTL_REDRAWANOTHERPANEL};
    HANDLE panel[]={PANEL_ACTIVE,PANEL_PASSIVE};
    for(unsigned int i=0;i<sizeofa(panels);i++)
    {
      CFarPanel pInfo(panel[i],get[i]);
      _tcscpy(panels[i],_T("|"));
      if(pInfo.PanelType()==PTYPE_FILEPANEL)
      {
        _tcscpy(panels[i],pInfo.CurDir());
        UNCPath(panels[i]);
        FSF.AddEndSlash(panels[i]);
      }
    }
    wc2mb(receive->SrcDir,from,sizeofa(from));
    wc2mb(receive->DestDir,to,sizeofa(to));
    if(receive->type>INFOTYPE_MOVE) _tcscpy(to,_T("*"));
    if(receive->type==INFOTYPE_COPY||receive->type==INFOTYPE_ATTR) _tcscpy(from,_T("*"));
    for(unsigned int i=0;i<sizeofa(panels);i++)
    {
      if(!_tcsicmp(panels[i],from)||!_tcsicmp(panels[i],to))
      {
        Info.ControlShort3(panel[i],update[i],(FIRST_PARAM)1);
        Info.ControlShort(panel[i],redraw[i],0);
      }
    }
  }
}

void SendCommand(DWORD ThreadId,DWORD Command)
{
  DWORD send[3]={OPERATION_INFO,Command,ThreadId};
  DWORD Transfered;
  HANDLE hPipe=CreateFile(PIPE_NAMEP,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
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
  HANDLE hPipe=CreateFile(PIPE_NAMEP,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
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
      const TCHAR *MsgItems[]={GetMsg(mName),GetMsg(mAbortText),GetMsg(mInfoDlgStop),GetMsg(mInfoDlgContinue)};
      if(Info.Message(Info.ModuleNumber,FMSG_WARNING,_T("Abort"),MsgItems,sizeofa(MsgItems),2)) process=false;
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
