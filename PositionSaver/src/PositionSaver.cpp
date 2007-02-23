/*
    Position Saver plugin for FAR Manager
    Copyright (C) 2002-2005 Alex Yaroslavsky

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
#include "PositionSaver.hpp"
#include "memory.hpp"
#include "common.hpp"
#include "psitems.hpp"
#include "sgmlext.hpp"
#include "tree.hpp"
#include "registry.hpp"

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}
#endif

struct PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
FARAPIVIEWERCONTROL ViewerControl;
char PluginRootKey[80];
static const char EDITOR[] = "editor";
#ifdef ALLOW_VIEWER_API
static const char VIEWER[] = "viewer";
#endif
static const char SHOWINPANELS[] = "ShowInPanels";
static const char REMOVEOLD[] = "RemoveOld";
static const char XMLFILE[] = "CfgFile";
static const char AUTOMATIC[] = "Automatic";
static const char CHECKMASK[] = "CheckMask";
static const char MASK[] = "Mask";
#ifdef ALLOW_VIEWER_API
static const char VAUTOMATIC[] = "VAutomatic";
static const char VCHECKMASK[] = "VCheckMask";
static const char VMASK[] = "VMask";
#endif
static int ShowInPanels, RemoveOld;
static int EAutomatic, ECheckMask;
static char EMask[512];
#ifdef ALLOW_VIEWER_API
static int VAutomatic, VCheckMask;
static char VMask[512];
#endif
static char XMLFile[_MAX_PATH];
static char DefXMLFile[_MAX_PATH];
static PSItems XMLItems;
static PSTree Tree;
#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

void ConfigSpecific(const char *what);
void ConfigCommon(void);

enum
{
  MTitle,
  MMenuTrack,
  MMenuSave,
  MMenuRestore,
  MMenuEdit,
  MMenuRemoveOld,
  MMenuConfig,
  MListMenuCommands,
//  MDlgFile,
//  MDlgPosition,
  MCfgCommon,
  MCfgEditor,
  #ifdef ALLOW_VIEWER_API
  MCfgViewer,
  #endif
  MCfgShowInPanels,
  MCfgAutomatic,
  MCfgRemoveOld,
  MCfgCheckMask,
  MCfgXMLFile,
  MDeleteAsk,
  MNoItems,
  MNoMoreItems,
  MRemovedItems,
  MOk,
  MCancel,
};

void ReadRegistryCommon(void)
{
  GetRegKey(SHOWINPANELS,&ShowInPanels,0);
  GetRegKey(REMOVEOLD,&RemoveOld,1);
  GetRegKey(XMLFILE,XMLFile,DefXMLFile,_MAX_PATH);
  FSF.Unquote(XMLFile);
  if (!*XMLFile)
    lstrcpy(XMLFile,DefXMLFile);
}

void ReadRegistryEditor(void)
{
  GetRegKey(AUTOMATIC,&EAutomatic,1);
  GetRegKey(CHECKMASK,&ECheckMask,0);
  GetRegKey(MASK,EMask,"",512);
}

#ifdef ALLOW_VIEWER_API
void ReadRegistryViewer(void)
{
  GetRegKey(VAUTOMATIC,&VAutomatic,1);
  GetRegKey(VCHECKMASK,&VCheckMask,0);
  GetRegKey(VMASK,VMask,"",512);
}
#endif

void ReadRegistry(void)
{
  ReadRegistryCommon();
  ReadRegistryEditor();
  #ifdef ALLOW_VIEWER_API
  ReadRegistryViewer();
  #endif
}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *psi)
{
  Info=*psi;
  FSF=*psi->FSF;
  Info.FSF=&FSF;
  ViewerControl=(FARAPIVIEWERCONTROL)Info.Reserved[1];
  FSF.sprintf(PluginRootKey,"%s\\PositionSaver",Info.RootKey);
  lstrcpy(DefXMLFile,::Info.ModuleName);
  size_t i = lstrlen(DefXMLFile)-1;
  DefXMLFile[i]='g'; DefXMLFile[--i]='f'; DefXMLFile[--i]='c';
  ReadRegistry();
}

void WINAPI _export ExitFAR( void )
{
  if (RemoveOld)
    XMLItems.RemoveOld(&Tree,XMLFile);
}

enum
{
  PS_XMLREAD,
  PS_UPDATE,
  PS_SAVE,
  PS_RESTORE,
  PS_OPEN,
};

bool PosSave(int Action)
{
  struct EditorInfo ei;
  struct EditorSetPosition esp;

  Info.EditorControl(ECTL_GETINFO,&ei);

  switch (Action)
  {
    case PS_RESTORE:
      {
        /*PSInfo psi = {(char *)EDITOR,(char *)ei.FileName, &esp};
        if (XMLItems.GetItem(psi))
        {
          Info.EditorControl(ECTL_SETPOSITION,psi.esp);
          return true;
        }*/
        PSTrackInfo ti;
        ti.EditorID = ei.EditorID;
        ti.Type = TI_EDITOR;
        if (Tree.GetID(&ti))
        {
          Info.EditorControl(ECTL_SETPOSITION,&ti.esp);
          return true;
        }
      }
      return false;

    case PS_OPEN:
    case PS_UPDATE:
    case PS_SAVE:
      {
        esp.CurLine = ei.CurLine;
        esp.CurPos = ei.CurPos;
        esp.CurTabPos = -1;
        esp.TopScreenLine = ei.TopScreenLine;
        esp.LeftPos = ei.LeftPos;
        esp.Overtype = ei.Overtype;
        PSTrackInfo ti;
        ti.EditorID = ei.EditorID;
        ti.Type = TI_EDITOR;
        if (Action == PS_UPDATE)
        {
          if (!Tree.FindID(&ti) || !Tree.GetID(&ti))
            return false;
        }
        lstrcpy(ti.FileName,ei.FileName);
        ti.mod_flag = ei.CurState & ECSTATE_MODIFIED;
        ti.mod_flag &= !(ei.CurState & ECSTATE_SAVED);
        ti.esp = esp;
        (ti.mod_flag ? ti.mod = esp : ti.unmod = esp);
        if (Action == PS_SAVE)
          ti.unmod = esp;
        if (Action == PS_OPEN)
        {
          PSInfo psi = {(char *)EDITOR,(char *)ei.FileName,&ti.esp};
          if (!XMLItems.GetItem(psi))
            return false;
        }
        if (!Tree.Insert(&ti))
          return false;
        if (Action == PS_SAVE)
        {
          PSInfo psi = {(char *)EDITOR,(char *)ei.FileName, &esp};
          if (XMLItems.FindItem(psi))
          {
            if (!XMLItems.SetItem(psi))
            {
              Tree.Delete(&ti);
              return false;
            }
          }
          else
          {
            if (!XMLItems.NewItem(psi))
            {
              Tree.Delete(&ti);
              return false;
            }
          }
          XMLItems.SaveFile(XMLFile,&Tree,false);
        }
        /*if (XMLItems.SetItem(psi))
        {
          XMLItems.SaveFile(XMLFile,&Tree);
          return true;
        }
        if (Action == PS_TRACK)
        {
          if (XMLItems.NewItem(psi))
          {
            XMLItems.SaveFile(XMLFile,&Tree);
            return true;
          }
        }*/
      }
      return true;
    case PS_XMLREAD:
      {
        return XMLItems.LoadFile(XMLFile,&Tree);
      }
  }
  return false;
}

#ifdef ALLOW_VIEWER_API
bool ViewerPosSave(int Action)
{
  struct ViewerInfo vi;
  struct ViewerSetPosition vsp;

  vi.StructSize=sizeof(vi);
  ViewerControl(VCTL_GETINFO,&vi);

  switch (Action)
  {
    case PS_RESTORE:
      {
        PSTrackInfo ti;
        ti.ViewerID = vi.ViewerID;
        ti.Type = TI_VIEWER;
        if (Tree.GetID(&ti))
        {
          ti.vsp.StartPos.i64+=(vi.CurMode.Hex ? 0 : 1);
          ViewerControl(VCTL_SETPOSITION,&ti.vsp);
          return true;
        }
      }
      return false;

    case PS_OPEN:
    case PS_UPDATE:
    case PS_SAVE:
      {
        vsp.StartPos = vi.FilePos;
        vsp.LeftPos = vi.LeftPos;
        vsp.Flags = 0;
        PSTrackInfo ti;
        ti.ViewerID = vi.ViewerID;
        ti.Type = TI_VIEWER;
        if (Action == PS_UPDATE)
        {
          if (!Tree.FindID(&ti) || !Tree.GetID(&ti))
            return false;
        }
        lstrcpy(ti.FileName,vi.FileName);
        ti.mod_flag = 0;
        ti.vsp = vsp;
        if (Action == PS_OPEN)
        {
          PSInfo psi = {(char *)VIEWER,(char *)vi.FileName,(struct EditorSetPosition *)&ti.vsp};
          if (!XMLItems.GetItem(psi))
            return false;
        }
        if (!Tree.Insert(&ti))
          return false;
        if (Action == PS_SAVE)
        {
          PSInfo psi = {(char *)VIEWER,(char *)vi.FileName,(struct EditorSetPosition *)&vsp};
          if (XMLItems.FindItem(psi))
          {
            if (!XMLItems.SetItem(psi))
            {
              Tree.Delete(&ti);
              return false;
            }
          }
          else
          {
            if (!XMLItems.NewItem(psi))
            {
              Tree.Delete(&ti);
              return false;
            }
          }
          XMLItems.SaveFile(XMLFile,&Tree,false);
        }
      }
      return true;
    case PS_XMLREAD:
      {
        return XMLItems.LoadFile(XMLFile,&Tree);
      }
  }
  return false;
}
#endif

static void EditSavedItems(void)
{
  struct FarMenuItemEx *MenuItems, *MenuItem;
  int ItemsCount;
  typedef char TFileName[_MAX_PATH];
  TFileName *FileName;
  struct EditorSetPosition esp;
  int MenuCode;
  static const int BreakKeys[] =
  {
    //VK_RETURN,
    //VK_F4,
    VK_DELETE,
    0
  };
  int BreakCode = 0;
  int LastPos = 0;

/*enum
  {
    DLG_WIDTH = 77,
    DLG_HEIGHT = 11,
    DEF_BUTTON = 5,
    DATA_FILENAME = 2,
    DATA_POSITION = 3,
  };
  static const struct InitDialogItem InitDialogItems[] =
  {
    DI_DOUBLEBOX   ,3  ,1  ,73 ,9  ,0                  ,MTitle,
    DI_TEXT        ,5  ,3  ,0  ,0  ,0                  ,MDlgFile,
    DI_EDIT        ,5  ,4  ,71 ,0  ,0                  ,-1,
    DI_TEXT        ,5  ,5  ,0  ,0  ,0                  ,-1,
    DI_TEXT        ,0  ,7  ,0  ,0  ,DIF_SEPARATOR      ,-1,
    DI_BUTTON      ,0  ,8  ,0  ,0  ,DIF_CENTERGROUP    ,MOk,
    DI_BUTTON      ,0  ,8  ,0  ,0  ,DIF_CENTERGROUP    ,MCancel
  };*/
  //struct FarDialogItem DialogItems[sizeofa(InitDialogItems)];
  static const char *msg[2], *msg2[2];
  int i;

  //InitDialog(InitDialogItems,DialogItems,sizeofa(InitDialogItems));
  //DialogItems[DATA_FILENAME].Focus = TRUE;
  //DialogItems[DEF_BUTTON].DefaultButton = 1;

  msg[0] = msg2[0] = GetMsg(MTitle);
  msg[1] = GetMsg(MDeleteAsk);
  msg2[1] = GetMsg(MNoItems);

  XMLItems.LoadFile(XMLFile,&Tree);
  ItemsCount = XMLItems.CountItems(EDITOR);
  #ifdef ALLOW_VIEWER_API
  ItemsCount += XMLItems.CountItems(VIEWER);
  #endif

  if (!ItemsCount)
  {
    Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_MB_OK,NULL,msg2,2,0);
    return;
  }

  MenuItems = (struct FarMenuItemEx *)malloc(ItemsCount*sizeof(struct FarMenuItemEx));
  FileName = (TFileName *)malloc(sizeof(TFileName)*ItemsCount);
  if (!MenuItems || !FileName)
    return;

  PSgmlExt Item = XMLItems.Base()->next();
  for (i=0, MenuItem=MenuItems; i<ItemsCount; i++, MenuItem++,Item = Item->next())
  {
    while (Item)
    {
      if (Item->getname() && !FSF.LStricmp(Item->getname(),"item") && Item->GetChrParam("path") && Item->GetChrParam("type")
          &&
          (
           !FSF.LStricmp(Item->GetChrParam("type"),EDITOR)
           #ifdef ALLOW_VIEWER_API
           || !FSF.LStricmp(Item->GetChrParam("type"),VIEWER)
           #endif
          )
         )
        break;
      Item = Item->next();
    }
    if (!Item) break;
    lstrcpy(FileName[i],Item->GetChrParam("path"));
    char temp[_MAX_PATH];
    lstrcpy(temp,FileName[i]);
    FSF.TruncPathStr(temp,62);
    char type[20];
    lstrcpy(type,Item->GetChrParam("type"));
    FSF.LStrlwr(type);
    FSF.sprintf(MenuItem->Text.Text,"%s: %s",type,temp);
    #ifdef ALLOW_VIEWER_API
    if (!lstrcmp(EDITOR,type))
      MenuItem->UserData = TI_EDITOR;
    else
      MenuItem->UserData = TI_VIEWER;
    #else
    MenuItem->UserData = TI_EDITOR;
    #endif
  }
  ItemsCount = i;

  MenuItems[0].Flags = MIF_SELECTED;

  msg2[1] = GetMsg(MNoMoreItems);

  while (BreakCode < sizeofa(BreakKeys))
  {
    if (!ItemsCount)
    {
      Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_MB_OK,NULL,msg2,2,0);
      break;
    }
    MenuCode = Info.Menu(Info.ModuleNumber,-1,-1,0,
                         FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT|FMENU_USEEXT,
                         GetMsg(MTitle),GetMsg(MListMenuCommands),NULL,BreakKeys,&BreakCode,
                         (struct FarMenuItem *)MenuItems,ItemsCount);
    if (MenuCode == -1)
      break;
    MenuItems[LastPos].Flags = 0;
    switch (BreakCode)
    {
      /*
      case 0:
      case 1:
        lstrcpy(DialogItems[DATA_FILENAME].Data,FileName[MenuCode]);
        PSInfo info = {(char *)EDITOR, FileName[MenuCode], &esp};
        XMLItems.GetItem(info);
        FSF.sprintf(DialogItems[DATA_POSITION].Data,GetMsg(MDlgPosition),esp.CurLine,esp.CurPos);
        i = Info.Dialog(Info.ModuleNumber,-1,-1,DLG_WIDTH,DLG_HEIGHT,NULL,(struct FarDialogItem *)&DialogItems,sizeofa(DialogItems));
        if (i == DEF_BUTTON)
        {
          FSF.Unquote(DialogItems[DATA_FILENAME].Data);
          if (FSF.LStricmp(DialogItems[DATA_FILENAME].Data,FileName[MenuCode]))
          {
            info.FileName = DialogItems[DATA_FILENAME].Data;
            XMLItems.SelItem(info);
          }
          lstrcpy(FileName[MenuCode],DialogItems[DATA_FILENAME].Data);
          lstrcpy(MenuItems[MenuCode].Text,FSF.TruncPathStr(DialogItems[DATA_FILENAME].Data,71));
        }
        MenuItems[LastPos=MenuCode].Selected = TRUE;
        break;
      */
      //case 2:
      case 0:
        {
          if (Info.Message(Info.ModuleNumber,FMSG_MB_YESNO,NULL,msg,2,0) != 0)
            break;
          XMLItems.LoadFile(XMLFile,&Tree);
          ItemsCount--;
          PSInfo info = {NULL, FileName[MenuCode], NULL};
          PSTrackInfo ti;
          lstrcpy(ti.FileName,FileName[MenuCode]);
          ti.Type = MenuItems[MenuCode].UserData;
          #ifdef ALLOW_VIEWER_API
          if (ti.Type == TI_EDITOR)
            info.Type = (char *)EDITOR;
          else
            info.Type = (char *)VIEWER;
          #else
          info.Type = (char *)EDITOR;
          #endif
          XMLItems.DelItem(info);
          if (Tree.GetFN(&ti))
            Tree.Delete(&ti);
          XMLItems.SaveFile(XMLFile,&Tree,false);
          for (i=MenuCode, MenuItem=MenuItems+MenuCode; i<ItemsCount; i++, MenuItem++)
          {
            *MenuItem = *(MenuItem+1);
            lstrcpy(FileName[i],FileName[i+1]);
          }
          if (ItemsCount && MenuCode < ItemsCount)
            MenuItems[LastPos=MenuCode].Flags = MIF_SELECTED;
          else if (ItemsCount && MenuCode > 0)
            MenuItems[LastPos=(MenuCode-1)].Flags = MIF_SELECTED;
          break;
        }
    }
    continue;
  }
  if (MenuItems)
    free(MenuItems);
}

void Config(void)
{
  #ifdef ALLOW_VIEWER_API
  FarMenuItem Menu[3];
  #else
  FarMenuItem Menu[2];
  #endif
  memset(Menu,0,sizeof(Menu));
  lstrcpy(Menu[0].Text,GetMsg(MCfgCommon));
  lstrcpy(Menu[1].Text,GetMsg(MCfgEditor));
  #ifdef ALLOW_VIEWER_API
  lstrcpy(Menu[2].Text,GetMsg(MCfgViewer));
  #endif
  int selected = 0, code;
  do
  {
    Menu[selected].Selected = TRUE;
    code = Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT,GetMsg(MTitle),NULL,NULL,NULL,NULL,Menu,sizeofa(Menu));
    switch (code)
    {
      case 0:
        ConfigCommon();
        break;
      case 1:
        ConfigSpecific(EDITOR);
        break;
      #ifdef ALLOW_VIEWER_API
      case 2:
        ConfigSpecific(VIEWER);
        break;
      #endif
    }
    Menu[selected].Selected = FALSE;
    selected = code;
  } while (code != -1);
}

void ConfigCommon(void)
{
  enum
  {
    DLG_WIDTH = 64,
    DLG_HEIGHT = 11,
    DEF_BUTTON = 7,
  };
  static const struct InitDialogItem InitDialogItems[] =
  {
    {DI_DOUBLEBOX  ,3  ,1  ,60 ,9  ,0               ,MCfgCommon},
    {DI_CHECKBOX   ,5  ,2  ,0  ,0  ,0               ,MCfgShowInPanels},
    {DI_CHECKBOX   ,5  ,3  ,0  ,0  ,0               ,MCfgRemoveOld},
    {DI_TEXT       ,0  ,4  ,0  ,0  ,DIF_SEPARATOR   ,-1},
    {DI_TEXT       ,5  ,5  ,0  ,0  ,0               ,MCfgXMLFile},
    {DI_EDIT       ,5  ,6  ,58 ,0  ,0               ,-1},
    {DI_TEXT       ,0  ,7  ,0  ,0  ,DIF_SEPARATOR   ,-1},
    {DI_BUTTON     ,0  ,8  ,0  ,0  ,DIF_CENTERGROUP ,MOk},
    {DI_BUTTON     ,0  ,8  ,0  ,0  ,DIF_CENTERGROUP ,MCancel},
  };
  struct FarDialogItem DialogItems[sizeofa(InitDialogItems)];

  ReadRegistryCommon();
  XMLItems.LoadFile(XMLFile,&Tree);
  InitDialog(InitDialogItems,DialogItems,sizeofa(InitDialogItems));
  DialogItems[1].Focus = TRUE;
  DialogItems[1].Selected = ShowInPanels;
  DialogItems[2].Selected = RemoveOld;
  lstrcpy(DialogItems[5].Data,XMLFile);
  DialogItems[DEF_BUTTON].DefaultButton = 1;

  if (Info.Dialog(Info.ModuleNumber,-1,-1,DLG_WIDTH,DLG_HEIGHT,NULL,(struct FarDialogItem *)&DialogItems,sizeofa(DialogItems)) != DEF_BUTTON)
    return;

  ShowInPanels = DialogItems[1].Selected;
  RemoveOld = DialogItems[2].Selected;
  lstrcpy(XMLFile,DialogItems[5].Data);
  FSF.Unquote(XMLFile);
  if (!*XMLFile)
    lstrcpy(XMLFile,DefXMLFile);
  SetRegKey(SHOWINPANELS,ShowInPanels);
  SetRegKey(REMOVEOLD,RemoveOld);
  SetRegKey(XMLFILE,XMLFile);
  XMLItems.SaveFile(XMLFile,&Tree);
}

void ConfigSpecific(const char *what)
{
  enum
  {
    DLG_WIDTH = 64,
    DLG_HEIGHT = 8,
    DEF_BUTTON = 5,
  };
  static const struct InitDialogItem InitDialogItems[] =
  {
    {DI_DOUBLEBOX  ,3  ,1  ,60 ,6  ,0               ,-1},
    {DI_CHECKBOX   ,5  ,2  ,0  ,0  ,0               ,MCfgAutomatic},
    {DI_CHECKBOX   ,5  ,3  ,0  ,0  ,0               ,MCfgCheckMask},
    {DI_EDIT       ,25 ,3  ,58 ,0  ,0               ,-1},
    {DI_TEXT       ,0  ,4  ,0  ,0  ,DIF_SEPARATOR   ,-1},
    {DI_BUTTON     ,0  ,5  ,0  ,0  ,DIF_CENTERGROUP ,MOk},
    {DI_BUTTON     ,0  ,5  ,0  ,0  ,DIF_CENTERGROUP ,MCancel},
  };
  struct FarDialogItem DialogItems[sizeofa(InitDialogItems)];

  InitDialog(InitDialogItems,DialogItems,sizeofa(InitDialogItems));
  DialogItems[1].Focus = TRUE;

  #ifdef ALLOW_VIEWER_API
  if (!lstrcmpi(what,EDITOR))
  {
  #endif
    ReadRegistryEditor();
    lstrcpy(DialogItems[0].Data,GetMsg(MCfgEditor));
    DialogItems[1].Selected = EAutomatic;
    DialogItems[2].Selected = ECheckMask;
    lstrcpy(DialogItems[3].Data,EMask);
  #ifdef ALLOW_VIEWER_API
  }
  else
  {
    ReadRegistryViewer();
    lstrcpy(DialogItems[0].Data,GetMsg(MCfgViewer));
    DialogItems[1].Selected = VAutomatic;
    DialogItems[2].Selected = VCheckMask;
    lstrcpy(DialogItems[3].Data,VMask);
  }
  #endif

  DialogItems[DEF_BUTTON].DefaultButton = 1;

  if (Info.Dialog(Info.ModuleNumber,-1,-1,DLG_WIDTH,DLG_HEIGHT,NULL,(struct FarDialogItem *)&DialogItems,sizeofa(DialogItems)) != DEF_BUTTON)
    return;

  #ifdef ALLOW_VIEWER_API
  if (!lstrcmpi(what,EDITOR))
  {
  #endif
    EAutomatic = DialogItems[1].Selected;
    ECheckMask = DialogItems[2].Selected;
    lstrcpy(EMask,DialogItems[3].Data);
    SetRegKey(AUTOMATIC,EAutomatic);
    SetRegKey(CHECKMASK,ECheckMask);
    SetRegKey(MASK,EMask);
  #ifdef ALLOW_VIEWER_API
  }
  else
  {
    VAutomatic = DialogItems[1].Selected;
    VCheckMask = DialogItems[2].Selected;
    lstrcpy(VMask,DialogItems[3].Data);
    SetRegKey(VAUTOMATIC,VAutomatic);
    SetRegKey(VCHECKMASK,VCheckMask);
    SetRegKey(VMASK,VMask);
  }
  #endif
}

int WINAPI _export ProcessEditorEvent(int Event, void *Param)
{
  //struct EditorSetPosition esp;

  switch (Event)
  {
    case EE_REDRAW:
      if (!EAutomatic)
        return 0;
      PosSave(PS_UPDATE);
      return 0;

    case EE_READ:
      PosSave(PS_XMLREAD);
      PosSave(PS_OPEN);
      if (EAutomatic)
      {
        PosSave(PS_RESTORE);
        if (ECheckMask)
        {
          struct EditorInfo ei;
          Info.EditorControl(ECTL_GETINFO,&ei);
          char ExpandedMask[512];
          FSF.ExpandEnvironmentStr(EMask,ExpandedMask,512);
          if (FSF.ProcessName(ExpandedMask,(char *)ei.FileName,PN_CMPNAMELIST))
            PosSave(PS_SAVE);
        }
      }
      return 0;

    case EE_SAVE:
      if (!EAutomatic)
        return 0;
      PosSave(PS_XMLREAD);
      if (PosSave(PS_UPDATE))
        PosSave(PS_SAVE);
      return 0;

    case EE_CLOSE:
      {
        PSTrackInfo ti;
        ti.EditorID = *((int *)Param);
        ti.Type = TI_EDITOR;
        if (!EAutomatic)
        {
          Tree.Delete(&ti);
          return 0;
        }
        XMLItems.LoadFile(XMLFile,&Tree);
        if (!Tree.GetID(&ti))
          return 0;
        PSInfo psi = {(char *)EDITOR,(char *)ti.FileName, &ti.unmod};
        XMLItems.SetItem(psi);
        XMLItems.SaveFile(XMLFile,&Tree,false);
        Tree.Delete(&ti);
        return 0;
      }
  }
  return 0;
}

#ifdef ALLOW_VIEWER_API
int WINAPI _export ProcessViewerEvent(int Event, void *Param)
{
  switch (Event)
  {
    case VE_READ:
      ViewerPosSave(PS_XMLREAD);
      ViewerPosSave(PS_OPEN);
      if (VAutomatic)
        ViewerPosSave(PS_RESTORE);
        if (VCheckMask)
        {
          struct ViewerInfo vi;
          vi.StructSize=sizeof(vi);
          ViewerControl(VCTL_GETINFO,&vi);
          char ExpandedMask[512];
          FSF.ExpandEnvironmentStr(VMask,ExpandedMask,512);
          if (FSF.ProcessName(ExpandedMask,(char *)vi.FileName,PN_CMPNAMELIST))
            ViewerPosSave(PS_SAVE);
        }
      return 0;

    case VE_CLOSE:
      {
        PSTrackInfo ti;
        ti.EditorID = *((int *)Param);
        ti.Type = TI_VIEWER;
        if (!VAutomatic)
        {
          Tree.Delete(&ti);
          return 0;
        }
        ViewerPosSave(PS_XMLREAD);
        if (ViewerPosSave(PS_UPDATE))
          ViewerPosSave(PS_SAVE);
        Tree.Delete(&ti);
        return 0;
      }
  }
  return 0;
}
#endif

HANDLE WINAPI _export OpenPlugin(int OpenFrom, int Item)
{
  (void) Item;
  struct EditorInfo ei;
  struct EditorSetPosition esp;
  #ifdef ALLOW_VIEWER_API
  struct ViewerInfo vi;
  struct ViewerSetPosition vsp;
  #endif
  int MenuCode,i;
  int Msgs[]={MMenuTrack, MMenuRestore, MMenuEdit, MMenuRemoveOld, MMenuConfig};
  struct FarMenuItemEx MenuItems[sizeofa(Msgs)], *MenuItem;

  XMLItems.LoadFile(XMLFile,&Tree);
  if (OpenFrom != OPEN_PLUGINSMENU)
  {
    if (OpenFrom == OPEN_EDITOR)
    {
      Info.EditorControl(ECTL_GETINFO,&ei);
      PSInfo psi = {(char *)EDITOR,(char *)ei.FileName, &esp};
      PSTrackInfo ti;
      ti.EditorID = ei.EditorID;
      ti.Type = TI_EDITOR;
      if (XMLItems.GetItem(psi) || Tree.FindID(&ti))
        Msgs[0] = MMenuSave;
    }
    #ifdef ALLOW_VIEWER_API
    else
    {
      vi.StructSize=sizeof(vi);
      ViewerControl(VCTL_GETINFO,&vi);
      PSInfo psi = {(char *)VIEWER,(char *)vi.FileName,(struct EditorSetPosition *)&vsp};
      PSTrackInfo ti;
      ti.ViewerID = vi.ViewerID;
      ti.Type = TI_VIEWER;
      if (XMLItems.GetItem(psi) || Tree.FindID(&ti))
        Msgs[0] = MMenuSave;
    }
    #endif
  }
  int MenuItemsCount = sizeofa(MenuItems);
  int Skip = 0;
  if (OpenFrom == OPEN_PLUGINSMENU)
  {
    Skip = 2;
    MenuItemsCount = sizeofa(MenuItems)-Skip;
  }
  //memset(MenuItems,0,sizeof(MenuItems));
  for (MenuItem=MenuItems,i=Skip; i < sizeofa(Msgs); i++, MenuItem++)
  {
    lstrcpy(MenuItem->Text.Text,GetMsg(Msgs[i]));
    MenuItem->Flags = MenuItem->AccelKey = MenuItem->Reserved = MenuItem->UserData = 0;
  }
  if (OpenFrom != OPEN_PLUGINSMENU)
  {
    if (Msgs[0] == MMenuTrack)
      MenuItems[1].Flags = MIF_DISABLE;
  }
  int Selected = 0;
  loop:
  MenuItems[Selected].Flags=MIF_SELECTED;
  MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE|FMENU_USEEXT,
                     GetMsg(MTitle),NULL,NULL,NULL,NULL,
                     (struct FarMenuItem *)MenuItems,MenuItemsCount);
  MenuItems[Selected].Flags ^= MIF_SELECTED;
  Selected = MenuCode;
  if (OpenFrom == OPEN_PLUGINSMENU)
  {
    if (MenuCode >= 0)
      MenuCode += Skip;
  }
  switch (MenuCode)
  {
    case 0:
      XMLItems.LoadFile(XMLFile,&Tree);
      if (OpenFrom == OPEN_EDITOR)
        PosSave(PS_SAVE);
      #ifdef ALLOW_VIEWER_API
      else
        ViewerPosSave(PS_SAVE);
      #endif
      break;
    case 1:
      XMLItems.LoadFile(XMLFile,&Tree);
      if (OpenFrom == OPEN_EDITOR)
        PosSave(PS_RESTORE);
      #ifdef ALLOW_VIEWER_API
      else
        ViewerPosSave(PS_RESTORE);
      #endif
      break;
    case 2:
      EditSavedItems();
      break;
    case 3:
      {
        int count = XMLItems.RemoveOld(&Tree,XMLFile);
        const char *msg[2];
        msg[0] = GetMsg(MTitle);
        char note[70];
        FSF.sprintf(note,GetMsg(MRemovedItems),count);
        msg[1] = note;
        Info.Message(Info.ModuleNumber,FMSG_MB_OK,NULL,msg,2,0);
      }
      break;
    case 4:
      Config();
      break;
  }
  if ((OpenFrom == OPEN_PLUGINSMENU && MenuCode >=0) || MenuCode >=2)
  {
    goto loop;
  }
  return INVALID_HANDLE_VALUE;
}

void WINAPI _export GetPluginInfo(struct PluginInfo *pi)
{
  static const char *MenuStrings[1];

  pi->StructSize=sizeof(struct PluginInfo);
  pi->Flags = PF_EDITOR;
  #ifdef ALLOW_VIEWER_API
  pi->Flags |= PF_VIEWER;
  #endif
  if (!ShowInPanels)
    pi->Flags |= PF_DISABLEPANELS;
  MenuStrings[0] = GetMsg(MTitle);
  pi->PluginMenuStrings = MenuStrings;
  pi->PluginMenuStringsNumber = 1;
  pi->PluginConfigStrings = MenuStrings;
  pi->PluginConfigStringsNumber = 1;
}

int WINAPI _export Configure(int ItemNumber)
{
  (void) ItemNumber;
  Config();
  return TRUE;
}
