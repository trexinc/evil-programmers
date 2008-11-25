/*
    ab_selcolor.cpp
    Copyright (C) 2000-2008 zg

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

#include "../../plugin.hpp"
#include "ab_main.h"

long WINAPI ColorDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  int color=0; FarDialogItem DialogItem;
  switch(Msg)
  {
    case DN_INITDIALOG:
    case DN_BTNCLICK:
      for(int i=1;i<17;i++)
      {
        Info.SendDlgMessage(hDlg,DM_GETDLGITEM,i,(long)&DialogItem);
        if(DialogItem.Selected)
        {
          color|=(DialogItem.Flags&0xF0)>>4;
          break;
        }
      }
      for(int i=18;i<34;i++)
      {
        Info.SendDlgMessage(hDlg,DM_GETDLGITEM,i,(long)&DialogItem);
        if(DialogItem.Selected)
        {
          color|=(DialogItem.Flags&0xF0);
          break;
        }
      }
      for(int i=36;i<39;i++)
      {
        Info.SendDlgMessage(hDlg,DM_GETDLGITEM,i,(long)&DialogItem);
        DialogItem.Flags=(DialogItem.Flags&0xffffff00)|color;
        Info.SendDlgMessage(hDlg,DM_SETDLGITEM,i,(long)&DialogItem);
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool SelectColor(int *fg,int *bg)
{
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
  static struct InitDialogItem InitItems[]={
  /* 0*/  {DI_SINGLEBOX,5,2,18,7,0,0,0,0,(char *)mColorForeground},
  /* 1*/  {DI_RADIOBUTTON, 6,3,0,0,0,0,DIF_GROUP|DIF_MOVESELECT|DIF_SETCOLOR|0x07,0,""},
  /* 2*/  {DI_RADIOBUTTON, 6,4,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x40,0,""},
  /* 3*/  {DI_RADIOBUTTON, 6,5,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x87,0,""},
  /* 4*/  {DI_RADIOBUTTON, 6,6,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xc0,0,""},
  /* 5*/  {DI_RADIOBUTTON, 9,3,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x17,0,""},
  /* 6*/  {DI_RADIOBUTTON, 9,4,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x50,0,""},
  /* 7*/  {DI_RADIOBUTTON, 9,5,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x90,0,""},
  /* 8*/  {DI_RADIOBUTTON, 9,6,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xd0,0,""},
  /* 9*/  {DI_RADIOBUTTON,12,3,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x20,0,""},
  /*10*/  {DI_RADIOBUTTON,12,4,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x60,0,""},
  /*11*/  {DI_RADIOBUTTON,12,5,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xa0,0,""},
  /*12*/  {DI_RADIOBUTTON,12,6,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xe0,0,""},
  /*13*/  {DI_RADIOBUTTON,15,3,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x30,0,""},
  /*14*/  {DI_RADIOBUTTON,15,4,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x70,0,""},
  /*15*/  {DI_RADIOBUTTON,15,5,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xb0,0,""},
  /*16*/  {DI_RADIOBUTTON,15,6,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xf0,0,""},
  /*17*/  {DI_SINGLEBOX,20,2,33,7,0,0,0,0,(char *)mColorBackground},
  /*18*/  {DI_RADIOBUTTON,21,3,0,0,0,0,DIF_GROUP|DIF_MOVESELECT|DIF_SETCOLOR|0x07,0,""},
  /*19*/  {DI_RADIOBUTTON,21,4,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x40,0,""},
  /*20*/  {DI_RADIOBUTTON,21,5,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x87,0,""},
  /*21*/  {DI_RADIOBUTTON,21,6,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xc0,0,""},
  /*22*/  {DI_RADIOBUTTON,24,3,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x17,0,""},
  /*23*/  {DI_RADIOBUTTON,24,4,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x50,0,""},
  /*24*/  {DI_RADIOBUTTON,24,5,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x90,0,""},
  /*25*/  {DI_RADIOBUTTON,24,6,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xd0,0,""},
  /*26*/  {DI_RADIOBUTTON,27,3,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x20,0,""},
  /*27*/  {DI_RADIOBUTTON,27,4,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x60,0,""},
  /*28*/  {DI_RADIOBUTTON,27,5,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xa0,0,""},
  /*29*/  {DI_RADIOBUTTON,27,6,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xe0,0,""},
  /*30*/  {DI_RADIOBUTTON,30,3,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x30,0,""},
  /*31*/  {DI_RADIOBUTTON,30,4,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x70,0,""},
  /*32*/  {DI_RADIOBUTTON,30,5,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xb0,0,""},
  /*33*/  {DI_RADIOBUTTON,30,6,0,0,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xf0,0,""},
  /*34*/  {DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,1,(char *)mColorSet},
  /*35*/  {DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,0,(char *)mColorCancel},
  /*36*/  {DI_TEXT,5,8,0,0,0,0,DIF_SETCOLOR,0,(char *)mColorTest1},
  /*37*/  {DI_TEXT,5,9,0,0,0,0,DIF_SETCOLOR,0,(char *)mColorTest2},
  /*38*/  {DI_TEXT,5,10,0,0,0,0,DIF_SETCOLOR,0,(char *)mColorTest3},
  /*39*/  {DI_DOUBLEBOX,3,1,35,13,0,0,0,0,(char *)mColorMain},
  /*40*/  {DI_TEXT,-1,11,0,0,0,0,DIF_SEPARATOR,0,""},
  };
  const unsigned char color_translate[]={0,4,8,0xc,1,5,9,0xd,2,6,0xa,0xe,3,7,0xb,0xf};
  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  DialogItems[color_translate[*fg]+1].Focus=DialogItems[color_translate[*fg]+1].Selected=true;
  DialogItems[color_translate[*bg]+18].Selected=true;
  int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,39,15,"Color",DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,ColorDialogProc,0);
  if(DlgCode==34)
  {
    for(int i=1;i<17;i++)
      if(DialogItems[i].Selected)
      {
        *fg=(DialogItems[i].Flags&0xF0)>>4;
        break;
      }
    for(int i=18;i<34;i++)
      if(DialogItems[i].Selected)
      {
        *bg=(DialogItems[i].Flags&0xF0)>>4;
        break;
      }
    return true;
  }
  return false;
}
