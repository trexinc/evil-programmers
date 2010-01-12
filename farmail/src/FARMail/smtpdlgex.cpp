/*
    FARMail plugin for FAR Manager
    Copyright (C) 2002-2005 FARMail Group
    Copyright (C) 1999,2000 Serge Alexandrov

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
#include "farkeys.hpp"

static const char *HistoryNameFrom="FARMailSender";
static const char *HistoryName="FARMailRecipient";
static const char *HistoryName2="FARMailCC";
static const char *HistoryName3="FARMailBCC";
static const char *SubjectHistoryName="FARMailSubject";
static const char VLINE[] = "³";
static const char ABOOK[] = "(&?)";
static const char ABOOK_CC[] = "&(?)";
static const char ABOOK_BCC[] = "(?&)";

enum
{
  S_SNDR=2,
  S_TXT1,
  S_RECIP,
  S_ADRBOOK,

  S_CC_TXT,
  S_CC_FLD,
  S_CC_ADR,

  S_BCC_TXT,
  S_BCC_FLD,
  S_BCC_ADR,

  S_TXT2,
  S_SUBJ,
  S_RB1_1,
  S_RB1_2,
  S_RB1_3,
  S_RB1_4,
  S_RB1_5,
  S_RB1_6,
  S_Z1,
  S_RB2_1,
  S_RB2_2,
  S_RB2_3,
  S_RB2_4,
  S_RB2_5,
  S_ENCODING,
  S_Z6,
  S_RB3_1,
  S_RB3_2,
  S_RB3_3,
  S_RB3_4,
  S_ENCODETO,
  S_Z2,
  S_Z3,
  S_Z4,
  S_Z5,
  S_Z5b,
  S_Z7,
  S_Z8,
  S_Z9,
  S_Z0,
  S_Z11,
  S_Z12,
  S_Z13,
  S_Z14,
  S__ATTACHES,
  S__USERHEADERS,
  S__OK,
  S__CANCEL,
  S__ALL,
  S__ANALYZE_ALL,
  S_H1,
  S_H2,
  S_H3,
  S_H4,
  S_H5,
  S_H6,
  S_H7,
  S_H8,
  S_H9,
};

struct MailSendDialogParams
{
  FARMail *obj;
  MAILSEND *parm;
  bool multi,all,analize_all;
};

void GenerateUserCharsetList(MailSendDialogParams *DlgParams,struct FarList *List)
{
 int i;
 int j;

 if (!List) return;

 List->ItemsNumber = 0;

 for (i=4, j=0; i<7; i++)
 {
   if (DlgParams->obj->CharsetTable[i].num != UNKNOWN_CHARSET_TABLE)
     j++;
 }

 List->Items = (struct FarListItem *) z_calloc(j, sizeof(struct FarListItem));
 if (!List->Items) return;

 for (i=4; i<7; i++)
 {
   if (DlgParams->obj->CharsetTable[i].num != UNKNOWN_CHARSET_TABLE)
   {
     lstrcpy(List->Items[i-4].Text,DlgParams->obj->CharsetTable[i].charset);
   }
 }
 List->ItemsNumber = j;
 List->Items[0].Flags = LIF_SELECTED;
 return;
}

static void UpdateMailSendDialog(HANDLE hDlg,MailSendDialogParams *DlgParams)
{
  if(DlgParams->multi)
  {
    for(int i=S_RB1_1;i<=S_ENCODING;i++) Dialog_Hide(i);
    for(int i=S_Z3;i<=S_Z14;i++) Dialog_Hide(i);
    for(int i=S__ATTACHES;i<=S__USERHEADERS;i++) Dialog_Hide(i);
    for(int i=S_H1;i<=S_H5;i++) Dialog_Hide(i);
    Dialog_Move(S_Z6,5,8);
    for(int i=S_RB3_1;i<=S_RB3_4;i++) Dialog_Move(i,6,9+i-S_RB3_1);
    for(int i=S_H6;i<=S_H9;i++) Dialog_Move(i,5,9+i-S_H6);
    Dialog_Move(S_ENCODETO,10,12);
    {
      SMALL_RECT pos;
      _Info.SendDlgMessage(hDlg,DM_GETITEMPOSITION,0,(long)&pos);
      pos.Bottom=13;
      _Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,0,(long)&pos);
    }
    for(int i=S_RB3_1; i<S_RB3_3; i++)
      if (DlgParams->obj->CharsetTable[i-S_RB3_1+2].num == UNKNOWN_CHARSET_TABLE)
        Dialog_Disable(i);
    {
      struct FarListInfo li;
      _Info.SendDlgMessage(hDlg,DM_LISTINFO,S_ENCODETO,(long)&li);
      if (!li.ItemsNumber)
      {
        Dialog_Disable(S_ENCODETO);
        Dialog_Disable(S_RB3_4);
      }
    }
    for(int i=S__OK;i<=S__CANCEL;i++) Dialog_MoveV(i,14);
    for(int i=S__ALL;i<=S__ANALYZE_ALL;i++) Dialog_MoveV(i,14);
    {
      SMALL_RECT in; COORD out;
      _Info.SendDlgMessage(hDlg,DM_GETDLGRECT,0,(long)&in);
      out.X=in.Right-in.Left+1;
      out.Y=16;
      _Info.SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(long)&out);
    }
  }
  else
  {
    {
      SMALL_RECT in; COORD out;
      _Info.SendDlgMessage(hDlg,DM_GETDLGRECT,0,(long)&in);
      out.X=in.Right-in.Left+1;
      out.Y=24;
      _Info.SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(long)&out);
    }
    for(int i=S_RB1_1;i<=S_ENCODING;i++) Dialog_Show(i);
    for(int i=S_Z3;i<=S_Z14;i++) Dialog_Show(i);
    for(int i=S_H1;i<=S_H5;i++) Dialog_Show(i);
    for(int i=S__ATTACHES;i<=S__USERHEADERS;i++) Dialog_Show(i);
    Dialog_Move(S_Z6,36,12);
    for(int i=S_RB3_1;i<=S_RB3_4;i++) Dialog_Move(i,37,13+i-S_RB3_1);
    for(int i=S_H6;i<=S_H9;i++) Dialog_Move(i,36,13+i-S_H6);
    Dialog_Move(S_ENCODETO,41,16);
    {
      SMALL_RECT pos;
      _Info.SendDlgMessage(hDlg,DM_GETITEMPOSITION,0,(long)&pos);
      pos.Bottom=22;
      _Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,0,(long)&pos);
    }
    for(int i=S_RB2_3; i<S_RB2_5; i++)
      if (DlgParams->obj->CharsetTable[i-S_RB2_3+2].num == UNKNOWN_CHARSET_TABLE)
        Dialog_Disable(i);
    {
      struct FarListInfo li;
      _Info.SendDlgMessage(hDlg,DM_LISTINFO,S_ENCODING,(long)&li);
      if (!li.ItemsNumber)
      {
        Dialog_Disable(S_ENCODING);
        Dialog_Disable(S_RB2_5);
      }
    }
    for(int i=S_RB3_1; i<S_RB3_3; i++)
      if (DlgParams->obj->CharsetTable[i-S_RB3_1+2].num == UNKNOWN_CHARSET_TABLE)
        Dialog_Disable(i);
    {
      struct FarListInfo li;
      _Info.SendDlgMessage(hDlg,DM_LISTINFO,S_ENCODETO,(long)&li);
      if (!li.ItemsNumber)
      {
        Dialog_Disable(S_ENCODETO);
        Dialog_Disable(S_RB3_4);
      }
    }
    for(int i=S__OK;i<=S__CANCEL;i++) Dialog_MoveV(i,21);
    for(int i=S__ALL;i<=S__ANALYZE_ALL;i++) Dialog_MoveV(i,21);
  }
  if(DlgParams->parm->UseCC)
  {
    for(int i=S_CC_TXT;i<=S_BCC_ADR;i++)
      Dialog_Show(i);
    for(int i=S_TXT1;i<=S_ADRBOOK;i++)
      Dialog_MoveV(i,3);
  }
  else
  {
    for(int i=S_CC_TXT;i<=S_BCC_ADR;i++)
      Dialog_Hide(i);
    for(int i=S_TXT1;i<=S_ADRBOOK;i++)
      Dialog_MoveV(i,4);
  }
  if(DlgParams->all)
  {
    Dialog_Show(S__ALL);
    Dialog_SetFlag(S__ALL,DIF_CENTERGROUP);
  }
  else
  {
    Dialog_Hide(S__ALL);
    Dialog_UnsetFlag(S__ALL,DIF_CENTERGROUP);
  }
  if(DlgParams->analize_all)
  {
    Dialog_Show(S__ANALYZE_ALL);
    Dialog_SetFlag(S__ANALYZE_ALL,DIF_CENTERGROUP);
  }
  else
  {
    Dialog_Hide(S__ANALYZE_ALL);
    Dialog_UnsetFlag(S__ANALYZE_ALL,DIF_CENTERGROUP);
  }
}

static void LoadMailSendDialog(HANDLE hDlg,MailSendDialogParams *DlgParams)
{
  UpdateMailSendDialog(hDlg,DlgParams);
  {
    bool old_unc;
    old_unc=Dialog_GetUnchanged(S_SNDR);
    Dialog_SetText(S_SNDR,DlgParams->parm->Sender);
    Dialog_SetUnchanged(S_SNDR,old_unc);
    old_unc=Dialog_GetUnchanged(S_RECIP);
    Dialog_SetText(S_RECIP,DlgParams->parm->Recipient);
    Dialog_SetUnchanged(S_RECIP,old_unc);
    old_unc=Dialog_GetUnchanged(S_SUBJ);
    Dialog_SetText(S_SUBJ,DlgParams->parm->Subject);
    Dialog_SetUnchanged(S_SUBJ,old_unc);
    old_unc=Dialog_GetUnchanged(S_CC_FLD);
    Dialog_SetText(S_CC_FLD,DlgParams->parm->CC);
    Dialog_SetUnchanged(S_CC_FLD,old_unc);
    old_unc=Dialog_GetUnchanged(S_BCC_FLD);
    Dialog_SetText(S_BCC_FLD,DlgParams->parm->BCC);
    Dialog_SetUnchanged(S_BCC_FLD,old_unc);
  }

  Dialog_SetState(S_RB1_1-1+DlgParams->parm->how,true);

  int charset_in,charset_out;

  if(DlgParams->multi)
  {
    charset_out=FindCharset(DlgParams->parm->encode,&(DlgParams->obj->CharsetTable));
    if(charset_out>3)
    {
      charset_out=5;
      Dialog_SetText(S_ENCODETO,DlgParams->parm->encode);
    }
    if(charset_out<2) charset_out=4;
    Dialog_SetState(S_RB3_1+charset_out-2,true);
  }
  else
  {
    charset_in=FindCharset(DlgParams->parm->charset,&(DlgParams->obj->CharsetTable));
    if(charset_in>3)
    {
      charset_in=4;
      Dialog_SetText(S_ENCODING,DlgParams->parm->charset);
    }
    Dialog_SetState(S_RB2_1+charset_in,true);

    charset_out=FindCharset(DlgParams->parm->encode,&(DlgParams->obj->CharsetTable));
    if(charset_in==charset_out&&charset_out<4)
    {
      charset_out=2;
    }
    else if(charset_out>3)
    {
      charset_out=3;
      Dialog_SetText(S_ENCODETO,DlgParams->parm->encode);
    }
    else if(charset_out==2)
    {
      charset_out=0;
    }
    else if(charset_out==3)
    {
      charset_out=1;
    }
    else
    {
      charset_out=2;
    }
    Dialog_SetState(S_RB3_1+charset_out,true);
  }
}

static void SaveMailSendDialog(HANDLE hDlg,MailSendDialogParams *DlgParams)
{
  Dialog_GetText(S_SNDR,DlgParams->parm->Sender);
  Dialog_GetText(S_RECIP,DlgParams->parm->Recipient);
  Dialog_GetText(S_SUBJ,DlgParams->parm->Subject);
  Dialog_GetText(S_CC_FLD,DlgParams->parm->CC);
  Dialog_GetText(S_BCC_FLD,DlgParams->parm->BCC);
  if(!DlgParams->multi)
  {
    DlgParams->parm->how=0;
    for(int i=S_RB1_1;i<=S_RB1_6;i++)
    {
      DlgParams->parm->how++;
      if(Dialog_GetState(i)) break;
    }
    *(DlgParams->parm->charset) = 0;
    for(int i=S_RB2_1;i<=S_RB2_5;i++)
    {
      if(Dialog_GetState(i))
      {
        if(i==S_RB2_5)
          Dialog_GetText(S_ENCODING,DlgParams->parm->charset);
        else
          lstrcpy(DlgParams->parm->charset,DlgParams->obj->CharsetTable[i-S_RB2_1].charset);
        break;
      }
    }
  }
  if(Dialog_GetState(S_RB3_1))
    lstrcpy(DlgParams->parm->encode,DlgParams->obj->CharsetTable[2].charset);
  else if(Dialog_GetState(S_RB3_2))
    lstrcpy(DlgParams->parm->encode,DlgParams->obj->CharsetTable[3].charset);
  else if(Dialog_GetState(S_RB3_3))
  {
    if(DlgParams->multi) lstrcpy(DlgParams->parm->encode,DlgParams->obj->CharsetTable[0].charset);
    else lstrcpy(DlgParams->parm->encode,DlgParams->parm->charset);
  }
  else
    Dialog_GetText(S_ENCODETO,DlgParams->parm->encode);
}

static long WINAPI MailSendDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  MailSendDialogParams *DlgParams=(MailSendDialogParams *)_Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_INITDIALOG:
      _Info.SendDlgMessage(hDlg,DM_SETDLGDATA,0,Param2);
      LoadMailSendDialog(hDlg,(MailSendDialogParams *)Param2);
      break;
    case DN_BTNCLICK:
      switch(Param1)
      {
        case S_ADRBOOK:
        case S_CC_ADR:
        case S_BCC_ADR:
        {
          char new_data[512];
          *new_data=0;
          Dialog_ShowDialog(FALSE);
          pm->ShowAddressBookMenu(new_data);
          Dialog_ShowDialog(TRUE);
          if(*new_data) Dialog_SetText(Param1-1,new_data); //WARNING!
          Dialog_Focus(Param1-1);
          return TRUE;
        }
        case S_RB2_1:
        case S_RB2_2:
        case S_RB2_3:
        case S_RB2_4:
        case S_RB2_5:
        {
          char oldcharset[512];
          lstrcpy(oldcharset,DlgParams->parm->charset);
          if (Param1 != S_RB2_5)
            lstrcpy(DlgParams->parm->charset,DlgParams->obj->CharsetTable[Param1-S_RB2_1].charset);
          else
            Dialog_GetText(S_ENCODING,DlgParams->parm->charset);
          if (Dialog_GetUnchanged(S_SNDR)&&DlgParams->parm->UserSender)
          {
            Dialog_GetText(S_SNDR,DlgParams->parm->Sender);
            DlgParams->obj->EncodeStr8(DlgParams->parm->Sender, oldcharset);
            DlgParams->obj->DecodeStr8(DlgParams->parm->Sender, DlgParams->parm->charset);
            Dialog_SetText(S_SNDR,DlgParams->parm->Sender);
            Dialog_SetUnchanged(S_SNDR,true);
          }

          if (Dialog_GetUnchanged(S_RECIP))
          {
            Dialog_GetText(S_RECIP,DlgParams->parm->Recipient);
            DlgParams->obj->EncodeStr8(DlgParams->parm->Recipient, oldcharset);
            DlgParams->obj->DecodeStr8(DlgParams->parm->Recipient, DlgParams->parm->charset);
            Dialog_SetText(S_RECIP,DlgParams->parm->Recipient);
            Dialog_SetUnchanged(S_RECIP,true);
          }

          if (Dialog_GetUnchanged(S_SUBJ))
          {
            Dialog_GetText(S_SUBJ,DlgParams->parm->Subject);
            DlgParams->obj->EncodeStr8(DlgParams->parm->Subject, oldcharset);
            DlgParams->obj->DecodeStr8(DlgParams->parm->Subject, DlgParams->parm->charset);
            Dialog_SetText(S_SUBJ,DlgParams->parm->Subject);
            Dialog_SetUnchanged(S_SUBJ,true);
          }

          if (Dialog_GetUnchanged(S_CC_FLD))
          {
            Dialog_GetText(S_CC_FLD,DlgParams->parm->CC);
            DlgParams->obj->EncodeStr8(DlgParams->parm->CC, oldcharset);
            DlgParams->obj->DecodeStr8(DlgParams->parm->CC, DlgParams->parm->charset);
            Dialog_SetText(S_CC_FLD,DlgParams->parm->CC);
            Dialog_SetUnchanged(S_CC_FLD,true);
          }

          if (Dialog_GetUnchanged(S_BCC_FLD))
          {
            Dialog_GetText(S_BCC_FLD,DlgParams->parm->BCC);
            DlgParams->obj->EncodeStr8(DlgParams->parm->BCC, oldcharset);
            DlgParams->obj->DecodeStr8(DlgParams->parm->BCC, DlgParams->parm->charset);
            Dialog_SetText(S_BCC_FLD,DlgParams->parm->BCC);
            Dialog_SetUnchanged(S_BCC_FLD,true);
          }

          return TRUE;
        }
        case S__ATTACHES:
        {
          return TRUE;
        }
        case S__USERHEADERS:
        {
          return TRUE;
        }
      }
      break;
    case DN_KEY:
      switch (Param2)
      {
        case KEY_ALTSHIFT|KEY_SLASH:
        case KEY_ALT|KEY_SLASH:
          Dialog_Click(S_ADRBOOK);
          return TRUE;
        case KEY_ALTSHIFT9:
        //case KEY_ALT9:
          if(DlgParams->parm->UseCC)
            Dialog_Click(S_CC_ADR);
          return TRUE;
        case KEY_ALTSHIFT0:
        //case KEY_ALT0:
          if(DlgParams->parm->UseCC)
            Dialog_Click(S_BCC_ADR);
          return TRUE;
        case KEY_CTRLSHIFTC:
          DlgParams->parm->UseCC=!DlgParams->parm->UseCC;
          UpdateMailSendDialog(hDlg,DlgParams);
          return TRUE;
        case KEY_CTRLL:
          {
            switch(Param1)
            {
              case S_RECIP:
              case S_CC_FLD:
              case S_BCC_FLD:
              case S_SUBJ:
                Dialog_SetUnchanged(Param1,true);
                return TRUE;
              case S_SNDR:
                if(!Dialog_GetUnchanged(S_SNDR))
                  DlgParams->parm->UserSender=true;
                Dialog_SetUnchanged(Param1,true);
                return TRUE;
          }
          }
          break;
#if 0
        case KEY_CTRLSHIFT|'M':
          DlgParams->multi=!DlgParams->multi;
          UpdateMailSendDialog(hDlg,DlgParams);
          return TRUE;
#endif
      }
      break;
    case DN_HOTKEY:
      switch(Param1)
      {
        case S_H1:
        case S_H2:
        case S_H3:
        case S_H4:
        case S_H5:
        {
          if (!Dialog_IsEnabled(Param1-S_H1+S_RB2_1))
            return FALSE;
          Dialog_Focus(Param1-S_H1+S_RB2_1);
          Dialog_SetState(Param1-S_H1+S_RB2_1,true);
          Dialog_Click(Param1-S_H1+S_RB2_1);
          return FALSE;
        }
        case S_H6:
        case S_H7:
        case S_H8:
        case S_H9:
        {
          if (!Dialog_IsEnabled(Param1-S_H6+S_RB3_1))
            return FALSE;
          Dialog_Focus(Param1-S_H6+S_RB3_1);
          Dialog_SetState(Param1-S_H6+S_RB3_1,true);
          Dialog_Click(Param1-S_H6+S_RB3_1);
          return FALSE;
        }
      }
      break;
    case DN_CLOSE:
      switch(Param1)
      {
        case S__OK:
        case S__ALL:
        case S__ANALYZE_ALL:
        {
          SaveMailSendDialog(hDlg,DlgParams);
          //int charset=FindCharset(DlgParams->parm->encode,&(DlgParams->obj->CharsetTable));
          //if(charset<0||DlgParams->obj->CharsetTable[charset].num<0)
          //{
          //  SayError(::GetMsg(MesSendDialog_NoSuchEncoding));
          //  return FALSE;
          //}
          if(Param1==S__ALL||Param1==S__ANALYZE_ALL) DlgParams->parm->all=TRUE;
          if(Param1==S__ANALYZE_ALL) DlgParams->parm->analize_all=TRUE;
          break;
        }
      }
      break;
  }
  return _Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

int FARMail::MailSendDialog(MAILSEND *parm,char *name,bool multi,bool all,bool analize_all)
{
  int i,j;
  char str[40];
  char str2[200];

  struct InitDialogItem InitItems[]=
  {
    //type,x1,y1,x2,y2,focus,selected,flags,default,data
    {DI_DOUBLEBOX,3,1,67,22,0,0,DIF_SHOWAMPERSAND,0,str2},
    //new
    {DI_TEXT,5,2,0,0,0,0,0,0,(char*)MesSendDialog_From},
    {DI_EDIT,17,2,64,2,0,(DWORD)HistoryNameFrom,DIF_HISTORY,0,NULLSTR},
    //end new
    {DI_TEXT,5,3,0,0,0,0,0,0,(char*)MesSendDialog_To},
    {DI_EDIT,17,3,60,3,0,(DWORD)HistoryName,DIF_HISTORY,0,NULLSTR},
    {DI_BUTTON,63,3,0,0,0,0,DIF_NOBRACKETS|DIF_BTNNOCLOSE|DIF_NOFOCUS,0,ABOOK},

    {DI_TEXT,5,4,0,0,0,0,0,0,(char*)MesSendDialog_CC},
    {DI_EDIT,17,4,60,4,0,(DWORD)HistoryName2,DIF_HISTORY,0,NULLSTR},// 3
    {DI_BUTTON,63,4,0,0,0,0,DIF_NOBRACKETS|DIF_BTNNOCLOSE|DIF_NOFOCUS,0,ABOOK_CC},

    {DI_TEXT,5,5,0,0,0,0,0,0,(char*)MesSendDialog_BCC},
    {DI_EDIT,17,5,60,5,0,(DWORD)HistoryName3,DIF_HISTORY,0,NULLSTR},// 3
    {DI_BUTTON,63,5,0,0,0,0,DIF_NOBRACKETS|DIF_BTNNOCLOSE|DIF_NOFOCUS,0,ABOOK_BCC},

    {DI_TEXT,5,6,0,0,0,0,0,0,(char*)MesSendDialog_Subj},
    {DI_EDIT,17,6,64,6,0,(DWORD)SubjectHistoryName,DIF_HISTORY,0,NULLSTR},
    {DI_RADIOBUTTON,17,8,0,0,0,0,DIF_GROUP ,0,str},
    {DI_RADIOBUTTON,17,9,0,0,0,0,0,0,(char*)MesSendDialog_SendAs_TextBase64},
    {DI_RADIOBUTTON,17,10,0,0,0,0,0,0,(char*)MesSendDialog_SendAs_AttachedFile},
// -- new
    {DI_RADIOBUTTON,44,8,0,0,0,0,0,0,(char*)MesSendDialog_SendAs_HtmlAsIs},
    {DI_RADIOBUTTON,44,9,0,0,0,0,0,0,(char*)MesSendDialog_SendAs_HtmlBase64},
    {DI_RADIOBUTTON,44,10,0,0,0,0,0,0,(char*)MesSendDialog_SendAs_Raw},

    {DI_TEXT,5,12,0,0,0,0,0,0,(char*)MesSendDialog_OriginalCharset},
    {DI_RADIOBUTTON,7,13,32,0,0,0,DIF_GROUP,0,(char*)MesSendDialog_OEM},
    {DI_RADIOBUTTON,7,14,32,0,0,0,0,0,(char*)MesSendDialog_USASCII},

    {DI_RADIOBUTTON,7,15,32,0,0,0,0,0,NULLSTR},
    {DI_RADIOBUTTON,7,16,32,0,0,0,0,0,NULLSTR},
    {DI_RADIOBUTTON,7,17,32,0,0,0,0,0,NULLSTR},

    {DI_COMBOBOX,11,17,31,17,0,0,0,0,NULLSTR},

    {DI_TEXT,36,12,0,0,0,0,0,0,(char*)MesSendDialog_EncodeTo},
    {DI_RADIOBUTTON,37,13,62,0,0,0,DIF_GROUP,0,NULLSTR},
    {DI_RADIOBUTTON,37,14,62,0,0,0,0,0,NULLSTR},
    {DI_RADIOBUTTON,37,15,62,0,0,0,0,0,(char*)MesSendDialog_DoNotEncode},
    {DI_RADIOBUTTON,37,16,62,0,0,0,0,0,NULLSTR},

    {DI_COMBOBOX,41,16,64,16,0,0,0,0,NULLSTR},

    {DI_TEXT,3,7,0,0,0,0,DIF_SEPARATOR,0,NULLSTR},// separator
    {DI_TEXT,5,8,0,0,0,0,0,0,(char*)MesSendDialog_SendAs},
    {DI_TEXT,3,11,0,0,0,0,DIF_SEPARATOR,0,NULLSTR},// separator
    {DI_TEXT,3,18,0,0,0,0,DIF_SEPARATOR,0,NULLSTR},// separator
    {DI_TEXT,3,20,0,0,0,0,DIF_SEPARATOR,0,NULLSTR},// separator

    {DI_TEXT,34,11,34,0,0,0,0,0,"Â"},
    {DI_TEXT,34,12,34,0,0,0,0,0,VLINE},
    {DI_TEXT,34,13,34,0,0,0,0,0,VLINE},
    {DI_TEXT,34,14,34,0,0,0,0,0,VLINE},
    {DI_TEXT,34,15,34,0,0,0,0,0,VLINE},
    {DI_TEXT,34,16,34,0,0,0,0,0,VLINE},
    {DI_TEXT,34,17,34,0,0,0,0,0,VLINE},
    {DI_TEXT,34,18,34,0,0,0,0,0,"Á"},

    {DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,0,NULLSTR},
    {DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,0,NULLSTR},
    {DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,1,(char *)MesOk},
    {DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,0,(char *)MesCancel},
    {DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,0,(char*)MesSendDialog_AllBtn},
    {DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,0,(char*)MesSendDialog_AnalizeAllBtn},
    {DI_TEXT,6,13,6,0,0,0,0,0, (char*)MesHotkey1},
    {DI_TEXT,6,14,6,0,0,0,0,0, (char*)MesHotkey2},
    {DI_TEXT,6,15,6,0,0,0,0,0, (char*)MesHotkey3},
    {DI_TEXT,6,16,6,0,0,0,0,0, (char*)MesHotkey4},
    {DI_TEXT,6,17,6,0,0,0,0,0, (char*)MesHotkey5},
    {DI_TEXT,36,13,36,0,0,0,0,0,(char*)MesHotkey6},
    {DI_TEXT,36,14,36,0,0,0,0,0,(char*)MesHotkey7},
    {DI_TEXT,36,15,36,0,0,0,0,0,(char*)MesHotkey8},
    {DI_TEXT,36,16,36,0,0,0,0,0,(char*)MesHotkey9}
  };
  struct FarList ListEncoding;
  struct FarList ListEncodeTo;

  FarDialogItem DialogItems[sizeofa(InitItems)];
  FSF.sprintf(str,::GetMsg(MesSendDialog_SendAs_TextAsIs),parm->nbit);
  if(multi)
  {
    lstrcpy(str2,::GetMsg(MesSendDialog_MultiPartTitle));
  }
  else
  {
    if(!name||parm->infofound)
    {
      if(Opt.ConfirmBinary&&parm->bitcontrol)
      {
        const char *MsgItems[]={GetMsg(MesOverwrite_Title),GetMsg(MesConfirmBinary_Message),GetMsg(MesConfirmBinary_Continue),GetMsg(MesDelete_CancelBtn),GetMsg(MesOverwrite_SkipBtn)};
        int i=1;
        if (name)
          i=0;
        i=_Info.Message(_Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeofa(MsgItems)-i,3-i);
        if (i==1 || i==-1)
         return TRUE;
        else if (i==2)
          return -1;
      }
      if (!name)
        name=GetEditorFileName();
    }
    FSF.sprintf(str2,::GetMsg(MesSendDialog_Title),name);
  }
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));

  MailSendDialogParams params={this,parm,multi,all,analize_all};

  if (*parm->Recipient) DialogItems[S_SUBJ].Focus=1;
  else DialogItems[S_RECIP].Focus=1;

  for (int i=0; i<2; i++)
  {
    lstrcpy(DialogItems[S_RB2_3+i].Data, CharsetTable[i+2].charset);
    lstrcpy(DialogItems[S_RB3_1+i].Data, CharsetTable[i+2].charset);
  }

  FSF.sprintf(DialogItems[S__ATTACHES].Data,GetMsg(MesSendDialog_AttachesBtn),parm->Header.CountAttach());
  FSF.sprintf(DialogItems[S__USERHEADERS].Data,GetMsg(MesSendDialog_UserHeadersBtn),parm->Header.CountHeader());

  GenerateUserCharsetList(&params, &ListEncoding);
  DialogItems[S_ENCODING].ListItems = &ListEncoding;
  GenerateUserCharsetList(&params, &ListEncodeTo);
  DialogItems[S_ENCODETO].ListItems = &ListEncodeTo;

  int key=_Info.DialogEx(_Info.ModuleNumber,-1,-1,71,24,"SendOptions",DialogItems,sizeofa(DialogItems),0,0,MailSendDialogProc,(DWORD)&params);

  z_free(ListEncoding.Items);
  z_free(ListEncodeTo.Items);

  if(key==S__OK||key==S__ALL||key==S__ANALYZE_ALL)
  {
    FinalMacroProcessing(parm);
    return FALSE;
  }
  return TRUE;
}
