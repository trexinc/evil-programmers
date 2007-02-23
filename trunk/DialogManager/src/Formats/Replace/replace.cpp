/*
    Replace plugin for DialogManager
    Copyright (C) 2005 Shynkarenko Ivan aka 4ekucT

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
#include <stdlib.h>
#include "../../plugin.hpp"
#include "../../farkeys.hpp"
#include "replace.hpp"
#include "registry.hpp"

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

BOOL WINAPI DllMain(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}

PluginStartupInfo FarInfo;
FARSTANDARDFUNCTIONS FSF;
DialogPluginStartupInfo DialogInfo;
char PluginRootKey[100];

char *GetMsg(int MsgNum,char *Str)
{
  DialogInfo.GetMsg(DialogInfo.MessageName,MsgNum,Str);
  return Str;
}

void InitDialogItems(const struct InitDialogItem *Init,struct FarDialogItem *Item,int ItemsNumber)
{
 int I;
 struct FarDialogItem *PItem=Item;
 const struct InitDialogItem *PInit=Init;
 for (I=0;I<ItemsNumber;I++,PItem++,PInit++)
 {
  PItem->Type=PInit->Type;
  PItem->X1=PInit->X1;
  PItem->Y1=PInit->Y1;
  PItem->X2=PInit->X2;
  PItem->Y2=PInit->Y2;
  PItem->Focus=PInit->Focus;
  PItem->Selected=PInit->Selected;
  if (PInit->History!=NULL)
   PItem->History=PInit->History;
  PItem->Flags=PInit->Flags;
  PItem->DefaultButton=PInit->DefaultButton;
  strcpy(PItem->Data,PInit->Data);
 }
}

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct DialogPluginStartupInfo *DialogInfo)
{
  ::FarInfo=*FarInfo;
  FSF=*FarInfo->FSF;
  ::DialogInfo=*DialogInfo;
  FSF.sprintf(PluginRootKey,"%s\\Replace",::DialogInfo.RootKey);
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
        strcpy(data->HotkeyID,"4ekucT_replace");
        data->Flags=FMMSG_MENU;
      }
      return TRUE;
    case FMMSG_MENU:
      {
        MenuInData *dlg=(MenuInData *)InData;
        FarDialogItem DialogItem;
        FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,dlg->ItemID,(long)&DialogItem);
        if(DialogItem.Type==DI_EDIT||DialogItem.Type==DI_FIXEDIT)
        {
         long length=FarInfo.SendDlgMessage(dlg->hDlg,DM_GETTEXTLENGTH,dlg->ItemID,0)+1;
         char *buffer=(char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,length);
         if(buffer)
         {
          FarInfo.SendDlgMessage(dlg->hDlg,DM_GETTEXTPTR,dlg->ItemID,(long)buffer);
          COORD Pos; FarInfo.SendDlgMessage(dlg->hDlg,DM_GETCURSORPOS,dlg->ItemID,(long)&Pos);
          char msg1[128]; GetMsg(mReplaceCaption,msg1);
          char msg2[128]; GetMsg(mSearch,msg2);
          char msg3[128]; GetMsg(mReplace,msg3);
          char msg4[128]; GetMsg(mCase,msg4);
          char msg5[128]; GetMsg(mCurPos,msg5);
          char msg6[128]; GetMsg(mOK,msg6);
          char msg7[128]; GetMsg(mCancel,msg7);
          int c=GetRegKey(HKEY_CURRENT_USER,"","CaseSensitive",0);
          int p=GetRegKey(HKEY_CURRENT_USER,"","SearchFromCurPos",1);
          struct InitDialogItem InitItems[]=
          {//       Type            X1 Y1 X2 Y2 Fo Se Fl                                    DB      Data */
           /*00*/ { DI_DOUBLEBOX,   3, 1,55,11, 0, 0, DIF_BOXCOLOR,                         0,      msg1, NULL},
           /*01*/ { DI_TEXT,        5, 2, 0, 2, 0, 0, 0,                                    0,      msg2, NULL},
           /*02*/ { DI_EDIT,        5, 3,53, 3, 0, 0, DIF_HISTORY|DIF_USELASTHISTORY,       0,      "",   "DialogManager.Search"},
           /*03*/ { DI_TEXT,        5, 4, 0, 4, 0, 0, 0,                                    0,      msg3, NULL},
           /*04*/ { DI_EDIT,        5, 5,53, 5, 0, 0, DIF_HISTORY|DIF_USELASTHISTORY,       0,      "",   "DialogManager.Replace"},
           /*05*/ { DI_TEXT,        0, 6, 0, 6, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR,           0,      "",   NULL},
           /*06*/ { DI_CHECKBOX,    5, 7, 0, 7, 0, c, 0,                                    0,      msg4, NULL},
           /*07*/ { DI_CHECKBOX,    5, 8, 0, 8, 0, p, 0,                                    0,      msg5, NULL},
           /*08*/ { DI_TEXT,        0, 9, 0, 9, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR,           0,      "",   NULL},
           /*09*/ { DI_BUTTON,      0,10, 0,10, 0, 0, DIF_CENTERGROUP,                      1,      msg6, NULL},
           /*10*/ { DI_BUTTON,      0,10, 0,10, 0, 0, DIF_CENTERGROUP,                      0,      msg7, NULL}
          };
          struct FarDialogItem DialogItems[(sizeof(InitItems)/sizeof(InitItems[0]))];
          InitDialogItems(InitItems,DialogItems,(sizeof(InitItems)/sizeof(InitItems[0])));
          int n=FarInfo.Dialog(FarInfo.ModuleNumber,-1,-1,59,13,NULL,DialogItems,(sizeof(InitItems)/sizeof(InitItems[0])));
          if (n==9)
          {
           c=(DialogItems[6].Selected==TRUE)?1:0;
           p=(DialogItems[7].Selected==TRUE)?1:0;
           SetRegKey(HKEY_CURRENT_USER,"","CaseSensitive",c);
           SetRegKey(HKEY_CURRENT_USER,"","SearchFromCurPos",p);
           if (p==1)
            buffer+=Pos.X;
           else
            Pos.X=0;
           unsigned n=0;
           unsigned l=strlen(buffer);
           for (unsigned i=0;i<l;)
           {
            int r=(c==1)?strncmp(buffer+i,DialogItems[2].Data,(int)strlen(DialogItems[2].Data)):FSF.LStrnicmp(buffer+i,DialogItems[2].Data,(int)strlen(DialogItems[2].Data));
            if (r==0)
            {
             n++;
             i+=(int)strlen(DialogItems[2].Data);
            }
            else
             i++;
           }
           unsigned newlength=strlen(buffer-Pos.X)-n*strlen(DialogItems[2].Data)+n*strlen(DialogItems[4].Data);
           char *newbuffer=(char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,newlength);
           for (int i=0;i<Pos.X;i++)
            newbuffer[i]=buffer[i-Pos.X];
           unsigned j=Pos.X;
           for (unsigned i=0;i<l;)
           {
            int r=(c==1)?strncmp(buffer+i,DialogItems[2].Data,(int)strlen(DialogItems[2].Data)):FSF.LStrnicmp(buffer+i,DialogItems[2].Data,(int)strlen(DialogItems[2].Data));
            if (r==0)
            {
             for (unsigned k=0;k<strlen(DialogItems[4].Data);k++)
              newbuffer[j++]=DialogItems[4].Data[k];
             i+=(int)strlen(DialogItems[2].Data);
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
           FarInfo.SendDlgMessage(dlg->hDlg,DM_SETTEXTPTR,dlg->ItemID,(long)newbuffer);
           FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,dlg->ItemID,(long)&DialogItem);
           FarInfo.SendDlgMessage(dlg->hDlg,DN_EDITCHANGE,dlg->ItemID,(long)&DialogItem);
           FarInfo.SendDlgMessage(dlg->hDlg,DM_SETCURSORPOS,dlg->ItemID,(long)&Pos);
           HeapFree(GetProcessHeap(),0,newbuffer);
          }
          HeapFree(GetProcessHeap(),0,buffer);
         }
        }
      }
      return TRUE;
  }
  return FALSE;
}
