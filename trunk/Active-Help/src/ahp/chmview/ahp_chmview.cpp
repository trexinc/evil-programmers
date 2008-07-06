/*
    ChmView sub-plugin for Active-Help plugin for FAR Manager
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
#include "registry.hpp"
#include "chmlib.h"
#include "crt.hpp"
#include "memory.hpp"
#define sizeofa(array) (sizeof(array)/sizeof(array[0]))
#define PATH_MAX 512

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
FARSTANDARDFUNCTIONS FSF;
static struct AHPluginStartupInfo AHInfo;
char PluginRootKey[80];

extern "C"
{
  int chm_getfile(chmfile *c, const char *name, ulong *length, ubyte **outbuf);
  chmfile *chm_openfile(const char *fname);
  void chm_close(chmfile *l);
};

static int Modality, Visuality, RepeatMenu;
static char PathToFilter[512], FilterArguments[512];
enum
{
  MTitle,
  MGeneratingHash,
  MCommon,
  MShowIn,
  MModal,
  MNonModal,
  MEditor,
  MViewer,
  MRepeatMenu,
  MCHMViewFiltered,
  MPathToFilter,
  MFilterArguments,
  MErrOpeningCHM,
  MErrUnsupportedCHM,
  MErrOpeningTopic,
  MErrWritingTemp,
  MOk,
  MCancel,
};

static int ShowCHM(const char *FileName, const char *Keyword, const char *Encoding, char *Error, int UseExternalFilter);
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
  FSF.sprintf(PluginRootKey,"%s\\%s",AHInfo->RootKey,"CHMView");
  GetRegKey("Modality",&Modality,0);
  Modality %= 2;
  GetRegKey("Visuality",&Visuality,0);
  Visuality %= 2;
  GetRegKey("RepeatMenu",&RepeatMenu,0);
  RepeatMenu %= 2;
  GetRegKey("PathToFilter",PathToFilter,"zd2t",sizeof(PathToFilter));
  GetRegKey("FilterArguments",FilterArguments,"/ZNwcvh \"%i\" > \"%o\"",sizeof(FilterArguments));
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
      data->TypesNumber=2;
      static struct TypeInfo TypesInfo[2];
      memset(TypesInfo,0,sizeof(TypesInfo));
      for (int i=0; i<2; i++)
      {
        TypesInfo[i].StructSize=sizeof(TypesInfo[0]);
        lstrcpy(TypesInfo[i].Encoding,"WIN");
        lstrcpy(TypesInfo[i].Mask,"*.chm,*.its");
        TypesInfo[i].HFlags=F_INPUTKEYWORD;
      }
      lstrcpy(TypesInfo[0].TypeString,"CHMView");
      lstrcpy(TypesInfo[1].TypeString,"CHMViewFiltered");
      data->TypesInfo=TypesInfo;
      GetMsg(MTitle,data->ConfigString);
      data->Flags=AHMSG_CONFIG;
      return TRUE;
    }

    case AHMSG_SHOWHELP:
    {
      HelpInData *data = (HelpInData *)InData;
      if (!data->Keyword || !(*data->Keyword))
        return 1;
      char *Keyword = strdup(data->Keyword);
      if (!Keyword)
        return 0;
      AHInfo.Decode(Keyword,lstrlen(Keyword),data->Encoding);
      HelpOutData *odata = (HelpOutData *)OutData;
      int ret=ShowCHM(data->FileName,Keyword,data->Encoding,odata->Error,data->TypeNumber);
      free(Keyword);
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

static void MakeCommandLine(char *CommandLine, const char *Path, const char *Command, const char *Input, const char *Output)
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
        case 'o':
        case 'O':
        {
          srcp+=2;
          lstrcpy(ptr,Output);
          ptr+=lstrlen(ptr);
          break;
        }
        case 'i':
        case 'I':
        {
          srcp+=2;
          lstrcpy(ptr,Input);
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

int ExternalHTMLFilter(const char *Input, const char *Output)
{
  char CommandLine[2048];
  lstrcpy(CommandLine,"%COMSPEC% /C ");
  FSF.ExpandEnvironmentStr(CommandLine,CommandLine,sizeof(CommandLine));
  MakeCommandLine(CommandLine+lstrlen(CommandLine),PathToFilter,FilterArguments,Input,Output);
  STARTUPINFO si={0};
  si.cb=sizeof(si);
  PROCESS_INFORMATION pi;
  int ret = CreateProcess(NULL,CommandLine,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);
  if (ret)
  {
    WaitForSingleObject(pi.hProcess,INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }
  return ret;
}


static ulong make_dword(ubyte *b)
{
  return ((ulong)(b[0] + b[1]*0x100 + b[2]*0x10000 + b[3]*0x1000000));
}

static int ShowCHM(const char *FileName, const char *Keyword, const char *Encoding, char *Error, int UseExternalFilter)
{
  ulong l_topics, l_strings, l_urltbl, l_urlstr;
  ubyte *o_topics, *o_strings, *o_urltbl, *o_urlstr;
  char url[PATH_MAX*2];
  lstrcpy(url,FileName);
  lstrcpy(url+lstrlen(url)-3,"chi");
  chmfile *h = chm_openfile(url);
  if (!h)
    h = chm_openfile(FileName);
  if (!h)
  {
    GetMsg(MErrOpeningCHM,Error);
    return 0;
  }
  ulong i;
  i=chm_getfile(h, "/#TOPICS", &l_topics, &o_topics);
  i+=chm_getfile(h, "/#STRINGS", &l_strings, &o_strings);
  i+=chm_getfile(h, "/#URLTBL", &l_urltbl, &o_urltbl);
  i+=chm_getfile(h, "/#URLSTR", &l_urlstr, &o_urlstr);
  chm_close(h);
  if (i!=0)
  {
    GetMsg(MErrUnsupportedCHM,Error);
    if (o_topics) free(o_topics);
    if (o_urlstr) free(o_urlstr);
    if (o_strings) free(o_strings);
    if (o_urltbl) free(o_urltbl);
    return 0;
  }
  i = 12;
  //while (i<l_topics && make_dword(&o_topics[i])!=0)
    //i += 16;
  if (i<l_topics)
    i -= 8;

  int KeywordLen=lstrlen(Keyword);
  FarMenuItemEx *Menu=NULL;
  int ItemsCount=0;
  while (true)
  {
    bool found=false;
    char topic[512];
    lstrcpyn(topic,(const char *) &o_strings[make_dword(&o_topics[i])],512);
    AHInfo.Decode(topic,lstrlen(topic),Encoding);
    while (i<l_topics && FSF.LStrnicmp(topic,Keyword,KeywordLen))
    {
      i += 16;
      lstrcpyn(topic,(const char *) &o_strings[make_dword(&o_topics[i])],512);
      AHInfo.Decode(topic,lstrlen(topic),Encoding);
    }
    ulong x;
    if (i<l_topics && !FSF.LStrnicmp(topic,Keyword,KeywordLen))
    {
      found = true;
      x = make_dword(&o_urltbl[make_dword(&o_topics[i+4])+8]);
      while (x<l_urlstr && o_urlstr[x]==0)
        x++;
    }
    if (found)
    {
      Menu = (struct FarMenuItemEx *)realloc(Menu,sizeof(struct FarMenuItemEx)*(++ItemsCount));
      memset(Menu+ItemsCount-1,0,sizeof(struct FarMenuItemEx));
      lstrcpyn(Menu[ItemsCount-1].Text.Text,topic,128);
      Menu[ItemsCount-1].UserData=x;
    }
    else
      break;
    i+=16;
  }

  int ret=1;

  for (int Selected=0;;)
  {
    bool Show=false;
    if (ItemsCount)
    {
      if (ItemsCount>1)
      {
        char temp[128];
        int Prev = Selected;
        Selected = Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT|FMENU_USEEXT,GetMsg(MTitle,temp),NULL,NULL,NULL,NULL,(const struct FarMenuItem *)Menu,ItemsCount);
        if (Selected>=0)
        {
          if (Menu[Prev].Flags&MIF_SELECTED)
            Menu[Prev].Flags-=MIF_SELECTED;
          Menu[Selected].Flags=MIF_SELECTED;
        }
      }
      if (Selected>=0)
      {
        url[0]='/';
        lstrcpy(&url[1],(const char *) &o_urlstr[Menu[Selected].UserData]);
        char *p = strchr(url,'#');
        if (p)
          *p = 0;
        Show=true;
      }
    }

    if (Show)
    {
      char TempFileName[MAX_PATH];
      char TempFileName2[MAX_PATH];
      if (CreateDirectory(FSF.MkTemp(TempFileName,"ACTH"),NULL))
      {
        chmfile *c = chm_openfile(FileName);
        if (!c)
        {
          GetMsg(MErrOpeningCHM,Error);
          ret=0;
          break;
        }
        lstrcpy(TempFileName2,TempFileName);
        lstrcat(TempFileName2,"\\chmview.txt");
        lstrcat(TempFileName,"\\chmview.htm");
        ulong length;
        ubyte *outbuf;
        i=chm_getfile(c, url, &length, &outbuf);
        chm_close(c);
        if (i!=0)
        {
          GetMsg(MErrOpeningTopic,Error);
          ret=0;
          break;
        }
        //char *p;
        //while ((p = strchr(TempFileName,'\\')) != NULL)
          //*p = '/';
        HANDLE f = CreateFile(TempFileName, GENERIC_WRITE, FILE_SHARE_READ,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
        if (f!=INVALID_HANDLE_VALUE)
        {
          DWORD t;
          WriteFile(f,outbuf, length, &t, NULL);
          CloseHandle(f);
        }
        else
          i = 1;
        if (outbuf) free(outbuf);
        if (i!=0)
        {
          GetMsg(MErrWritingTemp,Error);
          ret=0;
          break;
        }
      }
      else
        break;
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
      char *File = TempFileName;
      if (UseExternalFilter && ExternalHTMLFilter(TempFileName,TempFileName2))
      {
        File = TempFileName2;
        DeleteFile(TempFileName);
      }
      if (Visuality==0)
        Info.Editor(File,Keyword,0,0,-1,-1,(Modality==0&&!Modal?EF_NONMODAL:0)|EF_DELETEONCLOSE,0,1);
      else
        Info.Viewer(File,Keyword,0,0,-1,-1,(Modality==0&&!Modal?VF_NONMODAL:0)|VF_DELETEONCLOSE);
    }

    if (!RepeatMenu || !Show || !Modality || ItemsCount==1)
      break;
  }

  if (Menu) free(Menu);
  if (o_topics) free(o_topics);
  if (o_urlstr) free(o_urlstr);
  if (o_strings) free(o_strings);
  if (o_urltbl) free(o_urltbl);

  return ret;
}

static void Config(void)
{
  static const struct InitDialogItem PreDialogItems[] =
  {
    {DI_DOUBLEBOX  ,3  ,1  ,41 ,14 ,0               ,MTitle},
    {DI_SINGLEBOX  ,4  ,2  ,40 ,6  ,DIF_LEFTTEXT    ,MCommon},
    {DI_TEXT       ,6  ,3  ,0  ,0  ,0               ,MShowIn},
    {DI_COMBOBOX   ,6  ,4  ,20 ,0  ,DIF_DROPDOWNLIST,-1},
    {DI_COMBOBOX   ,23 ,4  ,38 ,0  ,DIF_DROPDOWNLIST,-1},
    {DI_CHECKBOX   ,6  ,5  ,38 ,0  ,0               ,MRepeatMenu},
    {DI_SINGLEBOX  ,4  ,7  ,40 ,12 ,DIF_LEFTTEXT    ,MCHMViewFiltered},
    {DI_TEXT       ,6  ,8  ,0  ,0  ,0               ,MPathToFilter},
    {DI_EDIT       ,6  ,9  ,38 ,0  ,DIF_EDITEXPAND  ,-1},
    {DI_TEXT       ,6  ,10 ,0  ,0  ,0               ,MFilterArguments},
    {DI_EDIT       ,6  ,11 ,38 ,0  ,0               ,-1},
    {DI_BUTTON     ,0  ,13 ,0  ,0  ,DIF_CENTERGROUP ,MOk},
    {DI_BUTTON     ,0  ,13 ,0  ,0  ,DIF_CENTERGROUP ,MCancel},
  };
  struct FarDialogItem DialogItems[sizeofa(PreDialogItems)];

  InitDialogItems(PreDialogItems,DialogItems,sizeofa(PreDialogItems));
  DialogItems[3].Focus = TRUE;
  struct FarListItem li1[2];
  memset(li1,0,sizeof(li1));
  struct FarList fl1 = {2, li1};
  li1[Modality].Flags=LIF_SELECTED;
  GetMsg(MNonModal,li1[0].Text);
  GetMsg(MModal,li1[1].Text);
  DialogItems[3].ListItems = &fl1;
  struct FarListItem li2[2];
  memset(li2,0,sizeof(li2));
  struct FarList fl2 = {2, li2};
  li2[Visuality].Flags=LIF_SELECTED;
  GetMsg(MEditor,li2[0].Text);
  GetMsg(MViewer,li2[1].Text);
  DialogItems[4].ListItems = &fl2;
  DialogItems[5].Selected = RepeatMenu;
  lstrcpy(DialogItems[8].Data,PathToFilter);
  lstrcpy(DialogItems[10].Data,FilterArguments);
  DialogItems[11].DefaultButton = 1;

  if (Info.Dialog(Info.ModuleNumber,-1,-1,45,16,NULL,(struct FarDialogItem *)&DialogItems,sizeofa(DialogItems)) != 11)
    return;

  Modality = DialogItems[3].ListPos;
  Visuality = DialogItems[4].ListPos;
  RepeatMenu = DialogItems[5].Selected;
  lstrcpy(PathToFilter,*DialogItems[8].Data?DialogItems[8].Data:"zd2t");
  lstrcpy(FilterArguments,*DialogItems[10].Data?DialogItems[10].Data:"/ZNwcvh \"%i\" > \"%o\"");

  SetRegKey("Modality",Modality);
  SetRegKey("Visuality",Visuality);
  SetRegKey("RepeatMenu",RepeatMenu);
  SetRegKey("PathToFilter",PathToFilter);
  SetRegKey("FilterArguments",FilterArguments);
}
