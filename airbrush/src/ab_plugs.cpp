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
#include "far_settings.h"
#include "farcolor.hpp"
#include "memory.h"
#include "abplugin.h"
#include "ab_main.h"
#include "abversion.h"
#include "guid.h"
#include <initguid.h>
// {55828A68-CBF7-4695-B1E3-26C16116C2AF}
DEFINE_GUID(MessageFatalGuid, 0x55828a68, 0xcbf7, 0x4695, 0xb1, 0xe3, 0x26, 0xc1, 0x61, 0x16, 0xc2, 0xaf);
// {40B98695-3971-4c4c-88A9-F3E5BAC777EB}
DEFINE_GUID(MessageLoadingGuid, 0x40b98695, 0x3971, 0x4c4c, 0x88, 0xa9, 0xf3, 0xe5, 0xba, 0xc7, 0x77, 0xeb);



struct PluginItem *PluginsData=NULL;
size_t PluginsCount=0;

static TCHAR UnknownPluginName[20];

static void WINAPI addcolor(struct ColorizeParams* params,intptr_t lno,intptr_t start,intptr_t len,const struct ABColor* color,enum ColorizePriority priority)
{
  if((color->ForegroundDefault)&&(color->BackgroundDefault)) return;
  if(len==0) return;
  if(lno>=params->topmargin&&lno<params->bottommargin)
  {
    if(!(start>=params->margins[lno-params->topmargin].right||(start+len-1)<params->margins[lno-params->topmargin].left))
    {
      EditorColor ec;
      ec.StructSize=sizeof(ec);
      ec.StringNumber=lno;
      ec.StartPos=start;
      ec.EndPos=start+len-1;
      ConvertColor(*color,ec.Color);
      ec.Owner=MainGuid;
      switch(priority)
      {
        case EPriorityNormal:
          ec.Priority=Opt.Priority;
          break;
        case EPriorityBrackets:
          ec.Priority=Opt.BrPriority;
          break;
      }
      ec.Flags=ECF_AUTODELETE;
      Info.EditorControl(params->eid,ECTL_ADDCOLOR,0,&ec);
    }
  }
}

static const TCHAR WINAPI *getline(intptr_t eid,intptr_t lno,intptr_t *len)
{
  EditorGetString egs;
  egs.StructSize=sizeof(egs);
  egs.StringNumber=lno;
  Info.EditorControl(eid,ECTL_GETSTRING,0,&egs);
  if(len)
    *len=egs.StringLength;
  return egs.StringText;
}

static bool WINAPI addstate(intptr_t eid,intptr_t pos,size_t size,unsigned char *data)
{
  bool res=true;
  PEditFile fl=ef_getfile(eid);
  if(fl)
  {
    if(pos==fl->cachesize)
    {
      StateCache **prev_cache=&(fl->cache);
      while(*prev_cache)
        prev_cache=&((*prev_cache)->next);
      *prev_cache=reinterpret_cast<StateCache*>(malloc(sizeof(StateCache)));
      if(*prev_cache)
      {
        if(size) (*prev_cache)->data=reinterpret_cast<unsigned char*>(malloc(size));
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
      wsprintf(buff,GetMsg(mFatalLine2),fl->cachesize,pos);
      const TCHAR* MsgItems[]={GetMsg(mError),GetMsg(mFatalLine1),buff,GetMsg(mButtonOk)};
      Info.Message(&MainGuid,&MessageFatalGuid,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
    }
  }
  if(!res) RaiseException(0,0,0,NULL);
  return res;
}

static void WINAPI getcursor(intptr_t eid,intptr_t *row,intptr_t *col)
{
  EditorInfo ei;
  ei.StructSize=sizeof(ei);
  Info.EditorControl(eid,ECTL_GETINFO,0,&ei);
  *row=ei.CurLine;
  *col=ei.CurPos;
}

static void WINAPI callparser(const GUID *parser,struct ColorizeParams *params)
{
  if(FarGuid!=*parser)
    for(size_t i=0;i<PluginsCount;i++)
      if(*parser==PluginsData[i].Id)
        if(PluginsData[i].pColorize)
          PluginsData[i].pColorize(PluginsData[i].Index,params);
}

static void WINAPI setbracket(intptr_t eid,intptr_t row1,intptr_t col1,intptr_t len1,bool active1,intptr_t row2,intptr_t col2,intptr_t len2,bool active2)
{
  PEditFile fl=ef_getfile(eid);
  if(fl)
  {
    fl->bracket[0].x=col1;
    fl->bracket[0].y=row1;
    fl->bracket[0].len=len1;
    fl->bracket[0].active=active1;
    fl->bracket[1].x=col2;
    fl->bracket[1].y=row2;
    fl->bracket[1].len=len2;
    fl->bracket[1].active=active2;
  }
}

struct InterpolationItem{uint64_t key;bool value;};
const size_t InterpolationCacheMaxSize=256;
static size_t InterpolationCacheSize=0;
static InterpolationItem InterpolationCache[InterpolationCacheMaxSize];

static uint64_t hash(const wchar_t* str,size_t len)
{
  uint64_t b=378551u,a=63689u,result=0u;
  for(size_t ii=0;ii<len;++ii,++str)
  {
    result=result*a+*str;
    a*=b;
  }
  return result;
}

int compare(const void* a,const void* b)
{
  const InterpolationItem* a1=reinterpret_cast<const InterpolationItem*>(a),*b1=reinterpret_cast<const InterpolationItem*>(b);
  if(a1->key<b1->key) return -1;
  if(a1->key>b1->key) return 1;
  return 0;
}

static bool WINAPI interpolation(const wchar_t* str,intptr_t len)
{
  static bool recurse=false;
  bool res=true;
  if(!recurse)
  {
    recurse=true;
    InterpolationItem key={hash(str,len),false};
    InterpolationItem* item=reinterpret_cast<InterpolationItem*>(bsearch(&key,InterpolationCache,InterpolationCacheSize,sizeof(InterpolationItem),compare));
    if(item)
    {
      res=item->value;
    }
    else
    {
      wchar_t* text=reinterpret_cast<wchar_t*>(malloc((len+1)*sizeof(*str)));
      if(text)
      {
        memcpy(text,str,len*sizeof(*str));
        text[len]=0;
        if(len)
        {
          FarMacroValue value;
          value.Type=FMVT_STRING;
          value.String=text;
          MacroExecuteString seq={sizeof(MacroExecuteString),KMFLAGS_MOONSCRIPT,_T("{:build_grammar}=require\"moonscript.parse\"\ng=build_grammar!\nr=g\\match [[\"]]..(...)..[[\"]]\nr and r[1] and r[1][3] and r[1][3][1]=='interpolate'"),1,&value,0,NULL};
          if(Info.MacroControl(0,MCTL_EXECSTRING,0,&seq)&&seq.OutCount>0&&FMVT_BOOLEAN==seq.OutValues[0].Type&&!seq.OutValues[0].Boolean) res=false;
        }
        free(text);
        key.value=res;
        if(InterpolationCacheSize==InterpolationCacheMaxSize) InterpolationCacheSize=0;
        InterpolationCache[InterpolationCacheSize++]=key;
        qsort(InterpolationCache,InterpolationCacheSize,sizeof(InterpolationItem),compare);
      }
    }
    recurse=false;
  }
  return res;
}

void LoadPlugs(const TCHAR* ModuleName)
{
  HANDLE hSScr=Info.SaveScreen(0,0,-1,-1);
  {
    const TCHAR* MsgItems[]={GetMsg(mName),GetMsg(mLoading)};
    Info.Message(&MainGuid,&MessageLoadingGuid,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
  }
  TCHAR PluginsFolder[MAX_PATH],PluginsMask[MAX_PATH],*NamePtr;
  lstrcpy(UnknownPluginName,GetMsg(mUnknown));
  lstrcpy(PluginsFolder,ModuleName);
  NamePtr=const_cast<TCHAR*>(FSF.PointToName(PluginsFolder));
  lstrcpy(NamePtr,_T("\\Formats\\"));
  wsprintf(PluginsMask,_T("%s*.fmt"),PluginsFolder);

  CFarSettings settings(MainGuid);
  HANDLE FindHandle;
  WIN32_FIND_DATA fdata;
  int Done=((FindHandle=FindFirstFile(PluginsMask,&fdata))==INVALID_HANDLE_VALUE);
  while(!Done)
  {
    TCHAR PluginName[MAX_PATH];
    wsprintf(PluginName,_T("%s%s"),PluginsFolder,fdata.cFileName);
    HMODULE hModule=LoadLibrary(PluginName);
    if (hModule!=NULL)
    {
      struct PluginItem CurPlugin;
      CurPlugin.hModule=hModule;
      CurPlugin.Type=0;
      CurPlugin.pLoadSyntaxModule=reinterpret_cast<PLUGINLOADSYNTAXMODULE>(reinterpret_cast<void(*)()>(GetProcAddress(hModule,"LoadSyntaxModule")));
      CurPlugin.pSetColorizeInfo=reinterpret_cast<PLUGINSETCOLORIZEINFO>(reinterpret_cast<void(*)()>(GetProcAddress(hModule,"SetColorizeInfo")));
      CurPlugin.pGetParams=reinterpret_cast<PLUGINGETPARAMS>(reinterpret_cast<void(*)()>(GetProcAddress(hModule,"GetParams")));
      CurPlugin.pColorize=reinterpret_cast<PLUGINCOLORIZE>(reinterpret_cast<void(*)()>(GetProcAddress(hModule,"Colorize")));
      CurPlugin.pInput=reinterpret_cast<PLUGININPUT>(reinterpret_cast<void(*)()>(GetProcAddress(hModule,"Input")));
      CurPlugin.pGetSyntaxCount=reinterpret_cast<PLUGINGETSYNTAXCOUNT>(reinterpret_cast<void(*)()>(GetProcAddress(hModule,"GetSyntaxCount")));
      CurPlugin.pExit=reinterpret_cast<PLUGINEXIT>(reinterpret_cast<void(*)()>(GetProcAddress(hModule,"Exit")));

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
        lInfo.pSetBracket=setbracket;
        lInfo.pInterpolation=interpolation;
        accept_plug=CurPlugin.pSetColorizeInfo(&lInfo);
      }

      if(accept_plug)
      {
        unsigned long subcount=1;
        if(CurPlugin.pGetSyntaxCount)
          subcount=CurPlugin.pGetSyntaxCount();

        if(subcount)
        {
          struct PluginItem *NewPluginsData=reinterpret_cast<struct PluginItem*>(realloc(PluginsData,sizeof(*PluginsData)*(PluginsCount+subcount)));
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
            const ABName* data=NULL;
            TCHAR buff_mask[2048],buff_start[2048];
            lstrcpy(buff_mask,_T("")); lstrcpy(buff_start,_T(""));
            if(CurPlugin.pGetParams)
            {
              CurPlugin.Params=CurPlugin.pGetParams(CurPlugin.Index,PAR_GET_PARAMS,NULL);
              CurPlugin.pGetParams(CurPlugin.Index,PAR_GET_NAME,reinterpret_cast<const char**>(&data));
              //load mask
              if(data) if(RPC_S_OK!=UuidToString(const_cast<UUID*>(&data->Id),reinterpret_cast<unsigned short**>(const_cast<TCHAR**>(&name)))) name=NULL;
              if(name&&(CurPlugin.Params&PAR_MASK_CACHE))
              {
                if(!CurPlugin.pGetParams(CurPlugin.Index,PAR_GET_MASK,reinterpret_cast<const char**>(&mask)))
                  mask=_T("");
                lstrcpy(buff_mask,mask);
                if(CurPlugin.Params&PAR_MASK_STORE)
                {
                  settings.Root();
                  settings.Change(PLUGIN_MASK_KEY);
                  settings.Get(name,buff_mask,ArraySize(buff_mask));
                }
              }
              //load starts
              if(name&&(CurPlugin.Params&PAR_FILESTART_CACHE))
              {
                if(!CurPlugin.pGetParams(CurPlugin.Index,PAR_GET_FILESTART,reinterpret_cast<const char**>(&start)))
                  start=_T("");
                lstrcpy(buff_start,start);
                if(CurPlugin.Params&PAR_FILESTART_STORE)
                {
                  settings.Root();
                  settings.Change(PLUGIN_START_KEY);
                  settings.Get(name,buff_start,ArraySize(buff_start));
                }
              }
            }
            if(data)
            {
              CurPlugin.Id=data->Id;
              CurPlugin.Name=data->Name;
            }
            else CurPlugin.Name=UnknownPluginName;
            lstrcpy(CurPlugin.IdStr,name?name:UnknownPluginName);
            RpcStringFree(reinterpret_cast<unsigned short**>(const_cast<TCHAR**>(&name)));
            if(lstrlen(buff_mask))
              CurPlugin.Mask=reinterpret_cast<TCHAR*>(malloc((lstrlen(buff_mask)+1)*sizeof(TCHAR)));
            if(CurPlugin.Mask)
              lstrcpy(CurPlugin.Mask,buff_mask);
            if(lstrlen(buff_start))
              CurPlugin.Start=reinterpret_cast<TCHAR*>(malloc((lstrlen(buff_start)+1)*sizeof(TCHAR)));
            if(CurPlugin.Start)
              lstrcpy(CurPlugin.Start,buff_start);
            if(CurPlugin.pGetParams)
            { //load colors
              int ColorCount; ABColor* Colors;
              if((CurPlugin.Params&PAR_COLORS_STORE)&&CurPlugin.pGetParams(CurPlugin.Index,PAR_GET_COLOR_COUNT,reinterpret_cast<const char**>(&ColorCount))&&CurPlugin.pGetParams(CurPlugin.Index,PAR_GET_COLOR,const_cast<const char**>(reinterpret_cast<char**>(&Colors))))
              {
                LoadColors(settings,CurPlugin.IdStr,Colors,ColorCount);
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
  Info.Text(0,0,NULL,NULL);
}

void UnloadPlugs(void)
{
  for(size_t i=0;i<PluginsCount;i++)
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
