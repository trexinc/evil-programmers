/*
    Macro plugin for DialogManager
    Copyright (C) 2004 Vadim Yegorov
    Copyright (C) 2005 Vadim Yegorov and Alex Yaroslavsky

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
#include "macro.hpp"
#include "dialogs_macro.hpp"
#include "dm_macro.hpp"

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void)hDll;
  (void)dwReason;
  (void)lpReserved;
  return TRUE;
}

PluginStartupInfo FarInfo;
FARSTANDARDFUNCTIONS FSF;
DialogPluginStartupInfo DialogInfo;
char PluginRootKey[100];
dialogs_macro *macros=NULL;
dialogs_macro *macros_global=NULL;
static bool InMacro=false;

static char *GetMsg(int MsgNum,char *Str)
{
  DialogInfo.GetMsg(DialogInfo.MessageName,MsgNum,Str);
  return Str;
}

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct DialogPluginStartupInfo *DialogInfo)
{
  ::FarInfo=*FarInfo;
  FSF=*FarInfo->FSF;
  ::DialogInfo=*DialogInfo;
  FSF.sprintf(PluginRootKey,"%s\\Macro",::DialogInfo.RootKey);
  HKEY hKey=NULL;
  if(RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
  {
    DWORD Type,DataSize=0;
    char global_name[1024];
    strcpy(global_name,"@Global");
    DataSize=sizeof(global_name);
    RegQueryValueEx(hKey,"Global",0,&Type,(LPBYTE)&global_name,&DataSize);
    char NameBuffer[1024];
    DWORD j;
    LONG Result;
    for(j=0;;j++)
    {
      Result=RegEnumKey(hKey,j,NameBuffer,sizeof(NameBuffer));
      if(Result==ERROR_NO_MORE_ITEMS)
        break;
      if(Result==ERROR_SUCCESS)
      {
        char path[2048];
        strcpy(path,PluginRootKey);
        strcat(path,"\\");
        strcat(path,NameBuffer);
        if(!strcmp(NameBuffer,global_name))
        {
          if(!macros_global)
          {
            macros_global=new dialogs_macro(path,true,macros);
          }
        }
        else
        {
          dialogs_macro *new_macros=new dialogs_macro(path,false,macros);
          if(new_macros) macros=new_macros;
        }
      }
      else break;
    }
    RegCloseKey(hKey);
  }
  return 0;
}

void WINAPI _export Exit(void)
{
}

int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData)
{
  (void)OutData;
  switch(Msg)
  {
    case FMMSG_GETINFO:
      {
        GetInfoOutData *data=(GetInfoOutData *)OutData;
        GetMsg(mName,data->MenuString);
        strcpy(data->HotkeyID,"zg_macro");
        data->Flags=FMMSG_FILTER;
      }
      return TRUE;
    case FMMSG_FILTER:
      {
        {
          FilterInData *dlg=(FilterInData *)InData;
          if(dlg->Msg==DN_KEY&&!InMacro)
          {
            if(macros_global&&macros_global->check(dlg->hDlg,dlg->Param1,dlg->Param2))
            {
              InMacro=true;
              *(dlg->Result)=TRUE;
            }
            else if(macros&&!FarInfo.SendDlgMessage(dlg->hDlg,DN_MACRO_DISABLE,0,0))
            {
              long length=0; char *title=NULL;
              MacroDialogInfo mdi; mdi.StructSize=sizeof(mdi);
              if(FarInfo.SendDlgMessage(dlg->hDlg,DN_MACRO_GETDIALOGINFO,0,(long)&mdi))
              {
                length=strlen(mdi.DialogId);
                if(length) title=new char[length+1];
                if(title) strcpy(title,mdi.DialogId);
              }
              else
              {
                length=FarInfo.SendDlgMessage(dlg->hDlg,DM_GETTEXTLENGTH,0,0);
                /*if(length)*/ title=new char[length+1];
                if(title)
                {
                  *title=0;
                  FarInfo.SendDlgMessage(dlg->hDlg,DM_GETTEXTPTR,0,(long)title);
                }
              }

              if(title)
              {
                long item_count=0;
                for(FarDialogItem DialogItem;FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,item_count,(long)&DialogItem);item_count++) ;
                dialogs_macro *curr_macros=macros;
                while(curr_macros)
                {
                  if(FarInfo.CmpName((const char *)(const unsigned char *)curr_macros->title,title,FALSE)&&(!curr_macros->dialog_item_count||!item_count||curr_macros->dialog_item_count==item_count))
                  {
                    if(curr_macros->check(dlg->hDlg,dlg->Param1,dlg->Param2))
                    {
                      InMacro=true;
                      *(dlg->Result)=TRUE;
                      break;
                    }
                  }
                  curr_macros=curr_macros->next;
                }
                delete [] title;
              }

            }
          }
          else if(dlg->Msg==DN_ENTERIDLE) InMacro=false;
        }
      }
      return TRUE;
  }
  return FALSE;
}
