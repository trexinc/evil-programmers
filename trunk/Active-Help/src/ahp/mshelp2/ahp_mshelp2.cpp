/*
    MS-Help2 sub-plugin for Active-Help plugin for FAR Manager
    Copyright (C) 2003 Alex Yaroslavsky

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
#include "dexplorer.hpp"
#include "crt.hpp"
#include "registry.hpp"
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
static char PathToViewer[512];
static char OpenContents[512];
static char KeywordSearch[512];
static Help *pHelp=0;
static bool OleInitialized=false;

enum
{
  MTitle,
  MPathToViewer,
  MOpenContents,
  MKeywordSearch,
  MOk,
  MCancel,
};

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

static void MakeCommandLine(char *CommandLine, const char *Path, const char *Command, const char *FileName, const char *Keyword)
{
  lstrcpy(CommandLine,Path);
  FSF.QuoteSpaceOnly(CommandLine);
  char *ptr=CommandLine+lstrlen(CommandLine);
  *(ptr++)=' ';
  const char *srcp=Command;
  while (*srcp)
  {
    if (*srcp=='%')
    {
      switch (*(srcp+1))
      {
        case 'f':
        case 'F':
        {
          srcp+=2;
          lstrcpy(ptr,FileName);
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

int MSHelp2(const char *Keyword, const char *FileName, char *Error)
{
  char CommandLine[2048];
  if (Keyword)
    MakeCommandLine(CommandLine,PathToViewer,KeywordSearch,FileName,Keyword);
  else
    MakeCommandLine(CommandLine,PathToViewer,OpenContents,FileName,Keyword);
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
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }
  return ret;
}

int MSHelp2DE(const char *Keyword, const char *FileName, char *Error)
{
  HRESULT hr = S_OK;
  if (!OleInitialized)
  {
    OleInitialized=true;
    hr = CoInitialize(NULL);
  }
  if (!pHelp)
  {
    CLSID clsid; // Class ID
   hr = CLSIDFromProgID(L"DExplore.AppObj", &clsid);
   if (SUCCEEDED(hr))
      hr = CoCreateInstance(clsid,0,CLSCTX_LOCAL_SERVER,IID_Help,(void**)&pHelp);
  }
  if (!pHelp)
  {
    if (SUCCEEDED(hr)) hr=ERROR_CLASS_DOES_NOT_EXIST;
  }
  else
  {
    wchar_t *uCollection = (wchar_t*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(lstrlen(FileName)+1)*sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP,0,FileName,-1,uCollection,lstrlen(FileName)+1);
    BSTR tmp=SysAllocString(uCollection);
    pHelp->SetCollection(tmp,0);
    SysFreeString(tmp);
    if (uCollection)
      HeapFree(GetProcessHeap(),0,uCollection);
    if (*Keyword)
    {
      wchar_t *uKeyword = (wchar_t*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(lstrlen(Keyword)+1)*sizeof(wchar_t));
      MultiByteToWideChar(CP_ACP,0,Keyword,-1,uKeyword,lstrlen(Keyword)+1);
      BSTR tmp=SysAllocString(uKeyword);
      HRESULT hr1=pHelp->Index();
      HRESULT hr2=pHelp->DisplayTopicFromKeyword(tmp);
      HRESULT hr3=pHelp->DisplayTopicFromF1Keyword(tmp);
      SysFreeString(tmp);
      if (uKeyword)
        HeapFree(GetProcessHeap(),0,uKeyword);
      if (FAILED(hr1) && FAILED(hr2) && FAILED(hr3))
        hr = hr3;
      else
        hr = S_OK;
    }
    else
    {
      hr = pHelp->Contents();
    }
  }
  if (FAILED(hr) && Error)
  {
    LPSTR MessageBuffer;
    DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_SYSTEM;
    DWORD dwBufferLength = FormatMessageA(dwFormatFlags,NULL,hr,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPSTR)&MessageBuffer,0,NULL);
    lstrcpyn(Error,MessageBuffer,dwBufferLength>128?128:dwBufferLength);
    CharToOem(Error,Error);
    char *pszR = strchr(Error,'\r');
    while (pszR)
    {
      *pszR = ' '; pszR = strchr(Error,'\r');
    }
  }
  return SUCCEEDED(hr);
}

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct AHPluginStartupInfo *AHInfo)
{
  Info=*FarInfo;
  FSF=*FarInfo->FSF;
  ::AHInfo=*AHInfo;
  FSF.sprintf(PluginRootKey,"%s\\%s",AHInfo->RootKey,"MS-Help2");
  GetRegKey("PathToViewer",PathToViewer,"H2Viewer.exe",sizeof(PathToViewer));
  GetRegKey("OpenContents",OpenContents,"/appid \"%f\" /helpcol \"%f\"",sizeof(OpenContents));
  GetRegKey("KeywordSearch",KeywordSearch,"/appid \"%f\" /helpcol \"%f\" /index \"%k\"",sizeof(KeywordSearch));
  return 0;
}

void WINAPI _export Exit(void)
{
  if (pHelp)
    pHelp->Release();
  if (OleInitialized)
    CoUninitialize();
}

int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData)
{
  switch(Msg)
  {
    case AHMSG_GETINFO:
    {
      GetInfoOutData *data=(GetInfoOutData *)OutData;
      data->TypesNumber=2;
      static struct TypeInfo TypesInfo[2];
      memset(TypesInfo,0,sizeof(TypesInfo));
      for (int i=0; i<2; i++)
      {
        TypesInfo[i].StructSize=sizeof(TypesInfo[0]);
        lstrcpy(TypesInfo[i].Encoding,"WIN");
        TypesInfo[i].HFlags=F_NOSKIPPATH;
      }
      lstrcpy(TypesInfo[0].TypeString,"MS-Help2");
      lstrcpy(TypesInfo[0].Mask,"*.hxs,ms-help://*");
      lstrcpy(TypesInfo[1].TypeString,"MS-Help2DE");
      lstrcpy(TypesInfo[1].Mask,"ms-help://*");
      data->TypesInfo=TypesInfo;
      GetMsg(MTitle,data->ConfigString);
      data->Flags=AHMSG_CONFIG;
    }
    return TRUE;

    case AHMSG_SHOWHELP:
    {
      HelpInData *data = (HelpInData *)InData;
      HelpOutData *odata = (HelpOutData *)OutData;

      int ret;
      if (data->TypeNumber == 0)
        ret = MSHelp2(data->Keyword,data->FileName,odata->Error);
      else
        ret = MSHelp2DE(data->Keyword,data->FileName,odata->Error);
      return ret;
    }

    case AHMSG_CONFIG:
    {
      Config();
      return TRUE;
    }
  }
  return FALSE;
}

static void Config(void)
{
  static const struct InitDialogItem PreDialogItems[] =
  {
    {DI_DOUBLEBOX  ,3  ,1  ,50 ,10 ,0               ,MTitle},
    {DI_TEXT       ,5  ,2  ,0  ,0  ,0               ,MPathToViewer},
    {DI_EDIT       ,5  ,3  ,47 ,0  ,DIF_EDITEXPAND  ,-1},
    {DI_TEXT       ,5  ,4  ,0  ,0  ,0               ,MOpenContents},
    {DI_EDIT       ,5  ,5  ,47 ,0  ,0               ,-1},
    {DI_TEXT       ,5  ,6  ,0  ,0  ,0               ,MKeywordSearch},
    {DI_EDIT       ,5  ,7  ,47 ,0  ,0               ,-1},
    {DI_TEXT       ,4  ,8  ,0  ,0  ,DIF_SEPARATOR   ,-1},
    {DI_BUTTON     ,0  ,9  ,0  ,0  ,DIF_CENTERGROUP ,MOk},
    {DI_BUTTON     ,0  ,9  ,0  ,0  ,DIF_CENTERGROUP ,MCancel},
  };
  struct FarDialogItem DialogItems[sizeofa(PreDialogItems)];

  InitDialogItems(PreDialogItems,DialogItems,sizeofa(PreDialogItems));
  DialogItems[2].Focus = TRUE;
  lstrcpy(DialogItems[2].Data,PathToViewer);
  lstrcpy(DialogItems[4].Data,OpenContents);
  lstrcpy(DialogItems[6].Data,KeywordSearch);
  DialogItems[8].DefaultButton = 1;

  if (Info.Dialog(Info.ModuleNumber,-1,-1,54,12,NULL,(struct FarDialogItem *)&DialogItems,sizeofa(DialogItems)) != 8)
    return;

  lstrcpy(PathToViewer,DialogItems[2].Data);
  lstrcpy(OpenContents,DialogItems[4].Data);
  lstrcpy(KeywordSearch,DialogItems[6].Data);

  SetRegKey("PathToViewer",PathToViewer);
  SetRegKey("OpenContents",OpenContents);
  SetRegKey("KeywordSearch",KeywordSearch);
}
