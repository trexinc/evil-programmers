/*
    FARMail plugin for FAR Manager
    Copyright (C) 2002-2004 FARMail Group

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
#include "farmail.hpp"
#include "fmp.hpp"

char PluginPluginsRootKey[100];

PluginManager::PluginManager(char *plug_dir)
{
  //FIXME: Show message here
  Language=NULL;
  UpdateLanguage();
  PluginsData=NULL;
  PluginsCount=0;
  FSF.FarRecursiveSearch(plug_dir,"*.fmp",(FRSUSERFUNC)FindPlugin,FRS_RECUR,this);
  LoadPluginsData();
}

PluginManager::~PluginManager()
{
  for(int i=0;i<PluginsCount;i++)
  {
    if(PluginsData[i].pExit) PluginsData[i].pExit();
    FreeLibrary(PluginsData[i].hModule);
  }
  delete [] PluginsData;
  PluginsData=NULL;
  PluginsCount=0;
  z_free(Language);
  Language=NULL;
}

bool PluginManager::UpdateLanguage(void)
{
  char new_language[512];
  DWORD res=GetEnvironmentVariable("FARLANG",new_language,sizeof(new_language));
  if(!(res&&res<sizeof(new_language))) lstrcpy(new_language,"English");
  if(Language)
  {
    if(!lstrcmp(Language,new_language)) return false;
    z_free(Language);
  }
  Language=(char *)z_malloc(lstrlen(new_language)+1);
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
    if(SendMessage(i,FMMSG_GETINFO,NULL,(void *)&data))
    {
      PluginsData[i].Flags=data.Flags;
      lstrcpy(PluginsData[i].MenuString,data.MenuString);
      lstrcpy(PluginsData[i].HotkeyID,data.HotkeyID);
      PluginsData[i].Hotkey=' ';
      if(PluginsData[i].HotkeyID[0])
      {
        int value;
        if(GetRegKey2(HKEY_CURRENT_USER,(char *)PluginHotkeyKey,NULLSTR,(char *)PluginsData[i].HotkeyID,&value,' ')) PluginsData[i].Hotkey=value;
      }
    }
  }
}

int WINAPI PluginManager::FindPlugin(const WIN32_FIND_DATA *fdata,const char *filename,PluginManager *self)
{
  (void)fdata;
  self->AddPlugin(filename);
  return TRUE;
}

void PluginManager::AddPlugin(const char *filename)
{
#ifdef TDEBUG
try
{
#endif
  HMODULE hModule=LoadLibrary(filename);
  if(hModule!=NULL)
  {
    PluginItem CurPlugin;
    CurPlugin.hModule=hModule;
    CurPlugin.Flags=0;
    CurPlugin.MenuString[0]=0;
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
    MailPluginStartupInfo MailInfo;
    MailInfo.StructSize=sizeof(MailInfo);
    lstrcpy(MailInfo.ModuleName,filename);
    {
      lstrcpy(MailInfo.MessageName,filename);
      char *name=FSF.PointToName(MailInfo.MessageName);
      lstrcpy(name,"*.fml");
      WIN32_FIND_DATA fd;
      HANDLE fh=FindFirstFile(MailInfo.MessageName,&fd);
      if (fh)
      {
        lstrcpy(name,fd.cFileName);
        FindClose(fh);
      }
      else
      {
        lstrcpy(name,filename+(name-MailInfo.MessageName));
        lstrcpy(name+lstrlen(name)-3,"fml");
      }
    }
    FSF.sprintf(PluginPluginsRootKey,"%s\\Plugins",PluginRootKey);
    MailInfo.RootKey=PluginPluginsRootKey;
    MailInfo.GetMsg=api_get_message;
    MailInfo.GetHeaderField=api_get_header_field;
    MailInfo.GetFreeNumber=api_get_free_number;
    MailInfo.InitBoundary=api_init_boundary;
    MailInfo.ShowHelpDlgProc=api_show_help_dlg_proc;
    MailInfo.ShowHelpMenu=api_show_help_menu;
    MailInfo.GetValue=api_get_value;
    MailInfo.GetString=api_get_string;
    MailInfo.AddressBook=api_address_book;
    MailInfo.EncodeHeader=api_encode_header;
    if(CurPlugin.pStart) CurPlugin.pStart(&_Info,&MailInfo);
  }
  return;
#ifdef TDEBUG
}
#ifdef __BORLANDC__
__except(1)
#else
catch(...)
#endif
{
  SayException( "AddPlugin" );
  return;
}
#endif
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

void PluginManager::SetHotkey(int index)
{
  if(!PluginsData[index].HotkeyID[0]) return;
  /*
    0000000000111111111122222222223333333333444444444455555555556666
    0123456789012345678901234567890123456789012345678901234567890123
  00                                                                00
  01   ษออออออออออออออออ Assign plugin hot key อออออออออออออออออป   01
  02   บ Enter hot key (letter or digit)                        บ   02
  03   บ B                                                      บ   03
  04   ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ   04
  05                                                                05
    0000000000111111111122222222223333333333444444444455555555556666
    0123456789012345678901234567890123456789012345678901234567890123
  */
  struct InitDialogItem InitItems[]=
  {
    {DI_DOUBLEBOX,3,1,60,4,0,0,0,0,(char*)MesPluginHotkeyDialogTitle},
    {DI_TEXT,5,2,0,0,0,0,0,0,(char*)MesPluginHotkeyDialogLabel},
    {DI_FIXEDIT,5,3,5,3,1,0,0,0,NULLSTR}
  };
  struct FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));
  DialogItems[2].Data[0]=PluginsData[index].Hotkey;
  DialogItems[2].Data[1]=0;
  if(_Info.Dialog(_Info.ModuleNumber,-1,-1,64,6,NULL,DialogItems,sizeofa(DialogItems))==2)
  {
    PluginsData[index].Hotkey=DialogItems[2].Data[0];
    if(!PluginsData[index].Hotkey) PluginsData[index].Hotkey=' ';
    SetRegKey2(HKEY_CURRENT_USER,(char *)PluginHotkeyKey,NULLSTR,(char *)PluginsData[index].HotkeyID,PluginsData[index].Hotkey);
  }
}

int PluginManager::ShowMenu(unsigned long flag, int select, int *selected, void *InData, void *OutData)
{
  if(UpdateLanguage()) LoadPluginsData();
#ifdef TDEBUG
try
{
#endif
  int ItemCount=0,res=0,MenuCode=-1;
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
        FSF.sprintf(PlugList[j].Text,"%c %s",PluginsData[i].Hotkey,PluginsData[i].MenuString);
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
    PlugList[select>=0&&select<ItemCount?select:0].Selected=1;
    BOOL fShowMenu=TRUE;
    switch(flag)
    {
      case FMMSG_ADDRESSBOOK:
      case FMMSG_FILTER:
        if(ItemCount==1)
        {
          fShowMenu=FALSE;
          MenuCode=0;
        }
        break;

      default:
        break;
    }
    if(fShowMenu)
    {
      int BreakCode;
      int BreakKeys[2]={VK_F4,0};
      while(true)
      {
        MenuCode=_Info.Menu(_Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,GetMsg(MesPluginEditor),GetMsg(MesPluginHotkey),NULL,BreakKeys,&BreakCode,PlugList,ItemCount);
        if(MenuCode>=0&&BreakCode==0)
        {
          SetHotkey(ids[MenuCode]);
          for(int i=0;i<ItemCount;i++)
            PlugList[i].Selected=0;
          PlugList[MenuCode].Selected=1;
          FSF.sprintf(PlugList[MenuCode].Text,"%c %s",PluginsData[ids[MenuCode]].Hotkey,PluginsData[ids[MenuCode]].MenuString);
        }
        else break;
      }
    }
    if(MenuCode>=0)
      res=SendMessage(ids[MenuCode],flag,InData,OutData);
    delete [] PlugList;
    delete [] ids;
  }
  if (selected)
    *selected=MenuCode;
  return res;
#ifdef TDEBUG
}
#ifdef __BORLANDC__
__except(1)
#else
catch(...)
#endif
{
  SayException( "ShowMenu" );
  return -1;
}
#endif
}

int PluginManager::ShowEditorMenu(void)
{
  return ShowMenu(FMMSG_MENU);
}

int PluginManager::ShowConfigMenu(void)
{
  int res=1;
  int selected=0;
  while (res)
  {
    res=ShowMenu(FMMSG_CONFIG,selected,&selected);
  }
  return res;
}

int PluginManager::ShowAddressBookMenu(char *str)
{
  AddressOutData data={sizeof(AddressOutData),str};
  return ShowMenu(FMMSG_ADDRESSBOOK,0,NULL,NULL,&data);
}

int PluginManager::ShowFilterMenu(HANDLE hPlugin,int index)
{
  MsgPanelInData data={sizeof(MsgPanelInData),hPlugin,index};
  return ShowMenu(FMMSG_FILTER,0,NULL,&data);
}
