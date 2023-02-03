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
#include "guid.hpp"
#include <initguid.h>
// {44769AF2-E33D-4c76-9EA4-A0E5B795A604}
DEFINE_GUID(ReplaceDialogGuid, 0x44769af2, 0xe33d, 0x4c76, 0x9e, 0xa4, 0xa0, 0xe5, 0xb7, 0x95, 0xa6, 0x4);

intptr_t WINAPI ReplaceDialogProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void DoReplace(HANDLE aDlg)
{
  LONG_PTR itemID=Info.SendDlgMessage(aDlg,DM_GETFOCUS,0,0);
  FarDialogItem DialogItem;
  Info.SendDlgMessage(aDlg,DM_GETDLGITEMSHORT,itemID,&DialogItem);
  if(DialogItem.Type==DI_EDIT||DialogItem.Type==DI_FIXEDIT)
  {
    size_t length=Info.SendDlgMessage(aDlg,DM_GETTEXT,itemID,0)+1;
    TCHAR *buffer=(TCHAR *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,length*sizeof(TCHAR));
    if(buffer)
    {
      FarDialogItemData getdata={sizeof(FarDialogItemData),length-1,buffer};
      Info.SendDlgMessage(aDlg,DM_GETTEXT,itemID,&getdata);
      COORD Pos; Info.SendDlgMessage(aDlg,DM_GETCURSORPOS,itemID,&Pos);
      __int64 c=0;
      __int64 p=1;
      {
        CFarSettings set(MainGuid);
        set.Get(_T("CaseSensitive"),c);
        set.Get(_T("SearchFromCurPos"),p);
      }
      enum {IBox,ISearchLabel,ISearch,IReplaceLabel,IReplace,IDummy1,ICase,ICurrentPosition,IDummy2,ISave,ICancel};
      FarDialogItem DialogItems[]=
      {//       Type            X1 Y1 X2 Y2 Se Fl                                     Data */
       /*00*/ { DI_DOUBLEBOX,   3, 1,55,11, {0},          NULL,                       NULL, DIF_BOXCOLOR,                          GetMsg(mReplaceCaption)  ,0,0},
       /*01*/ { DI_TEXT,        5, 2, 0, 2, {0},          NULL,                       NULL, 0,                                     GetMsg(mSearch)          ,0,0},
       /*02*/ { DI_EDIT,        5, 3,53, 3, {0},          _T("DialogManager.Search"), NULL, DIF_HISTORY|DIF_USELASTHISTORY,        _T("")                   ,0,0},
       /*03*/ { DI_TEXT,        5, 4, 0, 4, {0},          NULL,                       NULL, 0,                                     GetMsg(mReplace)         ,0,0},
       /*04*/ { DI_EDIT,        5, 5,53, 5, {0},          _T("DialogManager.Replace"),NULL, DIF_HISTORY|DIF_USELASTHISTORY,        _T("")                   ,0,0},
       /*05*/ { DI_TEXT,        0, 6, 0, 6, {0},          NULL,                       NULL, DIF_BOXCOLOR|DIF_SEPARATOR,            _T("")                   ,0,0},
       /*06*/ { DI_CHECKBOX,    5, 7, 0, 7, {(intptr_t)c},NULL,                       NULL, 0,                                     GetMsg(mCase)            ,0,0},
       /*07*/ { DI_CHECKBOX,    5, 8, 0, 8, {(intptr_t)p},NULL,                       NULL, 0,                                     GetMsg(mCurPos)          ,0,0},
       /*08*/ { DI_TEXT,        0, 9, 0, 9, {0},          NULL,                       NULL, DIF_BOXCOLOR|DIF_SEPARATOR,            _T("")                   ,0,0},
       /*09*/ { DI_BUTTON,      0,10, 0,10, {0},          NULL,                       NULL, DIF_CENTERGROUP|DIF_DEFAULTBUTTON,     GetMsg(mOk)              ,0,0},
       /*10*/ { DI_BUTTON,      0,10, 0,10, {0},          NULL,                       NULL, DIF_CENTERGROUP,                       GetMsg(mCancel)          ,0,0}
      };
      CFarDialog dialog;
      int n=dialog.Execute(MainGuid,ReplaceDialogGuid,-1,-1,59,13,NULL,DialogItems,ArraySize(DialogItems),0,0,ReplaceDialogProc,NULL);
      if (n==ISave)
      {
        c=dialog.Check(ICase)?1:0;
        p=dialog.Check(ICurrentPosition)?1:0;
        const wchar_t* search_str=dialog.Str(ISearch);
        size_t search_str_len=_tcslen(search_str);
        size_t search_str_skip=search_str_len?search_str_len:1;
        const wchar_t* replace_str=dialog.Str(IReplace);
        size_t replace_str_len=_tcslen(replace_str);
        CFarSettings set(MainGuid);
        set.Set(_T("CaseSensitive"),c);
        set.Set(_T("SearchFromCurPos"),p);
        TCHAR *buffer_temp=buffer;
        if (p==1)
          buffer_temp+=Pos.X;
        else
          Pos.X=0;
        size_t n=0;
        size_t l=_tcslen(buffer_temp);
        for (size_t i=0;i<l;)
        {
          int r=(c==1)?_tcsncmp(buffer_temp+i,search_str,search_str_len):FSF.LStrnicmp(buffer_temp+i,search_str,search_str_len);
          if (r==0)
          {
            n++;
            i+=search_str_skip;
          }
          else
            i++;
        }
        size_t newlength=_tcslen(buffer)-n*search_str_len+n*replace_str_len+1;
        TCHAR *newbuffer=(TCHAR *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,newlength*sizeof(TCHAR));
        for (size_t i=0;i<static_cast<size_t>(Pos.X);i++)
          newbuffer[i]=buffer[i];
        size_t j=Pos.X;
        for (size_t i=0;i<l;)
        {
          int r=(c==1)?_tcsncmp(buffer_temp+i,search_str,search_str_len):FSF.LStrnicmp(buffer_temp+i,search_str,search_str_len);
          if (r==0)
          {
            for (size_t k=0;k<replace_str_len;k++)
              newbuffer[j++]=replace_str[k];
            if (!search_str_len)
            {
              newbuffer[j++]=buffer_temp[i++];
            }
            i+=search_str_len;
          }
          else
          {
            newbuffer[j++]=buffer_temp[i++];
          }
        }
        newbuffer[j]='\0';
        Pos.X=j;
        Info.SendDlgMessage(aDlg,DM_SETTEXTPTR,itemID,newbuffer);
        Info.SendDlgMessage(aDlg,DM_SETCURSORPOS,itemID,&Pos);
        HeapFree(GetProcessHeap(),0,newbuffer);
      }
      HeapFree(GetProcessHeap(),0,buffer);
    }
  }
}
