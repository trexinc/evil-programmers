/*
   Copyright (C) Vadim Yegorov
*/
#include "Visualizer.hpp"

static long WINAPI ColorDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
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
  /* 0*/  {DI_SINGLEBOX,5,2,18,7,0,MColorForeground},
  /* 1*/  {DI_RADIOBUTTON, 6,3,0,0,DIF_GROUP|DIF_MOVESELECT|DIF_SETCOLOR|0x07,-1},
  /* 2*/  {DI_RADIOBUTTON, 6,4,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x40,-1},
  /* 3*/  {DI_RADIOBUTTON, 6,5,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x87,-1},
  /* 4*/  {DI_RADIOBUTTON, 6,6,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xc0,-1},
  /* 5*/  {DI_RADIOBUTTON, 9,3,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x17,-1},
  /* 6*/  {DI_RADIOBUTTON, 9,4,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x50,-1},
  /* 7*/  {DI_RADIOBUTTON, 9,5,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x90,-1},
  /* 8*/  {DI_RADIOBUTTON, 9,6,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xd0,-1},
  /* 9*/  {DI_RADIOBUTTON,12,3,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x20,-1},
  /*10*/  {DI_RADIOBUTTON,12,4,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x60,-1},
  /*11*/  {DI_RADIOBUTTON,12,5,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xa0,-1},
  /*12*/  {DI_RADIOBUTTON,12,6,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xe0,-1},
  /*13*/  {DI_RADIOBUTTON,15,3,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x30,-1},
  /*14*/  {DI_RADIOBUTTON,15,4,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x70,-1},
  /*15*/  {DI_RADIOBUTTON,15,5,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xb0,-1},
  /*16*/  {DI_RADIOBUTTON,15,6,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xf0,-1},
  /*17*/  {DI_SINGLEBOX,20,2,33,7,0,MColorBackground},
  /*18*/  {DI_RADIOBUTTON,21,3,0,0,DIF_GROUP|DIF_MOVESELECT|DIF_SETCOLOR|0x07,-1},
  /*19*/  {DI_RADIOBUTTON,21,4,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x40,-1},
  /*20*/  {DI_RADIOBUTTON,21,5,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x87,-1},
  /*21*/  {DI_RADIOBUTTON,21,6,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xc0,-1},
  /*22*/  {DI_RADIOBUTTON,24,3,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x17,-1},
  /*23*/  {DI_RADIOBUTTON,24,4,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x50,-1},
  /*24*/  {DI_RADIOBUTTON,24,5,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x90,-1},
  /*25*/  {DI_RADIOBUTTON,24,6,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xd0,-1},
  /*26*/  {DI_RADIOBUTTON,27,3,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x20,-1},
  /*27*/  {DI_RADIOBUTTON,27,4,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x60,-1},
  /*28*/  {DI_RADIOBUTTON,27,5,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xa0,-1},
  /*29*/  {DI_RADIOBUTTON,27,6,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xe0,-1},
  /*30*/  {DI_RADIOBUTTON,30,3,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x30,-1},
  /*31*/  {DI_RADIOBUTTON,30,4,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0x70,-1},
  /*32*/  {DI_RADIOBUTTON,30,5,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xb0,-1},
  /*33*/  {DI_RADIOBUTTON,30,6,0,0,         DIF_MOVESELECT|DIF_SETCOLOR|0xf0,-1},
  /*34*/  {DI_BUTTON,0,12,0,0,DIF_CENTERGROUP,MOk},
  /*35*/  {DI_BUTTON,0,12,0,0,DIF_CENTERGROUP,MCancel},
  /*36*/  {DI_TEXT,5,8,0,0,DIF_SETCOLOR,MColorTest1},
  /*37*/  {DI_TEXT,5,9,0,0,DIF_SETCOLOR,MColorTest2},
  /*38*/  {DI_TEXT,5,10,0,0,DIF_SETCOLOR,MColorTest3},
  /*39*/  {DI_DOUBLEBOX,3,1,35,13,0,MColorTitle},
  /*40*/  {DI_TEXT,-1,11,0,0,DIF_SEPARATOR,-1},
  };
  const unsigned char color_translate[]={0,4,8,0xc,1,5,9,0xd,2,6,0xa,0xe,3,7,0xb,0xf};
  struct FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));
  DialogItems[34].DefaultButton=1;
  DialogItems[color_translate[*fg]+1].Focus=DialogItems[color_translate[*fg]+1].Selected=true;
  DialogItems[color_translate[*bg]+18].Selected=true;
  int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,39,15,NULL,DialogItems,sizeofa(DialogItems),0,0,ColorDialogProc,0);
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
