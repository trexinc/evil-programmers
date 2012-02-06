/*
    ab_classes.cpp
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

#include <tchar.h>
#include "plugin.hpp"
#include "memory.h"
#include "abplugin.h"
#include "ab_main.h"
#include "guid.h"
#include <initguid.h>
// {015CCDE2-16D8-4829-BBA2-A7CD548A935C}
DEFINE_GUID(MessageStopGuid, 0x15ccde2, 0x16d8, 0x4829, 0xbb, 0xa2, 0xa7, 0xcd, 0x54, 0x8a, 0x93, 0x5c);

PEditFile editfiles;

// files list support
PEditFile ef_create(bool m)
{
  PEditFile result;
  result=(PEditFile)malloc(sizeof(CEditFile));
  result->id=-1;
  result->main=m;
  result->next=result->prev = result;
  result->cachesize=0;
  result->cache=NULL;
  return result;
}

void ef_free(PEditFile value)
{
  //clear cache
  StateCache *sc;
  while(value->cache!=NULL)
  {
    sc=value->cache;
    value->cache=sc->next;
    free(sc->data);
    free(sc);
  }
  value->cachesize=0;

  PEditFile fl=value->next;
  if(value->main)
    while(fl!=value)
    {
      fl=fl->next;
      ef_free(fl->prev);
    }
  free(value);
}

PEditFile ef_getfile(int fid)
{
  PEditFile fl=editfiles->next;
  while(fl!=editfiles)
  {
    if (fl->id == fid) return fl;
    fl = fl->next;
  }
  return NULL;
}

PEditFile ef_addfile(int fid)
{
  PEditFile fl = editfiles->prev;
  fl->next = ef_create(false);
  fl->next->prev = fl;
  fl = fl->next;
  fl->next = editfiles;
  editfiles->prev = fl;
  fl->id = fid;
  return fl;
}

bool ef_deletefile(int fid)
{
  PEditFile fl = ef_getfile(fid);
  if (!fl) return false;
  fl->prev->next = fl->next;
  fl->next->prev = fl->prev;
  ef_free(fl);
  return true;
}

void OnLoad(void)
{
  editfiles=ef_create(true);
}

void OnExit(void)
{
  ef_free(editfiles);
}

PEditFile loadfile(int eid,int type)
{
  PEditFile fl;
  fl = ef_addfile(eid);
  if(fl) fl->type=type;
  return fl;
}

struct ColorizeThreadData
{
  PLUGINCOLORIZE pColorize;
  int index;
  struct ColorizeParams *params;
};

DWORD WINAPI ColorizeThread(LPVOID lpvThreadParm)
{
  ColorizeThreadData *ctd=(ColorizeThreadData *)lpvThreadParm;
  ctd->pColorize(ctd->index,ctd->params);
  return 0;
}

void ColorizeThreadExit(void)
{
  ExitThread(0);
}

// controls editor events
int OnEditorEvent(int event,void *param,int editorid)
{
  static bool stop_colorize=false;
  if(stop_colorize) return 0;

  TCHAR* editorfilename;
  const TCHAR* filename;
  PEditFile curfile;
  EditorInfo ei;

  // close file
  if(event==EE_CLOSE)
  {
    ef_deletefile(editorid);
    return 0;
  }
  // ignores
  if(event!=EE_REDRAW) return 0;

  if((INT_PTR)param)
  {
    Info.EditorControl(-1,ECTL_REDRAW,0,0);
    return 0;
  };

  // search file in list
  Info.EditorControl(-1,ECTL_GETINFO,0,&ei);

  editorfilename = (TCHAR *)malloc(Info.EditorControl(-1,ECTL_GETFILENAME,0,NULL)*sizeof(TCHAR));
  Info.EditorControl(-1,ECTL_GETFILENAME,0,editorfilename);

  filename=FSF.PointToName(editorfilename); // deletes path...

  EditorGetString egs;
  egs.StringNumber=0;
  Info.EditorControl(-1,ECTL_GETSTRING,0,&egs);

  if((!(curfile=ef_getfile(ei.EditorID)))&&Opt.Active&&(ei.TotalLines<=Opt.MaxLines))
  {
    TCHAR ini_path[MAX_PATH],ini_type[256];
    lstrcpyn(ini_path,editorfilename,filename-editorfilename+1);
    ini_path[filename-editorfilename]=0;
    lstrcat(ini_path,_T("airbrush.ini"));
    if(GetPrivateProfileString(_T("types"),filename,_T(""),ini_type,sizeof(ini_type)/sizeof(ini_type[0]),ini_path))
    {
      for(int i=0;i<PluginsCount;i++)
        if(!lstrcmpi(PluginsData[i].Name,ini_type))
        {
          curfile=loadfile(ei.EditorID,i);
          break;
        }
    }
    else
    {
      for(int i=0;i<PluginsCount;i++)
      {
        if(PluginsData[i].Params&PAR_MASK_CACHE)
        {
          TCHAR* mask=PluginsData[i].Mask;
          if(mask)
          {
            TCHAR FileMask[MAX_PATH];
            while((mask=GetCommaWord(mask,FileMask))!=NULL)
              if(FSF.ProcessName(FileMask,(wchar_t*)filename,0,PN_CMPNAME|PN_SKIPPATH))
              {
                curfile=loadfile(ei.EditorID,i);
                break;
              }
          }
          if(curfile) break;
        }
        else
        {
          if(PluginsData[i].pGetParams)
            if(PluginsData[i].pGetParams(PluginsData[i].Index,PAR_CHECK_MASK,(const char**)&filename))
            {
              curfile=loadfile(ei.EditorID,i);
              break;
            }
        }
        if(PluginsData[i].Params&PAR_FILESTART_CACHE)
        {
          if(egs.StringLength&&PluginsData[i].Start)
          {
            int len=lstrlen(PluginsData[i].Start);
            if(len&&(FSF.ProcessName(PluginsData[i].Start,(wchar_t*)egs.StringText,0,PN_CMPNAME)))
            {
              curfile=loadfile(ei.EditorID,i);
              break;
            }
          }
        }
        else
        {
          if(PluginsData[i].pGetParams)
            if(PluginsData[i].pGetParams(PluginsData[i].Index,PAR_CHECK_FILESTART,(const char**)&egs.StringText))
            {
              curfile=loadfile(ei.EditorID,i);
              break;
            }
        }
      }
    }
  }

  free(editorfilename);

  if((!curfile)&&(Opt.ColorizeAll)) curfile=loadfile(ei.EditorID,-1);
  if(!curfile) return 0;

  ColorizeParams params; unsigned char *cache_data=NULL;
  params.size=sizeof(ColorizeParams);
  params.eid=ei.EditorID;
  params.startcolumn=0;
  params.topline=ei.TopScreenLine;
  params.data_size=0;
  params.data=NULL;
  params.flags=0;
  params.callback=NULL;
  params.param=NULL;

  params.startline=ei.TopScreenLine;
  if((ei.BlockType!=BTYPE_NONE)&&(ei.BlockStartLine<params.startline))
    params.startline=ei.BlockStartLine;
  params.endline=ei.TopScreenLine+ei.WindowSizeY;
  if(params.endline>ei.TotalLines) params.endline=ei.TotalLines;

  if(ei.TotalLines>Opt.MaxLines) curfile->type=-1;
  if((curfile->type<0)||(!Opt.Active))
  {
    EditorDeleteColor edc;
    edc.StructSize=sizeof(edc);
    edc.Owner=MainGuid;
    edc.StartPos=-1;
    for(int i=ei.TopScreenLine;i<params.endline;i++)
    {
      edc.StringNumber=i;
      Info.EditorControl(-1,ECTL_DELCOLOR,0,&edc);
    }
    return 0;
  }
  if(!(curfile->cachesize))
  {
    curfile->cache=(StateCache *)malloc(sizeof(StateCache));
    if(curfile->cache) (curfile->cachesize)++;
  }
  if(!(curfile->cachesize)) return 0;

  int cacheindex=params.startline/PARSER_CACHESTR+1;
  if(cacheindex>(curfile->cachesize))
    cacheindex=curfile->cachesize;
  StateCache **scl=&(curfile->cache),*scl1,*scl2;
  for(int i=1;i<cacheindex;i++)
    scl=&((*scl)->next);
  scl2=*scl;
  *scl=NULL;
  params.data_size=scl2->data_size;
  params.data=scl2->data;
  cache_data=scl2->data;
  scl1=scl2->next;
  free(scl2);
  while(scl1)
  {
    scl2=scl1;
    scl1=scl1->next;
    free(scl2->data);
    free(scl2);
  }
  curfile->cachesize=cacheindex-1;
  params.startline=(cacheindex-1)*PARSER_CACHESTR;
  fatal=false;
  if(PluginsData[curfile->type].pColorize)
  {
    DWORD ThreadID;
    ColorizeThreadData ctd;
    HANDLE handles[2];
    cursor_row=ei.CurLine;
    cursor_col=ei.CurPos;
    params.LocalHeap=HeapCreate(HEAP_NO_SERIALIZE,0,0);
    ctd.pColorize=PluginsData[curfile->type].pColorize;
    ctd.index=PluginsData[curfile->type].Index;
    ctd.params=&params;
    handles[1]=CreateFile(_T("CONIN$"),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    handles[0]=CreateThread(NULL,0,ColorizeThread,&ctd,CREATE_SUSPENDED,&ThreadID);
    if(handles[0])
    {
      ResumeThread(handles[0]);
      if(WaitForSingleObject(handles[0],200)==WAIT_TIMEOUT)
      {
        bool poll=true;
        INPUT_RECORD *AllEvents=NULL,CurEvent;
        unsigned long AllEventsCount=0,CurEventCount;
        while(poll)
        {
          DWORD wfmo=WaitForMultipleObjects(sizeof(handles)/sizeof(handles[0]),handles,FALSE,INFINITE);
          switch(wfmo)
          {
            case WAIT_OBJECT_0:
              poll=false;
              break;
            case WAIT_OBJECT_0+1:
              WaitForSingleObject(Mutex,INFINITE);
              ReadConsoleInput(handles[1],&CurEvent,1,&CurEventCount);
              ReleaseMutex(Mutex);
              if(CurEventCount==1)
              {
                if((CurEvent.EventType==KEY_EVENT)&&(CurEvent.Event.KeyEvent.bKeyDown)&&(CurEvent.Event.KeyEvent.wVirtualKeyCode==VK_ESCAPE))
                {
                  WaitForSingleObject(Mutex,INFINITE);
                  const TCHAR* MsgItems[]={GetMsg(mError),GetMsg(mStopQuestion),GetMsg(mButtonYes),GetMsg(mButtonNo)};
                  stop_colorize=true;
                  bool ContinueThread=Info.Message(&MainGuid,&MessageStopGuid,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2);
                  stop_colorize=false;
                  ReleaseMutex(Mutex);
                  if(!ContinueThread)
                  {
                    CONTEXT c;
                    WaitForSingleObject(Mutex,INFINITE);
                    SuspendThread(handles[0]);
                    c.ContextFlags=CONTEXT_CONTROL;
                    GetThreadContext(handles[0],&c);
#ifdef _AMD64_
                    c.Rip
#else
                    c.Eip
#endif
                    =(DWORD_PTR)ColorizeThreadExit;
                    SetThreadContext(handles[0],&c);
                    ResumeThread(handles[0]);
                    ReleaseMutex(Mutex);
                    WaitForSingleObject(handles[0],INFINITE);
                    poll=false;
                    curfile->type=-1;
                    Info.EditorControl(-1,ECTL_REDRAW,0,0);
                  }
                }
                else
                {
                  WaitForSingleObject(Mutex,INFINITE);
                  AllEvents=(INPUT_RECORD *)realloc(AllEvents,(AllEventsCount+1)*sizeof(INPUT_RECORD));
                  AllEvents[AllEventsCount++]=CurEvent;
                  ReleaseMutex(Mutex);
                }
              }
              break;
          }
        }
        if(AllEvents)
          WriteConsoleInput(handles[1],AllEvents,AllEventsCount,&CurEventCount);
        free(AllEvents);
      }
      CloseHandle(handles[0]);
      CloseHandle(handles[1]);
    }
    cursor_row=-1;
    cursor_col=-1;
    HeapDestroy(params.LocalHeap);
  }
  free(cache_data);
  EditorSetPosition esp;

  esp.CurLine=ei.CurLine;
  esp.CurPos=ei.CurPos;
  esp.TopScreenLine=ei.TopScreenLine;
  esp.LeftPos=ei.LeftPos;
  esp.CurTabPos=-1;
  esp.Overtype=-1;
  Info.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
  if(fatal) RaiseException(0,0,0,NULL);
  return 0;
}

int OnEditorInput(const INPUT_RECORD *Rec)
{
  EditorInfo ei;
  Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
  PEditFile curfile=ef_getfile(ei.EditorID);
  if(curfile&&(curfile->type>-1)&&PluginsData[curfile->type].pInput)
    return PluginsData[curfile->type].pInput(Rec);
  return 0;
}
