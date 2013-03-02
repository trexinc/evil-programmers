#include <stdio.h>
#include <stdlib.h>
#include "far_helper.h"
#include "farcolor.hpp"
#include "evplugin.h"
#include "memory.h"
#include "PluginSettings.hpp"

#include <initguid.h>
// {D888455B-4E68-43f8-8D54-9A96913974B2}
DEFINE_GUID(MainGuid, 0xd888455b, 0x4e68, 0x43f8, 0x8d, 0x54, 0x9a, 0x96, 0x91, 0x39, 0x74, 0xb2);
// {E08DD543-15F9-4f3d-9517-2C443ADC1E4F}
DEFINE_GUID(DiskMenuGuid, 0xe08dd543, 0x15f9, 0x4f3d, 0x95, 0x17, 0x2c, 0x44, 0x3a, 0xdc, 0x1e, 0x4f);
// {0FC3E83E-0F0D-47c9-87C5-11E13F5C5100}
DEFINE_GUID(PluginMenuGuid, 0xfc3e83e, 0xf0d, 0x47c9, 0x87, 0xc5, 0x11, 0xe1, 0x3f, 0x5c, 0x51, 0x0);
// {BFDA5B9F-3C7E-4c62-9B89-6BC872E89037}
DEFINE_GUID(ConfigMenuGuid, 0xbfda5b9f, 0x3c7e, 0x4c62, 0x9b, 0x89, 0x6b, 0xc8, 0x72, 0xe8, 0x90, 0x37);
// {1A001727-EAEE-4bf9-893C-0FB4246FC483}
DEFINE_GUID(ConfigGuid, 0x1a001727, 0xeaee, 0x4bf9, 0x89, 0x3c, 0xf, 0xb4, 0x24, 0x6f, 0xc4, 0x83);

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

struct Options {
  BOOL AddToDisksMenu;
  BOOL AddToPluginsMenu;
  BOOL AddToConfigMenu;
  BOOL BrowseEvtFiles;
  BOOL StripExt;
  BOOL Restore;
  DWORD ScanType;
  wchar_t Prefix[16];
} Opt;

struct QVOptions {
  BOOL ShowHeader;
  BOOL ShowDescription;
  BOOL ShowData;
} QVOpt;

struct TechOptions
{
  wchar_t Separator[1024];
  BOOL NetBackup;
} TechOpt;

struct PluginState
{
  wchar_t Path[MAX_PATH];
  wchar_t Computer[MAX_PATH];
  int Current;
  int Top;
  int ViewMode;
  int SortMode;
  int SortOrder;
} State;

static const wchar_t *GetMsg(int MsgId)
{
  return Info.GetMsg(&MainGuid,MsgId);
}

static const wchar_t *default_column_data=L"";

static bool CheckRemoteEventLog(wchar_t *computer)
{
#if 0
  HANDLE hSScr=Info.SaveScreen(0,0,-1,-1);
  const wchar_t *MsgItems[]={L"",GetMsg(mOtherConnecting)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,ArraySize(MsgItems),0);
  HANDLE evt=OpenEventLog(computer,L"System"); //REMOTE
  Info.RestoreScreen(hSScr);
  if(evt)
  {
    CloseEventLog(evt);
    return true;
  }
#endif
  return false;
}

#include "el_utils.cpp"
#include "el_config.cpp"

#define DISPLAY_WIN_ERROR \
{ \
  const wchar_t *MsgItems[]={GetMsg(mError),GetMsg(mOk)}; \
  Info.Message(Info.ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,ArraySize(MsgItems),1); \
}

static HANDLE RealOpenFilePlugin(const wchar_t *Name,const unsigned char *Data,int DataSize);

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=MAKEFARVERSION(0,5,0,14,VS_ALPHA);
  Info->Guid=MainGuid;
  Info->Title=L"NT Events";
  Info->Description=L"NT Events";
  Info->Author=L"Vadim Yegorov";
} 

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
  memset(&::Info, 0, sizeof(::Info));
  memmove(&::Info, Info, (Info->StructSize > (int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;

  PluginSettings settings(MainGuid,::Info.SettingsControl);
  //tech options
  Opt.AddToConfigMenu=settings.Get(0,L"AddToConfigMenu",TRUE);
  //main options
  Opt.AddToDisksMenu=settings.Get(0,L"AddToDisksMenu",FALSE);
  Opt.AddToPluginsMenu=settings.Get(0,L"AddToPluginsMenu",TRUE);
  Opt.BrowseEvtFiles=settings.Get(0,L"BrowseEvtFiles",TRUE);
  Opt.StripExt=settings.Get(0,L"StripExt",TRUE);
  Opt.ScanType=settings.Get(0,L"ScanType",0);
  settings.Get(0,L"Prefix",Opt.Prefix,ArraySize(Opt.Prefix),L"evt");
  Opt.Restore=settings.Get(0,L"Restore",TRUE);

  //state options
  settings.Get(0,L"StatePath",State.Path,ArraySize(State.Path),L"");
  settings.Get(0,L"StateComputer",State.Computer,ArraySize(State.Computer),L"");
  State.Current=settings.Get(0,L"StateCurrent",0);
  State.Top=settings.Get(0,L"StateTop",0);
  State.ViewMode=settings.Get(0,L"StateViewMode",-1);
  State.SortMode=settings.Get(0,L"StateSortMode",-1);
  State.SortOrder=settings.Get(0,L"StateSortOrder",-1);

  //QuickView
  QVOpt.ShowHeader=settings.Get(0,L"ShowHeader",FALSE);
  QVOpt.ShowDescription=settings.Get(0,L"ShowDescription",TRUE);
  QVOpt.ShowData=settings.Get(0,L"ShowData",TRUE);

  //Tech
  settings.Get(0,L"Separator",TechOpt.Separator,ArraySize(TechOpt.Separator),L"");
  TechOpt.NetBackup=settings.Get(0,L"NetBackup",FALSE);
  if((Opt.ScanType!=0)&&(Opt.ScanType!=1))
    Opt.ScanType=0;
  if(!Opt.Prefix[0])
    FSF.sprintf(Opt.Prefix,L"%s",L"evt");
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
    Info->StructSize=sizeof(*Info);
    Info->Flags=0;

    static const wchar_t *DisksMenuStrings[1];
    DisksMenuStrings[0]=GetMsg(mNameDisk);
    Info->DiskMenu.Strings=DisksMenuStrings;
    Info->DiskMenu.Count=Opt.AddToDisksMenu?1:0;
		Info->DiskMenu.Guids=&DiskMenuGuid;

    static const wchar_t *PluginMenuStrings[1];
    PluginMenuStrings[0]=GetMsg(mName);
    Info->PluginMenu.Strings=PluginMenuStrings;
    Info->PluginMenu.Count=Opt.AddToPluginsMenu?(ArraySize(PluginMenuStrings)):0;
		Info->PluginMenu.Guids=&PluginMenuGuid;
    Info->PluginConfig.Strings=PluginMenuStrings;
    Info->PluginConfig.Count=Opt.AddToConfigMenu?(ArraySize(PluginMenuStrings)):0;
    Info->PluginConfig.Guids=&ConfigMenuGuid;
    Info->CommandPrefix=Opt.Prefix;
}

HANDLE WINAPI OpenPluginW(int OpenFrom,int Item)
{
#if 0
  EventViewer *panel=(EventViewer *)malloc(sizeof(EventViewer));
  if(!panel)
    return INVALID_HANDLE_VALUE;
  panel->level=0;
  wcscpy(panel->path,L"");
  wcscpy(panel->computer,L"");
  wcscpy(panel->computer_oem,L"");
  panel->computer_ptr=NULL;
  panel->redraw=Opt.Restore;
  if(Opt.Restore)
  {
    wcscpy(panel->path,State.Path);
    if(wcslen(panel->path)) panel->level=1;
    if(wcslen(State.Computer))
    {
      wchar_t temp_computer_name[MAX_PATH]; t_OemToChar(State.Computer,temp_computer_name);
      if(CheckRemoteEventLog(temp_computer_name))
      {
        wcscpy(panel->computer,temp_computer_name);
        wcscpy(panel->computer_oem,State.Computer);
        panel->computer_ptr=panel->computer;
      }
    }
  }
  return (HANDLE)panel;
#endif
  return 0;
}

void WINAPI ClosePluginW(HANDLE hPlugin)
{
#if 0
  free((EventViewer *)hPlugin);
#endif
}

int WINAPI GetFindDataW(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
#if 0
    HANDLE hSScr=Info.SaveScreen(0,0,-1,-1),console=INVALID_HANDLE_VALUE;
    if(!(OpMode&(OPM_FIND)))
    {
      const wchar_t *MsgItems[]={L"",GetMsg(mOtherScanning)};
      Info.Message(Info.ModuleNumber,0,NULL,MsgItems,ArraySize(MsgItems),0);
      console=CreateFile(L"CONIN$",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    }
    EventViewer *panel=(EventViewer *)hPlugin;
    HANDLE evt=INVALID_HANDLE_VALUE; DWORD scan=EVENTLOG_FORWARDS_READ;
    if(Opt.ScanType==1) scan=EVENTLOG_BACKWARDS_READ;
    *pPanelItem=NULL; *pItemsNumber=0;
    wchar_t path_ansi[MAX_PATH];
    t_OemToChar(panel->path,path_ansi);
    switch(panel->level)
    {
      case 0:
      {
        HKEY hKey=NULL;
        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,EVENTLOG_KEY,0,KEY_READ,&hKey)==ERROR_SUCCESS)
        {
          wchar_t NameBuffer[MAX_PATH]; LONG Result;
          for(int i=0;;i++)
          {
            Result=RegEnumKey(hKey,i,NameBuffer,ArraySize(NameBuffer));
            if(Result==ERROR_NO_MORE_ITEMS)
              break;
            if(Result==ERROR_SUCCESS)
            {
              struct PluginPanelItem *pNewPanelItem=(PluginPanelItem *)realloc(*pPanelItem,sizeof(PluginPanelItem)*((*pItemsNumber)+1));
              if(pNewPanelItem)
              {
                *pPanelItem=pNewPanelItem;
                PluginPanelItem *curr=(*pPanelItem)+(*pItemsNumber)++;
                memset(curr,0,sizeof(PluginPanelItem));
                curr->FindData.PANEL_FILENAME=(wchar_t*)malloc((wcslen(NameBuffer)+1)*sizeof(wchar_t));
                if(curr->FindData.PANEL_FILENAME) wcscpy((wchar_t*)curr->FindData.PANEL_FILENAME,NameBuffer);
                curr->FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
              }
              else
              {
                free(*pPanelItem);
                *pPanelItem=NULL;
                *pItemsNumber=0;
                break;
              }
            }
          }
          RegCloseKey(hKey);
        }
        break;
      }
      case 1:
        evt=OpenEventLog(panel->computer_ptr,path_ansi); //REMOTE
      case 2:
        if(evt==INVALID_HANDLE_VALUE)
          evt=OpenBackupEventLog(NULL,path_ansi); //LOCAL
        if(evt)
        {
          if(GetNumberOfEventLogRecords(evt,(DWORD *)pItemsNumber))
          {
            (*pItemsNumber)++;
            *pPanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*(*pItemsNumber));
            if(*pPanelItem)
            {
              memset(*pPanelItem,0,sizeof(PluginPanelItem)*(*pItemsNumber));
              (*pPanelItem)->FindData.PANEL_FILENAME=(wchar_t*)malloc(3*sizeof(wchar_t));
              wcscpy((wchar_t*)((*pPanelItem)->FindData.PANEL_FILENAME),L"..");
              wchar_t **CustomColumnData=(wchar_t **)malloc(sizeof(wchar_t *)*CUSTOM_COLUMN_COUNT);
              if(CustomColumnData)
              {
                CustomColumnData[0]=(wchar_t *)malloc(sizeof(wchar_t));
                CustomColumnData[1]=(wchar_t *)malloc(sizeof(wchar_t));
                CustomColumnData[2]=(wchar_t *)malloc(sizeof(wchar_t)*3);
                if(CustomColumnData[2])
                  FSF.sprintf(CustomColumnData[2],L"%s",L"..");
                CustomColumnData[3]=(wchar_t *)malloc(sizeof(wchar_t));
                CustomColumnData[4]=(wchar_t *)malloc(sizeof(wchar_t));
                CustomColumnData[5]=L"";
                for(int i=0;i<(CUSTOM_COLUMN_COUNT-1);i++)
                  if(!CustomColumnData[i])
                    CustomColumnData[i]=(wchar_t*)default_column_data;
                (*pPanelItem)->CustomColumnNumber=CUSTOM_COLUMN_COUNT;
              }
              (*pPanelItem)->CustomColumnData=CustomColumnData;
              for(int i=1;i<(*pItemsNumber);i++)
              {
                ((*pPanelItem)+i)->FindData.PANEL_FILENAME=(wchar_t*)malloc(512*sizeof(wchar_t));
                FSF.sprintf((wchar_t*)(((*pPanelItem)+i)->FindData.PANEL_FILENAME),L"%s.%s.%s",GetMsg(mExtNameError),GetMsg(mExtError),GetMsg(mExtMain));
              }

              unsigned long long evt_date_time,evt_date_time_local;
              void *user_data; const wchar_t *suffix;
              //read events
              EVENTLOGRECORD *curr_rec;
              wchar_t *buff=NULL;
              DWORD buffsize=BIG_BUFFER,readed,needed;
              buff=(wchar_t *)malloc(buffsize);
              if(buff)
              {
                bool work=true;
                for(int i=1;(i<(*pItemsNumber))&&work;)
                {
                  if(!ReadEventLog(evt,scan|EVENTLOG_SEQUENTIAL_READ,0,buff,buffsize,&readed,&needed))
                  {
                    if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
                    {
                      free(buff);
                      buffsize=needed;
                      buff=(wchar_t *)malloc(buffsize);
                      if(buff) continue;
                    }
                    else if(!(OpMode&(OPM_FIND)))
                      DISPLAY_WIN_ERROR
                    break;
                  }
                  curr_rec=(EVENTLOGRECORD *)buff;
                  while(readed&&work)
                  {
                    user_data=malloc(curr_rec->Length);
                    if(user_data)
                    {
                      ((*pPanelItem)+i)->Flags=PPIF_USERDATA;
                      memcpy(user_data,curr_rec,curr_rec->Length);
                      ((*pPanelItem)+i)->UserData=(DWORD)user_data;
                      wchar_t *description=(wchar_t *)(((EVENTLOGRECORD *)user_data)+1);
                      wchar_t* Description=(wchar_t *)malloc((wcslen(description)+1)*sizeof(wchar_t));
                      if(Description)
                      {
                        FSF.sprintf(Description,L"%s",description);
                      }
                      ((*pPanelItem)+i)->Description=Description;
                    }
                    suffix=L"";
                    switch(curr_rec->EventType)
                    {
                      case EVENTLOG_ERROR_TYPE:
                        suffix=GetMsg(mExtError);
                        break;
                      case EVENTLOG_WARNING_TYPE:
                        suffix=GetMsg(mExtWarning);
                        break;
                      case EVENTLOG_INFORMATION_TYPE:
                        suffix=GetMsg(mExtInformation);
                        break;
                      case EVENTLOG_AUDIT_SUCCESS:
                        suffix=GetMsg(mExtAuditSuccess);
                        break;
                      case EVENTLOG_AUDIT_FAILURE:
                        suffix=GetMsg(mExtAuditFailure);
                        break;
                    }
                    ((*pPanelItem)+i)->FindData.PANEL_FILENAME=(wchar_t*)malloc(512*sizeof(wchar_t));
                    FSF.sprintf((wchar_t*)(((*pPanelItem)+i)->FindData.PANEL_FILENAME),L"%08ld.%s.%s",curr_rec->RecordNumber,suffix,GetMsg(mExtMain));
                    ((*pPanelItem)+i)->FindData.nFileSize=curr_rec->Length;
                    evt_date_time=curr_rec->TimeWritten; evt_date_time=evt_date_time*10000000ULL+EVENT_START_TIME;
  //                  FileTimeToLocalFileTime((FILETIME *)&evt_date_time,(FILETIME *)&evt_date_time_local);
                    memcpy(&(((*pPanelItem)+i)->FindData.ftLastWriteTime),&evt_date_time,sizeof(evt_date_time));
                    evt_date_time=curr_rec->TimeGenerated; evt_date_time=evt_date_time*10000000ULL+EVENT_START_TIME;
                    memcpy(&(((*pPanelItem)+i)->FindData.ftCreationTime),&evt_date_time,sizeof(evt_date_time));
                    FileTimeToLocalFileTime((FILETIME *)&evt_date_time,(FILETIME *)&evt_date_time_local);
                    CustomColumnData=(wchar_t **)malloc(sizeof(wchar_t *)*CUSTOM_COLUMN_COUNT);
                    if(CustomColumnData)
                    {
                      CustomColumnData[0]=(wchar_t *)malloc(sizeof(wchar_t)*6);
                      if(CustomColumnData[0])
                        FSF.sprintf(CustomColumnData[0],L"%5ld",curr_rec->EventID&0xffff);
                      const wchar_t *category=GetCategory(curr_rec);
                      CustomColumnData[1]=(wchar_t *)malloc((wcslen(category)+1)*sizeof(wchar_t));
                      if(CustomColumnData[1])
                        wcscpy(CustomColumnData[1],category);
                      CustomColumnData[2]=(wchar_t *)malloc(sizeof(wchar_t)*20);
                      if(CustomColumnData[2])
                      {
                        SYSTEMTIME time;
                        FileTimeToSystemTime((FILETIME *)&evt_date_time_local,&time);
                        FSF.sprintf(CustomColumnData[2],L"%02d.%02d.%04d %02d:%02d:%02d",time.wDay,time.wMonth,time.wYear,time.wHour,time.wMinute,time.wSecond);
                      }
                      wchar_t *compname=GetComputerName(curr_rec);
                      CustomColumnData[3]=(wchar_t *)malloc(sizeof(wchar_t)*(wcslen(compname)+1));
                      if(CustomColumnData[3])
                        wcscpy(CustomColumnData[3],compname);
                      const wchar_t *username=GetUserName(panel->computer_ptr,curr_rec);
                      CustomColumnData[4]=(wchar_t *)malloc(sizeof(wchar_t)*(wcslen(username)+1));
                      if(CustomColumnData[4])
                        wcscpy(CustomColumnData[4],username);
                      CustomColumnData[5]=CustomColumnData[2];
                      for(int j=0;j<(CUSTOM_COLUMN_COUNT-1);j++)
                        if(!CustomColumnData[j])
                          CustomColumnData[j]=(wchar_t*)default_column_data;
                      ((*pPanelItem)+i)->CustomColumnNumber=CUSTOM_COLUMN_COUNT;
                    }
                    ((*pPanelItem)+i)->CustomColumnData=CustomColumnData;
                    readed-=curr_rec->Length;
                    curr_rec=(EVENTLOGRECORD *)((char*)curr_rec+curr_rec->Length);
                    i++;
                    if(!(OpMode&(OPM_FIND)))
                    {
                      INPUT_RECORD *InputRec;
                      DWORD NumberOfEvents,ReadCnt;
                      if(console!=INVALID_HANDLE_VALUE)
                      {
                        if(GetNumberOfConsoleInputEvents(console,&NumberOfEvents))
                        {
                          if(NumberOfEvents)
                            if((InputRec=(INPUT_RECORD *)malloc(NumberOfEvents*sizeof(INPUT_RECORD)))!=NULL)
                            {
                              if(PeekConsoleInput(console,InputRec,NumberOfEvents,&ReadCnt))
                                for(unsigned long j=0;j<NumberOfEvents;j++)
                                {
                                  if((InputRec[j].EventType==KEY_EVENT)&&(InputRec[j].Event.KeyEvent.bKeyDown)&&(InputRec[j].Event.KeyEvent.wVirtualKeyCode==VK_ESCAPE))
                                  {
                                    FlushConsoleInputBuffer(console);
                                    work=false;
                                    break;
                                  }
                                }
                              free(InputRec);
                            }
                        }
                      }
                    }
                  }
                }
                free(buff);
              }
            }
            else
              *pItemsNumber=0;
          }
          else if(!(OpMode&(OPM_FIND)))
          {
            DISPLAY_WIN_ERROR
            *pItemsNumber=0;
          }
          CloseEventLog(evt);
        }
        else if(!(OpMode&(OPM_FIND)))
          DISPLAY_WIN_ERROR
        break;
    }
    if(!(OpMode&(OPM_FIND)))
      CloseHandle(console);
    Info.RestoreScreen(hSScr);
#endif
    return TRUE;
}

void WINAPI FreeFindDataW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
#if 0
  for(int i=0;i<ItemsNumber;i++)
  {
    free((void*)PanelItem[i].UserData);
    free((void*)PanelItem[i].Owner);
    free((void*)PanelItem[i].Description);
    if(PanelItem[i].CustomColumnData)
      for(int j=0;j<(CUSTOM_COLUMN_COUNT-1);j++)
        if(PanelItem[i].CustomColumnData[j]!=(wchar_t*)default_column_data)
          free((void*)PanelItem[i].CustomColumnData[j]);
    free((void*)PanelItem[i].CustomColumnData);
    free((void*)PanelItem[i].FindData.lpwszFileName);
  }
  free(PanelItem);
#endif
}

int WINAPI SetDirectoryW(HANDLE hPlugin,const wchar_t *Dir,int OpMode)
{
#if 0
  int res=TRUE;
  EventViewer *panel=(EventViewer *)hPlugin;
  if(panel->level==2)
    return TRUE;
  if(!wcscmp(Dir,L"\\"))
  {
    panel->level=0;
    wcscpy(panel->path,L"");
  }
  else if(!wcscmp(Dir,L".."))
  {
    if(panel->level==1)
    {
      panel->level=0;
      wcscpy(panel->path,L"");
    }
    else res=FALSE;
  }
  else if((panel->level==0)&&(CheckLogName(panel,Dir)))
  {
    panel->level=1;
    wcscpy(panel->path,Dir);
  }
  else res=FALSE;
  return res;
#endif
  return false;
}

static long WINAPI CopyDialogProc(HANDLE hDlg, int Msg,int Param1,long Param2)
{
#if 0
  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_SETTEXTLENGTH,2,MAX_PATH-1);
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
#endif
  return true;
}

static int NumberType(int num)
{
  int Result=0,mod;
  mod=num%100;
  if((mod>20)||(mod<5))
  {
    mod=num%10;
    if((mod<5)&&(mod>0)) Result--;
    if(mod==1) Result--;
  }
  return Result;
}

static BOOL GetDestDir(wchar_t *dir,int move)
{
  /*
    0000000000111111111122222222223333333333444444444455555555556666666666777777
    0123456789012345678901234567890123456789012345678901234567890123456789012345
  00                                                                            00
  01   ษอออออออออออออออออออออออออออออออ Copy อออออออออออออออออออออออออออออออป   01
  02   บ Copy "todo" to                                                     บ   02
  03   บ D:\00000005.DIR\c\far.plug\EventViewer\                           |บ   03
  04   วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ   04
  05   บ                 [ Copy ]                [ Cancel ]                 บ   05
  06   ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ   06
  07                                                                            07
    0000000000111111111122222222223333333333444444444455555555556666666666777777
    0123456789012345678901234567890123456789012345678901234567890123456789012345
  */
#if 0
  static const wchar_t *NTEventCopyHistoryName=L"NTEventCopy";
  static struct InitDialogItem InitDlg[]={
  /*0*/  {DI_DOUBLEBOX,3,1,72,6,0,0,0,0,L""},
  /*1*/  {DI_TEXT,5,2,0,0,0,0,0,0,L""},
  /*2*/  {DI_EDIT,5,3,70,0,1,(DWORD)NTEventCopyHistoryName,DIF_HISTORY,0,L""},
  /*3*/  {DI_TEXT,5,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L""},
  /*4*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,1,L""},
  /*5*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,0,(wchar_t *)mCpyDlgCancel}
  };
  struct FarDialogItem DialogItems[ArraySize(InitDlg)];
  InitDialogItems(InitDlg,DialogItems,ArraySize(InitDlg));
  PanelInfo PInfo;
  Info.ControlShort(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,(SECOND_PARAM)&PInfo); //FIXME
  INIT_DLG_DATA(DialogItems[0],GetMsg(mCpyDlgCopyTitle+move));
  wchar_t CopyText[512];
  FSF.sprintf(CopyText,GetMsg(mCpyDlgCopyToN+3*move+NumberType(PInfo.SelectedItemsNumber)),PInfo.SelectedItemsNumber);
  DialogItems[1].PtrData=CopyText;
  INIT_DLG_DATA(DialogItems[2],dir);
  INIT_DLG_DATA(DialogItems[4],GetMsg(mCpyDlgCopyOk+move));
  CFarDialog dialog;
  int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,76,8,NULL,DialogItems,(ArraySize(InitDlg)),0,0,CopyDialogProc,0);
  if(DlgCode==4)
  {
    FSF.sprintf(dir,L"%s",dialog.Str(2));
    return TRUE;
  }
  else
    return FALSE;
#endif
  return false;
}

static void GetFileAttr(wchar_t *file,unsigned long long *size,SYSTEMTIME *mod)
{
#if 0
  *size=0;
  memset(mod,0,sizeof(SYSTEMTIME));
  WIN32_FIND_DATA find; HANDLE hFind;
  hFind=FindFirstFile(file,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
    *size=(unsigned long long)find.nFileSizeLow+(unsigned long long)find.nFileSizeHigh*4294967296ULL;
    FILETIME local;
    FileTimeToLocalFileTime(&find.ftLastWriteTime,&local);
    FileTimeToSystemTime(&local,mod);
  }
#endif
}

static int CheckRetry(const wchar_t *afrom,const wchar_t *ato)
{
#if 0
  wchar_t from[512],to[512],buff[MAX_PATH];
  FSF.sprintf(buff,L"%s",afrom);
  FSF.TruncPathStr(buff,55);
  FSF.sprintf(from,GetMsg(mRetryFrom),buff);
  FSF.sprintf(buff,L"%s",ato);
  FSF.TruncPathStr(buff,55);
  FSF.sprintf(to,GetMsg(mRetryTo),buff);
  const wchar_t *MsgItems[]={GetMsg(mRetryError),from,to,GetMsg(mRetryRetry),GetMsg(mRetrySkip),GetMsg(mRetryCancel)};
  return Info.Message(Info.ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,ArraySize(MsgItems),3);
#endif
}

static long WINAPI FileExistsDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
#if 0
  switch(Msg)
  {
    case DN_CTLCOLORDIALOG:
      return Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGTEXT);
    case DN_CTLCOLORDLGITEM:
      switch(Param1)
      {
        case 0:
          return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOX)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTBOXTITLE)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOXTITLE));
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
          return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOX)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTTEXT)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGTEXT));
        case 7:
        case 8:
        case 9:
        case 10:
          if(Info.SendDlgMessage(hDlg,DM_GETFOCUS,0,0)==Param1)
            return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGSELECTEDBUTTON));
          else
            return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTBUTTON)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBUTTON));
      }
    case DN_ENTERIDLE:
    {
      FarDialogItemData Caption;
      wchar_t Buff[512]; SYSTEMTIME mod;
      GetLocalTime(&mod);
      FSF.sprintf(Buff,GetMsg(mExistSource),0ULL,mod.wDay,mod.wMonth,mod.wYear,mod.wHour,mod.wMinute,mod.wSecond);
      Caption.PtrData=Buff;
      Caption.PtrLength=wcslen(Caption.PtrData);
      Info.SendDlgMessage(hDlg,DM_SETTEXT,3,(long)&Caption);
      //refresh
      Info.SendDlgMessage(hDlg,DM_SETREDRAW,0,0);
      break;
    }
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
#endif
}


#define _D(p) (*p)

int WINAPI GetFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t **DestPath,int OpMode)
{
#if 0
  EventViewer *panel=(EventViewer *)hPlugin;
  int OpMode2=OpMode&(~OPM_TOPLEVEL);
  if((!panel->level)&&(panel->computer_ptr)&&(!TechOpt.NetBackup)) return 0; //no remote backup
  static wchar_t DestPathReal[MAX_PATH];
  wcscpy(DestPathReal,*DestPath);
  *DestPath=DestPathReal;

  wchar_t Dir[MAX_PATH],Dest[MAX_PATH];
  int result=1;
  FSF.sprintf(Dir,L"%s",_D(DestPath));
  FSF.AddEndSlash(Dir);
  if(!OpMode2)
  {
    if(!GetDestDir(Dir,Move?1:0))
      return -1;
    wchar_t *filename;
    filename=wcsrchr(Dir,'\\');
    if(!filename)
      filename=Dir;
    if(!(wcscmp(filename,L".")&&wcscmp(filename,L"..")))
    {
      wcscat(Dir,L"\\");
    }
    if(!GetFullPathName(Dir,ArraySize(Dest),Dest,&filename))
    {
      if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
      {
        wchar_t err1[512];
        FSF.sprintf(err1,GetMsg(mErr1),Dir);
        const wchar_t *MsgItems[]={GetMsg(mError),err1,GetMsg(mOk)};
        Info.Message(Info.ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,ArraySize(MsgItems),1);
      }
      return 0;
    }
    if(Dest[wcslen(Dest)-1]!='\\')
    {
      WIN32_FIND_DATA find;
      HANDLE hFind=FindFirstFile(Dest,&find);
      if(hFind!=INVALID_HANDLE_VALUE)
      {
        FindClose(hFind);
        if(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
        {
          wcscat(Dest,L"\\");
        }
      }
    }
  }
  else
  {
    FSF.sprintf(Dest,L"%s",_D(DestPath));
    FSF.AddEndSlash(Dest);
  }
  BOOL IsDir=(Dest[wcslen(Dest)-1]=='\\');
  wchar_t filename[MAX_PATH]; HANDLE file; DWORD transferred;
  FSF.AddEndSlash((wchar_t*)_D(DestPath));
  if(panel->level)
  {
    BOOL FlagAll=FALSE; BOOL ActionType=atNone;
    wchar_t copyname[56],progress[56]; HANDLE screen=NULL,console=INVALID_HANDLE_VALUE;
    const wchar_t *Items[]={GetMsg(mCpyDlgCopyTitle),copyname,progress};
    if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
    {
      screen=Info.SaveScreen(0,0,-1,-1);
      console=CreateFile(L"CONIN$",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    }
    bool firsttime=true;
    for(int i=0;i<ItemsNumber;i++)
    {
      if(!(PanelItem[i].Flags&PPIF_USERDATA)) continue;
      EVENTLOGRECORD *curr_rec=(EVENTLOGRECORD *)(PanelItem[i].UserData);
      if(IsDir)
      {
        if(OpMode2||(!Opt.StripExt))
          FSF.sprintf(filename,L"%s%s",Dest,PanelItem[i].FindData.PANEL_FILENAME);
        else
          FSF.sprintf(filename,L"%s%08ld.%s",Dest,curr_rec->RecordNumber,GetMsg(mExtMain));
      }
      else
        FSF.sprintf(filename,L"%s",Dest);
retry_main:
      file=CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN|(OpMode&OPM_EDIT?FILE_ATTRIBUTE_READONLY:0),NULL); //FIXME
      if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
      {
        if((file==INVALID_HANDLE_VALUE)&&(GetLastError()==ERROR_FILE_EXISTS))
        {
          if(!FlagAll)
          {
            /*
              000000000011111111112222222222333333333344444444445555555555666666666
              012345678901234567890123456789012345678901234567890123456789012345678
            00                                                                     00
            01   ษอออออออออออออออ Destination already exists ออออออออออออออออออป   01
            02   บ                            1                                บ   02
            03   วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ   03
            04   บ Source                              0  27.06.2001  13:53:55 บ   04
            05   บ Destination      12345678901234567890  27.06.2001  13:53:55 บ   05
            06   วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ   06
            07   บ [ ] Accept choice for all files                             บ   07
            08   บ                                                             บ   08
            09   บ     [ Overwrite ]  [ Skip ]  [ Append ]  [ Cancel ]         บ   09
            10   ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ   10
            11                                                                     11
              000000000011111111112222222222333333333344444444445555555555666666666
              012345678901234567890123456789012345678901234567890123456789012345678
            */
            static struct InitDialogItem InitItems[]={
            /* 0*/  {DI_DOUBLEBOX,3,1,65,10,0,0,0,0,(wchar_t *)mExistDestAlreadyExists},
            /* 1*/  {DI_TEXT,5,2,0,0,0,0,0,0,L""},
            /* 2*/  {DI_TEXT,-1,3,0,0,0,0,DIF_SEPARATOR,0,L""},
            /* 3*/  {DI_TEXT,5,4,0,0,0,0,0,0,L""},
            /* 4*/  {DI_TEXT,5,5,0,0,0,0,0,0,L""},
            /* 5*/  {DI_TEXT,-1,6,0,0,0,0,DIF_SEPARATOR,0,L""},
            /* 6*/  {DI_CHECKBOX,5,7,0,0,1,0,0,0,(wchar_t *)mExistAcceptChoice},
            /* 7*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,1,(wchar_t *)mExistOverwrite},
            /* 8*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(wchar_t *)mExistSkip},
            /* 9*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(wchar_t *)mExistAppend},
            /*10*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(wchar_t *)mExistCancel},
            };
            struct FarDialogItem DialogItems[ArraySize(InitItems)];
            InitDialogItems(InitItems,DialogItems,ArraySize(InitItems));
            wchar_t FilenameText[512],SrcText[512],DstText[512];
            FSF.sprintf(FilenameText,L"%s",filename);
            FSF.TruncPathStr(FilenameText,56);
            DialogItems[1].PtrData=FilenameText;
            FSF.sprintf(SrcText,GetMsg(mExistSource),0ULL,0,0,0,0,0,0);
            DialogItems[3].PtrData=SrcText;
            {
              unsigned long long size;
              SYSTEMTIME mod;
              GetFileAttr(filename,&size,&mod);
              FSF.sprintf(DstText,GetMsg(mExistDestination),size,mod.wDay,mod.wMonth,mod.wYear,mod.wHour,mod.wMinute,mod.wSecond);
              DialogItems[4].PtrData=DstText;
            }
            CFarDialog dialog;
            int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,69,12,NULL,DialogItems,ArraySize(DialogItems),0,0,FileExistsDialogProc,0);
            if((DlgCode==-1)||(DlgCode==10))
            {
              result=-1;
              break;
            }
            FlagAll=dialog.Check(6);
            ActionType=DlgCode-7+atOverwrite;
          }
          if(ActionType==atOverwrite)
          {
retry_overwrite:
            file=CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
            if(file==INVALID_HANDLE_VALUE)
            {
              int chk_res=CheckRetry(PanelItem[i].FindData.PANEL_FILENAME,filename);
              if(chk_res==rRetry)
                goto retry_overwrite;
              if(chk_res==rCancel)
              {
                result=-1;
                break;
              }
            }
          }
          else if(ActionType==atAppend)
          {
retry_append:
            file=CreateFile(filename,GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
            if(file!=INVALID_HANDLE_VALUE)
            {
              long HighPointer=0;
              SetFilePointer(file,0,&HighPointer,FILE_END);
            }
            else
            {
              int chk_res=CheckRetry(PanelItem[i].FindData.PANEL_FILENAME,filename);
              if(chk_res==rRetry)
                goto retry_append;
              if(chk_res==rCancel)
              {
                result=-1;
                break;
              }
            }
          }
        }
        else if(file==INVALID_HANDLE_VALUE)
        {
          int chk_res=CheckRetry(PanelItem[i].FindData.PANEL_FILENAME,filename);
          if(chk_res==rRetry)
            goto retry_main;
          if(chk_res==rCancel)
          {
            result=-1;
            break;
          }
        }
      }
      if(file!=INVALID_HANDLE_VALUE)
      {
//        WriteFile(file,(void *)(PanelItem[i].UserData),*((DWORD *)(PanelItem[i].UserData)),&transferred,NULL);
        wchar_t buff[SMALL_BUFFER];
        FSF.sprintf(buff,L"%c",0xfeff);
        WriteFile(file,buff,wcslen(buff)*sizeof(wchar_t),&transferred,NULL);
        if(QVOpt.ShowHeader||!(OpMode&OPM_QUICKVIEW))
        {
          FILETIME localtime;
          SYSTEMTIME time;
          FileTimeToLocalFileTime(&(PanelItem[i].FindData.ftCreationTime),&localtime);
          FileTimeToSystemTime(&localtime,&time);
          FSF.sprintf(buff,L"%s: %02d.%02d.%04d\r\n%s: %02d:%02d:%02d\r\n",GetMsg(mFileDate),time.wDay,time.wMonth,time.wYear,GetMsg(mFileTime),time.wHour,time.wMinute,time.wSecond);
          WriteFile(file,buff,wcslen(buff)*sizeof(wchar_t),&transferred,NULL);
          FSF.sprintf(buff,L"%s: %s\r\n%s: %s\r\n%s: %ld\r\n",GetMsg(mFileUser),(PanelItem[i].CustomColumnData?PanelItem[i].CustomColumnData[4]:NULL),GetMsg(mFileComputer),(PanelItem[i].CustomColumnData?PanelItem[i].CustomColumnData[3]:NULL),GetMsg(mFileEventID),curr_rec->EventID&0xffff);
          WriteFile(file,buff,wcslen(buff)*sizeof(wchar_t),&transferred,NULL);
          FSF.sprintf(buff,L"%s: %s\r\n%s: %s\r\n%s: %s\r\n\r\n",GetMsg(mFileSource),PanelItem[i].Description,GetMsg(mFileType),GetType(curr_rec->EventType),GetMsg(mFileCategory),(PanelItem[i].CustomColumnData?PanelItem[i].CustomColumnData[1]:NULL));
          WriteFile(file,buff,wcslen(buff)*sizeof(wchar_t),&transferred,NULL);
        }
        if(QVOpt.ShowDescription||!(OpMode&OPM_QUICKVIEW))
        {
          FSF.sprintf(buff,L"%s:\r\n",GetMsg(mFileDescription));
          WriteFile(file,buff,wcslen(buff)*sizeof(wchar_t),&transferred,NULL);
          wchar_t *msg=FormatLogMessage(panel->path,curr_rec);
          if(msg) WriteFile(file,msg,wcslen(msg)*sizeof(wchar_t),&transferred,NULL);
          free(msg);
          FSF.sprintf(buff,L"\r\n\r\n");
          WriteFile(file,buff,wcslen(buff)*sizeof(wchar_t),&transferred,NULL);
        }
        if(QVOpt.ShowData||!(OpMode&OPM_QUICKVIEW))
        {
          if(curr_rec->DataLength)
          {
            FSF.sprintf(buff,L"%s:\r\n",GetMsg(mFileData));
            WriteFile(file,buff,wcslen(buff)*sizeof(wchar_t),&transferred,NULL);
            int len=curr_rec->DataLength,offset=0;
            const int cols=16;
            while(len>0)
            {
              FSF.sprintf(buff,L"%08x: ",offset);
              for(int i=0;i<cols;i++)
              {
                if((len-i)>0)
                  FSF.sprintf(buff,L"%s%02X ",buff,*((unsigned char*)curr_rec+curr_rec->DataOffset+offset+i));
                else
                  FSF.sprintf(buff,L"%s   ",buff);
              }
              for(int i=0;i<cols;i++)
              {
                if((len-i)>0)
                {
                  unsigned char c=*((unsigned char*)curr_rec+curr_rec->DataOffset+offset+i);
                  if(c<' ') c='.';
                  FSF.sprintf(buff,L"%s%c",buff,c);
                }
              }
              FSF.sprintf(buff,L"%s\r\n",buff);
              offset+=cols;
              len-=cols;
              WriteFile(file,buff,wcslen(buff)*sizeof(wchar_t),&transferred,NULL);
            }
            FSF.sprintf(buff,L"\r\n");
            WriteFile(file,buff,wcslen(buff)*sizeof(wchar_t),&transferred,NULL);
          }
        }
        if(TechOpt.Separator[0]) WriteFile(file,TechOpt.Separator,wcslen(TechOpt.Separator)*sizeof(wchar_t),&transferred,NULL);
        CloseHandle(file);
      }
      if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
      {
        FSF.sprintf(copyname,L"%.55s ",filename);
        FSF.sprintf(progress,L"%3d%% ",(i+1)*100/ItemsNumber);
        for(int j=0;j<50;j++)
          progress[5+j]=BOX1;
        progress[55]=0;
        for(int j=0;j<((i+1)*50/ItemsNumber);j++)
          progress[5+j]=BOX2;
        Info.Message(Info.ModuleNumber,firsttime?(firsttime=false,FMSG_LEFTALIGN):(FMSG_KEEPBACKGROUND|FMSG_LEFTALIGN),NULL,Items,ArraySize(Items),0);

        INPUT_RECORD *InputRec;
        DWORD NumberOfEvents,ReadCnt;
        if(console!=INVALID_HANDLE_VALUE)
        {
          if(GetNumberOfConsoleInputEvents(console,&NumberOfEvents))
          {
            if(NumberOfEvents)
              if((InputRec=(INPUT_RECORD *)malloc(NumberOfEvents*sizeof(INPUT_RECORD)))!=NULL)
              {
                if(PeekConsoleInput(console,InputRec,NumberOfEvents,&ReadCnt))
                  for(unsigned long j=0;j<NumberOfEvents;j++)
                  {
                    if((InputRec[j].EventType==KEY_EVENT)&&(InputRec[j].Event.KeyEvent.bKeyDown)&&(InputRec[j].Event.KeyEvent.wVirtualKeyCode==VK_ESCAPE))
                    {
                      FlushConsoleInputBuffer(console);
                      result=-1;
                    }
                  }
                free(InputRec);
              }
          }
        }
      }
      PanelItem[i].Flags&=~PPIF_SELECTED;
      if(result==-1) break;
    }
    if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
    {
      CloseHandle(console);
      Info.RestoreScreen(screen);
    }
  }
  else
  {
    for(int i=0;i<ItemsNumber;i++)
    {
      if(IsDir)
        FSF.sprintf(filename,L"%s%s.evt",Dest,PanelItem[i].FindData.PANEL_FILENAME);
      else
        FSF.sprintf(filename,L"%s",Dest);
      wchar_t path_ansi[MAX_PATH];
      t_OemToChar(PanelItem[i].FindData.PANEL_FILENAME,path_ansi);
      HANDLE evt=OpenEventLog(panel->computer_ptr,path_ansi); //REMOTE
      if(evt)
      {
        wchar_t filename_ansi[MAX_PATH];
        t_OemToChar(filename,filename_ansi);
        if(!BackupEventLog(evt,filename_ansi))
          if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
            DISPLAY_WIN_ERROR
        CloseEventLog(evt);
      } else if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
        DISPLAY_WIN_ERROR
    }
    if(Move)
      DeleteFilesW(hPlugin,PanelItem,ItemsNumber,OpMode);
  }
  return result;
#endif
  return false;
}

int WINAPI DeleteFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
#if 0
  EventViewer *panel=(EventViewer *)hPlugin;
  if(panel->level==0)
  {
    for(int i=0;i<ItemsNumber;i++)
    {
      int MsgCode=0;
      if(!(OpMode&(OPM_SILENT)))
      {
        wchar_t Msg[1024];
        FSF.sprintf(Msg,GetMsg(mClearLog),PanelItem[i].FindData.PANEL_FILENAME);
        const wchar_t *MsgItems[]={L"",Msg,GetMsg(mClearClear),GetMsg(mClearSkip),GetMsg(mClearCancel)};
        MsgCode=Info.Message(Info.ModuleNumber,0,NULL,MsgItems,ArraySize(MsgItems),3);
      }
      if(MsgCode==0)
      {
        wchar_t path_ansi[MAX_PATH];
        t_OemToChar(PanelItem[i].FindData.PANEL_FILENAME,path_ansi);
        HANDLE evt=OpenEventLog(panel->computer_ptr,path_ansi); //REMOTE
        if(evt)
        {
          if(!ClearEventLog(evt,NULL))
          {
            if(!(OpMode&(OPM_SILENT)))
              DISPLAY_WIN_ERROR
          }
          else
          {
            CFarPanel pInfo(hPlugin,FCTL_GETPANELINFO);
            pInfo.StartSelection();
            for(int j=0;j<pInfo.ItemsNumber();j++)
              if(!FSF.LStricmp(pInfo[j].FindData.PANEL_FILENAME,PanelItem[i].FindData.PANEL_FILENAME))
                pInfo.RemoveSelection(j);
            pInfo.CommitSelection();
          }
          CloseEventLog(evt);
        } else if(!(OpMode&(OPM_SILENT)))
          DISPLAY_WIN_ERROR
      }
      else if((MsgCode==2)||(MsgCode==-1))
        break;
    }
    Info.ControlShort(hPlugin,FCTL_REDRAWPANEL,NULL);
    return TRUE;
  }
#endif
  return 0;
}

HANDLE WINAPI OpenFilePluginW(const wchar_t *Name,const unsigned char *Data,int DataSize,int OpMode)
{
#if 0
  if(!Name)
    return INVALID_HANDLE_VALUE;
  if(!Opt.BrowseEvtFiles)
    return INVALID_HANDLE_VALUE;
  if(!FSF.ProcessName(L"*.evt",(wchar_t*)Name, 0,PN_CMPNAMELIST))
    return INVALID_HANDLE_VALUE;
  return RealOpenFilePlugin(Name,Data,DataSize);
#endif
  return 0;
}

static HANDLE RealOpenFilePlugin(const wchar_t *Name,const unsigned char *Data,int DataSize)
{
#if 0
  wchar_t path_ansi[MAX_PATH];
  t_OemToChar(Name,path_ansi);
  HANDLE evt=OpenBackupEventLog(NULL,path_ansi); //LOCAL
  if(evt)
    CloseEventLog(evt);
  else
    return INVALID_HANDLE_VALUE;
  EventViewer *panel=(EventViewer *)malloc(sizeof(EventViewer));
  if(!panel)
    return INVALID_HANDLE_VALUE;
  panel->level=2;
  wcscpy(panel->path,Name);
  wcscpy(panel->computer,L"");
  wcscpy(panel->computer_oem,L"");
  panel->computer_ptr=NULL;
  panel->redraw=FALSE;
  return (HANDLE)panel;
#endif
  return 0;
}

int WINAPI CompareW(HANDLE hPlugin,const struct PluginPanelItem *Item1,const struct PluginPanelItem *Item2,unsigned int Mode)
{
#if 0
  EventViewer *panel=(EventViewer *)hPlugin;
  if(((Mode==SM_NAME)||(Mode==SM_EXT))&&panel->level)
  {
    if((Item1->UserData)&&(Item2->UserData))
    {
      int res;
      if(Opt.ScanType==0)
        res=((EVENTLOGRECORD *)(Item1->UserData))->RecordNumber-((EVENTLOGRECORD *)(Item2->UserData))->RecordNumber;
      else
        res=((EVENTLOGRECORD *)(Item2->UserData))->RecordNumber-((EVENTLOGRECORD *)(Item1->UserData))->RecordNumber;
      res=res/abs(res);
      return res;
    }
  }
#endif
  return -2;
}

// C0: Event
// C1: Category
// C2: Time
// C3: Computer
// C4: User
// C5: Time
// Z : Source
static wchar_t *GetTitles(const wchar_t *str)
{
#if 0
  wchar_t *Result=NULL;
  if((!wcsncmp(str,L"C0",2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleEvent);
  }
  else if((!wcsncmp(str,L"C1",2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleCategory);
  }
  else if((!wcsncmp(str,L"C2",2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleTime);
  }
  else if((!wcsncmp(str,L"C3",2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleComputer);
  }
  else if((!wcsncmp(str,L"C4",2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleUser);
  }
  else if((!wcsncmp(str,L"C5",2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleTime);
  }
  else if((str[0]=='Z')&&((!str[1])||(str[1]==',')))
  {
    Result=GetMsg(mTitleSource);
  }
  return Result;
#endif
}

void WINAPI GetOpenPluginInfoW(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
#if 0
  EventViewer *panel=(EventViewer *)hPlugin;
  Info->StructSize=sizeof(*Info);
  Info->Flags=OPIF_USEHIGHLIGHTING|OPIF_ADDDOTS|OPIF_SHOWNAMESONLY|OPIF_FINDFOLDERS;
  Info->HostFile=NULL;
  Info->CurDir=panel->path;
  if(panel->level==2)
  {
    Info->HostFile=panel->path;
    Info->CurDir=L"";
  }

  Info->Format=GetMsg(mName);

  static wchar_t Title[MAX_PATH];
  switch(panel->level)
  {
    case 0:
      if(panel->computer_ptr)
        FSF.sprintf(Title,L" %s: %s ",panel->computer_oem,GetMsg(mRootName));
      else
        FSF.sprintf(Title,L" %s ",GetMsg(mRootName));
      break;
    case 1:
      if(panel->computer_ptr)
        FSF.sprintf(Title,L" %s: %s\\%s ",panel->computer_oem,GetMsg(mRootName),panel->path);
      else
        FSF.sprintf(Title,L" %s\\%s ",GetMsg(mRootName),panel->path);
      break;
    case 2:
      FSF.sprintf(Title,L" %s ",panel->path);
      break;
  }
  Info->PanelTitle=Title;

  Info->InfoLines=NULL;
  Info->InfoLinesNumber=0;

  Info->DescrFiles=NULL;
  Info->DescrFilesNumber=0;

  static struct PanelMode PanelModesArray[10];
  static wchar_t *ColumnTitles[10][32];
  int msg_start=(panel->level)?mPanelTypeFile0:mPanelTypeDir0;
  wchar_t *tmp_msg;

  memset(PanelModesArray,0,sizeof(PanelModesArray));
  memset(ColumnTitles,0,sizeof(ColumnTitles));
  for(int i=0;i<10;i++)
  {
    for(int j=0;j<5;j++)
    {
      tmp_msg=GetMsg(msg_start+i*5+j);
      if(tmp_msg[0])
        switch(j)
        {
          case 0:
            PanelModesArray[i].ColumnTypes=tmp_msg;
            break;
          case 1:
            PanelModesArray[i].ColumnWidths=tmp_msg;
            break;
          case 2:
            PanelModesArray[i].StatusColumnTypes=tmp_msg;
            break;
          case 3:
            PanelModesArray[i].StatusColumnWidths=tmp_msg;
            break;
          case 4:
            PanelModesArray[i].FullScreen=tmp_msg[0]-'0';
            break;
        }
    }
    if(panel->level)
    {
      int j=0; const wchar_t *scan=PanelModesArray[i].ColumnTypes;
      if(scan)
      {
        while(TRUE)
        {
          if(j==32) break;
          ColumnTitles[i][j]=GetTitles(scan);
          scan=wcschr(scan,',');
          if(!scan) break;
          scan++;
          if(!scan[0]) break;
          j++;
        }
        PanelModesArray[i].ColumnTitles=ColumnTitles[i];
      }
    }
  }
  Info->PanelModesArray=PanelModesArray;
  Info->PanelModesNumber=ArraySize(PanelModesArray);
  Info->StartPanelMode='3';
  static struct KeyBarTitles KeyBar;
  memset(&KeyBar,0,sizeof(KeyBar));
  KeyBar.Titles[7-1]=(wchar_t*)L"";

  if(panel->level==2)
    KeyBar.Titles[6-1]=(wchar_t*)L"";
  else
    KeyBar.Titles[6-1]=GetMsg(mKeyRemote);
  if(panel->level)
  {
    KeyBar.Titles[8-1]=(wchar_t*)L"";
    KeyBar.ShiftTitles[8-1]=(wchar_t*)L"";
  }
  else
  {
    if(!panel->computer_ptr)
    {
      KeyBar.Titles[5-1]=GetMsg(mKeyBackup);
      KeyBar.ShiftTitles[5-1]=GetMsg(mKeyBackup);
    }
    else
    {
      KeyBar.Titles[5-1]=(wchar_t*)L"";
      KeyBar.ShiftTitles[5-1]=(wchar_t*)L"";
    }
    KeyBar.Titles[8-1]=GetMsg(mKeyClear);
    KeyBar.ShiftTitles[8-1]=GetMsg(mKeyClear);
  }
  KeyBar.ShiftTitles[1-1]=(wchar_t*)L"";
  KeyBar.ShiftTitles[2-1]=(wchar_t*)L"";
  if(panel->level>0)
  {
    KeyBar.ShiftTitles[3-1]=GetMsg(mKeyViewData);
    KeyBar.ShiftTitles[4-1]=GetMsg(mKeyEditData);
  }
  else
  {
    KeyBar.ShiftTitles[3-1]=(wchar_t*)L"";
    KeyBar.ShiftTitles[4-1]=(wchar_t*)L"";
  }
  if(panel->level==2)
    KeyBar.ShiftTitles[6-1]=(wchar_t*)L"";
  else
    KeyBar.ShiftTitles[6-1]=GetMsg(mKeyLocal);
  KeyBar.AltTitles[6-1]=(wchar_t*)L"";
  Info->KeyBar=&KeyBar;
#endif
}

static long WINAPI ComputerDialogProc(HANDLE hDlg, int Msg,int Param1,long Param2)
{
#if 0
  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_SETTEXTLENGTH,2,MAX_PATH-3);
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
#endif
  return 0;
}

int WINAPI ProcessKeyW(HANDLE hPlugin,int Key,unsigned int ControlState)
{
#if 0
  EventViewer *panel=(EventViewer *)hPlugin;
  if((ControlState==PKF_SHIFT)&&(Key==VK_F4||Key==VK_F3)) //skip Shift-F4
  {
    if(panel->level>0)
    {
      CFarPanel pInfo(hPlugin,FCTL_GETPANELINFO);
      if(pInfo.ItemsNumber()&&(pInfo[pInfo.CurrentItem()].Flags&PPIF_USERDATA))
      {
        wchar_t temp[MAX_PATH],tempfile[MAX_PATH];
        if(GetTempPath(MAX_PATH,temp)&&GetTempFileName(temp,L"evt",0,tempfile))
        {
          HANDLE hdata=CreateFile(tempfile,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
          if(hdata!=INVALID_HANDLE_VALUE)
          {
            DWORD dWritten;
            EVENTLOGRECORD *curr_rec=(EVENTLOGRECORD *)(pInfo[pInfo.CurrentItem()].UserData);
            if(curr_rec->DataLength)
              WriteFile(hdata,(unsigned char*)curr_rec+curr_rec->DataOffset,curr_rec->DataLength,&dWritten,NULL);
            CloseHandle(hdata);
            if(Key==VK_F3)
              Info.Viewer(tempfile,NULL,0,0,-1,-1,VF_DELETEONLYFILEONCLOSE|VF_ENABLE_F6|VF_DISABLEHISTORY|VF_NONMODAL|VF_IMMEDIATERETURN,CP_AUTODETECT);
            else
              Info.Editor(tempfile,NULL,0,0,-1,-1,EF_DELETEONLYFILEONCLOSE|EF_ENABLE_F6|EF_DISABLEHISTORY|EF_NONMODAL|EF_IMMEDIATERETURN,1,1,CP_AUTODETECT);
          }
        }
      }
    }
    return TRUE;
  }
  if((ControlState==0)&&(Key==VK_F6)) //skip F6
  {
    if(panel->level<2)
    {
      /*
        000000000011111111112222222222333333333344444444
        012345678901234567890123456789012345678901234567
      00                                                00
      01   ษอออออออออออ Select Computer ออออออออออออป   01
      02   บ Computer:                              บ   02
      03   บ ZG                                    ณบ   03
      04   บ (empty for local system)               บ   04
      05   ศออออออออออออออออออออออออออออออออออออออออผ   05
      06                                                06
        000000000011111111112222222222333333333344444444
        012345678901234567890123456789012345678901234567
      */
      static const wchar_t *NTEventComputerHistoryName=L"NTEventComputer";
      static struct InitDialogItem InitDlg[]={
      /*0*/  {DI_DOUBLEBOX,3,1,44,5,0,0,0,0,(wchar_t *)mSelCompTitle},
      /*1*/  {DI_TEXT,5,2,0,0,0,0,0,0,(wchar_t *)mSelCompLabel},
      /*2*/  {DI_EDIT,5,3,42,0,1,(DWORD)NTEventComputerHistoryName,DIF_HISTORY,0,L""},
      /*3*/  {DI_TEXT,5,4,0,0,0,0,0,0,(wchar_t *)mSelCompFootnote},
      };
      struct FarDialogItem DialogItems[ArraySize(InitDlg)];
      InitDialogItems(InitDlg,DialogItems,ArraySize(InitDlg));
      DialogItems[2].PtrData=panel->computer_oem;
      CFarDialog dialog;
      int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,48,7,NULL,DialogItems,ArraySize(InitDlg),0,0,ComputerDialogProc,0);
      if(DlgCode!=-1)
      {
        if(dialog.Str(2)[0])
        {
          wchar_t temp_computer_name[MAX_PATH];
          if(wcsncmp(dialog.Str(2),L"\\\\",2))
          {
            wchar_t tmp[512];
            FSF.sprintf(tmp,L"\\\\%s",dialog.Str(2));
            wcscpy(temp_computer_name,tmp);
          }
          if(CheckRemoteEventLog(temp_computer_name))
          {
            wcscpy(panel->computer,temp_computer_name);
            wcscpy(panel->computer_oem,temp_computer_name);
            panel->computer_ptr=panel->computer;
            Info.ControlShort3(hPlugin,FCTL_UPDATEPANEL,(FIRST_PARAM)0);
            Info.ControlShort(hPlugin,FCTL_REDRAWPANEL,NULL);
          }
          else
          {
            const wchar_t *MsgItems[]={GetMsg(mError),GetMsg(mErr2),GetMsg(mOk)};
            Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,ArraySize(MsgItems),1);
          }
        }
        else
        {
          wcscpy(panel->computer,L"");
          wcscpy(panel->computer_oem,L"");
          panel->computer_ptr=NULL;
          Info.ControlShort3(hPlugin,FCTL_UPDATEPANEL,(FIRST_PARAM)0);
          Info.ControlShort(hPlugin,FCTL_REDRAWPANEL,NULL);
        }
      }
    }
    return TRUE;
  }
  if((ControlState==PKF_SHIFT)&&(Key==VK_F6)) //skip Shift-F6
  {
    wcscpy(panel->computer,L"");
    wcscpy(panel->computer_oem,L"");
    panel->computer_ptr=NULL;
    Info.ControlShort3(hPlugin,FCTL_UPDATEPANEL,(FIRST_PARAM)0);
    Info.ControlShort(hPlugin,FCTL_REDRAWPANEL,NULL);
    return TRUE;
  }
  if((ControlState==0)&&(Key==VK_F7)) //skip F7
    return TRUE;
  if((((ControlState==0)||(ControlState==PKF_SHIFT))&&(Key==VK_F8))||(((ControlState==PKF_ALT)||(ControlState==PKF_SHIFT))&&(Key==VK_DELETE))) //skip F8,Shift-F8,Alt-Del,Shift-Del
  {
    if(panel->level>0)
      return TRUE;
  }
  return FALSE;
#endif
  return false;
}

intptr_t WINAPI ConfigureW(const struct ConfigureInfo *Info)
{
  return Config();
}

int WINAPI ProcessEventW(HANDLE hPlugin,int Event,void *Param)
{
#if 0
  EventViewer *panel=(EventViewer *)hPlugin;
  if(Event==FE_REDRAW&&panel->redraw)
  {
    panel->redraw=FALSE;
    if(State.ViewMode!=-1)
      Info.ControlShort2(hPlugin,FCTL_SETVIEWMODE,State.ViewMode);
    if(State.SortMode!=-1)
      Info.ControlShort2(hPlugin,FCTL_SETSORTMODE,State.SortMode);
    if(State.SortOrder!=-1)
      Info.ControlShort2(hPlugin,FCTL_SETSORTORDER,State.SortOrder);
    PanelRedrawInfo ri={State.Current,State.Top};
    Info.ControlShort(hPlugin,FCTL_REDRAWPANEL,(SECOND_PARAM)&ri);
    return TRUE;
  }
  else if(Event==FE_CLOSE&&panel->level<2)
  {
    PanelInfo PInfo;
    Info.ControlShort(hPlugin,FCTL_GETPANELSHORTINFO,(SECOND_PARAM)&PInfo);
    State.Current=PInfo.CurrentItem;
    State.Top=PInfo.TopPanelItem;
    State.ViewMode=PInfo.ViewMode;
    State.SortMode=PInfo.SortMode;
    State.SortOrder=PInfo.Flags&PFLAGS_REVERSESORTORDER?1:0;
    wcscpy(State.Path,panel->path);
    if(panel->computer_ptr)
      wcscpy(State.Computer,panel->computer_oem);
    else
      wcscpy(State.Computer,L"");
    if(Opt.Restore)
    {
      HKEY hKey;
      DWORD Disposition;
      if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
      {
        RegSetValueEx(hKey,L"StatePath",0,REG_SZ,(LPBYTE)State.Path,(wcslen(State.Path)+1)*sizeof(wchar_t));
        RegSetValueEx(hKey,L"StateComputer",0,REG_SZ,(LPBYTE)State.Computer,(wcslen(State.Computer)+1)*sizeof(wchar_t));
        RegSetValueEx(hKey,L"StateCurrent",0,REG_DWORD,(LPBYTE)&State.Current,sizeof(State.Current));
        RegSetValueEx(hKey,L"StateTop",0,REG_DWORD,(LPBYTE)&State.Top,sizeof(State.Top));
        RegSetValueEx(hKey,L"StateViewMode",0,REG_DWORD,(LPBYTE)&State.ViewMode,sizeof(State.ViewMode));
        RegSetValueEx(hKey,L"StateSortMode",0,REG_DWORD,(LPBYTE)&State.SortMode,sizeof(State.SortMode));
        RegSetValueEx(hKey,L"StateSortOrder",0,REG_DWORD,(LPBYTE)&State.SortOrder,sizeof(State.SortOrder));
        RegCloseKey(hKey);
      }
    }
  }
  return FALSE;
#endif
  return FALSE;
}

int WINAPI GetMinFarVersionW(void)
{
#if 0
  return FARMANAGERVERSION;
#endif
  return 0;
}

void WINAPI ExitFARW()
{
  free_sid_cache();
  free_category_cache();
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
  return TRUE;
}
