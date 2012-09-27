/*
    pasteselection.cpp
    Copyright (C) 2004 Alex Yaroslavsky
    Copyright (C) 2008 zg

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

#include "dt.hpp"
#include "guid.hpp"
#include <stdio.h>

inline TCHAR hex(TCHAR c)
{
  if (c<10) return ('0'+c);
  return ('A'+c-10);
}

#define MACRO_LEN 6

void DoPaste(HANDLE aDlg)
{
  LONG_PTR itemID=Info.SendDlgMessage(aDlg,DM_GETFOCUS,0,0);
  FarDialogItem DialogItem;
  Info.SendDlgMessage(aDlg,DM_GETDLGITEMSHORT,itemID,&DialogItem);
  if((DialogItem.Type==DI_EDIT||DialogItem.Type==DI_FIXEDIT||DialogItem.Type==DI_PSWEDIT||(DialogItem.Type==DI_COMBOBOX&&(!(DialogItem.Flags&DIF_DROPDOWNLIST)))))
  {
    EditorInfo ei={sizeof(EditorInfo)};
    if(Info.EditorControl(-1,ECTL_GETINFO,0,&ei)&&ei.BlockType!=BTYPE_NONE)
    {
      EditorGetString egs={sizeof(EditorGetString)};
      egs.StringNumber=ei.BlockStartLine;
      Info.EditorControl(-1,ECTL_GETSTRING,0,&egs);
      if(egs.SelStart>=0&&egs.SelStart<egs.StringLength)
      {
        int len=(egs.SelEnd>=0)?(egs.SelEnd-egs.SelStart):(egs.StringLength-egs.SelStart);
        if (len)
        {
          TCHAR *text=(TCHAR *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(len+1)*sizeof(TCHAR));
          if (text)
          {
            memcpy(text,egs.StringText+egs.SelStart,len*sizeof(TCHAR));
            text[len]=0;
            len=_tcslen(text);
            if (len)
            {
              TCHAR *buffer=(TCHAR *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(len*MACRO_LEN+8+1)*sizeof(TCHAR));
              if (buffer)
              {
                _tcscpy(buffer,_T("print(\""));
                for (int i=7, j=0; j<len; j++)
                {
                  buffer[i++]='\\';
                  buffer[i++]='x';
                  unsigned value=text[j];
                  buffer[i++]=hex(value/4096);
                  value%=4096;
                  buffer[i++]=hex(value/256);
                  value%=256;
                  buffer[i++]=hex(value/16);
                  buffer[i++]=hex(value%16);
                }
                _tcscpy(buffer+len*MACRO_LEN+7,_T("\")"));
                MacroSendMacroText seq={sizeof(MacroSendMacroText),KMFLAGS_DISABLEOUTPUT,{0},buffer};
                Info.MacroControl(0,MCTL_SENDSTRING,MSSC_POST,&seq);
                HeapFree(GetProcessHeap(),0,buffer);
              }
            }
            HeapFree(GetProcessHeap(),0,text);
          }
        }
      }
    }
  }
}
