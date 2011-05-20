/*
    SelColor.cpp
    Copyright (C) 2002-2008 zg

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

#include "plugin.hpp"
#include "language.hpp"
#include "EditCmpl.hpp"
#include <stdio.h>
#include <tchar.h>
#include "guid.hpp"
#include <initguid.h>
// {E456CA5B-3A02-445f-A217-3BD52ABC3057}
DEFINE_GUID(ColorGuid, 0xe456ca5b, 0x3a02, 0x445f, 0xa2, 0x17, 0x3b, 0xd5, 0x2a, 0xbc, 0x30, 0x57);

enum
{
  color_box_fg =  0,
  color_box_bg = 17,
  color_set    = 34,
  color_cancel = 35,
  color_test1  = 36,
  color_test2  = 37,
  color_test3  = 38,
  color_border = 39,
  color_sep    = 40,
};

static INT_PTR WINAPI ColorDialogProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
  int color=0;
  FarDialogItem DialogItem;
  switch(Msg)
  {
    case DN_INITDIALOG:
    case DN_BTNCLICK:
      for(int i=1;i<17;i++)
      {
        if(Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,i,&DialogItem))
        {
          if(DialogItem.Selected)
          {
            color|=(DialogItem.Flags&0xF0)>>4;
            break;
          }
        }
      }
      for(int i=18;i<34;i++)
      {
        if(Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,i,&DialogItem))
        {
          if(DialogItem.Selected)
          {
            color|=(DialogItem.Flags&0xF0);
            break;
          }
        }
      }
      for(int i=36;i<39;i++)
      {
        if(Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,i,&DialogItem))
        {
          DialogItem.Flags=(DialogItem.Flags&0xffffff00)|color;
          Info.SendDlgMessage(hDlg,DM_SETDLGITEMSHORT,i,&DialogItem);
        }
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool SelectColor(int *fg,int *bg)
{
  bool result=false;
  //Show dialog
  /*
    000000000011111111112222222222333333333
    012345678901234567890123456789012345678
  00                                       00
  01   ÉÍÍÍÍÍÍÍÍÍÍÍÍ Color ÍÍÍÍÍÍÍÍÍÍÍÍ»   01
  02   º Ú Foreground ¿ Ú Background ¿ º   02
  03   º ³            ³ ³          * ³ º   03
  04   º ³            ³ ³            ³ º   04
  05   º ³            ³ ³            ³ º   05
  06   º ³          * ³ ³            ³ º   06
  07   º ÀÄÄÄÄÄÄÄÄÄÄÄÄÙ ÀÄÄÄÄÄÄÄÄÄÄÄÄÙ º   07
  08   º Text Text Text Text Text Text º   08
  09   º Text Text Text Text Text Text º   09
  10   º Text Text Text Text Text Text º   10
  11   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   11
  12   º      [ Set ]  [ Cancel ]      º   12
  13   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   13
  14                                       14
    000000000011111111112222222222333333333
    012345678901234567890123456789012345678
  */
  const unsigned char color_translate[]={0,4,8,0xc,1,5,9,0xd,2,6,0xa,0xe,3,7,0xb,0xf};
  const unsigned char colors[]={0x07,0x40,0x87,0xc0,0x17,0x50,0x90,0xd0,0x20,0x60,0xa0,0xe0,0x30,0x70,0xb0,0xf0};


  struct FarDialogItem DialogItems[color_sep+1];
  memset(DialogItems,0,sizeof(DialogItems));
  DialogItems[color_box_fg].Type=DI_SINGLEBOX;
  DialogItems[color_box_fg].X1=5;
  DialogItems[color_box_fg].Y1=2;
  DialogItems[color_box_fg].X2=18;
  DialogItems[color_box_fg].Y2=7;
  INIT_DLG_DATA(DialogItems[color_box_fg],GetMsg(MColorForeground));
  DialogItems[color_box_bg].Type=DI_SINGLEBOX;
  DialogItems[color_box_bg].X1=20;
  DialogItems[color_box_bg].Y1=2;
  DialogItems[color_box_bg].X2=33;
  DialogItems[color_box_bg].Y2=7;
  INIT_DLG_DATA(DialogItems[color_box_bg],GetMsg(MColorBackground));
  for(int i=0,shift=1;i<2;i++,shift+=color_box_bg-color_box_fg)
    for(int j=0;j<16;j++)
    {
      DialogItems[shift+j].Type=DI_RADIOBUTTON;
      DialogItems[shift+j].X1=(j/4+2)*3+i*15;
      DialogItems[shift+j].Y1=j%4+3;
      DialogItems[shift+j].Flags=DIF_MOVESELECT|DIF_SETCOLOR|colors[j];
      if(j==0) DialogItems[shift+j].Flags|=DIF_GROUP;
    }
  for(int i=0;i<2;i++)
  {
    DialogItems[color_set+i].Type=DI_BUTTON;
    DialogItems[color_set+i].Y1=12;
    DialogItems[color_set+i].Flags=DIF_CENTERGROUP;
    DialogItems[color_set+i].Flags|=(!i)?DIF_DEFAULTBUTTON:0;
    INIT_DLG_DATA(DialogItems[color_set+i],GetMsg(MColorSet+i));
  }
  for(int i=0;i<3;i++)
  {
    DialogItems[color_test1+i].Type=DI_TEXT;
    DialogItems[color_test1+i].X1=5;
    DialogItems[color_test1+i].Y1=i+8;
    DialogItems[color_test1+i].Flags=DIF_SETCOLOR;
    INIT_DLG_DATA(DialogItems[color_test1+i],GetMsg(MColorTest1+i));
  }
  DialogItems[color_border].Type=DI_DOUBLEBOX;
  DialogItems[color_border].X1=3;
  DialogItems[color_border].Y1=1;
  DialogItems[color_border].X2=35;
  DialogItems[color_border].Y2=13;
  INIT_DLG_DATA(DialogItems[color_border],GetMsg(MColorMain));
  DialogItems[color_sep].Type=DI_TEXT;
  DialogItems[color_sep].X1=-1;
  DialogItems[color_sep].Y1=11;
  DialogItems[color_sep].Flags=DIF_SEPARATOR;

  DialogItems[color_translate[*fg]+1].Flags|=DIF_FOCUS;
  DialogItems[color_translate[*fg]+1].Selected=true;
  DialogItems[color_translate[*bg]+18].Selected=true;
  CFarDialog dialog;
  int DlgCode=dialog.Execute(MainGuid,ColorGuid,-1,-1,39,15,NULL,DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,ColorDialogProc,0);
  if(DlgCode==color_set)
  {
    for(int i=1;i<17;i++)
      if(dialog.Check(i))
      {
        *fg=(dialog.Flags(i)&0xF0)>>4;
        break;
      }
    for(int i=18;i<34;i++)
      if(dialog.Check(i))
      {
        *bg=(dialog.Flags(i)&0xF0)>>4;
        break;
      }
    result=true;
  }
  return result;
}
