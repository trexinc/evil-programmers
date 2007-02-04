/*
    DialogManager plugin for FAR Manager
    Copyright (C) 2003 Vadim Yegorov
    Copyright (C) 2004 Vadim Yegorov and Alex Yaroslavsky

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
#include "plugin.hpp"
#include "dialogM.hpp"

static struct PluginItem *PluginsData=NULL;
static int PluginsCount=0;

//internal plugin manager data
static char *Language;
static int SendMessage(int Index,unsigned long Msg,void *InData,void *OutData);

static void WINAPI api_get_message(const char *file_name,int index,char *message)
{
  static char language[512];
  static char buffer[32*1024-1];
  *message=0;
  DWORD res=GetEnvironmentVariable("FARLANG",language,sizeof(language));
  if(!(res&&res<sizeof(language))) strcpy(language,"English");
  for(int i=0;i<2;i++)
  {
    if(GetPrivateProfileSection(language,buffer,sizeof(buffer),file_name))
    {
      char *ptr=buffer;
      while(index>=0&&strlen(ptr))
      {
        if(!index) strcpy(message,ptr);
        index--;
        ptr=ptr+strlen(ptr)+1;
      }
      break;
    }
    else if(!i) strcpy(language,"English");
  }
}

static bool UpdateLanguage(void)
{
  char new_language[512];
  DWORD res=GetEnvironmentVariable("FARLANG",new_language,sizeof(new_language));
  if(!(res&&res<sizeof(new_language))) strcpy(new_language,"English");
  if(Language)
  {
    if(!strcmp(Language,new_language)) return false;
    HeapFree(GetProcessHeap(),0,Language);
  }
  Language=(char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,strlen(new_language)+1);
  if(Language) strcpy(Language,new_language);
  return true;
}

static int WINAPI FindPlugin(const WIN32_FIND_DATA *fdata,const char *filename,void *Param)
{
  (void)fdata; (void)Param;
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
    bool Allocated=true;
    if(PluginsData)
    {
      PluginItem *NewPluginsData=(PluginItem *)HeapReAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,PluginsData,sizeof(PluginItem)*(PluginsCount+1));
      if(NewPluginsData)
        PluginsData=NewPluginsData;
      else
        Allocated=false;
    }
    else
    {
      PluginsData=(PluginItem *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(PluginItem)*(PluginsCount+1));
      if(!PluginsData) Allocated=false;
    }
    if(Allocated)
    {
      PluginsData[PluginsCount]=CurPlugin;
      PluginsCount++;
      DialogPluginStartupInfo DialogInfo;
      DialogInfo.StructSize=sizeof(DialogInfo);
      strcpy(DialogInfo.ModuleName,filename);
      strcpy(DialogInfo.MessageName,filename);
      strcpy(DialogInfo.MessageName+strlen(DialogInfo.MessageName)-3,"msg");
      DialogInfo.RootKey=PluginPluginsRootKey;
      DialogInfo.GetMsg=api_get_message;
      if(CurPlugin.pStart) CurPlugin.pStart(&Info,&DialogInfo);
    }
    else
      FreeLibrary(hModule);
  }
  return TRUE;
}

void LoadPluginsData(void)
{
  for(int i=0;i<PluginsCount;i++)
  {
    GetInfoOutData data;
    memset(&data,0,sizeof(data));
    data.StructSize=sizeof(data);
    if(SendMessage(i,FMMSG_GETINFO,NULL,(void *)&data))
    {
      PluginsData[i].Flags=data.Flags;
      strcpy(PluginsData[i].MenuString,data.MenuString);
      strcpy(PluginsData[i].HotkeyID,data.HotkeyID);
      PluginsData[i].Hotkey=' ';
      if(PluginsData[i].HotkeyID[0])
      {
        HKEY hKey;
        if((RegOpenKeyEx(HKEY_CURRENT_USER,PluginHotkeyKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
        {
          DWORD Value,Type,DataSize=sizeof(Value);
          if(RegQueryValueEx(hKey,PluginsData[i].HotkeyID,0,&Type,(LPBYTE)&Value,&DataSize)==ERROR_SUCCESS) PluginsData[i].Hotkey=Value;
          RegCloseKey(hKey);
        }
      }
    }
  }
}

void LoadPlugs(void)
{
  Language=NULL;
  UpdateLanguage();
  PluginsData=NULL;
  PluginsCount=0;
  {
    char plug_dir[MAX_PATH];
    strcpy(plug_dir,Info.ModuleName);
    *(FSF.PointToName(plug_dir))=0;
    strcat(plug_dir,"Formats\\");
    FSF.FarRecursiveSearch(plug_dir,"*.fmt",(FRSUSERFUNC)FindPlugin,FRS_RECUR,NULL);
  }
  LoadPluginsData();
}

void UnloadPlugs(void)
{
  for(int i=0;i<PluginsCount;i++)
  {
    if(PluginsData[i].pExit) PluginsData[i].pExit();
    FreeLibrary(PluginsData[i].hModule);
  }
  HeapFree(GetProcessHeap(),0,PluginsData);
  PluginsData=NULL;
  PluginsCount=0;
  HeapFree(GetProcessHeap(),0,Language);
  Language=NULL;
}

static int SendMessage(int Index,unsigned long Msg,void *InData,void *OutData)
{
  if(Index>=0&&Index<PluginsCount&&PluginsData[Index].pMessage)
    return PluginsData[Index].pMessage(Msg,InData,OutData);
  return 0;
}

void SendBroadcastMessage(unsigned long Msg,void *InData,void *OutData)
{
  for(int i=0;i<PluginsCount;i++)
    if(PluginsData[i].pMessage&&(PluginsData[i].Flags&Msg)) PluginsData[i].pMessage(Msg,InData,OutData);
}

static void SetHotkey(int index)
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
  FarDialogItem DialogItems[3];
  memset(DialogItems,0,sizeof(DialogItems));
  DialogItems[0].Type=DI_DOUBLEBOX;
  DialogItems[0].X1=3; DialogItems[0].Y1=1; DialogItems[0].X2=60; DialogItems[0].Y2=4;
  strcpy(DialogItems[0].Data,Info.GetMsg(Info.ModuleNumber,mHotkeyDialogTitle));

  DialogItems[1].Type=DI_TEXT;
  DialogItems[1].X1=5; DialogItems[1].Y1=2;
  strcpy(DialogItems[1].Data,Info.GetMsg(Info.ModuleNumber,mHotkeyDialogLabel));

  DialogItems[2].Type=DI_FIXEDIT; DialogItems[2].Focus=1;
  DialogItems[2].X1=5; DialogItems[2].Y1=3; DialogItems[2].X2=5; DialogItems[2].Y2=3;

  DialogItems[2].Data[0]=PluginsData[index].Hotkey;
  DialogItems[2].Data[1]=0;
  if(Info.Dialog(Info.ModuleNumber,-1,-1,64,6,NULL,DialogItems,sizeofa(DialogItems))==2)
  {
    PluginsData[index].Hotkey=DialogItems[2].Data[0];
    if(!PluginsData[index].Hotkey) PluginsData[index].Hotkey=' ';
    HKEY hKey; DWORD Disposition;
    if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginHotkeyKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
    {
      RegSetValueEx(hKey,PluginsData[index].HotkeyID,0,REG_DWORD,(LPBYTE)&PluginsData[index].Hotkey,sizeof(PluginsData[index].Hotkey));
      RegCloseKey(hKey);
    }
  }
}

int PluginCount(unsigned long flag)
{
  int ItemCount=0;
  for(int i=0;i<PluginsCount;i++)
  {
    if(PluginsData[i].Flags&flag)
      ItemCount++;
  }
  return ItemCount;
}

int ShowMenu(unsigned long flag,void *InData,void *OutData)
{
  if(UpdateLanguage()) LoadPluginsData();
  int ItemCount=PluginCount(flag),res=0,MenuCode=-1;
  if(ItemCount)
  {
    int *ids=(int *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(int)*ItemCount);
    FarMenuItem *PlugList=(FarMenuItem *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(FarMenuItem)*ItemCount);
    if(ids&&PlugList)
    {
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
      PlugList[0].Selected=1;
      int BreakCode;
      int BreakKeys[2]={VK_F4,0};
      while(true)
      {
        MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,Info.GetMsg(Info.ModuleNumber,mEditor),Info.GetMsg(Info.ModuleNumber,mHotkey),NULL,BreakKeys,&BreakCode,PlugList,ItemCount);
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
      if(MenuCode>=0) res=SendMessage(ids[MenuCode],flag,InData,OutData);
    }
    if(PlugList) HeapFree(GetProcessHeap(),0,PlugList);
    if(ids) HeapFree(GetProcessHeap(),0,ids);
  }
  return res;
}
