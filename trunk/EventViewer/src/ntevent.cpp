#include <stdio.h>
#include <stdlib.h>
#include "../../plugin.hpp"
#include "../../farcolor.hpp"
#include "evplugin.h"
#include "memory.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
char PluginRootKey[80];
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
  char Prefix[16];
} Opt={FALSE,0,TRUE,TRUE,TRUE,TRUE,TRUE,0,"evt"};

struct QVOptions {
  BOOL ShowHeader;
  BOOL ShowDescription;
  BOOL ShowData;
} QVOpt={FALSE,TRUE,TRUE};

struct TechOptions
{
  char Separator[1024];
  BOOL NetBackup;
} TechOpt;

struct PluginState
{
  char Path[MAX_PATH];
  char Computer[MAX_PATH];
  int Current;
  int Top;
  int ViewMode;
  int SortMode;
  int SortOrder;
} State={"","",0,0,-1,-1,-1};

static char *GetMsg(int MsgId)
{
  return (char *)Info.GetMsg(Info.ModuleNumber,MsgId);
}

static char *default_column_data="";

struct InitDialogItem
{
  int Type;
  int X1, Y1, X2, Y2;
  int Focus;
  int Selected;
  unsigned int Flags;
  int DefaultButton;
  char *Data;
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
    if ((unsigned)Init[i].Data<2000)
      strcpy(Item[i].Data,GetMsg((unsigned int)Init[i].Data));
    else
      strcpy(Item[i].Data,Init[i].Data);
  }
}

static bool CheckRemoteEventLog(char *computer)
{
  HANDLE hSScr=Info.SaveScreen(0,0,-1,-1);
  const char *MsgItems[]={"",GetMsg(mOtherConnecting)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
  HANDLE evt=OpenEventLog(computer,"System"); //REMOTE
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
  const char *MsgItems[]={GetMsg(mError),GetMsg(mOk)}; \
  Info.Message(Info.ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1); \
}

static HANDLE RealOpenFilePlugin(char *Name,const unsigned char *Data,int DataSize);

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  memset(&::Info, 0, sizeof(::Info));
  memmove(&::Info, Info, (Info->StructSize > (int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
  if(Info->StructSize > FAR165_INFO_SIZE)
  {
    IsOldFAR=FALSE;
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;

    strcpy(PluginRootKey,Info->RootKey);
    strcat(PluginRootKey,"\\ntevent");
    TechOpt.NetBackup=FALSE;
    TechOpt.Separator[0]=0;
    HKEY hKey;
    DWORD Type;
    DWORD DataSize=0;
    if((RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
    {
      //tech options
      DataSize=sizeof(Opt.AddToConfigMenu);
      RegQueryValueEx(hKey,"AddToConfigMenu",0,&Type,(LPBYTE)&Opt.AddToConfigMenu,&DataSize);
      //state options
      DataSize=sizeof(State.Path);
      RegQueryValueEx(hKey,"StatePath",0,&Type,(LPBYTE)State.Path,&DataSize);
      DataSize=sizeof(State.Computer);
      RegQueryValueEx(hKey,"StateComputer",0,&Type,(LPBYTE)State.Computer,&DataSize);
      DataSize=sizeof(State.Current);
      RegQueryValueEx(hKey,"StateCurrent",0,&Type,(LPBYTE)&State.Current,&DataSize);
      DataSize=sizeof(State.Top);
      RegQueryValueEx(hKey,"StateTop",0,&Type,(LPBYTE)&State.Top,&DataSize);
      DataSize=sizeof(State.ViewMode);
      RegQueryValueEx(hKey,"StateViewMode",0,&Type,(LPBYTE)&State.ViewMode,&DataSize);
      DataSize=sizeof(State.SortMode);
      RegQueryValueEx(hKey,"StateSortMode",0,&Type,(LPBYTE)&State.SortMode,&DataSize);
      DataSize=sizeof(State.SortOrder);
      RegQueryValueEx(hKey,"StateSortOrder",0,&Type,(LPBYTE)&State.SortOrder,&DataSize);
      //main options
      DataSize=sizeof(Opt.AddToDisksMenu);
      RegQueryValueEx(hKey,"AddToDisksMenu",0,&Type,(LPBYTE)&Opt.AddToDisksMenu,&DataSize);
      DataSize=sizeof(Opt.DisksMenuDigit);
      RegQueryValueEx(hKey,"DisksMenuDigit",0,&Type,(LPBYTE)&Opt.DisksMenuDigit,&DataSize);
      DataSize=sizeof(Opt.AddToPluginsMenu);
      RegQueryValueEx(hKey,"AddToPluginsMenu",0,&Type,(LPBYTE)&Opt.AddToPluginsMenu,&DataSize);
      DataSize=sizeof(Opt.BrowseEvtFiles);
      RegQueryValueEx(hKey,"BrowseEvtFiles",0,&Type,(LPBYTE)&Opt.BrowseEvtFiles,&DataSize);
      DataSize=sizeof(Opt.StripExt);
      RegQueryValueEx(hKey,"StripExt",0,&Type,(LPBYTE)&Opt.StripExt,&DataSize);
      DataSize=sizeof(Opt.ScanType);
      RegQueryValueEx(hKey,"ScanType",0,&Type,(LPBYTE)&Opt.ScanType,&DataSize);
      DataSize=sizeof(Opt.Prefix);
      RegQueryValueEx(hKey,"Prefix",0,&Type,(LPBYTE)Opt.Prefix,&DataSize);
      DataSize=sizeof(Opt.Restore);
      RegQueryValueEx(hKey,"Restore",0,&Type,(LPBYTE)&Opt.Restore,&DataSize);
      //QuickView
      DataSize=sizeof(QVOpt.ShowHeader);
      RegQueryValueEx(hKey,"ShowHeader",0,&Type,(LPBYTE)&QVOpt.ShowHeader,&DataSize);
      DataSize=sizeof(QVOpt.ShowDescription);
      RegQueryValueEx(hKey,"ShowDescription",0,&Type,(LPBYTE)&QVOpt.ShowDescription,&DataSize);
      DataSize=sizeof(QVOpt.ShowData);
      RegQueryValueEx(hKey,"ShowData",0,&Type,(LPBYTE)&QVOpt.ShowData,&DataSize);
      //Tech
      DataSize=sizeof(TechOpt.Separator);
      RegQueryValueEx(hKey,"Separator",0,&Type,(LPBYTE)TechOpt.Separator,&DataSize);
      DataSize=sizeof(TechOpt.NetBackup);
      RegQueryValueEx(hKey,"NetBackup",0,&Type,(LPBYTE)&TechOpt.NetBackup,&DataSize);

      RegCloseKey(hKey);
      if((Opt.ScanType!=0)&&(Opt.ScanType!=1))
        Opt.ScanType=0;
      if(!Opt.Prefix[0])
        sprintf(Opt.Prefix,"%s","evt");
    }
  }
}

void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  if(!IsOldFAR)
  {
    Info->StructSize=sizeof(*Info);
    Info->Flags=0;

    static char *DisksMenuStrings[1];
    DisksMenuStrings[0]=GetMsg(mNameDisk);
    static int DisksMenuNumbers[1];
    Info->DiskMenuStrings=DisksMenuStrings;
    DisksMenuNumbers[0]=Opt.DisksMenuDigit;
    Info->DiskMenuNumbers=DisksMenuNumbers;
    Info->DiskMenuStringsNumber=Opt.AddToDisksMenu?1:0;

    static char *PluginMenuStrings[1];
    PluginMenuStrings[0]=GetMsg(mName);
    Info->PluginMenuStrings=PluginMenuStrings;
    Info->PluginMenuStringsNumber=Opt.AddToPluginsMenu?(sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0])):0;
    Info->PluginConfigStrings=PluginMenuStrings;
    Info->PluginConfigStringsNumber=Opt.AddToConfigMenu?(sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0])):0;
    Info->CommandPrefix=Opt.Prefix;
  }
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  if(IsOldFAR)
    return INVALID_HANDLE_VALUE;
  char *cmd=(char *)Item;
  if((OpenFrom==OPEN_COMMANDLINE)&&(strlen(cmd)))
  {
    char Name[MAX_PATH],FullName[MAX_PATH],*File;
    FSF.Unquote(FSF.Trim(strcpy(Name,cmd)));
    if(GetFullPathName(Name,sizeof(FullName),FullName,&File))
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
    strcpy(panel->path,"");
    strcpy(panel->computer,"");
    strcpy(panel->computer_oem,"");
    panel->computer_ptr=NULL;
    panel->redraw=Opt.Restore;
    if(Opt.Restore)
    {
      strcpy(panel->path,State.Path);
      if(strlen(panel->path)) panel->level=1;
      if(strlen(State.Computer))
      {
        char temp_computer_name[MAX_PATH]; OemToChar(State.Computer,temp_computer_name);
        if(CheckRemoteEventLog(temp_computer_name))
        {
          strcpy(panel->computer,temp_computer_name);
          strcpy(panel->computer_oem,State.Computer);
          panel->computer_ptr=panel->computer;
        }
      }
    }
    return (HANDLE)panel;
  }
}

void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
  free((EventViewer *)hPlugin);
}

int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  if(!IsOldFAR)
  {
    HANDLE hSScr=Info.SaveScreen(0,0,-1,-1),console=INVALID_HANDLE_VALUE;
    if(!(OpMode&(OPM_FIND)))
    {
      const char *MsgItems[]={"",GetMsg(mOtherScanning)};
      Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
      console=CreateFile("CONIN$",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    }
    EventViewer *panel=(EventViewer *)hPlugin;
    HANDLE evt=INVALID_HANDLE_VALUE; DWORD scan=EVENTLOG_FORWARDS_READ;
    if(Opt.ScanType==1) scan=EVENTLOG_BACKWARDS_READ;
    *pPanelItem=NULL; *pItemsNumber=0;
    char path_ansi[MAX_PATH];
    OemToChar(panel->path,path_ansi);
    switch(panel->level)
    {
      case 0:
      {
        HKEY hKey=NULL;
        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,EVENTLOG_KEY,0,KEY_READ,&hKey)==ERROR_SUCCESS)
        {
          char NameBuffer[MAX_PATH]; LONG Result;
          for(int i=0;;i++)
          {
            Result=RegEnumKey(hKey,i,NameBuffer,sizeof(NameBuffer));
            if(Result==ERROR_NO_MORE_ITEMS)
              break;
            if(Result==ERROR_SUCCESS)
            {
              CharToOem(NameBuffer,NameBuffer);
              struct PluginPanelItem *pNewPanelItem=(PluginPanelItem *)realloc(*pPanelItem,sizeof(PluginPanelItem)*((*pItemsNumber)+1));
              if(pNewPanelItem)
              {
                *pPanelItem=pNewPanelItem;
                PluginPanelItem *curr=(*pPanelItem)+(*pItemsNumber)++;
                memset(curr,0,sizeof(PluginPanelItem));
                strcpy(curr->FindData.cFileName,NameBuffer);
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
              sprintf((*pPanelItem)->FindData.cFileName,"%s","..");
              (*pPanelItem)->CustomColumnData=(char **)malloc(sizeof(char *)*CUSTOM_COLUMN_COUNT);
              if((*pPanelItem)->CustomColumnData)
              {
                (*pPanelItem)->CustomColumnData[0]=(char *)malloc(sizeof(char));
                (*pPanelItem)->CustomColumnData[1]=(char *)malloc(sizeof(char));
                (*pPanelItem)->CustomColumnData[2]=(char *)malloc(sizeof(char)*3);
                if((*pPanelItem)->CustomColumnData[2])
                  sprintf((*pPanelItem)->CustomColumnData[2],"%s","..");
                (*pPanelItem)->CustomColumnData[3]=(char *)malloc(sizeof(char));
                (*pPanelItem)->CustomColumnData[4]=(char *)malloc(sizeof(char));
                (*pPanelItem)->CustomColumnData[5]="";
                for(int i=0;i<(CUSTOM_COLUMN_COUNT-1);i++)
                  if(!(*pPanelItem)->CustomColumnData[i])
                    (*pPanelItem)->CustomColumnData[i]=default_column_data;
                (*pPanelItem)->CustomColumnNumber=CUSTOM_COLUMN_COUNT;
              }
              for(int i=1;i<(*pItemsNumber);i++)
                sprintf(((*pPanelItem)+i)->FindData.cFileName,"%s.%s.%s",GetMsg(mExtNameError),GetMsg(mExtError),GetMsg(mExtMain));

              unsigned long long evt_date_time,evt_date_time_local;
              void *user_data; char *suffix;
              //read events
              EVENTLOGRECORD *curr_rec;
              char *buff=NULL;
              DWORD buffsize=BIG_BUFFER,readed,needed;
              buff=(char *)malloc(buffsize);
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
                      buff=(char *)malloc(buffsize);
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
                      char *description=(char *)(((EVENTLOGRECORD *)user_data)+1);
                      ((*pPanelItem)+i)->Description=(char *)malloc(strlen(description)+1);
                      if(((*pPanelItem)+i)->Description)
                      {
                        sprintf(((*pPanelItem)+i)->Description,"%s",description);
                        CharToOem(((*pPanelItem)+i)->Description,((*pPanelItem)+i)->Description);
                      }
                    }
                    suffix="";
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
                    sprintf(((*pPanelItem)+i)->FindData.cFileName,"%08ld.%s.%s",curr_rec->RecordNumber,suffix,GetMsg(mExtMain));
                    ((*pPanelItem)+i)->FindData.nFileSizeLow=curr_rec->Length;
                    evt_date_time=curr_rec->TimeWritten; evt_date_time=evt_date_time*10000000ULL+EVENT_START_TIME;
  //                  FileTimeToLocalFileTime((FILETIME *)&evt_date_time,(FILETIME *)&evt_date_time_local);
                    memcpy(&(((*pPanelItem)+i)->FindData.ftLastWriteTime),&evt_date_time,sizeof(evt_date_time));
                    evt_date_time=curr_rec->TimeGenerated; evt_date_time=evt_date_time*10000000ULL+EVENT_START_TIME;
                    memcpy(&(((*pPanelItem)+i)->FindData.ftCreationTime),&evt_date_time,sizeof(evt_date_time));
                    FileTimeToLocalFileTime((FILETIME *)&evt_date_time,(FILETIME *)&evt_date_time_local);
                    ((*pPanelItem)+i)->CustomColumnData=(char **)malloc(sizeof(char *)*CUSTOM_COLUMN_COUNT);
                    if(((*pPanelItem)+i)->CustomColumnData)
                    {
                      ((*pPanelItem)+i)->CustomColumnData[0]=(char *)malloc(sizeof(char)*6);
                      if(((*pPanelItem)+i)->CustomColumnData[0])
                        sprintf(((*pPanelItem)+i)->CustomColumnData[0],"%5ld",curr_rec->EventID&0xffff);
                      char *category=GetCategory(curr_rec);
                      ((*pPanelItem)+i)->CustomColumnData[1]=(char *)malloc(strlen(category)+1);
                      if(((*pPanelItem)+i)->CustomColumnData[1])
                        strcpy(((*pPanelItem)+i)->CustomColumnData[1],category);
                      ((*pPanelItem)+i)->CustomColumnData[2]=(char *)malloc(sizeof(char)*20);
                      if(((*pPanelItem)+i)->CustomColumnData[2])
                      {
                        SYSTEMTIME time;
                        FileTimeToSystemTime((FILETIME *)&evt_date_time_local,&time);
                        sprintf(((*pPanelItem)+i)->CustomColumnData[2],"%02d.%02d.%04d %02d:%02d:%02d",time.wDay,time.wMonth,time.wYear,time.wHour,time.wMinute,time.wSecond);
                      }
                      char *compname=GetComputerName(curr_rec);
                      ((*pPanelItem)+i)->CustomColumnData[3]=(char *)malloc(sizeof(char)*(strlen(compname)+1));;
                      if(((*pPanelItem)+i)->CustomColumnData[3])
                        strcpy(((*pPanelItem)+i)->CustomColumnData[3],compname);
                      char *username=GetUserName(panel->computer_ptr,curr_rec);
                      ((*pPanelItem)+i)->CustomColumnData[4]=(char *)malloc(sizeof(char)*(strlen(username)+1));
                      if(((*pPanelItem)+i)->CustomColumnData[4])
                        strcpy(((*pPanelItem)+i)->CustomColumnData[4],username);

                      ((*pPanelItem)+i)->CustomColumnData[5]=((*pPanelItem)+i)->CustomColumnData[2];
                      for(int j=0;j<(CUSTOM_COLUMN_COUNT-1);j++)
                        if(!((*pPanelItem)+i)->CustomColumnData[j])
                          ((*pPanelItem)+i)->CustomColumnData[j]=default_column_data;
                      ((*pPanelItem)+i)->CustomColumnNumber=CUSTOM_COLUMN_COUNT;
                    }
                    readed-=curr_rec->Length;
                    curr_rec=(EVENTLOGRECORD *)((char *)curr_rec+curr_rec->Length);
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

void WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  for(int i=0;i<ItemsNumber;i++)
  {
    free((void *)PanelItem[i].UserData);
    free(PanelItem[i].Owner);
    free(PanelItem[i].Description);
    if(PanelItem[i].CustomColumnData)
      for(int j=0;j<(CUSTOM_COLUMN_COUNT-1);j++)
        if(PanelItem[i].CustomColumnData[j]!=default_column_data)
          free(PanelItem[i].CustomColumnData[j]);
    free(PanelItem[i].CustomColumnData);
  }
  free(PanelItem);
}

int WINAPI _export SetDirectory(HANDLE hPlugin,const char *Dir,int OpMode)
{
  if(!IsOldFAR)
  {
    int res=TRUE;
    EventViewer *panel=(EventViewer *)hPlugin;
    if(panel->level==2)
      return TRUE;
    if(!strcmp(Dir,"\\"))
    {
      panel->level=0;
      strcpy(panel->path,"");
    }
    else if(!strcmp(Dir,".."))
    {
      if(panel->level==1)
      {
        panel->level=0;
        strcpy(panel->path,"");
      }
      else res=FALSE;
    }
    else if((panel->level==0)&&(CheckLogName(panel,Dir)))
    {
      panel->level=1;
      strcpy(panel->path,Dir);
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

static BOOL GetDestDir(char *dir,int move)
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
  static char *NTEventCopyHistoryName="NTEventCopy";
  static struct InitDialogItem InitDlg[]={
  /*0*/  {DI_DOUBLEBOX,3,1,72,6,0,0,0,0,""},
  /*1*/  {DI_TEXT,5,2,0,0,0,0,0,0,""},
  /*2*/  {DI_EDIT,5,3,70,0,1,(DWORD)NTEventCopyHistoryName,DIF_HISTORY,0,""},
  /*3*/  {DI_TEXT,5,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
  /*4*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,1,""},
  /*5*/  {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,0,(char *)mCpyDlgCancel}
  };
  struct FarDialogItem DialogItems[sizeof(InitDlg)/sizeof(InitDlg[0])];
  InitDialogItems(InitDlg,DialogItems,sizeof(InitDlg)/sizeof(InitDlg[0]));
  PanelInfo PInfo;
  Info.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&PInfo); //FIXME
  sprintf(DialogItems[0].Data,"%s",GetMsg(mCpyDlgCopyTitle+move));
  sprintf(DialogItems[1].Data,GetMsg(mCpyDlgCopyToN+3*move+NumberType(PInfo.SelectedItemsNumber)),PInfo.SelectedItemsNumber);
  sprintf(DialogItems[2].Data,"%s",dir);
  sprintf(DialogItems[4].Data,"%s",GetMsg(mCpyDlgCopyOk+move));
  int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,76,8,NULL,DialogItems,(sizeof(InitDlg)/sizeof(InitDlg[0])),0,0,CopyDialogProc,0);
  if(DlgCode==4)
  {
    sprintf(dir,"%s",DialogItems[2].Data);
    return TRUE;
  }
  else
    return FALSE;
}

static void GetFileAttr(char *file,unsigned long long *size,SYSTEMTIME *mod)
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

static int CheckRetry(char *afrom,char *ato)
{
  char from[512],to[512],buff[MAX_PATH];
  sprintf(buff,"%s",afrom);
  FSF.TruncPathStr(buff,55);
  sprintf(from,GetMsg(mRetryFrom),buff);
  sprintf(buff,"%s",ato);
  FSF.TruncPathStr(buff,55);
  sprintf(to,GetMsg(mRetryTo),buff);
  const char *MsgItems[]={GetMsg(mRetryError),from,to,GetMsg(mRetryRetry),GetMsg(mRetrySkip),GetMsg(mRetryCancel)};
  return Info.Message(Info.ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),3);
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
      char Buff[512]; SYSTEMTIME mod;
      GetLocalTime(&mod);
      sprintf(Buff,GetMsg(mExistSource),0ULL,mod.wDay,mod.wMonth,mod.wYear,mod.wHour,mod.wMinute,mod.wSecond);
      Caption.PtrData=Buff;
      Caption.PtrLength=strlen(Caption.PtrData);
      Info.SendDlgMessage(hDlg,DM_SETTEXT,3,(long)&Caption);
      //refresh
      Info.SendDlgMessage(hDlg,DM_SETREDRAW,0,0);
      break;
    }
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

int WINAPI _export GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode)
{
  if(!IsOldFAR)
  {
    EventViewer *panel=(EventViewer *)hPlugin;
    int OpMode2=OpMode&(~OPM_TOPLEVEL);
    if((!panel->level)&&(panel->computer_ptr)&&(!TechOpt.NetBackup)) return 0; //no remote backup
    char Dir[MAX_PATH],Dest[MAX_PATH];
    int result=1;
    sprintf(Dir,"%s",DestPath);
    FSF.AddEndSlash(Dir);
    if(!OpMode2)
    {
      if(!GetDestDir(Dir,Move?1:0))
        return -1;
      char *filename;
      filename=strrchr(Dir,'\\');
      if(!filename)
        filename=Dir;
      if(!(strcmp(filename,".")&&strcmp(filename,"..")))
        sprintf(Dir,"%s\\",Dir);
      if(!GetFullPathName(Dir,sizeof(Dest),Dest,&filename))
      {
        if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
        {
          char err1[512];
          sprintf(err1,GetMsg(mErr1),Dir);
          const char *MsgItems[]={GetMsg(mError),err1,GetMsg(mOk)};
          Info.Message(Info.ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
        }
        return 0;
      }
      if(Dest[strlen(Dest)-1]!='\\')
      {
        WIN32_FIND_DATAA find;
        HANDLE hFind=FindFirstFileA(Dest,&find);
        if(hFind!=INVALID_HANDLE_VALUE)
        {
          FindClose(hFind);
          if(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
            sprintf(Dest,"%s\\",Dest);
        }
      }
    }
    else
    {
      sprintf(Dest,"%s",DestPath);
      FSF.AddEndSlash(Dest);
    }
    BOOL IsDir=(Dest[strlen(Dest)-1]=='\\');
    char filename[MAX_PATH]; HANDLE file; DWORD transferred;
    FSF.AddEndSlash(DestPath);
    if(panel->level)
    {
      BOOL FlagAll=FALSE; BOOL ActionType=atNone;
      char copyname[56],progress[56]; HANDLE screen=NULL,console=INVALID_HANDLE_VALUE;
      const char *Items[]={GetMsg(mCpyDlgCopyTitle),copyname,progress};
      if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
      {
        screen=Info.SaveScreen(0,0,-1,-1);
        console=CreateFile("CONIN$",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
      }
      bool firsttime=true;
      for(int i=0;i<ItemsNumber;i++)
      {
        if(!(PanelItem[i].Flags&PPIF_USERDATA)) continue;
        EVENTLOGRECORD *curr_rec=(EVENTLOGRECORD *)(PanelItem[i].UserData);
        if(IsDir)
        {
          if(OpMode2||(!Opt.StripExt))
            sprintf(filename,"%s%s",Dest,PanelItem[i].FindData.cFileName);
          else
            sprintf(filename,"%s%08ld.%s",Dest,curr_rec->RecordNumber,GetMsg(mExtMain));
        }
        else
          sprintf(filename,"%s",Dest);
retry_main:
        file=CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN|(OpMode&OPM_EDIT?FILE_ATTRIBUTE_READONLY:0),NULL); //FIXME
        if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
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
              /* 0*/  {DI_DOUBLEBOX,3,1,65,10,0,0,0,0,(char *)mExistDestAlreadyExists},
              /* 1*/  {DI_TEXT,5,2,0,0,0,0,0,0,""},
              /* 2*/  {DI_TEXT,-1,3,0,0,0,0,DIF_SEPARATOR,0,""},
              /* 3*/  {DI_TEXT,5,4,0,0,0,0,0,0,""},
              /* 4*/  {DI_TEXT,5,5,0,0,0,0,0,0,""},
              /* 5*/  {DI_TEXT,-1,6,0,0,0,0,DIF_SEPARATOR,0,""},
              /* 6*/  {DI_CHECKBOX,5,7,0,0,1,0,0,0,(char *)mExistAcceptChoice},
              /* 7*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,1,(char *)mExistOverwrite},
              /* 8*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(char *)mExistSkip},
              /* 9*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(char *)mExistAppend},
              /*10*/  {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(char *)mExistCancel},
              };
              struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
              InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
              sprintf(DialogItems[1].Data,"%s",filename);
              FSF.TruncPathStr(DialogItems[1].Data,56);
              sprintf(DialogItems[3].Data,GetMsg(mExistSource),0ULL,0,0,0,0,0,0);
              {
                unsigned long long size;
                SYSTEMTIME mod;
                GetFileAttr(filename,&size,&mod);
                sprintf(DialogItems[4].Data,GetMsg(mExistDestination),size,mod.wDay,mod.wMonth,mod.wYear,mod.wHour,mod.wMinute,mod.wSecond);
              }
              int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,69,12,NULL,DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,FileExistsDialogProc,0);
              if((DlgCode==-1)||(DlgCode==10))
              {
                result=-1;
                break;
              }
              FlagAll=DialogItems[6].Selected;
              ActionType=DlgCode-7+atOverwrite;
            }
            if(ActionType==atOverwrite)
            {
retry_overwrite:
              file=CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
              if(file==INVALID_HANDLE_VALUE)
              {
                int chk_res=CheckRetry(PanelItem[i].FindData.cFileName,filename);
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
                int chk_res=CheckRetry(PanelItem[i].FindData.cFileName,filename);
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
            int chk_res=CheckRetry(PanelItem[i].FindData.cFileName,filename);
            if(chk_res==rRetry)
              goto retry_main;
            if(chk_res==rCancel)
            {
              result=-1;
              break;
            }
          }
        if(file!=INVALID_HANDLE_VALUE)
        {
//        WriteFile(file,(void *)(PanelItem[i].UserData),*((DWORD *)(PanelItem[i].UserData)),&transferred,NULL);
          char buff[SMALL_BUFFER];
          if(QVOpt.ShowHeader||!(OpMode&OPM_QUICKVIEW))
          {
            FILETIME localtime;
            SYSTEMTIME time;
            FileTimeToLocalFileTime(&(PanelItem[i].FindData.ftCreationTime),&localtime);
            FileTimeToSystemTime(&localtime,&time);
            sprintf(buff,"%s: %02d.%02d.%04d\r\n%s: %02d:%02d:%02d\r\n",GetMsg(mFileDate),time.wDay,time.wMonth,time.wYear,GetMsg(mFileTime),time.wHour,time.wMinute,time.wSecond);
            WriteFile(file,buff,strlen(buff),&transferred,NULL);
            sprintf(buff,"%s: %s\r\n%s: %s\r\n%s: %ld\r\n",GetMsg(mFileUser),(PanelItem[i].CustomColumnData?PanelItem[i].CustomColumnData[4]:NULL),GetMsg(mFileComputer),(PanelItem[i].CustomColumnData?PanelItem[i].CustomColumnData[3]:NULL),GetMsg(mFileEventID),curr_rec->EventID&0xffff);
            WriteFile(file,buff,strlen(buff),&transferred,NULL);
            sprintf(buff,"%s: %s\r\n%s: %s\r\n%s: %s\r\n\r\n",GetMsg(mFileSource),PanelItem[i].Description,GetMsg(mFileType),GetType(curr_rec->EventType),GetMsg(mFileCategory),(PanelItem[i].CustomColumnData?PanelItem[i].CustomColumnData[1]:NULL));
            WriteFile(file,buff,strlen(buff),&transferred,NULL);
          }
          if(QVOpt.ShowDescription||!(OpMode&OPM_QUICKVIEW))
          {
            sprintf(buff,"%s:\r\n",GetMsg(mFileDescription));
            WriteFile(file,buff,strlen(buff),&transferred,NULL);
            char *msg=FormatLogMessage(panel->path,curr_rec);
            if(msg) WriteFile(file,msg,strlen(msg),&transferred,NULL);
            free(msg);
            sprintf(buff,"\r\n\r\n");
            WriteFile(file,buff,strlen(buff),&transferred,NULL);
          }
          if(QVOpt.ShowData||!(OpMode&OPM_QUICKVIEW))
          {
            if(curr_rec->DataLength)
            {
              sprintf(buff,"%s:\r\n",GetMsg(mFileData));
              WriteFile(file,buff,strlen(buff),&transferred,NULL);
              int len=curr_rec->DataLength,offset=0;
              const int cols=16;
              while(len>0)
              {
                sprintf(buff,"%08x: ",offset);
                for(int i=0;i<cols;i++)
                {
                  if((len-i)>0)
                    sprintf(buff,"%s%02X ",buff,*((unsigned char *)curr_rec+curr_rec->DataOffset+offset+i));
                  else
                    sprintf(buff,"%s   ",buff);
                }
                for(int i=0;i<cols;i++)
                {
                  if((len-i)>0)
                  {
                    unsigned char c=*((unsigned char *)curr_rec+curr_rec->DataOffset+offset+i);
                    if(c<' ') c='.';
                    sprintf(buff,"%s%c",buff,c);
                  }
                }
                sprintf(buff,"%s\r\n",buff);
                offset+=cols;
                len-=cols;
                WriteFile(file,buff,strlen(buff),&transferred,NULL);
              }
              sprintf(buff,"\r\n");
              WriteFile(file,buff,strlen(buff),&transferred,NULL);
            }
          }
          if(TechOpt.Separator[0]) WriteFile(file,TechOpt.Separator,strlen(TechOpt.Separator),&transferred,NULL);
          CloseHandle(file);
        }
        if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
        {
          sprintf(copyname,"%.55s ",filename);
          sprintf(progress,"%3d%% ",(i+1)*100/ItemsNumber);
          memset(progress+5,'ฐ',50); progress[55]=0;
          for(int j=0;j<((i+1)*50/ItemsNumber);j++)
            progress[5+j]='Û';
          Info.Message(Info.ModuleNumber,firsttime?(firsttime=false,FMSG_LEFTALIGN):(FMSG_KEEPBACKGROUND|FMSG_LEFTALIGN),NULL,Items,sizeof(Items)/sizeof(Items[0]),0);

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
          sprintf(filename,"%s%s.evt",Dest,PanelItem[i].FindData.cFileName);
        else
          sprintf(filename,"%s",Dest);
        char path_ansi[MAX_PATH];
        OemToChar(PanelItem[i].FindData.cFileName,path_ansi);
        HANDLE evt=OpenEventLog(panel->computer_ptr,path_ansi); //REMOTE
        if(evt)
        {
          char filename_ansi[MAX_PATH];
          OemToChar(filename,filename_ansi);
          if(!BackupEventLog(evt,filename_ansi))
            if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
              DISPLAY_WIN_ERROR
          CloseEventLog(evt);
        } else if(!(OpMode&(OPM_SILENT|OPM_FIND|OPM_DESCR)))
          DISPLAY_WIN_ERROR
      }
      if(Move)
        DeleteFiles(hPlugin,PanelItem,ItemsNumber,OpMode);
    }
    return result;
  }
  return 0;
}

int WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
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
          char Msg[1024];
          sprintf(Msg,GetMsg(mClearLog),PanelItem[i].FindData.cFileName);
          const char *MsgItems[]={"",Msg,GetMsg(mClearClear),GetMsg(mClearSkip),GetMsg(mClearCancel)};
          MsgCode=Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),3);
        }
        if(MsgCode==0)
        {
          char path_ansi[MAX_PATH];
          OemToChar(PanelItem[i].FindData.cFileName,path_ansi);
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
                if(!FSF.LStricmp(PInfo.PanelItems[j].FindData.cFileName,PanelItem[i].FindData.cFileName))
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

HANDLE WINAPI _export OpenFilePlugin(char *Name,const unsigned char *Data,int DataSize)
{
  if(!Name)
    return INVALID_HANDLE_VALUE;
  if(!Opt.BrowseEvtFiles)
    return INVALID_HANDLE_VALUE;
  if(IsOldFAR||!FSF.ProcessName("*.evt",Name,PN_CMPNAMELIST))
    return INVALID_HANDLE_VALUE;
  return RealOpenFilePlugin(Name,Data,DataSize);
}

static HANDLE RealOpenFilePlugin(char *Name,const unsigned char *Data,int DataSize)
{
  char path_ansi[MAX_PATH];
  OemToChar(Name,path_ansi);
  HANDLE evt=OpenBackupEventLog(NULL,path_ansi); //LOCAL
  if(evt)
    CloseEventLog(evt);
  else
    return INVALID_HANDLE_VALUE;
  EventViewer *panel=(EventViewer *)malloc(sizeof(EventViewer));
  if(!panel)
    return INVALID_HANDLE_VALUE;
  panel->level=2;
  strcpy(panel->path,Name);
  strcpy(panel->computer,"");
  strcpy(panel->computer_oem,"");
  panel->computer_ptr=NULL;
  panel->redraw=FALSE;
  return (HANDLE)panel;
}

int WINAPI _export Compare(HANDLE hPlugin,const struct PluginPanelItem *Item1,const struct PluginPanelItem *Item2,unsigned int Mode)
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
static char *GetTitles(char *str)
{
  char *Result=NULL;
  if((!strncmp(str,"C0",2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleEvent);
  }
  else if((!strncmp(str,"C1",2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleCategory);
  }
  else if((!strncmp(str,"C2",2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleTime);
  }
  else if((!strncmp(str,"C3",2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleComputer);
  }
  else if((!strncmp(str,"C4",2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleUser);
  }
  else if((!strncmp(str,"C5",2))&&((!str[2])||(str[2]==',')))
  {
    Result=GetMsg(mTitleTime);
  }
  else if((str[0]=='Z')&&((!str[1])||(str[1]==',')))
  {
    Result=GetMsg(mTitleSource);
  }
  return Result;
}

void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
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
      Info->CurDir="";
    }

    Info->Format=GetMsg(mName);

    static char Title[MAX_PATH];
    switch(panel->level)
    {
      case 0:
        if(panel->computer_ptr)
          sprintf(Title," %s: %s ",panel->computer_oem,GetMsg(mRootName));
        else
          sprintf(Title," %s ",GetMsg(mRootName));
        break;
      case 1:
        if(panel->computer_ptr)
          sprintf(Title," %s: %s\\%s ",panel->computer_oem,GetMsg(mRootName),panel->path);
        else
          sprintf(Title," %s\\%s ",GetMsg(mRootName),panel->path);
        break;
      case 2:
        sprintf(Title," %s ",panel->path);
        break;
    }
    Info->PanelTitle=Title;

    Info->InfoLines=NULL;
    Info->InfoLinesNumber=0;

    Info->DescrFiles=NULL;
    Info->DescrFilesNumber=0;

    static struct PanelMode PanelModesArray[10];
    static char *ColumnTitles[10][32];
    int msg_start=(panel->level)?mPanelTypeFile0:mPanelTypeDir0;
    char *tmp_msg;

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
        int j=0; char *scan=PanelModesArray[i].ColumnTypes;
        if(scan)
        {
          while(TRUE)
          {
            if(j==32) break;
            ColumnTitles[i][j]=GetTitles(scan);
            scan=strchr(scan,',');
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
    Info->PanelModesNumber=sizeof(PanelModesArray)/sizeof(PanelModesArray[0]);
    Info->StartPanelMode='3';
    static struct KeyBarTitles KeyBar;
    memset(&KeyBar,0,sizeof(KeyBar));
    KeyBar.Titles[7-1]="";

    if(panel->level==2)
      KeyBar.Titles[6-1]="";
    else
      KeyBar.Titles[6-1]=GetMsg(mKeyRemote);
    if(panel->level)
    {
      KeyBar.Titles[8-1]="";
      KeyBar.ShiftTitles[8-1]="";
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
        KeyBar.Titles[5-1]="";
        KeyBar.ShiftTitles[5-1]="";
      }
      KeyBar.Titles[8-1]=GetMsg(mKeyClear);
      KeyBar.ShiftTitles[8-1]=GetMsg(mKeyClear);
    }
    KeyBar.ShiftTitles[1-1]="";
    KeyBar.ShiftTitles[2-1]="";
    if(panel->level>0)
    {
      KeyBar.ShiftTitles[3-1]=GetMsg(mKeyViewData);
      KeyBar.ShiftTitles[4-1]=GetMsg(mKeyEditData);
    }
    else
    {
      KeyBar.ShiftTitles[3-1]="";
      KeyBar.ShiftTitles[4-1]="";
    }
    if(panel->level==2)
      KeyBar.ShiftTitles[6-1]="";
    else
      KeyBar.ShiftTitles[6-1]=GetMsg(mKeyLocal);
    KeyBar.AltTitles[6-1]="";
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

int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
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
        char temp[MAX_PATH],tempfile[MAX_PATH];
        if(GetTempPath(MAX_PATH,temp)&&GetTempFileName(temp,"evt",0,tempfile))
        {
          HANDLE hdata=CreateFile(tempfile,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
          if(hdata!=INVALID_HANDLE_VALUE)
          {
            DWORD dWritten;
            EVENTLOGRECORD *curr_rec=(EVENTLOGRECORD *)(PInfo.PanelItems[PInfo.CurrentItem].UserData);
            if(curr_rec->DataLength)
              WriteFile(hdata,(unsigned char *)curr_rec+curr_rec->DataOffset,curr_rec->DataLength,&dWritten,NULL);
            CloseHandle(hdata);
            if(Key==VK_F3)
              Info.Viewer(tempfile,NULL,0,0,-1,-1,VF_DELETEONLYFILEONCLOSE|VF_ENABLE_F6|VF_DISABLEHISTORY|VF_NONMODAL|VF_IMMEDIATERETURN);
            else
              Info.Editor(tempfile,NULL,0,0,-1,-1,EF_DELETEONLYFILEONCLOSE|EF_ENABLE_F6|EF_DISABLEHISTORY|EF_NONMODAL|EF_IMMEDIATERETURN,1,1);
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
      static char *NTEventComputerHistoryName="NTEventComputer";
      static struct InitDialogItem InitDlg[]={
      /*0*/  {DI_DOUBLEBOX,3,1,44,5,0,0,0,0,(char *)mSelCompTitle},
      /*1*/  {DI_TEXT,5,2,0,0,0,0,0,0,(char *)mSelCompLabel},
      /*2*/  {DI_EDIT,5,3,42,0,1,(DWORD)NTEventComputerHistoryName,DIF_HISTORY,0,""},
      /*3*/  {DI_TEXT,5,4,0,0,0,0,0,0,(char *)mSelCompFootnote},
      };
      struct FarDialogItem DialogItems[sizeof(InitDlg)/sizeof(InitDlg[0])];
      InitDialogItems(InitDlg,DialogItems,sizeof(InitDlg)/sizeof(InitDlg[0]));
      sprintf(DialogItems[2].Data,"%s",panel->computer_oem);
      int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,48,7,NULL,DialogItems,(sizeof(InitDlg)/sizeof(InitDlg[0])),0,0,ComputerDialogProc,0);
      if(DlgCode!=-1)
      {
        if(DialogItems[2].Data[0])
        {
          if(strncmp(DialogItems[2].Data,"\\\\",2))
          {
            char tmp[512];
            sprintf(tmp,"\\\\%s",DialogItems[2].Data);
            strcpy(DialogItems[2].Data,tmp);
          }
          char temp_computer_name[MAX_PATH]; OemToChar(DialogItems[2].Data,temp_computer_name);
          if(CheckRemoteEventLog(temp_computer_name))
          {
            strcpy(panel->computer,temp_computer_name);
            strcpy(panel->computer_oem,DialogItems[2].Data);
            panel->computer_ptr=panel->computer;
            Info.Control(hPlugin,FCTL_UPDATEPANEL,NULL);
            Info.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
          }
          else
          {
            const char *MsgItems[]={GetMsg(mError),GetMsg(mErr2),GetMsg(mOk)};
            Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
          }
        }
        else
        {
          strcpy(panel->computer,"");
          strcpy(panel->computer_oem,"");
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
    strcpy(panel->computer,"");
    strcpy(panel->computer_oem,"");
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

int WINAPI _export Configure(int ItemNumber)
{
  switch(ItemNumber)
  {
    case 0:
      return(Config());
  }
  return(FALSE);
}

int WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param)
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
    strcpy(State.Path,panel->path);
    if(panel->computer_ptr)
      strcpy(State.Computer,panel->computer_oem);
    else
      strcpy(State.Computer,"");
    if(Opt.Restore)
    {
      HKEY hKey;
      DWORD Disposition;
      if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
      {
        RegSetValueEx(hKey,"StatePath",0,REG_SZ,(LPBYTE)State.Path,strlen(State.Path)+1);
        RegSetValueEx(hKey,"StateComputer",0,REG_SZ,(LPBYTE)State.Computer,strlen(State.Computer)+1);
        RegSetValueEx(hKey,"StateCurrent",0,REG_DWORD,(LPBYTE)&State.Current,sizeof(State.Current));
        RegSetValueEx(hKey,"StateTop",0,REG_DWORD,(LPBYTE)&State.Top,sizeof(State.Top));
        RegSetValueEx(hKey,"StateViewMode",0,REG_DWORD,(LPBYTE)&State.ViewMode,sizeof(State.ViewMode));
        RegSetValueEx(hKey,"StateSortMode",0,REG_DWORD,(LPBYTE)&State.SortMode,sizeof(State.SortMode));
        RegSetValueEx(hKey,"StateSortOrder",0,REG_DWORD,(LPBYTE)&State.SortOrder,sizeof(State.SortOrder));
        RegCloseKey(hKey);
      }
    }
  }
  return FALSE;
}

int WINAPI _export GetMinFarVersion(void)
{
  return FARMANAGERVERSION;
}

void WINAPI ExitFAR()
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
