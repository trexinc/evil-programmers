#include "common.hpp"

void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item, int ItemsNumber)
{
  for (int I=0;I<ItemsNumber;I++)
  {
    Item[I].Type=Init[I].Type;
    Item[I].X1=Init[I].X1;
    Item[I].Y1=Init[I].Y1;
    Item[I].X2=Init[I].X2;
    Item[I].Y2=Init[I].Y2;
    Item[I].Focus=Init[I].Focus;
    Item[I].Selected=Init[I].Selected;
    Item[I].Flags=Init[I].Flags;
    Item[I].DefaultButton=Init[I].DefaultButton;
    if ((unsigned int)Init[I].Data<2000)
      lstrcpy(Item[I].Data,GetMsg((unsigned int)Init[I].Data));
    else
      lstrcpy(Item[I].Data,Init[I].Data);
  }
}

const char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

void ErrorMessage(int Msg)
{
  const char *err[2];
  err[0] = GetMsg(MTitle);
  err[1] = GetMsg(Msg);
  Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_MB_OK,NULL,err,2,0);
}
