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
#ifndef __SCRIPTS_HPP__
#define __SCRIPTS_HPP__

#include "plugin.hpp"
#include "../FARMail/fmp.hpp"

#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

struct OPTIONS
{
  char DefScriptDir[MAX_PATH];
  char NewMessageScript[MAX_PATH];
  char ReplyScript[MAX_PATH];
  char EXT[20];
};

struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  unsigned int Selected;
  unsigned int Flags;
  unsigned char DefaultButton;
  const char *Data;
};

enum
{
  ERR_INTERNALERROR,
  ERR_DIVZERO,
  ERR_SYNTAX,
};

extern char PluginRootKey[80];
extern const char NULLSTR[];

extern PluginStartupInfo FInfo;
extern FARSTANDARDFUNCTIONS FSF;
extern MailPluginStartupInfo MInfo;
extern OPTIONS Opt;

extern int GetScriptInfo(char *fd, char *fn, int i);
extern void SetScriptInfo(char *fd, char *fn, int i);
extern void DelScriptInfo(int i);

extern char *GetMsg(int MsgNum,char *Str);
extern void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item, int ItemsNumber);
extern char *ExpandFilename(char *name);
extern void SayError(int type,const char *message,const char *extra_message=NULL);

extern void SetRegKey2(HKEY hRoot, const char *root, const char *Key,const char *ValueName,const char *ValueData,DWORD type=REG_SZ);
extern int GetRegKey2(HKEY hRoot,const char * root, const char *Key,const char *ValueName,char *ValueData,const char *Default,DWORD DataSize);
extern void DelRegValue2(HKEY hRoot, const char *root, const char *Key, const char * ValueName);

extern int run_script(char *name);

//RTL
int Random(int x);
void Randomize(void);

#endif
