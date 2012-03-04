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

INT_PTR WINAPI SearchDialogProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void DoSearch(HANDLE aDlg)
{
  LONG_PTR itemID=Info.SendDlgMessage(aDlg,DM_GETFOCUS,0,0);
  FarDialogItem DialogItem;
  Info.SendDlgMessage(aDlg,DM_GETDLGITEMSHORT,itemID,&DialogItem);
  if(DialogItem.Type==DI_EDIT||DialogItem.Type==DI_FIXEDIT)
  {
    long length=Info.SendDlgMessage(aDlg,DM_GETTEXTLENGTH,itemID,0)+1;
    TCHAR *buffer=(TCHAR *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,length*sizeof(TCHAR));
    if(buffer)
    {
      Info.SendDlgMessage(aDlg,DM_GETTEXTPTR,itemID,buffer);
      COORD Pos; Info.SendDlgMessage(aDlg,DM_GETCURSORPOS,itemID,&Pos);
      __int64 c=0;
      __int64 p=1;
      {
        CFarSettings set(MainGuid);
        set.Get(_T("CaseSensitive"),c);
        set.Get(_T("SearchFromCurPos"),p);
      }
      FarDialogItem DialogItems[]=
      {//       Type            X1 Y1 X2 Y2 Se Fl                                     Data */
       /*00*/ { DI_DOUBLEBOX,   3, 1,55, 9, {0},NULL,                       NULL, DIF_BOXCOLOR,                          GetMsg(mSearchCaption) ,0,0},
       /*01*/ { DI_TEXT,        5, 2, 0, 2, {0},NULL,                       NULL, 0,                                     GetMsg(mSearch)        ,0,0},
       /*02*/ { DI_EDIT,        5, 3,53, 3, {0},_T("DialogManager.Search"), NULL, DIF_USELASTHISTORY,                    _T("")                 ,0,0},
       /*03*/ { DI_TEXT,        0, 4, 0, 4, {0},NULL,                       NULL, DIF_BOXCOLOR|DIF_SEPARATOR,            _T("")                 ,0,0},
       /*04*/ { DI_CHECKBOX,    5, 5, 0, 5, {c},NULL,                       NULL, 0,                                     GetMsg(mCase)          ,0,0},
       /*05*/ { DI_CHECKBOX,    5, 6, 0, 6, {p},NULL,                       NULL, 0,                                     GetMsg(mCurPos)        ,0,0},
       /*06*/ { DI_TEXT,        0, 7, 0, 7, {0},NULL,                       NULL, DIF_BOXCOLOR|DIF_SEPARATOR,            _T("")                 ,0,0},
       /*07*/ { DI_BUTTON,      0, 8, 0, 8, {0},NULL,                       NULL, DIF_CENTERGROUP|DIF_DEFAULTBUTTON,     GetMsg(mOk)            ,0,0},
       /*08*/ { DI_BUTTON,      0, 8, 0, 8, {0},NULL,                       NULL, DIF_CENTERGROUP,                       GetMsg(mCancel)        ,0,0}
      };
      CFarDialog dialog;
      int n=dialog.Execute(MainGuid,SearchDialogGuid,-1,-1,59,11,NULL,DialogItems,ArraySize(DialogItems),0,0,SearchDialogProc,NULL);
      if (n==7)
      {
        c=(DialogItems[4].Selected==TRUE)?1:0;
        p=(DialogItems[5].Selected==TRUE)?1:0;
        CFarSettings set(MainGuid);
        set.Set(_T("CaseSensitive"),c);
        set.Set(_T("SearchFromCurPos"),p);
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
            Info.SendDlgMessage(aDlg,DM_SETCURSORPOS,itemID,&Pos);
            break;
          }
        }
      }
      HeapFree(GetProcessHeap(),0,buffer);
    }
  }
}
