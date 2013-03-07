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

class CFarDialog
{
  private:
    HANDLE iDlg;
  public:
    inline CFarDialog(): iDlg(INVALID_HANDLE_VALUE) {};
    inline ~CFarDialog() {Info.DialogFree(iDlg);};
    inline intptr_t Execute(const GUID& PluginId,const GUID& Id,intptr_t X1,intptr_t Y1,intptr_t X2,intptr_t Y2,const wchar_t* HelpTopic,struct FarDialogItem* Item,size_t ItemsNumber,intptr_t Reserved,FARDIALOGFLAGS Flags,FARWINDOWPROC DlgProc,void* Param)
    {
      iDlg=Info.DialogInit(&PluginId,&Id,X1,Y1,X2,Y2,HelpTopic,Item,ItemsNumber,Reserved,Flags,DlgProc,Param);
      return Info.DialogRun(iDlg);
    };
    inline HANDLE Handle(void) {return iDlg;};
    inline int Check(int index) {return (int)Info.SendDlgMessage(iDlg,DM_GETCHECK,index,0);};
    inline const wchar_t* Str(int index) {return (const wchar_t*)Info.SendDlgMessage(iDlg,DM_GETCONSTTEXTPTR,index,0);};
    inline FARDIALOGITEMFLAGS Flags(int index)
    {
      FarDialogItem DialogItem;
      if(Info.SendDlgMessage(iDlg,DM_GETDLGITEMSHORT,index,&DialogItem)) return DialogItem.Flags;
      return DIF_NONE;
    };
    inline DWORD Type(int index)
    {
      FarDialogItem DialogItem;
      if(Info.SendDlgMessage(iDlg,DM_GETDLGITEMSHORT,index,&DialogItem)) return DialogItem.Type;
      return 0;
    };
};

#define BOX1 0x2591
#define BOX2 0x2588

void Realloc(wchar_t*& aData,int& aLength,int aNewLength);
void Realloc(PluginPanelItem*& aData,int& aSize,int aNewSize);

class CFarPanel
{
  private:
    HANDLE iPlugin;
    PanelInfo iInfo;
    intptr_t iResult;
    wchar_t* iCurDir;
    size_t iCurDirSize;
    PluginPanelItem* iItem;
    size_t iItemSize;
  private:
    CFarPanel();
  public:
    inline CFarPanel(HANDLE aPlugin,FILE_CONTROL_COMMANDS aCommand): iPlugin(aPlugin),iCurDir(NULL),iCurDirSize(0),iItem(NULL),iItemSize(0) {iInfo.StructSize=sizeof(iInfo);iResult=Info.PanelControl(aPlugin,aCommand,0,&iInfo);};
    ~CFarPanel();
    inline bool IsOk(void) {return !!iResult;}
    inline int PanelType(void) {return iInfo.PanelType;};
    inline int Plugin(void) {return iInfo.Flags&PFLAGS_PLUGIN;};
    inline size_t ItemsNumber(void) {return iInfo.ItemsNumber;};
    inline size_t SelectedItemsNumber(void) {return iInfo.SelectedItemsNumber;};
    inline size_t CurrentItem(void) {return iInfo.CurrentItem;};
    inline PANELINFOFLAGS Flags(void) {return iInfo.Flags;};
    wchar_t* CurDir(void);
    PluginPanelItem& operator[](size_t index);
    PluginPanelItem& Selected(size_t index);
    inline void StartSelection(void) {Info.PanelControl(iPlugin,FCTL_BEGINSELECTION,0,0);};
    inline void RemoveSelection(size_t index) {Info.PanelControl(iPlugin,FCTL_CLEARSELECTION,index,0);};
    inline void CommitSelection(void) {Info.PanelControl(iPlugin,FCTL_ENDSELECTION,0,0);};
};

#endif
