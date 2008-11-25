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
#include "../../plugin.hpp"
#include "../../farcolor.hpp"
#include "memory.h"
#include "ab_main.h"
#include "./bootstrap/abplugin.h"
#include "./bootstrap/abversion.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
char PluginRootKey[80];
char PluginMaskKey[80];
char PluginColorKey[80];
char PluginStartKey[80];
bool IsOldFAR=true;
HANDLE Mutex=NULL;
int cursor_row=-1,cursor_col=-1;

struct Options Opt={true,50000,true};
bool fatal;

void InitDialogItems(InitDialogItem *Init,FarDialogItem *Item,int ItemsNumber)
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

static char hotkeys[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

const char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

char *GetCommaWord(char *Src,char *Word)
{
  int WordPos,SkipBrackets;
  if (*Src==0)
    return(NULL);
  SkipBrackets=false;
  for (WordPos=0;*Src!=0;Src++,WordPos++)
  {
    if (*Src=='['&&strchr(Src+1,']')!=NULL)
      SkipBrackets=true;
    if (*Src==']')
      SkipBrackets=false;
    if (*Src==','&&!SkipBrackets)
    {
      Word[WordPos]=0;
      Src++;
      while (isspace(*Src))
        Src++;
      return(Src);
    }
    else
      Word[WordPos]=*Src;
  }
  Word[WordPos]=0;
  return(Src);
}

void WINAPI SetStartupInfo(const struct PluginStartupInfo *Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>(int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
  if(Info->StructSize>FAR165_INFO_SIZE)
  {
    IsOldFAR=false;
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;
    Mutex=CreateMutex(NULL,FALSE,NULL);
    //read settings
    strcpy(PluginRootKey,Info->RootKey);
    strcat(PluginRootKey,"\\AirBrush");
    strcpy(PluginMaskKey,PluginRootKey);
    strcat(PluginMaskKey,"\\masks");
    strcpy(PluginColorKey,PluginRootKey);
    strcat(PluginColorKey,"\\colors");
    strcpy(PluginStartKey,PluginRootKey);
    strcat(PluginStartKey,"\\starts");

    HKEY hKey;
    DWORD Type;
    DWORD DataSize=0;
    if((RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
    {
      DataSize=sizeof(Opt.Active);
      RegQueryValueEx(hKey,"Active",0,&Type,(LPBYTE)&Opt.Active,&DataSize);
      DataSize=sizeof(Opt.MaxLines);
      RegQueryValueEx(hKey,"MaxLines",0,&Type,(LPBYTE)&Opt.MaxLines,&DataSize);
      DataSize=sizeof(Opt.ColorizeAll);
      RegQueryValueEx(hKey,"ColorizeAll",0,&Type,(LPBYTE)&Opt.ColorizeAll,&DataSize);
      RegCloseKey(hKey);
      if((Opt.MaxLines<0)||(Opt.MaxLines>9999999)) Opt.MaxLines=50000;
    }
    LoadPlugs(Info->ModuleName);
    OnLoad();
  }
}

void WINAPI GetPluginInfo(struct PluginInfo *Info)
{
  if(!IsOldFAR)
  {
    static const char *PluginMenuStrings;
    memset(Info,0,sizeof(*Info));
    Info->Flags = PF_EDITOR | PF_DISABLEPANELS;
    Info->StructSize = sizeof(*Info);
    Info->PluginConfigStringsNumber = 1;
    Info->PluginMenuStringsNumber = 1;
    PluginMenuStrings = GetMsg(mName);
    Info->PluginConfigStrings = &PluginMenuStrings;
    Info->PluginMenuStrings = &PluginMenuStrings;
  }
}

void WINAPI ExitFAR()
{
  if(!IsOldFAR)
  {
    OnExit();
    UnloadPlugs();
    CloseHandle(Mutex);
  }
}

int WINAPI _export GetMinFarVersion(void)
{
  return MAKEFARVERSION(1,70,1282);
}

HANDLE WINAPI OpenPlugin(int OpenFrom,int Item)
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
      SyntaxTypes=(FarMenuItem *)malloc(sizeof(FarMenuItem)*(Count+1));
      if(SyntaxTypes)
      {
        EditorInfo ei;
        PEditFile curfile;
        int index=-1;
        Info.EditorControl(ECTL_GETINFO,&ei);
        curfile=ef_getfile(ei.EditorID);
        if(curfile) index=curfile->type+1;
        sprintf(SyntaxTypes[0].Text,"%c. %s",hotkeys[0],GetMsg(mDefault));
        if(!index) SyntaxTypes[0].Checked='*';
        ids[0]=-1;
        index--;
        for(int i=0,j=1;i<PluginsCount;i++)
        {
          if(PluginsData[i].Params&PAR_SHOW_IN_LIST)
          {
            if(j<(int)(strlen(hotkeys)-1))
              sprintf(SyntaxTypes[j].Text,"%c. %s",hotkeys[j],PluginsData[i].Name);
            else
              sprintf(SyntaxTypes[j].Text,"%c. %s",' ',PluginsData[i].Name);
            if(index==i) SyntaxTypes[j].Checked='*';
            ids[j]=i;
            j++;
          }
        }
        int MenuCode=0,BreakKeys[]={VK_F4,0},BreakCode;
        while(true)
        {
          for(int i=0;i<=Count;i++)
            SyntaxTypes[i].Selected=false;
          SyntaxTypes[MenuCode].Selected=true;
          BreakCode=-1;
          MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,"",NULL,NULL,BreakKeys,&BreakCode,SyntaxTypes,Count+1);
          if(BreakCode==-1) break;
          if((BreakCode==0)&&(MenuCode>0))
            if(PluginsData[ids[MenuCode]].pGetParams)
              PluginsData[ids[MenuCode]].pGetParams(PluginsData[ids[MenuCode]].Index,PAR_CONFIGURE1,NULL);
        }
        if(MenuCode>-1)
        {
          EditorInfo ei;
          Info.EditorControl(ECTL_GETINFO,&ei);
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

long WINAPI Config1DialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

int WINAPI Configure(int ItemNumber)
{
  FarMenuItem MenuItems[4];
  memset(MenuItems,0,sizeof(MenuItems));
  int Msgs[]={mConfigMenu1,mConfigMenu2,mConfigMenu3,mConfigMenu4};

  for(unsigned int i=0;i<sizeof(MenuItems)/sizeof(MenuItems[0]);i++)
  {
    MenuItems[i].Checked=MenuItems[i].Separator=0;
    sprintf(MenuItems[i].Text,"%s",GetMsg(Msgs[i])); // Text in menu
  };
  int MenuCode=0;
  while(true)
  {
    for(unsigned int i=0;i<sizeof(MenuItems)/sizeof(MenuItems[0]);i++)
      MenuItems[i].Selected=0;
    MenuItems[MenuCode].Selected=true;
    // Show menu
    MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,
                           NULL,NULL,"Config",NULL,NULL,
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
      /*0*/  {DI_DOUBLEBOX,3,1,72,7,0,0,0,0,(const char*)mName},
      /*1*/  {DI_CHECKBOX,5,2,0,0,1,0,0,0,(const char*)mConfigDialogActive},
      /*2*/  {DI_CHECKBOX,5,3,0,0,1,0,0,0,(const char*)mConfigDialogColorizeAll},
      /*3*/  {DI_TEXT,5,4,0,0,0,0,0,0,(const char*)mConfigDialogMaxLines},
      /*4*/  {DI_FIXEDIT,64,4,70,0,1,(int)"######9",DIF_MASKEDIT,0,""},
      /*5*/  {DI_TEXT,-1,5,0,0,0,0,DIF_SEPARATOR,0,""},
      /*6*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP,1,(const char*)mConfigSave},
      /*7*/  {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP,0,(const char*)mConfigCancel}
      };
      struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
      InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
      DialogItems[1].Selected=Opt.Active;
      DialogItems[2].Selected=Opt.ColorizeAll;
      sprintf(DialogItems[4].Data,"%d",Opt.MaxLines);
      int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,76,9,"Config1",DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,Config1DialogProc,0);
      if(DlgCode==6)
      {
        Opt.Active=DialogItems[1].Selected;
        Opt.ColorizeAll=DialogItems[2].Selected;
        Opt.MaxLines=atoi(DialogItems[4].Data); if((Opt.MaxLines<0)||(Opt.MaxLines>9999999)) Opt.MaxLines=50000;
        HKEY hKey;
        DWORD Disposition;
        if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
        {
          RegSetValueEx(hKey,"Active",0,REG_BINARY,(LPBYTE)&Opt.Active,sizeof(Opt.Active));
          RegSetValueEx(hKey,"MaxLines",0,REG_DWORD,(LPBYTE)&Opt.MaxLines,sizeof(Opt.MaxLines));
          RegSetValueEx(hKey,"ColorizeAll",0,REG_DWORD,(LPBYTE)&Opt.ColorizeAll,sizeof(Opt.ColorizeAll));
          RegCloseKey(hKey);
        }
      }
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
          SyntaxTypes=(FarMenuItem *)malloc(ConfCount*sizeof(FarMenuItem));
          if(SyntaxTypes)
          {
            for(int i=0,j=0;(i<PluginsCount)&&(j<ConfCount);i++)
            {
              if(PluginsData[i].Params&PAR_MASK_STORE)
              {
                if(j<(int)strlen(hotkeys))
                  sprintf(SyntaxTypes[j].Text,"%c. %s",hotkeys[j],PluginsData[i].Name);
                else
                  sprintf(SyntaxTypes[j].Text,"%c. %s",' ',PluginsData[i].Name);
                ids[j]=i;
                j++;
              }
            }
            int MenuCode=0;
            while(true)
            {
              for(int i=0;i<ConfCount;i++)
                SyntaxTypes[i].Selected=0;
              SyntaxTypes[MenuCode].Selected=true;

              MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,"",NULL,"Config2",NULL,NULL,SyntaxTypes,ConfCount);
              if(MenuCode==-1) break;
              char mask[2048];
              strcpy(mask,"");
              if(PluginsData[ids[MenuCode]].Mask)
                strcpy(mask,PluginsData[ids[MenuCode]].Mask);
              static struct InitDialogItem InitItems[]={
              /*0*/  {DI_DOUBLEBOX,3,1,72,5,0,0,0,0,""},
              /*1*/  {DI_EDIT,5,2,70,0,1,0,DIF_VAREDIT,0,""},
              /*2*/  {DI_TEXT,-1,3,0,0,0,0,DIF_SEPARATOR,0,""},
              /*3*/  {DI_BUTTON,0,4,0,0,0,0,DIF_CENTERGROUP,1,(char *)mConfigSave},
              /*4*/  {DI_BUTTON,0,4,0,0,0,0,DIF_CENTERGROUP,0,(char *)mConfigCancel}
              };
              struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
              InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
              DialogItems[1].Ptr.PtrFlags=0;
              DialogItems[1].Ptr.PtrLength=sizeof(mask);
              DialogItems[1].Ptr.PtrData=mask;
              strcpy(DialogItems[0].Data,PluginsData[ids[MenuCode]].Name);
              int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,76,7,"Config3",DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,Config1DialogProc,0);
              if(DlgCode==3)
              {
                if(PluginsData[ids[MenuCode]].Params&PAR_MASK_STORE)
                {
                  HKEY hKey; DWORD Disposition;
                  if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginMaskKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
                  {
                    RegSetValueEx(hKey,PluginsData[ids[MenuCode]].Name,0,REG_SZ,(LPBYTE)mask,strlen(mask)+1);
                    RegCloseKey(hKey);
                  }
                }
                free(PluginsData[ids[MenuCode]].Mask);
                PluginsData[ids[MenuCode]].Mask=(char *)malloc(strlen(mask)+1);
                if(PluginsData[ids[MenuCode]].Mask)
                  strcpy(PluginsData[ids[MenuCode]].Mask,mask);
              }
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
          SyntaxTypes=(FarMenuItem *)malloc(ConfCount*sizeof(FarMenuItem));
          if(SyntaxTypes)
          {
            for(int i=0,j=0;(i<PluginsCount)&&(j<ConfCount);i++)
            {
              if(PluginsData[i].Params&PAR_FILESTART_STORE)
              {
                if(j<(int)strlen(hotkeys))
                  sprintf(SyntaxTypes[j].Text,"%c. %s",hotkeys[j],PluginsData[i].Name);
                else
                  sprintf(SyntaxTypes[j].Text,"%c. %s",' ',PluginsData[i].Name);
                ids[j]=i;
                j++;
              }
            }
            int MenuCode=0;
            while(true)
            {
              for(int i=0;i<ConfCount;i++)
                SyntaxTypes[i].Selected=0;
              SyntaxTypes[MenuCode].Selected=true;

              MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,"",NULL,"Config4",NULL,NULL,SyntaxTypes,ConfCount);
              if(MenuCode==-1) break;
              char start[2048];
              strcpy(start,"");
              if(PluginsData[ids[MenuCode]].Start)
                strcpy(start,PluginsData[ids[MenuCode]].Start);
              static struct InitDialogItem InitItems[]={
              /*0*/  {DI_DOUBLEBOX,3,1,72,5,0,0,0,0,""},
              /*1*/  {DI_EDIT,5,2,70,0,1,0,DIF_VAREDIT,0,""},
              /*2*/  {DI_TEXT,-1,3,0,0,0,0,DIF_SEPARATOR,0,""},
              /*3*/  {DI_BUTTON,0,4,0,0,0,0,DIF_CENTERGROUP,1,(char *)mConfigSave},
              /*4*/  {DI_BUTTON,0,4,0,0,0,0,DIF_CENTERGROUP,0,(char *)mConfigCancel}
              };
              struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
              InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
              DialogItems[1].Ptr.PtrFlags=0;
              DialogItems[1].Ptr.PtrLength=sizeof(start);
              DialogItems[1].Ptr.PtrData=start;
              strcpy(DialogItems[0].Data,PluginsData[ids[MenuCode]].Name);
              int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,76,7,"Config5",DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,Config1DialogProc,0);
              if(DlgCode==3)
              {
                if(PluginsData[ids[MenuCode]].Params&PAR_FILESTART_STORE)
                {
                  HKEY hKey; DWORD Disposition;
                  if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginStartKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
                  {
                    RegSetValueEx(hKey,PluginsData[ids[MenuCode]].Name,0,REG_SZ,(LPBYTE)start,strlen(start)+1);
                    RegCloseKey(hKey);
                  }
                }
                free(PluginsData[ids[MenuCode]].Start);
                PluginsData[ids[MenuCode]].Start=(char *)malloc(strlen(start)+1);
                if(PluginsData[ids[MenuCode]].Start)
                  strcpy(PluginsData[ids[MenuCode]].Start,start);
              }
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
          SyntaxTypes=(FarMenuItem *)malloc(ConfCount*sizeof(FarMenuItem));
          if(SyntaxTypes)
          {
            for(int i=0,j=0;(i<PluginsCount)&&(j<ConfCount);i++)
            {
              if(PluginsData[i].Params&PAR_COLORS_STORE)
              {
                if(j<(int)strlen(hotkeys))
                  sprintf(SyntaxTypes[j].Text,"%c. %s",hotkeys[j],PluginsData[i].Name);
                else
                  sprintf(SyntaxTypes[j].Text,"%c. %s",' ',PluginsData[i].Name);
                ids[j]=i;
                j++;
              }
            }
            int MenuCode=0;
            while(true)
            {
              for(int i=0;i<ConfCount;i++)
                SyntaxTypes[i].Selected=0;
              SyntaxTypes[MenuCode].Selected=true;

              MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,"",NULL,"Config6",NULL,NULL,SyntaxTypes,ConfCount);
              if(MenuCode==-1) break;
              int ColorCount; char **ColorNames; int *Colors;
              if(PluginsData[ids[MenuCode]].pGetParams&&PluginsData[ids[MenuCode]].pGetParams(PluginsData[ids[MenuCode]].Index,PAR_GET_COLOR_COUNT,(const char **)&ColorCount)&&PluginsData[ids[MenuCode]].pGetParams(PluginsData[ids[MenuCode]].Index,PAR_GET_COLOR_NAME,(const char **)&ColorNames)&&PluginsData[ids[MenuCode]].pGetParams(PluginsData[ids[MenuCode]].Index,PAR_GET_COLOR,(const char **)&Colors))
              {
                FarMenuItem *ColorTypes=NULL;
                ColorTypes=(FarMenuItem *)malloc(ColorCount*sizeof(FarMenuItem));
                if(ColorTypes)
                {
                  for(int i=0;i<ColorCount;i++)
                  {
                    if(i<(int)strlen(hotkeys))
                      sprintf(ColorTypes[i].Text,"%c. %s",hotkeys[i],ColorNames[i]);
                    else
                      sprintf(ColorTypes[i].Text,"%c. %s",' ',ColorNames[i]);
                  }
                  int ColorCode=0;
                  while(true)
                  {
                    for(int i=0;i<ColorCount;i++)
                      ColorTypes[i].Selected=0;
                    ColorTypes[ColorCode].Selected=true;
                    ColorCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,PluginsData[ids[MenuCode]].Name,NULL,"Config7",NULL,NULL,ColorTypes,ColorCount);
                    if(ColorCode==-1) break;
                    if(Colors[ColorCode*2]==-1)
                      Colors[ColorCode*2]=Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_EDITORTEXT)&0x0F;
                    if(Colors[ColorCode*2+1]==-1)
                      Colors[ColorCode*2+1]=(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_EDITORTEXT)&0xF0)>>4;
                    if(SelectColor(Colors+ColorCode*2,Colors+ColorCode*2+1))
                    {
                      if(Colors[ColorCode*2]==(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_EDITORTEXT)&0x0F))
                        Colors[ColorCode*2]=-1;
                      if(Colors[ColorCode*2+1]==(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_EDITORTEXT)&0xF0)>>4)
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

int WINAPI ProcessEditorEvent(int Event,void *Param)
{
  if(!IsOldFAR)
  {
    return OnEditorEvent(Event,Param);;
  }
  return 0;
}
int WINAPI ProcessEditorInput(const INPUT_RECORD *Rec)
{
  if(!IsOldFAR)
  {
    return OnEditorInput(Rec);
  }
  return 0;
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
