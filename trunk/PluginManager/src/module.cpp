/*
    PluginManager plugin for FAR Manager
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
#include "overrides.hpp"

#define MAX_LANGUAGE_LINE_LENGTH 1000

enum
{
  OF_OPENPLUGIN,
  OF_OPENFILEPLUGIN,
  OF_CONFIGURE,
  OF_MANAGER,
};

typedef void   WINAPI ClosePlugin_t(HANDLE hPlugin);
typedef int    WINAPI Compare_t(HANDLE hPlugin,const struct PluginPanelItem *Item1,const struct PluginPanelItem *Item2,unsigned int Mode);
typedef int    WINAPI Configure_t(int ItemNumber);
typedef int    WINAPI DeleteFiles_t(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef void   WINAPI ExitFAR_t(void);
typedef void   WINAPI FreeFindData_t(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
typedef void   WINAPI FreeVirtualFindData_t(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
typedef int    WINAPI GetFiles_t(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
typedef int    WINAPI GetFindData_t(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
typedef int    WINAPI GetMinFarVersion_t(void);
typedef void   WINAPI GetOpenPluginInfo_t(HANDLE hPlugin,struct OpenPluginInfo *Info);
typedef void   WINAPI GetPluginInfo_t(struct PluginInfo *Info);
typedef int    WINAPI GetVirtualFindData_t(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,const char *Path);
typedef int    WINAPI MakeDirectory_t(HANDLE hPlugin,char *Name,int OpMode);
typedef HANDLE WINAPI OpenFilePlugin_t(char *Name,const unsigned char *Data,int DataSize);
typedef HANDLE WINAPI OpenPlugin_t(int OpenFrom,int Item);
typedef int    WINAPI ProcessEditorEvent_t(int Event,void *Param);
typedef int    WINAPI ProcessEditorInput_t(const INPUT_RECORD *Rec);
typedef int    WINAPI ProcessEvent_t(HANDLE hPlugin,int Event,void *Param);
typedef int    WINAPI ProcessHostFile_t(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef int    WINAPI ProcessKey_t(HANDLE hPlugin,int Key,unsigned int ControlState);
typedef int    WINAPI ProcessViewerEvent_t(int Event,void *Param);
typedef int    WINAPI PutFiles_t(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
typedef int    WINAPI SetDirectory_t(HANDLE hPlugin,const char *Dir,int OpMode);
typedef int    WINAPI SetFindList_t(HANDLE hPlugin,const struct PluginPanelItem *PanelItem,int ItemsNumber);
typedef void   WINAPI SetStartupInfo_t(const struct PluginStartupInfo *Info);

class CModule
{
private:
  HMODULE hModule;
  char ModulePath[NM];
  int MinFarVersion;
  char Language[MAX_LANGUAGE_LINE_LENGTH];
  char **Messages;
  int MessagesCount;
  struct PluginInfo pi;
  bool loaded;
  CModule(const char *filename);
  bool AddMsg(const char *str);
  void ClearMsgs();
  void UpdateLanguage();

public:
  static CModule *Create(const char *filename);
  ~CModule();
  const char *GetModulePath() {return ModulePath;}
  int CheckMinFarVersion() {return MinFarVersion;}
  const char *GetMsg(int MsgId) {UpdateLanguage(); if (MsgId>=0 && MsgId<MessagesCount) return Messages[MsgId]; else return "";}
  void ModuleSetStartupInfo(const struct PluginStartupInfo *Info);
  void ModuleGetPluginInfo();
  bool IsOpenFrom(int OF, int OpenFrom,int Item);
  bool IsFullCommandline();
  const char *GetPrefixes();
  int GetDiskMenuStringsNumber();
  const char * const *GetDiskMenuStrings();
  int *GetDiskMenuNumbers();
  int GetPluginMenuStringsNumber();
  int GetConfigMenuStringsNumber();
  int GetMenuStringsNumber(int OF,int OpenFrom);
  const char *GetMenuString(int OF,int OpenFrom,int Item);
  const char *GetName();
  ClosePlugin_t         *ClosePlugin;
  Compare_t             *Compare;
  Configure_t           *Configure;
  DeleteFiles_t         *DeleteFiles;
  ExitFAR_t             *ExitFAR;
  FreeFindData_t        *FreeFindData;
  FreeVirtualFindData_t *FreeVirtualFindData;
  GetFiles_t            *GetFiles;
  GetFindData_t         *GetFindData;
  GetMinFarVersion_t    *GetMinFarVersion;
  GetOpenPluginInfo_t   *GetOpenPluginInfo;
  GetPluginInfo_t       *GetPluginInfo;
  GetVirtualFindData_t  *GetVirtualFindData;
  MakeDirectory_t       *MakeDirectory;
  OpenFilePlugin_t      *OpenFilePlugin;
  OpenPlugin_t          *OpenPlugin;
  ProcessEditorEvent_t  *ProcessEditorEvent;
  ProcessEditorInput_t  *ProcessEditorInput;
  ProcessEvent_t        *ProcessEvent;
  ProcessHostFile_t     *ProcessHostFile;
  ProcessKey_t          *ProcessKey;
  ProcessViewerEvent_t  *ProcessViewerEvent;
  PutFiles_t            *PutFiles;
  SetDirectory_t        *SetDirectory;
  SetFindList_t         *SetFindList;
  SetStartupInfo_t      *SetStartupInfo;
};

CModule::CModule(const char *filename)
{
  loaded=false;

  hModule = LoadLibraryEx(filename,NULL,DONT_RESOLVE_DLL_REFERENCES);
  //DONT_RESOLVE_DLL_REFERENCES это чтобы не напрягать длл-ки которые
  //не плагины, если решаем что экспортит достаточно чтоб быть плагином
  //тогда выгружаем и загружаем по нормальному.
  if (!hModule)
    return;

  GetPluginInfo       = (GetPluginInfo_t *)       GetProcAddress(hModule, "GetPluginInfo");
  SetStartupInfo      = (SetStartupInfo_t *)      GetProcAddress(hModule, "SetStartupInfo");
  if (!SetStartupInfo || !GetPluginInfo)
    return;

  FreeLibrary(hModule);

  hModule = LoadLibraryEx(filename,NULL,0);
  if (!hModule)
    return;

  ClosePlugin         = (ClosePlugin_t *)         GetProcAddress(hModule, "ClosePlugin");
  Compare             = (Compare_t *)             GetProcAddress(hModule, "Compare");
  Configure           = (Configure_t *)           GetProcAddress(hModule, "Configure");
  DeleteFiles         = (DeleteFiles_t *)         GetProcAddress(hModule, "DeleteFiles");
  ExitFAR             = (ExitFAR_t *)             GetProcAddress(hModule, "ExitFAR");
  FreeFindData        = (FreeFindData_t *)        GetProcAddress(hModule, "FreeFindData");
  FreeVirtualFindData = (FreeVirtualFindData_t *) GetProcAddress(hModule, "FreeVirtualFindData");
  GetFiles            = (GetFiles_t *)            GetProcAddress(hModule, "GetFiles");
  GetFindData         = (GetFindData_t *)         GetProcAddress(hModule, "GetFindData");
  GetMinFarVersion    = (GetMinFarVersion_t *)    GetProcAddress(hModule, "GetMinFarVersion");
  GetOpenPluginInfo   = (GetOpenPluginInfo_t *)   GetProcAddress(hModule, "GetOpenPluginInfo");
  GetPluginInfo       = (GetPluginInfo_t *)       GetProcAddress(hModule, "GetPluginInfo");
  GetVirtualFindData  = (GetVirtualFindData_t *)  GetProcAddress(hModule, "GetVirtualFindData");
  MakeDirectory       = (MakeDirectory_t *)       GetProcAddress(hModule, "MakeDirectory");
  OpenFilePlugin      = (OpenFilePlugin_t *)      GetProcAddress(hModule, "OpenFilePlugin");
  OpenPlugin          = (OpenPlugin_t *)          GetProcAddress(hModule, "OpenPlugin");
  ProcessEditorEvent  = (ProcessEditorEvent_t *)  GetProcAddress(hModule, "ProcessEditorEvent");
  ProcessEditorInput  = (ProcessEditorInput_t *)  GetProcAddress(hModule, "ProcessEditorInput");
  ProcessEvent        = (ProcessEvent_t *)        GetProcAddress(hModule, "ProcessEvent");
  ProcessHostFile     = (ProcessHostFile_t *)     GetProcAddress(hModule, "ProcessHostFile");
  ProcessKey          = (ProcessKey_t *)          GetProcAddress(hModule, "ProcessKey");
  ProcessViewerEvent  = (ProcessViewerEvent_t *)  GetProcAddress(hModule, "ProcessViewerEvent");
  PutFiles            = (PutFiles_t *)            GetProcAddress(hModule, "PutFiles");
  SetDirectory        = (SetDirectory_t *)        GetProcAddress(hModule, "SetDirectory");
  SetFindList         = (SetFindList_t *)         GetProcAddress(hModule, "SetFindList");
  SetStartupInfo      = (SetStartupInfo_t *)      GetProcAddress(hModule, "SetStartupInfo");

  if (!SetStartupInfo || !GetPluginInfo)
    return;

  lstrcpy(ModulePath,filename);
  *Language = 0;
  Messages = NULL;
  MessagesCount = 0;

  UpdateLanguage();

  if (GetMinFarVersion)
    MinFarVersion = GetMinFarVersion();
  else
    MinFarVersion = 0;

  loaded=true;
}

CModule::~CModule()
{
  if (loaded && ExitFAR)
    ExitFAR();
  if (hModule)
    FreeLibrary(hModule);
}

CModule *CModule::Create(const char *filename)
{
  CModule *module = new CModule(filename);
  if (module)
  {
    if (!module->loaded)
    {
      delete module;
      module = NULL;
    }
  }
  return module;
}

bool ReadLine(HANDLE file,char *buffer,DWORD len)
{
  DWORD CurrPos = SetFilePointer(file,0,NULL,FILE_CURRENT), transferred;
  bool res = true;
  if (ReadFile(file,buffer,len-1,&transferred,NULL) && transferred)
  {
    buffer[len-1] = 0;
    DWORD len = 0;
    char *ptr = buffer;
    for (size_t i=0; i<transferred; i++,len++,ptr++)
    {
      if (buffer[i]=='\n' || buffer[i]=='\r')
      {
        if ((buffer[i+1]=='\n' || buffer[i+1]=='\r') && buffer[i]!=buffer[i+1])
          len++;
        break;
      }
    }
    *ptr = 0;
    SetFilePointer(file,CurrPos+len+1,NULL,FILE_BEGIN);
  }
  else
  {
    buffer[0] = 0;
    res = false;
  }
  return res;
}

void CModule::UpdateLanguage()
{
  char new_language[MAX_LANGUAGE_LINE_LENGTH];
  DWORD res = GetEnvironmentVariable("FARLANG",new_language,sizeof(new_language));
  if (!(res && res<sizeof(new_language)))
    lstrcpy(new_language,"English");
  if (!lstrcmp(Language,new_language))
    return;
  lstrcpy(Language,new_language);
  ClearMsgs();
  char path[NM];
  lstrcpy(path,ModulePath);
  lstrcpy(FSF.PointToName(path),"*.lng");
  char EnglishFile[NM], LastFile[NM], LastFileLanguage[MAX_LANGUAGE_LINE_LENGTH];
  *EnglishFile = 0;
  *LastFile = 0;
  WIN32_FIND_DATA fd;
  HANDLE lfh = INVALID_HANDLE_VALUE;
  HANDLE fh = FindFirstFile(path,&fd);
  if (fh!=INVALID_HANDLE_VALUE)
  {
    do
    {
      if (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
        continue;
      lfh = CreateFile(fd.cFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
      if (lfh==INVALID_HANDLE_VALUE)
        continue;
      char buf[MAX_LANGUAGE_LINE_LENGTH];
      if (!ReadLine(lfh,buf,sizeof(buf)))
      {
        CloseHandle(lfh);
        lfh=INVALID_HANDLE_VALUE;
        continue;
      }
      char *ptr=buf;
      int res=0;
      for (int i=0; i<3; i++)
      {
        while (*ptr==0x20 || *ptr=='\t')
          ptr++;
        switch (i)
        {
          case 0:
            res = lstrlen(ptr)>10; //.Language=
            res = res && !FSF.LStrnicmp(ptr,".Language",9);
            ptr += 9;
            break;
          case 1:
            res = lstrlen(ptr)>2; //=
            res = res && *ptr=='=';
            ptr++;
            break;
          case 2:
          {
            char *ptr_s = ptr;
            while (*ptr && *ptr!=0x20 && *ptr!='\t' && *ptr!=',')
            {
              LastFileLanguage[ptr-ptr_s] = *ptr;
              ptr++;
            }
            LastFileLanguage[ptr-ptr_s] = 0;
            break;
          }
        }
        if (!res)
          break;
      }
      if (res && *LastFileLanguage)
      {
        if (!lstrcmpi(LastFileLanguage,Language))
          break;
        if (!lstrcmpi(LastFileLanguage,"English"))
          lstrcpy(EnglishFile,fd.cFileName);
        else
          lstrcpy(LastFile,fd.cFileName);
      }
      CloseHandle(lfh);
      lfh=INVALID_HANDLE_VALUE;
    } while (FindNextFile(fh,&fd));
    FindClose(fh);
    if (lfh==INVALID_HANDLE_VALUE && (*EnglishFile || *LastFile))
    {
      lfh = CreateFile(*EnglishFile?EnglishFile:LastFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    }
    if (fh!=INVALID_HANDLE_VALUE)
    {
      char buf[MAX_LANGUAGE_LINE_LENGTH];
      while (ReadLine(lfh,buf,sizeof(buf)))
      {
        char *ptr = buf;
        while (*ptr==0x20 || *ptr=='\t')
          ptr++;
        if (*ptr!='"')
          continue;
        ptr++;
        char *ptr_e = buf+lstrlen(buf)-1;
        while (ptr_e>ptr && (*ptr_e==0x20 || *ptr_e=='\t'))
          ptr_e--;
        //if (ptr_e==ptr)
          //continue;
        *ptr_e = 0;
        if (!AddMsg(ptr))
          break;
      }
      CloseHandle(lfh);
      lfh=OpenLog("G:\\Program Files\\Far\\pm\\PluginManager_lang");
      for (int i=0; i<MessagesCount; i++)
      {
        WriteLog(lfh,Messages[i]);
        WriteLog(lfh,"\n");
      }
      CloseLog(&lfh);
    }
  }
}

bool CModule::AddMsg(const char *str)
{
  char **NewMessages = (char **) realloc(Messages,(MessagesCount+1)*sizeof(char *));
  if (!NewMessages)
    return false;
  Messages = NewMessages;
  Messages[MessagesCount] = (char *) malloc((lstrlen(str)+1)*sizeof(char));
  if (Messages[MessagesCount])
  {
    lstrcpy(Messages[MessagesCount++],str);
    return true;
  }
  return false;
}

void CModule::ClearMsgs()
{
  if (Messages)
  {
    for (int i=0; i<MessagesCount; i++)
    {
      if (Messages[i])
        free(Messages[i]);
    }
    free(Messages);
    MessagesCount = 0;
  }
}

void CModule::ModuleSetStartupInfo(const struct PluginStartupInfo *Info)
{
  struct PluginStartupInfo psi;
  psi = *Info;
  lstrcpy(psi.ModuleName,ModulePath);
  psi.ModuleNumber     = (int)this;
  psi.Menu             = MMenu;
  psi.Dialog           = MDialog;
  psi.Message          = MMessage;
  psi.GetMsg           = MGetMsg;
  psi.Control          = MControl;
  //psi.SaveScreen       = MSaveScreen;
  //psi.RestoreScreen    = MRestoreScreen;
  //psi.GetDirList       = MGetDirList;
  psi.GetPluginDirList = MGetPluginDirList;
  //psi.FreeDirList      = MFreeDirList;
  //psi.Viewer           = MViewer;
  //psi.Editor           = MEditor;
  //psi.CmpName          = MCmpName;
  //psi.CharTable        = MCharTable;
  //psi.Text             = MText;
  //psi.EditorControl    = MEditorControl;
  //psi.ShowHelp         = MShowHelp;
  psi.AdvControl       = MAdvControl;
  //psi.InputBox         = MInputBox;
  psi.DialogEx         = MDialogEx;
  //psi.SendDlgMessage   = MSendDlgMessage;
  //psi.DefDlgProc       = MDefDlgProc;
  psi.Reserved[1]      = (DWORD) MViewerControl;
  SetStartupInfo(&psi);
}

void CModule::ModuleGetPluginInfo()
{
  memset(&pi,0,sizeof(pi));
  GetPluginInfo(&pi);
}

bool CheckPrefix(const char *Prefixes,const char *Prefix)
{
  const char *ptr=Prefixes;
  int len=lstrlen(Prefix);
  while (*ptr)
  {
    if (!FSF.LStrnicmp(ptr,Prefix,len) && (*(ptr+len)==':' || *(ptr+len)==0))
      return true;
    while (*ptr && *ptr!=':')
      ptr++;
    while (*ptr==':')
      ptr++;
  }
  return false;
}

bool CModule::IsOpenFrom(int OF, int OpenFrom, int Item)
{
  switch (OF)
  {
    case OF_OPENPLUGIN:
      if (OpenPlugin)
      {
        switch (OpenFrom)
        {
          case OPEN_DISKMENU:
            if (pi.DiskMenuStringsNumber)
              return true;
            break;
          case OPEN_PLUGINSMENU:
            if (pi.PluginMenuStringsNumber && !(pi.Flags&PF_DISABLEPANELS))
              return true;
            break;
          case OPEN_FINDLIST:
            if (SetFindList)
              return true;
            break;
          case OPEN_SHORTCUT:
            if (!(pi.Flags&PF_DISABLEPANELS) && Item && !FSF.LStricmp((char *)Item,ModulePath))
              return true;
            break;
          case OPEN_COMMANDLINE:
            if (pi.CommandPrefix && Item && CheckPrefix(pi.CommandPrefix,(char *)Item))
              return true;
            break;
          case OPEN_EDITOR:
            if (pi.PluginMenuStringsNumber && pi.Flags&PF_EDITOR)
              return true;
            break;
          case OPEN_VIEWER:
            if (pi.PluginMenuStringsNumber && pi.Flags&PF_VIEWER)
              return true;
            break;
        }
      }
      break;
    case OF_OPENFILEPLUGIN:
      if (OpenFilePlugin)
        return true;
      break;
    case OF_CONFIGURE:
      if (Configure && pi.PluginConfigStringsNumber)
        return true;
      break;
    case OF_MANAGER:
      return true;
  }
  return false;
}

inline bool CModule::IsFullCommandline()
{
  return pi.Flags&PF_FULLCMDLINE;
}

inline const char *CModule::GetPrefixes()
{
  return pi.CommandPrefix;
}

inline int CModule::GetDiskMenuStringsNumber()
{
  return pi.DiskMenuStringsNumber;
}

inline int CModule::GetPluginMenuStringsNumber()
{
  return pi.PluginMenuStringsNumber;
}

inline int CModule::GetConfigMenuStringsNumber()
{
  return pi.PluginConfigStringsNumber;
}

inline const char * const *CModule::GetDiskMenuStrings()
{
  return pi.DiskMenuStrings;
}

inline int *CModule::GetDiskMenuNumbers()
{
  return pi.DiskMenuNumbers;
}

int CModule::GetMenuStringsNumber(int OF, int OpenFrom)
{
  switch (OF)
  {
    case OF_OPENPLUGIN:
      if (OpenFrom==OPEN_PLUGINSMENU||OpenFrom==OPEN_VIEWER||OpenFrom==OPEN_EDITOR)
        return pi.PluginMenuStringsNumber;
      break;
    case OF_CONFIGURE:
      return pi.PluginConfigStringsNumber;
  }
  return 1;
}

const char *CModule::GetMenuString(int OF, int OpenFrom, int Item)
{
  switch (OF)
  {
    case OF_OPENPLUGIN:
      if (OpenFrom==OPEN_PLUGINSMENU||OpenFrom==OPEN_VIEWER||OpenFrom==OPEN_EDITOR)
        return ((pi.PluginMenuStringsNumber>Item&&Item>=0)?pi.PluginMenuStrings[Item]:FSF.PointToName(ModulePath));
      break;
    case OF_CONFIGURE:
      return ((pi.PluginConfigStringsNumber>Item&&Item>=0)?pi.PluginConfigStrings[Item]:FSF.PointToName(ModulePath));
  }
  return FSF.PointToName(ModulePath);
}

inline const char *CModule::GetName()
{
  return ModulePath;
}
