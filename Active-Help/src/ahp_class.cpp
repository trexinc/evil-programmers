/*
    Active-Help plugin for FAR Manager
    Copyright (C) 2002 Alex Yaroslavsky

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
#include "Active-Help.hpp"
#include "ahp_internal.hpp"
#include "ahp.hpp"
#include "memory.hpp"
#include "common.hpp"
#include "crt.hpp"
#include "language.hpp"

char PluginPluginsRootKey[100];

void GetLanguage(char *lang, DWORD size)
{
  *lang=0;
  DWORD res=GetEnvironmentVariable("FARLANG",lang,size);
  if(!(res&&res<size)) lstrcpy(lang,"English");
}

PluginManager::PluginManager(char *plug_dir)
{
  //FIXME: Show message here
  ItemsNumber=0;
  TypesList.ItemsNumber=0;
  TypesList.Items=NULL;
  TypesListItems=NULL;
  Language=NULL;
  UpdateLanguage();
  PluginsData=NULL;
  PluginsCount=0;
  FSF.FarRecursiveSearch(plug_dir,"*.ahp",(FRSUSERFUNC)FindPlugin,FRS_RECUR,this);
  LoadPluginsData();
}

PluginManager::~PluginManager()
{
  free(TypesListItems);
  TypesList.ItemsNumber=0;
  for(int i=0;i<PluginsCount;i++)
  {
    if(PluginsData[i].pExit) PluginsData[i].pExit();
    FreeLibrary(PluginsData[i].hModule);
  }
  delete [] PluginsData;
  PluginsData=NULL;
  PluginsCount=0;
  free(Language);
  Language=NULL;
}

bool PluginManager::UpdateLanguage(void)
{
  char new_language[512];
  GetLanguage(new_language,sizeof(new_language));
  if(Language)
  {
    if(!lstrcmp(Language,new_language)) return false;
    free(Language);
  }
  Language=(char *)malloc(lstrlen(new_language)+1);
  if(Language) lstrcpy(Language, new_language);
  return true;
}

void PluginManager::LoadPluginsData(void)
{
  for(int i=0;i<PluginsCount;i++)
  {
    GetInfoOutData data;
    memset(&data,0,sizeof(data));
    data.StructSize=sizeof(data);
    if(SendMessage(i,AHMSG_GETINFO,NULL,(void *)&data))
    {
      PluginsData[i].Flags=data.Flags;
      if (data.TypesInfo&&data.TypesNumber>0)
      {
        PluginsData[i].TypesNumber=data.TypesNumber;
        PluginsData[i].TypesInfo=data.TypesInfo;
        TypesListItems=(struct FarListItem *)realloc(TypesListItems,sizeof(struct FarListItem)*(ItemsNumber+PluginsData[i].TypesNumber));
        for (int j=0; j<PluginsData[i].TypesNumber; j++)
        {
          memset(&TypesListItems[ItemsNumber+j],0,sizeof(struct FarListItem));
          lstrcpy(TypesListItems[ItemsNumber+j].Text,PluginsData[i].TypesInfo[j].TypeString);
        }
        ItemsNumber+=PluginsData[i].TypesNumber;
      }
      if (data.Flags&AHMSG_CONFIG)
        lstrcpy(PluginsData[i].ConfigString,data.ConfigString);
    }
  }
  TypesList.ItemsNumber=ItemsNumber;
  TypesList.Items=TypesListItems;
}

int WINAPI PluginManager::FindPlugin(const WIN32_FIND_DATA *fdata,const char *filename,PluginManager *self)
{
  (void)fdata;
  self->AddPlugin(filename);
  return TRUE;
}

void PluginManager::AddPlugin(const char *filename)
{
  HMODULE hModule=LoadLibrary(filename);
  if(hModule!=NULL)
  {
    PluginItem CurPlugin;
    CurPlugin.hModule=hModule;
    CurPlugin.Flags=0;
    CurPlugin.TypesNumber=0;
    CurPlugin.TypesInfo=NULL;
    CurPlugin.ConfigString[0]=0;
    CurPlugin.pStart=(PLUGINSTART)GetProcAddress(hModule,"Start");
    CurPlugin.pExit=(PLUGINEXIT)GetProcAddress(hModule,"Exit");
    CurPlugin.pMessage=(PLUGMESSAGE)GetProcAddress(hModule,"Message");
    PluginItem *NewPluginsData=new PluginItem[PluginsCount+1];
    for(int i=0;i<PluginsCount;i++)
      NewPluginsData[i]=PluginsData[i];
    delete [] PluginsData;
    PluginsData=NewPluginsData;
    PluginsData[PluginsCount]=CurPlugin;
    PluginsCount++;
    AHPluginStartupInfo AHInfo;
    AHInfo.StructSize=sizeof(AHInfo);
    lstrcpy(AHInfo.ModuleName,filename);
    lstrcpy(AHInfo.MessageName,filename);
    lstrcpy(AHInfo.MessageName+lstrlen(AHInfo.MessageName)-3,"ahl");
    FSF.sprintf(PluginPluginsRootKey,"%s\\Plugins",PluginRootKey);
    AHInfo.RootKey=PluginPluginsRootKey;
    AHInfo.GetMsg=api_get_message;
    AHInfo.ShowHelpDlgProc=api_show_help_dlg_proc;
    AHInfo.ShowHelpMenu=api_show_help_menu;
    AHInfo.Encode=api_encode;
    AHInfo.Decode=api_decode;
    if(CurPlugin.pStart) CurPlugin.pStart(&Info,&AHInfo);
  }
  return;
}

void PluginManager::SendBroadcastMessage(unsigned long Msg,void *InData,void *OutData)
{
  for(int i=0;i<PluginsCount;i++)
    if(PluginsData[i].pMessage) PluginsData[i].pMessage(Msg,InData,OutData);
}

int PluginManager::SendMessage(int Index,unsigned long Msg,void *InData,void *OutData)
{
  if(Index>=0 && Index<PluginsCount && PluginsData[Index].pMessage)
    return PluginsData[Index].pMessage(Msg,InData,OutData);
  return 0;
}

void PluginManager::ShowConfigMenu(void)
{
  if(UpdateLanguage()) LoadPluginsData();
  int ItemCount=0;
  unsigned long flag=AHMSG_CONFIG;
  for(int i=0;i<PluginsCount;i++)
    if(PluginsData[i].Flags&flag)
      ItemCount++;
  if(ItemCount)
  {
    int *ids=new int[ItemCount];
    FarMenuItem *PlugList=new FarMenuItem[ItemCount];
    for(int i=0,j=0;(i<PluginsCount)&&(j<ItemCount);i++)
    {
      if(PluginsData[i].Flags&flag)
      {
        lstrcpy(PlugList[j].Text,PluginsData[i].ConfigString);
        ids[j]=i;
        j++;
      }
    }
    for(int i=0;i<ItemCount;i++)
    {
      PlugList[i].Selected=0;
      PlugList[i].Checked=0;
      PlugList[i].Separator=0;
    }
    int select=0,selected=0;
    while (select>=0)
    {
      PlugList[selected].Selected=0;
      PlugList[select].Selected=1;
      selected=select;
      int MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,GetMsg(MPluginConfig),NULL,NULL,NULL,NULL,PlugList,ItemCount);
      if(MenuCode>=0)
        SendMessage(ids[MenuCode],flag,NULL,NULL);
      select=MenuCode;
    }
    delete [] PlugList;
    delete [] ids;
  }
}

int PluginManager::ShowHelp(const char *Type, const char *FileName, const char *Keyword, unsigned long Flags, int CallType, const char *Encoding, char *Error)
{
  HelpOutData outdata;
  memset(&outdata,0,sizeof(outdata));
  outdata.StructSize=sizeof(outdata);
  HelpInData data;
  memset(&data,0,sizeof(data));
  data.StructSize=sizeof(data);
  data.FileName = FileName;
  data.Keyword = Keyword;
  data.Encoding = Encoding;
  data.Flags = Flags;
  data.CallType = CallType;
  for (int i=0; i<PluginsCount; i++)
  {
    for (int j=0; j<PluginsData[i].TypesNumber; j++)
    {
      if (!FSF.LStricmp(Type,PluginsData[i].TypesInfo[j].TypeString))
      {
        data.TypeNumber = j;
        int ret = SendMessage(i,AHMSG_SHOWHELP,(void *)&data,(void *)&outdata);
        lstrcpy(Error,*outdata.Error==0?GetMsg(MErrX):outdata.Error);
        return ret;
      }
    }
  }
  return TRUE;
}

int PluginManager::PluginsTotal(void)
{
  return PluginsCount;
}

bool PluginManager::DetectType(const char *Str, int Action, const struct TypeInfo ***data, int *DetectedTypes)
{
  *DetectedTypes=0;
  *data = 0;
  for (int i=0; i<PluginsCount; i++)
  {
    for (int j=0; j<PluginsData[i].TypesNumber; j++)
    {
      if ((Action==0 && FSF.ProcessName(PluginsData[i].TypesInfo[j].Mask,(char *)Str,PN_CMPNAMELIST|((PluginsData[i].TypesInfo[j].HFlags&F_NOSKIPPATH)?0:PN_SKIPPATH)))
          || (Action==1 && !FSF.LStricmp(Str,PluginsData[i].TypesInfo[j].TypeString)))
      {
        *data = (const struct TypeInfo **)realloc(*data,(*DetectedTypes+1)*sizeof(struct TypeInfo *));
        (*data)[(*DetectedTypes)++]=&PluginsData[i].TypesInfo[j];
      }
    }
  }
  return true;
}
