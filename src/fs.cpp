/*
    WFX2FAR plugin for FAR Manager
    Copyright (C) 2003-2005 Alex Yaroslavsky

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
struct param
{
  char *path;
  const char *cmd;
  FsExecuteFile_t *FsExecuteFile;
};

DWORD WINAPI LLThread(void *ptr)
{
  param *p=(param *)ptr;
  int ret=p->FsExecuteFile(GetDesktopWindow(),p->path,(char *)p->cmd);
  if (ret==FS_EXEC_OK)
  {
    MSG msg;
    while(GetMessage(&msg,NULL,0,0))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  return ret;
}

class FileSystem
{
private:
  char CurrentDir[1024];
  char DisconnectRoot[1024];
  bool connected;
  //bool pump;
  bool loaded;
  WIN32_FIND_DATA fd;
  bool NoMoreFiles;
  HANDLE fh;
  HANDLE hScreen;
  WFX *wfx;
  const char *RealCurDir();
  FileSystem(WFX *wfx, const char *ini);

public:
  static FileSystem *Create(WFX *wfx, const char *ini);
  ~FileSystem() {CheckConnection("");}
  const char *CurDir() {return *CurrentDir?(const char *)(CurrentDir+1):CurrentDir;}
  const char *Title() {return wfx->Title();}
  void Connected(const char *Root) {connected=true; lstrcpy(DisconnectRoot,Root);}
  void Disconnected() {connected=false; *DisconnectRoot=0;}
  void CheckConnection(const char *NewDir);
  void Config() {Exec("\\","properties",true);}
  int SetDirectory(const char *Dir,int OpMode);
  void GetOpenPluginInfo(struct OpenPluginInfo *Info);
  void ClosePlugin() {CheckConnection(""); delete this;}
  int GetFindData(struct PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode);
  void FreeFindData(struct PluginPanelItem *PanelItem, int ItemsNumber) {if (PanelItem) free(PanelItem);}
  int Exec(char *path,const char *cmd,bool pump);
  int ProcessKey(int Key, unsigned int ControlState);
  int GetFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
  int PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
  int DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
  int MakeDirectory(char *Name,int OpMode);
  void StatusInfo(int InfoStartEnd,int InfoOperation);
};

int __stdcall ProgressProc(int PluginNr,char* SourceName,char* TargetName,int PercentDone)
{
  (void) PluginNr;
  (void) SourceName;
  (void) TargetName;
  (void) PercentDone;
  return 0;
}

void __stdcall LogProc(int PluginNr,int MsgType,char *LogString)
{
  FileSystem *fs = (FileSystem *) PluginNr;

  if (MsgType==MSGTYPE_CONNECT && lstrlen(LogString)>8 && LogString[8]=='\\')
  {
    fs->Connected(LogString+8);/*CONNECT \SomeRoot*/
  }
  else if (MsgType==MSGTYPE_DISCONNECT)
  {
    fs->Disconnected();
  }
}

BOOL __stdcall RequestProc(int PluginNr,int RequestType,char* CustomTitle,char* CustomText,char* ReturnedText,int maxlen)
{
  (void) PluginNr;
  const char *UserTitle=(!CustomTitle)||(!*CustomTitle)?GetMsg(MTitle):CustomTitle;
  if (RequestType < RT_MsgOK)
  {
    const char *UserLabel=(!CustomText)||(!*CustomText)?GetMsg(MsgRT_Other+RequestType):CustomText;
    const char *UserData=ReturnedText?ReturnedText:"";
    InitDialogItem InitItems[]=
    {
      // type,x1,y1,x2,y2,focus,selected,flags,default,data
      {DI_DOUBLEBOX,3,1,57,4,0,0,0,0,UserTitle},
      {DI_TEXT,5,2,0,0,0,0,0,0,UserLabel},
      {DI_EDIT,5,3,55,0,1,0,DIF_HISTORY,0,UserData},
      {DI_BUTTON,3,5,0,0,0,0,0,1,(char *)MsgOK},
    };
    FarDialogItem DialogItems[sizeofa(InitItems)];
    InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));
    if (CustomTitle&&*CustomTitle)
      CharToOem(DialogItems[0].Data,DialogItems[0].Data);
    CharToOem(DialogItems[1].Data,DialogItems[1].Data);
    CharToOem(DialogItems[2].Data,DialogItems[2].Data);
    int key=Info.Dialog(Info.ModuleNumber,-1,-1,60,6,NULL,DialogItems,sizeofa(DialogItems));
    if (key==3)
    {
      if (ReturnedText)
      {
        lstrcpyn(ReturnedText,DialogItems[2].Data,maxlen);
        OemToChar(ReturnedText,ReturnedText);
      }
      return 1;
    }
    else
      return 0;
  }
  else
  {
    unsigned int Flags=0;
    switch (RequestType)
    {
      case RT_MsgOK: Flags = FMSG_MB_OK; break;
      case RT_MsgYesNo: Flags = FMSG_MB_YESNO; break;
      case RT_MsgOKCancel: Flags = FMSG_MB_OKCANCEL; break;
    }
    char title[512];
    lstrcpyn(title,UserTitle,512);
    if (CustomTitle&&*CustomTitle)
      CharToOem(title,title);
    const char *msg[2] = {title, ""};
    int ret = Info.Message(Info.ModuleNumber,Flags,NULL,msg,2,0);
    if (RequestType == RT_MsgOK)
      return 1;
    return !ret;
  }
}

void WinFindDataToFarFindData(struct FAR_FIND_DATA *ffd, WIN32_FIND_DATA *wfd)
{
  lstrcpy(ffd->cFileName, wfd->cFileName);
  lstrcpy(ffd->cAlternateFileName, wfd->cAlternateFileName);
  ffd->dwFileAttributes = wfd->dwFileAttributes;
  ffd->ftCreationTime = wfd->ftCreationTime;
  ffd->ftLastAccessTime = wfd->ftLastAccessTime;
  ffd->ftLastWriteTime = wfd->ftLastWriteTime;
  ffd->nFileSizeHigh = wfd->nFileSizeHigh;
  ffd->nFileSizeLow = wfd->nFileSizeLow;
  ffd->dwReserved0 = wfd->dwReserved0;
  ffd->dwReserved1 = wfd->dwReserved1;
}

char *PointToName(char *str)
{
  char *ptr;
  for (ptr=str+lstrlen(str); ptr>str; ptr--)
    if (*(ptr-1)=='\\')
      break;
  return ptr;
}

FileSystem::FileSystem(WFX *wfx, const char *ini)
{
  loaded=false;

  this->wfx = wfx;

  if (!wfx || wfx->FsInit((int)this,ProgressProc,LogProc,RequestProc))
    return;

  /*
  char buf[32767];
  *buf=0;
  GetPrivateProfileSection("NeedMessagePump",buf,sizeof(buf),ini);
  char *ptr=buf;
  pump=false;
  while(lstrlen(ptr))
  {
    if (!lstrcmp(ptr,wfx->Title()))
    {
      pump=true;
      break;
    }
    ptr=ptr+lstrlen(ptr)+1;
  }
  */

  fh=INVALID_HANDLE_VALUE;
  hScreen=NULL;
  NoMoreFiles=false;
  lstrcpy(CurrentDir, "\\");
  connected=false;
  *DisconnectRoot=0;

  if (wfx->FsSetDefaultParams)
  {
    FsDefaultParamStruct dps;
    dps.size = sizeof(dps);
    dps.PluginInterfaceVersionLow = 30;
    dps.PluginInterfaceVersionHi = 1;
    lstrcpy(dps.DefaultIniName,ini);
    wfx->FsSetDefaultParams(&dps);
  }

  loaded=true;
}

FileSystem *FileSystem::Create(WFX *wfx, const char *ini)
{
  FileSystem *fs = new FileSystem(wfx,ini);
  if (fs)
  {
    if (!fs->loaded)
    {
      delete fs;
      fs=NULL;
    }
  }
  return fs;
}

void FileSystem::CheckConnection(const char *NewDir)
{
  if (connected && wfx->FsDisconnect)
  {
    if (lstrlen(NewDir)<lstrlen(DisconnectRoot) || !*NewDir)
      connected=false;
    else
    {
      char temp[1024];
      lstrcpyn(temp,NewDir,lstrlen(DisconnectRoot)+1);
      if (lstrcmp(temp,DisconnectRoot))
        connected=false;
    }
    if (!connected)
    {
      wfx->FsDisconnect(DisconnectRoot);
      *DisconnectRoot=0;
    }
  }
}

int FileSystem::SetDirectory(const char *Dir,int OpMode)
{
  (void) OpMode;
  char NewDir[1024];
  lstrcpy(NewDir,CurrentDir);
  if (!lstrcmp(Dir,".."))
  {
    if (lstrlen(CurrentDir) == 1)
    {
      *CurrentDir = 0;
      return 1;
    }
    else
    {
      *(PointToName(NewDir)) = 0;
      //int i = lstrlen(NewDir);
      //if (i > 1 && NewDir[i-1] == '\\')
        //NewDir[i-1] = 0;
      CheckConnection(NewDir);
    }
  }
  else if (Dir[0]=='\\')
  {
    lstrcpy(NewDir,Dir);
  }
  else
  {
    FSF.AddEndSlash(NewDir);
    lstrcat(NewDir,Dir);
  }
  {
    int i = lstrlen(NewDir);
    if (i > 1 && NewDir[i-1] == '\\')
      NewDir[i-1] = 0;
  }
  StatusInfo(FS_STATUS_START,FS_STATUS_OP_LIST);
  NoMoreFiles=false;
  hScreen=Info.SaveScreen(0,0,-1,-1);
  fh = wfx->FsFindFirst(NewDir, &fd);
  if (fh != INVALID_HANDLE_VALUE)
  {
    lstrcpy(CurrentDir,NewDir);
    //wfx->FsFindClose(fh);
  }
  else if (GetLastError() == ERROR_NO_MORE_FILES)
  {
    NoMoreFiles=true;
    lstrcpy(CurrentDir,NewDir);
  }
  else
  {
    StatusInfo(FS_STATUS_END,FS_STATUS_OP_LIST);
    return 0;
  }
  StatusInfo(FS_STATUS_END,FS_STATUS_OP_LIST);
  CheckConnection(CurrentDir);
  return 1;
}

void FileSystem::GetOpenPluginInfo(struct OpenPluginInfo *Info)
{
  static char PanelTitle[1024*2+1];
  FSF.sprintf(PanelTitle,"%s:%s",wfx->Title(),CurDir());
  Info->StructSize = sizeof(struct OpenPluginInfo);
  Info->Flags = OPIF_ADDDOTS|OPIF_USEHIGHLIGHTING;
  Info->HostFile = NULL;
  Info->CurDir = CurDir();
  Info->Format = wfx->Title();
  Info->PanelTitle = PanelTitle;
  Info->InfoLines = NULL;
  Info->InfoLinesNumber = 0;
  Info->DescrFiles = NULL;
  Info->DescrFilesNumber = 0;
  Info->PanelModesArray = NULL;
  Info->PanelModesNumber = 0;
  Info->StartPanelMode = 0;
  Info->StartSortMode = 0;
  Info->StartSortOrder = 0;
  Info->KeyBar = NULL;
  Info->ShortcutData = NULL;
  Info->Reserverd = 0;
}

int FileSystem::GetFindData(struct PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode)
{
  (void) OpMode;
  int ret=FALSE;
  *pPanelItem=NULL;
  *pItemsNumber=0;
  if (NoMoreFiles)
  {
    if (hScreen)
    {
      Info.RestoreScreen(NULL);
      Info.RestoreScreen(hScreen);
      hScreen=NULL;
    }
    NoMoreFiles=false;
    return TRUE;
  }
  if (fh == INVALID_HANDLE_VALUE)
  {
    hScreen=Info.SaveScreen(0,0,-1,-1);
    fh = wfx->FsFindFirst(CurrentDir,&fd);
  }
  if (fh != INVALID_HANDLE_VALUE)
  {
    BOOL fn;
    do
    {
      if (lstrcmp(fd.cFileName,"."))
      {
        PluginPanelItem *NewPanelItem=(PluginPanelItem *)realloc(*pPanelItem,(*pItemsNumber+1)*sizeof(PluginPanelItem));
        if (!NewPanelItem)
        {
          wfx->FsFindClose(fh);
          fh=INVALID_HANDLE_VALUE;
          ret=FALSE;
          goto finish;
        }
        *pPanelItem=NewPanelItem;

        memset(&NewPanelItem[*pItemsNumber],0,sizeof(PluginPanelItem));
        WinFindDataToFarFindData(&NewPanelItem[*pItemsNumber].FindData,&fd);
        (*pItemsNumber)++;
      }
      fn = wfx->FsFindNext(fh, &fd);
    } while (fn);
    wfx->FsFindClose(fh);
    fh=INVALID_HANDLE_VALUE;
    ret=TRUE;
  }
  else if (GetLastError() == ERROR_NO_MORE_FILES)
    ret=TRUE;

finish:
  if (hScreen)
  {
    Info.RestoreScreen(NULL);
    Info.RestoreScreen(hScreen);
    hScreen=NULL;
  }

  return ret;
}

int FileSystem::Exec(char *path,const char *cmd,bool pump)
{
  if (!wfx->FsExecuteFile)
    return FS_EXEC_ERROR;
  StatusInfo(FS_STATUS_START,FS_STATUS_OP_EXEC);
  int ret;
  if (pump)
  {
    DWORD tid;
    param p = {path,cmd,wfx->FsExecuteFile};
    HANDLE thr=CreateThread(NULL,0,LLThread,(LPVOID)&p,0,&tid);
    SetForegroundWindow(GetDesktopWindow());
    StatusInfo(FS_STATUS_END,FS_STATUS_OP_EXEC);
    const char *msg[2] = {GetMsg(MTitle),GetMsg(MPumpWarn)};
    Info.Message(Info.ModuleNumber,FMSG_MB_OK,NULL,msg,2,0);
    PostThreadMessage(tid,WM_QUIT,0,0);
    WaitForSingleObject(thr,INFINITE);
    GetExitCodeThread(thr,(LPDWORD)&ret);
    CloseHandle(thr);
  }
  else
  {
    ret=wfx->FsExecuteFile(FarHWND,path,(char *)cmd);
  }
  return ret;
}

int FileSystem::ProcessKey(int Key, unsigned int ControlState)
{
  if (Key==VK_F8&&(!ControlState||ControlState==PKF_SHIFT))
  {
    if (wfx->FsDeleteFile)
      return FALSE;
    return TRUE;
  }
  if (Key==VK_F7&&(!ControlState))
  {
    if (wfx->FsMkDir)
      return FALSE;
    return TRUE;
  }
  if (wfx->FsExecuteFile)
  {
    int cmd;
    bool pump=false;
    if (!ControlState && Key==VK_RETURN)
      cmd=0;
    else if (ControlState==PKF_SHIFT && Key==VK_F7)
    {
      cmd=0; pump=true;
    }
    else if (ControlState==PKF_ALT && Key==VK_F6)
      cmd=1;
    else if (ControlState==(PKF_ALT|PKF_SHIFT) && Key==VK_F6)
    {
      cmd=1; pump=true;
    }
    else
      return FALSE;
    PanelInfo pi;
    Info.Control(this,FCTL_GETPANELINFO,(void *)&pi);
    if (pi.ItemsNumber<=0 || (cmd==0&&(pi.PanelItems[pi.CurrentItem].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)))
      return FALSE;
    char path[MAX_PATH];
    OemToChar(pi.PanelItems[pi.CurrentItem].FindData.cFileName,pi.PanelItems[pi.CurrentItem].FindData.cFileName);
    FSF.sprintf(path,"%s%s%s",CurrentDir,lstrcmp(CurrentDir,"\\")?"\\":"",pi.PanelItems[pi.CurrentItem].FindData.cFileName);
    int ret = Exec(path,cmd?"properties":"open",pump);
    if (cmd) return TRUE;
    switch (ret)
    {
      case FS_EXEC_OK:
        Info.Control(this,FCTL_UPDATEPANEL,(void *)1);
        Info.Control(this,FCTL_REDRAWPANEL,NULL);
        return TRUE;
      case FS_EXEC_ERROR:
        //return TRUE;
      case FS_EXEC_YOURSELF:
        return FALSE;
      case FS_EXEC_SYMLINK:
        if (SetDirectory(path,0))
        {
          Info.Control(this,FCTL_UPDATEPANEL,NULL);
          Info.Control(this,FCTL_REDRAWPANEL,NULL);
        }
        return TRUE;
    }
  }
  return FALSE;
}

const char ILLEGAL_SYMB[] = "/<>:\\|?*\"";
const char ILLEGAL_REPL[] = "!()_!!__'";

void ReplaceIllegalChars(char *ptr)
{
  while (*ptr)
  {
    const char *zz;
    if ((( zz = strchr(ILLEGAL_SYMB,*ptr)) != NULL))
      *ptr = ILLEGAL_REPL[zz-ILLEGAL_SYMB];
    if ( *ptr < 32 || *ptr == 127 ) *ptr = 32;
    ptr++;
  }
}

int FileSystem::GetFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode)
{
  (void) OpMode;
  if (wfx->FsGetFile&&ItemsNumber)
  {
    char dest[1024];
    lstrcpy(dest,DestPath);
    FSF.AddEndSlash(dest);
    int dest_len=lstrlen(dest);
    Move = Move?FS_COPYFLAGS_MOVE:0;
    int ret=0;
    for (int i=0; i<ItemsNumber; i++)
    {
      if (PanelItem[i].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
        continue;
      lstrcpy(dest+dest_len,PanelItem[i].FindData.cFileName);
      ReplaceIllegalChars(dest+dest_len);
      char src[1024];
      FSF.sprintf(src,"%s%s%s",CurrentDir,lstrcmp(CurrentDir,"\\")?"\\":"",PanelItem[i].FindData.cFileName);
      RemoteInfoStruct ri;
      ri.SizeLow=PanelItem[i].FindData.nFileSizeLow;
      ri.SizeHigh=PanelItem[i].FindData.nFileSizeHigh;
      ri.LastWriteTime=PanelItem[i].FindData.ftLastWriteTime;
      ri.Attr=PanelItem[i].FindData.dwFileAttributes;
      ret=wfx->FsGetFile(src,dest,Move,&ri)==FS_FILE_OK?1:0;
      if (!ret)
        break;
      if (PanelItem[i].Flags&PPIF_SELECTED)
        PanelItem[i].Flags&=~PPIF_SELECTED;
    }
    return ret;
  }
  return 0;
}

int FileSystem::PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode)
{
  (void) OpMode;
  if (wfx->FsPutFile&&ItemsNumber)
  {
    char dest[1024];
    lstrcpy(dest,CurrentDir);
    FSF.AddEndSlash(dest);
    int dest_len=lstrlen(dest);
    Move = Move?FS_COPYFLAGS_MOVE:0;
    int ret=0;
    for (int i=0; i<ItemsNumber; i++)
    {
      if (PanelItem[i].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
        continue;
      lstrcpy(dest+dest_len,PanelItem[i].FindData.cFileName);
      ret=wfx->FsPutFile(PanelItem[i].FindData.cFileName,dest,Move)==FS_FILE_OK?1:0;
      if (!ret)
        break;
      if (PanelItem[i].Flags&PPIF_SELECTED)
        PanelItem[i].Flags&=~PPIF_SELECTED;
    }
    return ret;
  }
  return 0;
}

int FileSystem::DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  (void) OpMode;
  if (wfx->FsDeleteFile)
  {
    int ret=FALSE;
    for (int i=0; i<ItemsNumber; i++)
    {
      char src[1024];
      FSF.sprintf(src,"%s%s%s",CurrentDir,lstrcmp(CurrentDir,"\\")?"\\":"",PanelItem[i].FindData.cFileName);
      if (PanelItem[i].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
      {
        if (wfx->FsRemoveDir)
          ret=wfx->FsRemoveDir(src);
        else
          continue;
      }
      else
        ret=wfx->FsDeleteFile(src);
      if (!ret)
        break;
      if (PanelItem[i].Flags&PPIF_SELECTED)
        PanelItem[i].Flags&=~PPIF_SELECTED;
    }
    return ret;
  }
  return FALSE;
}

int FileSystem::MakeDirectory(char *Name,int OpMode)
{
  (void) OpMode;
  if (wfx->FsMkDir)
  {
    char src[1024];
    FSF.sprintf(src,"%s%s%s",CurrentDir,lstrcmp(CurrentDir,"\\")?"\\":"",Name);
    return wfx->FsMkDir(src);
  }
  return 0;
}

void FileSystem::StatusInfo(int InfoStartEnd,int InfoOperation)
{
  if (wfx->FsStatusInfo)
  {
    char RemoteDir[1024];
    lstrcpy(RemoteDir,CurrentDir);
    wfx->FsStatusInfo(RemoteDir,InfoStartEnd,InfoOperation);
  }
}
