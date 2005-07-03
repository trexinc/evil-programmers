/*
    FARMail plugin for FAR Manager
    Copyright (C) 2002-2004 FARMail Group

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
#include "farmail.hpp"
#include "fmp.hpp"
#include "version.hpp"

void WINAPI api_get_message(const char *file_name,int index,char *message)
{
  static char language[512];
  static char buffer[32*1024-1];
  *message=0;
  DWORD res=GetEnvironmentVariable("FARLANG",language,sizeof(language));
  if(!(res&&res<sizeof(language))) lstrcpy(language,"English");
  for(int i=0;i<2;i++)
  {
    if(GetPrivateProfileSection(language,buffer,sizeof(buffer),file_name))
    {
      char *ptr=buffer;
      while(index>=0&&lstrlen(ptr))
      {
        if(!index) lstrcpy(message,ptr);
        index--;
        ptr=ptr+lstrlen(ptr)+1;
      }
      break;
    }
    else if(!i) lstrcpy(language,"English");
  }
}

int WINAPI api_get_header_field(const char *header,const char *field_name,char *field,size_t field_size)
{
  int res;
  res=!GetGeaderField(header,field,field_name,field_size-1);
  if(res)
  {
    CHARSET_TABLE *CharsetTable=NULL;
    if(!ConstructCharset(&CharsetTable))
    {
      ReadCharsetTable(CharsetTable);
      InitCharset(&CharsetTable);
      DecodeField(field,NULL,&CharsetTable);
    }
    DestructCharset(&CharsetTable);
  }
  return res;
}

int WINAPI api_get_free_number(char *dir)
{
  return GetFreeNumber(dir);
}

void WINAPI api_init_boundary(char *bound)
{
  InitBoundary(bound);
}

long WINAPI api_show_help_dlg_proc(HANDLE hDlg, int Msg,int Param1,long Param2)
{
  static char *ModuleName;

  switch (Msg)
  {
    case DN_INITDIALOG:
      ModuleName = (char *) Param2;
      return FALSE;

    case DN_HELP:
      _Info.ShowHelp(ModuleName,(char *) Param2,FHELP_SELFHELP);
      return (long)NULL;
  }
  return _Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

int WINAPI api_show_help_menu(
  const char *ModuleName,
  int PluginNumber,
  int X, int Y,
  int MaxHeight,
  unsigned int Flags,
  const char *Title,
  const char *Bottom,
  const char *HelpTopic,
  const int *BreakKeys,
  int *BreakCode,
  const struct FarMenuItem *Item,
  int ItemsNumber)
{
  int breakcode;
  const int breakkeys[] = {VK_F1, 0};
  if (!BreakKeys)
  {
    BreakKeys = breakkeys;
    BreakCode = &breakcode;
  }
  int menucode;
  for(;;)
  {
    menucode = _Info.Menu(PluginNumber, X, Y, MaxHeight, Flags, Title, Bottom, HelpTopic, BreakKeys, BreakCode, Item, ItemsNumber);
    if (menucode >= 0 && *BreakCode == 0)
    {
      _Info.ShowHelp(ModuleName,HelpTopic,FHELP_SELFHELP);
      continue;
    }
    else
      break;
  }
  return menucode;
}

long WINAPI api_get_value(int type)
{
  switch(type)
  {
    case FMVAL_VERSION_MAJOR:
      return FSF.atoi(VER_HI);
    case FMVAL_VERSION_MINOR:
      return FSF.atoi(VER_LO);
    case FMVAL_VERSION_BUILD:
      return FSF.atoi(BUILD);
  }
  return 0;
}

int WINAPI api_get_string(int type,char *str,size_t size,size_t *ret_size)
{
  size_t len=0;
  int ret_val=FALSE;
  switch(type)
  {
    case FMSTR_START:
    case FMSTR_END:
      {
        char buffer[101];
        GetRegKey2(HKEY_CURRENT_USER,PluginCommonKey,NULLSTR,(type==FMSTR_START)?STARTBLOCK:ENDBLOCK,buffer,(type==FMSTR_START)?DEFSTARTBLOCK:DEFENDBLOCK,sizeof(buffer)-1);
        len=lstrlen(buffer)+1;
        if(size>=len)
        {
          lstrcpy(str,buffer);
          ret_val=TRUE;
        }
      }
      break;
    case FMSTR_MSGEXT:
      {
        len=lstrlen(Opt.EXT)+1;
        if(size>=len)
        {
          lstrcpy(str,Opt.EXT);
          ret_val=TRUE;
        }
      }
      break;
  }
  if(ret_size) *ret_size=len;
  return ret_val;
}

int WINAPI api_address_book(char *str)
{
  return pm->ShowAddressBookMenu(str);
}

void WINAPI api_encode_header(const char *in,char *out,size_t *out_size,const char *charset)
{
  if (!in || !charset || !out_size)
  {
    if (out_size)
      *out_size=0;
    return;
  }
  size_t os=*out_size;
  *out_size=0;
  long length=lstrlen(in);
  if(length)
  {
    long enc_length;
    //RFC2047 2. Syntax of encoded-words
    enc_length=(75-7-lstrlen(charset));
    enc_length=enc_length/4*3;
    if (!out)
    {
      *out_size=(size_t)((length/3+1)*4+(length/enc_length+1)*(lstrlen(charset)+8)+1)*sizeof(char);
      return;
    }
    if(os) *out=0;
    char *buffer=(char *)z_malloc(((length+1)+(length/3+1)*4+(length/enc_length+1)*(lstrlen(charset)+8)+1)*sizeof(char));
    if(buffer)
    {
      char *encoded_in=buffer,*encoded_out=buffer+length+1;
      lstrcpy(encoded_in,in);
      *encoded_out=0;

      CHARSET_TABLE *CharsetTable=NULL;
      if(!ConstructCharset(&CharsetTable))
      {
        ReadCharsetTable(CharsetTable);
        InitCharset(&CharsetTable);
        int t=FindCharset(charset,&CharsetTable);
        if(t>=0)
        {
         for(int i=0;i<length;i++)
           encoded_in[i]=CharsetTable[t].EncodeTable[encoded_in[i]];
        }
      }
      DestructCharset(&CharsetTable);

      long curr_pos=0;
      while(curr_pos<length)
      {
        long curr_len=((length-curr_pos)>enc_length)?enc_length:(length-curr_pos);
        if(curr_pos) lstrcat(encoded_out," ");
        lstrcat(encoded_out,"=?");
        lstrcat(encoded_out,charset);
        lstrcat(encoded_out,"?B?");
        EncodeBase64(encoded_out+lstrlen(encoded_out),encoded_in+curr_pos,curr_len);
        lstrcat(encoded_out,"?=");
        curr_pos+=curr_len;
      }
      if(os>(size_t)lstrlen(encoded_out))
      {
         lstrcpy(out,encoded_out);
         *out_size=(size_t)lstrlen(encoded_out)+1;
      }
    }
    z_free(buffer);
  }
}
