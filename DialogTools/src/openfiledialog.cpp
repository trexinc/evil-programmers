/*
    openfiledialog.cpp
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

HKEY OpenRegKey(HKEY hRoot, const TCHAR *root, const TCHAR *Key)
{
  HKEY hKey;
  TCHAR FullKeyName[512];
  FSF.sprintf(FullKeyName,_T("%s%s%s"),root,*Key ? _T("\\"):_T(""),Key);
  if (RegOpenKeyEx(hRoot,FullKeyName,0,KEY_READ,&hKey)!=ERROR_SUCCESS)
    return(NULL);
  return(hKey);
}

int GetRegKey(HKEY hRoot,const TCHAR * root, const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData,const TCHAR *Default,DWORD DataSize)
{
  HKEY hKey=OpenRegKey(hRoot,root, Key);
  DWORD Type;
  int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(BYTE*)ValueData,&DataSize);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    lstrcpy(ValueData,Default);
    return(FALSE);
  }
  return(TRUE);
}

void DoOpenFile(HANDLE aDlg)
{
  LONG_PTR itemID=Info.SendDlgMessage(aDlg,DM_GETFOCUS,0,0);
  FarDialogItem DialogItem;
  Info.SendDlgMessage(aDlg,DM_GETDLGITEMSHORT,itemID,(LONG_PTR)&DialogItem);
  if(DialogItem.Type==DI_EDIT)
  {
    TCHAR path[MAX_PATH],filename[MAX_PATH];
    path[0]=0;
#ifdef UNICODE
    if(Info.Control(PANEL_ACTIVE,FCTL_GETCURRENTDIRECTORY,ArraySize(path),(LONG_PTR)path))
#else
    if(GetCurrentDirectory(sizeof(path),path))
#endif
      FSF.AddEndSlash(path);
    if(open_file_dialog(path,filename))
    {
      Info.SendDlgMessage(aDlg,DM_SETTEXTPTR,itemID,(long)filename);
      COORD Pos={0,0};
      Info.SendDlgMessage(aDlg,DM_SETCURSORPOS,itemID,(LONG_PTR)&Pos);
      Pos.X=(short)lstrlen(filename);
      Info.SendDlgMessage(aDlg,DM_SETCURSORPOS,itemID,(LONG_PTR)&Pos);
    }
  }
}
