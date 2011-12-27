/*
    [ESC] Editor's settings changer plugin for FAR Manager
    Copyright (C) 2001 Ivan Sintyurin
    Copyright (C) 2008 Alex Yaroslavsky

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
#include <CRT/crt.hpp>
#include "plugin.hpp"
#include <initguid.h>
#include "guid.h"
#include "version.hpp"

#if defined(__GNUC__)
extern "C"
{
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
};

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}
#endif

#include "KeySequenceStorage.hpp"
#include "rbtree.cpp"
#include "TArray.cpp"
#include "CUserMacros.hpp"
#include "esc.hpp"
#include "mix.hpp"
#include "SaveAndLoadConfig.hpp"
#include "CIndicator.hpp"
#include "syslog.hpp"

#ifdef _DEBUG
extern wchar_t SysLogName[MAX_PATH*2];
#endif

#include "xmlite.hpp"

extern bool prc_Minuses;

extern wchar_t GLOBAL_EOL[10];

BOOL WINAPI CompLT(const EditorSettingsStorage &a,const EditorSettingsStorage &b)
{
  return a.EditorId<b.EditorId;
}

BOOL WINAPI CompEQ(const EditorSettingsStorage &a,const EditorSettingsStorage &b)
{
  return a.EditorId==b.EditorId;
}

BOOL WINAPI ESCFICompLT(const ESCFileInfo &a,const ESCFileInfo &b)
{
  return (a.Name.str && b.Name.str)?FSF.LStricmp(a.Name.str, b.Name.str)<0:0;
}

BOOL WINAPI ESCFICompEQ(const ESCFileInfo &a,const ESCFileInfo &b)
{
  return a.Name==b.Name;
}

BOOL InitSetsTree(const EditorInfo &EI, const wchar_t *editorfilename)
{
   if(!NodeData)
     return FALSE;
   BOOL Ret=FALSE;
   EditorSettingsStorage ESS;
   Node_<EditorSettingsStorage> *Item;
   int TotalNodes=NodeData->getSize();
   NODEDATA *nd;
   int LastAcceptableType=-1;
   for(int i=TotalNodes-1;i>=0;--i)
   {
     nd=NodeData->getItem(i);
     if(CmpWithFileMask(nd->mask.str, editorfilename, (nd->Options&E_SkipPath_On)!=0))
     {
       LastAcceptableType=i;
       if(nd->Options&E_OverLoad_On)
         break;
     }
   }

   if(LastAcceptableType>=0)
   {
     nd=NodeData->getItem(LastAcceptableType);
     ESS.EditorId=EI.EditorID;
     ESS.index=LastAcceptableType;
     Item=ESETStorage->findNode(ESS);
     if(Item)
       Item->data.Data=*nd;
     else
     {
       ESS.Data=*nd;
       ESETStorage->insertNode(ESS);
     }

     Ret=TRUE;
   }
   _D(SysLog(L"IST: Ret=%d", Ret));
   return Ret;
}

void FreeMem()
{
  if(NodeData)
  {
    delete NodeData;
    NodeData=NULL;
  }
  XMLLoadedOK=FALSE;
  if(ESETStorage)
    ESETStorage->Empty();
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=PLUGIN_VERSION;
  Info->Guid=MainGuid;
  Info->Title=PLUGIN_NAME;
  Info->Description=PLUGIN_DESC;
  Info->Author=PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  EditorControl=::Info.EditorControl;
  FarMessage=::Info.Message;
  IsOldFar=TRUE;
  XMLLoadedOK=FALSE;
  NodeData=NULL;
  _D(wsprintf(SysLogName, L"%s.LOG", ::Info.ModuleName););
  _D(SysLog(L"SetStartupInfo - start"));
  if(Info->StructSize >= (int)sizeof(struct PluginStartupInfo))
  {
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;
    TruncPathStr=FSF.TruncPathStr;
    wcscpy(XMLFilePath, ::Info.ModuleName);
    int offset=lstrlen(XMLFilePath)-3;
    wcscpy(XMLFilePath+offset, L"XML");

    IsOldFar=FALSE;
    LoadGlobalConfig();

    nlsStopChars=new strcon;
    ESETStorage=new CRedBlackTree<EditorSettingsStorage>(CompLT, CompEQ);
    FileInfoTree=new CRedBlackTree<ESCFileInfo>(ESCFICompLT, ESCFICompEQ);
    if(!nlsStopChars || !ESETStorage || !FileInfoTree)
      IsOldFar=TRUE;

    int OldSet=Opt.ReloadSettingsAutomatically;
    Opt.ReloadSettingsAutomatically=TRUE;
    ReloadSettings(TRUE);
    Opt.ReloadSettingsAutomatically=OldSet;
  }
  _D(SysLog(L"SetStartupInfo - end (%d)", IsOldFar));
}

HANDLE WINAPI OpenW(const struct OpenInfo*)
{
    ReloadSettings(FALSE);
    if(!IsOldFar)
    {
        DWORD TotalNodes=XMLLoadedOK?NodeData->getSize():0;
        DWORD FMISize=(1+TotalNodes)*sizeof(FarMenuItem);
        FarMenuItem *fmi=static_cast<FarMenuItem*>(malloc(FMISize));
        if(fmi)
        {
            wchar_t *editorfilename = (wchar_t *)malloc(EditorControl(-1,ECTL_GETFILENAME,0,0)*sizeof(wchar_t));
            EditorControl(-1, ECTL_GETFILENAME, 0, editorfilename);
            EditorControl(-1, ECTL_GETINFO, 0, &ei);
            if(ESETStorage->IsEmpty())
               InitSetsTree(ei, editorfilename);
            memset(fmi, 0, FMISize);

            int RealPos=-1; // где ставить галку, если "-1", то нигде
            NODEDATA *nd;
            int LastAcceptableType=-1;
            for(int f=TotalNodes-1;f>=0;--f)
            {
              nd=NodeData->getItem(f);
              if(RealPos==-1 && CmpWithFileMask(nd->mask.str, editorfilename, (nd->Options&E_SkipPath_On)!=0))
              {
                LastAcceptableType=f;
                if(nd->Options&E_OverLoad_On)
                  RealPos=LastAcceptableType;
              }
            }
            if(RealPos<0)
              RealPos=LastAcceptableType;

            wchar_t fmt1[32], fmt2[32];

            if(Opt.ShowFileMaskInMenu)
            {
               wcscpy(fmt1, L"&%c. %*s \x2502 %s");
               wcscpy(fmt2, L"   %*s \x2502 %s");
            }
            else
            {
               wcscpy(fmt1, L"&%c. %s");
               wcscpy(fmt2, L"   %s");
            }

            if(Opt.ShowFileMaskInMenu)
            {
              int maxlen=0;
              for (DWORD f = 0; f < TotalNodes; ++f)
              {
                int len = NodeData->getItem(f)->Name.getLength();
                if (len > maxlen)
                  maxlen = len;

              }
              for (DWORD f = 0; f < TotalNodes; ++f)
              {
                  nd=NodeData->getItem(f);
                  size_t len = nd->Name.getLength() + nd->mask.getLength() + 50;
                  wchar_t *buf = (wchar_t *)malloc(len*sizeof(wchar_t));
                  if (!buf) continue;
                  if(f<10) FSF.sprintf(buf, fmt1,L'0'+f,maxlen,nd->Name.str,nd->mask.str);
                  else if(f<36) FSF.sprintf(buf, fmt1,L'A'-10+f,maxlen,nd->Name.str,nd->mask.str);
                  else FSF.sprintf(buf, fmt2, maxlen, nd->Name.str, nd->mask.str);

                  fmi[f].Text = buf;
              }
            }
            else for (DWORD f = 0; f < TotalNodes; ++f)
            {
                size_t len = NodeData->getItem(f)->Name.getLength() + 50;
                wchar_t *buf = (wchar_t *)malloc(len*sizeof(wchar_t));
                if (!buf) continue;
                if(f<10) FSF.sprintf(buf, fmt1,L'0'+f,NodeData->getItem(f)->Name.str);
                else if(f<36) FSF.sprintf(buf, fmt1,L'A'-10+f,NodeData->getItem(f)->Name.str);
                else FSF.sprintf(buf, fmt2, NodeData->getItem(f)->Name.str);

                fmi[f].Text = buf;
            }

            EditorSettingsStorage ESS(ei.EditorID);
            Node_<EditorSettingsStorage> *Item=ESETStorage->findNode(ESS);
            int MenuCode=Item?Item->data.index:0, NewMenuCode;
            if(MenuCode<0 || MenuCode>=(int)TotalNodes)
                MenuCode=0;
            fmi[MenuCode].Flags|=MIF_SELECTED;
            if(RealPos>-1) fmi[RealPos].Flags|=L'*'|MIF_CHECKED;
            NewMenuCode = Info.Menu(&MainGuid, &MenuGuid, -1, -1, 0,
                                    FMENU_AUTOHIGHLIGHT | FMENU_WRAPMODE | FMENU_CHANGECONSOLETITLE,
                                    GetMsg(MTitle), NULL, NULL,
                                    NULL, NULL, fmi, TotalNodes);
            if(NewMenuCode>=0 && (!Item || (Item && NewMenuCode!=MenuCode)))
            {
               if(Item)
               {
                  Item->data.Data.LockFile.Off();
                  Item->data.index=NewMenuCode;
                  Item->data.Data=*NodeData->getItem(NewMenuCode);
               }
               else
               {
                  ESS.index=NewMenuCode;
                  ESS.Data=*NodeData->getItem(NewMenuCode);
                  Item=ESETStorage->insertNode(ESS);
               }

               if(Item)
                 ApplyEditorOptions(Item->data.Data,editorfilename);
            }
            for (DWORD f = 0; f < TotalNodes; ++f)
              free((void *)fmi[f].Text);
            free(fmi);
            free(editorfilename);
        }
    }
    EditorControl(-1, ECTL_TURNOFFMARKINGBLOCK, 0, 0);
    return INVALID_HANDLE_VALUE;
}

void WINAPI ExitFARW(const struct ExitInfo*)
{
  FreeMem();
  delete ESETStorage;
  ESETStorage=NULL;
  delete FileInfoTree;
  FileInfoTree=NULL;
  delete nlsStopChars;
  nlsStopChars=NULL;
}

INT_PTR WINAPI ConfigDlgProc (HANDLE hDlg, int Msg, int Param1, void* Param2)
{
  switch (Msg)
    {
      case DN_BTNCLICK:
      {
        switch (Param1)
        {
           case 1:
             if(Param2) // включили плагин
             {
               Info.SendDlgMessage(hDlg,DM_ENABLE,2,(void*)TRUE);
               Info.SendDlgMessage(hDlg,DM_ENABLE,3,(void*)TRUE);
               Info.SendDlgMessage(hDlg,DM_ENABLE,5,(void*)TRUE);
               if(BSTATE_UNCHECKED==Info.SendDlgMessage(hDlg,DM_GETCHECK,2,(void*)0))
                 Info.SendDlgMessage(hDlg,DM_ENABLE,6,(void*)TRUE);
             }
             else       // выключили плагин
             {
               Info.SendDlgMessage(hDlg,DM_ENABLE,2,(void*)FALSE);
               Info.SendDlgMessage(hDlg,DM_ENABLE,3,(void*)FALSE);
               Info.SendDlgMessage(hDlg,DM_ENABLE,5,(void*)FALSE);
               Info.SendDlgMessage(hDlg,DM_ENABLE,6,(void*)FALSE);
             }
             return TRUE;
           case 2:
             Info.SendDlgMessage(hDlg,DM_ENABLE,6,(Param2?(void*)FALSE:(void*)TRUE));
           case 3:
             return TRUE;
           default:
             return FALSE;
        }
      }
    }
  return Info.DefDlgProc (hDlg, Msg, Param1, Param2);
}

int WINAPI ConfigureW(const struct ConfigureInfo*)
{
  if(IsOldFar) return FALSE;

  FarDialogItem DialogItems[] = {
/*00*/ {DI_DOUBLEBOX, 3, 1, 71, 9, {0}, NULL, NULL, 0,                                   GetMsg(MTitle),                      0,0},
/*01*/ {DI_CHECKBOX,  5, 2,  0, 0, {0}, NULL, NULL, 0,                                   GetMsg(MTurnOnPluginModule),         0,0},
/*02*/ {DI_CHECKBOX,  5, 3,  0, 0, {0}, NULL, NULL, 0,                                   GetMsg(MReloadSettingsAutomatically),0,0},
/*03*/ {DI_CHECKBOX,  5, 4,  0, 0, {0}, NULL, NULL, 0,                                   GetMsg(MShowFileMaskInMenu),         0,0},
/*04*/ {DI_TEXT,      0, 5,  0, 0, {0}, NULL, NULL, DIF_BOXCOLOR | DIF_SEPARATOR,        L"",                                 0,0},
/*05*/ {DI_BUTTON,    0, 6,  0, 0, {0}, NULL, NULL, DIF_CENTERGROUP,                     GetMsg(MCheckOutSettings),           0,0},
/*06*/ {DI_BUTTON,    0, 6,  0, 0, {0}, NULL, NULL, DIF_CENTERGROUP,                     GetMsg(MReloadSettings),             0,0},
/*07*/ {DI_TEXT,      0, 7,  0, 0, {0}, NULL, NULL, DIF_BOXCOLOR | DIF_SEPARATOR,        L"",                                 0,0},
/*08*/ {DI_BUTTON,    0, 8,  0, 0, {0}, NULL, NULL, DIF_CENTERGROUP |DIF_DEFAULTBUTTON,  GetMsg(MOk),                         0,0},
/*09*/ {DI_BUTTON,    0, 8,  0, 0, {0}, NULL, NULL, DIF_CENTERGROUP,                     GetMsg(MCancel),                     0,0},
  };
  int ExitCode, oldTurnOnPluginModule=Opt.TurnOnPluginModule;

  //заполнение диалога
  DialogItems[1].Selected = Opt.TurnOnPluginModule;
  DialogItems[2].Selected = Opt.ReloadSettingsAutomatically;
  DialogItems[3].Selected = Opt.ShowFileMaskInMenu;

  if(DialogItems[2].Selected)
    DialogItems[6].Flags|=DIF_DISABLE;

  if(!DialogItems[1].Selected)
  {
    DialogItems[2].Flags|=DIF_DISABLE;
    DialogItems[3].Flags|=DIF_DISABLE;
    DialogItems[5].Flags|=DIF_DISABLE;
    DialogItems[6].Flags|=DIF_DISABLE;
  }

  //показ диалога
  for(;;)
  {
    HANDLE hDlg = Info.DialogInit (&MainGuid, &ConfigureGuid, -1, -1, 75, 11,
                                    NULL, DialogItems, 10, 0, 0, ConfigDlgProc, 0);
    ExitCode = Info.DialogRun(hDlg);

    if(8==ExitCode)
    {
        Opt.TurnOnPluginModule=(int)Info.SendDlgMessage(hDlg,DM_GETCHECK,1,0);
        Opt.ReloadSettingsAutomatically=(int)Info.SendDlgMessage(hDlg,DM_GETCHECK,2,0);
        Opt.ShowFileMaskInMenu=(int)Info.SendDlgMessage(hDlg,DM_GETCHECK,3,0);
        SaveGlobalConfig();
        if(!Opt.TurnOnPluginModule)
          FreeMem();
        else if(Opt.TurnOnPluginModule!=oldTurnOnPluginModule)
          ReloadSettings(TRUE);
        break;
    }
    else if(5==ExitCode)
    {
        TestCfgFiles();
    }
    else if(6==ExitCode)
    {
        ReloadSettings(TRUE);
        if(XMLLoadedOK)
          FarAllInOneMessage(GetMsg(MReloaded), 0);
    }
    else
    {
        Info.DialogFree(hDlg);
        break;
    }

    Info.DialogFree(hDlg);
  }


  return TRUE;
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
  if(IsOldFar) return;
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_PRELOAD|PF_DISABLEPANELS|PF_EDITOR;

  static wchar_t *PluginConfigStrings[1], *PluginMenuStrings[1];

  PluginConfigStrings[0]=
  PluginMenuStrings[0]=const_cast<wchar_t*>(GetMsg(MTitle));
  Info->PluginMenu.Guids=&MainGuid;
  Info->PluginMenu.Count=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  Info->PluginMenu.Strings=PluginMenuStrings;
  Info->PluginConfig.Guids=&MainGuid;
  Info->PluginConfig.Strings=PluginConfigStrings;
  Info->PluginConfig.Count=sizeof(PluginConfigStrings)/sizeof(PluginConfigStrings[0]);
}

static int ProcessEditorEventInternal(int Event, void *Param)
{
  /*#ifdef _DEBUG
  wchar_t *EES[]={"EE_READ","EE_SAVE","EE_REDRAW","EE_CLOSE"};
  if(Event>=EE_READ && Event<=EE_CLOSE)
    SysLog(L"PEE: Event: %s", EES[Event]);
  else
    SysLog(L"PEE: Event: Unknown (%d)", Event);
  #endif*/
  if(ESETStorage)
  {
   if(Event==EE_REDRAW ||
      ((Event==EE_READ || Event==EE_SAVE) && ReloadSettings(FALSE)))
   {
    wchar_t *editorfilename = (wchar_t *)malloc(EditorControl(-1, ECTL_GETFILENAME, 0, 0)*sizeof(wchar_t));
    EditorControl(-1, ECTL_GETFILENAME, 0, editorfilename);
    EditorControl(-1, ECTL_GETINFO, 0, &ei);
    if(Event==EE_READ) InitSetsTree(ei,editorfilename);
    EditorSettingsStorage ESS(ei.EditorID);
    Node_<EditorSettingsStorage> *Item=ESETStorage->findNode(ESS);
    if(Item)
    {
        NODEDATA &nodedata=Item->data.Data;
        if(Event==EE_SAVE)
        {
          bool RestorePos=FALSE, IsLocked=(ei.CurState&ECSTATE_LOCKED)!=0;

          if(!IsLocked && ((nodedata.Options&E_KillSpaces_On)!=0 || nodedata.EOL))
          {
             _D(SysLog(L"PEE: 0. TotalLines=%d", ei.TotalLines));

             prc_Minuses=(nodedata.Options&E_Process_Minuses_On)!=0;

             CIndicator Indicator(NULL);
             Indicator.StartTimer(1000);
             Indicator.SetParams(GetMsg(MWorking), ei.TotalLines);

             RestorePos=TRUE;
             InitNLS(ei, nodedata);
             InitESPandEGS(esp, egs);
             int EscBreak=FALSE, CheckEsc=0;
             void (*StrPrc)();
             if(nodedata.Options&E_KillSpaces_On && nodedata.EOL)
                StrPrc=KillSpacesAndChangeEOL;
             else if(nodedata.Options&E_KillSpaces_On)
                StrPrc=KillSpaces;
             else
                StrPrc=ChangeEOL;
             UnpackEOL(nodedata.EOL, GLOBAL_EOL);
             for(esp.CurLine=0; esp.CurLine<ei.TotalLines; ++esp.CurLine)
             {
                ++CheckEsc;
                if (CheckEsc>512)
                  {
                    Indicator.ShowProgress(esp.CurLine);
                    if (CheckForEsc())
                      {
                        if (!FarAllInOneMessage(GetMsg(MMsgStop), FMSG_MB_YESNO))
                          {
                            EscBreak=TRUE;
                            break;
                          }
                      }
                    CheckEsc=0;
                  }

                StrPrc();
             }

             if(!EscBreak) Indicator.ShowFinal();
          }

          if(!IsLocked && nodedata.Options&E_KillEmptyLines_On)
          {
             RestorePos=TRUE;
             InitESPandEGS(esp, egs);
             int LastNotEmptyLine, LimitLine, lineFound=FALSE, lEmpty;

             LastNotEmptyLine=LimitLine=(nodedata.Options&E_ForceKillEL_On)?
                                         0:ei.CurLine;

             esp.CurLine=ei.TotalLines-1;
             EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
             EditorControl(-1, ECTL_GETSTRING, 0, &egs);
             lEmpty=!*egs.StringEOL;
             if(egs.StringLength>0)
             {
               LastNotEmptyLine=esp.CurLine;
               lineFound=TRUE;
             }
             else for(esp.CurLine=ei.TotalLines-2; esp.CurLine>=LimitLine;
                  --esp.CurLine)
             {
                EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
                EditorControl(-1, ECTL_GETSTRING, 0, &egs);
                if(egs.StringLength>0)
                {
                  LastNotEmptyLine=esp.CurLine;
                  lineFound=TRUE;
                  break;
                }
             }

             if(!lineFound) --LastNotEmptyLine;
             for(esp.CurLine=ei.TotalLines-1-lEmpty;
                 esp.CurLine>LastNotEmptyLine; --esp.CurLine)
             {
                EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
                EditorControl(-1, ECTL_DELETESTRING, 0, 0);
             }
          }

          {
            CEditorOptions eo(nodedata);
            eo.ApplyOption(ESPT_SAVEFILEPOSITION);
          }

          if(RestorePos) // восстановим старую позицию:
             RestorePosition(ei);
          nodedata.LockFile.Off();
        }
        else if(Event==EE_REDRAW)
        {
          if(nodedata.Options&E_LockFile_On && !nodedata.LockFile.IsOn())
            nodedata.LockFile.On(editorfilename);
        }
        else  // EE_READ
        {
          ApplyEditorOptions(nodedata,editorfilename);
          KeySequence macro;
          const KeySequenceStorage *KSS;
          BOOL stop=TRUE;
          if(!FileExists(editorfilename)) // файла нет на диске, значит он новый
          {
            EditorSetParameter espar;
            memset(&espar, 0, sizeof(espar));
            espar.Type=ESPT_CODEPAGE;
            bool done=false;
            if(*nodedata.TableName.str)
            {
              espar.iParam=FindCodeTable(nodedata.TableName.str);
              if(espar.iParam>-1)
              {
                espar.iParam+=3;
                EditorControl(-1, ECTL_SETPARAM, 0, &espar);
                done=true;
              }
            }
            if(!done && nodedata.Table>0)
            {
              espar.iParam=nodedata.Table;
              EditorControl(-1, ECTL_SETPARAM, 0, &espar);
            }

            if((nodedata.Options2&E_Bom_On) || (nodedata.Options2&E_Bom_Off))
            {
              memset(&espar, 0, sizeof(espar));
              espar.Type=ESPT_SETBOM;
              espar.iParam=(nodedata.Options2&E_Bom_On)?TRUE:FALSE;
              EditorControl(-1, ECTL_SETPARAM, 0, &espar);
            }

            _D(SysLog(L"PEE: auto-macros / OnCreate - start"));
            nodedata.OnCreateMacros.toBegin();
            do
            {
              KSS=nodedata.OnCreateMacros.getItem();
              if(KSS)
              {
                macro=KSS->GetSequence(stop);
                FARPostMacro(&macro);
              }
              nodedata.OnCreateMacros.toNext();
            } while(KSS && !stop);
            _D(SysLog(L"PEE: auto-macros / OnCreate - end"));
          }
          _D(SysLog(L"PEE: auto-macros / OnLoad - start"));
          nodedata.OnLoadMacros.toBegin();
          do
          {
            KSS=nodedata.OnLoadMacros.getItem();
            if(KSS)
            {
              macro=KSS->GetSequence(stop);
              FARPostMacro(&macro);
            }
            nodedata.OnLoadMacros.toNext();
          } while(KSS && !stop);
          _D(SysLog(L"PEE: auto-macros / OnLoad - end"));
        }
    }
    free(editorfilename);
   }
   else if(Event==EE_CLOSE)
   {
     EditorSettingsStorage ESS(*(int *)Param);
     Node_<EditorSettingsStorage> *Item=ESETStorage->findNode(ESS);
     if(Item) ESETStorage->deleteNode(Item);
   }
  }
//  _D(SysLog(L"PEE: end"));
  return 0;
}

int WINAPI ProcessEditorEventW(const struct ProcessEditorEventInfo* Info)
{
  return ProcessEditorEventInternal(Info->Event,Info->Param);
}

static int ProcessEditorInputInternal(const INPUT_RECORD *Rec)
{
    static int isReenter=0, Lines, CoordX, RetCode;

    if(IsOldFar || isReenter ||
       (Rec->EventType!=KEY_EVENT && Rec->EventType!=FARMACRO_KEY_EVENT && Rec->EventType!=MOUSE_EVENT))
    {
      //_D(SysLog(L"PEI: 1 return"));
      return 0;
    }

    if((Rec->EventType==KEY_EVENT || Rec->EventType==FARMACRO_KEY_EVENT) && !Rec->Event.KeyEvent.bKeyDown)
    {
      //_D(SysLog(L"PEI: 2 return"));
      return 0;
    }

    if(Rec->EventType==MOUSE_EVENT &&
       (Rec->Event.MouseEvent.dwEventFlags&DOUBLE_CLICK ||
        !(Rec->Event.MouseEvent.dwButtonState&(FROM_LEFT_1ST_BUTTON_PRESSED|
          RIGHTMOST_BUTTON_PRESSED|FROM_LEFT_2ND_BUTTON_PRESSED))
       )
      )
    {
      //_D(SysLog(L"PEI: 3 return"));
      return 0;
    }

    _D(SysLog(L"PEI: EventType=%s",
       (Rec->EventType==KEY_EVENT)?L"KEY_EVENT":
         ((Rec->EventType==FARMACRO_KEY_EVENT)?L"FARMACRO_KEY_EVENT":
           (Rec->EventType==MOUSE_EVENT)?L"MOUSE_EVENT":L"UNKNOWN"
         )
       )
      );

    EditorControl(-1, ECTL_GETINFO, 0, &ei);
    if(ESETStorage->IsEmpty())
    {
       wchar_t *editorfilename = (wchar_t *)malloc(EditorControl(-1, ECTL_GETFILENAME, 0, 0)*sizeof(wchar_t));
       EditorControl(-1, ECTL_GETFILENAME, 0, editorfilename);
       InitSetsTree(ei, editorfilename);
       free(editorfilename);
    }
    EditorSettingsStorage ESS(ei.EditorID);
    Node_<EditorSettingsStorage> *Item=ESETStorage->findNode(ESS);
    if(NULL==Item)
    {
      _D(SysLog(L"PEI: 4 return"));
      return 0;
    }
    NODEDATA &nodedata=Item->data.Data;

    if(Rec->EventType==MOUSE_EVENT) // обработка макросов
    {
      if(nodedata.Options&E_UserMacro_On)
      {
         const MOUSE_EVENT_RECORD &MouseEvent=Rec->Event.MouseEvent;
         UserMacroID id;
         if(MouseEvent.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)
           id.ButtonState|=EBS_LClick_On;
         if(MouseEvent.dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED)
           id.ButtonState|=EBS_MClick_On;
         if(MouseEvent.dwButtonState&RIGHTMOST_BUTTON_PRESSED)
           id.ButtonState|=EBS_RClick_On;

         if(MouseEvent.dwControlKeyState&RIGHT_CTRL_PRESSED)
           id.ControlState|=ECS_RCtrl_On;
         if(MouseEvent.dwControlKeyState&LEFT_CTRL_PRESSED)
           id.ControlState|=ECS_LCtrl_On;
         if(MouseEvent.dwControlKeyState&RIGHT_ALT_PRESSED)
           id.ControlState|=ECS_RAlt_On;
         if(MouseEvent.dwControlKeyState&LEFT_ALT_PRESSED)
           id.ControlState|=ECS_LAlt_On;
         if(MouseEvent.dwControlKeyState&SHIFT_PRESSED)
           id.ControlState|=ECS_Shift_On;

         BOOL Stop;
         if(EditorPostMacro(nodedata.MouseMacros,id,ei,Stop))
         {
           _D(SysLog(L"PEI: 5 return"));
           return Stop?1:0;
         }
      }
      _D(SysLog(L"PEI: 6 return"));
      return 0;
    }

    if(Rec->EventType!=KEY_EVENT && Rec->EventType!=FARMACRO_KEY_EVENT)
    {
      _D(SysLog(L"PEI: 7 return"));
      return 0;
    }

    const KEY_EVENT_RECORD &KeyEvent=Rec->Event.KeyEvent;

    // обработка макросов
    // делаем это только для KEY_EVENT, чтобы не вызвать зацикливание (вызов макроса из макроса)
    if(nodedata.Options&E_UserMacro_On && Rec->EventType==KEY_EVENT)
    {
       UserMacroID id(Rec->Event.KeyEvent.wVirtualKeyCode,Rec->Event.KeyEvent.dwControlKeyState);
       BOOL Stop;
       if(EditorPostMacro(nodedata.KeyMacros,id,ei,Stop))
       {
         _D(SysLog(L"PEI: 8 return"));
         return Stop?1:0;
       }
    }

    if((KeyEvent.dwControlKeyState &
          (LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_ALT_PRESSED|
           RIGHT_CTRL_PRESSED)
       ) ||
       (KeyEvent.wVirtualKeyCode!=VK_END &&
        KeyEvent.wVirtualKeyCode!=VK_TAB &&
        KeyEvent.wVirtualKeyCode!=VK_BACK &&
        KeyEvent.wVirtualKeyCode!=VK_HOME &&
        KeyEvent.wVirtualKeyCode!=VK_RETURN &&
        KeyEvent.wVirtualKeyCode!=VK_DELETE &&
        (
          KeyEvent.dwControlKeyState&ENHANCED_KEY ||
          ((BYTE)KeyEvent.uChar.UnicodeChar)<32
        )
       )
      )
    {
      _D(SysLog(L"PEI: 9 return"));
      return 0; // все лишнее отсекаем сразу
                // реагируем только на печатные символы и end/shift-end
    }

    int isWrapQuote=(nodedata.Options&E_ProcessQuote_On)?1:0;
    bool IsLocked=(ei.CurState&ECSTATE_LOCKED)!=0;

    // wVirtualKeyCode, обработка которых не зависит от IsLocked
    if(KeyEvent.wVirtualKeyCode==VK_HOME)
    {
      RetCode=0;
      if(!(KeyEvent.dwControlKeyState & SHIFT_PRESSED) &&
         nodedata.Options & E_SmartHome_On)
      {
         isReenter=1;
         RetCode=GotoHome(ei, nodedata);
         if(RetCode)
           EditorControl(-1, ECTL_TURNOFFMARKINGBLOCK, 0, 0);
         isReenter=0;
      }
      _D(SysLog(L"PEI: 9 return"));
      return RetCode;
    }

    // wVirtualKeyCode, обработка которых частично зависит от IsLocked
    if(KeyEvent.wVirtualKeyCode==VK_TAB)
    {
      RetCode=0;
      isReenter=1;
      if(nodedata.Lines && (nodedata.Options&E_SmartTab_On) &&
         (ei.Options&EOPT_EXPANDALLTABS)
        )
      {
        Lines=nodedata.Lines;
        InitESPandEGS(esp, egs);
        InitNLS(ei, nodedata);
        if(KeyEvent.dwControlKeyState & SHIFT_PRESSED) // shift-tab
        {
          CoordX=GetPrevCoordX(ei, Lines, nlsStopChars->str);
          if(CoordX>-1)
          {
            esp.CurPos=CoordX;
            RetCode=ChangeCoordX(ei, esp);
            if(RetCode && !(ei.Options&EOPT_PERSISTENTBLOCKS))
              EditorUnmarkBlock();
          }
        }
        else if(!IsLocked)                             // tab
        {
          if(ei.BlockType!=BTYPE_NONE && !(ei.Options&EOPT_PERSISTENTBLOCKS))
          {
            EditorControl(-1, ECTL_DELETEBLOCK, 0, 0);
            EditorControl(-1, ECTL_GETINFO, 0, &ei);
          }
          CoordX=GetNextCoordX(ei, Lines, nlsStopChars->str)-ei.CurPos;
          //_D(SysLog(L"SmartTab=%d", CoordX));
          if(CoordX>-1)
          {
            EditorControl(-1, ECTL_GETSTRING, 0, &egs);
            ess.StringNumber=-1;
            ess.StringEOL=egs.StringEOL;
            ess.StringLength=egs.StringLength+CoordX;
            if(ei.CurPos>=egs.StringLength)
              ess.StringLength+=ei.CurPos-egs.StringLength;
            ess.StringText=static_cast<wchar_t*>(malloc(ess.StringLength*sizeof(wchar_t)));
            if(ess.StringText)
            {
              if(egs.StringLength>ei.CurPos)
                 memcpy((wchar_t *)ess.StringText, egs.StringText, ei.CurPos*sizeof(wchar_t));
              else
              {
                 memcpy((wchar_t *)ess.StringText, egs.StringText, egs.StringLength*sizeof(wchar_t));
                 wmemset((wchar_t *)ess.StringText+egs.StringLength, nlsSpace,
                         ei.CurPos-egs.StringLength);
              }
              wmemset((wchar_t *)ess.StringText+ei.CurPos, nlsSpace, CoordX);
              if(egs.StringLength>ei.CurPos)
                 memcpy((wchar_t *)ess.StringText+ei.CurPos+CoordX,
                        egs.StringText+ei.CurPos, (egs.StringLength-ei.CurPos)*sizeof(wchar_t));
              if(EditorControl(-1, ECTL_SETSTRING, 0, &ess))
              {
                InitESPandEGS(esp, egs);
                esp.CurPos=ei.CurPos+CoordX;
                EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
                EditorControl(-1, ECTL_REDRAW, 0, 0);
                RetCode=1;
              }
              free((wchar_t *)ess.StringText);
            }
          }
        }
      }
      if(RetCode)
        EditorControl(-1, ECTL_TURNOFFMARKINGBLOCK, 0, 0);
      isReenter=0;
      _D(SysLog(L"PEI: 10 return"));
      return RetCode;
    }

    if(IsLocked)
    {
      _D(SysLog(L"PEI: 11 return"));
      return 0;
    }

    // wVirtualKeyCode, обработка которых разрешена только при !IsLocked
    if(KeyEvent.wVirtualKeyCode==VK_END)
    {
      if(nodedata.Options&E_ProcessKeyEnd_On)
      {
        isReenter=1;
        InitNLS(ei, nodedata);
        InitESPandEGS(esp, egs);
        prc_Minuses=(nodedata.Options&E_Process_Minuses_On)!=0;
        KillSpaces();
        isReenter=0;
      }
      _D(SysLog(L"PEI: 12 return"));
      return 0;
    }
    else if(KeyEvent.wVirtualKeyCode==VK_BACK)
    {
      RetCode=0;
      if(nodedata.Lines && (nodedata.Options&E_SmartBackSpace_On) &&
         (ei.Options&(EOPT_EXPANDALLTABS|EOPT_EXPANDONLYNEWTABS)) &&
         ei.CurPos && !(KeyEvent.dwControlKeyState & SHIFT_PRESSED) &&
         (ei.BlockType==BTYPE_NONE || !(ei.Options&EOPT_DELREMOVESBLOCKS) ||
          ei.Options&EOPT_PERSISTENTBLOCKS)
        )
      {
        Lines=nodedata.Lines;
        isReenter=1;
        InitESPandEGS(esp, egs);
        InitNLS(ei, nodedata);
        EditorControl(-1, ECTL_GETSTRING, 0, &egs);
        int X=Min(egs.StringLength, ei.CurPos);
        for(int f=X-1;f>=0;--f)
        {
          if(!IsCSpaceOrTab(egs.StringText[f]))
          {
             X=-1;
             break;
          }
        }

        CoordX=(X==-1)?-1:GetPrevCoordX(ei, Lines, nlsStopChars->str);
        //_D(SysLog(L"SmartBackSpace: %d", CoordX));
        if(CoordX>-1)
        {
          EditorControl(-1, ECTL_GETSTRING, 0, &egs);
          ess.StringNumber=-1;
          ess.StringEOL=egs.StringEOL;
          ess.StringLength=egs.StringLength+CoordX;
          if(ei.CurPos>=egs.StringLength)
            ess.StringLength=egs.StringLength?CoordX:0;
          else
            ess.StringLength=egs.StringLength-(ei.CurPos-CoordX);
          ess.StringText=static_cast<wchar_t*>(malloc((ess.StringLength+1)*sizeof(wchar_t)));
          if(ess.StringText)
          {
            if(egs.StringLength>CoordX)
              memcpy((wchar_t *)ess.StringText, egs.StringText, CoordX*sizeof(wchar_t));
            else
              memcpy((wchar_t *)ess.StringText, egs.StringText, egs.StringLength*sizeof(wchar_t));

            if(egs.StringLength>ei.CurPos)
              memcpy((wchar_t *)ess.StringText+CoordX,
                     egs.StringText+ei.CurPos, (egs.StringLength-ei.CurPos)*sizeof(wchar_t));
            if(EditorControl(-1, ECTL_SETSTRING, 0, &ess))
            {
              InitESPandEGS(esp, egs);
              esp.CurPos=CoordX;
              EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
              EditorControl(-1, ECTL_REDRAW, 0, 0);
              RetCode=1;
              EditorControl(-1, ECTL_TURNOFFMARKINGBLOCK, 0, 0);
            }
            free((wchar_t *)ess.StringText);
          }
        }
        isReenter=0;
      }
      _D(SysLog(L"PEI: 13 return"));
      return RetCode;
    }
    else if(KeyEvent.wVirtualKeyCode==VK_RETURN)
    {
      RetCode=0;
      if(isWrapQuote &&
         !(!(ei.Options&EOPT_PERSISTENTBLOCKS) && ei.BlockType!=BTYPE_NONE) &&
         !(KeyEvent.dwControlKeyState & SHIFT_PRESSED)
        )
      {
        InitESPandEGS(esp, egs);
        InitNLS(ei, nodedata);
        EditorControl(-1, ECTL_GETSTRING, 0, &egs);
        RetCode=ProcessKeyEnter(ei, esp, egs,
                IsQuote(egs.StringText,egs.StringLength),
                (nodedata.Options&E_QuoteEOL_On)!=0);
        if(RetCode)
          EditorControl(-1, ECTL_TURNOFFMARKINGBLOCK, 0, 0);
      }
      _D(SysLog(L"PEI: 14 return"));
      return RetCode;
    }
    else if(KeyEvent.wVirtualKeyCode==VK_DELETE)
    {
      RetCode=0;
      if(isWrapQuote &&
         !(ei.Options&EOPT_DELREMOVESBLOCKS && ei.BlockType!=BTYPE_NONE) &&
         !(KeyEvent.dwControlKeyState & SHIFT_PRESSED)
        )
      {
        InitESPandEGS(esp, egs);
        InitNLS(ei, nodedata);
        EditorControl(-1, ECTL_GETSTRING, 0, &egs);
        if(ei.CurPos>=egs.StringLength)
        {
          RetCode=ProcessKeyDelete(egs,
                  IsQuote(egs.StringText,egs.StringLength));
          if(RetCode)
            EditorControl(-1, ECTL_TURNOFFMARKINGBLOCK, 0, 0);
        }
      }
      _D(SysLog(L"PEI: 15 return"));
      return RetCode;
    }
    else if(nodedata.Options&E_AddSymbol_On)
    {
      isReenter=1;
      if(ei.BlockType!=BTYPE_NONE && !(ei.Options&EOPT_PERSISTENTBLOCKS))
      {
        EditorControl(-1, ECTL_DELETEBLOCK, 0, 0);
        EditorControl(-1, ECTL_GETINFO, 0, &ei);
      }
      RetCode=0;
      InitNLS(ei, nodedata);
      //_D(SysLog(L"UnicodeChar=[%c] AddSym_S=[%s]",KeyEvent.uChar.UnicodeChar, nodedata.AddSym_S.str));
      if(nodedata.AddSym_S.getLength() &&
         nodedata.AddSym_S.getLength()==nodedata.AddSym_E.getLength())
         RetCode=InsertAdditionalSymbol(ei, esp, ess, egs,
                            nodedata.AddSym_S, nodedata.AddSym_E,
                            KeyEvent.uChar.UnicodeChar, TRUE);
      if(!RetCode &&
         nodedata.AddSym_S_B.getLength() &&
         nodedata.AddSym_S_B.getLength()==nodedata.AddSym_E_B.getLength())
         RetCode=InsertAdditionalSymbol(ei, esp, ess, egs,
                            nodedata.AddSym_S_B, nodedata.AddSym_E_B,
                            KeyEvent.uChar.UnicodeChar, FALSE);
      isReenter=0;
      if(RetCode)
      {
        EditorControl(-1, ECTL_TURNOFFMARKINGBLOCK, 0, 0);
        _D(SysLog(L"PEI: 16 return"));
        return TRUE;
      }
    }

    wchar_t* buff, *buff1, *buff2=NULL, *buff3=NULL;
    int buff2TextLength=0;
    int Blank;
    int i;
    int nbCount, nbExtra;
    int nWrapPos=CalcWrapPos(nodedata, ei),
        isJustifyEnabled=(nodedata.Options2&E_Wrap_Justify)?1:0;
    div_t SpaceCount;

    _D(SysLog(L"PEI: Wrap=%d", !!(nodedata.Options&E_AutoWrap_On)));
    if( nWrapPos<1 || !(nodedata.Options&E_AutoWrap_On))
    {
      _D(SysLog(L"PEI: 17 return"));
      return 0;
    }

    if( ei.CurTabPos>=nWrapPos ){

        isReenter=1;
        EditorControl(-1, ECTL_PROCESSINPUT, 0, (void*)Rec);
        isReenter=0;

        i=-1;
        EditorControl(-1, ECTL_EXPANDTABS, 0, &i); // probably it's needed to pack it back later

        EditorControl(-1, ECTL_GETINFO, 0, &ei);
        InitNLS(ei, nodedata);

        InitESPandEGS(esp, egs);
        EditorControl(-1, ECTL_GETSTRING, 0, &egs);

        _D(SysLog(L"PEI: ei.CurTabPos=%d, nWrapPos=%d, egs.StringLength=%d",
           ei.CurPos, nWrapPos, egs.StringLength));
        if( ei.CurTabPos>nWrapPos && ei.CurTabPos==egs.StringLength ){ // must be at the end of string
          buff1=(wchar_t*)malloc(egs.StringLength*sizeof(wchar_t));
          buff=(wchar_t*)malloc(nWrapPos*sizeof(wchar_t));
          if(buff1 && buff){
            memcpy(buff1,egs.StringText,egs.StringLength*sizeof(wchar_t));

            memcpy(buff,buff1,nWrapPos*sizeof(wchar_t));

            Blank=0;
            if( egs.StringLength>nWrapPos && buff1[nWrapPos]==nlsSpace )
                Blank=1;
            for( ess.StringLength=nWrapPos-1; ess.StringLength>=0; ess.StringLength-- ){
                if( !Blank ){
                    if( buff[ess.StringLength]==nlsSpace )Blank=1;
                }else{
                    if( buff[ess.StringLength]!=nlsSpace ){
                        ess.StringLength++;
                        break;
                    }
                }
            }

            int nQuote=isWrapQuote?IsQuote(buff,egs.StringLength):0;

            if( ess.StringLength>0 ){ // we have found the blank where to crop the string...

                // portion of string that will go to the next line
                egs.StringLength-=ess.StringLength;
                buff1+=ess.StringLength;

                // skip leading blanks in the rest of the string
                for( ; egs.StringLength && buff1[0]==nlsSpace; egs.StringLength-- )
                    buff1++;

                if( isJustifyEnabled ){
                    // make justify on buff...

                    buff2=(wchar_t*)malloc(nWrapPos*sizeof(wchar_t));
                    if(!buff2) goto EXIT;

                    while( ess.StringLength ){ // trim end spaces
                        if( buff[ess.StringLength-1]==nlsSpace ) ess.StringLength--;
                        else break;
                    }
                    for( i=0; i<ess.StringLength; i++ ){
                        if( buff[i]!=nlsSpace ) break; // skip leading blanks
                        else buff2[buff2TextLength++]=nlsSpace;
                    }
                    for( nbCount=Blank=0; i<ess.StringLength; i++ ){
                        if( Blank<=0 ){ // it was also blank before or just beggining
                            if( buff[i]!=nlsSpace ){
                                Blank=-Blank+1; // increment word count
                                nbCount++;
                            }
                        }else{ // it was a word before
                            if( buff[i]==nlsSpace ) Blank=-Blank;
                            else nbCount++;
                        }
                    }
                    if( Blank<0 ) Blank=-Blank;
                    // now in Blank we have word count and non-blank syms count in nbCount...

                    // now let us try to get some words from the resting string portion...
                    if( egs.StringLength ){ // if something really rested
                        nbExtra=0;
                        for( i=0; i<=egs.StringLength; i++ ){
                            if( i==egs.StringLength || buff1[i]==nlsSpace ){
                                if( buff2TextLength+Blank+nbCount /* the most comact string len+1 */
                                    + nbExtra > nWrapPos ) break;
                                Blank++;
                                nbCount+=nbExtra;
                                nbExtra=0;
                                while( ++i<egs.StringLength ){
                                    if( buff1[i]!=nlsSpace ){
                                        i--;
                                        break;
                                    }
                                }
                            }else
                                nbExtra++;
                        }
                    }

                    if( Blank==1 ){
                        free(buff2);
                        buff2=buff;
                        buff2TextLength=ess.StringLength;

                    }else{

                        SpaceCount.quot=(nWrapPos-nbCount-buff2TextLength)/--Blank;
                        SpaceCount.rem=nWrapPos-nbCount-buff2TextLength-SpaceCount.quot*Blank;
                        SpaceCount.rem=Blank-SpaceCount.rem;

                        // now we have minimum space length in SpaceCount.quot
                        // and amount of blank fields with _no_ extra spacing in SpaceCount.rem

                        for( i=buff2TextLength; i<ess.StringLength; ){
                            if( buff[i]==nlsSpace ){
                                wmemset( buff2+buff2TextLength, nlsSpace, SpaceCount.quot );
                                buff2TextLength+=SpaceCount.quot;
                                if( SpaceCount.rem ) SpaceCount.rem--;
                                else buff2[buff2TextLength++]=nlsSpace;
                                Blank--;
                                while( i++ < ess.StringLength )
                                    if( buff[i]!=nlsSpace ) break;
                            }else
                                buff2[buff2TextLength++]=buff[i++];
                        }

                        // fill up to nWrapPos with the initial part of the resting string...
                        if( Blank ){
                            wmemset( buff2+buff2TextLength, nlsSpace, SpaceCount.quot);
                            buff2TextLength+=SpaceCount.quot;
                            if( SpaceCount.rem ) SpaceCount.rem--;
                            else buff2[buff2TextLength++]=nlsSpace;
                            Blank--;
                            for( i=0; i<egs.StringLength; ){
                                if( buff1[i]==nlsSpace ){
                                    if( Blank ){
                                        wmemset( buff2+buff2TextLength, nlsSpace, SpaceCount.quot );
                                        buff2TextLength+=SpaceCount.quot;
                                        if( SpaceCount.rem ) SpaceCount.rem--;
                                        else buff2[buff2TextLength++]=nlsSpace;
                                    }
                                    while( i < egs.StringLength )
                                        if( buff1[i]!=nlsSpace ) break;
                                        else i++;
                                    if( Blank-- == 0 ) break;
                                }else
                                    buff2[buff2TextLength++]=buff1[i++];
                            }
                            buff1+=i;
                            egs.StringLength-=i;
                        }
                    }

                }else{
                    buff2=buff;
                    buff2TextLength=ess.StringLength;
                }

                ess.StringText=buff2;
                ess.StringLength=buff2TextLength;
                ess.StringNumber=-1;
                ess.StringEOL=const_cast<wchar_t*>(egs.StringEOL);
                EditorControl(-1, ECTL_SETSTRING, 0, &ess);

                // may be here we need to pack the tabs back, if tabs are enabled...
                // this time for the current line we will forget now forever...

                if( egs.StringLength || KeyEvent.wVirtualKeyCode==VK_SPACE || KeyEvent.wVirtualKeyCode==VK_TAB ){
                    EditorControl(-1, ECTL_INSERTSTRING, 0, 0);

                    if( (ei.Options&EOPT_AUTOINDENT) && nQuote==0 ){

                        for( i=0; i<ess.StringLength; i++ )
                            if( ess.StringText[i]!=nlsSpace ) break;

                        ess.StringLength=egs.StringLength+i;
                        ess.StringText=buff3=(wchar_t*)malloc((ess.StringLength+1)*sizeof(wchar_t));
                        if( ess.StringText==NULL ){
                            ess.StringLength=egs.StringLength;
                            ess.StringText=buff1;
                        }else{
                            wmemset((wchar_t *)ess.StringText,nlsSpace,i);
                            memcpy((wchar_t *)ess.StringText+i,buff1,egs.StringLength*sizeof(wchar_t));
                        }
                    }else{
                        ess.StringLength=egs.StringLength;
                        ess.StringText=buff1;
                    }

                    EditorControl(-1, ECTL_SETSTRING, 0, &ess);

                    if( nQuote ){ // insert wrapped quote from pervious line...
                        esp.CurLine=-1;
                        esp.CurPos=0;
                        esp.CurTabPos=-1;
                        esp.TopScreenLine=-1;
                        esp.LeftPos=-1;
                        esp.Overtype=-1;
                        EditorControl(-1, ECTL_SETPOSITION, 0, &esp);

                        buff[nQuote]=L'\0';
                        EditorControl(-1, ECTL_INSERTTEXT, 0, buff);
                    }

                    // may be here we need to pack the tabs, if tabs are enabled...
                }else nQuote=0; // we do nothing with quote this time...

                esp.CurLine=-1;
                esp.CurPos=ess.StringLength+nQuote;
                esp.CurTabPos=-1;
                esp.TopScreenLine=-1;
                esp.LeftPos=-1;
                esp.Overtype=-1;
                EditorControl(-1, ECTL_SETPOSITION, 0, &esp);

                EditorControl(-1, ECTL_REDRAW, 0, 0);
            }
          }
EXIT:
          if(buff3) free(buff3);
          if(buff2 && buff2 != buff) free(buff2);
          if(buff)  free(buff);
          if(buff1) free(buff1);
        }
        // may be here we need to pack the tabs back, if tabs are enabled...
        _D(SysLog(L"PEI: 18 return"));
        return 1;
    }
    _D(SysLog(L"PEI: last return"));
    return 0;
}

int WINAPI ProcessEditorInputW(const ProcessEditorInputInfo* Info)
{
  return ProcessEditorInputInternal(&Info->Rec);
}

#ifdef __cplusplus
extern "C"
{
#endif
int WINAPI GetEditorSettingsW(int EditorID, const wchar_t *szName, void *Param)
{
  if(!szName || !Param || *szName==0 || !NodeData || !ESETStorage)
    return FALSE;

  EditorSettingsStorage ESS(EditorID);
  Node_<EditorSettingsStorage> *Item=ESETStorage->findNode(ESS);
  if(NULL==Item) return FALSE;

  const NODEDATA &Data=Item->data.Data;
  BOOL RetCode=TRUE;

  if(!lstrcmp(szName, XMLStr.TabSize))
  {
     *static_cast<int *>(Param)=Data.TabSize;
  }
  else if(!lstrcmp(szName, XMLStr.ExpandTabs))
  {
     if(Data.Options&E_ExpandTabs_On)
       *static_cast<int *>(Param)=1;
     else if(Data.Options&E_ExpandTabs_OnlyNew)
       *static_cast<int *>(Param)=3;
     else if(Data.Options&E_ExpandTabs_Off)
       *static_cast<int *>(Param)=0;
     else
       *static_cast<int *>(Param)=2;
  }
  else if(!lstrcmp(szName, XMLStr.CursorBEOL))
  {
     *static_cast<int *>(Param)=(Data.Options&E_CursorBeyondEOL_On)?1:
       ((Data.Options&E_CursorBeyondEOL_Off)?0:2);
  }
  else if(!lstrcmp(szName, XMLStr.AutoIndent))
  {
     *static_cast<int *>(Param)=(Data.Options&E_AutoIndent_On)?1:
       ((Data.Options&E_AutoIndent_On)?0:2);
  }
  else if(!lstrcmp(szName, XMLStr.CharCodeBase))
  {
     *static_cast<int *>(Param)=(Data.Options&E_CharCodeBase_Oct)?8:
       ((Data.Options&E_CharCodeBase_Dec)?10:
        ((Data.Options&E_CharCodeBase_Hex)?16:0));
  }
  else if(!lstrcmp(szName, XMLStr.KillSpaces))
  {
     *static_cast<int *>(Param)=(Data.Options&E_KillSpaces_On)?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.p_Minuses))
  {
     *static_cast<int *>(Param)=(Data.Options&E_Process_Minuses_On)?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.KillEmptyLines))
  {
     *static_cast<int *>(Param)=(Data.Options&E_KillEmptyLines_On)?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.ForceKillEmptyLines))
  {
     *static_cast<int *>(Param)=(Data.Options&E_ForceKillEL_On)?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.AutoWrap))
  {
     *static_cast<int *>(Param)=(Data.Options&E_AutoWrap_On)?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.Wrap))
  {
     EditorControl(-1, ECTL_GETINFO, 0, &ei);
     *static_cast<int *>(Param)=CalcWrapPos(Data, ei);
  }
  else if(!lstrcmp(szName, XMLStr.Justify))
  {
     *static_cast<int *>(Param)=(Data.Options2&E_Wrap_Justify)?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.eol))
  {
     *static_cast<DWORD *>(Param)=Data.EOL;
  }
  else if(!lstrcmp(szName, XMLStr.Table))
  {
     *static_cast<DWORD *>(Param)=Data.Table;
  }
  else if(!lstrcmp(szName, XMLStr.ProcessKeyEnd))
  {
     *static_cast<int *>(Param)=(Data.Options&E_ProcessKeyEnd_On)?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.SmartTab))
  {
     *static_cast<int *>(Param)=(Data.Options&E_SmartTab_On)?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.SmartBackSpace))
  {
     *static_cast<int *>(Param)=(Data.Options&E_SmartBackSpace_On)?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.Lines))
  {
     *static_cast<int *>(Param)=Data.Lines;
  }
  else if(!lstrcmp(szName, XMLStr.SmartHome))
  {
     *static_cast<int *>(Param)=(Data.Options&E_SmartHome_On)?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.ProcessQuote))
  {
     *static_cast<int *>(Param)=(Data.Options&E_ProcessQuote_On)?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.QuoteEOL))
  {
     *static_cast<int *>(Param)=(Data.Options&E_QuoteEOL_On)?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.SaveFilePos))
  {
     *static_cast<int *>(Param)=(Data.Options&E_SaveFilePos_On)?1:
       ((Data.Options&E_SaveFilePos_Off)?0:2);
  }
  else if(!lstrcmp(szName, XMLStr.LockMode))
  {
     *static_cast<int *>(Param)=(Data.Options&E_LockMode_On)?1:
       ((Data.Options&E_LockMode_Off)?0:2);
  }
  else if(!lstrcmp(szName, XMLStr.MinLinesNum))
  {
     *static_cast<int *>(Param)=Data.MinLinesNum;
  }
  else if(!lstrcmp(szName, XMLStr.AddSymbol))
  {
    *static_cast<int *>(Param)=(Data.Options&E_AddSymbol_On)?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.LockFile))
  {
    *static_cast<int *>(Param)=Data.LockFile.IsOn()?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.WordSym))
  {
    *static_cast<int *>(Param)=(Data.Options2&E_WordSym_On)?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.AlphaNum))
  {
    *static_cast<int *>(Param)=(Data.Options2&E_AlphaNum_On)?1:0;
  }
  else if(!lstrcmp(szName, XMLStr.Additional) && Param!=NULL)
  {
    wcscpy(static_cast<wchar_t *>(Param), Data.AdditionalLetters.str);
  }
  else if(!lstrcmp(szName, XMLStr.ShowWhiteSpace))
  {
     *static_cast<int *>(Param)=(Data.Options2&E_Show_White_Space_On)?1:
       ((Data.Options2&E_Show_White_Space_Off)?0:2);
  }
  else if(!lstrcmp(szName, XMLStr.Bom))
  {
     *static_cast<int *>(Param)=(Data.Options2&E_Bom_On)?1:
       ((Data.Options2&E_Bom_Off)?0:2);
  }
  else
    RetCode=FALSE;

  return RetCode;
}

int WINAPI SetEditorOptionW(int EditorID, const wchar_t *szName, void *Param)
{
  if(!szName || !Param || *szName==0 || !NodeData || !ESETStorage)
    return FALSE;

  EditorSettingsStorage ESS(EditorID);
  Node_<EditorSettingsStorage> *Item=ESETStorage->findNode(ESS);
  if(NULL==Item) return FALSE;

  NODEDATA &Data=Item->data.Data;
  const NODEDATA oldData(Data);
  int RetCode=TRUE, RCisFALSEbutApplyIs=FALSE, param=*static_cast<int *>(Param);
  DWORD Type(-1); //EDITOR_SETPARAMETER_TYPES

  if(!lstrcmp(szName, XMLStr.TabSize))
  {
    Data.TabSize=param;
    Type=ESPT_TABSIZE;
  }
  else if(!lstrcmp(szName, XMLStr.ExpandTabs))
  {
    Data.Options&=~E_ExpandTabs_On;
    Data.Options&=~E_ExpandTabs_Off;
    Data.Options&=~E_ExpandTabs_OnlyNew;
    if(1==param)
      Data.Options|=E_ExpandTabs_On;
    else if(0==param)
      Data.Options|=E_ExpandTabs_Off;
    else if(3==param)
      Data.Options|=E_ExpandTabs_OnlyNew;
    Type=ESPT_EXPANDTABS;
  }
  else if(!lstrcmp(szName, XMLStr.CursorBEOL))
  {
    Data.Options&=~E_CursorBeyondEOL_On;
    Data.Options&=~E_CursorBeyondEOL_Off;
    Data.Options|=(param==1)?E_CursorBeyondEOL_On:
                  ((param==0)?E_CursorBeyondEOL_Off:0);
    Type=ESPT_CURSORBEYONDEOL;
  }
  else if(!lstrcmp(szName, XMLStr.AutoIndent))
  {
    Data.Options&=~E_AutoIndent_On;
    Data.Options&=~E_AutoIndent_Off;
    Data.Options|=(param==1)?E_AutoIndent_On:
                  ((param==0)?E_AutoIndent_Off:0);
    Type=ESPT_AUTOINDENT;
  }
  else if(!lstrcmp(szName, XMLStr.CharCodeBase))
  {
    Data.Options&=~E_CharCodeBase_Dec;
    Data.Options&=~E_CharCodeBase_Oct;
    Data.Options&=~E_CharCodeBase_Hex;
    Data.Options|=(param==10)?E_CharCodeBase_Dec:
                   ((param==8)?E_CharCodeBase_Oct:
                    ((param==16)?E_CharCodeBase_Hex:0)
                   );
    Type=ESPT_CHARCODEBASE;
  }
  else if(!lstrcmp(szName, XMLStr.KillSpaces))
  {
    if(param)
      Data.Options|=E_KillSpaces_On;
    else
    {
      Data.Options&=~E_KillSpaces_On;
      Data.Options&=~E_ProcessKeyEnd_On;
    }
  }
  else if(!lstrcmp(szName, XMLStr.p_Minuses))
  {
    if(param) Data.Options|=E_Process_Minuses_On;
    else      Data.Options&=~E_Process_Minuses_On;
  }
  else if(!lstrcmp(szName, XMLStr.KillEmptyLines))
  {
    if(param) Data.Options|=E_KillEmptyLines_On;
    else      Data.Options&=~E_KillEmptyLines_On;
  }
  else if(!lstrcmp(szName, XMLStr.ForceKillEmptyLines))
  {
    if(param) Data.Options|=E_ForceKillEL_On;
    else      Data.Options&=~E_ForceKillEL_On;
  }
  else if(!lstrcmp(szName, XMLStr.AutoWrap))
  {
    if(param) Data.Options|=E_AutoWrap_On;
    else      Data.Options&=~E_AutoWrap_On;
  }
  else if(!lstrcmp(szName, XMLStr.Wrap))
  {
    Data.Options2&=~E_Wrap_Percent;
    Data.WrapPos=param;
  }
  else if(!lstrcmp(szName, XMLStr.Justify))
  {
    if(param) Data.Options2|=E_Wrap_Justify;
    else      Data.Options2&=~E_Wrap_Justify;
  }
  else if(!lstrcmp(szName, XMLStr.eol))
  {
    Data.EOL=*static_cast<DWORD*>(Param);
  }
  else if(!lstrcmp(szName, XMLStr.Table))
  {
    Data.Table=*static_cast<DWORD *>(Param);
  }
  else if(!lstrcmp(szName, XMLStr.ProcessKeyEnd))
  {
    if(param)
    {
      RCisFALSEbutApplyIs=TRUE;
      Data.Options&=~E_ProcessKeyEnd_On;
      if(Data.Options&E_KillSpaces_On)
      {
        Data.Options|=E_ProcessKeyEnd_On;
        RetCode=TRUE;
      }
      else
        RetCode=FALSE;
    }
    else
      Data.Options&=~E_ProcessKeyEnd_On;
  }
  else if(!lstrcmp(szName, XMLStr.SmartTab))
  {
    if(param) Data.Options|=E_SmartTab_On;
    else      Data.Options&=~E_SmartTab_On;
  }
  else if(!lstrcmp(szName, XMLStr.SmartBackSpace))
  {
    if(param) Data.Options|=E_SmartBackSpace_On;
    else      Data.Options&=~E_SmartBackSpace_On;
  }
  else if(!lstrcmp(szName, XMLStr.Lines))
  {
    Data.Lines=param;
  }
  else if(!lstrcmp(szName, XMLStr.SmartHome))
  {
    if(param) Data.Options|=E_SmartHome_On;
    else      Data.Options&=~E_SmartHome_On;
  }
  else if(!lstrcmp(szName, XMLStr.ProcessQuote))
  {
    if(param) Data.Options|=E_ProcessQuote_On;
    else      Data.Options&=~E_ProcessQuote_On;
  }
  else if(!lstrcmp(szName, XMLStr.QuoteEOL))
  {
    if(param) Data.Options|=E_QuoteEOL_On;
    else      Data.Options&=~E_QuoteEOL_On;
  }
  else if(!lstrcmp(szName, XMLStr.SaveFilePos))
  {
    Data.Options&=~E_SaveFilePos_On;
    Data.Options&=~E_SaveFilePos_Off;
    Data.Options|=(param==1)?E_SaveFilePos_On:
                  ((param==0)?E_SaveFilePos_Off:0);
    Type=ESPT_SAVEFILEPOSITION;
  }
  else if(!lstrcmp(szName, XMLStr.LockMode))
  {
    Data.Options&=~E_LockMode_On;
    Data.Options&=~E_LockMode_Off;
    Data.Options|=(param==1)?E_LockMode_On:
                  ((param==0)?E_LockMode_Off:0);
    Type=ESPT_LOCKMODE;
  }
  else if(!lstrcmp(szName, XMLStr.MinLinesNum))
  {
    Data.MinLinesNum=param;
    Type=ESPT_SAVEFILEPOSITION;
  }
  else if(!lstrcmp(szName, XMLStr.AddSymbol))
  {
    if(param) Data.Options|=E_AddSymbol_On;
    else      Data.Options&=~E_AddSymbol_On;
  }
  else if(!lstrcmp(szName, XMLStr.LockFile))
  {
    if(param)
    {
      Data.Options|=E_LockFile_On;
      Data.LockFile.On();
    }
    else
    {
      Data.Options&=~E_LockFile_On;
      Data.LockFile.Off();
    }
  }
  else if(!lstrcmp(szName, XMLStr.WordSym))
  {
    if(param) Data.Options2|=E_WordSym_On;
    else      Data.Options2&=~E_WordSym_On;
    Type=ESPT_SETWORDDIV;
  }
  else if(!lstrcmp(szName, XMLStr.AlphaNum))
  {
    if(param) Data.Options2|=E_AlphaNum_On;
    else      Data.Options2&=~E_AlphaNum_On;
    Type=ESPT_SETWORDDIV;
  }
  else if(!lstrcmp(szName, XMLStr.Additional))
  {
    Data.AdditionalLetters=Param?static_cast<wchar_t*>(Param):L"";
    Type=ESPT_SETWORDDIV;
  }
  else if(!lstrcmp(szName, XMLStr.ShowWhiteSpace))
  {
    Data.Options2&=~E_Show_White_Space_On;
    Data.Options2&=~E_Show_White_Space_Off;
    Data.Options2|=(param==1)?E_Show_White_Space_On:
                  ((param==0)?E_Show_White_Space_Off:0);
    Type=ESPT_SHOWWHITESPACE;
  }
  else if(!lstrcmp(szName, XMLStr.Bom))
  {
    Data.Options2&=~E_Bom_On;
    Data.Options2&=~E_Bom_Off;
    Data.Options2|=(param==1)?E_Bom_On:
                  ((param==0)?E_Bom_Off:0);
    Type=ESPT_SETBOM;
  }
  else
    RetCode=FALSE;

  if(RetCode && Type!=static_cast<DWORD>(-1))
  {
    CEditorOptions eo(Data);
    RetCode=eo.ApplyOption(static_cast<EDITOR_SETPARAMETER_TYPES>(Type));
  }

  if(RetCode || RCisFALSEbutApplyIs)
    Item->data.index=-1;
  else
    Item->data.Data=oldData;

  return RetCode;
}

BOOL WINAPI CreateMultiMacroW(HANDLE *Macro, const wchar_t *Sequence, DWORD total)
{
  if(Macro)
  {
    if(!*Macro)
      *Macro=new KeySequenceStorage();
    if(*Macro)
    {
      int Error;
      strcon s;
      return static_cast<KeySequenceStorage*>(*Macro)->compile(Sequence,true,
        total,TRUE,Error,s);
    }
  }
  return FALSE;
}

BOOL WINAPI CreateMacroW(HANDLE *Macro, const wchar_t *Sequence)
{
  return CreateMultiMacroW(Macro,Sequence,1);
}

BOOL WINAPI CloneMacroW(const HANDLE Src, HANDLE *Dest)
{
  if(Dest && Src)
  {
    if(!*Dest)
      *Dest=new KeySequenceStorage();
    if(*Dest)
    {
      *static_cast<KeySequenceStorage*>(*Dest)=
        *static_cast<const KeySequenceStorage*>(Src);
      return TRUE;
    }
  }
  return FALSE;
}

BOOL WINAPI IsMacroOKW(const HANDLE Macro)
{
  return Macro?
    static_cast<const KeySequenceStorage*>(Macro)->IsOK():FALSE;
}

void WINAPI DeleteMacroW(HANDLE *Macro)
{
  if(Macro)
  {
     delete static_cast<KeySequenceStorage*>(*Macro);
     *Macro=NULL;
  }
}

BOOL WINAPI PostMacroW(const HANDLE Macro, BOOL silent)
{
   const KeySequenceStorage *Cmd=static_cast<const KeySequenceStorage*>(Macro);
   if(Cmd && Cmd->IsOK())
   {
      BOOL Stop;
      KeySequence KS=Cmd->GetSequence(Stop);
      KS.Flags=silent?KMFLAGS_DISABLEOUTPUT:0;
      return FARPostMacro(&KS);
   }
   return FALSE;
}

#ifdef __cplusplus
}
#endif
