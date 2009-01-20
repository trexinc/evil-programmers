#include <stdio.h>
#include <stdlib.h>
#include "far_helper.h"
#include "farcolor.hpp"
#include "evplugin.h"
#include "memory.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
TCHAR PluginRootKey[80];
BOOL IsOldFAR=TRUE;

struct Options {
  BOOL AddToDisksMenu;
  int DisksMenuDigit;
  BOOL AddToPluginsMenu;
  BOOL AddToConfigMenu;
  BOOL BrowseEvtFiles;
  BOOL StripExt;
  BOOL Restore;
  DWORD ScanType;
  TCHAR Prefix[16];
} Opt={FALSE,0,TRUE,TRUE,TRUE,TRUE,TRUE,0,_T("evt")};

struct QVOptions {
  BOOL ShowHeader;
  BOOL ShowDescription;
  BOOL ShowData;
} QVOpt={FALSE,TRUE,TRUE};

struct TechOptions
{
  TCHAR Separator[1024];
  BOOL NetBackup;
} TechOpt;

struct PluginState
{
  TCHAR Path[MAX_PATH];
  TCHAR Computer[MAX_PATH];
  int Current;
  int Top;
  int ViewMode;
  int SortMode;
  int SortOrder;
} State={_T(""),_T(""),0,0,-1,-1,-1};

static TCHAR *GetMsg(int MsgId)
{
  return (TCHAR *)Info.GetMsg(Info.ModuleNumber,MsgId);
}

static const TCHAR *default_column_data=_T("");

struct InitDialogItem
{
  int Type;
  int X1, Y1, X2, Y2;
  int Focus;
  int Selected;
  unsigned int Flags;
  int DefaultButton;
  const TCHAR *Data;
};

static void InitDialogItems(InitDialogItem *Init,FarDialogItem *Item,int ItemsNumber)
{
  for (int i=0;i<ItemsNumber;i++)
  {
    Item[i].Type=Init[i].Type;
    Item[i].X1=Init[i].X1;
    Item[i].Y1=Init[i].Y1;
    Item[i].X2=Init[i].X2;
    Item[i].Y2=Init[i].Y2;
    Item[i].Focus=Init[i].Focus;
    Item[i].Selected=Init[i].Selected;
    Item[i].Flags=Init[i].Flags;
    Item[i].DefaultButton=Init[i].DefaultButton;
#ifdef UNICODE
    Item[i].MaxLen=0;
#endif
    if((unsigned)Init[i].Data<2000)
#ifdef UNICODE
      Item[i].PtrData=GetMsg((unsigned int)(DWORD_PTR)Init[i].Data);
#else
      _tcscpy(Item[i].Data,GetMsg((unsigned int)(DWORD_PTR)Init[i].Data));
#endif
    else
#ifdef UNICODE
      Item[i].PtrData=Init[i].Data;
#else
      _tcscpy(Item[i].Data,Init[i].Data);
#endif
  }
}

static bool CheckRemoteEventLog(TCHAR *computer)
{
  HANDLE hSScr=Info.SaveScreen(0,0,-1,-1);
  const TCHAR *MsgItems[]={_T(""),GetMsg(mOtherConnecting)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,ArraySize(MsgItems),0);
  HANDLE evt=OpenEventLog(computer,_T("System")); //REMOTE
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
  const TCHAR *MsgItems[]={GetMsg(mError),GetMsg(mOk)}; \
  Info.Message(Info.ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,ArraySize(MsgItems),1); \
}

static HANDLE RealOpenFilePlugin(const TCHAR *Name,const unsigned char *Data,int DataSize);

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
  memset(&::Info, 0, sizeof(::Info));
  memmove(&::Info, Info, (Info->StructSize > (int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
#ifndef UNICODE
  if(Info->StructSize > FAR165_INFO_SIZE)
  {
#endif
    IsOldFAR=FALSE;
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;

    _tcscpy(PluginRootKey,Info->RootKey);
    _tcscat(PluginRootKey,_T("\\ntevent"));
    TechOpt.NetBackup=FALSE;
    TechOpt.Separator[0]=0;
    HKEY hKey;
    DWORD Type;
    DWORD DataSize=0;
    if((RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
    {
      //tech options
      DataSize=sizeof(Opt.AddToConfigMenu);
      RegQueryValueEx(hKey,_T("AddToConfigMenu"),0,&Type,(LPBYTE)&Opt.AddToConfigMenu,&DataSize);
      //state options
      DataSize=sizeof(State.Path);
      RegQueryValueEx(hKey,_T("StatePath"),0,&Type,(LPBYTE)State.Path,&DataSize);
      DataSize=sizeof(State.Computer);
      RegQueryValueEx(hKey,_T("StateComputer"),0,&Type,(LPBYTE)State.Computer,&DataSize);
      DataSize=sizeof(State.Current);
      RegQueryValueEx(hKey,_T("StateCurrent"),0,&Type,(LPBYTE)&State.Current,&DataSize);
      DataSize=sizeof(State.Top);
      RegQueryValueEx(hKey,_T("StateTop"),0,&Type,(LPBYTE)&State.Top,&DataSize);
      DataSize=sizeof(State.ViewMode);
      RegQueryValueEx(hKey,_T("StateViewMode"),0,&Type,(LPBYTE)&State.ViewMode,&DataSize);
      DataSize=sizeof(State.SortMode);
      RegQueryValueEx(hKey,_T("StateSortMode"),0,&Type,(LPBYTE)&State.SortMode,&DataSize);
      DataSize=sizeof(State.SortOrder);
      RegQueryValueEx(hKey,_T("StateSortOrder"),0,&Type,(LPBYTE)&State.SortOrder,&DataSize);
      //main options
      DataSize=sizeof(Opt.AddToDisksMenu);
      RegQueryValueEx(hKey,_T("AddToDisksMenu"),0,&Type,(LPBYTE)&Opt.AddToDisksMenu,&DataSize);
      DataSize=sizeof(Opt.DisksMenuDigit);
      RegQueryValueEx(hKey,_T("DisksMenuDigit"),0,&Type,(LPBYTE)&Opt.DisksMenuDigit,&DataSize);
      DataSize=sizeof(Opt.AddToPluginsMenu);
      RegQueryValueEx(hKey,_T("AddToPluginsMenu"),0,&Type,(LPBYTE)&Opt.AddToPluginsMenu,&DataSize);
      DataSize=sizeof(Opt.BrowseEvtFiles);
      RegQueryValueEx(hKey,_T("BrowseEvtFiles"),0,&Type,(LPBYTE)&Opt.BrowseEvtFiles,&DataSize);
      DataSize=sizeof(Opt.StripExt);
      RegQueryValueEx(hKey,_T("StripExt"),0,&Type,(LPBYTE)&Opt.StripExt,&DataSize);
      DataSize=sizeof(Opt.ScanType);
      RegQueryValueEx(hKey,_T("ScanType"),0,&Type,(LPBYTE)&Opt.ScanType,&DataSize);
      DataSize=sizeof(Opt.Prefix);
      RegQueryValueEx(hKey,_T("Prefix"),0,&Type,(LPBYTE)Opt.Prefix,&DataSize);
      DataSize=sizeof(Opt.Restore);
      RegQueryValueEx(hKey,_T("Restore"),0,&Type,(LPBYTE)&Opt.Restore,&DataSize);
      //QuickView
      DataSize=sizeof(QVOpt.ShowHeader);
      RegQueryValueEx(hKey,_T("ShowHeader"),0,&Type,(LPBYTE)&QVOpt.ShowHeader,&DataSize);
      DataSize=sizeof(QVOpt.ShowDescription);
      RegQueryValueEx(hKey,_T("ShowDescription"),0,&Type,(LPBYTE)&QVOpt.ShowDescription,&DataSize);
      DataSize=sizeof(QVOpt.ShowData);
      RegQueryValueEx(hKey,_T("ShowData"),0,&Type,(LPBYTE)&QVOpt.ShowData,&DataSize);
      //Tech
      DataSize=sizeof(TechOpt.Separator);
      RegQueryValueEx(hKey,_T("Separator"),0,&Type,(LPBYTE)TechOpt.Separator,&DataSize);
      DataSize=sizeof(TechOpt.NetBackup);
      RegQueryValueEx(hKey,_T("NetBackup"),0,&Type,(LPBYTE)&TechOpt.NetBackup,&DataSize);

      RegCloseKey(hKey);
      if((Opt.ScanType!=0)&&(Opt.ScanType!=1))
        Opt.ScanType=0;
      if(!Opt.Prefix[0])
        FSF.sprintf(Opt.Prefix,_T("%s"),_T("evt"));
    }
#ifndef UNICODE
  }
#endif
}

void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
  if(!IsOldFAR)
  {
    Info->StructSize=sizeof(*Info);
    Info->Flags=0;

    static TCHAR *DisksMenuStrings[1];
    DisksMenuStrings[0]=GetMsg(mNameDisk);
    static int DisksMenuNumbers[1];
    Info->DiskMenuStrings=DisksMenuStrings;
    DisksMenuNumbers[0]=Opt.DisksMenuDigit;
    Info->DiskMenuNumbers=DisksMenuNumbers;
    Info->DiskMenuStringsNumber=Opt.AddToDisksMenu?1:0;

    static TCHAR *PluginMenuStrings[1];
    PluginMenuStrings[0]=GetMsg(mName);
    Info->PluginMenuStrings=PluginMenuStrings;
    Info->PluginMenuStringsNumber=Opt.AddToPluginsMenu?(ArraySize(PluginMenuStrings)):0;
    Info->PluginConfigStrings=PluginMenuStrings;
    Info->PluginConfigStringsNumber=Opt.AddToConfigMenu?(ArraySize(PluginMenuStrings)):0;
    Info->CommandPrefix=Opt.Prefix;
  }
}

HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom,int Item)
{
  if(IsOldFAR)
    return INVALID_HANDLE_VALUE;
  TCHAR *cmd=(TCHAR *)Item;
  if((OpenFrom==OPEN_COMMANDLINE)&&(_tcslen(cmd)))
  {
    TCHAR Name[MAX_PATH],FullName[MAX_PATH],*File;
    FSF.Unquote(FSF.Trim(_tcscpy(Name,cmd)));
    if(GetFullPathName(Name,ArraySize(FullName),FullName,&File))
      return RealOpenFilePlugin(FullName,NULL,0);
    else
      return RealOpenFilePlugin(Name,NULL,0);
  }
  else
  {
    EventViewer *panel=(EventViewer *)malloc(sizeof(EventViewer));
    if(!panel)
      return INVALID_HANDLE_VALUE;
    panel->level=0;
    _tcscpy(panel->path,_T(""));
    _tcscpy(panel->computer,_T(""));
    _tcscpy(panel->computer_oem,_T(""));
    panel->computer_ptr=NULL;
    panel->redraw=Opt.Restore;
    if(Opt.Restore)
    {
      _tcscpy(panel->path,State.Path);
      if(_tcslen(panel->path)) panel->level=1;
      if(_tcslen(State.Computer))
      {
        TCHAR temp_computer_name[MAX_PATH]; t_OemToChar(State.Computer,temp_computer_name);
        if(CheckRemoteEventLog(temp_computer_name))
        {
          _tcscpy(panel->computer,temp_computer_name);
          _tcscpy(panel->computer_oem,State.Computer);
          panel->computer_ptr=panel->computer;
        }
      }
    }
    return (HANDLE)panel;
  }
}

void WINAPI EXP_NAME(ClosePlugin)(HANDLE hPlugin)
{
  free((EventViewer *)hPlugin);
}

int WINAPI EXP_NAME(GetFindData)(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  if(!IsOldFAR)
  {
    HANDLE hSScr=Info.SaveScreen(0,0,-1,-1),console=INVALID_HANDLE_VALUE;
    if(!(OpMode&(OPM_FIND)))
    {
      const TCHAR *MsgItems[]={_T(""),GetMsg(mOtherScanning)};
      Info.Message(Info.ModuleNumber,0,NULL,MsgItems,ArraySize(MsgItems),0);
      console=CreateFile(_T("CONIN$"),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    }
    EventViewer *panel=(EventViewer *)hPlugin;
    HANDLE evt=INVALID_HANDLE_VALUE; DWORD scan=EVENTLOG_FORWARDS_READ;
    if(Opt.ScanType==1) scan=EVENTLOG_BACKWARDS_READ;
    *pPanelItem=NULL; *pItemsNumber=0;
    TCHAR path_ansi[MAX_PATH];
    t_OemToChar(panel->path,path_ansi);
    switch(panel->level)
    {
      case 0:
      {
        HKEY hKey=NULL;
        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,EVENTLOG_KEY,0,KEY_READ,&hKey)==ERROR_SUCCESS)
        {
          TCHAR NameBuffer[MAX_PATH]; LONG Result;
          for(int i=0;;i++)
          {
            Result=RegEnumKey(hKey,i,NameBuffer,ArraySize(NameBuffer));
            if(Result==ERROR_NO_MORE_ITEMS)
              break;
            if(Result==ERROR_SUCCESS)
            {
#ifndef UNICODE
              CharToOem(NameBuffer,NameBuffer);
#endif
              struct PluginPanelItem *pNewPanelItem=(PluginPanelItem *)realloc(*pPanelItem,sizeof(PluginPanelItem)*((*pItemsNumber)+1));
              if(pNewPanelItem)
              {
                *pPanelItem=pNewPanelItem;
                PluginPanelItem *curr=(*pPanelItem)+(*pItemsNumber)++;
                memset(curr,0,sizeof(PluginPanelItem));
#ifdef UNICODE
                curr->FindData.PANEL_FILENAME=(TCHAR*)malloc((_tcslen(NameBuffer)+1)*sizeof(TCHAR));
                if(curr->FindData.PANEL_FILENAME) _tcscpy((TCHAR*)curr->FindData.PANEL_FILENAME,NameBuffer);
#else
                _tcscpy(curr->FindData.cFileName,NameBuffer);
#endif
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
#ifdef UNICODE
              (*pPanelItem)->FindData.PANEL_FILENAME=(TCHAR*)malloc(3*sizeof(TCHAR));
              _tcscpy((TCHAR*)((*pPanelItem)->FindData.PANEL_FILENAME),_T(".."));
#else
              FSF.sprintf((*pPanelItem)->FindData.cFileName,_T("%s"),_T(".."));
#endif
              (*pPanelItem)->CustomColumnData=(TCHAR **)malloc(sizeof(TCHAR *)*CUSTOM_COLUMN_COUNT);
              if((*pPanelItem)->CustomColumnData)
              {
                (*pPanelItem)->CustomColumnData[0]=(TCHAR *)malloc(sizeof(TCHAR));
                (*pPanelItem)->CustomColumnData[1]=(TCHAR *)malloc(sizeof(TCHAR));
                (*pPanelItem)->CustomColumnData[2]=(TCHAR *)malloc(sizeof(TCHAR)*3);
                if((*pPanelItem)->CustomColumnData[2])
                  FSF.sprintf((*pPanelItem)->CustomColumnData[2],_T("%s"),_T(".."));
                (*pPanelItem)->CustomColumnData[3]=(TCHAR *)malloc(sizeof(TCHAR));
                (*pPanelItem)->CustomColumnData[4]=(TCHAR *)malloc(sizeof(TCHAR));
                (*pPanelItem)->CustomColumnData[5]=(TCHAR*)_T("");
                for(int i=0;i<(CUSTOM_COLUMN_COUNT-1);i++)
                  if(!(*pPanelItem)->CustomColumnData[i])
                    (*pPanelItem)->CustomColumnData[i]=(TCHAR*)default_column_data;
                (*pPanelItem)->CustomColumnNumber=CUSTOM_COLUMN_COUNT;
              }
              for(int i=1;i<(*pItemsNumber);i++)
              {
#ifdef UNICODE
                ((*pPanelItem)+i)->FindData.PANEL_FILENAME=(TCHAR*)malloc(512*sizeof(TCHAR));
                FSF.sprintf((TCHAR*)(((*pPanelItem)+i)->FindData.PANEL_FILENAME),_T("%s.%s.%s"),GetMsg(mExtNameError),GetMsg(mExtError),GetMsg(mExtMain));
#else
                FSF.sprintf(((*pPanelItem)+i)->FindData.cFileName,_T("%s.%s.%s"),GetMsg(mExtNameError),GetMsg(mExtError),GetMsg(mExtMain));
#endif
              }

              unsigned long long evt_date_time,evt_date_time_local;
              void *user_data; const TCHAR *suffix;
              //read events
              EVENTLOGRECORD *curr_rec;
              TCHAR *buff=NULL;
              DWORD buffsize=BIG_BUFFER,readed,needed;
              buff=(TCHAR *)malloc(buffsize);
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
                      buff=(TCHAR *)malloc(buffsize);
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
                      TCHAR *description=(TCHAR *)(((EVENTLOGRECORD *)user_data)+1);
                      ((*pPanelItem)+i)->Description=(TCHAR *)malloc((_tcslen(description)+1)*sizeof(TCHAR));
                      if(((*pPanelItem)+i)->Description)
                      {
                        FSF.sprintf(((*pPanelItem)+i)->Description,_T("%s"),description);
#ifndef UNICODE
                        CharToOem(((*pPanelItem)+i)->Description,((*pPanelItem)+i)->Description);
#endif
                      }
                    }
                    suffix=_T("");
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
#ifdef UNICODE
                    ((*pPanelItem)+i)->FindData.PANEL_FILENAME=(TCHAR*)malloc(512*sizeof(TCHAR));
                    FSF.sprintf((TCHAR*)(((*pPanelItem)+i)->FindData.PANEL_FILENAME),_T("%08ld.%s.%s"),curr_rec->RecordNumber,suffix,GetMsg(mExtMain));
                    ((*pPanelItem)+i)->FindData.nFileSize=curr_rec->Length;
#else
                    FSF.sprintf(((*pPanelItem)+i)->FindData.cFileName,_T("%08ld.%s.%s"),curr_rec->RecordNumber,suffix,GetMsg(mExtMain));
                    ((*pPanelItem)+i)->FindData.nFileSizeLow=curr_rec->Length;
#endif
                    evt_date_time=curr_rec->TimeWritten; evt_date_time=evt_date_time*10000000ULL+EVENT_START_TIME;
  //                  FileTimeToLocalFileTime((FILETIME *)&evt_date_time,(FILETIME *)&evt_date_time_local);
                    memcpy(&(((*pPanelItem)+i)->FindData.ftLastWriteTime),&evt_date_time,sizeof(evt_date_time));
                    evt_date_time=curr_rec->TimeGenerated; evt_date_time=evt_date_time*10000000ULL+EVENT_START_TIME;
                    memcpy(&(((*pPanelItem)+i)->FindData.ftCreationTime),&evt_date_time,sizeof(evt_date_time));
                    FileTimeToLocalFileTime((FILETIME *)&evt_date_time,(FILETIME *)&evt_date_time_local);
                    ((*pPanelItem)+i)->CustomColumnData=(TCHAR **)malloc(sizeof(TCHAR *)*CUSTOM_COLUMN_COUNT);
                    if(((*pPanelItem)+i)->CustomColumnData)
                    {
                      ((*pPanelItem)+i)->CustomColumnData[0]=(TCHAR *)malloc(sizeof(TCHAR)*6);
                      if(((*pPanelItem)+i)->CustomColumnData[0])
                        FSF.sprintf(((*pPanelItem)+i)->CustomColumnData[0],_T("%5ld"),curr_rec->EventID&0xffff);
                      const TCHAR *category=GetCategory(curr_rec);
                      ((*pPanelItem)+i)->CustomColumnData[1]=(TCHAR *)malloc((_tcslen(category)+1)*sizeof(TCHAR));
                      if(((*pPanelItem)+i)->CustomColumnData[1])
                        _tcscpy(((*pPanelItem)+i)->CustomColumnData[1],category);
                      ((*pPanelItem)+i)->CustomColumnData[2]=(TCHAR *)malloc(sizeof(TCHAR)*20);
                      if(((*pPanelItem)+i)->CustomColumnData[2])
                      {
                        SYSTEMTIME time;
                        FileTimeToSystemTime((FILETIME *)&evt_date_time_local,&time);
                        FSF.sprintf(((*pPanelItem)+i)->CustomColumnData[2],_T("%02d.%02d.%04d %02d:%02d:%02d"),time.wDay,time.wMonth,time.wYear,time.wHour,time.wMinute,time.wSecond);
                      }
                      TCHAR *compname=GetComputerName(curr_rec);
                      ((*pPanelItem)+i)->CustomColumnData[3]=(TCHAR *)malloc(sizeof(TCHAR)*(_tcslen(compname)+1));
                      if(((*pPanelItem)+i)->CustomColumnData[3])
                        _tcscpy(((*pPanelItem)+i)->CustomColumnData[3],compname);
                      TCHAR *username=GetUserName(panel->computer_ptr,curr_rec);
                      ((*pPanelItem)+i)->CustomColumnData[4]=(TCHAR *)malloc(sizeof(TCHAR)*(_tcslen(username)+1));
                      if(((*pPanelItem)+i)->CustomColumnData[4])
                        _tcscpy(((*pPanelItem)+i)->CustomColumnData[4],username);
                      ((*pPanelItem)+i)->CustomColumnData[5]=((*pPanelItem)+i)->CustomColumnData[2];
                      for(int j=0;j<(CUSTOM_COLUMN_COUNT-1);j++)
                        if(!((*pPanelItem)+i)->CustomColumnData[j])
                          ((*pPanelItem)+i)->CustomColumnData[j]=(TCHAR*)default_column_data;
                      ((*pPanelItem)+i)->CustomColumnNumber=CUSTOM_COLUMN_COUNT;
                    }
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
    return TRUE;
  }
  return FALSE;
}

void WINAPI EXP_NAME(FreeFindData)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  for(int i=0;i<ItemsNumber;i++)
  {
    free((void *)PanelItem[i].UserData);
    free(PanelItem[i].Owner);
    free(PanelItem[i].Description);
    if(PanelItem[i].CustomColumnData)
      for(int j=0;j<(CUSTOM_COLUMN_COUNT-1);j++)
        if(PanelItem[i].CustomColumnData[j]!=(TCHAR*)default_column_data)
          free(PanelItem[i].CustomColumnData[j]);
    free(PanelItem[i].CustomColumnData);
#ifdef UNICODE
    free(PanelItem[i].FindData.lpwszFileName);
#endif
  }
  free(PanelItem);
}

int WINAPI EXP_NAME(SetDirectory)(HANDLE hPlugin,const TCHAR *Dir,int OpMode)
{
  if(!IsOldFAR)
  {
    int res=TRUE;
    EventViewer *panel=(EventViewer *)hPlugin;
    if(panel->level==2)
      return TRUE;
    if(!_tcscmp(Dir,_T("\\")))
    {
      panel->level=0;
      _tcscpy(panel->path,_T(""));
    }
    else if(!_tcscmp(Dir,_T("..")))
    {
      if(panel->level==1)
      {
        panel->level=0;
        _tcscpy(panel->path,_T(""));
      }
      else res=FALSE;
    }
    else if((panel->level==0)&&(CheckLogName(panel,Dir)))
    {
      panel->level=1;
      _tcscpy(panel->path,Dir);
    }
    else res=FALSE;
    return res;
  }
  return FALSE;
}

static long WINAPI CopyDialogProc(HANDLE hDlg, int Msg,int Param1,long Param2)
{
  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_SETTEXTLENGTH,2,MAX_PATH-1);
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

static BOOL GetDestDir(TCHAR *dir,int move)
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
  static const TCHAR *NTEventCopyHistoryName=_T("NTEventCopy");
  static struct InitDialogItem InitDlg[]={
  /*0*/  {DI_DOUBLEBOX,3,1,72,6,0,0,0,0,_T("")},
  /*1*/  {DI_TEXT,5,2,0,0,0,0,0,0,_T("")},
  /*2*/  {DI_EDIT,5,3,70,0,1,(DWORD)NTEventCopyHistoryName,DIF_HISTORY,0,_T("")},
  /*3*/  {DI_TEXT,5,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
  /*4*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,1,_T("")},
  /*5*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)mCpyDlgCancel}
  };
  struct FarDialogItem DialogItems[ArraySize(InitDlg)];
  InitDialogItems(InitDlg,DialogItems,ArraySize(InitDlg));
  PanelInfo PInfo;
  Info.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&PInfo); //FIXME
  INIT_DLG_DATA(DialogItems[0],GetMsg(mCpyDlgCopyTitle+move));
#ifdef UNICODE
  TCHAR CopyText[512];
  FSF.sprintf(CopyText,GetMsg(mCpyDlgCopyToN+3*move+NumberType(PInfo.SelectedItemsNumber)),PInfo.SelectedItemsNumber);
  DialogItems[1].PtrData=CopyText;
#else
  FSF.sprintf(DialogItems[1].Data,GetMsg(mCpyDlgCopyToN+3*move+NumberType(PInfo.SelectedItemsNumber)),PInfo.SelectedItemsNumber);
#endif
  INIT_DLG_DATA(DialogItems[2],dir);
  INIT_DLG_DATA(DialogItems[4],GetMsg(mCpyDlgCopyOk+move));
  CFarDialog dialog;
  int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,76,8,NULL,DialogItems,(ArraySize(InitDlg)),0,0,CopyDialogProc,0);
  if(DlgCode==4)
  {
    FSF.sprintf(dir,_T("%s"),dialog.Str(2));
    return TRUE;
  }
  else
    return FALSE;
}

static void GetFileAttr(TCHAR *file,unsigned long long *size,SYSTEMTIME *mod)
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

static int CheckRetry(TCHAR *afrom,TCHAR *ato)
{
  TCHAR from[512],to[512],buff[MAX_PATH];
  FSF.sprintf(buff,_T("%s"),afrom);
  FSF.TruncPathStr(buff,55);
  FSF.sprintf(from,GetMsg(mRetryFrom),buff);
  FSF.sprintf(buff,_T("%s"),ato);
  FSF.TruncPathStr(buff,55);
  FSF.sprintf(to,GetMsg(mRetryTo),buff);
  const TCHAR *MsgItems[]={GetMsg(mRetryError),from,to,GetMsg(mRetryRetry),GetMsg(mRetrySkip),GetMsg(mRetryCancel)};
  return Info.Message(Info.ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,ArraySize(MsgItems),3);
}

static long WINAPI FileExistsDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
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
      TCHAR Buff[512]; SYSTEMTIME mod;
      GetLocalTime(&mod);
      FSF.sprintf(Buff,GetMsg(mExistSource),0ULL,mod.wDay,mod.wMonth,mod.wYear,mod.wHour,mod.wMinute,mod.wSecond);
      Caption.PtrData=Buff;
      Caption.PtrLength=_tcslen(Caption.PtrData);
      Info.SendDlgMessage(hDlg,DM_SETTEXT,3,(long)&Caption);
      //refresh
      Info.SendDlgMessage(hDlg,DM_SETREDRAW,0,0);
      break;
    }
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}


#ifdef UNICODE
#define _D(p) (*p)
#else
#define _D(p) (p)
#endif

int WINAPI EXP_NAME(GetFiles)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,
#ifdef UNICODE
const TCHAR **
#else
TCHAR *
#endif
DestPath,int OpMode)
{
  if(!IsOldFAR)
  {
    EventViewer *panel=(EventViewer *)hPlugin;
    int OpMode2=OpMode&(~OPM_TOPLEVEL);
    if((!panel->level)&&(panel->computer_ptr)&&(!TechOpt.NetBackup)) return 0; //no remote backup
#ifdef UNICODE
    static TCHAR DestPathReal[MAX_PATH];
    _tcscpy(DestPathReal,*DestPath);
    *DestPath=DestPathReal;
#endif
    ;
    TCHAR Dir[MAX_PATH],Dest[MAX_PATH];
    int result=1;
    FSF.sprintf(Dir,_T("%s"),_D(DestPath));
    FSF.AddEndSlash(Dir);
    if(!OpMode2)
    {
      if(!GetDestDir(Dir,Move?1:0))
        return -1;
      TCHAR *filename;
      filename=_tcsrchr(Dir,'\\');
      if(!filename)
        filename=Dir;
      if(!(_tcscmp(filename,_T("."))&&_tcscmp(filename,_T(".."))))
      {
        _tcscat(Dir,_T("\\"));
      }
      if(!GetFullPathName(Dir,ArraySize(Dest),Dest,&filename))
      {
        if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
        {
          TCHAR err1[512];
          FSF.sprintf(err1,GetMsg(mErr1),Dir);
          const TCHAR *MsgItems[]={GetMsg(mError),err1,GetMsg(mOk)};
          Info.Message(Info.ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,ArraySize(MsgItems),1);
        }
        return 0;
      }
      if(Dest[_tcslen(Dest)-1]!='\\')
      {
        WIN32_FIND_DATA find;
        HANDLE hFind=FindFirstFile(Dest,&find);
        if(hFind!=INVALID_HANDLE_VALUE)
        {
          FindClose(hFind);
          if(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
          {
            _tcscat(Dest,_T("\\"));
          }
        }
      }
    }
    else
    {
      FSF.sprintf(Dest,_T("%s"),_D(DestPath));
      FSF.AddEndSlash(Dest);
    }
    BOOL IsDir=(Dest[_tcslen(Dest)-1]=='\\');
    TCHAR filename[MAX_PATH]; HANDLE file; DWORD transferred;
    FSF.AddEndSlash((TCHAR*)_D(DestPath));
    if(panel->level)
    {
      BOOL FlagAll=FALSE; BOOL ActionType=atNone;
      TCHAR copyname[56],progress[56]; HANDLE screen=NULL,console=INVALID_HANDLE_VALUE;
      const TCHAR *Items[]={GetMsg(mCpyDlgCopyTitle),copyname,progress};
      if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
      {
        screen=Info.SaveScreen(0,0,-1,-1);
        console=CreateFile(_T("CONIN$"),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
      }
      bool firsttime=true;
      for(int i=0;i<ItemsNumber;i++)
      {
        if(!(PanelItem[i].Flags&PPIF_USERDATA)) continue;
        EVENTLOGRECORD *curr_rec=(EVENTLOGRECORD *)(PanelItem[i].UserData);
        if(IsDir)
        {
          if(OpMode2||(!Opt.StripExt))
            FSF.sprintf(filename,_T("%s%s"),Dest,PanelItem[i].FindData.PANEL_FILENAME);
          else
            FSF.sprintf(filename,_T("%s%08ld.%s"),Dest,curr_rec->RecordNumber,GetMsg(mExtMain));
        }
        else
          FSF.sprintf(filename,_T("%s"),Dest);
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
              /* 0*/  {DI_DOUBLEBOX,3,1,65,10,0,0,0,0,(TCHAR *)mExistDestAlreadyExists},
              /* 1*/  {DI_TEXT,5,2,0,0,0,0,0,0,_T("")},
              /* 2*/  {DI_TEXT,-1,3,0,0,0,0,DIF_SEPARATOR,0,_T("")},
              /* 3*/  {DI_TEXT,5,4,0,0,0,0,0,0,_T("")},
              /* 4*/  {DI_TEXT,5,5,0,0,0,0,0,0,_T("")},
              /* 5*/  {DI_TEXT,-1,6,0,0,0,0,DIF_SEPARATOR,0,_T("")},
              /* 6*/  {DI_CHECKBOX,5,7,0,0,1,0,0,0,(TCHAR *)mExistAcceptChoice},
              /* 7*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)mExistOverwrite},
              /* 8*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)mExistSkip},
              /* 9*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)mExistAppend},
              /*10*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)mExistCancel},
              };
              struct FarDialogItem DialogItems[ArraySize(InitItems)];
              InitDialogItems(InitItems,DialogItems,ArraySize(InitItems));
#ifdef UNICODE
              TCHAR FilenameText[512],SrcText[512],DstText[512];
              FSF.sprintf(FilenameText,_T("%s"),filename);
              FSF.TruncPathStr(FilenameText,56);
              DialogItems[1].PtrData=FilenameText;
              FSF.sprintf(SrcText,GetMsg(mExistSource),0ULL,0,0,0,0,0,0);
              DialogItems[3].PtrData=SrcText;
#else
              FSF.sprintf(DialogItems[1].Data,_T("%s"),filename);
              FSF.TruncPathStr(DialogItems[1].Data,56);
              FSF.sprintf(DialogItems[3].Data,GetMsg(mExistSource),0ULL,0,0,0,0,0,0);
#endif
              {
                unsigned long long size;
                SYSTEMTIME mod;
                GetFileAttr(filename,&size,&mod);
#ifdef UNICODE
                FSF.sprintf(DstText,GetMsg(mExistDestination),size,mod.wDay,mod.wMonth,mod.wYear,mod.wHour,mod.wMinute,mod.wSecond);
                DialogItems[4].PtrData=DstText;
#else
                FSF.sprintf(DialogItems[4].Data,GetMsg(mExistDestination),size,mod.wDay,mod.wMonth,mod.wYear,mod.wHour,mod.wMinute,mod.wSecond);
#endif
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
          TCHAR buff[SMALL_BUFFER];
#ifdef UNICODE
          FSF.sprintf(buff,_T("%c"),0xfeff);
          WriteFile(file,buff,_tcslen(buff)*sizeof(TCHAR),&transferred,NULL);
#endif
          if(QVOpt.ShowHeader||!(OpMode&OPM_QUICKVIEW))
          {
            FILETIME localtime;
            SYSTEMTIME time;
            FileTimeToLocalFileTime(&(PanelItem[i].FindData.ftCreationTime),&localtime);
            FileTimeToSystemTime(&localtime,&time);
            FSF.sprintf(buff,_T("%s: %02d.%02d.%04d\r\n%s: %02d:%02d:%02d\r\n"),GetMsg(mFileDate),time.wDay,time.wMonth,time.wYear,GetMsg(mFileTime),time.wHour,time.wMinute,time.wSecond);
            WriteFile(file,buff,_tcslen(buff)*sizeof(TCHAR),&transferred,NULL);
            FSF.sprintf(buff,_T("%s: %s\r\n%s: %s\r\n%s: %ld\r\n"),GetMsg(mFileUser),(PanelItem[i].CustomColumnData?PanelItem[i].CustomColumnData[4]:NULL),GetMsg(mFileComputer),(PanelItem[i].CustomColumnData?PanelItem[i].CustomColumnData[3]:NULL),GetMsg(mFileEventID),curr_rec->EventID&0xffff);
            WriteFile(file,buff,_tcslen(buff)*sizeof(TCHAR),&transferred,NULL);
            FSF.sprintf(buff,_T("%s: %s\r\n%s: %s\r\n%s: %s\r\n\r\n"),GetMsg(mFileSource),PanelItem[i].Description,GetMsg(mFileType),GetType(curr_rec->EventType),GetMsg(mFileCategory),(PanelItem[i].CustomColumnData?PanelItem[i].CustomColumnData[1]:NULL));
            WriteFile(file,buff,_tcslen(buff)*sizeof(TCHAR),&transferred,NULL);
          }
          if(QVOpt.ShowDescription||!(OpMode&OPM_QUICKVIEW))
          {
            FSF.sprintf(buff,_T("%s:\r\n"),GetMsg(mFileDescription));
            WriteFile(file,buff,_tcslen(buff)*sizeof(TCHAR),&transferred,NULL);
            TCHAR *msg=FormatLogMessage(panel->path,curr_rec);
            if(msg) WriteFile(file,msg,_tcslen(msg)*sizeof(TCHAR),&transferred,NULL);
            free(msg);
            FSF.sprintf(buff,_T("\r\n\r\n"));
            WriteFile(file,buff,_tcslen(buff)*sizeof(TCHAR),&transferred,NULL);
          }
          if(QVOpt.ShowData||!(OpMode&OPM_QUICKVIEW))
          {
            if(curr_rec->DataLength)
            {
              FSF.sprintf(buff,_T("%s:\r\n"),GetMsg(mFileData));
              WriteFile(file,buff,_tcslen(buff)*sizeof(TCHAR),&transferred,NULL);
              int len=curr_rec->DataLength,offset=0;
              const int cols=16;
              while(len>0)
              {
                FSF.sprintf(buff,_T("%08x: "),offset);
                for(int i=0;i<cols;i++)
                {
                  if((len-i)>0)
                    FSF.sprintf(buff,_T("%s%02X "),buff,*((unsigned char*)curr_rec+curr_rec->DataOffset+offset+i));
                  else
                    FSF.sprintf(buff,_T("%s   "),buff);
                }
                for(int i=0;i<cols;i++)
                {
                  if((len-i)>0)
                  {
                    unsigned char c=*((unsigned char*)curr_rec+curr_rec->DataOffset+offset+i);
                    if(c<' ') c='.';
                    FSF.sprintf(buff,_T("%s%c"),buff,c);
                  }
                }
                FSF.sprintf(buff,_T("%s\r\n"),buff);
                offset+=cols;
                len-=cols;
                WriteFile(file,buff,_tcslen(buff)*sizeof(TCHAR),&transferred,NULL);
              }
              FSF.sprintf(buff,_T("\r\n"));
              WriteFile(file,buff,_tcslen(buff)*sizeof(TCHAR),&transferred,NULL);
            }
          }
          if(TechOpt.Separator[0]) WriteFile(file,TechOpt.Separator,_tcslen(TechOpt.Separator)*sizeof(TCHAR),&transferred,NULL);
          CloseHandle(file);
        }
        if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
        {
          FSF.sprintf(copyname,_T("%.55s "),filename);
          FSF.sprintf(progress,_T("%3d%% "),(i+1)*100/ItemsNumber);
          memset(progress+5,'ฐ',50); progress[55]=0;
          for(int j=0;j<((i+1)*50/ItemsNumber);j++)
            progress[5+j]='Û';
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
          FSF.sprintf(filename,_T("%s%s.evt"),Dest,PanelItem[i].FindData.PANEL_FILENAME);
        else
          FSF.sprintf(filename,_T("%s"),Dest);
        TCHAR path_ansi[MAX_PATH];
        t_OemToChar(PanelItem[i].FindData.PANEL_FILENAME,path_ansi);
        HANDLE evt=OpenEventLog(panel->computer_ptr,path_ansi); //REMOTE
        if(evt)
        {
          TCHAR filename_ansi[MAX_PATH];
          t_OemToChar(filename,filename_ansi);
          if(!BackupEventLog(evt,filename_ansi))
            if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
              DISPLAY_WIN_ERROR
          CloseEventLog(evt);
        } else if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
          DISPLAY_WIN_ERROR
      }
      if(Move)
        EXP_NAME_CALL(DeleteFiles)(hPlugin,PanelItem,ItemsNumber,OpMode);
    }
    return result;
  }
  return 0;
}

int WINAPI EXP_NAME(DeleteFiles)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  if(!IsOldFAR)
  {
    EventViewer *panel=(EventViewer *)hPlugin;
    if(panel->level==0)
    {
      for(int i=0;i<ItemsNumber;i++)
      {
        int MsgCode=0;
        if(!(OpMode&(OPM_SILENT)))
        {
          TCHAR Msg[1024];
          FSF.sprintf(Msg,GetMsg(mClearLog),PanelItem[i].FindData.PANEL_FILENAME);
          const TCHAR *MsgItems[]={_T(""),Msg,GetMsg(mClearClear),GetMsg(mClearSkip),GetMsg(mClearCancel)};
          MsgCode=Info.Message(Info.ModuleNumber,0,NULL,MsgItems,ArraySize(MsgItems),3);
        }
        if(MsgCode==0)
        {
          TCHAR path_ansi[MAX_PATH];
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
              PanelInfo PInfo;
              Info.Control(hPlugin,FCTL_GETPANELINFO,&PInfo);
              for(int j=0;j<PInfo.ItemsNumber;j++)
                if(!FSF.LStricmp(PInfo.PanelItems[j].FindData.PANEL_FILENAME,PanelItem[i].FindData.PANEL_FILENAME))
                  PInfo.PanelItems[j].Flags&=~PPIF_SELECTED;
              Info.Control(hPlugin,FCTL_SETSELECTION,&PInfo);
            }
            CloseEventLog(evt);
          } else if(!(OpMode&(OPM_SILENT)))
            DISPLAY_WIN_ERROR
        }
        else if((MsgCode==2)||(MsgCode==-1))
          break;
      }
      Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
      return TRUE;
    }
  }
  return FALSE;
}

HANDLE WINAPI EXP_NAME(OpenFilePlugin)(
#ifdef UNICODE
const
#endif
TCHAR *Name,const unsigned char *Data,int DataSize
#ifdef UNICODE
,int OpMode
#endif
)
{
  if(!Name)
    return INVALID_HANDLE_VALUE;
  if(!Opt.BrowseEvtFiles)
    return INVALID_HANDLE_VALUE;
  if(IsOldFAR||!FSF.ProcessName(_T("*.evt"),(TCHAR*)Name,
#ifdef UNICODE
  0,
#endif
  PN_CMPNAMELIST))
    return INVALID_HANDLE_VALUE;
  return RealOpenFilePlugin(Name,Data,DataSize);
}

static HANDLE RealOpenFilePlugin(const TCHAR *Name,const unsigned char *Data,int DataSize)
{
  TCHAR path_ansi[MAX_PATH];
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
  _tcscpy(panel->path,Name);
  _tcscpy(panel->computer,_T(""));
  _tcscpy(panel->computer_oem,_T(""));
  panel->computer_ptr=NULL;
  panel->redraw=FALSE;
  return (HANDLE)panel;
}

int WINAPI EXP_NAME(Compare)(HANDLE hPlugin,const struct PluginPanelItem *Item1,const struct PluginPanelItem *Item2,unsigned int Mode)
{
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
  return -2;
}

// C0: Event
// C1: Category
// C2: Time
// C3: Computer
// C4: User
// C5: Time
// Z : Source
static TCHAR *GetTitles(TCHAR *str)
{
  TCHAR *Result=NULL;
  if((!_tcsncmp(str,_T("C0"),2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleEvent);
  }
  else if((!_tcsncmp(str,_T("C1"),2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleCategory);
  }
  else if((!_tcsncmp(str,_T("C2"),2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleTime);
  }
  else if((!_tcsncmp(str,_T("C3"),2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleComputer);
  }
  else if((!_tcsncmp(str,_T("C4"),2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleUser);
  }
  else if((!_tcsncmp(str,_T("C5"),2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleTime);
  }
  else if((str[0]=='Z')&&((!str[1])||(str[1]==',')))
  {
    Result=GetMsg(mTitleSource);
  }
  return Result;
}

void WINAPI EXP_NAME(GetOpenPluginInfo)(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  if(!IsOldFAR)
  {
    EventViewer *panel=(EventViewer *)hPlugin;
    Info->StructSize=sizeof(*Info);
    Info->Flags=OPIF_USEHIGHLIGHTING|OPIF_ADDDOTS|OPIF_SHOWNAMESONLY|OPIF_FINDFOLDERS;
    Info->HostFile=NULL;
    Info->CurDir=panel->path;
    if(panel->level==2)
    {
      Info->HostFile=panel->path;
      Info->CurDir=_T("");
    }

    Info->Format=GetMsg(mName);

    static TCHAR Title[MAX_PATH];
    switch(panel->level)
    {
      case 0:
        if(panel->computer_ptr)
          FSF.sprintf(Title,_T(" %s: %s "),panel->computer_oem,GetMsg(mRootName));
        else
          FSF.sprintf(Title,_T(" %s "),GetMsg(mRootName));
        break;
      case 1:
        if(panel->computer_ptr)
          FSF.sprintf(Title,_T(" %s: %s\\%s "),panel->computer_oem,GetMsg(mRootName),panel->path);
        else
          FSF.sprintf(Title,_T(" %s\\%s "),GetMsg(mRootName),panel->path);
        break;
      case 2:
        FSF.sprintf(Title,_T(" %s "),panel->path);
        break;
    }
    Info->PanelTitle=Title;

    Info->InfoLines=NULL;
    Info->InfoLinesNumber=0;

    Info->DescrFiles=NULL;
    Info->DescrFilesNumber=0;

    static struct PanelMode PanelModesArray[10];
    static TCHAR *ColumnTitles[10][32];
    int msg_start=(panel->level)?mPanelTypeFile0:mPanelTypeDir0;
    TCHAR *tmp_msg;

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
        int j=0; TCHAR *scan=PanelModesArray[i].ColumnTypes;
        if(scan)
        {
          while(TRUE)
          {
            if(j==32) break;
            ColumnTitles[i][j]=GetTitles(scan);
            scan=_tcschr(scan,',');
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
    KeyBar.Titles[7-1]=(TCHAR*)_T("");

    if(panel->level==2)
      KeyBar.Titles[6-1]=(TCHAR*)_T("");
    else
      KeyBar.Titles[6-1]=GetMsg(mKeyRemote);
    if(panel->level)
    {
      KeyBar.Titles[8-1]=(TCHAR*)_T("");
      KeyBar.ShiftTitles[8-1]=(TCHAR*)_T("");
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
        KeyBar.Titles[5-1]=(TCHAR*)_T("");
        KeyBar.ShiftTitles[5-1]=(TCHAR*)_T("");
      }
      KeyBar.Titles[8-1]=GetMsg(mKeyClear);
      KeyBar.ShiftTitles[8-1]=GetMsg(mKeyClear);
    }
    KeyBar.ShiftTitles[1-1]=(TCHAR*)_T("");
    KeyBar.ShiftTitles[2-1]=(TCHAR*)_T("");
    if(panel->level>0)
    {
      KeyBar.ShiftTitles[3-1]=GetMsg(mKeyViewData);
      KeyBar.ShiftTitles[4-1]=GetMsg(mKeyEditData);
    }
    else
    {
      KeyBar.ShiftTitles[3-1]=(TCHAR*)_T("");
      KeyBar.ShiftTitles[4-1]=(TCHAR*)_T("");
    }
    if(panel->level==2)
      KeyBar.ShiftTitles[6-1]=(TCHAR*)_T("");
    else
      KeyBar.ShiftTitles[6-1]=GetMsg(mKeyLocal);
    KeyBar.AltTitles[6-1]=(TCHAR*)_T("");
    Info->KeyBar=&KeyBar;
  }
}

static long WINAPI ComputerDialogProc(HANDLE hDlg, int Msg,int Param1,long Param2)
{
  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_SETTEXTLENGTH,2,MAX_PATH-3);
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

int WINAPI EXP_NAME(ProcessKey)(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  EventViewer *panel=(EventViewer *)hPlugin;
  if((ControlState==PKF_SHIFT)&&(Key==VK_F4||Key==VK_F3)) //skip Shift-F4
  {
    if(panel->level>0)
    {
      PanelInfo PInfo;
      Info.Control(hPlugin,FCTL_GETPANELINFO,&PInfo);
      if(PInfo.ItemsNumber&&(PInfo.PanelItems[PInfo.CurrentItem].Flags&PPIF_USERDATA))
      {
        TCHAR temp[MAX_PATH],tempfile[MAX_PATH];
        if(GetTempPath(MAX_PATH,temp)&&GetTempFileName(temp,_T("evt"),0,tempfile))
        {
          HANDLE hdata=CreateFile(tempfile,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
          if(hdata!=INVALID_HANDLE_VALUE)
          {
            DWORD dWritten;
            EVENTLOGRECORD *curr_rec=(EVENTLOGRECORD *)(PInfo.PanelItems[PInfo.CurrentItem].UserData);
            if(curr_rec->DataLength)
              WriteFile(hdata,(unsigned char*)curr_rec+curr_rec->DataOffset,curr_rec->DataLength,&dWritten,NULL);
            CloseHandle(hdata);
            if(Key==VK_F3)
              Info.Viewer(tempfile,NULL,0,0,-1,-1,VF_DELETEONLYFILEONCLOSE|VF_ENABLE_F6|VF_DISABLEHISTORY|VF_NONMODAL|VF_IMMEDIATERETURN
#ifdef UNICODE
              ,CP_AUTODETECT
#endif
              );
            else
              Info.Editor(tempfile,NULL,0,0,-1,-1,EF_DELETEONLYFILEONCLOSE|EF_ENABLE_F6|EF_DISABLEHISTORY|EF_NONMODAL|EF_IMMEDIATERETURN,1,1
#ifdef UNICODE
              ,CP_AUTODETECT
#endif
              );
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
      static const TCHAR *NTEventComputerHistoryName=_T("NTEventComputer");
      static struct InitDialogItem InitDlg[]={
      /*0*/  {DI_DOUBLEBOX,3,1,44,5,0,0,0,0,(TCHAR *)mSelCompTitle},
      /*1*/  {DI_TEXT,5,2,0,0,0,0,0,0,(TCHAR *)mSelCompLabel},
      /*2*/  {DI_EDIT,5,3,42,0,1,(DWORD)NTEventComputerHistoryName,DIF_HISTORY,0,_T("")},
      /*3*/  {DI_TEXT,5,4,0,0,0,0,0,0,(TCHAR *)mSelCompFootnote},
      };
      struct FarDialogItem DialogItems[ArraySize(InitDlg)];
      InitDialogItems(InitDlg,DialogItems,ArraySize(InitDlg));
#ifdef UNICODE
      DialogItems[2].PtrData=panel->computer_oem;
#else
      FSF.sprintf(DialogItems[2].Data,_T("%s"),panel->computer_oem);
#endif
      CFarDialog dialog;
      int DlgCode=dialog.Execute(Info.ModuleNumber,-1,-1,48,7,NULL,DialogItems,ArraySize(InitDlg),0,0,ComputerDialogProc,0);
      if(DlgCode!=-1)
      {
        if(dialog.Str(2)[0])
        {
          TCHAR temp_computer_name[MAX_PATH];
          if(_tcsncmp(dialog.Str(2),_T("\\\\"),2))
          {
            TCHAR tmp[512];
            FSF.sprintf(tmp,_T("\\\\%s"),dialog.Str(2));
#ifdef UNICODE
            _tcscpy(temp_computer_name,tmp);
#else
            _tcscpy((char*)dialog.Str(2),tmp);
#endif
          }
#ifndef UNICODE
          t_OemToChar(dialog.Str(2),temp_computer_name);
#endif
          if(CheckRemoteEventLog(temp_computer_name))
          {
            _tcscpy(panel->computer,temp_computer_name);
#ifdef UNICODE
            _tcscpy(panel->computer_oem,temp_computer_name);
#else
            _tcscpy(panel->computer_oem,dialog.Str(2));
#endif
            panel->computer_ptr=panel->computer;
            Info.Control(hPlugin,FCTL_UPDATEPANEL,NULL);
            Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
          }
          else
          {
            const TCHAR *MsgItems[]={GetMsg(mError),GetMsg(mErr2),GetMsg(mOk)};
            Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,ArraySize(MsgItems),1);
          }
        }
        else
        {
          _tcscpy(panel->computer,_T(""));
          _tcscpy(panel->computer_oem,_T(""));
          panel->computer_ptr=NULL;
          Info.Control(hPlugin,FCTL_UPDATEPANEL,NULL);
          Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
        }
      }
    }
    return TRUE;
  }
  if((ControlState==PKF_SHIFT)&&(Key==VK_F6)) //skip Shift-F6
  {
    _tcscpy(panel->computer,_T(""));
    _tcscpy(panel->computer_oem,_T(""));
    panel->computer_ptr=NULL;
    Info.Control(hPlugin,FCTL_UPDATEPANEL,NULL);
    Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
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
}

int WINAPI EXP_NAME(Configure)(int ItemNumber)
{
  switch(ItemNumber)
  {
    case 0:
      return(Config());
  }
  return(FALSE);
}

int WINAPI EXP_NAME(ProcessEvent)(HANDLE hPlugin,int Event,void *Param)
{
  EventViewer *panel=(EventViewer *)hPlugin;
  if(Event==FE_REDRAW&&panel->redraw)
  {
    panel->redraw=FALSE;
    if(State.ViewMode!=-1)
      Info.Control(hPlugin,FCTL_SETVIEWMODE,&State.ViewMode);
    if(State.SortMode!=-1)
      Info.Control(hPlugin,FCTL_SETSORTMODE,&State.SortMode);
    if(State.SortOrder!=-1)
      Info.Control(hPlugin,FCTL_SETSORTORDER,&State.SortOrder);
    PanelRedrawInfo ri={State.Current,State.Top};
    Info.Control(hPlugin,FCTL_REDRAWPANEL,&ri);
    return TRUE;
  }
  else if(Event==FE_CLOSE&&panel->level<2)
  {
    PanelInfo PInfo;
    Info.Control(hPlugin,FCTL_GETPANELSHORTINFO,&PInfo);
    State.Current=PInfo.CurrentItem;
    State.Top=PInfo.TopPanelItem;
    State.ViewMode=PInfo.ViewMode;
    State.SortMode=PInfo.SortMode;
    State.SortOrder=PInfo.Flags&PFLAGS_REVERSESORTORDER?1:0;
    _tcscpy(State.Path,panel->path);
    if(panel->computer_ptr)
      _tcscpy(State.Computer,panel->computer_oem);
    else
      _tcscpy(State.Computer,_T(""));
    if(Opt.Restore)
    {
      HKEY hKey;
      DWORD Disposition;
      if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
      {
        RegSetValueEx(hKey,_T("StatePath"),0,REG_SZ,(LPBYTE)State.Path,(_tcslen(State.Path)+1)*sizeof(TCHAR));
        RegSetValueEx(hKey,_T("StateComputer"),0,REG_SZ,(LPBYTE)State.Computer,(_tcslen(State.Computer)+1)*sizeof(TCHAR));
        RegSetValueEx(hKey,_T("StateCurrent"),0,REG_DWORD,(LPBYTE)&State.Current,sizeof(State.Current));
        RegSetValueEx(hKey,_T("StateTop"),0,REG_DWORD,(LPBYTE)&State.Top,sizeof(State.Top));
        RegSetValueEx(hKey,_T("StateViewMode"),0,REG_DWORD,(LPBYTE)&State.ViewMode,sizeof(State.ViewMode));
        RegSetValueEx(hKey,_T("StateSortMode"),0,REG_DWORD,(LPBYTE)&State.SortMode,sizeof(State.SortMode));
        RegSetValueEx(hKey,_T("StateSortOrder"),0,REG_DWORD,(LPBYTE)&State.SortOrder,sizeof(State.SortOrder));
        RegCloseKey(hKey);
      }
    }
  }
  return FALSE;
}

int WINAPI EXP_NAME(GetMinFarVersion)(void)
{
  return FARMANAGERVERSION;
}

void WINAPI EXP_NAME(ExitFAR)()
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
