/*
    PasteSelection plugin for DialogManager
    Copyright (C) 2004 Alex Yaroslavsky

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
#include <stdlib.h>
#include "../../plugin.hpp"
#include "../../dm_module.hpp"

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}

enum
{
  mName,
};

PluginStartupInfo FarInfo;
DialogPluginStartupInfo DialogInfo;

char *GetMsg(int MsgNum,char *Str)
{
  DialogInfo.GetMsg(DialogInfo.MessageName,MsgNum,Str);
  return Str;
}

inline unsigned char hex(unsigned char c)
{
  if (c<10) return ('0'+c);
  return ('A'+c-10);
}

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct DialogPluginStartupInfo *DialogInfo)
{
  ::FarInfo=*FarInfo;
  ::DialogInfo=*DialogInfo;
  return 0;
}

void WINAPI _export Exit(void)
{

}

int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData)
{
  (void)OutData;
  switch(Msg)
  {
    case FMMSG_GETINFO:
    {
      GetInfoOutData *data=(GetInfoOutData *)OutData;
      GetMsg(mName,data->MenuString);
      strcpy(data->HotkeyID,"ay_pasteselection");
      data->Flags=FMMSG_MENU;
      return TRUE;
    }
    case FMMSG_MENU:
    {
      MenuInData *dlg=(MenuInData *)InData;
      FarDialogItem DialogItem;
      FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,dlg->ItemID,(long)&DialogItem);
      if((DialogItem.Type==DI_EDIT||DialogItem.Type==DI_FIXEDIT||DialogItem.Type==DI_PSWEDIT||(DialogItem.Type==DI_COMBOBOX&&(!(DialogItem.Flags&DIF_DROPDOWNLIST)))))
      {
        EditorInfo ei;
        if(FarInfo.EditorControl(ECTL_GETINFO,&ei)&&ei.BlockType!=BTYPE_NONE)
        {
          EditorGetString egs;
          egs.StringNumber=ei.BlockStartLine;
          FarInfo.EditorControl(ECTL_GETSTRING,&egs);
          if(egs.SelStart>=0&&egs.SelStart<egs.StringLength)
          {
            int len=(egs.SelEnd>=0)?(egs.SelEnd-egs.SelStart):(egs.StringLength-egs.SelStart);
            if (len)
            {
              char *text=(char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,len+1);
              if (text)
              {
                memcpy(text,egs.StringText+egs.SelStart,len);
                EditorConvertText ect;
                ect.Text=(char *)text;
                ect.TextLength=len;
                FarInfo.EditorControl(ECTL_EDITORTOOEM,&ect);
                text[len]=0;
                len=strlen(text);
                if (len)
                {
                  char *buffer=(char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,len*4+8+1);
                  if (buffer)
                  {
                    strcpy(buffer,"$Text \"");
                    for (int i=7, j=0; j<len; j++)
                    {
                      buffer[i++]='\\';
                      buffer[i++]='x';
                      buffer[i++]=hex(((unsigned char)text[j])/16);
                      buffer[i++]=hex(((unsigned char)text[j])%16);
                    }
                    strcpy(buffer+len*4+7,"\"");
                    ActlKeyMacro seq;
                    seq.Command=MCMD_POSTMACROSTRING;
                    seq.Param.PlainText.SequenceText=buffer;
                    seq.Param.PlainText.Flags=KSFLAGS_DISABLEOUTPUT;
                    FarInfo.AdvControl(FarInfo.ModuleNumber,ACTL_KEYMACRO,&seq);
                    HeapFree(GetProcessHeap(),0,buffer);
                  }
                }
                HeapFree(GetProcessHeap(),0,text);
              }
            }
          }
        }
      }
      return TRUE;
    }
  }
  return FALSE;
}
