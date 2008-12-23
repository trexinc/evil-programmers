/*
    replace.cpp
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

LONG_PTR WINAPI ReplaceDialogProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void DoReplace(HANDLE aDlg)
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
      Info.SendDlgMessage(aDlg,DM_GETTEXTPTR,itemID,(LONG_PTR)buffer);
      COORD Pos; Info.SendDlgMessage(aDlg,DM_GETCURSORPOS,itemID,(LONG_PTR)&Pos);
      int c=GetRegKey(HKEY_CURRENT_USER,_T(""),_T("CaseSensitive"),0);
      int p=GetRegKey(HKEY_CURRENT_USER,_T(""),_T("SearchFromCurPos"),1);
      struct InitDialogItem InitItems[]=
      {//       Type            X1 Y1 X2 Y2 Fo Se Fl                                    DB      Data */
       /*00*/ { DI_DOUBLEBOX,   3, 1,55,11, 0, 0, DIF_BOXCOLOR,                         0,      (const TCHAR*)mReplaceCaption},
       /*01*/ { DI_TEXT,        5, 2, 0, 2, 0, 0, 0,                                    0,      (const TCHAR*)mSearch},
       /*02*/ { DI_EDIT,        5, 3,53, 3, 0, 0, DIF_HISTORY|DIF_USELASTHISTORY,       0,      _T("")},
       /*03*/ { DI_TEXT,        5, 4, 0, 4, 0, 0, 0,                                    0,      (const TCHAR*)mReplace},
       /*04*/ { DI_EDIT,        5, 5,53, 5, 0, 0, DIF_HISTORY|DIF_USELASTHISTORY,       0,      _T("")},
       /*05*/ { DI_TEXT,        0, 6, 0, 6, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR,           0,      _T("")},
       /*06*/ { DI_CHECKBOX,    5, 7, 0, 7, 0, c, 0,                                    0,      (const TCHAR*)mCase},
       /*07*/ { DI_CHECKBOX,    5, 8, 0, 8, 0, p, 0,                                    0,      (const TCHAR*)mCurPos},
       /*08*/ { DI_TEXT,        0, 9, 0, 9, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR,           0,      _T("")},
       /*09*/ { DI_BUTTON,      0,10, 0,10, 0, 0, DIF_CENTERGROUP,                      1,      (const TCHAR*)mOk},
       /*10*/ { DI_BUTTON,      0,10, 0,10, 0, 0, DIF_CENTERGROUP,                      0,      (const TCHAR*)mCancel}
      };
      struct FarDialogItem DialogItems[(sizeof(InitItems)/sizeof(InitItems[0]))];
      InitDialogItems(InitItems,DialogItems,(sizeof(InitItems)/sizeof(InitItems[0])));
      DialogItems[2].History=_T("DialogManager.Search");
      DialogItems[4].History=_T("DialogManager.Replace");
      CFarDialog dialog;
      int n=dialog.Execute(Info.ModuleNumber,-1,-1,59,13,NULL,DialogItems,ArraySize(InitItems),0,0,ReplaceDialogProc,0);
      if (n==9)
      {
        c=(DialogItems[6].Selected==TRUE)?1:0;
        p=(DialogItems[7].Selected==TRUE)?1:0;
        SetRegKey(HKEY_CURRENT_USER,_T(""),_T("CaseSensitive"),c);
        SetRegKey(HKEY_CURRENT_USER,_T(""),_T("SearchFromCurPos"),p);
        if (p==1)
          buffer+=Pos.X;
        else
          Pos.X=0;
        unsigned n=0;
        unsigned l=_tcslen(buffer);
        for (unsigned i=0;i<l;)
        {
          int r=(c==1)?_tcsncmp(buffer+i,dialog.Str(2),(int)_tcslen(dialog.Str(2))):FSF.LStrnicmp(buffer+i,dialog.Str(2),(int)_tcslen(dialog.Str(2)));
          if (r==0)
          {
            n++;
            i+=(int)_tcslen(dialog.Str(2));
          }
          else
            i++;
        }
        unsigned newlength=_tcslen(buffer-Pos.X)-n*_tcslen(dialog.Str(2))+n*_tcslen(dialog.Str(4));
        TCHAR *newbuffer=(TCHAR *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,newlength*sizeof(TCHAR));
        for (int i=0;i<Pos.X;i++)
          newbuffer[i]=buffer[i-Pos.X];
        unsigned j=Pos.X;
        for (unsigned i=0;i<l;)
        {
          int r=(c==1)?_tcsncmp(buffer+i,dialog.Str(2),(int)_tcslen(dialog.Str(2))):FSF.LStrnicmp(buffer+i,dialog.Str(2),(int)_tcslen(dialog.Str(2)));
          if (r==0)
          {
            for (unsigned k=0;k<_tcslen(dialog.Str(4));k++)
              newbuffer[j++]=dialog.Str(4)[k];
            i+=(int)_tcslen(dialog.Str(2));
          }
          else
          {
            newbuffer[j]=buffer[i];
            i++;
            j++;
          }
        }
        newbuffer[j]='\0';
        Pos.X=j;
        Info.SendDlgMessage(aDlg,DM_SETTEXTPTR,itemID,(LONG_PTR)newbuffer);
#ifdef UNICODE
        FarDialogItem* pDialogItem;
        pDialogItem=(FarDialogItem*)(LONG_PTR)Info.SendDlgMessage(aDlg,DM_GETDLGITEM,itemID,0);
        Info.SendDlgMessage(aDlg,DN_EDITCHANGE,itemID,(LONG_PTR)pDialogItem);
        Info.SendDlgMessage(aDlg,DM_FREEDLGITEM,0,(LONG_PTR)pDialogItem);
#else
        Info.SendDlgMessage(aDlg,DM_GETDLGITEM,itemID,(LONG_PTR)&DialogItem);
        Info.SendDlgMessage(aDlg,DN_EDITCHANGE,itemID,(LONG_PTR)&DialogItem);
#endif
        Info.SendDlgMessage(aDlg,DM_SETCURSORPOS,itemID,(LONG_PTR)&Pos);
        HeapFree(GetProcessHeap(),0,newbuffer);
      }
      HeapFree(GetProcessHeap(),0,buffer);
    }
  }
}
