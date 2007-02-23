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
class CFileSystem
{
private:
  HANDLE hPlugin;
  CModule *module;
  bool loaded;
  CFileSystem(CModule *module, HANDLE hPlugin);

public:
  static CFileSystem *Create(CModule *module, HANDLE hPlugin);
  ~CFileSystem();
  HANDLE GetHandle();
  bool IsModule(CModule *module);
  //void ClosePlugin();
  void GetOpenPluginInfo(struct OpenPluginInfo *Info);
  int  GetFindData(struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
  void FreeFindData(struct PluginPanelItem *PanelItem,int ItemsNumber);
  int  GetVirtualFindData(struct PluginPanelItem **pPanelItem,int *pItemsNumber,const char *Path);
  void FreeVirtualFindData(struct PluginPanelItem *PanelItem,int ItemsNumber);
  int  Compare(const struct PluginPanelItem *Item1,const struct PluginPanelItem *Item2,unsigned int Mode);
  int  ProcessKey(int Key,unsigned int ControlState);
  int  ProcessEvent(int Event,void *Param);
  int  ProcessHostFile(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
  int  GetFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
  int  PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
  int  DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
  int  SetDirectory(const char *Dir,int OpMode);
  int  MakeDirectory(char *Name,int OpMode);
  int  SetFindList(const struct PluginPanelItem *PanelItem,int ItemsNumber);
};

CFileSystem::CFileSystem(CModule *module, HANDLE hPlugin)
{
  loaded=false;

  this->module = module;
  this->hPlugin = hPlugin;

  if (!module || ((int)hPlugin)<=0)
    return;

  loaded=true;
}

CFileSystem *CFileSystem::Create(CModule *module, HANDLE hPlugin)
{
  CFileSystem *fs = new CFileSystem(module,hPlugin);
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

CFileSystem::~CFileSystem()
{
  if (loaded && module->ClosePlugin)
    module->ClosePlugin(hPlugin);
}

inline HANDLE CFileSystem::GetHandle()
{
  return hPlugin;
}

inline bool CFileSystem::IsModule(CModule *module)
{
  return this->module==module;
}

void CFileSystem::GetOpenPluginInfo(struct OpenPluginInfo *Info)
{
  if (module->GetOpenPluginInfo)
  {
    module->GetOpenPluginInfo(hPlugin,Info);
    if (Info->Flags&OPIF_REALNAMES)
    {
      DWORD Flags = 0;
      if (!module->GetFiles)
        Flags |= OPIF_EXTERNALGET;
      if (!module->PutFiles)
        Flags |= OPIF_EXTERNALPUT;
      if (!module->DeleteFiles)
        Flags |= OPIF_EXTERNALDELETE;
      if (!module->MakeDirectory)
        Flags |= OPIF_EXTERNALMKDIR;
      Info->Flags |= Flags;
    }
    if ((MAXSIZE_SHORTCUTDATA-lstrlen(Info->ShortcutData)-1)>=lstrlen(module->GetName()))
    {
      static char buf[MAXSIZE_SHORTCUTDATA];
      FSF.sprintf(buf,"%s|%s",module->GetName(),Info->ShortcutData?Info->ShortcutData:"");
      Info->ShortcutData=buf;
    }
  }
  else
    Info->StructSize=sizeof(*Info);
}

int CFileSystem::GetFindData(struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  if (module->GetFindData)
    return module->GetFindData(hPlugin,pPanelItem,pItemsNumber,OpMode);
  return FALSE;
}

void CFileSystem::FreeFindData(struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  if (module->FreeFindData)
    module->FreeFindData(hPlugin,PanelItem,ItemsNumber);
}

int CFileSystem::GetVirtualFindData(struct PluginPanelItem **pPanelItem,int *pItemsNumber,const char *Path)
{
  if (module->GetVirtualFindData)
    return module->GetVirtualFindData(hPlugin,pPanelItem,pItemsNumber,Path);
  return FALSE;
}

void CFileSystem::FreeVirtualFindData(struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  if (module->FreeVirtualFindData)
    module->FreeVirtualFindData(hPlugin,PanelItem,ItemsNumber);
}

int CFileSystem::Compare(const struct PluginPanelItem *Item1,const struct PluginPanelItem *Item2,unsigned int Mode)
{
  if (module->Compare)
    return module->Compare(hPlugin,Item1,Item2,Mode);
  return -2;
}

int CFileSystem::ProcessKey(int Key,unsigned int ControlState)
{
  if (module->ProcessKey)
    return module->ProcessKey(hPlugin,Key,ControlState);
  return FALSE;
}

int CFileSystem::ProcessEvent(int Event,void *Param)
{
  if (module->ProcessEvent)
    return module->ProcessEvent(hPlugin,Event,Param);
  return FALSE;
}

int CFileSystem::ProcessHostFile(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  if (module->ProcessHostFile)
    return module->ProcessHostFile(hPlugin,PanelItem,ItemsNumber,OpMode);
  return FALSE;
}

int CFileSystem::GetFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode)
{
  if (module->GetFiles)
    return module->GetFiles(hPlugin,PanelItem,ItemsNumber,Move,DestPath,OpMode);
  return 0;
}

int CFileSystem::PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode)
{
  if (module->PutFiles)
    return module->PutFiles(hPlugin,PanelItem,ItemsNumber,Move,OpMode);
  return 0;
}

int CFileSystem::DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  if (module->DeleteFiles)
    return module->DeleteFiles(hPlugin,PanelItem,ItemsNumber,OpMode);
  return FALSE;
}

int CFileSystem::SetDirectory(const char *Dir,int OpMode)
{
  if (module->SetDirectory)
    return module->SetDirectory(hPlugin,Dir,OpMode);
  return FALSE;
}

int CFileSystem::MakeDirectory(char *Name,int OpMode)
{
  if (module->MakeDirectory)
    return module->MakeDirectory(hPlugin,Name,OpMode);
  return 0;
}

int CFileSystem::SetFindList(const struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  if (module->SetFindList)
    return module->SetFindList(hPlugin,PanelItem,ItemsNumber);
  return FALSE;
}
