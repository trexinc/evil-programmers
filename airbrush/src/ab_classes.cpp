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
#include "far_settings.h"
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
  result->topline=0;
  result->apitopline=0;
  result->full=false;
  for(size_t ii=0;ii<ArraySize(result->bracket);++ii)
  {
    result->bracket[ii].x=-1;
    result->bracket[ii].y=-1;
    result->bracket[ii].len=-1;
    result->bracket[ii].active=false;
  }
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

// controls editor events
int OnEditorEvent(int event,void *param,int editorid)
{
  static bool stop_colorize=false;
  if(stop_colorize) return 0;
  PEditFile curfile=ef_getfile(editorid);

  if(event==EE_CHANGE)
  {
    EditorChange* change=(EditorChange*)param;
    if(curfile&&curfile->topline>change->StringNumber)
    {
      curfile->topline=change->StringNumber;
    }
    return 0;
  }

  if(event==EE_READ)
  {
    EditorSubscribeChangeEvent esce={sizeof(EditorSubscribeChangeEvent),MainGuid};
    Info.EditorControl(editorid,ECTL_SUBSCRIBECHANGEEVENT,0,&esce);
    return 0;
  }

  TCHAR* editorfilename;
  const TCHAR* filename;
  EditorInfo ei;
  ei.StructSize=sizeof(ei);

  // close file
  if(event==EE_CLOSE)
  {
    ef_deletefile(editorid);
    return 0;
  }
  // ignores
  if(event!=EE_REDRAW) return 0;

  // search file in list
  Info.EditorControl(editorid,ECTL_GETINFO,0,&ei);

  size_t editorfilenamesize=Info.EditorControl(editorid,ECTL_GETFILENAME,0,NULL);
  editorfilename = (TCHAR *)malloc(editorfilenamesize*sizeof(TCHAR));
  Info.EditorControl(editorid,ECTL_GETFILENAME,editorfilenamesize,editorfilename);

  filename=FSF.PointToName(editorfilename); // deletes path...

  EditorGetString egs;
  egs.StructSize=sizeof(egs);
  egs.StringNumber=0;
  if(!Info.EditorControl(editorid,ECTL_GETSTRING,0,&egs))
  {
    free(editorfilename);
    return 0;
  }

  if((!curfile)&&Opt.Active&&(ei.TotalLines<=Opt.MaxLines))
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

  free(editorfilename);

  if(!curfile) curfile=loadfile(ei.EditorID,-1);
  if(!curfile) return 0;
  intptr_t topline=curfile->topline;
  curfile->apitopline=curfile->topline;
  curfile->topline=INTPTR_MAX;
  if(ei.TotalLines>Opt.MaxLines) curfile->type=-1;
  if(curfile->type<0||!Opt.Active) return 0;

  ColorizeParams params; unsigned char *cache_data=NULL;
  params.size=sizeof(ColorizeParams);
  params.eid=ei.EditorID;
  params.startcolumn=0;
  params.topmargin=ei.TopScreenLine;
  params.bottommargin=ei.TopScreenLine+ei.WindowSizeY;
  if(params.bottommargin>ei.TotalLines) params.bottommargin=ei.TotalLines;
  params.data_size=0;
  params.data=NULL;
  params.flags=0;
  params.callback=NULL;
  params.param=NULL;

  if(curfile->full)
  {
    curfile->full=false;
    params.startline=0;
    params.endline=ei.TotalLines;
  }
  else
  {
    params.startline=ei.TopScreenLine;
    if(curfile->apitopline<params.startline)
      params.startline=curfile->apitopline;
    params.endline=ei.TopScreenLine+ei.WindowSizeY;
    if(params.endline>ei.TotalLines) params.endline=ei.TotalLines;
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
  if(PluginsData[curfile->type].pColorize)
  {
    intptr_t visiblelines=params.bottommargin-params.topmargin;
    params.margins=(struct ColorizeMargin*)malloc(sizeof(*params.margins)*(visiblelines));
    if(params.margins)
    {
      intptr_t lefttab=ei.LeftPos,righttab=ei.LeftPos+ei.WindowSizeX;
      for(intptr_t ii=0;ii<visiblelines;++ii)
      {
        if(ei.Options&EOPT_EXPANDALLTABS)
        {
          params.margins[ii].left=lefttab;
          params.margins[ii].right=righttab;
        }
        else
        {
          egs.StringNumber=ii+params.topmargin;
          if(Info.EditorControl(ei.EditorID,ECTL_GETSTRING,0,&egs))
          {
            intptr_t left=-1,right=egs.StringLength;
            for(intptr_t jj=0,pos=0;jj<egs.StringLength;++jj)
            {
              if(left<0&&pos>=lefttab) left=jj;
              if(right>jj&&pos>=righttab)
              {
                right=jj;
                break;
              }
              if(egs.StringText[jj]=='\t') pos+=ei.TabSize-(pos%ei.TabSize);
              else ++pos;
            }
            params.margins[ii].left=left;
            params.margins[ii].right=(left<0)?-1:right;
          }
        }
      }
    }
    params.LocalHeap=HeapCreate(HEAP_NO_SERIALIZE,0,0);
    PluginsData[curfile->type].pColorize(PluginsData[curfile->type].Index,&params);
    HeapDestroy(params.LocalHeap);
    free(params.margins);
  }
  free(cache_data);
  return 0;
}

int OnEditorInput(const INPUT_RECORD *Rec)
{
  EditorInfo ei;
  ei.StructSize=sizeof(ei);
  Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
  PEditFile curfile=ef_getfile(ei.EditorID);
  if(curfile&&(curfile->type>-1)&&PluginsData[curfile->type].pInput)
    return PluginsData[curfile->type].pInput(Rec);
  return 0;
}
