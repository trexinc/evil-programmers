/*
    pwd.cpp
    Copyright (C) 2003 Vadim Yegorov
    Copyright (C) 2004 Vadim Yegorov and Alex Yaroslavsky
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
#include <initguid.h>
// {A43FF539-60AB-413b-A61A-143F2863370B}
DEFINE_GUID(PwdDialogGuid, 0xa43ff539, 0x60ab, 0x413b, 0xa6, 0x1a, 0x14, 0x3f, 0x28, 0x63, 0x37, 0xb);

INT_PTR WINAPI PwdDialogProc(HANDLE hDlg,int Msg,int Param1,INT_PTR Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void DoPwd(HANDLE aDlg)
{
  LONG_PTR itemID=Info.SendDlgMessage(aDlg,DM_GETFOCUS,0,0);
  FarDialogItem DialogItem;
  Info.SendDlgMessage(aDlg,DM_GETDLGITEMSHORT,itemID,(LONG_PTR)&DialogItem);
  if(DialogItem.Type==DI_PSWEDIT)
  {
    long length=Info.SendDlgMessage(aDlg,DM_GETTEXTLENGTH,itemID,0)+1;
    TCHAR *buffer=(TCHAR *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,length*sizeof(TCHAR));
    if(buffer)
    {
      static const struct InitDialogItem PreDialogItems[] =
      {
        { DI_DOUBLEBOX   ,3  ,1  ,43 ,5 ,0 ,0                                 ,(const TCHAR*)mNamePwd   },
        { DI_TEXT        ,5  ,2  ,0  ,0 ,0 ,0                                 ,(const TCHAR*)mPwd       },
        { DI_EDIT        ,5  ,3  ,41 ,0 ,0 ,DIF_READONLY|DIF_FOCUS            ,_T("")                   },
        { DI_BUTTON      ,0  ,4  ,0  ,0 ,0 ,DIF_CENTERGROUP|DIF_DEFAULTBUTTON ,(const TCHAR*)mOk        }
      };
      struct FarDialogItem DialogItems[sizeof(PreDialogItems)/sizeof(PreDialogItems[0])];

      InitDialogItems(PreDialogItems,DialogItems,sizeof(PreDialogItems)/sizeof(PreDialogItems[0]));
      Info.SendDlgMessage(aDlg,DM_GETTEXTPTR,itemID,(long)buffer);
      DialogItems[2].PtrData=buffer;
      CFarDialog dialog;
      dialog.Execute(Info.ModuleNumber,PwdDialogGuid,-1,-1,47,7,NULL,DialogItems,ArraySize(PreDialogItems),0,0,PwdDialogProc,0);
      HeapFree(GetProcessHeap(),0,buffer);
    }
  }
}
