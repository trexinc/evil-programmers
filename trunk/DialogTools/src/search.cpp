/*
    search.cpp
    Copyright (C) 2005 Shynkarenko Ivan aka 4ekucT
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
#include <initguid.h>
#include <stdio.h>
// {7452904C-CFD7-49b2-AE24-7ED22A9AAB99}
DEFINE_GUID(SearchDialogGuid, 0x7452904c, 0xcfd7, 0x49b2, 0xae, 0x24, 0x7e, 0xd2, 0x2a, 0x9a, 0xab, 0x99);

INT_PTR WINAPI SearchDialogProc(HANDLE hDlg,int Msg,int Param1,INT_PTR Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void DoSearch(HANDLE aDlg)
{
  LONG_PTR itemID=Info.SendDlgMessage(aDlg,DM_GETFOCUS,0,0);
  FarDialogItem DialogItem;
  Info.SendDlgMessage(aDlg,DM_GETDLGITEMSHORT,itemID,(LONG_PTR)&DialogItem);
  if(DialogItem.Type==DI_EDIT||DialogItem.Type==DI_FIXEDIT)
  {
    long length=Info.SendDlgMessage(aDlg,DM_GETTEXTLENGTH,itemID,0)+1;
    TCHAR *buffer=(TCHAR *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,length*sizeof(TCHAR));
    if(buffer)
    {
      Info.SendDlgMessage(aDlg,DM_GETTEXTPTR,itemID,(long)buffer);
      COORD Pos; Info.SendDlgMessage(aDlg,DM_GETCURSORPOS,itemID,(LONG_PTR)&Pos);
      int c=GetRegKey(HKEY_CURRENT_USER,_T(""),_T("CaseSensitive"),0);
      int p=GetRegKey(HKEY_CURRENT_USER,_T(""),_T("SearchFromCurPos"),1);
      struct InitDialogItem InitItems[]=
      {//       Type            X1 Y1 X2 Y2 Se Fl                                     Data */
       /*00*/ { DI_DOUBLEBOX,   3, 1,55, 9, 0, DIF_BOXCOLOR,                          (const TCHAR*)mSearchCaption},
       /*01*/ { DI_TEXT,        5, 2, 0, 2, 0, 0,                                     (const TCHAR*)mSearch},
       /*02*/ { DI_EDIT,        5, 3,53, 3, 0, DIF_HISTORY|DIF_USELASTHISTORY,        _T("")},
       /*03*/ { DI_TEXT,        0, 4, 0, 4, 0, DIF_BOXCOLOR|DIF_SEPARATOR,            _T("")},
       /*04*/ { DI_CHECKBOX,    5, 5, 0, 5, c, 0,                                     (const TCHAR*)mCase},
       /*05*/ { DI_CHECKBOX,    5, 6, 0, 6, p, 0,                                     (const TCHAR*)mCurPos},
       /*06*/ { DI_TEXT,        0, 7, 0, 7, 0, DIF_BOXCOLOR|DIF_SEPARATOR,            _T("")},
       /*07*/ { DI_BUTTON,      0, 8, 0, 8, 0, DIF_CENTERGROUP|DIF_DEFAULTBUTTON,     (const TCHAR*)mOk},
       /*08*/ { DI_BUTTON,      0, 8, 0, 8, 0, DIF_CENTERGROUP,                       (const TCHAR*)mCancel}
      };
      struct FarDialogItem DialogItems[(sizeof(InitItems)/sizeof(InitItems[0]))];
      InitDialogItems(InitItems,DialogItems,(sizeof(InitItems)/sizeof(InitItems[0])));
      DialogItems[2].History=_T("DialogManager.Search");
      CFarDialog dialog;
      int n=dialog.Execute(MainGuid,SearchDialogGuid,-1,-1,59,11,NULL,DialogItems,ArraySize(InitItems),0,0,SearchDialogProc,0);
      if (n==7)
      {
        c=(DialogItems[4].Selected==TRUE)?1:0;
        p=(DialogItems[5].Selected==TRUE)?1:0;
        SetRegKey(HKEY_CURRENT_USER,_T(""),_T("CaseSensitive"),c);
        SetRegKey(HKEY_CURRENT_USER,_T(""),_T("SearchFromCurPos"),p);
        if (p==1)
          buffer+=Pos.X;
        else
          Pos.X=0;
        unsigned l=_tcslen(buffer);
        for (unsigned i=0;i<l;i++)
        {
          int r=(c==1)?_tcsncmp(buffer+i,dialog.Str(2),(int)_tcslen(dialog.Str(2))):FSF.LStrnicmp(buffer+i,dialog.Str(2),(int)_tcslen(dialog.Str(2)));
          if (r==0)
          {
            Pos.X+=i;
            Info.SendDlgMessage(aDlg,DM_SETCURSORPOS,itemID,(LONG_PTR)&Pos);
            break;
          }
        }
      }
      HeapFree(GetProcessHeap(),0,buffer);
    }
  }
}
