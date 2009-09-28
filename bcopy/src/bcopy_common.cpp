/*
    bcopy_common.cpp
    Copyright (C) 2000-2009 zg

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

#include "far_helper.h"
#include "bcplugin.h"
#include <stdio.h>

void InitDialogItems(InitDialogItem *Init,FarDialogItem *Item,int ItemsNumber)
{
  for (int i=0;i<ItemsNumber;i++)
  {
    Item[i].Type=Init[i].Type;
    Item[i].X1=Init[i].X1;
    Item[i].Y1=Init[i].Y1;
    Item[i].X2=Init[i].X2;
    Item[i].Y2=Init[i].Y2;
    Item[i].Focus=Init[i].Focus;
    Item[i].Selected=Init[i].Selected;
    Item[i].Flags=Init[i].Flags;
    Item[i].DefaultButton=Init[i].DefaultButton;
#ifdef UNICODE
    Item[i].MaxLen=0;
#endif
    if((unsigned)Init[i].Data<2000)
#ifdef UNICODE
      Item[i].PtrData=GetMsg((unsigned int)(DWORD_PTR)Init[i].Data);
#else
      _tcscpy(Item[i].Data,GetMsg((unsigned int)(DWORD_PTR)Init[i].Data));
#endif
    else
#ifdef UNICODE
      Item[i].PtrData=Init[i].Data;
#else
      _tcscpy(Item[i].Data,Init[i].Data);
#endif
  }
}

const TCHAR *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
};

#define MY_COMBINE_PATH \
FSF.AddEndSlash(oem_buff); \
_tcscpy(res_buff,oem_buff); \
_tcscat(res_buff,path+3); \
_tcscpy(path,res_buff);
void UNCPath(TCHAR *path)
{
  TCHAR buff[MAX_PATH],oem_buff[MAX_PATH],res_buff[MAX_PATH],device[3]=_T(" :"); DWORD size;
  for(int i=0;i<2;i++)
  {
    if((_tcslen(path)>2)&&(path[1]==':'))
    {
      device[0]=path[0]; size=ArraySize(buff);
      if(WNetGetConnection(device,buff,&size)==NO_ERROR)
      {
        t_CharToOem(buff,oem_buff);
        MY_COMBINE_PATH
      }
      else if(QueryDosDevice(device,buff,ArraySize(buff)))
        if(!_tcsncmp(buff,_T("\\??\\"),4))
        {
          t_CharToOem(buff+4,oem_buff);
          MY_COMBINE_PATH
        }
    }
  }
  //extract symlinks only for local disk
  if((_tcslen(path)>2)&&(path[1]==':'))
    if((unsigned int)ConvertNameToReal(path,NULL,0)<ArraySize(buff))
    {
      ConvertNameToReal(path,buff,ArraySize(buff));
      _tcscpy(path,buff);
    }
  for(unsigned int i=0;i<_tcslen(path);i++) if(path[i]=='/') path[i]='\\';
}
#undef MY_COMBINE_PATH

void NormalizeName(int width,int msg,TCHAR *filename,TCHAR *dest)
{
  int msg_len=(int)_tcslen(GetMsg(msg))-2;
  TCHAR truncated[MAX_PATH];
  _tcscpy(truncated,filename);
  FSF.TruncPathStr(truncated,width-msg_len);
  _stprintf(dest,GetMsg(msg),truncated);
}

void NormalizeNameW(int width,int msg,wchar_t *filename,wchar_t *dest)
{
  int msg_len=(int)_tcslen(GetMsg(msg))-2;
  wchar_t truncated[MAX_PATH],mask[MAX_PATH];
#ifdef UNICODE
  _tcsncpy(mask,GetMsg(msg),sizeofa(mask)-1);
  mask[sizeofa(mask)-1]=0;
#else
  MultiByteToWideChar(CP_OEMCP,0,GetMsg(msg),-1,mask,sizeofa(mask));
#endif
  wcscpy(truncated,filename);
  TruncPathStrW(truncated,width-msg_len);
  swprintf(dest,mask,truncated);
}

wchar_t *TruncPathStrW(wchar_t *Str,int MaxLength)
{
  if(Str&&MaxLength>3)
  {
    int init_len=(int)wcslen(Str);
    if(init_len>MaxLength)
    {
      wchar_t *ptr=wcschr(Str,'\\');
      if(ptr==Str&&Str[1]=='\\')
      {
        ptr++;
        for(int i=0;i<2;i++)
          if(ptr)
          {
            ptr++;
            ptr=wcschr(ptr,'\\');
          }
          else break;
      }
      if(ptr&&(MaxLength-(ptr-Str)-1)>3)
        ptr++;
      else
        ptr=Str;
      for(int i=0;i<3;i++,ptr++) *ptr='.';
      int copy_len=MaxLength-(int)(ptr-Str)+1;
      for(int i=0;i<copy_len;i++,ptr++) *ptr=*(ptr+init_len-MaxLength);
    }
  }
  return Str;
}

void ShowError(int Message,bool SysError)
{
  const TCHAR *MsgItems[]={GetMsg(mError),GetMsg(Message),GetMsg(mOk)};
  Info.Message(Info.ModuleNumber,FMSG_WARNING|(SysError?FMSG_ERRORTYPE:0),NULL,MsgItems,sizeofa(MsgItems),1);
}
