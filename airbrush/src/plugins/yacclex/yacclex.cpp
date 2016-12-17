/*
    yacclex.cpp
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

#include <windows.h>
#include <tchar.h>
#include "abplugin.h"
#include "abyacclex.h"
#include <initguid.h>
#include "guid.h"
#include "../plugins/c/guid.h"

ColorizeInfo Info;
ABColor colors[]=
{
  {ABCF_4BIT,AB_OPAQUE(0x03),AB_OPAQUE(0x00),NULL,false,true },
  {ABCF_4BIT,AB_OPAQUE(0x0E),AB_OPAQUE(0x00),NULL,false,true },
  {ABCF_4BIT,AB_OPAQUE(0x00),AB_OPAQUE(0x0C),NULL,false,false},
  {ABCF_4BIT,AB_OPAQUE(0x0F),AB_OPAQUE(0x00),NULL,false,true }
};
const TCHAR *colornames[]={_T("Comment"),_T("String"),_T("Keyword"),_T("Set")};

struct CacheParam
{
  int c_state;
  int lex_state;
  char diff;
  int recurse;
};

struct CallbackParam
{
  int ok;
  int row;
  int col;
  ColorizeParams* params;
  CacheParam *cache;
};

int WINAPI SetColorizeInfo(ColorizeInfo *AInfo)
{
  if((AInfo->version<AB_VERSION)||(AInfo->api!=AB_API)) return false;
  Info=*AInfo;
  return true;
};

static int WINAPI c_callback(int from,int row,int col,void *param)
{
  const TCHAR *line;
  intptr_t linelen;
  line=Info.pGetLine(((CallbackParam *)param)->params->eid,row,&linelen);
  switch(((CallbackParam *)param)->cache->diff)
  {
    case 1:
    case 2:
      if(from==1)
      {
        if((!_tcsncmp(line+col,_T("%}"),2))&&(!col))
        {
          Info.pAddColor(((CallbackParam *)param)->params,row,col,2,colors+HC_KEYWORD,EPriorityNormal);
          ((CallbackParam *)param)->ok=1;
          ((CallbackParam *)param)->row=row;
          ((CallbackParam *)param)->col=col+2;
          return true;
        }
      }
      break;
    case 3:
      if(from==0)
      {
        if(!_tcsncmp(line+col,_T("{"),1))
          (((CallbackParam *)param)->cache->recurse)++;
        if(!_tcsncmp(line+col,_T("}"),1))
        {
          (((CallbackParam *)param)->cache->recurse)--;
          if(!(((CallbackParam *)param)->cache->recurse))
          {
            ((CallbackParam *)param)->ok=1;
            ((CallbackParam *)param)->row=row;
            ((CallbackParam *)param)->col=col+1;
            return true;
          }
        }
      }
      break;
  }
  return false;
}

static void CallParser(ColorizeParams *params,CallbackParam *data)
{
  ColorizeParams c_params;
  c_params.size=sizeof(ColorizeParams);
  c_params.eid=params->eid;
  c_params.startline=data->row;
  c_params.startcolumn=data->col;
  c_params.endline=params->endline;
  c_params.topline=params->topline;
  c_params.margins=params->margins;
  c_params.data_size=params->data_size;
  c_params.data=params->data;
  c_params.LocalHeap=params->LocalHeap;
  c_params.callback=c_callback;
  c_params.param=data;
  if(data->cache->diff==4)
  {
    c_params.callback=NULL;
    c_params.param=NULL;
  }
  data->ok=0;
  Info.pCallParser(&CplusplusGUID,&c_params);
  if(data->ok)
  {
    params->startline=data->row;
    params->startcolumn=data->col;
    if(data->cache->diff==1)
      data->cache->lex_state=PARSER_CLEAR;
    else
      data->cache->lex_state=PARSER_RULES;
    data->cache->c_state=INVALID_C_STATE;
  }
}

void WINAPI Colorize(intptr_t index,struct ColorizeParams *params)
{
  (void)index;
  int commentstart,stringstart;
  const TCHAR *line;
  intptr_t linelen;
  int startcol;
  CallbackParam callback_data;
  CacheParam state_data={INVALID_C_STATE,PARSER_CLEAR,0,0};
  CacheParam *state=&state_data;
  int state_size=sizeof(state_data);
  if(params->data_size>=sizeof(state_data))
  {
    state=(CacheParam *)(params->data);
    state_size=params->data_size;
  }
  else
  {
    params->data=(unsigned char *)state;
    params->data_size=state_size;
  }
  callback_data.cache=state;
  callback_data.params=params;
  if(state[0].c_state>INVALID_C_STATE)
  {
    callback_data.row=params->startline;
    callback_data.col=params->startcolumn;
    CallParser(params,&callback_data);
    if(!callback_data.ok) goto colorize_exit;
  }
colorize_start:
  for(int lno=params->startline;lno<params->endline;lno++)
  {
    startcol=(lno==params->startline)?params->startcolumn:0;
    if(((lno%Info.cachestr)==0)&&(!startcol))
      if(!Info.pAddState(params->eid,lno/Info.cachestr,state_size,(unsigned char *)state)) goto colorize_exit;
    commentstart=0;
    line=Info.pGetLine(params->eid,lno,&linelen);
    for(int i=startcol;i<linelen;i++)
    {
      switch(state[0].lex_state&0xffff)
      {
        case PARSER_CLEAR:
        case PARSER_RULES:
          if(!_tcsncmp(line+i,_T("/*"),2))
          {
            state[0].lex_state=PARSER_COMMENT|(state[0].lex_state<<16);
            commentstart=i;
            i++;
          }
          else if(*(line+i)=='\'')
          {
            stringstart=i;
            i++;
            while(i<linelen)
            {
              if(*(line+i)=='\\')
              {
                i++;
              }
              else if(*(line+i)=='\'')
              {
                Info.pAddColor(params,lno,stringstart,i + 1 - stringstart,colors+HC_STRING,EPriorityNormal);
                break;
              }
              i++;
            }
          }
          else if(*(line+i)=='\"')
          {
            state[0].lex_state=PARSER_STRING|(state[0].lex_state<<16);
            commentstart=i;
          }
          else if (!_tcsncmp(line+i,_T("//"),2))
          {
            Info.pAddColor(params,lno,i,linelen-i,colors+HC_COMMENT,EPriorityNormal);
            i=linelen;
          }
          else if(*(line+i)=='[')
          {
            state[0].lex_state=PARSER_SET|(state[0].lex_state<<16);
            commentstart=i;
          }
          else
          {
            if(params->callback)
              if(params->callback(0,lno,i,params->param))
                goto colorize_exit;
            if((!_tcsncmp(line+i,_T("%%"),2))&&(!i))
            {
              Info.pAddColor(params,lno,i,2,colors+HC_KEYWORD,EPriorityNormal);
              if(state[0].lex_state==PARSER_CLEAR)
              {
                state[0].lex_state=PARSER_RULES;
              }
              else
              {
                callback_data.row=lno;
                callback_data.col=i+2;
                state[0].diff=4;
                state[0].c_state=PARSER_CLEAR;
                CallParser(params,&callback_data);
                goto colorize_exit;
              }
            }
            else
            {
              if((!_tcsncmp(line+i,_T("%{"),2))&&(!i))
              {
                Info.pAddColor(params,lno,i,2,colors+HC_KEYWORD,EPriorityNormal);
                callback_data.row=lno;
                callback_data.col=i+2;
                state[0].diff=1+state[0].lex_state;
                state[0].c_state=PARSER_CLEAR;
                CallParser(params,&callback_data);
                if(!callback_data.ok) goto colorize_exit;
                goto colorize_start;
              }
              if(state[0].lex_state==PARSER_RULES)
              {
                if(!_tcsncmp(line+i,_T("{"),1))
                {
                  callback_data.row=lno;
                  callback_data.col=i;
                  state[0].diff=3;
                  state[0].c_state=PARSER_CLEAR;
                  CallParser(params,&callback_data);
                  if(!callback_data.ok) goto colorize_exit;
                  goto colorize_start;
                }
              }
            }
          }
          break;
        case PARSER_COMMENT:
          if(!_tcsncmp(line+i,_T("*/"),2))
          {
            i++;
            state[0].lex_state=state[0].lex_state>>16;
            Info.pAddColor(params,lno,commentstart,i+1-commentstart,colors+HC_COMMENT,EPriorityNormal);
          }
          break;
        case PARSER_STRING:
          if(*(line+i)=='\"')
          {
            state[0].lex_state=state[0].lex_state>>16;
            Info.pAddColor(params,lno,commentstart,i+1-commentstart,colors+HC_STRING,EPriorityNormal);
          }
          else if(*(line+i)=='\\')
            i++;
          break;
        case PARSER_SET:
          if(*(line+i)==']')
          {
            state[0].lex_state=state[0].lex_state>>16;
            Info.pAddColor(params,lno,commentstart,i+1-commentstart,colors+HC_SET,EPriorityNormal);
          }
          else if(*(line+i)=='\\')
            i++;
          break;
      }
    }
    if((state[0].lex_state&0xffff)==PARSER_COMMENT)
      Info.pAddColor(params,lno,commentstart,linelen-commentstart,colors+HC_COMMENT,EPriorityNormal);
    if((state[0].lex_state&0xffff)==PARSER_STRING)
      Info.pAddColor(params,lno,commentstart,linelen-commentstart,colors+HC_STRING,EPriorityNormal);
    if((state[0].lex_state&0xffff)==PARSER_SET)
      Info.pAddColor(params,lno,commentstart,linelen-commentstart,colors+HC_SET,EPriorityNormal);
  }
colorize_exit:
  return;
}

int WINAPI GetParams(intptr_t index,intptr_t command,const char **param)
{
  (void)index;
  static const ABName name={YaccLexGUID,_T("lex/yacc")};
  switch(command)
  {
    case PAR_GET_NAME:
      *param=(const char*)&name;
      return true;
    case PAR_GET_PARAMS:
      return PAR_MASK_STORE|PAR_MASK_CACHE|PAR_COLORS_STORE|PAR_SHOW_IN_LIST;
    case PAR_GET_MASK:
      *param=(const char*)_T("*.l,*.y");
      return true;
    case PAR_GET_COLOR_COUNT:
      *(int *)param=sizeof(colornames)/sizeof(colornames[0]);
      return true;
    case PAR_GET_COLOR:
      *(const ABColor**)param=colors;
      return true;
    case PAR_GET_COLOR_NAME:
      *param=(const char *)colornames;
      return true;
  }
  return false;
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
