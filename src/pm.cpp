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
class PluginManager
{
private:
  WFX **PluginsData;
  int PluginsCount;
  char ini_path[1024];
  static int WINAPI FindPlugin(const WIN32_FIND_DATA *fdata,const char *filename,PluginManager *self)
  {
    char dir[1024];
    lstrcpy(dir,filename);
    *(FSF.PointToName(dir))=0;
    SetCurrentDirectory(dir);
    self->AddPlugin(filename);
    return TRUE;
  };
  void AddPlugin(const char *filename);
public:
  PluginManager(char *plug_dir);
  ~PluginManager();
  FileSystem *SelectFS(bool showsingle);
};

PluginManager::PluginManager(char *plug_dir)
{
  PluginsData=NULL;
  PluginsCount=0;
  lstrcpy(ini_path,plug_dir);
  FSF.AddEndSlash(ini_path);
  lstrcat(ini_path,"wfx.ini");
  OemToChar(ini_path,ini_path);
  FSF.FarRecursiveSearch(plug_dir,"*.wfx",(FRSUSERFUNC)FindPlugin,FRS_RECUR,this);
}

PluginManager::~PluginManager()
{
  for (int i=0; i<PluginsCount; i++)
    delete PluginsData[i];
  free(PluginsData);
  PluginsData=NULL;
  PluginsCount=0;
}

/*
int WINAPI PluginManager::FindPlugin(const WIN32_FIND_DATA *fdata,const char *filename,PluginManager *self)
{
  (void)fdata;
  self->AddPlugin(filename);
  return TRUE;
}
*/

void PluginManager::AddPlugin(const char *filename)
{
  WFX **NewPluginsData = (WFX **) realloc(PluginsData,(PluginsCount+1)*sizeof(WFX *));
  if (NewPluginsData)
  {
    PluginsData = NewPluginsData;
    PluginsData[PluginsCount] = WFX::Create(filename);
    if (!PluginsData[PluginsCount])
    {
      PluginsData = (WFX **) realloc(PluginsData,(PluginsCount)*sizeof(WFX *));
    }
    else
    {
      PluginsCount++;
    }
  }
}

FileSystem *PluginManager::SelectFS(bool showsingle)
{
  FileSystem *Result=NULL;
  if ((showsingle && PluginsCount>0) || PluginsCount>1)
  {
    FarMenuItemEx *menu = (FarMenuItemEx *)malloc(PluginsCount*sizeof(FarMenuItemEx));
    if (menu)
    {
      memset(menu,0,sizeof(menu));
      for (int i=0; i<PluginsCount; i++)
      {
        menu[i].Flags=MIF_USETEXTPTR;
        menu[i].Text.TextPtr=PluginsData[i]->Title();
      }
      int ret=Info.Menu(Info.ModuleNumber,-1,-1,0,
                FMENU_SHOWAMPERSAND|FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT|FMENU_USEEXT,
                GetMsg(MTitle),NULL,NULL,NULL,NULL,(const struct FarMenuItem *)menu,PluginsCount);
      free(menu);
      if (ret>=0)
        Result=FileSystem::Create(PluginsData[ret],ini_path);
    }
  }
  else if (PluginsCount==1)
    Result=FileSystem::Create(PluginsData[0],ini_path);
  return Result;
}
