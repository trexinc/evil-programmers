/*
    ab_main.cpp
    Copyright (C) 2000-2008 zg

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
#include <stdlib.h>
#include <ctype.h>
#include "far_settings.h"
#include "farcolor.hpp"
#include "memory.h"
#define AB_MAIN
#include "abplugin.h"
#include "ab_main.h"
#include "abversion.h"
#include <initguid.h>
#include "guid.h"
// {2C20419A-CBA1-4a15-AB4C-AFF61510DA0C}
DEFINE_GUID(MenuGuid, 0x2c20419a, 0xcba1, 0x4a15, 0xab, 0x4c, 0xaf, 0xf6, 0x15, 0x10, 0xda, 0xc);
// {91737241-105F-460a-A5A0-0EC56C235D27}
DEFINE_GUID(Config1Guid, 0x91737241, 0x105f, 0x460a, 0xa5, 0xa0, 0xe, 0xc5, 0x6c, 0x23, 0x5d, 0x27);
// {A338563B-914D-4cbd-97C1-DBDDAE78DC6F}
DEFINE_GUID(Config3Guid, 0xa338563b, 0x914d, 0x4cbd, 0x97, 0xc1, 0xdb, 0xdd, 0xae, 0x78, 0xdc, 0x6f);
// {9F54C244-3CD2-4517-9A47-F82EB8C6C495}
DEFINE_GUID(Config5Guid, 0x9f54c244, 0x3cd2, 0x4517, 0x9a, 0x47, 0xf8, 0x2e, 0xb8, 0xc6, 0xc4, 0x95);
// {C6769357-6E93-4fa2-AB77-CB0AA068DAD9}
DEFINE_GUID(OpenMenuGuid, 0xc6769357, 0x6e93, 0x4fa2, 0xab, 0x77, 0xcb, 0xa, 0xa0, 0x68, 0xda, 0xd9);
// {D6FC23C7-B1F3-4f19-BAAC-72600282816A}
DEFINE_GUID(Config1MenuGuid, 0xd6fc23c7, 0xb1f3, 0x4f19, 0xba, 0xac, 0x72, 0x60, 0x2, 0x82, 0x81, 0x6a);
// {F4501773-3912-4c7e-9F0A-CBA290A60210}
DEFINE_GUID(Config3MenuGuid, 0xf4501773, 0x3912, 0x4c7e, 0x9f, 0xa, 0xcb, 0xa2, 0x90, 0xa6, 0x2, 0x10);
// {36B7F4A1-3A5D-48cd-AFB5-F9F3A8BB490E}
DEFINE_GUID(Config5MenuGuid, 0x36b7f4a1, 0x3a5d, 0x48cd, 0xaf, 0xb5, 0xf9, 0xf3, 0xa8, 0xbb, 0x49, 0xe);
// {31429939-7646-4459-8315-C05973DC9855}
DEFINE_GUID(Config6MenuGuid, 0x31429939, 0x7646, 0x4459, 0x83, 0x15, 0xc0, 0x59, 0x73, 0xdc, 0x98, 0x55);
// {83992D56-B22B-417a-A392-A17FCEF40DEC}
DEFINE_GUID(Config7MenuGuid, 0x83992d56, 0xb22b, 0x417a, 0xa3, 0x92, 0xa1, 0x7f, 0xce, 0xf4, 0xd, 0xec);


PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

struct Options Opt={true,50000,0,100};

#define GetCheck(i) static_cast<int>(Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0))
#define GetDataPtr(i) reinterpret_cast<const TCHAR*>(Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))

static TCHAR hotkeys[]=_T("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

const TCHAR* GetMsg(int MsgId)
{
  return(Info.GetMsg(&MainGuid,MsgId));
}

TCHAR* GetCommaWord(TCHAR* Src,TCHAR* Word)
{
  int WordPos,SkipBrackets;
  if (*Src==0)
    return(NULL);
  SkipBrackets=false;
  for (WordPos=0;*Src!=0;Src++,WordPos++)
  {
    if (*Src=='['&&_tcschr(Src+1,']')!=NULL)
      SkipBrackets=true;
    if (*Src==']')
      SkipBrackets=false;
    if (*Src==','&&!SkipBrackets)
    {
      Word[WordPos]=0;
      Src++;
      while (_istspace(*Src))
        Src++;
      return(Src);
    }
    else
      Word[WordPos]=*Src;
  }
  Word[WordPos]=0;
  return(Src);
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=MAKEFARVERSION(VER_MAJOR,VER_MINOR,0,VER_BUILD,VER_TYPE);
  Info->Guid=MainGuid;
  Info->Title=L"AirBrush";
  Info->Description=L"Syntax highlighting in editor";
  Info->Author=L"Vadim Yegorov";
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>sizeof(::Info))?sizeof(::Info):Info->StructSize);

  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;

  OnLoad();
  {
    CFarSettings settings(MainGuid);
    Opt.Active=settings.Get(_T("Active"),Opt.Active);
    Opt.MaxLines=settings.Get(_T("MaxLines"),Opt.MaxLines);
    Opt.Priority=settings.Get(_T("Priority"),Opt.Priority);
    Opt.BrPriority=settings.Get(_T("BrPriority"),Opt.BrPriority);
  }
  LoadPlugs(Info->ModuleName);
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
  static const TCHAR* PluginMenuStrings;
  memset(Info,0,sizeof(*Info));
  Info->Flags = PF_EDITOR | PF_DISABLEPANELS;
  Info->StructSize = sizeof(*Info);
  Info->PluginConfig.Guids = &MenuGuid;
  Info->PluginMenu.Guids = &MenuGuid;
  Info->PluginConfig.Count = 1;
  Info->PluginMenu.Count = 1;
  PluginMenuStrings = GetMsg(mName);
  Info->PluginConfig.Strings = &PluginMenuStrings;
  Info->PluginMenu.Strings = &PluginMenuStrings;
}

void WINAPI ExitFARW(const struct ExitInfo *Info)
{
  (void)Info;
  OnExit();
  UnloadPlugs();
}

static void WINAPI MacroCallback(void* CallbackData,FarMacroValue* Values,size_t Count)
{
  (void)Values;
  (void)Count;
  free(CallbackData);
}

static intptr_t GetValue(FarMacroValue* value,intptr_t def=0)
{
  intptr_t result=def;
  if(FMVT_INTEGER==value->Type)
    result=value->Integer;
  else if(FMVT_DOUBLE==value->Type)
    result=static_cast<intptr_t>(value->Double);
  return result;
}

HANDLE WINAPI OpenW(const struct OpenInfo *Info)
{
  if((Info->OpenFrom==OPEN_EDITOR)&&(PluginsCount))
  {
    int Count=0;
    for(size_t i=0;i<PluginsCount;i++)
      if(PluginsData[i].Params&PAR_SHOW_IN_LIST)
        Count++;

    int *ids=static_cast<int *>(malloc((Count+1)*sizeof(int)));
    if(ids)
    {
      FarMenuItem *SyntaxTypes=NULL;
      size_t size=sizeof(FarMenuItem)*(Count+1)+(Count+1)*128*sizeof(TCHAR);
      SyntaxTypes=static_cast<FarMenuItem *>(malloc(size));
      if(SyntaxTypes)
      {
        TCHAR* data=reinterpret_cast<TCHAR*>((reinterpret_cast<char*>(SyntaxTypes))+sizeof(FarMenuItem)*(Count+1));
        TCHAR* text=data;
        SyntaxTypes[0].Text=text;
        EditorInfo ei;
        ei.StructSize=sizeof(ei);
        PEditFile curfile;
        int index=-1;
        ::Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
        curfile=ef_getfile(ei.EditorID);
        if(curfile) index=curfile->type+1;
        wsprintf(text,_T("%c. %s"),hotkeys[0],GetMsg(mDefault));
        if(!index) SyntaxTypes[0].Flags|='*'|MIF_CHECKED;
        ids[0]=-1;
        index--;
        for(size_t i=0,j=1;i<PluginsCount;i++)
        {
          if(PluginsData[i].Params&PAR_SHOW_IN_LIST)
          {
            text=data+j*128;
            SyntaxTypes[j].Text=text;
            if(j<static_cast<size_t>(lstrlen(hotkeys)-1))
              wsprintf(text,_T("%c. %s"),hotkeys[j],PluginsData[i].Name);
            else
              wsprintf(text,_T("%c. %s"),_T(' '),PluginsData[i].Name);
            if(static_cast<size_t>(index)==i) SyntaxTypes[j].Flags|='*'|MIF_CHECKED;
            ids[j]=i;
            j++;
          }
        }
        intptr_t MenuCode=0,BreakCode;
        FarKey BreakKeys[]={{VK_F4,0},{0,0}};
        while(true)
        {
          for(int i=0;i<=Count;i++)
            SyntaxTypes[i].Flags&=~MIF_SELECTED;
          SyntaxTypes[MenuCode].Flags|=MIF_SELECTED;
          BreakCode=-1;
          MenuCode=::Info.Menu(&MainGuid,&OpenMenuGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,_T(""),NULL,NULL,BreakKeys,&BreakCode,SyntaxTypes,Count+1);
          if(BreakCode==-1) break;
          if((BreakCode==0)&&(MenuCode>0))
            if(PluginsData[ids[MenuCode]].pGetParams)
              PluginsData[ids[MenuCode]].pGetParams(PluginsData[ids[MenuCode]].Index,PAR_CONFIGURE1,NULL);
        }
        if(MenuCode>-1)
        {
          EditorInfo ei;
          ei.StructSize=sizeof(ei);
          ::Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
          ef_deletefile(ei.EditorID);
          loadfile(ei.EditorID,ids[MenuCode]);
        }
        free(SyntaxTypes);
      }
      free(ids);
    }
  }
  else if(Info->OpenFrom==OPEN_FROMMACRO)
  {
    const long long max52=(1LL<<52)-1;
    OpenMacroInfo* mi=reinterpret_cast<OpenMacroInfo*>(Info->Data);
    if(mi->Count)
    {
      intptr_t op=GetValue(mi->Values);
      intptr_t eid=-1;
      if(mi->Count>1) eid=GetValue(mi->Values+1,-1);
      if(-1==eid)
      {
        EditorInfo ei;
        ei.StructSize=sizeof(ei);
        ::Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
        eid=ei.EditorID;
      }
      if(0==op)
      {
        if(mi->Count>2&&(FMVT_STRING==mi->Values[2].Type||(FMVT_BINARY==mi->Values[2].Type&&sizeof(GUID)==mi->Values[2].Binary.Size)))
        {
          GUID syntax;
          bool res=true;
          if(FMVT_STRING==mi->Values[2].Type)
            res=RPC_S_OK==UuidFromString(reinterpret_cast<short unsigned*>(const_cast<wchar_t*>(mi->Values[2].String)),&syntax);
          else
            memcpy(&syntax,mi->Values[2].Binary.Data,sizeof(syntax));
          if(res)
          {
            for(ssize_t ii=-1;ii<static_cast<ssize_t>(PluginsCount);++ii)
              if(syntax==(-1==ii?FarGuid:PluginsData[ii].Id))
              {
                ef_deletefile(eid);
                loadfile(eid,ii);
                return reinterpret_cast<HANDLE>(1);
              }
          }
        }
      }
      else
      {
        PEditFile fl=ef_getfile(eid);
        if(fl)
        {
          switch(op)
          {
            case 1:
              if(fl->type>=0&&PluginsData[fl->type].Params&PAR_BRACKETS)
              {
                fl->full=true;
                ::Info.EditorControl(eid,ECTL_REDRAW,0,NULL);
                if(fl->bracket[0].active||fl->bracket[1].active)
                {
                  const size_t count=(1+4*ArraySize(fl->bracket));
                  FarMacroCall* fmc=reinterpret_cast<FarMacroCall*>(malloc(sizeof(FarMacroCall)+count*sizeof(FarMacroValue)));
                  if(fmc)
                  {
                    fmc->StructSize=sizeof(*fmc);
                    fmc->Count=count;
                    fmc->Values=reinterpret_cast<FarMacroValue*>(fmc+1);
                    fmc->Callback=MacroCallback;
                    fmc->CallbackData=fmc;
                    fmc->Values[0]=true;
                    for(size_t ii=0;ii<ArraySize(fl->bracket);++ii)
                    {
                      size_t jj=ii*4;
                      fmc->Values[jj+1]=fl->bracket[ii].x+1;
                      fmc->Values[jj+2]=fl->bracket[ii].y+1;
                      fmc->Values[jj+3]=fl->bracket[ii].len;
                      fmc->Values[jj+4]=fl->bracket[ii].active;
                    }
                    return fmc;
                  }
                }
              }
              break;
            case 2:
              {
                FarMacroCall* fmc=reinterpret_cast<FarMacroCall*>(malloc(sizeof(FarMacroCall)+3*sizeof(FarMacroValue)));
                if(fmc)
                {
                  fmc->StructSize=sizeof(*fmc);
                  fmc->Count=3;
                  fmc->Values=reinterpret_cast<FarMacroValue*>(fmc+1);
                  fmc->Callback=MacroCallback;
                  fmc->CallbackData=fmc;
                  fmc->Values[0]=true;
                  fmc->Values[1]=(fl->type>=0)?PluginsData[fl->type].Id:FarGuid;
                  fmc->Values[2]=(fl->apitopline<max52)?fl->apitopline+1:max52;
                  return fmc;
                }
              }
              break;
          }
        }
      }
    }
  }
  return NULL;
}

intptr_t WINAPI Config1DialogProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

intptr_t WINAPI ConfigureW(const struct ConfigureInfo *anInfo)
{
  (void)anInfo;
  FarMenuItem MenuItems[4];
  memset(MenuItems,0,sizeof(MenuItems));
  TCHAR data[4*128];
  memset(data,0,sizeof(data));
  int Msgs[]={mConfigMenu1,mConfigMenu2,mConfigMenu3,mConfigMenu4};

  for(unsigned int i=0;i<sizeof(MenuItems)/sizeof(MenuItems[0]);i++)
  {
    TCHAR* text=data+i*128;
    MenuItems[i].Text=text;
    wsprintf(text,_T("%s"),GetMsg(Msgs[i])); // Text in menu
  };
  int MenuCode=0;
  while(true)
  {
    for(unsigned int i=0;i<sizeof(MenuItems)/sizeof(MenuItems[0]);i++)
      MenuItems[i].Flags&=~MIF_SELECTED;
    MenuItems[MenuCode].Flags|=MIF_SELECTED;
    // Show menu
    MenuCode=Info.Menu(&MainGuid,&Config1MenuGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,
                           NULL,NULL,_T("Config"),NULL,NULL,
                           MenuItems,sizeof(MenuItems)/sizeof(MenuItems[0]));
    if(MenuCode==-1)
      break;
    else if(MenuCode==0)
    {
      enum {IBox,IActive,IMaxLinesLabel,IMaxLines,IPriorityLabel,IPriority,IBrPriorityLabel,IBrPriority,IDummy,ISave,ICancel};
      /*
        0000000000111111111122222222223333333333444444444455555555556666666666777777
        0123456789012345678901234567890123456789012345678901234567890123456789012345
      00                                                                            00
      01   浜様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様融   01
      02   � [x] Turn on plugin module                                          �   02
      03   � Don't colorize files with line count greater then:       栩栩栩栩� �   03
      04   把陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳超   04
      05   �                 [ Save ]              [ Cancel ]                   �   05
      06   藩様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様夕   06
      07                                                                            07
        0000000000111111111122222222223333333333444444444455555555556666666666777777
        0123456789012345678901234567890123456789012345678901234567890123456789012345
      */
      struct FarDialogItem DialogItems[]={
      /* 0*/  {DI_DOUBLEBOX,3 ,1,72,8,{0},NULL,NULL,0,                                GetMsg(mName)                   ,0,0,{0,0}},
      /* 1*/  {DI_CHECKBOX ,5 ,2,0 ,0,{0},NULL,NULL,DIF_FOCUS,                        GetMsg(mConfigDialogActive)     ,0,0,{0,0}},
      /* 2*/  {DI_TEXT     ,5 ,3,0 ,0,{0},NULL,NULL,0,                                GetMsg(mConfigDialogMaxLines)   ,0,0,{0,0}},
      /* 3*/  {DI_FIXEDIT  ,62,3,70,0,{0},NULL,NULL,DIF_MASKEDIT,                     _T("")                          ,0,0,{0,0}},
      /* 4*/  {DI_TEXT     ,5 ,4,0 ,0,{0},NULL,NULL,0,                                GetMsg(mConfigDialogPriority)   ,0,0,{0,0}},
      /* 5*/  {DI_FIXEDIT  ,62,4,70,0,{0},NULL,NULL,DIF_MASKEDIT,                     _T("")                          ,0,0,{0,0}},
      /* 6*/  {DI_TEXT     ,5 ,5,0 ,0,{0},NULL,NULL,0,                                GetMsg(mConfigDialogBrPriority) ,0,0,{0,0}},
      /* 7*/  {DI_FIXEDIT  ,62,5,70,0,{0},NULL,NULL,DIF_MASKEDIT,                     _T("")                          ,0,0,{0,0}},
      /* 8*/  {DI_TEXT     ,-1,6,0 ,0,{0},NULL,NULL,DIF_SEPARATOR,                    _T("")                          ,0,0,{0,0}},
      /* 9*/  {DI_BUTTON   ,0 ,7,0 ,0,{0},NULL,NULL,DIF_CENTERGROUP|DIF_DEFAULTBUTTON,GetMsg(mConfigSave)             ,0,0,{0,0}},
      /*10*/  {DI_BUTTON   ,0 ,7,0 ,0,{0},NULL,NULL,DIF_CENTERGROUP,                  GetMsg(mConfigCancel)           ,0,0,{0,0}}
      };
      DialogItems[IActive].Selected=Opt.Active;
      DialogItems[IMaxLines].Mask=DialogItems[IPriority].Mask=DialogItems[IBrPriority].Mask=_T("########9");

      const size_t count=3;
      TCHAR lines[count][32];
      const size_t edits[count]={IMaxLines,IPriority,IBrPriority};
      __int64 opts[count]={Opt.MaxLines,Opt.Priority,Opt.BrPriority};
      for(size_t ii=0;ii<count;++ii)
      {
        DialogItems[edits[ii]].Data=lines[ii];
        wsprintf(lines[ii],_T("%d"),opts[ii]);
      }
      int DlgCode=-1;
      HANDLE hDlg=Info.DialogInit(&MainGuid,&Config1Guid,-1,-1,76,10,_T("Config1"),DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,Config1DialogProc,0);
      if(hDlg!=INVALID_HANDLE_VALUE) DlgCode=Info.DialogRun(hDlg);
      if(DlgCode==ISave)
      {
        Opt.Active=GetCheck(IActive);
        Opt.MaxLines=FSF.atoi(GetDataPtr(IMaxLines));
        Opt.Priority=FSF.atoi(GetDataPtr(IPriority));
        Opt.BrPriority=FSF.atoi(GetDataPtr(IBrPriority));

        {
          CFarSettings settings(MainGuid);
          settings.Set(_T("Active"),Opt.Active);
          settings.Set(_T("MaxLines"),Opt.MaxLines);
          settings.Set(_T("Priority"),Opt.Priority);
          settings.Set(_T("BrPriority"),Opt.BrPriority);
        }
      }
      if(hDlg!=INVALID_HANDLE_VALUE) Info.DialogFree(hDlg);
    }
    else if(MenuCode==1)
    {
      size_t ConfCount=0;
      for(size_t i=0;i<PluginsCount;i++)
        if(PluginsData[i].Params&PAR_MASK_STORE)
          ConfCount++;
      if(ConfCount)
      {
        int *ids=reinterpret_cast<int*>(malloc(ConfCount*sizeof(int)));
        if(ids)
        {
          FarMenuItem *SyntaxTypes=NULL;
          size_t size=sizeof(FarMenuItem)*ConfCount+ConfCount*128*sizeof(TCHAR);
          SyntaxTypes=reinterpret_cast<FarMenuItem *>(malloc(size));
          if(SyntaxTypes)
          {
            TCHAR* data=reinterpret_cast<TCHAR*>((reinterpret_cast<char*>(SyntaxTypes))+sizeof(FarMenuItem)*ConfCount);
            for(size_t i=0,j=0;(i<PluginsCount)&&(j<ConfCount);i++)
            {
              if(PluginsData[i].Params&PAR_MASK_STORE)
              {
                TCHAR* text=data+j*128;
                SyntaxTypes[j].Text=text;
                if(j<static_cast<size_t>(lstrlen(hotkeys)))
                  wsprintf(text,_T("%c. %s"),hotkeys[j],PluginsData[i].Name);
                else
                  wsprintf(text,_T("%c. %s"),' ',PluginsData[i].Name);
                ids[j]=i;
                j++;
              }
            }
            int MenuCode=0;
            while(true)
            {
              for(size_t i=0;i<ConfCount;i++)
                SyntaxTypes[i].Flags&=~MIF_SELECTED;
              SyntaxTypes[MenuCode].Flags|=MIF_SELECTED;

              MenuCode=Info.Menu(&MainGuid,&Config3MenuGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,_T(""),NULL,_T("Config2"),NULL,NULL,SyntaxTypes,ConfCount);
              if(MenuCode==-1) break;
              TCHAR mask[2048];
              lstrcpy(mask,_T(""));
              if(PluginsData[ids[MenuCode]].Mask)
                lstrcpy(mask,PluginsData[ids[MenuCode]].Mask);
              struct FarDialogItem DialogItems[]={
              /*0*/  {DI_DOUBLEBOX, 3,1,72,5,{0},NULL,NULL,0,                                _T("")               ,0,0,{0,0}},
              /*1*/  {DI_EDIT,      5,2,70,0,{0},NULL,NULL,DIF_FOCUS,                        _T("")               ,0,0,{0,0}},
              /*2*/  {DI_TEXT,     -1,3,0 ,0,{0},NULL,NULL,DIF_SEPARATOR,                    _T("")               ,0,0,{0,0}},
              /*3*/  {DI_BUTTON,    0,4,0 ,0,{0},NULL,NULL,DIF_CENTERGROUP|DIF_DEFAULTBUTTON,GetMsg(mConfigSave)  ,0,0,{0,0}},
              /*4*/  {DI_BUTTON,    0,4,0 ,0,{0},NULL,NULL,DIF_CENTERGROUP,                  GetMsg(mConfigCancel),0,0,{0,0}}
              };
              DialogItems[1].MaxLength=sizeof(mask);
              DialogItems[1].Data=mask;
              DialogItems[0].Data=PluginsData[ids[MenuCode]].Name;
              int DlgCode=-1;
              HANDLE hDlg=Info.DialogInit(&MainGuid,&Config3Guid,-1,-1,76,7,_T("Config3"),DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,Config1DialogProc,0);
              if(hDlg!=INVALID_HANDLE_VALUE) DlgCode=Info.DialogRun(hDlg);
              if(DlgCode==3)
              {
                if(PluginsData[ids[MenuCode]].Params&PAR_MASK_STORE)
                {
                  CFarSettings settings(MainGuid);
                  settings.Change(PLUGIN_MASK_KEY);
                  settings.Set(PluginsData[ids[MenuCode]].IdStr,GetDataPtr(1));
                }
                free(PluginsData[ids[MenuCode]].Mask);
                PluginsData[ids[MenuCode]].Mask=reinterpret_cast<TCHAR*>(malloc((lstrlen(GetDataPtr(1))+1)*sizeof(TCHAR)));
                if(PluginsData[ids[MenuCode]].Mask)
                  lstrcpy(PluginsData[ids[MenuCode]].Mask,GetDataPtr(1));
              }
              if(hDlg!=INVALID_HANDLE_VALUE) Info.DialogFree(hDlg);
            }
            free(SyntaxTypes);
          }
          free(ids);
        }
      }
    }
    else if(MenuCode==2)
    {
      size_t ConfCount=0;
      for(size_t i=0;i<PluginsCount;i++)
        if(PluginsData[i].Params&PAR_FILESTART_STORE)
          ConfCount++;
      if(ConfCount)
      {
        int *ids=reinterpret_cast<int*>(malloc(ConfCount*sizeof(int)));
        if(ids)
        {
          FarMenuItem *SyntaxTypes=NULL;
          size_t size=sizeof(FarMenuItem)*ConfCount+ConfCount*128*sizeof(TCHAR);
          SyntaxTypes=reinterpret_cast<FarMenuItem*>(malloc(size));
          if(SyntaxTypes)
          {
            TCHAR* data=reinterpret_cast<TCHAR*>((reinterpret_cast<char*>(SyntaxTypes))+sizeof(FarMenuItem)*ConfCount);
            for(size_t i=0,j=0;(i<PluginsCount)&&(j<ConfCount);i++)
            {
              if(PluginsData[i].Params&PAR_FILESTART_STORE)
              {
                TCHAR* text=data+j*128;
                SyntaxTypes[j].Text=text;
                if(j<static_cast<size_t>(lstrlen(hotkeys)))
                  wsprintf(text,_T("%c. %s"),hotkeys[j],PluginsData[i].Name);
                else
                  wsprintf(text,_T("%c. %s"),' ',PluginsData[i].Name);
                ids[j]=i;
                j++;
              }
            }
            int MenuCode=0;
            while(true)
            {
              for(size_t i=0;i<ConfCount;i++)
                SyntaxTypes[i].Flags&=~MIF_SELECTED;
              SyntaxTypes[MenuCode].Flags|=MIF_SELECTED;

              MenuCode=Info.Menu(&MainGuid,&Config5MenuGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,_T(""),NULL,_T("Config4"),NULL,NULL,SyntaxTypes,ConfCount);
              if(MenuCode==-1) break;
              TCHAR start[2048];
              lstrcpy(start,_T(""));
              if(PluginsData[ids[MenuCode]].Start)
                lstrcpy(start,PluginsData[ids[MenuCode]].Start);
              struct FarDialogItem DialogItems[]={
              /*0*/  {DI_DOUBLEBOX, 3,1,72,5,{0},NULL,NULL,0,                                _T("")               ,0,0,{0,0}},
              /*1*/  {DI_EDIT,      5,2,70,0,{0},NULL,NULL,DIF_FOCUS,                        _T("")               ,0,0,{0,0}},
              /*2*/  {DI_TEXT,     -1,3,0 ,0,{0},NULL,NULL,DIF_SEPARATOR,                    _T("")               ,0,0,{0,0}},
              /*3*/  {DI_BUTTON,    0,4,0 ,0,{0},NULL,NULL,DIF_CENTERGROUP|DIF_DEFAULTBUTTON,GetMsg(mConfigSave)  ,0,0,{0,0}},
              /*4*/  {DI_BUTTON,    0,4,0 ,0,{0},NULL,NULL,DIF_CENTERGROUP,                  GetMsg(mConfigCancel),0,0,{0,0}}
              };
              DialogItems[1].MaxLength=sizeof(start);
              DialogItems[1].Data=start;
              DialogItems[0].Data=PluginsData[ids[MenuCode]].Name;
              int DlgCode=-1;
              HANDLE hDlg=Info.DialogInit(&MainGuid,&Config5Guid,-1,-1,76,7,_T("Config5"),DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,Config1DialogProc,0);
              if(hDlg!=INVALID_HANDLE_VALUE) DlgCode=Info.DialogRun(hDlg);
              if(DlgCode==3)
              {
                if(PluginsData[ids[MenuCode]].Params&PAR_FILESTART_STORE)
                {
                  CFarSettings settings(MainGuid);
                  settings.Change(PLUGIN_START_KEY);
                  settings.Set(PluginsData[ids[MenuCode]].IdStr,GetDataPtr(1));
                }
                free(PluginsData[ids[MenuCode]].Start);
                PluginsData[ids[MenuCode]].Start=reinterpret_cast<TCHAR*>(malloc((lstrlen(GetDataPtr(1))+1)*sizeof(TCHAR)));
                if(PluginsData[ids[MenuCode]].Start)
                  lstrcpy(PluginsData[ids[MenuCode]].Start,GetDataPtr(1));
              }
              if(hDlg!=INVALID_HANDLE_VALUE) Info.DialogFree(hDlg);
            }
            free(SyntaxTypes);
          }
          free(ids);
        }
      }
    }
    else if(MenuCode==3)
    {
      size_t ConfCount=0;
      for(size_t i=0;i<PluginsCount;i++)
        if(PluginsData[i].Params&PAR_COLORS_STORE)
          ConfCount++;
      if(ConfCount)
      {
        int *ids=reinterpret_cast<int*>(malloc(ConfCount*sizeof(int)));
        if(ids)
        {
          FarMenuItem *SyntaxTypes=NULL;
          size_t size=sizeof(FarMenuItem)*ConfCount+ConfCount*128*sizeof(TCHAR);
          SyntaxTypes=reinterpret_cast<FarMenuItem*>(malloc(size));
          if(SyntaxTypes)
          {
            TCHAR* data=reinterpret_cast<TCHAR*>((reinterpret_cast<char*>(SyntaxTypes))+sizeof(FarMenuItem)*ConfCount);
            for(size_t i=0,j=0;(i<PluginsCount)&&(j<ConfCount);i++)
            {
              if(PluginsData[i].Params&PAR_COLORS_STORE)
              {
                TCHAR* text=data+j*128;
                SyntaxTypes[j].Text=text;
                if(j<static_cast<size_t>(lstrlen(hotkeys)))
                  wsprintf(text,_T("%c. %s"),hotkeys[j],PluginsData[i].Name);
                else
                  wsprintf(text,_T("%c. %s"),' ',PluginsData[i].Name);
                ids[j]=i;
                j++;
              }
            }
            int MenuCode=0;
            while(true)
            {
              for(size_t i=0;i<ConfCount;i++)
                SyntaxTypes[i].Flags&=~MIF_SELECTED;
              SyntaxTypes[MenuCode].Flags|=MIF_SELECTED;

              MenuCode=Info.Menu(&MainGuid,&Config6MenuGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,_T(""),NULL,_T("Config6"),NULL,NULL,SyntaxTypes,ConfCount);
              if(MenuCode==-1) break;
              size_t ColorCount; char **ColorNames; FarColor* Colors;
              if(PluginsData[ids[MenuCode]].pGetParams&&PluginsData[ids[MenuCode]].pGetParams(PluginsData[ids[MenuCode]].Index,PAR_GET_COLOR_COUNT,reinterpret_cast<const char**>(&ColorCount))&&PluginsData[ids[MenuCode]].pGetParams(PluginsData[ids[MenuCode]].Index,PAR_GET_COLOR_NAME,const_cast<const char**>(reinterpret_cast<char**>(&ColorNames)))&&PluginsData[ids[MenuCode]].pGetParams(PluginsData[ids[MenuCode]].Index,PAR_GET_COLOR,const_cast<const char**>(reinterpret_cast<char**>(&Colors))))
              {
                FarMenuItem *ColorTypes=NULL;
                size_t size=sizeof(FarMenuItem)*ColorCount+ColorCount*128*sizeof(TCHAR);
                ColorTypes=reinterpret_cast<FarMenuItem*>(malloc(size));
                if(ColorTypes)
                {
                  TCHAR* data=reinterpret_cast<TCHAR*>((reinterpret_cast<char*>(ColorTypes))+sizeof(FarMenuItem)*ColorCount);
                  for(size_t i=0;i<ColorCount;i++)
                  {
                    TCHAR* text=data+i*128;
                    ColorTypes[i].Text=text;
                    if(i<static_cast<size_t>(lstrlen(hotkeys)))
                      wsprintf(text,_T("%c. %s"),hotkeys[i],ColorNames[i]);
                    else
                      wsprintf(text,_T("%c. %s"),' ',ColorNames[i]);
                  }
                  int ColorCode=0;
                  while(true)
                  {
                    for(size_t i=0;i<ColorCount;i++)
                      ColorTypes[i].Flags&=~MIF_SELECTED;
                    ColorTypes[ColorCode].Flags|=MIF_SELECTED;
                    ColorCode=Info.Menu(&MainGuid,&Config7MenuGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,PluginsData[ids[MenuCode]].Name,NULL,_T("Config7"),NULL,NULL,ColorTypes,ColorCount);
                    if(ColorCode==-1) break;
                    if(Info.ColorDialog(&MainGuid,CDF_NONE,Colors+ColorCode))
                    {
                      SaveColors(PluginsData[ids[MenuCode]].IdStr,Colors,ColorCount);
                    }
                  }
                }
              }
            }
            free(SyntaxTypes);
          }
          free(ids);
        }
      }
    }
  }
  return false;
}

intptr_t WINAPI ProcessEditorEventW(const struct ProcessEditorEventInfo *Info)
{
  return OnEditorEvent(Info->Event,Info->Param,Info->EditorID);
}

intptr_t WINAPI ProcessEditorInputW(const ProcessEditorInputInfo *Info)
{
  return OnEditorInput(&Info->Rec);
}

#ifdef __cplusplus
extern "C"{
#endif
  bool WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

bool WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void)hDll;
  (void)dwReason;
  (void)lpReserved;
  return true;
}
