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

#ifdef UNICODE
typedef unsigned short UTCHAR;
#define t_FarKeyToName(Key,KeyText,Size) FSF.FarKeyToName(Key,KeyText,Size)
#define t_KEY_CHAR UnicodeChar
#define INIT_DLG_DATA(item,str) item.PtrData=str
#define DLG_DATA(item) item.PtrData
#define DLG_DATA_SPRINTF(item,val,mask) FSF.sprintf(val##Text,mask,val); item.PtrData=val##Text
#define DLG_DATA_FARKEYTONAME(item,val) t_FarKeyToName(val,val##Text,ArraySize(val##Text)); item.PtrData=val##Text
#define DLG_DATA_ITOA(item,val) FSF.itoa(val,val##Text,10); item.PtrData=val##Text
#define DLG_REFERENCE HANDLE
#define Dlg_GetCheck(handle,items,index) ((int)Info.SendDlgMessage(handle,DM_GETCHECK,index,0))
#define Dlg_GetStr(handle,items,index) ((const wchar_t*)Info.SendDlgMessage(handle,DM_GETCONSTTEXTPTR,index,0))
class CFarDialog
{
  private:
    HANDLE iDlg;
  public:
    inline CFarDialog(): iDlg(INVALID_HANDLE_VALUE) {};
    inline ~CFarDialog() {if(iDlg!=INVALID_HANDLE_VALUE) Info.DialogFree(iDlg);};
    inline int Execute(INT_PTR PluginNumber,int X1,int Y1,int X2,int Y2,const TCHAR* HelpTopic,struct FarDialogItem* Item,int ItemsNumber,DWORD Reserved,DWORD Flags,FARWINDOWPROC DlgProc,LONG_PTR Param)
    {
      iDlg=Info.DialogInit(PluginNumber,X1,Y1,X2,Y2,HelpTopic,Item,ItemsNumber,Reserved,Flags,DlgProc,Param);
      return (iDlg!=INVALID_HANDLE_VALUE)?Info.DialogRun(iDlg):-1;
    };
    inline HANDLE Handle(void) {return iDlg;};
};
#define EXP_NAME(p) _export p ## W
#else
typedef unsigned char UTCHAR;
#define t_FarKeyToName(Key,KeyText,Size) FSF.FarKeyToName(Key,KeyText,Size-1)
#define t_KEY_CHAR AsciiChar
#define INIT_DLG_DATA(item,str) _tcscpy(item.Data,str)
#define DLG_DATA(item) item.Data
#define DLG_DATA_SPRINTF(item,val,mask) FSF.sprintf(item.Data,mask,val)
#define DLG_DATA_FARKEYTONAME(item,val) t_FarKeyToName(val,item.Data,ArraySize(item.Data));
#define DLG_DATA_ITOA(item,val) FSF.itoa(val,item.Data,10);
#define DLG_REFERENCE FarDialogItem*
#define Dlg_GetCheck(handle,items,index) items[index].Selected
#define Dlg_GetStr(handle,items,index) items[index].Data
class CFarDialog
{
  private:
  public:
    inline CFarDialog() {};
    inline int Execute(INT_PTR PluginNumber,int X1,int Y1,int X2,int Y2,const TCHAR* HelpTopic,struct FarDialogItem* Item,int ItemsNumber,DWORD Reserved,DWORD Flags,FARWINDOWPROC DlgProc,LONG_PTR Param)
    {
      return Info.DialogEx(PluginNumber,X1,Y1,X2,Y2,HelpTopic,Item,ItemsNumber,Reserved,Flags,DlgProc,Param);
    };
};
#define EXP_NAME(p) _export p
#endif

#endif
