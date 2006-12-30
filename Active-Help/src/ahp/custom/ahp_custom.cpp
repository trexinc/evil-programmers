/*
    Custom sub-plugin for Active-Help plugin for FAR Manager
    Copyright (C) 2005 Alex Yaroslavsky

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
#include "../../ahp.hpp"
#include "registry.hpp"
#include "crt.hpp"
#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

#if defined(__GNUC__)
extern "C"
{
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
};

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}
#endif

static struct PluginStartupInfo Info;
static FARSTANDARDFUNCTIONS FSF;
static struct AHPluginStartupInfo AHInfo;
char PluginRootKey[80];

static int Modality, Visuality;
enum
{
  MTitle,
  MShowIn,
  MModal,
  MNonModal,
  MEditor,
  MViewer,
  MErrWritingTemp,
  MOk,
  MCancel,
};

static int ShowCustom(const char *Command, const char *Keyword, char *Error);
static void Config(void);

static char *GetMsg(int MsgNum, char *Str)
{
  AHInfo.GetMsg(AHInfo.MessageName,MsgNum,Str);
  return Str;
}

struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1, Y1, X2, Y2;
  unsigned int Flags;
  signed char Data;
};

static void InitDialogItems(const struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber)
{
  int i;
  struct FarDialogItem *PItem = Item;
  const struct InitDialogItem *PInit = Init;

  for (i=0; i<ItemsNumber; i++, PItem++, PInit++)
  {
    PItem->Type = PInit->Type;
    PItem->X1 = PInit->X1;
    PItem->Y1 = PInit->Y1;
    PItem->X2 = PInit->X2;
    PItem->Y2 = PInit->Y2;
    PItem->Focus = 0;
    PItem->Selected = 0;
    PItem->Flags = PInit->Flags;
    PItem->DefaultButton = 0;
    if (PInit->Data==-1)
      *PItem->Data = 0;
    else
      GetMsg(PInit->Data,PItem->Data);
  }
}

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct AHPluginStartupInfo *AHInfo)
{
  Info=*FarInfo;
  FSF=*FarInfo->FSF;
  ::AHInfo=*AHInfo;
  FSF.sprintf(PluginRootKey,"%s\\%s",AHInfo->RootKey,"Custom");
  GetRegKey("Modality",&Modality,0);
  Modality %= 2;
  GetRegKey("Visuality",&Visuality,0);
  Visuality %= 2;
  return 0;
}

void WINAPI _export Exit(void)
{
}

int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData)
{
  switch(Msg)
  {
    case AHMSG_GETINFO:
    {
      GetInfoOutData *data=(GetInfoOutData *)OutData;
      data->TypesNumber=1;
      static struct TypeInfo TypesInfo[1];
      memset(TypesInfo,0,sizeof(TypesInfo));
      TypesInfo[0].StructSize=sizeof(TypesInfo[0]);
      lstrcpy(TypesInfo[0].TypeString,"Custom");
      *TypesInfo[0].Mask=0;
      TypesInfo[0].HFlags=F_INPUTKEYWORD;
      data->TypesInfo=TypesInfo;
      GetMsg(MTitle,data->ConfigString);
      data->Flags=AHMSG_CONFIG;
      return TRUE;
    }

    case AHMSG_SHOWHELP:
    {
      HelpInData *data = (HelpInData *)InData;
      HelpOutData *odata = (HelpOutData *)OutData;
      return ShowCustom(data->FileName,data->Keyword,odata->Error);
    }

    case AHMSG_CONFIG:
    {
      Config();
      return TRUE;
    }
  }
  return FALSE;
}

static void MakeCommandLine(char *CommandLine, const char *Command, const char *Keyword, const char *Output)
{
  char *ptr=CommandLine;
  *(ptr++)=' ';
  const char *srcp=Command;
  while (*srcp)
  {
    if (*srcp=='%')
    {
      switch (*(srcp+1))
      {
        case 'o':
        case 'O':
        {
          srcp+=2;
          lstrcpy(ptr,Output);
          ptr+=lstrlen(ptr);
          break;
        }
        case 'k':
        case 'K':
        {
          srcp+=2;
          lstrcpy(ptr,Keyword);
          ptr+=lstrlen(ptr);
          break;
        }
        case '%':
        {
          srcp+=2;
          *(ptr++)='%';
          break;
        }
        default:
          *(ptr++)=*(srcp++);
      }
    }
    else
      *(ptr++)=*(srcp++);
  }
  *ptr=0;
}

int ExternalFilter(const char *Command, const char *Keyword, const char *Output, char *Error)
{
  char CommandLine[2048];
  lstrcpy(CommandLine,"%COMSPEC% /C ");
  FSF.ExpandEnvironmentStr(CommandLine,CommandLine,sizeof(CommandLine));
  MakeCommandLine(CommandLine+lstrlen(CommandLine),Command,Keyword,Output);
  STARTUPINFO si={0};
  si.cb=sizeof(si);
  PROCESS_INFORMATION pi;
  int ret = CreateProcess(NULL,CommandLine,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);
  if (!ret)
  {
    LPSTR MessageBuffer;
    DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_SYSTEM;
    DWORD dwBufferLength = FormatMessageA(dwFormatFlags,NULL,GetLastError(),MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPSTR)&MessageBuffer,0,NULL);
    lstrcpyn(Error,MessageBuffer,dwBufferLength>128?128:dwBufferLength);
    CharToOem(Error,Error);
  }
  else
  {
    WaitForSingleObject(pi.hProcess,INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }
  return ret;
}

static int ShowCustom(const char *Command, const char *Keyword, char *Error)
{
  if (!Keyword || !*Keyword)
    return 1;

  char TempFileName[MAX_PATH];
  if (!CreateDirectory(FSF.MkTemp(TempFileName,"ACTH"),NULL))
  {
    GetMsg(MErrWritingTemp,Error);
    return 0;
  }
  lstrcat(TempFileName,"\\custom.txt");

  int Modal=1;
  if (Info.AdvControl(Info.ModuleNumber,ACTL_GETWINDOWCOUNT,0)>1)
  {
    Modal=0;
  }
  else
  {
    struct WindowInfo wi;
    wi.Pos=-1;
    Info.AdvControl(Info.ModuleNumber,ACTL_GETWINDOWINFO,(void *)&wi);
    if (wi.Type==WTYPE_PANELS)
      Modal=0;
  }

  if (ExternalFilter(Command,Keyword,TempFileName,Error))
  {
    if (Visuality==0)
      Info.Editor(TempFileName,Keyword,0,0,-1,-1,(Modality==0&&!Modal?EF_NONMODAL:0)|EF_DELETEONCLOSE,0,1);
    else
      Info.Viewer(TempFileName,Keyword,0,0,-1,-1,(Modality==0&&!Modal?VF_NONMODAL:0)|VF_DELETEONCLOSE);
  }
  else
    return 0;

  return 1;
}

static void Config(void)
{
  static const struct InitDialogItem PreDialogItems[] =
  {
    {DI_DOUBLEBOX  ,3  ,1  ,41 ,6  ,0               ,MTitle},
    {DI_TEXT       ,5  ,2  ,0  ,0  ,0               ,MShowIn},
    {DI_COMBOBOX   ,5  ,3  ,20 ,0  ,DIF_DROPDOWNLIST,-1},
    {DI_COMBOBOX   ,23 ,3  ,38 ,0  ,DIF_DROPDOWNLIST,-1},
    {DI_TEXT       ,4  ,4  ,0  ,0  ,DIF_SEPARATOR   ,-1},
    {DI_BUTTON     ,0  ,5  ,0  ,0  ,DIF_CENTERGROUP ,MOk},
    {DI_BUTTON     ,0  ,5  ,0  ,0  ,DIF_CENTERGROUP ,MCancel},
  };
  struct FarDialogItem DialogItems[sizeofa(PreDialogItems)];

  InitDialogItems(PreDialogItems,DialogItems,sizeofa(PreDialogItems));
  DialogItems[2].Focus = TRUE;
  struct FarListItem li1[2];
  memset(li1,0,sizeof(li1));
  struct FarList fl1 = {2, li1};
  li1[Modality].Flags=LIF_SELECTED;
  GetMsg(MNonModal,li1[0].Text);
  GetMsg(MModal,li1[1].Text);
  DialogItems[2].ListItems = &fl1;
  struct FarListItem li2[2];
  memset(li2,0,sizeof(li2));
  struct FarList fl2 = {2, li2};
  li2[Visuality].Flags=LIF_SELECTED;
  GetMsg(MEditor,li2[0].Text);
  GetMsg(MViewer,li2[1].Text);
  DialogItems[3].ListItems = &fl2;
  DialogItems[5].DefaultButton = 1;

  if (Info.Dialog(Info.ModuleNumber,-1,-1,45,8,NULL,(struct FarDialogItem *)&DialogItems,sizeofa(DialogItems)) != 5)
    return;

  Modality = DialogItems[2].ListPos;
  Visuality = DialogItems[3].ListPos;

  SetRegKey("Modality",Modality);
  SetRegKey("Visuality",Visuality);
}
