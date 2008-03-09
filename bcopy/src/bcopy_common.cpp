#include "../../plugin.hpp"
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
    Item[i].Reserved=Init[i].Selected;
    Item[i].Flags=Init[i].Flags;
    Item[i].DefaultButton=Init[i].DefaultButton;
    if ((unsigned)(DWORD_PTR)Init[i].Data<2000)
      strcpy(Item[i].Data,GetMsg((unsigned int)(DWORD_PTR)Init[i].Data));
    else
      strcpy(Item[i].Data,Init[i].Data);
  }
}

const char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
};

#define MY_COMBINE_PATH \
FSF.AddEndSlash(oem_buff); \
strcpy(res_buff,oem_buff); \
strcat(res_buff,path+3); \
strcpy(path,res_buff);
void UNCPath(char *path)
{
  char buff[MAX_PATH],oem_buff[MAX_PATH],res_buff[MAX_PATH],device[3]=" :"; DWORD size;
  for(int i=0;i<2;i++)
  {
    if((strlen(path)>2)&&(path[1]==':'))
    {
      device[0]=path[0]; size=sizeof(buff);
      if(WNetGetConnection(device,buff,&size)==NO_ERROR)
      {
        CharToOem(buff,oem_buff);
        MY_COMBINE_PATH
      }
      else if(QueryDosDevice(device,buff,sizeof(buff)))
        if(!strncmp(buff,"\\??\\",4))
        {
          CharToOem(buff+4,oem_buff);
          MY_COMBINE_PATH
        }
    }
  }
  //extract symlinks only for local disk
  if((strlen(path)>2)&&(path[1]==':'))
    if((unsigned int)FSF.ConvertNameToReal(path,NULL,0)<sizeof(buff))
    {
      FSF.ConvertNameToReal(path,buff,sizeof(buff));
      strcpy(path,buff);
    }
  for(unsigned int i=0;i<strlen(path);i++) if(path[i]=='/') path[i]='\\';
}
#undef MY_COMBINE_PATH

void NormalizeName(int width,int msg,char *filename,char *dest)
{
  int msg_len=(int)strlen(GetMsg(msg))-2;
  char truncated[MAX_PATH];
  strcpy(truncated,filename);
  FSF.TruncPathStr(truncated,width-msg_len);
  sprintf(dest,GetMsg(msg),truncated);
}

void NormalizeNameW(int width,int msg,wchar_t *filename,wchar_t *dest)
{
  int msg_len=(int)strlen(GetMsg(msg))-2;
  wchar_t truncated[MAX_PATH],mask[MAX_PATH];
  MultiByteToWideChar(CP_OEMCP,0,GetMsg(msg),-1,mask,sizeofa(mask));
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
  const char *MsgItems[]={GetMsg(mError),GetMsg(Message),GetMsg(mOk)};
  Info.Message(Info.ModuleNumber,FMSG_WARNING|(SysError?FMSG_ERRORTYPE:0),NULL,MsgItems,sizeofa(MsgItems),1);
}
