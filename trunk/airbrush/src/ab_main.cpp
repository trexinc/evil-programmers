/*
    ab_main.cpp
    Copyright (C) 2000-2008 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <tchar.h>
#include "plugin.hpp"
#include "farcolor.hpp"
#include "memory.h"
#include "ab_main.h"
#include "abplugin.h"
#include "abversion.h"
#include <initguid.h>
#include "guid.h"
// {2C20419A-CBA1-4a15-AB4C-AFF61510DA0C}
DEFINE_GUID(MenuGuid, 0x2c20419a, 0xcba1, 0x4a15, 0xab, 0x4c, 0xaf, 0xf6, 0x15, 0x10, 0xda, 0xc);
// {91737241-105F-460a-A5A0-0EC56C235D27}
DEFINE_GUID(Config1Guid, 0x91737241, 0x105f, 0x460a, 0xa5, 0xa0, 0xe, 0xc5, 0x6c, 0x23, 0x5d, 0x27);
// {A338563B-914D-4cbd-97C1-DBDDAE78DC6F}
DEFINE_GUID(Config3Guid, 0xa338563b, 0x914d, 0x4cbd, 0x97, 0xc1, 0xdb, 0xdd, 0xae, 0x78, 0xdc, 0x6f);
// {9F54C244-3CD2-4517-9A47-F82EB8C6C495}
DEFINE_GUID(Config5Guid, 0x9f54c244, 0x3cd2, 0x4517, 0x9a, 0x47, 0xf8, 0x2e, 0xb8, 0xc6, 0xc4, 0x95);

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
TCHAR PluginRootKey[80];
TCHAR PluginMaskKey[80];
TCHAR PluginColorKey[80];
TCHAR PluginStartKey[80];
HANDLE Mutex=NULL;
int cursor_row=-1,cursor_col=-1;

struct Options Opt={true,50000,true};
bool fatal;

#define GetCheck(i) (int)Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
#define GetDataPtr(i) ((const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))

void InitDialogItems(const InitDialogItem *Init,FarDialogItem *Item,int ItemsNumber)
{
  for (int i=0;i<ItemsNumber;i++)
  {
    Item[i].Type=Init[i].Type;
    Item[i].X1=Init[i].X1;
    Item[i].Y1=Init[i].Y1;
    Item[i].X2=Init[i].X2;
    Item[i].Y2=Init[i].Y2;
    Item[i].Selected=Init[i].Selected;
    Item[i].Flags=Init[i].Flags;
    Item[i].MaxLen=0;
    if((unsigned)Init[i].Data<2000)
      Item[i].PtrData=GetMsg((unsigned int)(DWORD_PTR)Init[i].Data);
    else
      Item[i].PtrData=Init[i].Data;
    Item[i].Mask=NULL;
    Item[i].History=NULL;
  }
}

static TCHAR hotkeys[]=_T("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

const TCHAR* GetMsg(int MsgId)
{
  return(Info.GetMsg(&MainGuid,MsgId));
}

TCHAR* GetCommaWord(TCHAR* Src,TCHAR* Word)
{
  int WordPos,SkipBrackets;
  if (*Src==0)
    return(NULL);
  SkipBrackets=false;
  for (WordPos=0;*Src!=0;Src++,WordPos++)
  {
    if (*Src=='['&&_tcschr(Src+1,']')!=NULL)
      SkipBrackets=true;
    if (*Src==']')
      SkipBrackets=false;
    if (*Src==','&&!SkipBrackets)
    {
      Word[WordPos]=0;
      Src++;
      while (_istspace(*Src))
        Src++;
      return(Src);
    }
    else
      Word[WordPos]=*Src;
  }
  Word[WordPos]=0;
  return(Src);
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=MAKEFARVERSION(VER_MAJOR,VER_MINOR,VER_BUILD);
  Info->Guid=MainGuid;
  Info->Title=L"AirBrush";
  Info->Description=L"Syntax highlighting in editor";
  Info->Author=L"Vadim Yegorov";
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>(int)sizeof(::Info))?sizeof(::Info):Info->StructSize);

  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;
  Mutex=CreateMutex(NULL,FALSE,NULL);
  //read settings
  lstrcpy(PluginRootKey,Info->RootKey);
  lstrcat(PluginRootKey,_T("\\AirBrush"));
  lstrcpy(PluginMaskKey,PluginRootKey);
  lstrcat(PluginMaskKey,_T("\\masks"));
  lstrcpy(PluginColorKey,PluginRootKey);
  lstrcat(PluginColorKey,_T("\\colors"));
  lstrcpy(PluginStartKey,PluginRootKey);
  lstrcat(PluginStartKey,_T("\\starts"));

  HKEY hKey;
  DWORD Type;
  DWORD DataSize=0;
  if((RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
  {
    DataSize=sizeof(Opt.Active);
    RegQueryValueEx(hKey,_T("Active"),0,&Type,(LPBYTE)&Opt.Active,&DataSize);
    DataSize=sizeof(Opt.MaxLines);
    RegQueryValueEx(hKey,_T("MaxLines"),0,&Type,(LPBYTE)&Opt.MaxLines,&DataSize);
    DataSize=sizeof(Opt.ColorizeAll);
    RegQueryValueEx(hKey,_T("ColorizeAll"),0,&Type,(LPBYTE)&Opt.ColorizeAll,&DataSize);
    RegCloseKey(hKey);
    if((Opt.MaxLines<0)||(Opt.MaxLines>9999999)) Opt.MaxLines=50000;
  }
  LoadPlugs(Info->ModuleName);
  OnLoad();
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
  static const TCHAR* PluginMenuStrings;
  memset(Info,0,sizeof(*Info));
  Info->Flags = PF_EDITOR | PF_DISABLEPANELS;
  Info->StructSize = sizeof(*Info);
  Info->PluginConfig.Guids = &MenuGuid;
  Info->PluginMenu.Guids = &MenuGuid;
  Info->PluginConfig.Count = 1;
  Info->PluginMenu.Count = 1;
  PluginMenuStrings = GetMsg(mName);
  Info->PluginConfig.Strings = &PluginMenuStrings;
  Info->PluginMenu.Strings = &PluginMenuStrings;
}

void WINAPI ExitFARW(void)
{
  OnExit();
  UnloadPlugs();
  CloseHandle(Mutex);
}

HANDLE WINAPI OpenPluginW(int OpenFrom,const GUID* Guid,INT_PTR Item)
{
  if((OpenFrom==OPEN_EDITOR)&&(PluginsCount))
  {
    int Count=0;
    for(int i=0;i<PluginsCount;i++)
      if(PluginsData[i].Params&PAR_SHOW_IN_LIST)
        Count++;

    int *ids=(int *)malloc((Count+1)*sizeof(int));
    if(ids)
    {
      FarMenuItem *SyntaxTypes=NULL;
      size_t size=sizeof(FarMenuItem)*(Count+1)+(Count+1)*128*sizeof(TCHAR);
      SyntaxTypes=(FarMenuItem *)malloc(size);
      if(SyntaxTypes)
      {
        TCHAR* data=(TCHAR*)(((char*)SyntaxTypes)+sizeof(FarMenuItem)*(Count+1));
        TCHAR* text=data;
        SyntaxTypes[0].Text=text;
        EditorInfo ei;
        PEditFile curfile;
        int index=-1;
        Info.EditorControl(-1,ECTL_GETINFO,0,(INT_PTR)&ei);
        curfile=ef_getfile(ei.EditorID);
        if(curfile) index=curfile->type+1;
        FSF.sprintf(text,_T("%c. %s"),hotkeys[0],GetMsg(mDefault));
        if(!index) SyntaxTypes[0].Flags|='*'|MIF_CHECKED;
        ids[0]=-1;
        index--;
        for(int i=0,j=1;i<PluginsCount;i++)
        {
          if(PluginsData[i].Params&PAR_SHOW_IN_LIST)
          {
            text=data+j*128;
            SyntaxTypes[j].Text=text;
            if(j<(int)(lstrlen(hotkeys)-1))
              FSF.sprintf(text,_T("%c. %s"),hotkeys[j],PluginsData[i].Name);
            else
              FSF.sprintf(text,_T("%c. %s"),_T(' '),PluginsData[i].Name);
            if(index==i) SyntaxTypes[j].Flags|='*'|MIF_CHECKED;
            ids[j]=i;
            j++;
          }
        }
        int MenuCode=0,BreakKeys[]={VK_F4,0},BreakCode;
        while(true)
        {
          for(int i=0;i<=Count;i++)
            SyntaxTypes[i].Flags&=~MIF_SELECTED;
          SyntaxTypes[MenuCode].Flags|=MIF_SELECTED;
          BreakCode=-1;
          MenuCode=Info.Menu(&MainGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,_T(""),NULL,NULL,BreakKeys,&BreakCode,SyntaxTypes,Count+1);
          if(BreakCode==-1) break;
          if((BreakCode==0)&&(MenuCode>0))
            if(PluginsData[ids[MenuCode]].pGetParams)
              PluginsData[ids[MenuCode]].pGetParams(PluginsData[ids[MenuCode]].Index,PAR_CONFIGURE1,NULL);
        }
        if(MenuCode>-1)
        {
          EditorInfo ei;
          Info.EditorControl(-1,ECTL_GETINFO,0,(INT_PTR)&ei);
          ef_deletefile(ei.EditorID);
          loadfile(ei.EditorID,ids[MenuCode]);
        }
        free(SyntaxTypes);
      }
      free(ids);
    }
  }
  return INVALID_HANDLE_VALUE;
}

INT_PTR WINAPI Config1DialogProc(HANDLE hDlg,int Msg,int Param1,INT_PTR Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

int WINAPI ConfigureW(const GUID* Guid)
{
  FarMenuItem MenuItems[4];
  memset(MenuItems,0,sizeof(MenuItems));
  TCHAR data[4*128];
  memset(data,0,sizeof(data));
  int Msgs[]={mConfigMenu1,mConfigMenu2,mConfigMenu3,mConfigMenu4};

  for(unsigned int i=0;i<sizeof(MenuItems)/sizeof(MenuItems[0]);i++)
  {
    TCHAR* text=data+i*128;
    MenuItems[i].Text=text;
    FSF.sprintf(text,_T("%s"),GetMsg(Msgs[i])); // Text in menu
  };
  int MenuCode=0;
  while(true)
  {
    for(unsigned int i=0;i<sizeof(MenuItems)/sizeof(MenuItems[0]);i++)
      MenuItems[i].Flags&=~MIF_SELECTED;
    MenuItems[MenuCode].Flags|=MIF_SELECTED;
    // Show menu
    MenuCode=Info.Menu(&MainGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,
                           NULL,NULL,_T("Config"),NULL,NULL,
                           MenuItems,sizeof(MenuItems)/sizeof(MenuItems[0]));
    if(MenuCode==-1)
      break;
    else if(MenuCode==0)
    {
      /*
        0000000000111111111122222222223333333333444444444455555555556666666666777777
        0123456789012345678901234567890123456789012345678901234567890123456789012345
      00                                                                            00
      01   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»   01
      02   º [x] Turn on plugin module                                          º   02
      03   º [x] Colorize all files                                             º   03
      04   º Don't colorize files with line count greater then:         ÛÛÛÛÛÛÛ º   04
      05   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   05
      06   º                 [ Save ]              [ Cancel ]                   º   06
      07   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   07
      08                                                                            08
        0000000000111111111122222222223333333333444444444455555555556666666666777777
        0123456789012345678901234567890123456789012345678901234567890123456789012345
      */
      static struct InitDialogItem InitItems[]={
      /*0*/  {DI_DOUBLEBOX,3 ,1,72,7,0,0,(const TCHAR*)mName},
      /*1*/  {DI_CHECKBOX ,5 ,2,0 ,0,0,DIF_FOCUS,(const TCHAR*)mConfigDialogActive},
      /*2*/  {DI_CHECKBOX ,5 ,3,0 ,0,0,0,(const TCHAR*)mConfigDialogColorizeAll},
      /*3*/  {DI_TEXT     ,5 ,4,0 ,0,0,0,(const TCHAR*)mConfigDialogMaxLines},
      /*4*/  {DI_FIXEDIT  ,64,4,70,0,0,DIF_MASKEDIT,_T("")},
      /*5*/  {DI_TEXT     ,-1,5,0 ,0,0,DIF_SEPARATOR,_T("")},
      /*6*/  {DI_BUTTON   ,0 ,6,0 ,0,0,DIF_CENTERGROUP|DIF_DEFAULTBUTTON,(const TCHAR*)mConfigSave},
      /*7*/  {DI_BUTTON   ,0 ,6,0 ,0,0,DIF_CENTERGROUP,(const TCHAR*)mConfigCancel}
      };
      struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
      InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
      DialogItems[1].Selected=Opt.Active;
      DialogItems[2].Selected=Opt.ColorizeAll;
      DialogItems[4].Mask=_T("######9");

      TCHAR lines[32];
      DialogItems[4].PtrData=lines;
      FSF.sprintf(lines,_T("%d"),Opt.MaxLines);
      int DlgCode=-1;
      HANDLE hDlg=Info.DialogInit(&MainGuid,&Config1Guid,-1,-1,76,9,_T("Config1"),DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,Config1DialogProc,0);
      if(hDlg!=INVALID_HANDLE_VALUE) DlgCode=Info.DialogRun(hDlg);
      if(DlgCode==6)
      {
        Opt.Active=GetCheck(1);
        Opt.ColorizeAll=GetCheck(2);
        Opt.MaxLines=FSF.atoi(GetDataPtr(4)); if((Opt.MaxLines<0)||(Opt.MaxLines>9999999)) Opt.MaxLines=50000;
        HKEY hKey;
        DWORD Disposition;
        if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
        {
          RegSetValueEx(hKey,_T("Active"),0,REG_BINARY,(LPBYTE)&Opt.Active,sizeof(Opt.Active));
          RegSetValueEx(hKey,_T("MaxLines"),0,REG_DWORD,(LPBYTE)&Opt.MaxLines,sizeof(Opt.MaxLines));
          RegSetValueEx(hKey,_T("ColorizeAll"),0,REG_DWORD,(LPBYTE)&Opt.ColorizeAll,sizeof(Opt.ColorizeAll));
          RegCloseKey(hKey);
        }
      }
      if(hDlg!=INVALID_HANDLE_VALUE) Info.DialogFree(hDlg);
    }
    else if(MenuCode==1)
    {
      int ConfCount=0;
      for(int i=0;i<PluginsCount;i++)
        if(PluginsData[i].Params&PAR_MASK_STORE)
          ConfCount++;
      if(ConfCount)
      {
        int *ids=(int *)malloc(ConfCount*sizeof(int));
        if(ids)
        {
          FarMenuItem *SyntaxTypes=NULL;
          size_t size=sizeof(FarMenuItem)*ConfCount+ConfCount*128*sizeof(TCHAR);
          SyntaxTypes=(FarMenuItem *)malloc(size);
          if(SyntaxTypes)
          {
            TCHAR* data=(TCHAR*)(((char*)SyntaxTypes)+sizeof(FarMenuItem)*ConfCount);
            for(int i=0,j=0;(i<PluginsCount)&&(j<ConfCount);i++)
            {
              if(PluginsData[i].Params&PAR_MASK_STORE)
              {
                TCHAR* text=data+j*128;
                SyntaxTypes[j].Text=text;
                if(j<(int)lstrlen(hotkeys))
                  FSF.sprintf(text,_T("%c. %s"),hotkeys[j],PluginsData[i].Name);
                else
                  FSF.sprintf(text,_T("%c. %s"),' ',PluginsData[i].Name);
                ids[j]=i;
                j++;
              }
            }
            int MenuCode=0;
            while(true)
            {
              for(int i=0;i<ConfCount;i++)
                SyntaxTypes[i].Flags&=~MIF_SELECTED;
              SyntaxTypes[MenuCode].Flags|=MIF_SELECTED;

              MenuCode=Info.Menu(&MainGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,_T(""),NULL,_T("Config2"),NULL,NULL,SyntaxTypes,ConfCount);
              if(MenuCode==-1) break;
              TCHAR mask[2048];
              lstrcpy(mask,_T(""));
              if(PluginsData[ids[MenuCode]].Mask)
                lstrcpy(mask,PluginsData[ids[MenuCode]].Mask);
              static struct InitDialogItem InitItems[]={
              /*0*/  {DI_DOUBLEBOX,3,1,72,5,0,0,_T("")},
              /*1*/  {DI_EDIT,5,2,70,0,0,DIF_FOCUS,_T("")},
              /*2*/  {DI_TEXT,-1,3,0,0,0,DIF_SEPARATOR,_T("")},
              /*3*/  {DI_BUTTON,0,4,0,0,0,DIF_CENTERGROUP|DIF_DEFAULTBUTTON,(const TCHAR *)mConfigSave},
              /*4*/  {DI_BUTTON,0,4,0,0,0,DIF_CENTERGROUP,(const TCHAR *)mConfigCancel}
              };
              struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
              InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
              DialogItems[1].MaxLen=sizeof(mask);
              DialogItems[1].PtrData=mask;
              DialogItems[0].PtrData=PluginsData[ids[MenuCode]].Name;
              int DlgCode=-1;
              HANDLE hDlg=Info.DialogInit(&MainGuid,&Config3Guid,-1,-1,76,7,_T("Config3"),DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,Config1DialogProc,0);
              if(hDlg!=INVALID_HANDLE_VALUE) DlgCode=Info.DialogRun(hDlg);
              if(DlgCode==3)
              {
                if(PluginsData[ids[MenuCode]].Params&PAR_MASK_STORE)
                {
                  HKEY hKey; DWORD Disposition;
                  if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginMaskKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
                  {
                    RegSetValueEx(hKey,PluginsData[ids[MenuCode]].Name,0,REG_SZ,(LPBYTE)GetDataPtr(1),(lstrlen(GetDataPtr(1))+1)*sizeof(TCHAR));
                    RegCloseKey(hKey);
                  }
                }
                free(PluginsData[ids[MenuCode]].Mask);
                PluginsData[ids[MenuCode]].Mask=(TCHAR*)malloc((lstrlen(GetDataPtr(1))+1)*sizeof(TCHAR));
                if(PluginsData[ids[MenuCode]].Mask)
                  lstrcpy(PluginsData[ids[MenuCode]].Mask,GetDataPtr(1));
              }
              if(hDlg!=INVALID_HANDLE_VALUE) Info.DialogFree(hDlg);
            }
            free(SyntaxTypes);
          }
          free(ids);
        }
      }
    }
    else if(MenuCode==2)
    {
      int ConfCount=0;
      for(int i=0;i<PluginsCount;i++)
        if(PluginsData[i].Params&PAR_FILESTART_STORE)
          ConfCount++;
      if(ConfCount)
      {
        int *ids=(int *)malloc(ConfCount*sizeof(int));
        if(ids)
        {
          FarMenuItem *SyntaxTypes=NULL;
          size_t size=sizeof(FarMenuItem)*ConfCount+ConfCount*128*sizeof(TCHAR);
          SyntaxTypes=(FarMenuItem *)malloc(size);
          if(SyntaxTypes)
          {
            TCHAR* data=(TCHAR*)(((char*)SyntaxTypes)+sizeof(FarMenuItem)*ConfCount);
            for(int i=0,j=0;(i<PluginsCount)&&(j<ConfCount);i++)
            {
              if(PluginsData[i].Params&PAR_FILESTART_STORE)
              {
                TCHAR* text=data+j*128;
                SyntaxTypes[j].Text=text;
                if(j<(int)lstrlen(hotkeys))
                  FSF.sprintf(text,_T("%c. %s"),hotkeys[j],PluginsData[i].Name);
                else
                  FSF.sprintf(text,_T("%c. %s"),' ',PluginsData[i].Name);
                ids[j]=i;
                j++;
              }
            }
            int MenuCode=0;
            while(true)
            {
              for(int i=0;i<ConfCount;i++)
                SyntaxTypes[i].Flags&=~MIF_SELECTED;
              SyntaxTypes[MenuCode].Flags|=MIF_SELECTED;

              MenuCode=Info.Menu(&MainGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,_T(""),NULL,_T("Config4"),NULL,NULL,SyntaxTypes,ConfCount);
              if(MenuCode==-1) break;
              TCHAR start[2048];
              lstrcpy(start,_T(""));
              if(PluginsData[ids[MenuCode]].Start)
                lstrcpy(start,PluginsData[ids[MenuCode]].Start);
              static struct InitDialogItem InitItems[]={
              /*0*/  {DI_DOUBLEBOX,3,1,72,5,0,0,_T("")},
              /*1*/  {DI_EDIT,5,2,70,0,0,DIF_FOCUS,_T("")},
              /*2*/  {DI_TEXT,-1,3,0,0,0,DIF_SEPARATOR,_T("")},
              /*3*/  {DI_BUTTON,0,4,0,0,0,DIF_CENTERGROUP|DIF_DEFAULTBUTTON,(const TCHAR*)mConfigSave},
              /*4*/  {DI_BUTTON,0,4,0,0,0,DIF_CENTERGROUP,(const TCHAR*)mConfigCancel}
              };
              struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
              InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
              DialogItems[1].MaxLen=sizeof(start);
              DialogItems[1].PtrData=start;
              DialogItems[0].PtrData=PluginsData[ids[MenuCode]].Name;
              int DlgCode=-1;
              HANDLE hDlg=Info.DialogInit(&MainGuid,&Config5Guid,-1,-1,76,7,_T("Config5"),DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,Config1DialogProc,0);
              if(hDlg!=INVALID_HANDLE_VALUE) DlgCode=Info.DialogRun(hDlg);
              if(DlgCode==3)
              {
                if(PluginsData[ids[MenuCode]].Params&PAR_FILESTART_STORE)
                {
                  HKEY hKey; DWORD Disposition;
                  if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginStartKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
                  {
                    RegSetValueEx(hKey,PluginsData[ids[MenuCode]].Name,0,REG_SZ,(LPBYTE)GetDataPtr(1),(lstrlen(GetDataPtr(1))+1)*sizeof(TCHAR));
                    RegCloseKey(hKey);
                  }
                }
                free(PluginsData[ids[MenuCode]].Start);
                PluginsData[ids[MenuCode]].Start=(TCHAR*)malloc((lstrlen(GetDataPtr(1))+1)*sizeof(TCHAR));
                if(PluginsData[ids[MenuCode]].Start)
                  lstrcpy(PluginsData[ids[MenuCode]].Start,GetDataPtr(1));
              }
              if(hDlg!=INVALID_HANDLE_VALUE) Info.DialogFree(hDlg);
            }
            free(SyntaxTypes);
          }
          free(ids);
        }
      }
    }
    else if(MenuCode==3)
    {
      int ConfCount=0;
      for(int i=0;i<PluginsCount;i++)
        if(PluginsData[i].Params&PAR_COLORS_STORE)
          ConfCount++;
      if(ConfCount)
      {
        int *ids=(int *)malloc(ConfCount*sizeof(int));
        if(ids)
        {
          FarMenuItem *SyntaxTypes=NULL;
          size_t size=sizeof(FarMenuItem)*ConfCount+ConfCount*128*sizeof(TCHAR);
          SyntaxTypes=(FarMenuItem *)malloc(size);
          if(SyntaxTypes)
          {
            TCHAR* data=(TCHAR*)(((char*)SyntaxTypes)+sizeof(FarMenuItem)*ConfCount);
            for(int i=0,j=0;(i<PluginsCount)&&(j<ConfCount);i++)
            {
              if(PluginsData[i].Params&PAR_COLORS_STORE)
              {
                TCHAR* text=data+j*128;
                SyntaxTypes[j].Text=text;
                if(j<(int)lstrlen(hotkeys))
                  FSF.sprintf(text,_T("%c. %s"),hotkeys[j],PluginsData[i].Name);
                else
                  FSF.sprintf(text,_T("%c. %s"),' ',PluginsData[i].Name);
                ids[j]=i;
                j++;
              }
            }
            int MenuCode=0;
            while(true)
            {
              for(int i=0;i<ConfCount;i++)
                SyntaxTypes[i].Flags&=~MIF_SELECTED;
              SyntaxTypes[MenuCode].Flags|=MIF_SELECTED;

              MenuCode=Info.Menu(&MainGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,_T(""),NULL,_T("Config6"),NULL,NULL,SyntaxTypes,ConfCount);
              if(MenuCode==-1) break;
              int ColorCount; char **ColorNames; int *Colors;
              if(PluginsData[ids[MenuCode]].pGetParams&&PluginsData[ids[MenuCode]].pGetParams(PluginsData[ids[MenuCode]].Index,PAR_GET_COLOR_COUNT,(const char **)&ColorCount)&&PluginsData[ids[MenuCode]].pGetParams(PluginsData[ids[MenuCode]].Index,PAR_GET_COLOR_NAME,(const char **)&ColorNames)&&PluginsData[ids[MenuCode]].pGetParams(PluginsData[ids[MenuCode]].Index,PAR_GET_COLOR,(const char **)&Colors))
              {
                FarMenuItem *ColorTypes=NULL;
                size_t size=sizeof(FarMenuItem)*ColorCount+ColorCount*128*sizeof(TCHAR);
                ColorTypes=(FarMenuItem *)malloc(size);
                if(ColorTypes)
                {
                  TCHAR* data=(TCHAR*)(((char*)ColorTypes)+sizeof(FarMenuItem)*ColorCount);
                  for(int i=0;i<ColorCount;i++)
                  {
                    TCHAR* text=data+i*128;
                    ColorTypes[i].Text=text;
                    if(i<(int)lstrlen(hotkeys))
                      FSF.sprintf(text,_T("%c. %s"),hotkeys[i],ColorNames[i]);
                    else
                      FSF.sprintf(text,_T("%c. %s"),' ',ColorNames[i]);
                  }
                  int ColorCode=0;
                  while(true)
                  {
                    for(int i=0;i<ColorCount;i++)
                      ColorTypes[i].Flags&=~MIF_SELECTED;
                    ColorTypes[ColorCode].Flags|=MIF_SELECTED;
                    ColorCode=Info.Menu(&MainGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,PluginsData[ids[MenuCode]].Name,NULL,_T("Config7"),NULL,NULL,ColorTypes,ColorCount);
                    if(ColorCode==-1) break;
                    if(Colors[ColorCode*2]==-1)
                      Colors[ColorCode*2]=Info.AdvControl(&MainGuid,ACTL_GETCOLOR,(void *)COL_EDITORTEXT)&0x0F;
                    if(Colors[ColorCode*2+1]==-1)
                      Colors[ColorCode*2+1]=(Info.AdvControl(&MainGuid,ACTL_GETCOLOR,(void *)COL_EDITORTEXT)&0xF0)>>4;
                    if(SelectColor(Colors+ColorCode*2,Colors+ColorCode*2+1))
                    {
                      if(Colors[ColorCode*2]==(Info.AdvControl(&MainGuid,ACTL_GETCOLOR,(void *)COL_EDITORTEXT)&0x0F))
                        Colors[ColorCode*2]=-1;
                      if(Colors[ColorCode*2+1]==(Info.AdvControl(&MainGuid,ACTL_GETCOLOR,(void *)COL_EDITORTEXT)&0xF0)>>4)
                        Colors[ColorCode*2+1]=-1;
                      HKEY hKey; DWORD Disposition;
                      if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginColorKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
                      {
                        RegSetValueEx(hKey,PluginsData[ids[MenuCode]].Name,0,REG_BINARY,(LPBYTE)Colors,ColorCount*2*sizeof(int));
                        RegCloseKey(hKey);
                      }
                    }
                  }
                }
              }
            }
            free(SyntaxTypes);
          }
          free(ids);
        }
      }
    }
  }
  return false;
}

int WINAPI ProcessEditorEventW(int Event,void *Param)
{
  return OnEditorEvent(Event,Param);;
}

int WINAPI ProcessEditorInputW(const INPUT_RECORD *Rec)
{
  return OnEditorInput(Rec);
}

#ifdef __cplusplus
extern "C"{
#endif
  bool WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

bool WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  return true;
}
