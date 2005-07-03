/*
    Scripts sub-plugin for FARMail
    Copyright (C) 2002-2004 FARMail Group
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
#include "scripts.hpp"
#include "language.hpp"

char *GetMsg(int MsgNum,char *Str)
{
  MInfo.GetMsg(MInfo.MessageName,MsgNum,Str);
  return Str;
}

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
      GetMsg((unsigned int)Init[I].Data,Item[I].Data);
    else
      lstrcpy(Item[I].Data,Init[I].Data);
  }
}

char *ExpandFilename(char *name)
{
  FSF.Unquote(name);
  if(FSF.PointToName(name)==name)
  {
    char fn[MAX_PATH];
    if((lstrlen(name)+lstrlen(Opt.DefScriptDir))<MAX_PATH)
    {
      FSF.sprintf(fn,"%s%s",Opt.DefScriptDir,name);
      lstrcpy(name,fn);
    }
  }
  return name;
}

void SayError(int type, const char *message,const char *extra_message)
{
  const char *err[4];
  char msg1[100];
  char msg2[100];
  char msg3[100];
  err[0]=GetMsg(MesError,msg1);
  err[1]=message;
  err[2]="";
  switch(type)
  {
    case ERR_INTERNALERROR:
      err[2]=GetMsg(SErr_internal,msg2);
      break;
    case ERR_DIVZERO:
      err[2]=GetMsg(SErr_Zero,msg2);
      break;
    case ERR_SYNTAX:
      if (extra_message)
        err[2]=extra_message;
      break;
  }
  err[3]=GetMsg(MesOk,msg3);
  FInfo.Message(FInfo.ModuleNumber,FMSG_WARNING,NULL,err,sizeofa(err),1);
}
