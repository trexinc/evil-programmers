/*
    bcopy_name.cpp
    Copyright (C) 2000-2009 zg

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
#include "far_helper.h"
#include "bcplugin.h"

struct Splited
{
  HANDLE Console;
  wchar_t Name[4][66];
};

static LONG_PTR WINAPI NameDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  Splited *DlgParams=(Splited *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_DRAWDLGITEM:
      if(Param1>0)
      {
        DWORD written; COORD coord;
        SMALL_RECT Rect;
        Info.SendDlgMessage(hDlg,DM_GETDLGRECT,0,(LONG_PTR)&Rect);
        coord.X=Rect.Left+((FarDialogItem *)Param2)->X1;
        coord.Y=Rect.Top+((FarDialogItem *)Param2)->Y1;
        wchar_t *string=DlgParams->Name[Param1-1];
        DWORD string_len=(DWORD)wcslen(string);
        WriteConsoleOutputCharacterW(DlgParams->Console,string,string_len,coord,&written);
      }
      break;
    case DN_ENTERIDLE:
      Info.SendDlgMessage(hDlg,DM_SETREDRAW,0,0);
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void ShowName(const wchar_t *Name)
{
  Splited Names;
  unsigned long length=(unsigned long)wcslen(Name);
  for(unsigned int i=0;i<sizeofa(Names.Name);i++) Names.Name[i][0]=0;
  for(unsigned int i=0;i<sizeofa(Names.Name);i++)
  {
    if(length<65)
    {
      memcpy(Names.Name[i],Name+i*65,sizeof(wchar_t)*length);
      Names.Name[i][length]=0;
      break;
    }
    else
    {
      memcpy(Names.Name[i],Name+i*65,sizeof(wchar_t)*65);
      Names.Name[i][65]=0;
      length-=65;
    }
  }
  FarDialogItem DialogItems[5];
  memset(DialogItems,0,sizeof(DialogItems));
  DialogItems[0].Type=DI_DOUBLEBOX; DialogItems[0].X1=3; DialogItems[0].Y1=1;
  DialogItems[0].X2=71; DialogItems[0].Y2=6;
  for(unsigned int i=1;i<sizeofa(DialogItems);i++)
  {
    DialogItems[i].Type=DI_TEXT; DialogItems[i].X1=5; DialogItems[i].Y1=1+i;
  }
  Names.Console=CreateFileW(L"CONOUT$",GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  if(Names.Console!=INVALID_HANDLE_VALUE)
  {
    CFarDialog dialog;
    dialog.Execute(Info.ModuleNumber,-1,-1,75,8,_T("ShowName"),DialogItems,sizeofa(DialogItems),0,0,NameDlgProc,(LONG_PTR)&Names);
    CloseHandle(Names.Console);
  }
}
