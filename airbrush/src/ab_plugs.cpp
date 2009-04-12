/*
    ab_plugs.cpp
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
#include <tchar.h>
#include "plugin.hpp"
#include "farcolor.hpp"
#include "memory.h"
#include "ab_main.h"
#include "abplugin.h"
#include "abversion.h"

struct PluginItem *PluginsData=NULL;
int PluginsCount=0;

static TCHAR Unknown[20];

static void WINAPI addcolor(int lno,int start,int len,int fg,int bg)
{
  if((fg==-1)&&(bg==-1)) return;
  if(len==0) return;
  WaitForSingleObject(Mutex,INFINITE);
  if(bg==-1) bg=Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_EDITORTEXT)&0xF0;
  else bg=bg<<4;
  if(fg==-1) fg=Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_EDITORTEXT)&0x0F;
  EditorColor ec;
  ec.StringNumber=lno;
  ec.StartPos=start;
  ec.EndPos=start+len-1;
  ec.Color=fg|bg;
  Info.EditorControl(ECTL_ADDCOLOR,&ec);
  ReleaseMutex(Mutex);
}

static const TCHAR WINAPI *getline(int lno,int *len)
{
  EditorGetString egs;
  EditorSetPosition esp;
  esp.CurPos=-1; esp.CurTabPos=-1; esp.TopScreenLine=-1; esp.LeftPos=-1; esp.Overtype=-1;
  egs.StringNumber=-1;
  esp.CurLine=lno;
  WaitForSingleObject(Mutex,INFINITE);
  Info.EditorControl(ECTL_SETPOSITION,&esp);
  Info.EditorControl(ECTL_GETSTRING,&egs);
  ReleaseMutex(Mutex);
  if(len)
    *len=egs.StringLength;
  return egs.StringText;
}

static bool WINAPI addstate(int eid,int pos,unsigned long size,unsigned char *data)
{
  bool res=true;
  WaitForSingleObject(Mutex,INFINITE);
  PEditFile fl=ef_getfile(eid);
  if(fl)
  {
    if(pos==fl->cachesize)
    {
      StateCache **prev_cache=&(fl->cache);
      while(*prev_cache)
        prev_cache=&((*prev_cache)->next);
      *prev_cache=(StateCache *)malloc(sizeof(StateCache));
      if(*prev_cache)
      {
        if(size) (*prev_cache)->data=(unsigned char *)malloc(size);
        if((*prev_cache)->data)
        {
          (*prev_cache)->data_size=size;
          memcpy((*prev_cache)->data,data,size);
        }
        (fl->cachesize)++;
      }
    }
    else if((pos+1)!=fl->cachesize)
    {
      res=false;
      TCHAR buff[256];
      FSF.sprintf(buff,GetMsg(mFatalLine2),fl->cachesize,pos);
      const TCHAR* MsgItems[]={GetMsg(mError),GetMsg(mFatalLine1),buff,GetMsg(mButtonOk)};
      Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
    }
  }
  if(!res) fatal=true;
  ReleaseMutex(Mutex);
  return res;
}

static void WINAPI getcursor(int *row,int *col)
{
  if((cursor_row>=0)&&(cursor_col>=0))
  {
    *row=cursor_row;
    *col=cursor_col;
  }
  else
  {
    EditorInfo ei;
    WaitForSingleObject(Mutex,INFINITE);
    Info.EditorControl(ECTL_GETINFO,&ei);
    ReleaseMutex(Mutex);
    *row=ei.CurLine;
    *col=ei.CurPos;
  }
}

static void WINAPI callparser(const TCHAR *parser,struct ColorizeParams *params)
{
  for(int i=0;i<PluginsCount;i++)
    if(!lstrcmpi(parser,PluginsData[i].Name))
      if(PluginsData[i].pColorize)
        PluginsData[i].pColorize(PluginsData[i].Index,params);
}

void LoadPlugs(const TCHAR* ModuleName)
{
  HANDLE hSScr=Info.SaveScreen(0,0,-1,-1);
  {
    const TCHAR* MsgItems[]={GetMsg(mName),GetMsg(mLoading)};
    Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
  }
  TCHAR PluginsFolder[MAX_PATH],PluginsMask[MAX_PATH],*NamePtr;
  lstrcpy(Unknown,GetMsg(mUnknown));
  lstrcpy(PluginsFolder,ModuleName);
  NamePtr=(TCHAR*)FSF.PointToName(PluginsFolder);
  lstrcpy(NamePtr,_T("\\Formats\\"));
  FSF.sprintf(PluginsMask,_T("%s*.fmt"),PluginsFolder);

  HANDLE FindHandle;
  WIN32_FIND_DATA fdata;
  int Done=((FindHandle=FindFirstFile(PluginsMask,&fdata))==INVALID_HANDLE_VALUE);
  while(!Done)
  {
    TCHAR PluginName[NM];
    FSF.sprintf(PluginName,_T("%s%s"),PluginsFolder,fdata.cFileName);
    HMODULE hModule=LoadLibrary(PluginName);
    if (hModule!=NULL)
    {
      struct PluginItem CurPlugin;
      CurPlugin.hModule=hModule;
      CurPlugin.Type=0;
      CurPlugin.pLoadSyntaxModule=(PLUGINLOADSYNTAXMODULE)GetProcAddress(hModule,"LoadSyntaxModule");
      CurPlugin.pSetColorizeInfo=(PLUGINSETCOLORIZEINFO)GetProcAddress(hModule,"SetColorizeInfo");
      CurPlugin.pGetParams=(PLUGINGETPARAMS)GetProcAddress(hModule,"GetParams");
      CurPlugin.pColorize=(PLUGINCOLORIZE)GetProcAddress(hModule,"Colorize");
      CurPlugin.pInput=(PLUGININPUT)GetProcAddress(hModule,"Input");
      CurPlugin.pGetSyntaxCount=(PLUGINGETSYNTAXCOUNT)GetProcAddress(hModule,"GetSyntaxCount");
      CurPlugin.pExit=(PLUGINEXIT)GetProcAddress(hModule,"Exit");

      if(CurPlugin.pLoadSyntaxModule)
        CurPlugin.Type=CurPlugin.pLoadSyntaxModule(PluginName,&Info);
      int accept_plug=true;
      if(CurPlugin.pSetColorizeInfo)
      {
        ColorizeInfo lInfo;
        lInfo.size=sizeof(lInfo);
        lInfo.version=VER_ALL;
        lInfo.api=VER_API;
        lInfo.cachestr=PARSER_CACHESTR;
        lstrcpy(lInfo.folder,PluginsFolder);
        lInfo.pAddColor=addcolor;
        lInfo.pGetLine=getline;
        lInfo.pAddState=addstate;
        lInfo.pGetCursor=getcursor;
        lInfo.pCallParser=callparser;
        accept_plug=CurPlugin.pSetColorizeInfo(&lInfo);
      }

      if(accept_plug)
      {
        unsigned long subcount=1;
        if(CurPlugin.pGetSyntaxCount)
          subcount=CurPlugin.pGetSyntaxCount();

        if(subcount)
        {
          struct PluginItem *NewPluginsData=(struct PluginItem *)realloc(PluginsData,sizeof(*PluginsData)*(PluginsCount+subcount));
          if (NewPluginsData==NULL)
            break;
          PluginsData=NewPluginsData;
          for(unsigned long i=0;i<subcount;i++)
          {
            CurPlugin.Index=i;
            CurPlugin.Mask=NULL;
            CurPlugin.Start=NULL;
            CurPlugin.Params=0;
            const TCHAR *mask,*start,*name=NULL;
            TCHAR buff_mask[2048],buff_start[2048];
            lstrcpy(buff_mask,_T("")); lstrcpy(buff_start,_T(""));
            if(CurPlugin.pGetParams)
            {
              CurPlugin.Params=CurPlugin.pGetParams(CurPlugin.Index,PAR_GET_PARAMS,NULL);
              CurPlugin.pGetParams(CurPlugin.Index,PAR_GET_NAME,(const char**)&name);
              //load mask
              if(name&&(CurPlugin.Params&PAR_MASK_CACHE))
              {
                if(!CurPlugin.pGetParams(CurPlugin.Index,PAR_GET_MASK,(const char**)&mask))
                  mask=_T("");
                lstrcpy(buff_mask,mask);
                if(CurPlugin.Params&PAR_MASK_STORE)
                {
                  HKEY hKey; DWORD Type; DWORD DataSize=0;
                  if((RegOpenKeyEx(HKEY_CURRENT_USER,PluginMaskKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
                  {
                    DataSize=sizeof(buff_mask);
                    RegQueryValueEx(hKey,name,0,&Type,(LPBYTE)buff_mask,&DataSize);
                    RegCloseKey(hKey);
                  }
                }
              }
              //load starts
              if(name&&(CurPlugin.Params&PAR_FILESTART_CACHE))
              {
                if(!CurPlugin.pGetParams(CurPlugin.Index,PAR_GET_FILESTART,(const char**)&start))
                  start=_T("");
                lstrcpy(buff_start,start);
                if(CurPlugin.Params&PAR_FILESTART_STORE)
                {
                  HKEY hKey; DWORD Type; DWORD DataSize=0;
                  if((RegOpenKeyEx(HKEY_CURRENT_USER,PluginStartKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
                  {
                    DataSize=sizeof(buff_start);
                    RegQueryValueEx(hKey,name,0,&Type,(LPBYTE)buff_start,&DataSize);
                    RegCloseKey(hKey);
                  }
                }
              }
            }
            if(name) CurPlugin.Name=name;
            else CurPlugin.Name=Unknown;
            if(lstrlen(buff_mask))
              CurPlugin.Mask=(TCHAR*)malloc((lstrlen(buff_mask)+1)*sizeof(TCHAR));
            if(CurPlugin.Mask)
              lstrcpy(CurPlugin.Mask,buff_mask);
            if(lstrlen(buff_start))
              CurPlugin.Start=(TCHAR*)malloc((lstrlen(buff_start)+1)*sizeof(TCHAR));
            if(CurPlugin.Start)
              lstrcpy(CurPlugin.Start,buff_start);
            if(CurPlugin.pGetParams)
            { //load colors
              int ColorCount; int *Colors;
              if((CurPlugin.Params&PAR_COLORS_STORE)&&CurPlugin.pGetParams(CurPlugin.Index,PAR_GET_COLOR_COUNT,(const char **)&ColorCount)&&CurPlugin.pGetParams(CurPlugin.Index,PAR_GET_COLOR,(const char **)&Colors))
              {
                HKEY hKey; DWORD Type; DWORD DataSize=0;
                if((RegOpenKeyEx(HKEY_CURRENT_USER,PluginColorKey,0,KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
                {
                  DataSize=ColorCount*2*sizeof(int);
                  RegQueryValueEx(hKey,CurPlugin.Name,0,&Type,(LPBYTE)Colors,&DataSize);
                  RegCloseKey(hKey);
                }
              }
            }
            PluginsData[PluginsCount]=CurPlugin;
            PluginsCount++;
          }
        }
      }
      else
        FreeLibrary(hModule);
    }
    Done=!FindNextFile(FindHandle,&fdata);
  }
  FindClose(FindHandle);
  Info.RestoreScreen(hSScr);
}

void UnloadPlugs(void)
{
  for(int i=0;i<PluginsCount;i++)
  {
    if(!PluginsData[i].Index)
    {
      if(PluginsData[i].pExit) PluginsData[i].pExit();
      FreeLibrary(PluginsData[i].hModule);
    }
    free(PluginsData[i].Mask);
    free(PluginsData[i].Start);
  }
  free(PluginsData);
}
