/*
    Helper sub-plugin for FARMail
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
#include "plugin.hpp"
#include "farcolor.hpp"
#include "farkeys.hpp"
#include "memory.h"
#include "../FARMail/fmp.hpp"
#include "helper.hpp"

struct OFDData
{
  char curr_dir[MAX_PATH];
  char *filename;
  bool result;
};

static int __cdecl fcmp(const void *first,const void *second)
{
  if((((const FarListItem *)second)->Flags)!=(((const FarListItem *)first)->Flags))
    return (((const FarListItem *)second)->Flags)-(((const FarListItem *)first)->Flags);
  if(!lstrcmp(((const FarListItem *)first)->Text,"..")) return -1;
  if(!lstrcmp(((const FarListItem *)second)->Text,"..")) return 1;
  return(FSF.LStricmp(((const FarListItem *)first)->Text,((const FarListItem *)second)->Text));
}

static void LoadDir(HANDLE hDlg,OFDData *DlgParams)
{
  { //clear listbox
    FarListDelete clear={0,0};
    FarInfo.SendDlgMessage(hDlg,DM_LISTDELETE,0,(long)&clear);
  }
  FarListItem *Items=NULL; DWORD count=0;
  if(DlgParams->curr_dir[0])
  {
    char mask[MAX_PATH]; WIN32_FIND_DATA find;
    lstrcpy(mask,DlgParams->curr_dir);
    lstrcat(mask,"*");
    HANDLE hFind=FindFirstFile(mask,&find);
    if(hFind!=INVALID_HANDLE_VALUE)
    {
      do
      {
        if(lstrcmp(find.cFileName,"."))
        {
          Items=(FarListItem *)realloc(Items,sizeof(FarListItem)*(count+1));
          lstrcpy(Items[count].Text,find.cFileName);
          Items[count].Flags=0;
          if(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
            Items[count].Flags=LIF_CHECKED|'+';
          count++;
        }
      } while(FindNextFile(hFind,&find));
      FindClose(hFind);
      if(Items) FSF.qsort(Items,count,sizeof(FarListItem),fcmp);
    }
  }
  else
  {
    DWORD Disks=GetLogicalDrives();
    for(DWORD i=0;i<26;i++)
      if(Disks&(1<<i))
      {
        Items=(FarListItem *)realloc(Items,sizeof(FarListItem)*(count+1));
        Items[count].Text[0]='A'+i;
        Items[count].Text[1]=':';
        Items[count].Text[2]='\\';
        Items[count].Text[3]=0;
        Items[count].Flags=LIF_CHECKED|'+';
        count++;
      }
  }
  if(count) //add items
  {
    FarList list={count,Items};
    FarInfo.SendDlgMessage(hDlg,DM_LISTADD,0,(long)&list);
  }
  free(Items);
  {
    FarListTitles titles;
    titles.Title=DlgParams->curr_dir;
    titles.TitleLen=lstrlen(titles.Title);
    char tmp[50];
    GetMsg(mListBottomTitle,tmp);
    titles.Bottom = tmp;
    titles.BottomLen=lstrlen(titles.Bottom);
    FarInfo.SendDlgMessage(hDlg,DM_LISTSETTITLES,0,(long)&titles);
  }
}

#define DM_UPDATESIZE DM_USER+1

static long WINAPI OFDProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  OFDData *DlgParams=(OFDData *)FarInfo.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_KEY:
      if(Param2==KEY_ENTER)
      {
        FarListGetItem item;
        item.ItemIndex=FarInfo.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
        if(FarInfo.SendDlgMessage(hDlg,DM_LISTGETITEM,0,(long)&item))
        {
          if(item.Item.Flags&LIF_CHECKED)
          {
            char buffer[MAX_PATH],*fileptr,old_dir[MAX_PATH];
            lstrcpy(old_dir,DlgParams->curr_dir);
            lstrcat(DlgParams->curr_dir,item.Item.Text);
            if(!GetFullPathName(DlgParams->curr_dir,sizeof(buffer),buffer,&fileptr)) /*break*/;
            lstrcpy(DlgParams->curr_dir,buffer);
            FSF.AddEndSlash(DlgParams->curr_dir);
            LoadDir(hDlg,DlgParams);
            if(!lstrcmp(item.Item.Text,".."))
            {
              long Len=lstrlen(old_dir);
              if(Len)
              {
                if(old_dir[Len-1]=='\\') old_dir[Len-1]=0;
                FarListPos Pos;
                FarListFind find={0,FSF.PointToName(old_dir),LIFIND_EXACTMATCH,0};
                Pos.SelectPos=FarInfo.SendDlgMessage(hDlg,DM_LISTFINDSTRING,0,(long)&find);
                Pos.TopPos=-1;
                if(Pos.SelectPos>=0) FarInfo.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,(long)&Pos);
              }
            }
            return TRUE;
          }
        } else return TRUE;
      }
      else if(Param2==KEY_SHIFTENTER)
      {
        FarListGetItem item;
        item.ItemIndex=FarInfo.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
        if(FarInfo.SendDlgMessage(hDlg,DM_LISTGETITEM,0,(long)&item))
        {
          char name[MAX_PATH];
          lstrcpy(name,DlgParams->curr_dir);
          lstrcat(name,item.Item.Text);
          OemToChar(name,name);
          SHELLEXECUTEINFO info;
          memset(&info,0,sizeof(info));
          info.cbSize=sizeof(info);
          info.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_DDEWAIT;
          info.lpFile=name;
          info.nShow=SW_SHOWNORMAL;
          ShellExecuteEx(&info);
        }
        return TRUE;
      }
      else if(Param2==KEY_CTRLBACKSLASH)
      {
        lstrcpy(DlgParams->curr_dir,NULLSTR);
        LoadDir(hDlg,DlgParams);
        return TRUE;
      }
      else if(Param2>=KEY_ALTA&&Param2<=KEY_ALTZ)
      {
        int Drive=Param2-KEY_ALTA;
        DWORD Disks=GetLogicalDrives();
        if(Disks&(1<<Drive))
        {
          char temp_dir[3]="A:",*fileptr;
          temp_dir[0]+=Drive;
          if(!GetFullPathName(temp_dir,sizeof(DlgParams->curr_dir),DlgParams->curr_dir,&fileptr)) lstrcpy(DlgParams->curr_dir,temp_dir);
          FSF.AddEndSlash(DlgParams->curr_dir);
          LoadDir(hDlg,DlgParams);
        }
        return TRUE;
      }
      else if(Param2>=KEY_RCTRL0&&Param2<=KEY_RCTRL9)
      {
        char key_path[MAX_PATH], value[64], data[MAX_PATH];;
        lstrcpyn(key_path,FarInfo.RootKey,FSF.PointToName(FarInfo.RootKey)-FarInfo.RootKey+1);
        lstrcat(key_path,"FolderShortcuts");
        FSF.sprintf(value,"PluginModule%d",Param2-KEY_RCTRL0);
        if(GetRegKey2(HKEY_CURRENT_USER,key_path,NULLSTR,value,data,NULLSTR,MAX_PATH)&&(!*data))
        {
          FSF.sprintf(value,"Shortcut%d",Param2-KEY_RCTRL0);
          GetRegKey2(HKEY_CURRENT_USER,key_path,NULLSTR,value,data,NULLSTR,MAX_PATH);
          if (*data)
          {
            lstrcpy(DlgParams->curr_dir,data);
            FSF.AddEndSlash(DlgParams->curr_dir);
            LoadDir(hDlg,DlgParams);
          }
        }
        return TRUE;
      }
      else if(Param2==KEY_LEFT)
      {
        DWORD Key=KEY_HOME;
        FarInfo.SendDlgMessage(hDlg,DM_KEY,1,(long)&Key);
        return TRUE;
      }
      else if(Param2==KEY_RIGHT)
      {
        DWORD Key=KEY_END;
        FarInfo.SendDlgMessage(hDlg,DM_KEY,1,(long)&Key);
        return TRUE;
      }
      break;
    case DN_CTLCOLORDIALOG:
      return FarInfo.AdvControl(FarInfo.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUTEXT);
    case DN_CTLCOLORDLGLIST:
      if(Param1==0)
      {
        FarListColors *Colors=(FarListColors *)Param2;
        int ColorIndex[]={COL_MENUTEXT,COL_MENUTEXT,COL_MENUTITLE,COL_MENUTEXT,COL_MENUHIGHLIGHT,COL_MENUTEXT,COL_MENUSELECTEDTEXT,COL_MENUSELECTEDHIGHLIGHT,COL_MENUSCROLLBAR,COL_MENUDISABLEDTEXT};
        int Count=sizeof(ColorIndex)/sizeof(ColorIndex[0]);
        if(Count>Colors->ColorCount) Count=Colors->ColorCount;
        for(int i=0;i<Count;i++)
          Colors->Colors[i]=FarInfo.AdvControl(FarInfo.ModuleNumber,ACTL_GETCOLOR,(void *)(ColorIndex[i]));
        return TRUE;
      }
      break;
    case DN_INITDIALOG:
      FarInfo.SendDlgMessage(hDlg,DM_UPDATESIZE,0,0);
      LoadDir(hDlg,(OFDData *)Param2);
      break;
    case DN_HELP:
      FarInfo.ShowHelp(FarMailInfo.ModuleName,"FileDialog",FHELP_SELFHELP);
      return (long)NULL;
    case DN_CLOSE:
      {
        FarListGetItem item;
        item.ItemIndex=FarInfo.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
        if((Param1==0)&&FarInfo.SendDlgMessage(hDlg,DM_LISTGETITEM,0,(long)&item))
        {
          lstrcpy(DlgParams->filename,DlgParams->curr_dir);
          lstrcat(DlgParams->filename,item.Item.Text);
          DlgParams->result=true;
        }
      }
      break;
    case DN_RESIZECONSOLE:
      FarInfo.SendDlgMessage(hDlg,DM_UPDATESIZE,0,0);
      break;
    case DM_UPDATESIZE:
      {
        const int minimal_width=65,minimal_height=12;
        int width=minimal_width,height=minimal_height;
        HANDLE console=CreateFile("CONOUT$",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
        if(console!=INVALID_HANDLE_VALUE)
        {
          CONSOLE_SCREEN_BUFFER_INFO console_info;
          GetConsoleScreenBufferInfo(console,&console_info);
          width=console_info.dwSize.X*3/5;
          if(width<minimal_width) width=minimal_width;
          height=console_info.dwSize.Y*2/3;
          if(height<minimal_height) height=minimal_height;
          if(width>(console_info.dwSize.X-11)) width=console_info.dwSize.X-11;
          if(height>(console_info.dwSize.Y-4)) height=console_info.dwSize.Y-4;
          CloseHandle(console);
        }
        FarInfo.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
        { //minimize listbox
          SMALL_RECT listbox_size={3,1,4,2};
          FarInfo.SendDlgMessage(hDlg,DM_SETITEMPOSITION,0,(long)&listbox_size);
        }
        { //resize and move dialog
          COORD dialog_size={width+6,height+2};
          FarInfo.SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(long)&dialog_size);
          COORD position={-1,-1};
          FarInfo.SendDlgMessage(hDlg,DM_MOVEDIALOG,TRUE,(long)&position);
        }
        { //resize listbox
          SMALL_RECT listbox_size={3,1,width+2,height};
          FarInfo.SendDlgMessage(hDlg,DM_SETITEMPOSITION,0,(long)&listbox_size);
        }
        FarInfo.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
      }
      break;
  }
  return FarInfo.DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool open_file_dialog(const char *curr_dir,char *filename)
{
  FarDialogItem DialogItems[1];
  OFDData params;
  lstrcpy(params.curr_dir,curr_dir);
  params.filename = filename;
  params.result = false;
  memset(DialogItems,0,sizeof(DialogItems));
  DialogItems[0].Type=DI_LISTBOX; //DialogItems[0].Flags=DIF_LISTWRAPMODE;
  FarInfo.DialogEx(FarInfo.ModuleNumber,-1,-1,0,0,NULL,DialogItems,sizeofa(DialogItems),0,0,OFDProc,(DWORD)&params);
  return params.result;
}
