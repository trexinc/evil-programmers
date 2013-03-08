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

// {71C5D811-D1AD-4443-ACAF-AC62AF95B99F}
DEFINE_GUID(ConnectingMessageDuid, 0x71c5d811, 0xd1ad, 0x4443, 0xac, 0xaf, 0xac, 0x62, 0xaf, 0x95, 0xb9, 0x9f);
// {0C2FB39F-7BBE-4f78-BF7D-FC9100F1EF5B}
DEFINE_GUID(ScanningMessageDuid, 0xc2fb39f, 0x7bbe, 0x4f78, 0xbf, 0x7d, 0xfc, 0x91, 0x0, 0xf1, 0xef, 0x5b);
// {444F7843-3136-4269-B858-F9F079E558A3}
DEFINE_GUID(WinErrorMessaGuid, 0x444f7843, 0x3136, 0x4269, 0xb8, 0x58, 0xf9, 0xf0, 0x79, 0xe5, 0x58, 0xa3);
// {808FD0C1-8B8D-48d4-8D44-4D77816A7BC6}
DEFINE_GUID(ErrorMessageGuid, 0x808fd0c1, 0x8b8d, 0x48d4, 0x8d, 0x44, 0x4d, 0x77, 0x81, 0x6a, 0x7b, 0xc6);
// {35E9A7A4-6C87-40cc-9718-B6299D4C3616}
DEFINE_GUID(CopyMessageGuid,0x35e9a7a4, 0x6c87, 0x40cc, 0x97, 0x18, 0xb6, 0x29, 0x9d, 0x4c, 0x36, 0x16);
// {7BA798A7-1E81-48ae-BE8B-A33DEDEAD8FC}
DEFINE_GUID(RetryMessageGuid, 0x7ba798a7, 0x1e81, 0x48ae, 0xbe, 0x8b, 0xa3, 0x3d, 0xed, 0xea, 0xd8, 0xfc);
// {8A773D5D-3F3F-47bb-AB0C-C3E6D0D3BDC7}
DEFINE_GUID(ClearMessageGuid, 0x8a773d5d, 0x3f3f, 0x47bb, 0xab, 0xc, 0xc3, 0xe6, 0xd0, 0xd3, 0xbd, 0xc7);
// {4E906535-51B4-4bb2-B770-A9CD09931FBE}
DEFINE_GUID(Error2MessageGuid, 0x4e906535, 0x51b4, 0x4bb2, 0xb7, 0x70, 0xa9, 0xcd, 0x9, 0x93, 0x1f, 0xbe);

// {35CA99D4-9383-4cec-9DE9-D9B07D5C4711}
DEFINE_GUID(DlgExistsGuid, 0x35ca99d4, 0x9383, 0x4cec, 0x9d, 0xe9, 0xd9, 0xb0, 0x7d, 0x5c, 0x47, 0x11);
// {E8B7C166-746C-4cce-8A37-7B88C1E9AFC6}
DEFINE_GUID(CopyDialogGuid, 0xe8b7c166, 0x746c, 0x4cce, 0x8a, 0x37, 0x7b, 0x88, 0xc1, 0xe9, 0xaf, 0xc6); 
// {F0FD70AC-A642-4ef1-8123-4F1F9C630EB4}
DEFINE_GUID(SelectCompGuid, 0xf0fd70ac, 0xa642, 0x4ef1, 0x81, 0x23, 0x4f, 0x1f, 0x9c, 0x63, 0xe, 0xb4);


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
  size_t Current;
  size_t Top;
  intptr_t ViewMode;
  int SortMode;
  int SortOrder;
} State;

static const wchar_t *GetMsg(intptr_t MsgId)
{
  return Info.GetMsg(&MainGuid,MsgId);
}

static const wchar_t *default_column_data=L"";

static bool CheckRemoteEventLog(wchar_t *computer)
{
  HANDLE hSScr=Info.SaveScreen(0,0,-1,-1);
  const wchar_t *MsgItems[]={L"",GetMsg(mOtherConnecting)};
  Info.Message(&MainGuid,&ConnectingMessageDuid,0,NULL,MsgItems,ArraySize(MsgItems),0);
  HANDLE evt=OpenEventLog(computer,L"System"); //REMOTE
  Info.RestoreScreen(hSScr);
  if(evt)
  {
    CloseEventLog(evt);
    return true;
  }
  return false;
}

#include "el_utils.cpp"
#include "el_config.cpp"

#define DISPLAY_WIN_ERROR \
{ \
  const wchar_t *MsgItems[]={GetMsg(mError),GetMsg(mOk)}; \
  Info.Message(&MainGuid,&WinErrorMessaGuid,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,ArraySize(MsgItems),1); \
}

static HANDLE RealOpenFilePlugin(const wchar_t *Name,const void *Data,size_t DataSize);

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=MAKEFARVERSION(0,6,2,0,VS_RELEASE);
  Info->Guid=MainGuid;
  Info->Title=L"NT Events";
  Info->Description=L"NT Events";
  Info->Author=L"Vadim Yegorov";
} 

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
  memset(&::Info, 0, sizeof(::Info));
  memmove(&::Info, Info, (Info->StructSize > sizeof(::Info))?sizeof(::Info):Info->StructSize);
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

HANDLE WINAPI OpenW(const struct OpenInfo *oInfo)
{
  if ((oInfo->OpenFrom == OPEN_PLUGINSMENU)||(oInfo->OpenFrom == OPEN_LEFTDISKMENU) 
    ||(oInfo->OpenFrom == OPEN_RIGHTDISKMENU)||(oInfo->OpenFrom ==OPEN_COMMANDLINE))
  {
    EventViewer *panel=(EventViewer *)malloc(sizeof(EventViewer));
    if(!panel)
      return NULL;
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
        if(CheckRemoteEventLog(State.Computer))
        {
          wcscpy(panel->computer,State.Computer);
          wcscpy(panel->computer_oem,State.Computer);
          panel->computer_ptr=panel->computer;
        }
      }
    }
    return (HANDLE)panel;
  }
  else 
  if (oInfo->OpenFrom == OPEN_ANALYSE)
  {
    const OpenAnalyseInfo* oai = (const OpenAnalyseInfo*)(oInfo->Data);
    EventViewer *panel=(EventViewer *)oai->Handle; 
    return (HANDLE)panel;
  }
  return NULL;
}

void WINAPI ClosePanelW(const struct ClosePanelInfo *pInfo)
{
  free((EventViewer *)pInfo->hPanel);
}

static void WINAPI FreeUserData(void* UserData,const FarPanelItemFreeInfo* Info)
{
	free(UserData);
}

intptr_t WINAPI GetFindDataW(struct GetFindDataInfo *dInfo)
{
    HANDLE hSScr=Info.SaveScreen(0,0,-1,-1),console=INVALID_HANDLE_VALUE;
    if(!(dInfo->OpMode&(OPM_FIND)))
    {
      const wchar_t *MsgItems[]={L"",GetMsg(mOtherScanning)};
      Info.Message(&MainGuid,&ScanningMessageDuid,0,NULL,MsgItems,ArraySize(MsgItems),0); 
      console=CreateFile(L"CONIN$",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    }
    EventViewer *panel=(EventViewer *)dInfo->hPanel; 
    HANDLE evt=INVALID_HANDLE_VALUE; DWORD scan=EVENTLOG_FORWARDS_READ;
    if(Opt.ScanType==1) scan=EVENTLOG_BACKWARDS_READ;
    dInfo->PanelItem=NULL; dInfo->ItemsNumber=0;
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
              struct PluginPanelItem *pNewPanelItem=(PluginPanelItem *)realloc(dInfo->PanelItem,sizeof(PluginPanelItem)*((dInfo->ItemsNumber)+1));
              if(pNewPanelItem)
              {
                dInfo->PanelItem=pNewPanelItem;
                PluginPanelItem *curr=dInfo->PanelItem+dInfo->ItemsNumber++;
                memset(curr,0,sizeof(PluginPanelItem));
                curr->FileName=(wchar_t*)malloc((wcslen(NameBuffer)+1)*sizeof(wchar_t));
                if(curr->FileName) wcscpy((wchar_t*)curr->FileName,NameBuffer);
                curr->FileAttributes=FILE_ATTRIBUTE_DIRECTORY;
              }
              else
              {
                free(dInfo->PanelItem);
                dInfo->PanelItem=NULL;
                dInfo->ItemsNumber=0;
                break;
              }
            }
          }
          RegCloseKey(hKey);
        }
        break;
      }
      case 1:
        evt=OpenEventLog(panel->computer_ptr,panel->path); //REMOTE
      case 2:
        if(evt==INVALID_HANDLE_VALUE)
          evt=OpenBackupEventLog(NULL,panel->path); //LOCAL
        if(evt)
        {
          if(GetNumberOfEventLogRecords(evt,(DWORD *)&dInfo->ItemsNumber))
          {
            dInfo->ItemsNumber++;
            dInfo->PanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*(dInfo->ItemsNumber));
            if(dInfo->PanelItem)
            {
              memset(dInfo->PanelItem,0,sizeof(PluginPanelItem)*(dInfo->ItemsNumber));
              dInfo->PanelItem->FileName=(wchar_t*)malloc(3*sizeof(wchar_t));
              wcscpy((wchar_t*)(dInfo->PanelItem->FileName),L"..");
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
                CustomColumnData[5]=(wchar_t*)L"";
                for(int i=0;i<(CUSTOM_COLUMN_COUNT-1);i++)
                  if(!CustomColumnData[i])
                    CustomColumnData[i]=(wchar_t*)default_column_data;
                dInfo->PanelItem->CustomColumnNumber=CUSTOM_COLUMN_COUNT;
              }
              dInfo->PanelItem->CustomColumnData=CustomColumnData;
              for(size_t i=1;i<dInfo->ItemsNumber;i++)
              {
                (dInfo->PanelItem+i)->FileName=(wchar_t*)malloc(512*sizeof(wchar_t));
                FSF.sprintf((wchar_t*)((dInfo->PanelItem+i)->FileName),L"%s.%s.%s",GetMsg(mExtNameError),GetMsg(mExtError),GetMsg(mExtMain));
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
                for(size_t i=1;(i<dInfo->ItemsNumber)&&work;)
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
                    else if(!(dInfo->OpMode&(OPM_FIND)))
                      DISPLAY_WIN_ERROR
                    break;
                  }
                  curr_rec=(EVENTLOGRECORD *)buff;
                  while(readed&&work)
                  {
                    user_data=malloc(curr_rec->Length);
                    if(user_data)
                    {
                      memcpy(user_data,curr_rec,curr_rec->Length);
                      (dInfo->PanelItem+i)->UserData.Data=user_data;
                      (dInfo->PanelItem+i)->UserData.FreeData=FreeUserData;
                      wchar_t *description=(wchar_t *)(((EVENTLOGRECORD *)user_data)+1);
                      wchar_t* Description=(wchar_t *)malloc((wcslen(description)+1)*sizeof(wchar_t));
                      if(Description)
                      {
                        FSF.sprintf(Description,L"%s",description);
                      }
                      (dInfo->PanelItem+i)->Description=Description;
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
                    if (!(dInfo->PanelItem+i)->FileName){
                      (dInfo->PanelItem+i)->FileName=(wchar_t*)malloc(512*sizeof(wchar_t));
                    }
                    FSF.sprintf((wchar_t*)((dInfo->PanelItem+i)->FileName),L"%08ld.%s.%s",curr_rec->RecordNumber,suffix,GetMsg(mExtMain));
                    (dInfo->PanelItem+i)->FileSize=curr_rec->Length;
                    evt_date_time=curr_rec->TimeWritten; evt_date_time=evt_date_time*10000000ULL+EVENT_START_TIME;
  //                  FileTimeToLocalFileTime((FILETIME *)&evt_date_time,(FILETIME *)&evt_date_time_local);
                    memcpy(&((dInfo->PanelItem+i)->LastWriteTime),&evt_date_time,sizeof(evt_date_time));
                    evt_date_time=curr_rec->TimeGenerated; evt_date_time=evt_date_time*10000000ULL+EVENT_START_TIME;
                    memcpy(&((dInfo->PanelItem+i)->CreationTime),&evt_date_time,sizeof(evt_date_time));
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
                      (dInfo->PanelItem+i)->CustomColumnNumber=CUSTOM_COLUMN_COUNT;
                    }
                    (dInfo->PanelItem+i)->CustomColumnData=CustomColumnData;
                    readed-=curr_rec->Length;
                    curr_rec=(EVENTLOGRECORD *)((char*)curr_rec+curr_rec->Length);
                    i++;
                    if(!(dInfo->OpMode&(OPM_FIND)))
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
              dInfo->ItemsNumber=0;
          }
          else if(!(dInfo->OpMode&(OPM_FIND)))
          {
            DISPLAY_WIN_ERROR
            dInfo->ItemsNumber=0;
          }
          CloseEventLog(evt);
        }
        else if(!(dInfo->OpMode&(OPM_FIND)))
          DISPLAY_WIN_ERROR
        break;
    }
    if(!(dInfo->OpMode&(OPM_FIND)))
      CloseHandle(console);
    Info.RestoreScreen(hSScr);
    return TRUE;
}

void WINAPI FreeFindDataW(const struct FreeFindDataInfo *dInfo)
{
  for(size_t i=0;i<dInfo->ItemsNumber;i++)
  {
    free((void*)dInfo->PanelItem[i].Owner);
    free((void*)dInfo->PanelItem[i].Description);
    if(dInfo->PanelItem[i].CustomColumnData)
      for(int j=0;j<(CUSTOM_COLUMN_COUNT-1);j++)
        if(dInfo->PanelItem[i].CustomColumnData[j]!=default_column_data)
          free((void*)dInfo->PanelItem[i].CustomColumnData[j]);
    free((void*)dInfo->PanelItem[i].CustomColumnData);
    free((void*)dInfo->PanelItem[i].FileName);
  }
  free(dInfo->PanelItem);
}

intptr_t WINAPI SetDirectoryW(const struct SetDirectoryInfo *dInfo)
{
  int res=TRUE;
  EventViewer *panel=(EventViewer *)dInfo->hPanel;
  if(panel->level==2)
    return TRUE;
  if(!wcscmp(dInfo->Dir,L"\\"))
  {
    panel->level=0;
    wcscpy(panel->path,L"");
  }
  else if(!wcscmp(dInfo->Dir,L".."))
  {
    if(panel->level==1)
    {
      panel->level=0;
      wcscpy(panel->path,L"");
    }
    else res=FALSE;
  }
  else if((panel->level==0)&&(CheckLogName(panel,dInfo->Dir)))
  {
    panel->level=1;
    wcscpy(panel->path,dInfo->Dir);
  }
  else res=FALSE;
  return res;
}

static intptr_t WINAPI CopyDialogProc(HANDLE hDlg, intptr_t Msg,intptr_t Param1,void* Param2)
{
  switch(Msg)
  {
    case DN_INITDIALOG:
      int length=MAX_PATH-1;
      Info.SendDlgMessage(hDlg,DM_SETMAXTEXTLENGTH,2,&length);
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
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
  01   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ Copy ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»   01
  02   º Copy "todo" to                                                     º   02
  03   º D:\00000005.DIR\c\far.plug\EventViewer\                           |º   03
  04   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   04
  05   º                 [ Copy ]                [ Cancel ]                 º   05
  06   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   06
  07                                                                            07
    0000000000111111111122222222223333333333444444444455555555556666666666777777
    0123456789012345678901234567890123456789012345678901234567890123456789012345
  */
  static struct FarDialogItem DialogItems[]={
  /*0*/  {DI_DOUBLEBOX,3,1,72,6,{0},0,0,0,L"",0,0},
  /*1*/  {DI_TEXT,5,2,0,0,{0},0,0,0,L"",0,0},
  /*2*/  {DI_EDIT,5,3,70,0,{1},L"NTEventCopy",0,DIF_HISTORY,L"",0,0},
  /*3*/  {DI_TEXT,5,4,0,0,{0},0,0,DIF_BOXCOLOR|DIF_SEPARATOR,L"",0,0},
  /*4*/  {DI_BUTTON,0,5,0,0,{0},0,0,DIF_CENTERGROUP|DIF_DEFAULTBUTTON,L"",0,0},
  /*5*/  {DI_BUTTON,0,5,0,0,{0},0,0,DIF_CENTERGROUP,GetMsg(mCpyDlgCancel),0,0}
  };
  PanelInfo PInfo; 
  PInfo.StructSize=sizeof(PanelInfo);
  Info.PanelControl(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,0,&PInfo); //FIXME
  DialogItems[0].Data=GetMsg(mCpyDlgCopyTitle+move);
  wchar_t CopyText[512];
  FSF.sprintf(CopyText,GetMsg(mCpyDlgCopyToN+3*move+NumberType(PInfo.SelectedItemsNumber)),PInfo.SelectedItemsNumber);
  DialogItems[1].Data=CopyText;
  DialogItems[2].Data=dir;
  DialogItems[4].Data=GetMsg(mCpyDlgCopyOk+move);
  CFarDialog dialog;
  intptr_t DlgCode=dialog.Execute(MainGuid,CopyDialogGuid,-1,-1,76,8,NULL,DialogItems,(ArraySize(DialogItems)),0,0,CopyDialogProc,0); 
  if(DlgCode==4)
  {
    FSF.sprintf(dir,L"%s",dialog.Str(2));
    return TRUE;
  }
  else
    return FALSE;
}

static void GetFileAttr(wchar_t *file,unsigned long long *size,SYSTEMTIME *mod)
{
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
}

static int CheckRetry(const wchar_t *afrom,const wchar_t *ato)
{
  wchar_t from[512],to[512],buff[MAX_PATH];
  FSF.sprintf(buff,L"%s",afrom);
  FSF.TruncPathStr(buff,55);
  FSF.sprintf(from,GetMsg(mRetryFrom),buff);
  FSF.sprintf(buff,L"%s",ato);
  FSF.TruncPathStr(buff,55);
  FSF.sprintf(to,GetMsg(mRetryTo),buff);
  const wchar_t *MsgItems[]={GetMsg(mRetryError),from,to,GetMsg(mRetryRetry),GetMsg(mRetrySkip),GetMsg(mRetryCancel)};
  return Info.Message(&MainGuid,&RetryMessageGuid,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,ArraySize(MsgItems),3);
}

static intptr_t WINAPI FileExistsDialogProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
  FarDialogItemColors* colors;
  FarColor c;
  switch(Msg)
  {
    case DN_CTLCOLORDIALOG:
      Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_WARNDIALOGTEXT, Param2);
      return true;
    case DN_CTLCOLORDLGITEM:
      colors=(FarDialogItemColors*)Param2;
      switch(Param1)
      {
        case 0:
          if (Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_WARNDIALOGBOX,&c)) colors->Colors[2]=c;
          if (Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_WARNDIALOGHIGHLIGHTBOXTITLE,&c)) colors->Colors[1]=c;
          if (Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_WARNDIALOGBOXTITLE,&c)) colors->Colors[0]=c;
          return true;               
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
          if (Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_WARNDIALOGBOX,&c)) colors->Colors[2]=c;
          if (Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_WARNDIALOGHIGHLIGHTTEXT,&c)) colors->Colors[1]=c;
          if (Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_WARNDIALOGTEXT,&c)) colors->Colors[0]=c;
          return true;  
        case 7:
        case 8:
        case 9:
        case 10:          
          if(Info.SendDlgMessage(hDlg,DM_GETFOCUS,0,0)==Param1)
          {
            if (Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON,&c)) colors->Colors[1]=c;
            if (Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_WARNDIALOGSELECTEDBUTTON,&c)) colors->Colors[0]=c;
          }
          else
          {
            if (Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_WARNDIALOGHIGHLIGHTBUTTON,&c)) colors->Colors[1]=c;
            if (Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_WARNDIALOGBUTTON,&c)) colors->Colors[0]=c;
          }
          return true;  
      }
    case DN_ENTERIDLE:
    {
      FarDialogItemData Caption;
      TCHAR Buff[512]; SYSTEMTIME mod;
      GetLocalTime(&mod);
      FSF.sprintf(Buff,GetMsg(mExistSource),0ULL,mod.wDay,mod.wMonth,mod.wYear,mod.wHour,mod.wMinute,mod.wSecond);
      Caption.StructSize=sizeof(FarDialogItemData);
      Caption.PtrData=Buff;
      Caption.PtrLength=wcslen(Caption.PtrData);
      Info.SendDlgMessage(hDlg,DM_SETTEXT,3,&Caption);
      //refresh
      Info.SendDlgMessage(hDlg,DM_REDRAW,0,0);
      break;
    }
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

intptr_t WINAPI GetFilesW(struct GetFilesInfo *fInfo)
{
  EventViewer *panel=(EventViewer *)fInfo->hPanel;
  int OpMode2=fInfo->OpMode&(~OPM_TOPLEVEL);
  if((!panel->level)&&(panel->computer_ptr)&&(!TechOpt.NetBackup)) return 0; //no remote backup
  static wchar_t DestPathReal[MAX_PATH];
  wcscpy(DestPathReal,fInfo->DestPath);
  fInfo->DestPath=DestPathReal;

  wchar_t Dir[MAX_PATH],Dest[MAX_PATH];
  int result=1;
  FSF.sprintf(Dir,L"%s",fInfo->DestPath);
  FSF.AddEndSlash(Dir);
  if(!OpMode2)
  {
    if(!GetDestDir(Dir,fInfo->Move?1:0))
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
      if(!(fInfo->OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
      {
        wchar_t err1[512];
        FSF.sprintf(err1,GetMsg(mErr1),Dir);
        const wchar_t *MsgItems[]={GetMsg(mError),err1,GetMsg(mOk)};
        Info.Message(&MainGuid,&ErrorMessageGuid,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,ArraySize(MsgItems),1);
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
    FSF.sprintf(Dest,L"%s",fInfo->DestPath);
    FSF.AddEndSlash(Dest);
  }
  BOOL IsDir=(Dest[wcslen(Dest)-1]=='\\');
  wchar_t filename[MAX_PATH]; HANDLE file; DWORD transferred;
  FSF.AddEndSlash((wchar_t*)fInfo->DestPath);
  if(panel->level)
  {
    BOOL FlagAll=FALSE; BOOL ActionType=atNone;
    wchar_t copyname[56],progress[56]; HANDLE screen=NULL,console=INVALID_HANDLE_VALUE;
    const wchar_t *Items[]={GetMsg(mCpyDlgCopyTitle),copyname,progress};
    if(!(fInfo->OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
    {
      screen=Info.SaveScreen(0,0,-1,-1);
      console=CreateFile(L"CONIN$",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    }
    bool firsttime=true;
    for(size_t i=0;i<fInfo->ItemsNumber;i++)
    {
      if(!(fInfo->PanelItem[i].UserData.Data)) continue;
      EVENTLOGRECORD *curr_rec=(EVENTLOGRECORD *)(fInfo->PanelItem[i].UserData.Data);
      if(IsDir)
      {
        if(OpMode2||(!Opt.StripExt))
          FSF.sprintf(filename,L"%s%s",Dest,fInfo->PanelItem[i].FileName);
        else
          FSF.sprintf(filename,L"%s%08ld.%s",Dest,curr_rec->RecordNumber,GetMsg(mExtMain));
      }
      else
        FSF.sprintf(filename,L"%s",Dest);
retry_main:
      file=CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN|(fInfo->OpMode&OPM_EDIT?FILE_ATTRIBUTE_READONLY:0),NULL); //FIXME
      if(!(fInfo->OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
      {
        if((file==INVALID_HANDLE_VALUE)&&(GetLastError()==ERROR_FILE_EXISTS))
        {
          if(!FlagAll)
          {
            /*
              000000000011111111112222222222333333333344444444445555555555666666666
              012345678901234567890123456789012345678901234567890123456789012345678
            00                                                                     00
            01   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ Destination already exists ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»   01
            02   º                            1                                º   02
            03   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   03
            04   º Source                              0  27.06.2001  13:53:55 º   04
            05   º Destination      12345678901234567890  27.06.2001  13:53:55 º   05
            06   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   06
            07   º [ ] Accept choice for all files                             º   07
            08   º                                                             º   08
            09   º     [ Overwrite ]  [ Skip ]  [ Append ]  [ Cancel ]         º   09
            10   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   10
            11                                                                     11
              000000000011111111112222222222333333333344444444445555555555666666666
              012345678901234567890123456789012345678901234567890123456789012345678
            */
            static struct FarDialogItem DialogItems[]={
            /* 0*/  {DI_DOUBLEBOX,3,1,65,10,{0},0,0,0,GetMsg(mExistDestAlreadyExists),0,0},
            /* 1*/  {DI_TEXT,5,2,0,0,{0},0,0,0,L"",0,0},
            /* 2*/  {DI_TEXT,-1,3,0,0,{0},0,0,DIF_SEPARATOR,L"",0,0},
            /* 3*/  {DI_TEXT,5,4,0,0,{0},0,0,0,L"",0,0},
            /* 4*/  {DI_TEXT,5,5,0,0,{0},0,0,0,L"",0,0},
            /* 5*/  {DI_TEXT,-1,6,0,0,{0},0,0,DIF_SEPARATOR,L"",0,0},
            /* 6*/  {DI_CHECKBOX,5,7,0,0,{0},0,0,DIF_FOCUS,GetMsg(mExistAcceptChoice),0,0},
            /* 7*/  {DI_BUTTON,0,9,0,0,{0},0,0,DIF_CENTERGROUP|DIF_DEFAULTBUTTON,GetMsg(mExistOverwrite),0,0},
            /* 8*/  {DI_BUTTON,0,9,0,0,{0},0,0,DIF_CENTERGROUP,GetMsg(mExistSkip),0,0},
            /* 9*/  {DI_BUTTON,0,9,0,0,{0},0,0,DIF_CENTERGROUP,GetMsg(mExistAppend),0,0},
            /*10*/  {DI_BUTTON,0,9,0,0,{0},0,0,DIF_CENTERGROUP,GetMsg(mExistCancel),0,0},
            };
            wchar_t FilenameText[512],SrcText[512],DstText[512];
            FSF.sprintf(FilenameText,L"%s",filename);
            FSF.TruncPathStr(FilenameText,56);
            DialogItems[1].Data=FilenameText;
            FSF.sprintf(SrcText,GetMsg(mExistSource),0ULL,0,0,0,0,0,0);
            DialogItems[3].Data=SrcText;
            {
              unsigned long long size;
              SYSTEMTIME mod;
              GetFileAttr(filename,&size,&mod);
              FSF.sprintf(DstText,GetMsg(mExistDestination),size,mod.wDay,mod.wMonth,mod.wYear,mod.wHour,mod.wMinute,mod.wSecond);
              DialogItems[4].Data=DstText;
            }
            CFarDialog dialog;
            intptr_t DlgCode=dialog.Execute(MainGuid,DlgExistsGuid,-1,-1,69,12,NULL,DialogItems,ArraySize(DialogItems),0,0,FileExistsDialogProc,0);
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
              int chk_res=CheckRetry(fInfo->PanelItem[i].FileName,filename);
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
              int chk_res=CheckRetry(fInfo->PanelItem[i].FileName,filename);
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
          int chk_res=CheckRetry(fInfo->PanelItem[i].FileName,filename);
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
        if(QVOpt.ShowHeader||!(fInfo->OpMode&OPM_QUICKVIEW))
        {
          FILETIME localtime;
          SYSTEMTIME time;
          FileTimeToLocalFileTime(&(fInfo->PanelItem[i].CreationTime),&localtime);
          FileTimeToSystemTime(&localtime,&time);
          FSF.sprintf(buff,L"%s: %02d.%02d.%04d\r\n%s: %02d:%02d:%02d\r\n",GetMsg(mFileDate),time.wDay,time.wMonth,time.wYear,GetMsg(mFileTime),time.wHour,time.wMinute,time.wSecond);
          WriteFile(file,buff,wcslen(buff)*sizeof(wchar_t),&transferred,NULL);
          FSF.sprintf(buff,L"%s: %s\r\n%s: %s\r\n%s: %ld\r\n",GetMsg(mFileUser),(fInfo->PanelItem[i].CustomColumnData?fInfo->PanelItem[i].CustomColumnData[4]:NULL),GetMsg(mFileComputer),(fInfo->PanelItem[i].CustomColumnData?fInfo->PanelItem[i].CustomColumnData[3]:NULL),GetMsg(mFileEventID),curr_rec->EventID&0xffff);
          WriteFile(file,buff,wcslen(buff)*sizeof(wchar_t),&transferred,NULL);
          FSF.sprintf(buff,L"%s: %s\r\n%s: %s\r\n%s: %s\r\n\r\n",GetMsg(mFileSource),fInfo->PanelItem[i].Description,GetMsg(mFileType),GetType(curr_rec->EventType),GetMsg(mFileCategory),(fInfo->PanelItem[i].CustomColumnData?fInfo->PanelItem[i].CustomColumnData[1]:NULL));
          WriteFile(file,buff,wcslen(buff)*sizeof(wchar_t),&transferred,NULL);
        }
        if(QVOpt.ShowDescription||!(fInfo->OpMode&OPM_QUICKVIEW))
        {
          FSF.sprintf(buff,L"%s:\r\n",GetMsg(mFileDescription));
          WriteFile(file,buff,wcslen(buff)*sizeof(wchar_t),&transferred,NULL);
          wchar_t *msg=FormatLogMessage(panel->path,curr_rec);
          if(msg) WriteFile(file,msg,wcslen(msg)*sizeof(wchar_t),&transferred,NULL);
          free(msg);
          FSF.sprintf(buff,L"\r\n\r\n");
          WriteFile(file,buff,wcslen(buff)*sizeof(wchar_t),&transferred,NULL);
        }
        if(QVOpt.ShowData||!(fInfo->OpMode&OPM_QUICKVIEW))
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
      if(!(fInfo->OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
      { 
        FSF.sprintf(copyname,L"%.55s ",filename);
        FSF.sprintf(progress,L"%3d%% ",(i+1)*100/fInfo->ItemsNumber);
        for(int j=0;j<50;j++)
          progress[5+j]=BOX1;
        progress[55]=0;
        for(size_t j=0;j<((i+1)*50/fInfo->ItemsNumber);j++)
          progress[5+j]=BOX2;
        Info.Message(&MainGuid,&CopyMessageGuid,firsttime?(firsttime=false,FMSG_LEFTALIGN):(FMSG_KEEPBACKGROUND|FMSG_LEFTALIGN),NULL,Items,ArraySize(Items),0);

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
      fInfo->PanelItem[i].Flags&=~PPIF_SELECTED;
      if(result==-1) break;
    }
    if(!(fInfo->OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
    {
      CloseHandle(console);
      Info.RestoreScreen(screen);
    }
  }
  else
  {
    for(size_t i=0;i<fInfo->ItemsNumber;i++)
    {
      if(IsDir)
        FSF.sprintf(filename,L"%s%s.evt",Dest,fInfo->PanelItem[i].FileName);
      else
        FSF.sprintf(filename,L"%s",Dest);
      HANDLE evt=OpenEventLog(panel->computer_ptr,fInfo->PanelItem[i].FileName); //REMOTE
      if(evt)
      {
        if(!BackupEventLog(evt,filename))
          if(!(fInfo->OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
            DISPLAY_WIN_ERROR
        CloseEventLog(evt);
      } else if(!(fInfo->OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
        DISPLAY_WIN_ERROR
    }
    if(fInfo->Move)
    {
      DeleteFilesInfo finfo;
      finfo.StructSize=sizeof(DeleteFilesInfo);
      finfo.hPanel=fInfo->hPanel;
      finfo.OpMode=fInfo->OpMode;
      finfo.PanelItem=fInfo->PanelItem;
      finfo.ItemsNumber=fInfo->ItemsNumber;
      DeleteFilesW(&finfo);
    }
  }
  return result;
}

intptr_t WINAPI DeleteFilesW(const struct DeleteFilesInfo *fInfo)
{
  EventViewer *panel=(EventViewer *)fInfo->hPanel;
  if(panel->level==0)
  {
    for(size_t i=0;i<fInfo->ItemsNumber;i++)
    {
      intptr_t MsgCode=0;
      if(!(fInfo->OpMode&(OPM_SILENT)))
      {
        wchar_t Msg[1024];
        FSF.sprintf(Msg,GetMsg(mClearLog),fInfo->PanelItem[i].FileName);
        const wchar_t *MsgItems[]={L"",Msg,GetMsg(mClearClear),GetMsg(mClearSkip),GetMsg(mClearCancel)};
        MsgCode=Info.Message(&MainGuid,&ClearMessageGuid,0,NULL,MsgItems,ArraySize(MsgItems),3);
      }
      if(MsgCode==0)
      {
        HANDLE evt=OpenEventLog(panel->computer_ptr,fInfo->PanelItem[i].FileName); //REMOTE
        if(evt)
        {
          if(!ClearEventLog(evt,NULL))
          {
            if(!(fInfo->OpMode&(OPM_SILENT)))
              DISPLAY_WIN_ERROR
          }
          else
          {
            CFarPanel pInfo(fInfo->hPanel,FCTL_GETPANELINFO);
            pInfo.StartSelection();
            for(size_t j=0;j<pInfo.ItemsNumber();j++)
              if(!FSF.LStricmp(pInfo[j].FileName,fInfo->PanelItem[i].FileName))
                pInfo.RemoveSelection(j);
            pInfo.CommitSelection();
          }
          CloseEventLog(evt);
        } else if(!(fInfo->OpMode&(OPM_SILENT)))
          DISPLAY_WIN_ERROR
      }
      else if((MsgCode==2)||(MsgCode==-1))
        break;
    }
    Info.PanelControl(fInfo->hPanel,FCTL_REDRAWPANEL,0,NULL);
    return TRUE;
  }
  return TRUE;
}

void WINAPI CloseAnalyseW(const struct CloseAnalyseInfo *aInfo)
{
  free((EventViewer *)aInfo->Handle);
}

HANDLE WINAPI AnalyseW(const struct AnalyseInfo *aInfo)
{
  if(!aInfo->FileName)
    return NULL;
  if(!Opt.BrowseEvtFiles)
    return NULL;
  if(!FSF.ProcessName(L"*.evt",(wchar_t*)aInfo->FileName, 0,PN_CMPNAMELIST))
    return NULL;
  return RealOpenFilePlugin(aInfo->FileName,aInfo->Buffer,aInfo->BufferSize);
}

static HANDLE RealOpenFilePlugin(const wchar_t *Name,const void *Data,size_t DataSize)
{
  HANDLE evt=OpenBackupEventLog(NULL,Name); //LOCAL
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

}

intptr_t WINAPI CompareW(const struct CompareInfo *cInfo)
{
  EventViewer *panel=(EventViewer *)cInfo->hPanel;
  if(((cInfo->Mode==SM_NAME)||(cInfo->Mode==SM_EXT))&&panel->level)
  {
    if((cInfo->Item1->UserData.Data)&&(cInfo->Item2->UserData.Data))
    {
      int res;
      if(Opt.ScanType==0)
        res=((EVENTLOGRECORD *)cInfo->Item1->UserData.Data)->RecordNumber-((EVENTLOGRECORD *)cInfo->Item2->UserData.Data)->RecordNumber;
      else
        res=((EVENTLOGRECORD *)cInfo->Item2->UserData.Data)->RecordNumber-((EVENTLOGRECORD *)cInfo->Item1->UserData.Data)->RecordNumber;
      res=res/abs(res);
      return res;
    }
  }
  return -2;
}

// C0: Event
// C1: Category
// C2: Time
// C3: Computer
// C4: User
// C5: Time
// Z : Source
static const wchar_t *GetTitles(const wchar_t *str)
{
  const wchar_t *Result=NULL;
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
}

void WINAPI GetOpenPanelInfoW(struct OpenPanelInfo *pInfo)
{
  EventViewer *panel=(EventViewer *)pInfo->hPanel;
  pInfo->StructSize=sizeof(*pInfo);
  pInfo->Flags=OPIF_ADDDOTS|OPIF_SHOWNAMESONLY;
  pInfo->HostFile=NULL;
  pInfo->CurDir=panel->path;
  if(panel->level==2)
  {
    pInfo->HostFile=panel->path;
    pInfo->CurDir=L"";
  }

  pInfo->Format=GetMsg(mName);

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
  pInfo->PanelTitle=Title;

  pInfo->InfoLines=NULL;
  pInfo->InfoLinesNumber=0;

  pInfo->DescrFiles=NULL;
  pInfo->DescrFilesNumber=0;

  static struct PanelMode PanelModesArray[10];
  static const wchar_t *ColumnTitles[10][32];
  int msg_start=(panel->level)?mPanelTypeFile0:mPanelTypeDir0;
  const wchar_t *tmp_msg;

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
            if(tmp_msg[0]-'0') PanelModesArray[i].Flags|=PMFLAGS_FULLSCREEN;
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
  pInfo->PanelModesArray=PanelModesArray;
  pInfo->PanelModesNumber=ArraySize(PanelModesArray);
  pInfo->StartPanelMode='3';

  static WORD FKeys[]=
	{
		VK_F7,0,
    VK_F6,0,
    VK_F8,0,
    VK_F8,SHIFT_PRESSED,
    VK_F5,0,
    VK_F5,SHIFT_PRESSED,
    VK_F1,SHIFT_PRESSED,
    VK_F2,SHIFT_PRESSED,
    VK_F3,SHIFT_PRESSED,
    VK_F4,SHIFT_PRESSED,
    VK_F6,SHIFT_PRESSED,
    VK_F6,RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED
	};
  static struct KeyBarLabel kbl[ARRAYSIZE(FKeys)/2];
	static struct KeyBarTitles kbt = {ARRAYSIZE(kbl), kbl};

  for (size_t j=0,i=0; i < ARRAYSIZE(FKeys); i+=2, ++j)
  {
    kbl[j].Key.VirtualKeyCode = FKeys[i];
    kbl[j].Key.ControlKeyState = FKeys[i+1];
  }
  kbl[0].Text=L"";

  if(panel->level==2)
    kbl[1].Text=L"";
  else
    kbl[1].Text=GetMsg(mKeyRemote);
  if(panel->level)
  {
    kbl[2].Text=L"";
    kbl[3].Text=L"";
  }
  else
  {
    if(!panel->computer_ptr)
    {
      kbl[4].Text=GetMsg(mKeyBackup);
      kbl[5].Text=GetMsg(mKeyBackup);
    }
    else
    {
      kbl[4].Text=L"";
      kbl[5].Text=L"";
    }
    kbl[2].Text=GetMsg(mKeyClear);
    kbl[3].Text=GetMsg(mKeyClear);
  }
  kbl[6].Text=L"";
  kbl[7].Text=L"";
  if(panel->level>0)
  {
    kbl[8].Text=GetMsg(mKeyViewData);
    kbl[9].Text=GetMsg(mKeyEditData);
  }
  else
  {
    kbl[8].Text=L"";
    kbl[9].Text=L"";
  }
  if(panel->level==2)
    kbl[10].Text=L"";
  else
    kbl[10].Text=GetMsg(mKeyLocal);
  kbl[11].Text=L"";
  pInfo->KeyBar=&kbt;
}

static intptr_t WINAPI ComputerDialogProc(HANDLE hDlg, intptr_t Msg,intptr_t Param1,void* Param2)
{
  switch(Msg)
  {
    case DN_INITDIALOG:
			int length=MAX_PATH-3;
      Info.SendDlgMessage(hDlg,DM_SETMAXTEXTLENGTH,2,&length);
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

#define ControlKeyAllMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED|RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED)
#define ControlKeyAltMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED)
#define ControlKeyNonAltMask (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED)
#define IsShift(rec) static_cast<bool>(((rec).Event.KeyEvent.dwControlKeyState&ControlKeyAllMask)==SHIFT_PRESSED)
#define IsNone(rec) static_cast<bool>(((rec).Event.KeyEvent.dwControlKeyState&ControlKeyAllMask)==0)
#define IsAlt(rec) static_cast<bool>(((rec).Event.KeyEvent.dwControlKeyState&ControlKeyAltMask)&&!((rec).Event.KeyEvent.dwControlKeyState&ControlKeyNonAltMask))

intptr_t WINAPI ProcessPanelInputW(const struct ProcessPanelInputInfo *ppiInfo)
{
  if (ppiInfo->Rec.EventType!=KEY_EVENT)
  {
    return false;
  }

  EventViewer *panel=(EventViewer *)ppiInfo->hPanel;
  WORD Key=ppiInfo->Rec.Event.KeyEvent.wVirtualKeyCode;

  if((IsShift(ppiInfo->Rec))&&(Key==VK_F4||Key==VK_F3)) //skip Shift-F4
  {
    if(panel->level>0)
    {
      CFarPanel pInfo(ppiInfo->hPanel,FCTL_GETPANELINFO);
      if(pInfo.ItemsNumber()&&(pInfo[pInfo.CurrentItem()].UserData.Data))
      {
        wchar_t temp[MAX_PATH],tempfile[MAX_PATH];
        if(GetTempPath(MAX_PATH,temp)&&GetTempFileName(temp,L"evt",0,tempfile))
        {
          HANDLE hdata=CreateFile(tempfile,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
          if(hdata!=INVALID_HANDLE_VALUE)
          {
            DWORD dWritten;
            EVENTLOGRECORD *curr_rec=(EVENTLOGRECORD *)(pInfo[pInfo.CurrentItem()].UserData.Data);
            if(curr_rec->DataLength)
              WriteFile(hdata,(unsigned char*)curr_rec+curr_rec->DataOffset,curr_rec->DataLength,&dWritten,NULL);
            CloseHandle(hdata);
            if(Key==VK_F3)
              Info.Viewer(tempfile,NULL,0,0,-1,-1,VF_DELETEONLYFILEONCLOSE|VF_ENABLE_F6|VF_DISABLEHISTORY|VF_NONMODAL|VF_IMMEDIATERETURN,CP_DEFAULT);
            else
              Info.Editor(tempfile,NULL,0,0,-1,-1,EF_DELETEONLYFILEONCLOSE|EF_ENABLE_F6|EF_DISABLEHISTORY|EF_NONMODAL|EF_IMMEDIATERETURN,1,1,CP_DEFAULT);
          }
        }
      }
    }
    return TRUE;
  }
  if((IsNone(ppiInfo->Rec))&&(Key==VK_F6)) //skip F6
  {
    if(panel->level<2)
    {
      /*
        000000000011111111112222222222333333333344444444
        012345678901234567890123456789012345678901234567
      00                                                00
      01   ÉÍÍÍÍÍÍÍÍÍÍÍ Select Computer ÍÍÍÍÍÍÍÍÍÍÍÍ»   01
      02   º Computer:                              º   02
      03   º ZG                                    ³º   03
      04   º (empty for local system)               º   04
      05   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   05
      06                                                06
        000000000011111111112222222222333333333344444444
        012345678901234567890123456789012345678901234567
      */
      static const wchar_t *NTEventComputerHistoryName=L"NTEventComputer";
      static struct FarDialogItem DialogItems[]={
      /*0*/  {DI_DOUBLEBOX,3,1,44,5,{0},0,0,0,GetMsg(mSelCompTitle),0,0},
      /*1*/  {DI_TEXT,5,2,0,0,{0},0,0,0,GetMsg(mSelCompLabel),0,0},
      /*2*/  {DI_EDIT,5,3,42,0,{0},NTEventComputerHistoryName,0,DIF_HISTORY|DIF_FOCUS,L"",0,0},
      /*3*/  {DI_TEXT,5,4,0,0,{0},0,0,0,GetMsg(mSelCompFootnote),0,0},
      };
      DialogItems[2].Data=panel->computer_oem;
      CFarDialog dialog;
      intptr_t DlgCode=dialog.Execute(MainGuid,SelectCompGuid,-1,-1,48,7,NULL,DialogItems,ArraySize(DialogItems),0,0,ComputerDialogProc,0);
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
            Info.PanelControl(ppiInfo->hPanel,FCTL_UPDATEPANEL,0,0);
            Info.PanelControl(ppiInfo->hPanel,FCTL_REDRAWPANEL,0,NULL);
          }
          else
          {
            const wchar_t *MsgItems[]={GetMsg(mError),GetMsg(mErr2),GetMsg(mOk)};
            Info.Message(&MainGuid,&Error2MessageGuid,FMSG_WARNING,NULL,MsgItems,ArraySize(MsgItems),1);
          }
        }
        else
        {
          wcscpy(panel->computer,L"");
          wcscpy(panel->computer_oem,L"");
          panel->computer_ptr=NULL;
          Info.PanelControl(ppiInfo->hPanel,FCTL_UPDATEPANEL,0,0);
          Info.PanelControl(ppiInfo->hPanel,FCTL_REDRAWPANEL,0,NULL);
        }
      }
    }
    return TRUE;
  }
  if((IsShift(ppiInfo->Rec))&&(Key==VK_F6)) //skip Shift-F6
  {
    wcscpy(panel->computer,L"");
    wcscpy(panel->computer_oem,L"");
    panel->computer_ptr=NULL;
    Info.PanelControl(ppiInfo->hPanel,FCTL_UPDATEPANEL,0,0);
    Info.PanelControl(ppiInfo->hPanel,FCTL_REDRAWPANEL,0,NULL);
    return TRUE;
  }
  if((IsNone(ppiInfo->Rec))&&(Key==VK_F7)) //skip F7
    return TRUE;
  if((((IsNone(ppiInfo->Rec))||(IsShift(ppiInfo->Rec)))&&(Key==VK_F8))||(((IsAlt(ppiInfo->Rec))||(IsShift(ppiInfo->Rec)))&&(Key==VK_DELETE))) //skip F8,Shift-F8,Alt-Del,Shift-Del
  {
    if(panel->level>0)
      return TRUE;
  }
  return FALSE;
}

intptr_t WINAPI ConfigureW(const struct ConfigureInfo *Info)
{
  return Config();
}

intptr_t WINAPI ProcessPanelEventW(const struct ProcessPanelEventInfo *eInfo)
{
  EventViewer *panel=(EventViewer *)eInfo->hPanel;
  if(eInfo->Event==FE_REDRAW&&panel->redraw)
  {
    panel->redraw=FALSE;
    if(State.ViewMode!=-1)
      Info.PanelControl(eInfo->hPanel,FCTL_SETVIEWMODE,State.ViewMode,0);
    if(State.SortMode!=-1)
      Info.PanelControl(eInfo->hPanel,FCTL_SETSORTMODE,State.SortMode,0);
    if(State.SortOrder!=-1)
      Info.PanelControl(eInfo->hPanel,FCTL_SETSORTORDER,State.SortOrder,0);
    PanelRedrawInfo ri={sizeof(PanelRedrawInfo),State.Current,State.Top};
    Info.PanelControl(eInfo->hPanel,FCTL_REDRAWPANEL,0,&ri);
    return TRUE;
  }
  else if(eInfo->Event==FE_CLOSE&&panel->level<2)
  {
    PanelInfo PInfo;
    PInfo.StructSize=sizeof(PanelInfo);
    Info.PanelControl(eInfo->hPanel,FCTL_GETPANELINFO,0,&PInfo);
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
      PluginSettings settings(MainGuid,::Info.SettingsControl);
      settings.Set(0,L"StatePath",State.Path);
      settings.Set(0,L"StateComputer",State.Computer);
      settings.Set(0,L"StateCurrent",State.Current);
      settings.Set(0,L"StateTop",State.Top);
      settings.Set(0,L"StateViewMode",State.ViewMode);
      settings.Set(0,L"StateSortMode",State.SortMode);
      settings.Set(0,L"StateSortOrder",State.SortOrder);
    }
  }
  return FALSE;
}

void WINAPI ExitFARW(const struct ExitInfo *Info)
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
