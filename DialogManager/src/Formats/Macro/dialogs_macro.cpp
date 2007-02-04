/*
    Macro plugin for DialogManager
    Copyright (C) 2004 Vadim Yegorov

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
#include "dialogs_macro.hpp"
#include "macro.hpp"

static long upper_key(long Key)
{
  long result=Key;
  if(FSF.LIsAlpha((unsigned long)Key)) result=FSF.LUpper((unsigned long)Key);
  return result;
}

dialogs_macro::dialogs_macro(const char *registry_key,bool new_global,dialogs_macro *new_next)
{
  next=new_next;
  global=new_global;
  HKEY hKey=NULL;
  if(RegOpenKeyEx(HKEY_CURRENT_USER,registry_key,0,KEY_READ,&hKey)==ERROR_SUCCESS)
  {
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
        HKEY hKey_macro=NULL;
        if(RegOpenKeyEx(hKey,NameBuffer,0,KEY_READ,&hKey_macro)==ERROR_SUCCESS)
        {
          DWORD Type,DataSize=0;
          char key[1024],sequence[1024]; DWORD disable_output=TRUE,flags=0,selection=0,noselection=0; int item_type=-1;
          strcpy(key,NameBuffer); strcpy(sequence,"");
          DataSize=sizeof(key);
          RegQueryValueEx(hKey_macro,"Key",0,&Type,(LPBYTE)key,&DataSize);
          DataSize=sizeof(sequence);
          RegQueryValueEx(hKey_macro,"Sequence",0,&Type,(LPBYTE)sequence,&DataSize);
          DataSize=sizeof(disable_output);
          RegQueryValueEx(hKey_macro,"DisableOutput",0,&Type,(LPBYTE)&disable_output,&DataSize);
          DataSize=sizeof(selection);
          RegQueryValueEx(hKey_macro,"Selection",0,&Type,(LPBYTE)&selection,&DataSize);
          DataSize=sizeof(noselection);
          RegQueryValueEx(hKey_macro,"NoSelection",0,&Type,(LPBYTE)&noselection,&DataSize);
          DataSize=sizeof(item_type);
          RegQueryValueEx(hKey_macro,"Type",0,&Type,(LPBYTE)&item_type,&DataSize);
          LONG key_code=FSF.FarNameToKey(key);
          if(key_code>-1)
          {
            key_code=upper_key(key_code);
            if(disable_output) flags|=KSFLAGS_DISABLEOUTPUT;
            avl_macro_data *Add=new avl_macro_data(key_code,(const unsigned char *)sequence,flags,selection,noselection,item_type);
            macros.insert(Add);
          }
          RegCloseKey(hKey_macro);
        }
      }
      else break;
    }
    //read title
    if(!global)
    {
      DWORD Type,DataSize=0;
      char title_in[1024];
      strcpy(title_in,"");
      dialog_item_count=0;
      DataSize=sizeof(title_in);
      RegQueryValueEx(hKey,"Title",0,&Type,(LPBYTE)title_in,&DataSize);
      title=(unsigned char *)title_in;
      DataSize=sizeof(dialog_item_count);
      RegQueryValueEx(hKey,"Count",0,&Type,(LPBYTE)&dialog_item_count,&DataSize);
    }
    RegCloseKey(hKey);
  }
}

bool dialogs_macro::check(HANDLE hDlg,int curr_item,long key)
{
  int curr_selection=MACRO_IGNORE_SELECTION,curr_type=-1;
  FarDialogItem DialogItem;
  if(FarInfo.SendDlgMessage(hDlg,DM_GETDLGITEM,curr_item,(long)&DialogItem)) curr_type=DialogItem.Type;
  avl_macro_data *data=macros.query(upper_key(key),curr_selection,curr_type);
  if(data)
  {
/*    if(data->type>-1)
    {
      FarDialogItem DialogItem;
      FarInfo.SendDlgMessage(hDlg,DM_GETDLGITEM,curr_item,(long)&DialogItem);
      if(DialogItem.Type!=data->type) return false;
      if((DialogItem.Type==DI_COMBOBOX||DialogItem.Type==DI_EDIT||DialogItem.Type==DI_FIXEDIT||DialogItem.Type==DI_PSWEDIT)&&data->selection!=MACRO_IGNORE_SELECTION)
      {
        EditorSelect es;
        if(FarInfo.SendDlgMessage(hDlg,DM_GETSELECTION,curr_item,(long)&es))
        {
          if(es.BlockType==BTYPE_STREAM&&es.BlockStartPos>=0)
          {
            if(data->selection==MACRO_NO_SELECTION) return false;
          }
          else
            if(data->selection==MACRO_SELECTION) return false;
        }
      }
    }*/
    ActlKeyMacro seq;
    seq.Command=MCMD_POSTMACROSTRING;
    seq.Param.PlainText.SequenceText=(char *)data->sequence.get();
    seq.Param.PlainText.Flags=data->flags;
    FarInfo.AdvControl(FarInfo.ModuleNumber,ACTL_KEYMACRO,&seq);
    return true;
  }
  return false;
}
