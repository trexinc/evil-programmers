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

#define INIT_DLG_DATA(item,str) item.Data=str
class CFarDialog
{
  private:
    HANDLE iDlg;
  public:
    inline CFarDialog(): iDlg(INVALID_HANDLE_VALUE) {};
    ~CFarDialog();
    int Execute(const GUID& PluginId,const GUID& Id,int X1,int Y1,int X2,int Y2,const TCHAR* HelpTopic,struct FarDialogItem* Item,int ItemsNumber,DWORD Reserved,DWORD Flags,FARWINDOWPROC DlgProc,void* Param);
    inline HANDLE Handle(void) {return iDlg;};
    int Check(int index);
    const wchar_t* Str(int index);
    inline DWORD Flags(int index)
    {
      FarDialogItem DialogItem;
      if(Info.SendDlgMessage(iDlg,DM_GETDLGITEMSHORT,index,&DialogItem)) return DialogItem.Flags;
      return 0;
    };
    DWORD Type(int index);
};

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
    CFarPanel(HANDLE aPlugin,FILE_CONTROL_COMMANDS aCommand);
    ~CFarPanel();
    inline bool IsOk(void) {return iResult;}
    inline int PanelType(void) {return iInfo.PanelType;};
    inline int Plugin(void) {return static_cast<bool>(iInfo.Flags&PFLAGS_PLUGIN);};
    inline int ItemsNumber(void) {return iInfo.ItemsNumber;};
    inline int SelectedItemsNumber(void) {return iInfo.SelectedItemsNumber;};
    inline int CurrentItem(void) {return iInfo.CurrentItem;};
    inline DWORD Flags(void) {return iInfo.Flags;};
    TCHAR* CurDir(void);
    PluginPanelItem& operator[](size_t index);
    PluginPanelItem& Selected(size_t index);
};

class CFarPanelSelection
{
  private:
    HANDLE iPlugin;
    PanelInfo iInfo;
    bool iSelection;
    PluginPanelItem* iItem;
    int iItemSize;
  private:
    CFarPanelSelection();
  public:
    CFarPanelSelection(HANDLE aPlugin,bool aSelection);
    ~CFarPanelSelection();
    inline int Number(void) {return iSelection?iInfo.SelectedItemsNumber:1;}
    PluginPanelItem& operator[](size_t index);
};

class CFarSettings
{
  private:
    HANDLE iSettings;
    size_t iRoot;
    CFarSettings();
  public:
    CFarSettings(const GUID& PluginId);
    ~CFarSettings();
    void Set(const wchar_t* aName,__int64 aValue);
    void Set(const wchar_t* aName,const wchar_t* aValue);
    void Get(const wchar_t* aName,__int64& aValue);
    void Get(const wchar_t* aName,wchar_t* aValue,size_t aSize);
};

#endif
