/*
   Copyright (C) Vadim Yegorov
*/
#include "Visualizer.hpp"

#ifndef UNICODE
#define GetFlags(i,hDlg) DialogItems[i].Flags
#else
static inline DWORD GetFlags(int i,HANDLE hDlg)
{
  DWORD flags=0;
  FarDialogItem* DialogItem=(FarDialogItem*)(LONG_PTR)Info.SendDlgMessage(hDlg,DM_GETDLGITEM,i,0);
  if(DialogItem)
  {
    flags=DialogItem->Flags;
    Info.SendDlgMessage(hDlg,DM_FREEDLGITEM,0,(LONG_PTR)DialogItem);
  }
  return flags;
}
#endif

long WINAPI ColorDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  int color=0;
#ifdef UNICODE
  FarDialogItem* DialogItem;
#else
  FarDialogItem DialogItem_; FarDialogItem* DialogItem=&DialogItem_;
#endif
  switch(Msg)
  {
    case DN_INITDIALOG:
    case DN_BTNCLICK:
      for(int i=1;i<17;i++)
      {
#ifdef UNICODE
        DialogItem=(FarDialogItem*)(LONG_PTR)Info.SendDlgMessage(hDlg,DM_GETDLGITEM,i,0);
#else
        Info.SendDlgMessage(hDlg,DM_GETDLGITEM,i,(LONG_PTR)DialogItem);
#endif
        if(DialogItem)
        {
          if(DialogItem->Selected)
          {
            color|=(DialogItem->Flags&0xF0)>>4;
            break;
          }
#ifdef UNICODE
          Info.SendDlgMessage(hDlg,DM_FREEDLGITEM,0,(LONG_PTR)DialogItem);
#endif
        }
      }
      for(int i=18;i<34;i++)
      {
#ifdef UNICODE
        DialogItem=(FarDialogItem*)(LONG_PTR)Info.SendDlgMessage(hDlg,DM_GETDLGITEM,i,0);
#else
        Info.SendDlgMessage(hDlg,DM_GETDLGITEM,i,(LONG_PTR)DialogItem);
#endif
        if(DialogItem)
        {
          if(DialogItem->Selected)
          {
            color|=(DialogItem->Flags&0xF0);
            break;
          }
#ifdef UNICODE
          Info.SendDlgMessage(hDlg,DM_FREEDLGITEM,0,(LONG_PTR)DialogItem);
#endif
        }
      }
      for(int i=36;i<39;i++)
      {
#ifdef UNICODE
        DialogItem=(FarDialogItem*)(LONG_PTR)Info.SendDlgMessage(hDlg,DM_GETDLGITEM,i,0);
#else
        Info.SendDlgMessage(hDlg,DM_GETDLGITEM,i,(LONG_PTR)DialogItem);
#endif
        if(DialogItem)
        {
          DialogItem->Flags=(DialogItem->Flags&0xffffff00)|color;
          Info.SendDlgMessage(hDlg,DM_SETDLGITEM,i,(LONG_PTR)DialogItem);
#ifdef UNICODE
          Info.SendDlgMessage(hDlg,DM_FREEDLGITEM,0,(LONG_PTR)DialogItem);
#endif
        }
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
  01   ษออออออออออออ Color ออออออออออออป   01
  02   บ ฺ Foreground ฟ ฺ Background ฟ บ   02
  03   บ ณ            ณ ณ          * ณ บ   03
  04   บ ณ            ณ ณ            ณ บ   04
  05   บ ณ            ณ ณ            ณ บ   05
  06   บ ณ          * ณ ณ            ณ บ   06
  07   บ ภฤฤฤฤฤฤฤฤฤฤฤฤู ภฤฤฤฤฤฤฤฤฤฤฤฤู บ   07
  08   บ Text Text Text Text Text Text บ   08
  09   บ Text Text Text Text Text Text บ   09
  10   บ Text Text Text Text Text Text บ   10
  11   วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ   11
  12   บ      [ Set ]  [ Cancel ]      บ   12
  13   ศอออออออออออออออออออออออออออออออผ   13
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

#ifndef UNICODE
  int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,39,15,NULL,DialogItems,sizeofa(DialogItems),0,0,ColorDialogProc,0);
#else
  HANDLE hDlg = Info.DialogInit(Info.ModuleNumber,-1,-1,39,15,NULL,DialogItems,sizeofa(DialogItems),0,0,ColorDialogProc,0);
  if (hDlg == INVALID_HANDLE_VALUE)
    return false;

  int DlgCode = Info.DialogRun(hDlg);
#endif

  if(DlgCode==34)
  {
    for(int i=1;i<17;i++)
      if(GetCheck(i))
      {
        *fg=(GetFlags(i,hDlg)&0xF0)>>4;
        break;
      }
    for(int i=18;i<34;i++)
      if(GetCheck(i))
      {
        *bg=(GetFlags(i,hDlg)&0xF0)>>4;
        break;
      }

#ifdef UNICODE
      Info.DialogFree(hDlg);
#endif

    return true;
  }

#ifdef UNICODE
      Info.DialogFree(hDlg);
#endif

  return false;
}
