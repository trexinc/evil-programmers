/*
    Scripts sub-plugin for FARMail
    Copyright (C) 2002-2004 FARMail Group
    Copyright (C) 1999,2000 Serge Alexandrov

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
//#include "farkeys.hpp"
#include "scripts.hpp"
#include "language.hpp"
#include <stdio.h>

#if defined(__GNUC__)

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}
#endif

static const char DEFSCRIPTDIR[]="DefScriptDir";

char PluginRootKey[80];
const char NULLSTR[]="";

PluginStartupInfo FInfo;
FARSTANDARDFUNCTIONS FSF;
MailPluginStartupInfo MInfo;
OPTIONS Opt;

static void ReadRegistry(void);
static void Config(void);
static int DoMain(char *FileName);

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct MailPluginStartupInfo *FarMailInfo)
{
  FInfo=*FarInfo;
  FSF=*FarInfo->FSF;
  MInfo=*FarMailInfo;
  FSF.sprintf(PluginRootKey,"%s\\Script",MInfo.RootKey);
  ReadRegistry();
  return 0;
}

void WINAPI _export Exit(void)
{
}

struct DlgData
{
  long index;
  long size;
};

static long WINAPI ScriptDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  switch(Msg)
  {
    case DN_INITDIALOG:
      FInfo.SendDlgMessage(hDlg,DM_SETTEXTLENGTH,((DlgData *)Param2)->index,((DlgData *)Param2)->size);
      break;
    case DN_HELP:
      FInfo.ShowHelp(MInfo.ModuleName,(char *)Param2,FHELP_SELFHELP);
      return (long)NULL;
  }
  return FInfo.DefDlgProc(hDlg,Msg,Param1,Param2);
}


int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData)
{
  (void)InData;
  switch(Msg)
  {
    case FMMSG_GETINFO:
      {
        GetInfoOutData *data=(GetInfoOutData *)OutData;
        GetMsg(MesMenu_Title,data->MenuString);
        lstrcpy(data->HotkeyID,"generic_scripts");
        data->Flags=FMMSG_MENU|FMMSG_CONFIG;
      }
      return TRUE;

    case FMMSG_CONFIG:
      {
        Config();
      }
      return TRUE;

    case FMMSG_MENU:
      {
        static const char *HName2="FARMailScriptRun";
        int key;
        int choice;
        char script[MAX_PATH];
        choice=DoMain(script);
        MInfo.GetString(FMSTR_MSGEXT,Opt.EXT,20,NULL);
        switch(choice)
        {
          case -1:
            return TRUE;
          case 0:
            {
              DlgData data={2,MAX_PATH-1};
              *script=0;
              struct InitDialogItem InitItems[]=
              {
                // type,x1,y1,x2,y2,focus,selected,flags,default,data
                {DI_DOUBLEBOX,3,1,65,6,0,0,0,0,(char*)MesMenu_RunScript},
                {DI_TEXT,5,3,0,0,0,0,0,0,(char*)MesRunScript_Filename},
                {DI_EDIT,5,4,62,0,1,(DWORD)HName2,DIF_HISTORY|DIF_USELASTHISTORY,0,NULLSTR},
                {DI_BUTTON,3,7,0,0,0,0,0,1,(char *)MesOk},
              };
              struct FarDialogItem DialogItems[sizeofa(InitItems)];
              InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));

              key=FInfo.DialogEx(FInfo.ModuleNumber,-1,-1,69,9,"RunScript",DialogItems,sizeofa(DialogItems),0,0,ScriptDlgProc,(long)&data);

              if(key==3)
              {
                lstrcpy(script,DialogItems[2].Data);
                FSF.Unquote(script);
              }
              if(!*script) return TRUE;
            }
            break;
        }
        run_script(script);
      }
      return TRUE;
  }
  return FALSE;
}

void ReadRegistry(void)
{
  HKEY hRoot=HKEY_CURRENT_USER;
  char path[MAX_PATH];
  lstrcpy(path,MInfo.ModuleName);
  *(FSF.PointToName(path))=0;
  lstrcat(path,"SCRIPTS\\");
  GetRegKey2(hRoot,PluginRootKey,NULLSTR,DEFSCRIPTDIR,Opt.DefScriptDir,path,MAX_PATH );
  if (!*Opt.DefScriptDir)
    lstrcpy(Opt.DefScriptDir,path);
  else
  {
    FSF.Unquote(Opt.DefScriptDir);
    FSF.AddEndSlash(Opt.DefScriptDir);
  }
}

void Config(void)
{
  struct InitDialogItem InitItems[]=
  {
    // type,x1,y1,x2,y2,focus,selected,flags,default,data
    {DI_DOUBLEBOX,3,1,65,6,0,0,0,0,(char*)MesConfig_Title},

    {DI_TEXT,5,2,0,0,0,0,0,0,(char*)MesConfig_DefScriptsDir},
    {DI_EDIT,5,3,63,0,1,0,0,0,NULLSTR},

    {DI_TEXT,3,4,0,0,0,0,DIF_SEPARATOR,0,NULLSTR},

    {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,1,(char *)MesOk},
    {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,0,(char *)MesCancel}
  };
  enum
  {
    C2_SCRDIR=2,
    C_TXT,
    C_OK,
    C_CANCEL
  };
  DlgData data={2,248};
  struct FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));

  ReadRegistry();

  lstrcpy(DialogItems[C2_SCRDIR].Data,Opt.DefScriptDir);

  if(FInfo.DialogEx(FInfo.ModuleNumber,-1,-1,69,8,"Config",DialogItems,sizeofa(DialogItems),0,0,ScriptDlgProc,(long)&data)==C_OK)
  {
    lstrcpy(Opt.DefScriptDir,DialogItems[C2_SCRDIR].Data);
    if(!*Opt.DefScriptDir)
    {
      char path[MAX_PATH];
      lstrcpy(path,MInfo.ModuleName);
      *(FSF.PointToName(path))=0;
      lstrcat(path,"SCRIPTS\\");
      lstrcpy(Opt.DefScriptDir,path);
    }
    else
    {
      FSF.Unquote(Opt.DefScriptDir);
      FSF.AddEndSlash(Opt.DefScriptDir);
    }
    SetRegKey2(HKEY_CURRENT_USER,PluginRootKey,NULLSTR,DEFSCRIPTDIR,Opt.DefScriptDir);
  }
}

int DoMain(char *FileName)
{
  #define MAX_ITEMS 20
  #define STATIC_ITEMS 2
  #define MENU_ITEMS (20+1+STATIC_ITEMS)
  char *ScriptFileNames[MAX_ITEMS];
  char *tmp;
  struct FarMenuItem MenuItems[MENU_ITEMS],mi_tmp;
  char fn[512],fd[512];
  int i;
  const char DescHistory[]="FARMailScriptDesc";
  const char PathHistory[]="FARMailScriptPath";
  #define DLG_WIDTH 77
  #define DLG_HEIGHT 10
  struct InitDialogItem InitItems[]=
  {
    {DI_DOUBLEBOX,3,1,73,8,0,0,0,0,(char *)MesDlg_Title},
    {DI_TEXT,5,2,0,0,0,0,0,0,(char *)MesDlg_Desc},
    {DI_EDIT,5,3,70,0,1,(DWORD)DescHistory,DIF_HISTORY,0,NULLSTR},
    {DI_TEXT,5,4,0,0,0,0,0,0,(char *)MesDlg_File},
    {DI_EDIT,5,5,70,0,0,(DWORD)PathHistory,DIF_HISTORY,0,NULLSTR},
    {DI_TEXT,0,6,0,0,0,0,DIF_SEPARATOR,0,NULLSTR},
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(char *)MesOk},
    {DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(char *)MesCancel},
  };
  DlgData data={4,MAX_PATH-1};
  struct FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));

  for(i=0;i<STATIC_ITEMS;i++)
    MenuItems[i].Checked=MenuItems[i].Selected=MenuItems[i].Separator=0;

  {
    char mtmp[100];
    GetMsg(MesMenu_RunScript,mtmp);
    FSF.sprintf(MenuItems[0].Text,"&1. %s",mtmp);
  }
  MenuItems[STATIC_ITEMS-1].Separator=1;
  for(i=0;i<MAX_ITEMS;i++)
  {
    if(!GetScriptInfo(fd,fn,i))
      break;
    ScriptFileNames[i]=(char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(char)*(lstrlen(fn)+1));
    if(ScriptFileNames[i]) lstrcpy(ScriptFileNames[i],fn);
    FSF.sprintf(MenuItems[i+STATIC_ITEMS].Text,"&%c. %.*s",i+'A',66,fd);
    MenuItems[i+STATIC_ITEMS].Checked=MenuItems[i+STATIC_ITEMS].Selected=MenuItems[i+STATIC_ITEMS].Separator=0;
  }
  FSF.sprintf(MenuItems[i+STATIC_ITEMS].Text,"%*c",66/2,' ');
  MenuItems[i+STATIC_ITEMS].Checked=MenuItems[i+STATIC_ITEMS].Selected=MenuItems[i+STATIC_ITEMS].Separator=0;
  int TotalItems=i;
  int LastPos=0;

  int ret=-1;
  while(TRUE)
  {
    int x;
    int BreakCode;
    const int BreakKeys[]=
    {
      VK_F1,
      VK_INSERT,
      VK_DELETE,
      VK_F4,
      (PKF_CONTROL<<16)|VK_DOWN,
      (PKF_CONTROL<<16)|VK_UP,
      VK_RETURN,
      0
    };
    if(ret!=-1) break;
    MenuItems[LastPos].Selected=1;

    {
      char mtmp[100],mtmp2[100];
      i=MInfo.ShowHelpMenu(MInfo.ModuleName,FInfo.ModuleNumber,-1,-1,0,FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT,GetMsg(MesMenu_Title,mtmp),GetMsg(MesMenu_Commands,mtmp2),"Usage",BreakKeys,&BreakCode,MenuItems,TotalItems+1+STATIC_ITEMS);
    }
    if(i>=0)
    {
      MenuItems[LastPos].Selected=0;
      LastPos=i;
      if(BreakCode==-1&&i>=STATIC_ITEMS&&TotalItems)
        BreakCode=6;
      switch(BreakCode)
      {
        case 1:
          DialogItems[2].Focus=TRUE;
          DialogItems[2].Data[0]=0;
          DialogItems[4].Data[0]=0;
          i=FInfo.DialogEx(FInfo.ModuleNumber,-1,-1,DLG_WIDTH,DLG_HEIGHT,"Settings",(struct FarDialogItem *)&DialogItems,sizeofa(InitItems),0,0,ScriptDlgProc,(long)&data);
          if(i!=6||!DialogItems[4].Data[0]||!DialogItems[2].Data[0]||TotalItems>=MAX_ITEMS) break;
          LastPos=TotalItems+STATIC_ITEMS;
          TotalItems++;
          ScriptFileNames[TotalItems-1]=(char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(char)*(lstrlen(DialogItems[4].Data)+1));
          lstrcpy(ScriptFileNames[TotalItems-1],DialogItems[4].Data);
          FSF.sprintf(MenuItems[TotalItems-1+STATIC_ITEMS].Text,"&%c. %.*s",TotalItems+'A'-1,66,DialogItems[2].Data);
          MenuItems[TotalItems-1+STATIC_ITEMS].Checked=MenuItems[TotalItems-1+STATIC_ITEMS].Selected=MenuItems[TotalItems-1+STATIC_ITEMS].Separator=0;
          MenuItems[TotalItems+STATIC_ITEMS].Checked=MenuItems[TotalItems+STATIC_ITEMS].Selected=MenuItems[TotalItems+STATIC_ITEMS].Separator=0;
          FSF.sprintf(MenuItems[TotalItems+STATIC_ITEMS].Text,"%*c",66/2,' ');
          SetScriptInfo(&MenuItems[TotalItems-1+STATIC_ITEMS].Text[4],ScriptFileNames[TotalItems-1],TotalItems-1);
          break;
        case 2:
          if(i>=TotalItems+STATIC_ITEMS||i<STATIC_ITEMS||!TotalItems) break;
          if(i==TotalItems+STATIC_ITEMS-1&&TotalItems>1) LastPos--;
          for(i=i-STATIC_ITEMS;i<TotalItems-1;i++)
          {
            MenuItems[i+STATIC_ITEMS]=MenuItems[i+1+STATIC_ITEMS];
            if(ScriptFileNames[i]) HeapFree(GetProcessHeap(),0,ScriptFileNames[i]);
            ScriptFileNames[i]=ScriptFileNames[i+1];
            ScriptFileNames[i+1]=NULL;
            SetScriptInfo(&MenuItems[i+STATIC_ITEMS].Text[4],ScriptFileNames[i],i);
          }
          MenuItems[i+STATIC_ITEMS]=MenuItems[i+1+STATIC_ITEMS];
          TotalItems--;
          DelScriptInfo(TotalItems);
          for(i=0;i<TotalItems;i++) MenuItems[i+STATIC_ITEMS].Text[1]=i+'A';
          break;
        case 3:
          {
            if(i>=TotalItems+STATIC_ITEMS||i<STATIC_ITEMS) break;
            i=i-STATIC_ITEMS;
            lstrcpy(DialogItems[2].Data,&MenuItems[i+STATIC_ITEMS].Text[4]);
            lstrcpy(DialogItems[4].Data,ScriptFileNames[i]);
            DialogItems[2].Focus=TRUE;
            int j=FInfo.DialogEx(FInfo.ModuleNumber,-1,-1,DLG_WIDTH,DLG_HEIGHT,"Settings",(struct FarDialogItem *)&DialogItems,sizeofa(InitItems),0,0,ScriptDlgProc,(long)&data);
            if(j!=6) break;
            if(ScriptFileNames[i]) HeapFree(GetProcessHeap(),0,ScriptFileNames[i]);
            ScriptFileNames[i]=(char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(char)*(lstrlen(DialogItems[4].Data)+1));
            lstrcpy(ScriptFileNames[i],DialogItems[4].Data);
            FSF.sprintf(MenuItems[i+STATIC_ITEMS].Text,"&%c. %.*s",i+'A',66,DialogItems[2].Data);
            SetScriptInfo(&MenuItems[i+STATIC_ITEMS].Text[4],ScriptFileNames[i],i);
          }
          break;
        case 4:
          x=1;
        case 5:
          {
            if(BreakCode==5) x=-1;
            if(BreakCode==4&&(TotalItems==1||i>=TotalItems+STATIC_ITEMS-1||i<STATIC_ITEMS)) break;
            if(BreakCode==5&&(TotalItems==1||i>=TotalItems+STATIC_ITEMS||i<=STATIC_ITEMS)) break;
            LastPos+=x;
            i=i-STATIC_ITEMS;
            tmp=ScriptFileNames[i+x];
            mi_tmp=MenuItems[i+x+STATIC_ITEMS];
            ScriptFileNames[i+x]=ScriptFileNames[i];
            MenuItems[i+x+STATIC_ITEMS]=MenuItems[i+STATIC_ITEMS];
            MenuItems[i+STATIC_ITEMS]=mi_tmp;
            ScriptFileNames[i]=tmp;
            int j=MenuItems[i+x+STATIC_ITEMS].Text[1];
            MenuItems[i+x+STATIC_ITEMS].Text[1]=(char)(j+x);
            MenuItems[i+STATIC_ITEMS].Text[1]=(char)j;
            SetScriptInfo(&MenuItems[i+STATIC_ITEMS].Text[4],ScriptFileNames[i],i);
            SetScriptInfo(&MenuItems[i+x+STATIC_ITEMS].Text[4],ScriptFileNames[i+x],i+x);
          }
          break;
        default:
          if(i<(TotalItems+STATIC_ITEMS))
          {
            if(i>=STATIC_ITEMS)
            {
              FSF.sprintf(FileName,"%s",ScriptFileNames[i-STATIC_ITEMS]);
            }
            ret=(i==0?0:i-STATIC_ITEMS+1);
          }
          break;
      }
      continue;
    }
    break;
  }
  for(i=0;i<TotalItems;i++)
  {
    if(ScriptFileNames[i]) HeapFree(GetProcessHeap(),0,ScriptFileNames[i]);
  }
  return ret;
}
