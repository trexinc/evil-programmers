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
#define MAX_OPEN_PANEL_PLUGINS 6
//as I get it the max is 3, but in any case

class PluginManager
{
private:
  struct DiskMenuTrackItem
  {
    int PluginNumber;
    int Item;
  };
  struct PluginsMenu
  {
    FarMenuItemEx *menu;
    int menu_items;
  };
  char plugins_dir[NM];
  struct PluginStartupInfo psi;
  FARSTANDARDFUNCTIONS psiFSF;
  CModule **PluginsData;
  CFileSystem *OpenPanelPluginsData[MAX_OPEN_PANEL_PLUGINS];
  int PluginsCount;
  int OpenPanelPluginsCount;
  int DiskMenuStringsNumber;
  const char **DiskMenuStrings;
  int *DiskMenuNumbers;
  DiskMenuTrackItem *DiskMenuPlugins;
  int **DiskMenuNumbersPtr;
  char *CommandPrefixes;
  int ConfigMenuStringsNumber;
  int PluginMenuStringsNumber;
  void AddPluginInfoStuff(int PluginNumber,const char * const *strs,int strsnum,int *nums,const char *prefixes);
  static int WINAPI FindPlugin(const WIN32_FIND_DATA *fdata,const char *filename,PluginManager *self)
  {
    char dir[1024];
    lstrcpy(dir,filename);
    *(FSF.PointToName(dir))=0;
    SetCurrentDirectory(dir);
    self->AddPlugin(filename);
    return TRUE;
  }
  void AddPlugin(const char *filename);
  bool UnloadPlugin(int PluginNumber);
  void UnloadAllPlugins();
  bool ReloadAllPlugins();
  bool CreateMenu(struct PluginsMenu *pmenu,int Items,int OF,int OpenFrom,int Item);
  int ShowMenu(struct PluginsMenu *pmenu,int Items,int OF,int OpenFrom,int Item);
  void DestroyMenu(struct PluginsMenu *pmenu);

public:
  PluginManager(char *plug_dir,const struct PluginStartupInfo *Info);
  ~PluginManager();
  void AddRef(CFileSystem *fs);
  void DelRef(CFileSystem *fs);
  void SetStartupInfo();
  void GetPluginInfo(struct PluginInfo *pi);
  int ProcessEditorInput(const INPUT_RECORD *Rec);
  int ProcessEditorEvent(int Event,void *Param);
  int ProcessViewerEvent(int Event,void *Param);
  int Manager();
  int Configure();
  HANDLE OpenPlugin(int OpenFrom,int Item);
  HANDLE OpenFilePlugin(char *Name,const unsigned char *Data,int DataSize);
  void ClosePlugin(HANDLE hPlugin);
  HANDLE GetFS(HANDLE hPlugin);
  bool CheckOpen(CFileSystem *fs);
};

PluginManager::PluginManager(char *plug_dir,const struct PluginStartupInfo *Info)
{
  lstrcpy(plugins_dir,plug_dir);
  psi = *Info;
  psiFSF = *Info->FSF;
  psi.FSF = &psiFSF;
  PluginsData = NULL;
  PluginsCount = 0;
  OpenPanelPluginsCount = 0;
  ConfigMenuStringsNumber = 0;
  PluginMenuStringsNumber = 0;
  DiskMenuStringsNumber = 0;
  DiskMenuStrings = NULL;
  DiskMenuNumbers = NULL;
  DiskMenuPlugins = NULL;
  DiskMenuNumbersPtr = NULL;
  CommandPrefixes = NULL;
  FSF.FarRecursiveSearch(plug_dir,"*.dll",(FRSUSERFUNC)FindPlugin,FRS_RECUR,this);
}

void PluginManager::UnloadAllPlugins()
{
  if (DiskMenuStrings) free(DiskMenuStrings);
  if (DiskMenuNumbers) free(DiskMenuNumbers);
  if (DiskMenuPlugins) free(DiskMenuPlugins);
  if (DiskMenuNumbersPtr) free(DiskMenuNumbersPtr);
  if (CommandPrefixes) free(CommandPrefixes);
  //for (int i=0; i<OpenPanelPluginsCount; i++)
    //delete OpenPanelPluginsData[i];
  for (int i=0; i<PluginsCount; i++)
    delete PluginsData[i];
  free(PluginsData);
  PluginsData = NULL;
  PluginsCount = 0;
  OpenPanelPluginsCount = 0;
  DiskMenuStrings = NULL;
  DiskMenuNumbers = NULL;
  DiskMenuPlugins = NULL;
  DiskMenuNumbersPtr = NULL;
  CommandPrefixes = NULL;
  ConfigMenuStringsNumber = 0;
  PluginMenuStringsNumber = 0;
  DiskMenuStringsNumber = 0;
}

PluginManager::~PluginManager()
{
  UnloadAllPlugins();
}

bool PluginManager::ReloadAllPlugins()
{
  if (OpenPanelPluginsCount)
  {
    ErrorMessage(MOpenPanels2);
    return false;
  }
  UnloadAllPlugins();
  {
    char dir[1024];
    GetCurrentDirectory(sizeof(dir),dir);
    FSF.FarRecursiveSearch(plugins_dir,"*.dll",(FRSUSERFUNC)FindPlugin,FRS_RECUR,this);
    SetCurrentDirectory(dir);
  }
  SetStartupInfo();
  struct PluginInfo pi;
  memset(&pi,0,sizeof(pi));
  GetPluginInfo(&pi);
  return true;
}

bool PluginManager::UnloadPlugin(int PluginNumber)
{
  CModule *module = PluginsData[PluginNumber];
  for (int i=0; i<OpenPanelPluginsCount; i++)
  {
    if (OpenPanelPluginsData[i]->IsModule(module))
    {
      ErrorMessage(MOpenPanels1);
      return false;
      //delete OpenPanelPluginsData[i];
      //OpenPanelPluginsData[i] = NULL;
    }
  }
  /*
  for (int i=0; i<OpenPanelPluginsCount; i++)
  {
    if (!OpenPanelPluginsData[i])
    {
      for (int j=i+1, k=i; j<OpenPanelPluginsCount; j++, k++)
        OpenPanelPluginsData[k] = OpenPanelPluginsData[j];
      OpenPanelPluginsCount--;
    }
  }
  */
  delete module;
  for (int i=PluginNumber+1; i<PluginsCount; i++)
    PluginsData[i-1] = PluginsData[i];
  PluginsCount--;
  struct PluginInfo pi;
  GetPluginInfo(&pi);
  return true;
}

void PluginManager::AddRef(CFileSystem *fs)
{
  if (OpenPanelPluginsCount<MAX_OPEN_PANEL_PLUGINS)
  {
    OpenPanelPluginsData[OpenPanelPluginsCount++] = fs;
  }
  /*
  else
  {
    MessageBox(NULL,"The fucking end of the workd has come!!!!!","Important message",MB_OK);
  }
  */
}

void PluginManager::DelRef(CFileSystem *fs)
{
  for (int i=0; i<OpenPanelPluginsCount; i++)
  {
    if (OpenPanelPluginsData[i] == fs)
    {
      for (int j=i; j<(OpenPanelPluginsCount-1); j++)
        OpenPanelPluginsData[j] = OpenPanelPluginsData[j+1];
      OpenPanelPluginsCount--;
      break;
    }
  }
}

void PluginManager::AddPlugin(const char *filename)
{
  CModule **NewPluginsData = (CModule **) realloc(PluginsData,(PluginsCount+1)*sizeof(CModule *));
  if (NewPluginsData)
  {
    PluginsData = NewPluginsData;
    PluginsData[PluginsCount] = CModule::Create(filename);
    if (!PluginsData[PluginsCount])
    {
      PluginsData = (CModule **) realloc(PluginsData,(PluginsCount)*sizeof(CModule *));
    }
    else
    {
      PluginsCount++;
    }
  }
}

void PluginManager::SetStartupInfo()
{
  for (int i=0; i<PluginsCount; i++)
  {
    PluginsData[i]->ModuleSetStartupInfo(&psi);
  }
}

void PluginManager::AddPluginInfoStuff(int PluginNumber,const char * const *strs,int strsnum,int *nums,const char *prefixes)
{
  if (strs)
  {
    DiskMenuStrings = (const char **) realloc(DiskMenuStrings,(DiskMenuStringsNumber+strsnum)*sizeof(const char *));
    DiskMenuNumbers = (int *) realloc(DiskMenuNumbers,(DiskMenuStringsNumber+strsnum)*sizeof(int));
    DiskMenuPlugins = (DiskMenuTrackItem *) realloc(DiskMenuPlugins,(DiskMenuStringsNumber+strsnum)*sizeof(DiskMenuTrackItem));
    DiskMenuNumbersPtr = (int **) realloc(DiskMenuNumbersPtr,(DiskMenuStringsNumber+strsnum)*sizeof(int *));
    for (int i=0; i<strsnum; i++)
    {
      if (!strs[i])
        continue;
      DiskMenuStrings[DiskMenuStringsNumber] = strs[i];
      DiskMenuNumbers[DiskMenuStringsNumber] = nums?nums[i]:0;
      DiskMenuPlugins[DiskMenuStringsNumber].PluginNumber = PluginNumber;
      DiskMenuPlugins[DiskMenuStringsNumber].Item = i;
      DiskMenuNumbersPtr[DiskMenuStringsNumber] = nums?nums+i:NULL;
      DiskMenuStringsNumber++;
    }
  }
  if (prefixes && *prefixes)
  {
    CommandPrefixes = (char *) realloc(CommandPrefixes,(lstrlen(CommandPrefixes)+lstrlen(prefixes)+2)*sizeof(char));
    if (*CommandPrefixes)
      lstrcat(CommandPrefixes,":");
    const char *ptr=prefixes;
    while (*ptr==':')
      ptr++;
    lstrcat(CommandPrefixes,ptr);
    while (*CommandPrefixes && CommandPrefixes[lstrlen(CommandPrefixes)-1]==':')
      CommandPrefixes[lstrlen(CommandPrefixes)-1]=0;
    CommandPrefixes = (char *) realloc(CommandPrefixes,(lstrlen(CommandPrefixes)+1)*sizeof(char));
  }
}

void PluginManager::GetPluginInfo(struct PluginInfo *pi)
{
  ConfigMenuStringsNumber = 0;
  PluginMenuStringsNumber = 0;
  DiskMenuStringsNumber = 0;
  if (DiskMenuStrings) {free(DiskMenuStrings); DiskMenuStrings=NULL;}
  if (DiskMenuNumbers) {free(DiskMenuNumbers); DiskMenuNumbers=NULL;}
  if (DiskMenuPlugins) {free(DiskMenuPlugins); DiskMenuPlugins=NULL;}
  if (DiskMenuNumbersPtr) {free(DiskMenuNumbersPtr); DiskMenuNumbersPtr=NULL;}
  if (CommandPrefixes) {free(CommandPrefixes); CommandPrefixes=NULL;}
  pi->StructSize = sizeof(struct PluginInfo);
  pi->Flags = PF_EDITOR|PF_VIEWER|PF_PRELOAD|PF_FULLCMDLINE;
  static const char *MenuStrings[1];
  static const char *ConfigStrings[2];
  MenuStrings[0] = GetMsg(MTitle);
  ConfigStrings[0] = GetMsg(MConfig1);
  ConfigStrings[1] = GetMsg(MConfig2);
  pi->PluginMenuStrings = MenuStrings;
  pi->PluginMenuStringsNumber = 1;
  pi->PluginConfigStrings = ConfigStrings;
  pi->PluginConfigStringsNumber = 2;
  for (int i=0; i<PluginsCount; i++)
  {
    PluginsData[i]->ModuleGetPluginInfo();
    AddPluginInfoStuff(i,PluginsData[i]->GetDiskMenuStrings(),PluginsData[i]->GetDiskMenuStringsNumber(),PluginsData[i]->GetDiskMenuNumbers(),PluginsData[i]->GetPrefixes());
    ConfigMenuStringsNumber += PluginsData[i]->GetConfigMenuStringsNumber();
    PluginMenuStringsNumber += PluginsData[i]->GetPluginMenuStringsNumber();
  }
  pi->DiskMenuStringsNumber=DiskMenuStringsNumber;
  pi->DiskMenuStrings=DiskMenuStrings;
  pi->DiskMenuNumbers=DiskMenuNumbers;
  pi->CommandPrefix=CommandPrefixes;
}

int PluginManager::ProcessEditorInput(const INPUT_RECORD *Rec)
{
  for (int i=0; i<PluginsCount; i++)
  {
    if (!PluginsData[i]->ProcessEditorInput)
      continue;
    int ret = PluginsData[i]->ProcessEditorInput(Rec);
    if (ret)
      return ret;
  }
  return 0;
}

int PluginManager::ProcessEditorEvent(int Event,void *Param)
{
  for (int i=0; i<PluginsCount; i++)
  {
    if (!PluginsData[i]->ProcessEditorEvent)
      continue;
    PluginsData[i]->ProcessEditorEvent(Event,Param);
  }
  return 0;
}

int PluginManager::ProcessViewerEvent(int Event,void *Param)
{
  for (int i=0; i<PluginsCount; i++)
  {
    if (!PluginsData[i]->ProcessViewerEvent)
      continue;
    PluginsData[i]->ProcessViewerEvent(Event,Param);
  }
  return 0;
}

bool PluginManager::CreateMenu(struct PluginsMenu *pmenu,int Items,int OF,int OpenFrom,int Item)
{
  pmenu->menu = (FarMenuItemEx *)malloc(Items*sizeof(FarMenuItemEx));
  if (!pmenu->menu)
    return false;
  pmenu->menu_items=0;
  for (int i=0; i<PluginsCount; i++)
  {
    if (PluginsData[i]->IsOpenFrom(OF,OpenFrom,Item))
    {
      for (int j=0; j<PluginsData[i]->GetMenuStringsNumber(OF,OpenFrom); j++)
      {
        memset(&pmenu->menu[pmenu->menu_items],0,sizeof(FarMenuItemEx));
        pmenu->menu[pmenu->menu_items].Flags=MIF_USETEXTPTR;
        pmenu->menu[pmenu->menu_items].Text.TextPtr=PluginsData[i]->GetMenuString(OF,OpenFrom,j);
        pmenu->menu[pmenu->menu_items].UserData=MAKELONG(j,i);
        pmenu->menu_items++;
      }
    }
  }
  return true;
}

int PluginManager::ShowMenu(struct PluginsMenu *pmenu,int Items,int OF,int OpenFrom,int Item)
{
  if (pmenu->menu_items==1 && ((OF==OF_OPENPLUGIN && (OpenFrom==OPEN_COMMANDLINE || OpenFrom==OPEN_SHORTCUT || OpenFrom==OPEN_FINDLIST)) || (OF==OF_OPENFILEPLUGIN)))
    return 0;

  int BreakCode;
  int BreakKeys[] = {(PKF_CONTROL<<16)|'R',VK_DELETE,0};
  while (true)
  {
    int ret=Info.Menu(Info.ModuleNumber,-1,-1,0,
                      FMENU_SHOWAMPERSAND|FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT|FMENU_USEEXT,
                      GetMsg(MTitle),GetMsg(MBottom),NULL,BreakKeys,&BreakCode,
                      (const struct FarMenuItem *)pmenu->menu,pmenu->menu_items);
    if (BreakCode<0)
      return ret;

    bool res;
    if (BreakCode==0)
      res = ReloadAllPlugins();
    else
      res = UnloadPlugin(HIWORD(pmenu->menu[ret].UserData));
    if (!res)
      continue;
    DestroyMenu(pmenu);
    if (!CreateMenu(pmenu,Items,OF,OpenFrom,Item))
      return -1;
  }
}

void PluginManager::DestroyMenu(struct PluginsMenu *pmenu)
{
  if (pmenu->menu)
    free(pmenu->menu);
  pmenu->menu = NULL;
  pmenu->menu_items = 0;
}

int PluginManager::Configure()
{
  PluginsMenu pmenu;
  if (CreateMenu(&pmenu,ConfigMenuStringsNumber,OF_CONFIGURE,0,0))
  {
    while (true)
    {
      int mn=ShowMenu(&pmenu,ConfigMenuStringsNumber,OF_CONFIGURE,0,0);
      if (mn>=0)
      {
        CModule *module = PluginsData[HIWORD(pmenu.menu[mn].UserData)];
        module->Configure(LOWORD(pmenu.menu[mn].UserData));
      }
      else
        break;
    }
    DestroyMenu(&pmenu);
  }
  return FALSE;
}

int PluginManager::Manager()
{
  PluginsMenu pmenu;
  if (CreateMenu(&pmenu,PluginsCount,OF_MANAGER,0,0))
  {
    ShowMenu(&pmenu,PluginsCount,OF_MANAGER,0,0);
    DestroyMenu(&pmenu);
  }
  return FALSE;
}

HANDLE PluginManager::OpenPlugin(int OpenFrom,int Item)
{
  if (OpenFrom==OPEN_DISKMENU)
  {
    if (PluginsCount)
    {
      CModule *module = PluginsData[DiskMenuPlugins[Item].PluginNumber];
      HANDLE hPlugin = module->OpenPlugin(OpenFrom,DiskMenuPlugins[Item].Item);
      if (hPlugin!=INVALID_HANDLE_VALUE && hPlugin!=NULL)
      {
        CFileSystem *fs = CFileSystem::Create(module,hPlugin);
        if (fs)
        {
          AddRef(fs);
          return (HANDLE) fs;
        }
      }
    }
    return INVALID_HANDLE_VALUE;
  }
  int AllocCount = PluginsCount;
  char buf[MAXSIZE_SHORTCUTDATA];
  char *ptr=NULL;
  if (OpenFrom==OPEN_SHORTCUT)
  {
    ptr = Item?strstr((char *)Item,"|"):NULL;
    if (!ptr)
      return INVALID_HANDLE_VALUE;
    lstrcpyn(buf,(char *)Item,ptr-((char *)Item)+1);
    Item=(int)buf;
  }
  else if (OpenFrom==OPEN_COMMANDLINE)
  {
    ptr = Item?strstr((char *)Item,":"):NULL;
    if (!ptr)
      return INVALID_HANDLE_VALUE;
    *ptr = 0;
  }
  else
  {
    AllocCount = PluginMenuStringsNumber;
  }
  PluginsMenu pmenu;
  if (CreateMenu(&pmenu,AllocCount,OF_OPENPLUGIN,OpenFrom,Item))
  {
    int mn=ShowMenu(&pmenu,AllocCount,OF_OPENPLUGIN,OpenFrom,Item);
    if (mn<0)
    {
      DestroyMenu(&pmenu);
      return INVALID_HANDLE_VALUE;
    }
    CModule *module = PluginsData[HIWORD(pmenu.menu[mn].UserData)];
    if (OpenFrom==OPEN_SHORTCUT)
    {
      lstrcpy(buf,ptr+1);
    }
    else if (OpenFrom==OPEN_COMMANDLINE)
    {
      *ptr = ':';
      if (!module->IsFullCommandline())
        Item = (int)(ptr+1);
    }
    else
    {
      Item = LOWORD(pmenu.menu[mn].UserData);
    }
    DestroyMenu(&pmenu);
    HANDLE hPlugin = module->OpenPlugin(OpenFrom,Item);
    if (hPlugin!=INVALID_HANDLE_VALUE && hPlugin!=NULL)
    {
      CFileSystem *fs = CFileSystem::Create(module,hPlugin);
      if (fs)
      {
        AddRef(fs);
        return (HANDLE) fs;
      }
    }
  }
  return INVALID_HANDLE_VALUE;
}

HANDLE PluginManager::OpenFilePlugin(char *Name,const unsigned char *Data,int DataSize)
{
  if (PluginsCount)
  {
    if (!Name)
    {
      PluginsMenu pmenu;
      if (CreateMenu(&pmenu,PluginsCount,OF_OPENFILEPLUGIN,0,0))
      {
        int mn=ShowMenu(&pmenu,PluginsCount,OF_OPENFILEPLUGIN,0,0);
        if (mn<0)
        {
          DestroyMenu(&pmenu);
          return INVALID_HANDLE_VALUE;
        }
        CModule *module = PluginsData[HIWORD(pmenu.menu[mn].UserData)];
        DestroyMenu(&pmenu);
        HANDLE hPlugin = module->OpenFilePlugin(Name,Data,DataSize);
        if (((int)hPlugin)>0)
        {
          CFileSystem *fs = CFileSystem::Create(module,hPlugin);
          if (fs)
          {
            AddRef(fs);
            return (HANDLE) fs;
          }
        }
        else if (((int)hPlugin)==-2)
          return hPlugin;
      }
    }
    else
    {
      for (int i=0; i<PluginsCount; i++)
      {
        if (!PluginsData[i]->OpenFilePlugin)
          continue;
        HANDLE hPlugin = PluginsData[i]->OpenFilePlugin(Name,Data,DataSize);
        if (((int)hPlugin)>0)
        {
          CFileSystem *fs = CFileSystem::Create(PluginsData[i],hPlugin);
          if (fs)
          {
            AddRef(fs);
            return (HANDLE) fs;
          }
          else
            return INVALID_HANDLE_VALUE;
        }
        else if (((int)hPlugin)==-2)
          return hPlugin;
      }
    }
  }
  return INVALID_HANDLE_VALUE;
}

void PluginManager::ClosePlugin(HANDLE hPlugin)
{
  CFileSystem *fs = (CFileSystem *) hPlugin;
  DelRef(fs);
  delete fs;
}

HANDLE PluginManager::GetFS(HANDLE hPlugin)
{
  if (hPlugin==INVALID_HANDLE_VALUE)
    return INVALID_HANDLE_VALUE;
  for (int i=0; i<OpenPanelPluginsCount; i++)
  {
    if (OpenPanelPluginsData[i]->GetHandle() == hPlugin)
      return (HANDLE) OpenPanelPluginsData[i];
  }
  return INVALID_HANDLE_VALUE;
}

bool PluginManager::CheckOpen(CFileSystem *fs)
{
  if (fs)
   for (int i=0; i<OpenPanelPluginsCount; i++)
     if (OpenPanelPluginsData[i]==fs)
       return true;
  return false;
}
