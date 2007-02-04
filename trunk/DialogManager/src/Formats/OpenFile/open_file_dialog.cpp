/*
    OpenFile plugin for DialogManager
    Copyright (C) 2003-2005 Vadim Yegorov and Alex Yaroslavsky

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
#include "../../plugin.hpp"
#include "../../farkeys.hpp"
#include "../../farcolor.hpp"
#include "../../dm_module.hpp"
#include "open_file_dialog.hpp"
#include "openfiledialog.hpp"
#include "memory.hpp"

typedef struct NMNames
{
  char Text[NM];
  DWORD Flags;
} NMNames;

struct OFDData
{
  char curr_dir[MAX_PATH];
  char *filename;
  NMNames *names;
  DWORD ncount;
  bool result;
};

const char folder_mark='+';

static int __cdecl fcmp(DWORD firstFlags,DWORD secondFlags,const char *firstText,const char *secondText)
{
  if(firstFlags!=secondFlags)
    return (secondFlags-firstFlags);
  if(!lstrcmp(firstText,"..")) return -1;
  if(!lstrcmp(secondText,"..")) return 1;
  return(FSF.LStricmp(firstText,secondText));
}

static int __cdecl fcmp1(const void *first,const void *second)
{
  return(
    fcmp(
      ((const FarListItem *)first)->Flags,
      ((const FarListItem *)second)->Flags,
      ((const FarListItem *)first)->Text,
      ((const FarListItem *)second)->Text
    )
  );
}

static int __cdecl fcmp2(const void *first,const void *second)
{
  return(
    fcmp(
      ((const NMNames *)first)->Flags,
      ((const NMNames *)second)->Flags,
      ((const NMNames *)first)->Text,
      ((const NMNames *)second)->Text
    )
  );
}

static int TryLoadDir(HANDLE hDlg, OFDData *DlgParams, char *newdir)
{
  NMNames *newnames = NULL;
  FarListItem *Items=NULL; DWORD count=0;
  if(newdir[0])
  {
    char mask[MAX_PATH]; WIN32_FIND_DATA find;
    lstrcpy(mask,newdir);
    lstrcat(mask,"*");
    HANDLE hFind=FindFirstFile(mask,&find);
    if(hFind!=INVALID_HANDLE_VALUE)
    {
      do
      {
        if(lstrcmp(find.cFileName,"."))
        {
          Items=(FarListItem *)realloc(Items,sizeof(FarListItem)*(count+1));
          newnames=(NMNames *)realloc(newnames,sizeof(NMNames)*(count+1));
          lstrcpy(newnames[count].Text,find.cFileName);
          lstrcpyn(Items[count].Text,find.cFileName,sizeof(Items[0].Text));
          Items[count].Text[sizeof(Items[0].Text)-1]=0;
          Items[count].Flags=0;
          newnames[count].Flags=0;
          if(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
          {
            Items[count].Flags=LIF_CHECKED|folder_mark;
            newnames[count].Flags=LIF_CHECKED|folder_mark;
          }
          count++;
        }
      } while(FindNextFile(hFind,&find));
      FindClose(hFind);
      if(Items) FSF.qsort(Items,count,sizeof(FarListItem),fcmp1);
      if(newnames) FSF.qsort(newnames,count,sizeof(NMNames),fcmp2);
    }
  }
  else
  {
    DWORD Disks=GetLogicalDrives();
    for(DWORD i=0;i<26;i++)
      if(Disks&(1<<i))
      {
        Items=(FarListItem *)realloc(Items,sizeof(FarListItem)*(count+1));
        newnames=(NMNames *)realloc(newnames,sizeof(NMNames)*(count+1));
        Items[count].Text[0]=(char)('A'+i);
        Items[count].Text[1]=':';
        Items[count].Text[2]='\\';
        Items[count].Text[3]=0;
        lstrcpy(newnames[count].Text,Items[count].Text);
        Items[count].Flags=LIF_CHECKED|folder_mark;
        newnames[count].Flags=LIF_CHECKED|folder_mark;
        count++;
      }
  }
  if (Items&&newnames) //add items
  {
    //clear name list
    if (DlgParams->names)
    {
      free(DlgParams->names);
      DlgParams->names=NULL;
    }

    DlgParams->names=(NMNames *)realloc(DlgParams->names,sizeof(NMNames)*(count));
    CopyMemory(DlgParams->names,newnames,sizeof(NMNames)*(count));

    lstrcpy(DlgParams->curr_dir,newdir);

    FarList list={count,Items};
    FarInfo.SendDlgMessage(hDlg,DM_LISTSET,0,(long)&list);

    free(Items);
    free(newnames);
    {
      FarListTitles titles;
      titles.Title=DlgParams->curr_dir;
      titles.TitleLen=lstrlen(titles.Title);
      char tmp[256];
      GetMsg(mBottomTitle,tmp);
      titles.Bottom=tmp;
      titles.BottomLen=lstrlen(titles.Bottom);
      FarInfo.SendDlgMessage(hDlg,DM_LISTSETTITLES,0,(long)&titles);
    }
  }
  return count;
}

#define DM_UPDATESIZE DM_USER+1

static long WINAPI OFDProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  OFDData *DlgParams=(OFDData *)FarInfo.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  char newdir[MAX_PATH];

  char newcurdir[MAX_PATH];
  FarListGetItem item;
  BOOL ItemPresent=FALSE;
  if (Msg==DN_KEY||Msg==DN_CLOSE)
  {
    item.ItemIndex=FarInfo.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
    ItemPresent=FarInfo.SendDlgMessage(hDlg,DM_LISTGETITEM,0,(long)&item);
    if(ItemPresent)
    {
      lstrcpy(newcurdir,DlgParams->curr_dir);
      lstrcat(newcurdir,DlgParams->names[item.ItemIndex].Text);
    }
  }

  switch(Msg)
  {
    case DN_KEY:
      if(Param2==KEY_ENTER||Param2==KEY_CTRLENTER||Param2==KEY_CTRLBACKSLASH)
      {
        bool bRoot = Param2==KEY_CTRLBACKSLASH;
        if(ItemPresent||bRoot)
        {
          if(bRoot||(item.Item.Flags&LIF_CHECKED&&Param2==KEY_ENTER))
          {
            char *ptr,old_dir[MAX_PATH];//,buffer[MAX_PATH];
            lstrcpy(old_dir,DlgParams->curr_dir);
            int Len=lstrlen(old_dir);
            if (bRoot)
            {
              *newdir=0;
              lstrcpy(newcurdir,"..");
            }
            else
            {
              GetFullPathName(newcurdir,sizeof(newdir),newdir,&ptr);
              FSF.AddEndSlash(newdir);
            }
            //MessageBox(NULL,old_dir,newcurdir,MB_OK);
            if (TryLoadDir(hDlg,DlgParams,newdir))
            {
              if(!lstrcmp(newcurdir+(bRoot?0:Len),".."))
              {
                if(Len)
                {
                  if (bRoot)
                  {
                    ptr=old_dir;
                    for (; *ptr && *ptr!='\\'; ptr++)
                      ;
                    if (*ptr=='\\')
                      ptr[1]=0;
                    ptr=old_dir;
                  }
                  else
                  {
                    if(old_dir[Len-1]=='\\')
                      old_dir[Len-1]=0;
                    ptr=FSF.PointToName(old_dir);
                  }
                  FarListPos Pos;
                  FarListFind find={0,ptr,LIFIND_EXACTMATCH,0};
                  Pos.SelectPos=FarInfo.SendDlgMessage(hDlg,DM_LISTFINDSTRING,0,(long)&find);
                  Pos.TopPos=-1;
                  if(Pos.SelectPos>=0)
                    FarInfo.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,(long)&Pos);
                }
              }
            }
            return TRUE;
          }
          if(Param2!=KEY_ENTER)
          {
            FarInfo.SendDlgMessage(hDlg,DM_CLOSE,0,0);
            return TRUE;
          }
          return FALSE;
        } else return TRUE;
      }
      else if(Param2==KEY_SHIFTENTER)
      {
        if(ItemPresent)
        {
          OemToChar(newcurdir,newcurdir);
          SHELLEXECUTEINFO info;
          ZeroMemory(&info,sizeof(info));
          info.cbSize=sizeof(info);
          info.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_DDEWAIT;
          info.lpFile=newcurdir;
          info.nShow=SW_SHOWNORMAL;
          ShellExecuteEx(&info);
        }
        return TRUE;
      }
      else if(Param2>=KEY_ALTA&&Param2<=KEY_ALTZ)
      {
        int Drive=Param2-KEY_ALTA;
        DWORD Disks=GetLogicalDrives();
        if(Disks&(1<<Drive))
        {
          char temp_dir[3]="A:",*fileptr;
          temp_dir[0]=(char)(temp_dir[0]+Drive);
          if(!GetFullPathName(temp_dir,sizeof(newdir),newdir,&fileptr))
            lstrcpy(newdir,temp_dir);
          FSF.AddEndSlash(newdir);
          TryLoadDir(hDlg,DlgParams,newdir);
        }
        return TRUE;
      }
      else if(Param2>=KEY_RCTRL0&&Param2<=KEY_RCTRL9)
      {
        char key_path[MAX_PATH], value[64], data[MAX_PATH];;
        lstrcpyn(key_path,FarInfo.RootKey,FSF.PointToName(FarInfo.RootKey)-FarInfo.RootKey+1);
        lstrcat(key_path,"FolderShortcuts");
        FSF.sprintf(value,"PluginModule%d",Param2-KEY_RCTRL0);
        if(GetRegKey(HKEY_CURRENT_USER,key_path,"",value,data,"",MAX_PATH)&&(!*data))
        {
          FSF.sprintf(value,"Shortcut%d",Param2-KEY_RCTRL0);
          GetRegKey(HKEY_CURRENT_USER,key_path,"",value,data,"",MAX_PATH);
          if(*data)
          {
            lstrcpy(newdir,data);
            FSF.AddEndSlash(newdir);
            TryLoadDir(hDlg,DlgParams,newdir);
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
          Colors->Colors[i]=(unsigned char)FarInfo.AdvControl(FarInfo.ModuleNumber,ACTL_GETCOLOR,(void *)(ColorIndex[i]));
        return TRUE;
      }
      break;
    case DN_INITDIALOG:
      FarInfo.SendDlgMessage(hDlg,DM_UPDATESIZE,0,0);
      TryLoadDir(hDlg,(OFDData *)Param2,((OFDData *)Param2)->curr_dir);
      break;
    case DN_HELP:
      FarInfo.ShowHelp(DialogInfo.ModuleName,"FileDialog",FHELP_SELFHELP);
      return (long)NULL;
    case DN_CLOSE:
      {
        if((Param1==0)&&ItemPresent)
        {
          lstrcpy(DlgParams->filename,DlgParams->curr_dir);
          if(lstrcmp(DlgParams->names[item.ItemIndex].Text,".."))
            lstrcat(DlgParams->filename,DlgParams->names[item.ItemIndex].Text);
          DlgParams->result=true;
        }
        if (DlgParams->names)
        {
          free(DlgParams->names);
          DlgParams->names=NULL;
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
          COORD dialog_size={(short)(width+6),(short)(height+2)};
          FarInfo.SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(long)&dialog_size);
          COORD position={-1,-1};
          FarInfo.SendDlgMessage(hDlg,DM_MOVEDIALOG,TRUE,(long)&position);
        }
        { //resize listbox
          SMALL_RECT listbox_size={3,1,(short)(width+2),(short)(height)};
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
  params.names=NULL;
  params.result = false;
  ZeroMemory(DialogItems,sizeof(DialogItems));
  DialogItems[0].Type=DI_LISTBOX; //DialogItems[0].Flags=DIF_LISTWRAPMODE;
  FarInfo.DialogEx(FarInfo.ModuleNumber,-1,-1,0,0,NULL,DialogItems,sizeofa(DialogItems),0,0,OFDProc,(DWORD)&params);
  return params.result;
}
