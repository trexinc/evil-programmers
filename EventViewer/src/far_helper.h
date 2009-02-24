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

#include <tchar.h>
#include "plugin.hpp"
extern PluginStartupInfo Info;
#define ArraySize(a) (sizeof(a)/sizeof(a[0]))

#ifdef UNICODE
#define INIT_DLG_DATA(item,str) item.PtrData=str
#define INIT_MENU_TEXT(index,str) MenuItems[index].Text=str
class CFarDialog
{
  private:
    HANDLE iDlg;
  public:
    inline CFarDialog(): iDlg(INVALID_HANDLE_VALUE) {};
    inline ~CFarDialog() {Info.DialogFree(iDlg);};
    inline int Execute(INT_PTR PluginNumber,int X1,int Y1,int X2,int Y2,const TCHAR* HelpTopic,struct FarDialogItem* Item,int ItemsNumber,DWORD Reserved,DWORD Flags,FARWINDOWPROC DlgProc,LONG_PTR Param)
    {
      iDlg=Info.DialogInit(PluginNumber,X1,Y1,X2,Y2,HelpTopic,Item,ItemsNumber,Reserved,Flags,DlgProc,Param);
      return Info.DialogRun(iDlg);
    };
    inline HANDLE Handle(void) {return iDlg;};
    inline int Check(int index) {return (int)Info.SendDlgMessage(iDlg,DM_GETCHECK,index,0);};
    inline const wchar_t* Str(int index) {return (const wchar_t*)Info.SendDlgMessage(iDlg,DM_GETCONSTTEXTPTR,index,0);};
    inline DWORD Flags(int index)
    {
      FarDialogItem DialogItem;
      if(Info.SendDlgMessage(iDlg,DM_GETDLGITEMSHORT,index,(LONG_PTR)&DialogItem)) return DialogItem.Flags;
      return 0;
    };
    inline DWORD Type(int index)
    {
      FarDialogItem DialogItem;
      if(Info.SendDlgMessage(iDlg,DM_GETDLGITEMSHORT,index,(LONG_PTR)&DialogItem)) return DialogItem.Type;
      return 0;
    };
};
#define EXP_NAME(p) _export p ## W
#define EXP_NAME_CALL(p) p ## W
#define t_CharToOem(s,d) wcscpy(d,s)
#define t_OemToChar(s,d) wcscpy(d,s)
typedef wchar_t UTCHAR;
#define PANEL_FILENAME lpwszFileName
#define SECOND_PARAM LONG_PTR
#define ControlShort(a,b,c) Control(a,b,0,c)
#define ControlShort2(a,b,c) Control(a,b,c,0)
#define FCTL_GETPANELSHORTINFO FCTL_GETPANELINFO
#define BOX1 0x2591
#define BOX2 0x2588
#else
#define DM_GETDLGITEMSHORT DM_GETDLGITEM
#define DM_SETDLGITEMSHORT DM_SETDLGITEM
#define INIT_DLG_DATA(item,str) _tcscpy(item.Data,str)
#define INIT_MENU_TEXT(index,str) _tcscpy(MenuItems[index].Text,str)
class CFarDialog
{
  private:
    FarDialogItem* iItems;
  public:
    inline CFarDialog() {};
    inline int Execute(INT_PTR PluginNumber,int X1,int Y1,int X2,int Y2,const TCHAR* HelpTopic,struct FarDialogItem* Item,int ItemsNumber,DWORD Reserved,DWORD Flags,FARWINDOWPROC DlgProc,LONG_PTR Param)
    {
      iItems=Item;
      return Info.DialogEx(PluginNumber,X1,Y1,X2,Y2,HelpTopic,Item,ItemsNumber,Reserved,Flags,DlgProc,Param);
    };
    inline FarDialogItem* Handle(void) {return iItems;};
    inline int Check(int index) {return iItems[index].Selected;};
    inline const char* Str(int index) {return iItems[index].Data;};
    inline DWORD Flags(int index) {return iItems[index].Flags;};
    inline DWORD Type(int index) {return iItems[index].Type;};
};
#define EXP_NAME(p) _export p
#define EXP_NAME_CALL(p) p
#define t_CharToOem(s,d) CharToOem(s,d)
#define t_OemToChar(s,d) OemToChar(s,d)
typedef unsigned char UTCHAR;
#define PANEL_FILENAME cFileName
#define SECOND_PARAM void*
#define ControlShort(a,b,c) Control(a,b,c)
#define ControlShort2(a,b,c) Control(a,b,&c)
#define BOX1 '°'
#define BOX2 'Û'
#endif

#ifdef UNICODE
void Realloc(TCHAR*& aData,int& aLength,int aNewLength);
void Realloc(PluginPanelItem*& aData,int& aSize,int aNewSize);

class CFarPanel
{
  private:
    HANDLE iPlugin;
    PanelInfo iInfo;
    int iResult;
    TCHAR* iCurDir;
    int iCurDirSize;
    PluginPanelItem* iItem;
    int iItemSize;
  private:
    CFarPanel();
  public:
    inline CFarPanel(HANDLE aPlugin,int aCommand): iPlugin(aPlugin),iCurDir(NULL),iCurDirSize(0),iItem(NULL),iItemSize(0) {iResult=Info.Control(aPlugin,aCommand,0,(LONG_PTR)&iInfo);};
    ~CFarPanel();
    inline bool IsOk(void) {return iResult;}
    inline int PanelType(void) {return iInfo.PanelType;};
    inline int Plugin(void) {return iInfo.Plugin;};
    inline int ItemsNumber(void) {return iInfo.ItemsNumber;};
    inline int SelectedItemsNumber(void) {return iInfo.SelectedItemsNumber;};
    inline int CurrentItem(void) {return iInfo.CurrentItem;};
    inline DWORD Flags(void) {return iInfo.Flags;};
    TCHAR* CurDir(void);
    PluginPanelItem& operator[](size_t index);
    PluginPanelItem& Selected(size_t index);
    inline void RemoveSelection(size_t index) {Info.Control(iPlugin,FCTL_SETSELECTION,index,0);};
    inline void CommitSelection(void) {};
};
#else
class CFarPanel
{
  private:
    HANDLE iPlugin;
    PanelInfo iInfo;
    int iResult;
  private:
    CFarPanel();
  public:
    inline CFarPanel(HANDLE aPlugin,int aCommand): iPlugin(aPlugin) {iResult=Info.Control(aPlugin,aCommand,(void*)&iInfo);};
    inline bool IsOk(void) {return iResult;};
    inline int PanelType(void) {return iInfo.PanelType;};
    inline int Plugin(void) {return iInfo.Plugin;};
    inline int ItemsNumber(void) {return iInfo.ItemsNumber;};
    inline int SelectedItemsNumber(void) {return iInfo.SelectedItemsNumber;};
    inline int CurrentItem(void) {return iInfo.CurrentItem;};
    inline DWORD Flags(void) {return iInfo.Flags;};
    inline TCHAR* CurDir(void) {return iInfo.CurDir;};
    inline PluginPanelItem& operator[](size_t index) {return iInfo.PanelItems[index];};
    inline PluginPanelItem& Selected(size_t index) {return iInfo.SelectedItems[index];};
    inline void RemoveSelection(size_t index) {iInfo.PanelItems[index].Flags&=~PPIF_SELECTED;};
    inline void CommitSelection(void) {Info.Control(iPlugin,FCTL_SETSELECTION,&iInfo);};
};
#endif

#endif
