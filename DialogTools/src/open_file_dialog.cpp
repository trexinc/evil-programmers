/*
    open_file_dialog.cpp
    Copyright (C) 2003-2005 Vadim Yegorov and Alex Yaroslavsky
    Copyright (C) 2008 zg

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

#include "dt.hpp"
#include "open_file_dialog.hpp"
#include "farcolor.hpp"
#include "guid.hpp"
#include <initguid.h>
// {6FF19CDE-E672-4887-81A0-05D49C96E42D}
DEFINE_GUID(FileDialogGuid, 0x6ff19cde, 0xe672, 0x4887, 0x81, 0xa0, 0x5, 0xd4, 0x9c, 0x96, 0xe4, 0x2d);

typedef struct NMNames
{
  TCHAR Text[MAX_PATH];
  DWORD Flags;
} NMNames;

struct OFDData
{
  TCHAR curr_dir[MAX_PATH];
  TCHAR *filename;
  NMNames *names;
  DWORD ncount;
  bool result;
};

const TCHAR folder_mark='+';

static int __cdecl fcmp(DWORD firstFlags,DWORD secondFlags,const TCHAR *firstText,const TCHAR *secondText)
{
  if(firstFlags!=secondFlags)
    return (secondFlags-firstFlags);
  if(!lstrcmp(firstText,_T(".."))) return -1;
  if(!lstrcmp(secondText,_T(".."))) return 1;
  return(FSF.LStricmp(firstText,secondText));
}

static int WINAPI fcmp2(const void *first,const void *second,void *)
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

static int TryLoadDir(HANDLE hDlg, OFDData *DlgParams, TCHAR *newdir)
{
  NMNames *newnames = NULL;
  FarListItem *Items=NULL; DWORD count=0;
  if(newdir[0])
  {
    TCHAR mask[MAX_PATH]; WIN32_FIND_DATA find;
    lstrcpy(mask,newdir);
    lstrcat(mask,_T("*"));
    HANDLE hFind=FindFirstFile(mask,&find);
    if(hFind!=INVALID_HANDLE_VALUE)
    {
      do
      {
        if(lstrcmp(find.cFileName,_T(".")))
        {
          newnames=(NMNames *)realloc(newnames,sizeof(NMNames)*(count+1));
          lstrcpy(newnames[count].Text,find.cFileName);
          newnames[count].Flags=0;
          if(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
          {
            newnames[count].Flags=LIF_CHECKED|folder_mark;
          }
          count++;
        }
      } while(FindNextFile(hFind,&find));
      FindClose(hFind);
      if(newnames) FSF.qsort(newnames,count,sizeof(NMNames),fcmp2,NULL);
    }
  }
  else
  {
    DWORD Disks=GetLogicalDrives();
    for(DWORD i=0;i<26;i++)
      if(Disks&(1<<i))
      {
        newnames=(NMNames *)realloc(newnames,sizeof(NMNames)*(count+1));
        newnames[count].Text[0]=(TCHAR)('A'+i);
        newnames[count].Text[1]=':';
        newnames[count].Text[2]='\\';
        newnames[count].Text[3]=0;
        newnames[count].Flags=LIF_CHECKED|folder_mark;
        count++;
      }
  }
  if (newnames) //add items
  {
    Items=(FarListItem *)malloc(sizeof(FarListItem)*count);
    for(size_t ii=0;ii<count;ii++)
    {
      Items[ii].Flags=newnames[ii].Flags;
      Items[ii].Text=newnames[ii].Text;
    }
    //clear name list
    if (DlgParams->names)
    {
      free(DlgParams->names);
      DlgParams->names=NULL;
    }

    DlgParams->names=(NMNames *)realloc(DlgParams->names,sizeof(NMNames)*(count));
    CopyMemory(DlgParams->names,newnames,sizeof(NMNames)*(count));

    lstrcpy(DlgParams->curr_dir,newdir);

    FarList list={sizeof(FarList),count,Items};
    Info.SendDlgMessage(hDlg,DM_LISTSET,0,&list);

    free(Items);
    free(newnames);
    {
      FarListTitles titles={sizeof(FarListTitles)};
      titles.Title=DlgParams->curr_dir;
      titles.TitleSize=lstrlen(titles.Title);
      TCHAR tmp[256];
      _tcscpy(tmp,GetMsg(mBottomTitle));
      titles.Bottom=tmp;
      titles.BottomSize=lstrlen(titles.Bottom);
      Info.SendDlgMessage(hDlg,DM_LISTSETTITLES,0,&titles);
    }
  }
  return count;
}

#define DM_UPDATESIZE DM_USER+1

static intptr_t WINAPI OFDProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
  OFDData *DlgParams=(OFDData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  TCHAR newdir[MAX_PATH];

  TCHAR newcurdir[MAX_PATH];
  FarListGetItem item={sizeof(FarListGetItem)};
  BOOL ItemPresent=FALSE;
  if (Msg==DN_CONTROLINPUT||Msg==DN_CLOSE)
  {
    item.ItemIndex=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
    ItemPresent=Info.SendDlgMessage(hDlg,DM_LISTGETITEM,0,&item);
    if(ItemPresent)
    {
      lstrcpy(newcurdir,DlgParams->curr_dir);
      lstrcat(newcurdir,DlgParams->names[item.ItemIndex].Text);
    }
  }

  switch(Msg)
  {
    case DN_CONTROLINPUT:
      {
        const INPUT_RECORD* record=(const INPUT_RECORD*)Param2;
        if(record->EventType==KEY_EVENT&&record->Event.KeyEvent.bKeyDown)
        {
          WORD Key=record->Event.KeyEvent.wVirtualKeyCode;
          wchar_t Char=FSF.LUpper(record->Event.KeyEvent.uChar.UnicodeChar);
          if((IsNone(record)&&Key==VK_RETURN)||(IsCtrl(record)&&Key==VK_RETURN)||(IsCtrl(record)&&Char=='\\'))
          {
            bool bRoot=(Char=='\\');
            if(ItemPresent||bRoot)
            {
              if(bRoot||(item.Item.Flags&LIF_CHECKED&&IsNone(record)))
              {
                TCHAR *ptr,old_dir[MAX_PATH];//,buffer[MAX_PATH];
                lstrcpy(old_dir,DlgParams->curr_dir);
                int Len=lstrlen(old_dir);
                if (bRoot)
                {
                  *newdir=0;
                  lstrcpy(newcurdir,_T(".."));
                }
                else
                {
                  GetFullPathName(newcurdir,ArraySize(newdir),newdir,&ptr);
                  FSF.AddEndSlash(newdir);
                }
                //MessageBox(NULL,old_dir,newcurdir,MB_OK);
                if (TryLoadDir(hDlg,DlgParams,newdir))
                {
                  if(!lstrcmp(newcurdir+(bRoot?0:Len),_T("..")))
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
                        ptr=(TCHAR*)FSF.PointToName(old_dir);
                      }
                      FarListPos Pos={sizeof(FarListPos)};
                      FarListFind find={sizeof(FarListFind),0,ptr,LIFIND_EXACTMATCH};
                      Pos.SelectPos=Info.SendDlgMessage(hDlg,DM_LISTFINDSTRING,0,&find);
                      Pos.TopPos=-1;
                      if(Pos.SelectPos>=0)
                        Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,&Pos);
                    }
                  }
                }
                return TRUE;
              }
              if(!(IsNone(record)&&Key==VK_RETURN))
              {
                Info.SendDlgMessage(hDlg,DM_CLOSE,0,0);
                return TRUE;
              }
              return FALSE;
            } else return TRUE;
          }
          else if(IsShift(record)&&Key==VK_RETURN)
          {
            if(ItemPresent)
            {
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
          else if(IsAlt(record)&&Char>='A'&&Char<='Z')
          {
            int Drive=Char-'A';
            DWORD Disks=GetLogicalDrives();
            if(Disks&(1<<Drive))
            {
              TCHAR temp_dir[3]=_T("A:"),*fileptr;
              temp_dir[0]=(TCHAR)(temp_dir[0]+Drive);
              if(!GetFullPathName(temp_dir,ArraySize(newdir),newdir,&fileptr))
                lstrcpy(newdir,temp_dir);
              FSF.AddEndSlash(newdir);
              TryLoadDir(hDlg,DlgParams,newdir);
            }
            return TRUE;
          }
          else if(IsRCtrl(record)&&Char>='0'&&Char<='9')
          {
            FarSettingsCreate settings={sizeof(FarSettingsCreate),FarGuid,INVALID_HANDLE_VALUE};
            HANDLE Settings=Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings)?settings.Handle:0;
            if(Settings)
            {
              FarSettingsEnum enums={sizeof(FarSettingsEnum),0,0,{0}};
              enums.Root=FSSF_FOLDERSHORTCUT_0+Char-'0';
              if(Info.SettingsControl(Settings,SCTL_ENUM,0,&enums)&&enums.Count==1)
              {
                lstrcpy(newdir,enums.Items[0].Name);
                FSF.AddEndSlash(newdir);
                TryLoadDir(hDlg,DlgParams,newdir);
              }
              Info.SettingsControl(Settings,SCTL_FREE,0,0);
            }
            return TRUE;
          }
          else if(IsNone(record)&&Key==VK_LEFT)
          {
            INPUT_RECORD keyOut={0};
            keyOut.EventType=KEY_EVENT;
            keyOut.Event.KeyEvent.bKeyDown=1;
            keyOut.Event.KeyEvent.wRepeatCount=1;
            keyOut.Event.KeyEvent.wVirtualKeyCode=VK_HOME;
            Info.SendDlgMessage(hDlg,DM_KEY,1,&keyOut);
            return TRUE;
          }
          else if(IsNone(record)&&Key==VK_RIGHT)
          {
            INPUT_RECORD keyOut={0};
            keyOut.EventType=KEY_EVENT;
            keyOut.Event.KeyEvent.bKeyDown=1;
            keyOut.Event.KeyEvent.wRepeatCount=1;
            keyOut.Event.KeyEvent.wVirtualKeyCode=VK_END;
            Info.SendDlgMessage(hDlg,DM_KEY,1,&keyOut);
            return TRUE;
          }
        }
      }
      break;
    case DN_CTLCOLORDIALOG:
      Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_MENUTEXT, Param2);
      return true;
    case DN_CTLCOLORDLGLIST:
      if(Param1==0)
      {
        FarDialogItemColors *fdic=(FarDialogItemColors *)Param2;
        FarColor *Colors = fdic->Colors;
        
        int ColorIndex[]={COL_MENUTEXT,COL_MENUTEXT,COL_MENUTITLE,COL_MENUTEXT,COL_MENUHIGHLIGHT,COL_MENUTEXT,COL_MENUSELECTEDTEXT,COL_MENUSELECTEDHIGHLIGHT,COL_MENUSCROLLBAR,COL_MENUDISABLEDTEXT};
        size_t Count=sizeof(ColorIndex)/sizeof(ColorIndex[0]);
        if(Count>fdic->ColorsCount) Count=fdic->ColorsCount;
        for(size_t i=0;i<Count;i++)
        {
          FarColor fc;
          if (Info.AdvControl(&MainGuid, ACTL_GETCOLOR, ColorIndex[i],&fc))
          {
            Colors[i] = fc;
          }
        }
        return TRUE;
      }
      break;
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_UPDATESIZE,0,0);
      TryLoadDir(hDlg,(OFDData *)Param2,((OFDData *)Param2)->curr_dir);
      break;
    case DN_HELP:
      Info.ShowHelp(Info.ModuleName,_T("FileDialog"),FHELP_SELFHELP);
      return (long)NULL;
    case DN_CLOSE:
      {
        if((Param1==0)&&ItemPresent)
        {
          lstrcpy(DlgParams->filename,DlgParams->curr_dir);
          if(lstrcmp(DlgParams->names[item.ItemIndex].Text,_T("..")))
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
      Info.SendDlgMessage(hDlg,DM_UPDATESIZE,0,0);
      break;
    case DM_UPDATESIZE:
      {
        const int minimal_width=65,minimal_height=12;
        int width=minimal_width,height=minimal_height;
        HANDLE console=CreateFile(_T("CONOUT$"),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
        if(console!=INVALID_HANDLE_VALUE)
        {
          COORD console_size={80,25};
          SMALL_RECT console_rect;
          if(Info.AdvControl(&MainGuid,ACTL_GETFARRECT,0,&console_rect))
          {
            console_size.X=console_rect.Right-console_rect.Left+1;
            console_size.Y=console_rect.Bottom-console_rect.Top+1;
          }
          width=console_size.X*3/5;
          if(width<minimal_width) width=minimal_width;
          height=console_size.Y*2/3;
          if(height<minimal_height) height=minimal_height;
          if(width>(console_size.X-11)) width=console_size.X-11;
          if(height>(console_size.Y-4)) height=console_size.Y-4;
          CloseHandle(console);
        }
        Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
        { //minimize listbox
          SMALL_RECT listbox_size={3,1,4,2};
          Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,0,&listbox_size);
        }
        { //resize and move dialog
          COORD dialog_size={(short)(width+6),(short)(height+2)};
          Info.SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,&dialog_size);
          COORD position={-1,-1};
          Info.SendDlgMessage(hDlg,DM_MOVEDIALOG,TRUE,&position);
        }
        { //resize listbox
          SMALL_RECT listbox_size={3,1,(short)(width+2),(short)(height)};
          Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,0,&listbox_size);
        }
        Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool open_file_dialog(const TCHAR *curr_dir,TCHAR *filename)
{
  FarDialogItem DialogItems[1];
  OFDData params;
  lstrcpy(params.curr_dir,curr_dir);
  params.filename = filename;
  params.names=NULL;
  params.result = false;
  ZeroMemory(DialogItems,sizeof(DialogItems));
  DialogItems[0].Type=DI_LISTBOX; //DialogItems[0].Flags=DIF_LISTWRAPMODE;
  CFarDialog dialog;
  dialog.Execute(MainGuid,FileDialogGuid,-1,-1,0,0,NULL,DialogItems,ArraySize(DialogItems),0,0,OFDProc,&params);
  return params.result;
}
