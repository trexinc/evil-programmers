/*
    Filter sub-plugin for FARMail
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
#include "filter.hpp"
#include "registry.hpp"

#if defined(__GNUC__)

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

void *memcpy(void * dst, const void * src, size_t count)
{
  void * ret = dst;

  while (count--)
  {
    *(char *)dst = *(char *)src;
    dst = (char *)dst + 1;
    src = (char *)src + 1;
  }
  return(ret);
}
#endif

struct PluginStartupInfo FarInfo;
FARSTANDARDFUNCTIONS FSF;
struct MailPluginStartupInfo FarMailInfo;
char PluginRootKey[80];
char DefFiltersDir[MAX_PATH];
static const char DEFFILTERSDIR[] = "DefFiltersDir";
const char NULLSTR[]="";

char *GetMsg(int MsgNum,char *Str)
{
  FarMailInfo.GetMsg(FarMailInfo.MessageName,MsgNum,Str);
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

void ReadRegistry(void)
{
  HKEY hRoot = HKEY_CURRENT_USER;
  char path[MAX_PATH];
  lstrcpy(path,FarMailInfo.ModuleName);
  *(FSF.PointToName(path)) = 0;
  lstrcat(path,"FILTERS\\");
  GetRegKey2(hRoot, PluginRootKey, NULLSTR, DEFFILTERSDIR,   DefFiltersDir , path , MAX_PATH );
  if (!*DefFiltersDir)
    lstrcpy(DefFiltersDir,path);
  else
  {
    FSF.Unquote(DefFiltersDir);
    FSF.AddEndSlash(DefFiltersDir);
  }
}

void Config(void)
{
  struct InitDialogItem InitItems[] =
  {
    // type, x1, y1, x2, y2, focus, selected, flags, default, data
    { DI_DOUBLEBOX,3, 1,65,6, 0,0,0,0, (char*)mConfig_Title },

    { DI_TEXT,     5, 2, 0,0, 0,0,0,0, (char*)mConfig_DefFiltersDir },
    { DI_EDIT,     5, 3,63,0, 1,0,0,0, NULLSTR },

    {DI_TEXT,3,4,0,0,0,0,DIF_SEPARATOR,0,NULLSTR},

    { DI_BUTTON,  0, 5, 0,0 ,0,0,DIF_CENTERGROUP,1, (char *)mOk },
    { DI_BUTTON,  0, 5, 0,0 ,0,0,DIF_CENTERGROUP,0, (char *)mCancel }
  };
  enum
  {
    C2_FDIR = 2,
    C_TXT,
    C_OK,
    C_CANCEL
  };
  struct FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));

  ReadRegistry();

  lstrcpy( DialogItems[C2_FDIR].Data , DefFiltersDir );

  if ( FarInfo.DialogEx(FarInfo.ModuleNumber,-1,-1,69,8,"Config",DialogItems,sizeofa(DialogItems),0,0,FarMailInfo.ShowHelpDlgProc,(long)FarMailInfo.ModuleName) == C_OK )
  {
    lstrcpyn( DefFiltersDir, DialogItems[C2_FDIR].Data, MAX_PATH);
    if (!*DefFiltersDir)
    {
      char path[MAX_PATH];
      lstrcpy(path,FarMailInfo.ModuleName);
      *(FSF.PointToName(path)) = 0;
      lstrcat(path,"FILTERS\\");
      lstrcpy(DefFiltersDir,path);
    }
    else
    {
      FSF.Unquote(DefFiltersDir);
      FSF.AddEndSlash(DefFiltersDir);
    }

    SetRegKey2( HKEY_CURRENT_USER, PluginRootKey, NULLSTR, DEFFILTERSDIR, DefFiltersDir);
  }
}

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct MailPluginStartupInfo *FarMailInfo)
{
  ::FarInfo=*FarInfo;
  FSF=*FarInfo->FSF;
  ::FarMailInfo=*FarMailInfo;
  FSF.sprintf(PluginRootKey,"%s\\Filter",::FarMailInfo.RootKey);
  ReadRegistry();
  return 0;
}

void WINAPI _export Exit(void)
{

}

int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData)
{
  switch(Msg)
  {
    case FMMSG_GETINFO:
      {
        GetInfoOutData *data=(GetInfoOutData *)OutData;
        GetMsg(mName,data->MenuString);
        lstrcpy(data->HotkeyID,"generic_filter");
        data->Flags=FMMSG_FILTER|FMMSG_CONFIG;
      }
      return TRUE;
    case FMMSG_FILTER:
      RunFilter(static_cast<MsgPanelInData *>(InData)->hPlugin,static_cast<MsgPanelInData *>(InData)->index);
      return TRUE;
    case FMMSG_CONFIG:
      Config();
      return TRUE;
  }
  return FALSE;
}
