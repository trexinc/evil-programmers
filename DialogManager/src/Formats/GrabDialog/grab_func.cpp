/*
    GrabDialog plugin for DialogManager
    Copyright (C) 2003-2004 Vadim Yegorov and Alex Yaroslavsky

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
#include <stdio.h>
#include "grab_func.hpp"

const char szDI_TEXT[] = "DI_TEXT";
const char szDI_VTEXT[] = "DI_VTEXT";
const char szDI_SINGLEBOX[] = "DI_SINGLEBOX";
const char szDI_DOUBLEBOX[] = "DI_DOUBLEBOX";
const char szDI_EDIT[] = "DI_EDIT";
const char szDI_PSWEDIT[] = "DI_PSWEDIT";
const char szDI_FIXEDIT[] = "DI_FIXEDIT";
const char szDI_BUTTON[] = "DI_BUTTON";
const char szDI_CHECKBOX[] = "DI_CHECKBOX";
const char szDI_RADIOBUTTON[] = "DI_RADIOBUTTON";
const char szDI_COMBOBOX[] = "DI_COMBOBOX";
const char szDI_LISTBOX[] = "DI_LISTBOX";
const char szDI_USERCONTROL[] = "DI_USERCONTROL";

const char szDIF_COLORMASK[] = "DIF_COLORMASK";
const char szDIF_SETCOLOR[] = "DIF_SETCOLOR";
const char szDIF_BOXCOLOR[] = "DIF_BOXCOLOR";
const char szDIF_GROUP[] = "DIF_GROUP";
const char szDIF_LEFTTEXT[] = "DIF_LEFTTEXT";
const char szDIF_MOVESELECT[] = "DIF_MOVESELECT";
const char szDIF_SHOWAMPERSAND[] = "DIF_SHOWAMPERSAND";
const char szDIF_CENTERGROUP[] = "DIF_CENTERGROUP";
const char szDIF_NOBRACKETS[] = "DIF_NOBRACKETS";
const char szDIF_MANUALADDHISTORY[] = "DIF_MANUALADDHISTORY";
const char szDIF_SEPARATOR[] = "DIF_SEPARATOR";
const char szDIF_VAREDIT[] = "DIF_VAREDIT";
const char szDIF_SEPARATOR2[] = "DIF_SEPARATOR2";
const char szDIF_EDITOR[] = "DIF_EDITOR";
const char szDIF_LISTNOAMPERSAND[] = "DIF_LISTNOAMPERSAND";
const char szDIF_LISTNOBOX[] = "DIF_LISTNOBOX";
const char szDIF_HISTORY[] = "DIF_HISTORY";
const char szDIF_BTNNOCLOSE[] = "DIF_BTNNOCLOSE";
const char szDIF_CENTERTEXT[] = "DIF_CENTERTEXT";
const char szDIF_EDITEXPAND[] = "DIF_EDITEXPAND";
const char szDIF_DROPDOWNLIST[] = "DIF_DROPDOWNLIST";
const char szDIF_USELASTHISTORY[] = "DIF_USELASTHISTORY";
const char szDIF_MASKEDIT[] = "DIF_MASKEDIT";
const char szDIF_SELECTONENTRY[] = "DIF_SELECTONENTRY";
const char szDIF_3STATE[] = "DIF_3STATE";
const char szDIF_LISTWRAPMODE[] = "DIF_LISTWRAPMODE";
const char szDIF_LISTAUTOHIGHLIGHT[] = "DIF_LISTAUTOHIGHLIGHT";
const char szDIF_HIDDEN[] = "DIF_HIDDEN";
const char szDIF_READONLY[] = "DIF_READONLY";
const char szDIF_NOFOCUS[] = "DIF_NOFOCUS";
const char szDIF_DISABLE[] = "DIF_DISABLE";

typedef struct DlgGrb
{
  DWORD Value;
  const char *Name;
};

DlgGrb DlgTextFlags[] =
{
  {DIF_SEPARATOR,szDIF_SEPARATOR},
  {DIF_SEPARATOR2,szDIF_SEPARATOR2},
  {DIF_BOXCOLOR,szDIF_BOXCOLOR},
  {DIF_CENTERGROUP,szDIF_CENTERGROUP},
  {DIF_SHOWAMPERSAND,szDIF_SHOWAMPERSAND},
  {DIF_CENTERTEXT,szDIF_CENTERTEXT},
  {DIF_COLORMASK,szDIF_COLORMASK},
  {DIF_SETCOLOR,szDIF_SETCOLOR},
  {DIF_HIDDEN,szDIF_HIDDEN},
  {DIF_DISABLE,szDIF_DISABLE},
};

DlgGrb DlgVTextFlags[] =
{
  {DIF_BOXCOLOR,szDIF_BOXCOLOR},
  {DIF_CENTERTEXT,szDIF_CENTERTEXT},
  {DIF_COLORMASK,szDIF_COLORMASK},
  {DIF_SETCOLOR,szDIF_SETCOLOR},
  {DIF_HIDDEN,szDIF_HIDDEN},
  {DIF_DISABLE,szDIF_DISABLE},
};

DlgGrb DlgDoubleBoxFlags[] =
{
  {DIF_LEFTTEXT,szDIF_LEFTTEXT},
  {DIF_SHOWAMPERSAND,szDIF_SHOWAMPERSAND},
  {DIF_COLORMASK,szDIF_COLORMASK},
  {DIF_SETCOLOR,szDIF_SETCOLOR},
  {DIF_HIDDEN,szDIF_HIDDEN},
  {DIF_DISABLE,szDIF_DISABLE},
};

DlgGrb DlgSingleBoxFlags[] =
{
  {DIF_LEFTTEXT,szDIF_LEFTTEXT},
  {DIF_SHOWAMPERSAND,szDIF_SHOWAMPERSAND},
  {DIF_COLORMASK,szDIF_COLORMASK},
  {DIF_SETCOLOR,szDIF_SETCOLOR},
  {DIF_HIDDEN,szDIF_HIDDEN},
  {DIF_DISABLE,szDIF_DISABLE},
};

DlgGrb DlgEditFlags[] =
{
  {DIF_MANUALADDHISTORY,szDIF_MANUALADDHISTORY},
  {DIF_VAREDIT,szDIF_VAREDIT},
  {DIF_EDITOR,szDIF_EDITOR},
  {DIF_HISTORY,szDIF_HISTORY},
  {DIF_EDITEXPAND,szDIF_EDITEXPAND},
  {DIF_USELASTHISTORY,szDIF_USELASTHISTORY},
  {DIF_SELECTONENTRY,szDIF_SELECTONENTRY},
  {DIF_READONLY,szDIF_READONLY},
  {DIF_NOFOCUS,szDIF_NOFOCUS},
  {DIF_HIDDEN,szDIF_HIDDEN},
  {DIF_DISABLE,szDIF_DISABLE},
};

DlgGrb DlgFixEditFlags[] =
{
  {DIF_MANUALADDHISTORY,szDIF_MANUALADDHISTORY},
  {DIF_HISTORY,szDIF_HISTORY},
  {DIF_MASKEDIT,szDIF_MASKEDIT},
  {DIF_USELASTHISTORY,szDIF_USELASTHISTORY},
  {DIF_SELECTONENTRY,szDIF_SELECTONENTRY},
  {DIF_READONLY,szDIF_READONLY},
  {DIF_NOFOCUS,szDIF_NOFOCUS},
  {DIF_HIDDEN,szDIF_HIDDEN},
  {DIF_DISABLE,szDIF_DISABLE},
};

DlgGrb DlgPswEditFlags[] =
{
  {DIF_SELECTONENTRY,szDIF_SELECTONENTRY},
  {DIF_READONLY,szDIF_READONLY},
  {DIF_NOFOCUS,szDIF_NOFOCUS},
  {DIF_HIDDEN,szDIF_HIDDEN},
  {DIF_DISABLE,szDIF_DISABLE},
};

DlgGrb DlgComboBoxFlags[] =
{
  {DIF_VAREDIT,szDIF_VAREDIT},
  {DIF_EDITEXPAND,szDIF_EDITEXPAND},
  {DIF_DROPDOWNLIST,szDIF_DROPDOWNLIST},
  {DIF_SELECTONENTRY,szDIF_SELECTONENTRY},
  {DIF_LISTWRAPMODE,szDIF_LISTWRAPMODE},
  {DIF_LISTAUTOHIGHLIGHT,szDIF_LISTAUTOHIGHLIGHT},
  {DIF_LISTNOAMPERSAND,szDIF_LISTNOAMPERSAND},
  {DIF_READONLY,szDIF_READONLY},
  {DIF_NOFOCUS,szDIF_NOFOCUS},
  {DIF_HIDDEN,szDIF_HIDDEN},
  {DIF_DISABLE,szDIF_DISABLE},
};

DlgGrb DlgListBoxFlags[] =
{
  {DIF_LISTNOAMPERSAND,szDIF_LISTNOAMPERSAND},
  {DIF_LISTNOBOX,szDIF_LISTNOBOX},
  {DIF_LISTWRAPMODE,szDIF_LISTWRAPMODE},
  {DIF_LISTAUTOHIGHLIGHT,szDIF_LISTAUTOHIGHLIGHT},
  {DIF_NOFOCUS,szDIF_NOFOCUS},
  {DIF_HIDDEN,szDIF_HIDDEN},
  {DIF_DISABLE,szDIF_DISABLE},
};

DlgGrb DlgRadioButtonFlags[] =
{
  {DIF_GROUP,szDIF_GROUP},
  {DIF_MOVESELECT,szDIF_MOVESELECT},
  {DIF_CENTERGROUP,szDIF_CENTERGROUP},
  {DIF_SHOWAMPERSAND,szDIF_SHOWAMPERSAND},
  {DIF_NOFOCUS,szDIF_NOFOCUS},
  {DIF_COLORMASK,szDIF_COLORMASK},
  {DIF_SETCOLOR,szDIF_SETCOLOR},
  {DIF_HIDDEN,szDIF_HIDDEN},
  {DIF_DISABLE,szDIF_DISABLE},
};

DlgGrb DlgCheckBoxFlags[] =
{
  {DIF_CENTERGROUP,szDIF_CENTERGROUP},
  {DIF_SHOWAMPERSAND,szDIF_SHOWAMPERSAND},
  {DIF_3STATE,szDIF_3STATE},
  {DIF_NOFOCUS,szDIF_NOFOCUS},
  {DIF_COLORMASK,szDIF_COLORMASK},
  {DIF_SETCOLOR,szDIF_SETCOLOR},
  {DIF_HIDDEN,szDIF_HIDDEN},
  {DIF_DISABLE,szDIF_DISABLE},
};

DlgGrb DlgButtonFlags[] =
{
  {DIF_CENTERGROUP,szDIF_CENTERGROUP},
  {DIF_NOBRACKETS,szDIF_NOBRACKETS},
  {DIF_SHOWAMPERSAND,szDIF_SHOWAMPERSAND},
  {DIF_BTNNOCLOSE,szDIF_BTNNOCLOSE},
  {DIF_NOFOCUS,szDIF_NOFOCUS},
  {DIF_COLORMASK,szDIF_COLORMASK},
  {DIF_SETCOLOR,szDIF_SETCOLOR},
  {DIF_HIDDEN,szDIF_HIDDEN},
  {DIF_DISABLE,szDIF_DISABLE},
};

DlgGrb DlgUserControlFlags[] =
{
  {DIF_NOFOCUS,szDIF_NOFOCUS},
  {DIF_HIDDEN,szDIF_HIDDEN},
  {DIF_DISABLE,szDIF_DISABLE},
};

DlgGrb DlgUnknownFlags[] =
{
  {DIF_HIDDEN,szDIF_HIDDEN},
  {DIF_DISABLE,szDIF_DISABLE},
};

DlgGrb DlgItems[] =
{
  {DI_TEXT,szDI_TEXT},
  {DI_VTEXT,szDI_VTEXT},
  {DI_SINGLEBOX,szDI_SINGLEBOX},
  {DI_DOUBLEBOX,szDI_DOUBLEBOX},
  {DI_EDIT,szDI_EDIT},
  {DI_PSWEDIT,szDI_PSWEDIT},
  {DI_FIXEDIT,szDI_FIXEDIT},
  {DI_BUTTON,szDI_BUTTON},
  {DI_CHECKBOX,szDI_CHECKBOX},
  {DI_RADIOBUTTON,szDI_RADIOBUTTON},
  {DI_COMBOBOX,szDI_COMBOBOX},
  {DI_LISTBOX,szDI_LISTBOX},
  {DI_USERCONTROL,szDI_USERCONTROL},
};

const char *GetDlgGrbNameByValueEq(DlgGrb *DlgGrbItems, int Items, DWORD Value)
{
  const char *ptr=NULL;
  for (int i=0; i<Items; i++)
    if (Value==DlgGrbItems[i].Value)
    {
      ptr=DlgGrbItems[i].Name;
      break;
    }
  return ptr;
}

const char *GetDlgGrbNameByValueAnd(DlgGrb *DlgGrbItems, int Items, int *FromIndex, DWORD *Value)
{
  const char *ptr=NULL;
  for (; *FromIndex<Items; (*FromIndex)++)
  {
    if ((*Value)&DlgGrbItems[*FromIndex].Value)
    {
      ptr=DlgGrbItems[*FromIndex].Name;
      (*Value)&=~(DlgGrbItems[*FromIndex].Value);
      (*FromIndex)++;
      break;
    }
  }
  return ptr;
}

const char *GetDlgItemNameByType(int Type)
{
  static char unknown[50];
  const char *ptr=GetDlgGrbNameByValueEq(DlgItems,sizeofa(DlgItems),Type);
  if (ptr)
    return ptr;
  FSF.sprintf(unknown,"%d",Type);
  return unknown;
}

const char *GetDlgFlagNameByValueAndType(int *FromIndex, DWORD *Value, int Type)
{
  static char unknown[50];
  if (*Value==0)
    return NULL;
  DlgGrb *DlgPtr;
  int Items;
  switch (Type)
  {
    case DI_TEXT:
      DlgPtr=DlgTextFlags;
      Items=sizeofa(DlgTextFlags);
      break;
    case DI_VTEXT:
      DlgPtr=DlgVTextFlags;
      Items=sizeofa(DlgVTextFlags);
      break;
    case DI_SINGLEBOX:
      DlgPtr=DlgSingleBoxFlags;
      Items=sizeofa(DlgSingleBoxFlags);
      break;
    case DI_DOUBLEBOX:
      DlgPtr=DlgDoubleBoxFlags;
      Items=sizeofa(DlgDoubleBoxFlags);
      break;
    case DI_EDIT:
      DlgPtr=DlgEditFlags;
      Items=sizeofa(DlgEditFlags);
      break;
    case DI_PSWEDIT:
      DlgPtr=DlgPswEditFlags;
      Items=sizeofa(DlgPswEditFlags);
      break;
    case DI_FIXEDIT:
      DlgPtr=DlgFixEditFlags;
      Items=sizeofa(DlgFixEditFlags);
      break;
    case DI_BUTTON:
      DlgPtr=DlgButtonFlags;
      Items=sizeofa(DlgButtonFlags);
      break;
    case DI_CHECKBOX:
      DlgPtr=DlgCheckBoxFlags;
      Items=sizeofa(DlgCheckBoxFlags);
      break;
    case DI_RADIOBUTTON:
      DlgPtr=DlgRadioButtonFlags;
      Items=sizeofa(DlgRadioButtonFlags);
      break;
    case DI_COMBOBOX:
      DlgPtr=DlgComboBoxFlags;
      Items=sizeofa(DlgComboBoxFlags);
      break;
    case DI_LISTBOX:
      DlgPtr=DlgListBoxFlags;
      Items=sizeofa(DlgListBoxFlags);
      break;
    case DI_USERCONTROL:
      DlgPtr=DlgUserControlFlags;
      Items=sizeofa(DlgUserControlFlags);
      break;
    default:
      DlgPtr=DlgUnknownFlags;
      Items=sizeofa(DlgUnknownFlags);
  }
  const char *ptr=GetDlgGrbNameByValueAnd(DlgPtr,Items,FromIndex,Value);
  if (ptr)
    return ptr;
  FSF.sprintf(unknown,"0x%08lx",*Value);
  *Value=0;
  return unknown;
}

const char *RawStringToCString(const char *str)
{
  static char temp[64*1024];
  *temp=0;
  for (size_t i=0; i<strlen(str); i++)
  {
    switch (str[i])
    {
      case '\n':
        strcat(temp,"\\n");
        break;
      case '\r':
        strcat(temp,"\\r");
        break;
      case '\\':
        strcat(temp,"\\\\");
        break;
      case '"':
        strcat(temp,"\\\"");
        break;
      case '\t':
        strcat(temp,"\\t");
        break;
      case '\a':
        strcat(temp,"\\a");
        break;
      case '\b':
        strcat(temp,"\\b");
        break;
      case '\f':
        strcat(temp,"\\f");
        break;
      case '\v':
        strcat(temp,"\\v");
        break;
      default:
        strncat(temp,str+i,1);
    }
  }
  return temp;
}

static unsigned long PrintfFile(HANDLE file,const char *format,...)
{
  va_list vl;
  char buff[1024*16]; unsigned long transferred=0;
  {
    va_start(vl,format);
    vsprintf(buff,format,vl);
    va_end(vl);
  }
  if(file!=INVALID_HANDLE_VALUE)
  {
    LONG dist=0;
    WaitForSingleObject(GrabMutex,INFINITE);
    if((SetFilePointer(file,0,&dist,FILE_END)!=0xFFFFFFFF)||(GetLastError()==NO_ERROR))
      WriteFile(file,buff,strlen(buff),&transferred,NULL);
    ReleaseMutex(GrabMutex);
  }
  return transferred;
}

void SimpleGrab(HANDLE log, MenuInData *dlg)
{
  FarDialogItem DialogItem;
  int Focus=-1, DefaultButton=-1;
  PrintfFile(log,"InitDialogItem idi[] =\n{\n");
  PrintfFile(log,"        /*Type,X1,Y1,X2,Y2,Param,Flags,Data*/\n");
  for(int i=0; FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,i,(long)&DialogItem); i++)
  {
    PrintfFile(log,"  /*%02d*/{",i);
    PrintfFile(log,GetDlgItemNameByType(DialogItem.Type));
    PrintfFile(log,",%d,%d,%d,%d",DialogItem.X1,DialogItem.Y1,DialogItem.X2,DialogItem.Y2);
    switch (DialogItem.Type)
    {
      case DI_CHECKBOX:
      case DI_RADIOBUTTON:
        PrintfFile(log,",%s",DialogItem.Param.Selected?"TRUE":"FALSE");
        break;
      case DI_FIXEDIT:
        if ((DialogItem.Flags&DIF_MASKEDIT)&&(!(DialogItem.Flags&DIF_HISTORY)))
        {
          if (DialogItem.Param.Mask)
            PrintfFile(log,",\"%s\"/*Mask=0x%8p*/",RawStringToCString(DialogItem.Param.Mask),DialogItem.Param.Mask);
          else
            PrintfFile(log,",0/*Mask=NULL*/");
          break;
        }
        // ^-- continue to check if history present --\/
      case DI_EDIT:
        if (DialogItem.Flags&DIF_HISTORY)
        {
          if (DialogItem.Param.History)
            PrintfFile(log,",\"%s\"/*History=0x%8p*/",RawStringToCString(DialogItem.Param.History),DialogItem.Param.History);
          else
            PrintfFile(log,",0/*History=NULL*/");
        }
        else
          PrintfFile(log,",%d",DialogItem.Param.Selected);
        break;
      case DI_USERCONTROL:
        PrintfFile(log,",0x%8p/*VBuf*/",DialogItem.Param.VBuf);
        break;
      case DI_LISTBOX:
      case DI_COMBOBOX:
        PrintfFile(log,",%d/*ListPos*/",DialogItem.Param.ListPos);
        break;
      default:
        PrintfFile(log,",%d",DialogItem.Param.Selected);
        break;
    }
    PrintfFile(log,",");
    {
      bool first_flag=true;
      int Index=0;
      const char *ptr;
      DWORD Flags=DialogItem.Flags;
      while ((ptr=GetDlgFlagNameByValueAndType(&Index,&Flags,DialogItem.Type)))
      {
        if(first_flag)
        {
          first_flag=false;
        }
        else
        {
          PrintfFile(log,"|");
        }
        PrintfFile(log,ptr);
      }
      if(first_flag)
        PrintfFile(log,"0");
    }
    if ((DialogItem.Type==DI_COMBOBOX||DialogItem.Type==DI_EDIT)&&DialogItem.Flags&DIF_VAREDIT)
    {
      if (DialogItem.Data.Ptr.PtrData)
        PrintfFile(log,",/*PtrData=0x%8p, PtrLength=%d*/\"%s\"},\n",DialogItem.Data.Ptr.PtrData,DialogItem.Data.Ptr.PtrLength,DialogItem.Data.Ptr.PtrData);
      else
        PrintfFile(log,",/*PtrData=NULL, Data:*/\"%s\"},\n",RawStringToCString(DialogItem.Data.Data));
    }
    else
    {
      PrintfFile(log,",\"%s\"},\n",RawStringToCString(DialogItem.Data.Data));
    }
    if (DefaultButton==-1&&DialogItem.DefaultButton)
      DefaultButton=i;
    if (Focus==-1&&DialogItem.Focus)
      Focus=i;
  }
  if (DefaultButton==-1)
      PrintfFile(log,"  /*DefaultButton=None");
  else
    PrintfFile(log,"  /*DefaultButton=%d",DefaultButton);
  if (Focus==-1)
      PrintfFile(log,", Focus=None*/");
  else
    PrintfFile(log,", Focus=%d*/",Focus);
  PrintfFile(log,"\n}\n\n");
}

void FullGrab(HANDLE log, MenuInData *dlg)
{
  FarDialogItem DialogItem;
  int Indent=4;
  PrintfFile(log,"FarDialogItem fdi[] =\n{\n");
  for(int i=0; FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,i,(long)&DialogItem); i++)
  {
    PrintfFile(log,"  { /*%02d*/",i);
    PrintfFile(log,"\n%*s",Indent," ");
    PrintfFile(log,"/*Type=*/");
    PrintfFile(log,GetDlgItemNameByType(DialogItem.Type));
    PrintfFile(log,",\n%*s",Indent," ");
    PrintfFile(log,"/*X1=*/%d, /*Y1=*/%d, /*X2=*/%d, /*Y2=*/%d",DialogItem.X1,DialogItem.Y1,DialogItem.X2,DialogItem.Y2);
    PrintfFile(log,",\n%*s",Indent," ");
    PrintfFile(log,"/*Focus=*/%d",DialogItem.Focus);
    PrintfFile(log,",\n%*s",Indent," ");
    PrintfFile(log,"/*Param.");
    switch (DialogItem.Type)
    {
      case DI_CHECKBOX:
      case DI_RADIOBUTTON:
        PrintfFile(log,"Selected=*/%d",DialogItem.Param.Selected);
        break;
      case DI_FIXEDIT:
        if ((DialogItem.Flags&DIF_MASKEDIT)&&(!(DialogItem.Flags&DIF_HISTORY)))
        {
          if (DialogItem.Param.Mask)
            PrintfFile(log,"Mask=0x%8p*/\"%s\"",DialogItem.Param.Mask,RawStringToCString(DialogItem.Param.Mask));
          else
            PrintfFile(log,"Mask=NULL*/0");
          break;
        }
        // ^-- continue to check if history present --\/
      case DI_EDIT:
        if (DialogItem.Flags&DIF_HISTORY)
        {
          if (DialogItem.Param.History)
            PrintfFile(log,"History=0x%8p*/\"%s\"",DialogItem.Param.History,RawStringToCString(DialogItem.Param.History));
          else
            PrintfFile(log,"History=NULL*/0");
        }
        else
          PrintfFile(log,"Selected=*/%d",DialogItem.Param.Selected);
        break;
      case DI_USERCONTROL:
        PrintfFile(log,"VBuf=*/0x%8p",DialogItem.Param.VBuf);
        break;
      case DI_LISTBOX:
      case DI_COMBOBOX:
        PrintfFile(log,"ListPos=*/%d",DialogItem.Param.ListPos);
        break;
      default:
        PrintfFile(log,"Selected=*/%d",DialogItem.Param.Selected);
        break;
    }
    PrintfFile(log,",\n%*s",Indent," ");
    PrintfFile(log,"/*Flags=*/");
    {
      bool first_flag=true;
      int Index=0;
      const char *ptr;
      DWORD Flags=DialogItem.Flags;
      while ((ptr=GetDlgFlagNameByValueAndType(&Index,&Flags,DialogItem.Type)))
      {
        if(first_flag)
        {
          first_flag=false;
          PrintfFile(log,"\n%*s",Indent+2," ");
        }
        else
        {
          PrintfFile(log,"|\n%*s",Indent+2," ");
        }
        PrintfFile(log,ptr);
      }
      if(first_flag)
        PrintfFile(log,"0");
    }
    PrintfFile(log,",\n%*s",Indent," ");
    PrintfFile(log,"/*DefaultButton=*/%d",DialogItem.DefaultButton);
    PrintfFile(log,",\n%*s",Indent," ");
    PrintfFile(log,"/*Data.");
    if ((DialogItem.Type==DI_COMBOBOX||DialogItem.Type==DI_EDIT)&&DialogItem.Flags&DIF_VAREDIT)
    {
      if (DialogItem.Data.Ptr.PtrData)
        PrintfFile(log,"Ptr.PtrData=0x%8p, PtrLength=%d*/\"%s\"},\n",DialogItem.Data.Ptr.PtrData,DialogItem.Data.Ptr.PtrLength,DialogItem.Data.Ptr.PtrData);
      else
        PrintfFile(log,"Ptr.PtrData=NULL, Lets try Data:*/\"%s\"},\n",RawStringToCString(DialogItem.Data.Data));
    }
    else
    {
      PrintfFile(log,"Data:*/\"%s\"",RawStringToCString(DialogItem.Data.Data));
    }
    PrintfFile(log,"\n%*s},\n",Indent-2," ");
  }
  PrintfFile(log,"}\n\n");
}
