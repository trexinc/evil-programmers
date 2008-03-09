#include <stdio.h>
#include "../../plugin.hpp"
#include "../../farkeys.hpp"
#include "../../farcolor.hpp"
#include "bcplugin.h"
#include "memory.h"
#include "bcopy_fast_redraw.h"

static bool GetJobList(DWORD *size,SmallInfoRec **receive)
{
  *size=0; *receive=NULL;
  bool res=false;
  {
    DWORD send[2]={OPERATION_INFO,INFOFLAG_ALL};
    DWORD dwBytesRead,dwBytesWritten,rec_size;
    HANDLE hPipe=CreateFile(PIPE_NAME,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
    if(hPipe!=INVALID_HANDLE_VALUE)
    {
      if(WriteFile(hPipe,send,sizeof(send),&dwBytesWritten,NULL))
        if(ReadFile(hPipe,size,sizeof(*size),&dwBytesRead,NULL))
          if(ReadFile(hPipe,&rec_size,sizeof(rec_size),&dwBytesRead,NULL)) //FIXME: get size from pipe
          {
            res=true;
            if(*size)
            {
              *receive=(SmallInfoRec *)malloc(sizeof(SmallInfoRec)*(*size));
              if((!(*receive))||(!ReadFile(hPipe,*receive,sizeof(SmallInfoRec)*(*size),&dwBytesRead,NULL)))
              {
                *size=0;
                free(*receive);
                *receive=NULL;
                res=false;
              }
            }
          }
      CloseHandle(hPipe);
    }
  }
  return res;
}

struct InfoMenuData
{
  BasicDialogData CommonData;
  DWORD count;
  SmallInfoRec *id;
  bool forcerefresh;
  bool firsttime;
};

static void UpdateItem(FarListItem *item,SmallInfoRec *data,int width)
{
  const int InfoMsgs[]={0,mInfoCopy,mInfoMove,mInfoWipe,mInfoDel,mInfoAttr};
  char SrcA[2*MAX_PATH],percent[10];
  WideCharToMultiByte(CP_OEMCP,0,data->Src,-1,SrcA,sizeofa(SrcA),NULL,NULL);
  sprintf(percent,"%3ld",data->percent); percent[3]=0;
  strcpy(item->Text,GetMsg(InfoMsgs[data->type]));
  strcat(item->Text,GetMsg(mInfoSep));
  strcat(item->Text,percent);
  strcat(item->Text,"%");
  strcat(item->Text,GetMsg(mInfoSep));
  strcat(item->Text,FSF.TruncPathStr(SrcA,width));
  if(data->wait)
  {
    if(data->pause)
      item->Flags=LIF_CHECKED|'w';
    else
      item->Flags=LIF_CHECKED|'W';
  }
  else if(data->Ask)
    item->Flags=LIF_CHECKED|'?';
  else if(data->pause)
    item->Flags=0;
  else
    item->Flags=LIF_CHECKED|'*';
}

static int WidthToPathWidth(int width)
{
  return width-strlen(GetMsg(mInfoCopy))-12-(2*strlen(GetMsg(mInfoSep))-1+4);
}

static long WINAPI InfoMenuProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  InfoMenuData *DlgParams=(InfoMenuData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_DRAGGED:
      return FALSE;
    case DN_CTLCOLORDIALOG:
      return Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUTEXT);
    case DN_CTLCOLORDLGLIST:
      if(Param1==0)
      {
        FarListColors *Colors=(FarListColors *)Param2;
        int ColorIndex[]={COL_MENUTEXT,COL_MENUTEXT,COL_MENUTITLE,COL_MENUTEXT,COL_MENUHIGHLIGHT,COL_MENUTEXT,COL_MENUSELECTEDTEXT,COL_MENUSELECTEDHIGHLIGHT,COL_MENUSCROLLBAR,COL_MENUDISABLEDTEXT};
        int Count=sizeofa(ColorIndex);
        if(Count>Colors->ColorCount) Count=Colors->ColorCount;
        for(int i=0;i<Count;i++)
          Colors->Colors[i]=Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)(ColorIndex[i]));
        return TRUE;
      }
      break;
    case DN_INITDIALOG:
      {
        FarListTitles titles;
        titles.Title=(char *)GetMsg(mName);
        titles.TitleLen=strlen(titles.Title);
        titles.Bottom=(char *)GetMsg(mInfoBottom);
        titles.BottomLen=strlen(titles.Bottom);
        Info.SendDlgMessage(hDlg,DM_LISTSETTITLES,0,(long)&titles);
      }
      break;
    case DN_DRAWDIALOG:
      if(DlgParams->firsttime)
      {
        Info.SendDlgMessage(hDlg,DN_TIMER,0,0);
        Info.SendDlgMessage(hDlg,DN_TIMER,0,0);
      }
      break;
    case DN_TIMER:
      {
        HANDLE console=CreateFileW(L"CONOUT$",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
        if(console!=INVALID_HANDLE_VALUE)
        {
          DWORD size,height=0,width=0,path_width,length; SmallInfoRec *receive;
          if(GetJobList(&size,&receive)||DlgParams->firsttime)
          {
            bool refresh=true;
            if(!DlgParams->forcerefresh)
            {
              if(DlgParams->count==size)
              {
                refresh=false;
                for(DWORD i=0;i<size;i++)
                  if(DlgParams->id[i].ThreadId!=receive[i].ThreadId)
                  {
                    refresh=true;
                    break;
                  }
              }
            }
            if(refresh)
            {
              DWORD old_pos=0; bool restore_pos=false;
              if(DlgParams->count)
              {
                DWORD pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,(long)NULL);
                if(pos<DlgParams->count)
                {
                  old_pos=DlgParams->id[pos].ThreadId;
                  restore_pos=true;
                }
              }
              free(DlgParams->id);
              DlgParams->id=receive;
              DlgParams->count=size;
              Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
              for(DWORD i=0;i<size;i++)
              {
                length=wcslen(receive[i].Src);
                if(length>width) width=length;
              }
              width+=strlen(GetMsg(mInfoCopy));
              width+=2*strlen(GetMsg(mInfoSep))-1+4; // -1 for headers, 4 for percents
              length=strlen(GetMsg(mName));
              if(length>width) width=length;
              length=strlen(GetMsg(mInfoBottom));
              if(length>width) width=length;
              width+=12;
              height=size+4;
              { //normalize according console size
                CONSOLE_SCREEN_BUFFER_INFO console_info;
                GetConsoleScreenBufferInfo(console,&console_info);
                if(width>(DWORD)(console_info.dwSize.X-4))
                  width=console_info.dwSize.X-4;
                if(height>(DWORD)(console_info.dwSize.Y-2))
                  height=console_info.dwSize.Y-2;
              }
              //calculate path width
              path_width=WidthToPathWidth(width);
              { //minimize listbox
                SMALL_RECT listbox_size={3,1,4,2};
                Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,0,(long)&listbox_size);
              }
              { //resize and move dialog
                COORD dialog_size={width,height};
                Info.SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(long)&dialog_size);
                COORD position={-1,-1};
                Info.SendDlgMessage(hDlg,DM_MOVEDIALOG,TRUE,(long)&position);
              }
              { //resize listbox
                SMALL_RECT listbox_size={3,1,width-4,height-2};
                Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,0,(long)&listbox_size);
              }
              { //refresh listbox
                FarListPos new_pos={0,-1};
                FarListDelete clear={0,0};
                FarListItem *list_items=(FarListItem *)malloc(sizeof(FarListItem)*size);
                FarList list={size,list_items};
                if(list_items)
                {
                  Info.SendDlgMessage(hDlg,DM_LISTDELETE,0,(long)&clear);
                  for(DWORD i=0;i<size;i++)
                  {
                    UpdateItem(list_items+i,receive+i,path_width);
                    if(restore_pos&&receive[i].ThreadId==old_pos) new_pos.SelectPos=i;
                  }
                  Info.SendDlgMessage(hDlg,DM_LISTADD,0,(long)&list);
                  free(list_items);
                  if(restore_pos) Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,(long)&new_pos);
                }
              }
              Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
              if(DlgParams->firsttime)
              {
                DlgParams->firsttime=false;
                if((PlgOpt.InfoMenu&INFO_MENU_ALT_1)&&(DlgParams->count==1))
                  Info.SendDlgMessage(hDlg,DM_CLOSE,-1,0);
              }
              else
              {
                DlgParams->forcerefresh=false;
                if((DlgParams->count==0)&&(!(PlgOpt.InfoMenu&INFO_MENU_ALT_0)))
                  Info.SendDlgMessage(hDlg,DM_CLOSE,-1,0);
              }
            }
            else
            {
              SMALL_RECT in;
              Info.SendDlgMessage(hDlg,DM_GETDLGRECT,0,(long)&in);
              path_width=WidthToPathWidth(in.Right-in.Left+1);
              for(DWORD i=0;i<size;i++)
              {
                if(receive[i].pause!=DlgParams->id[i].pause||receive[i].Ask!=DlgParams->id[i].Ask||receive[i].percent!=DlgParams->id[i].percent)
                {
                  FarListGetItem item;
                  item.ItemIndex=i;
                  Info.SendDlgMessage(hDlg,DM_LISTGETITEM,0,(long)&item);
                  UpdateItem(&(item.Item),receive+i,path_width);
                  Info.SendDlgMessage(hDlg,DM_LISTUPDATE,0,(long)&item);
                }
              }
              free(DlgParams->id);
              DlgParams->id=receive;
            }
          }
          CloseHandle(console);
        }
      }
      break;
    case DN_RESIZECONSOLE:
      DlgParams->forcerefresh=true;
      Info.SendDlgMessage(hDlg,DN_TIMER,0,0);
      break;
    case DN_CLOSE:
      {
        DWORD pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
        if((Param1==0)&&(DlgParams->id)&&(pos<DlgParams->count))
        {
          Info.SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
          SuspendThread(DlgParams->CommonData.Thread);
          ShowInfoDialog(DlgParams->id+pos);
          ResumeThread(DlgParams->CommonData.Thread);
          Info.SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
          Info.SendDlgMessage(hDlg,DN_ENTERIDLE,0,0);
          return FALSE;
        }
        else
          Redraw_Close(DlgParams->CommonData.Thread);
      }
      break;
  }
  return FastRedrawDefDlgProc(hDlg,Msg,Param1,Param2);
}

static long WINAPI InfoMenuDialogKeyProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  (void)Param1;
  InfoMenuData *DlgParams=(InfoMenuData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  if(Msg==DN_KEY)
  {
    if(((Param2==KEY_SPACE)||(Param2==KEY_DEL))&&DlgParams->count)
    {
      DWORD pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
      if(pos<DlgParams->count)
        switch(Param2)
        {
          case KEY_SPACE:
            SendCommand(DlgParams->id[pos].ThreadId,INFOFLAG_PAUSE);
            break;
          case KEY_DEL:
            AbortThread(DlgParams->id[pos].ThreadId);
            break;
        }
    }
  }
  return 0;
}

void WINAPI _export ShowInfoMenu(void)
{
  FarDialogItem DialogItems[1];
  memset(DialogItems,0,sizeof(DialogItems));
  DialogItems[0].Type=DI_LISTBOX; DialogItems[0].Flags=DIF_LISTWRAPMODE;
  ThreadData Thread={FALSE,NULL,NULL,NULL,PlgOpt.RefreshInterval};
  InitThreadData(&Thread);
  InfoMenuData params={{&Thread,0,{-1,-1},{L"",L""},INVALID_HANDLE_VALUE,false,InfoMenuDialogKeyProc,MACRO_INFO_MENU,true,false},0,NULL,true,true};
  Info.DialogEx(Info.ModuleNumber,-1,-1,0,0,"InfoMenu",DialogItems,sizeofa(DialogItems),0,0,InfoMenuProc,(DWORD)&params);
  FreeThreadData(&Thread);
  free(params.id);
}
