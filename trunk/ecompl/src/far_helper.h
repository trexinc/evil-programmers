/*
    far_helper.h
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

#ifndef __FAR_HELPER_H__
#define __FAR_HELPER_H__

#include "plugin.hpp"
extern PluginStartupInfo Info;
#define ArraySize(a) (sizeof(a)/sizeof(a[0]))

typedef unsigned short UTCHAR;
#define t_FarKeyToName(Key,KeyText,Size) FSF.FarKeyToName(Key,KeyText,Size)
#define t_KEY_CHAR UnicodeChar
#define INIT_DLG_DATA(item,str) item.Data=str
#define DLG_DATA(item) item.Data
#define DLG_DATA_SPRINTF(item,val,mask) FSF.sprintf(val##Text,mask,val); item.Data=val##Text
#define DLG_DATA_FARKEYTONAME(item,val) t_FarKeyToName(val,val##Text,ArraySize(val##Text)); item.Data=val##Text
#define DLG_DATA_ITOA(item,val) FSF.itoa(val,val##Text,10); item.Data=val##Text
class CFarDialog
{
  private:
    HANDLE iDlg;
  public:
    inline CFarDialog(): iDlg(INVALID_HANDLE_VALUE) {};
    inline ~CFarDialog() {Info.DialogFree(iDlg);};
    inline int Execute(const GUID& PluginId,const GUID& Id,int X1,int Y1,int X2,int Y2,const TCHAR* HelpTopic,struct FarDialogItem* Item,int ItemsNumber,DWORD Reserved,DWORD Flags,FARWINDOWPROC DlgProc,void* Param)
    {
      iDlg=Info.DialogInit(&PluginId,&Id,X1,Y1,X2,Y2,HelpTopic,Item,ItemsNumber,Reserved,Flags,DlgProc,Param);
      return Info.DialogRun(iDlg);
    };
    inline HANDLE Handle(void) {return iDlg;};
    inline int Check(int index) {return (int)Info.SendDlgMessage(iDlg,DM_GETCHECK,index,0);};
    inline const wchar_t* Str(int index) {return (const wchar_t*)Info.SendDlgMessage(iDlg,DM_GETCONSTTEXTPTR,index,0);};
    inline DWORD Flags(int index)
    {
      FarDialogItem DialogItem;
      if(Info.SendDlgMessage(iDlg,DM_GETDLGITEMSHORT,index,&DialogItem)) return DialogItem.Flags;
      return 0;
    };
};

#endif
