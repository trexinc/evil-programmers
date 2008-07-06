/*
    Mueller sub-plugin for Active-Help plugin for FAR Manager
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
#include "crt.hpp"
#include "memory.hpp"
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

static int Modality, Visuality, RepeatMenu;
enum
{
  MTitle,
  MGeneratingHash,
  MShowIn,
  MModal,
  MNonModal,
  MEditor,
  MViewer,
  MRepeatMenu,
  MErrOpeningDic,
  MErrOpeningHash,
  MErrBadDic,
  MErrBadHash,
  MErrBadX,
  MErrWritingTemp,
  MOk,
  MCancel,
};

static int ShowMueller(const char *FileName, const char *Keyword, char *Error);
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
  FSF.sprintf(PluginRootKey,"%s\\%s",AHInfo->RootKey,"Mueller");
  GetRegKey("Modality",&Modality,0);
  Modality %= 2;
  GetRegKey("Visuality",&Visuality,0);
  Visuality %= 2;
  GetRegKey("RepeatMenu",&RepeatMenu,0);
  RepeatMenu %= 2;
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
      lstrcpy(TypesInfo[0].TypeString,"Mueller");
      lstrcpy(TypesInfo[0].Mask,"mueller.txt");
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
      return ShowMueller(data->FileName,data->Keyword,odata->Error);
    }

    case AHMSG_CONFIG:
    {
      Config();
      return TRUE;
    }
  }
  return FALSE;
}

#define BUFFER_SIZE (64*1024+1)

static int isdigit(int c)
{
  return (c>='0' && c<='9' ? 1 : 0);
}

static int isroman(int c)
{
  return (c=='I' || c=='V' || c=='X' ? 1 : 0);
}

static bool ReadLine(HANDLE file,char *buffer,size_t len)
{
  DWORD CurrPos=SetFilePointer(file,0,NULL,FILE_CURRENT),transferred;
  bool res=true;
  if(ReadFile(file,buffer,len-1,&transferred,NULL)&&transferred)
  {
    buffer[len-1]=0;
    DWORD len=0; char *ptr=buffer;
    for(size_t i=0;i<transferred;i++,len++,ptr++)
    {
      if(buffer[i]=='\n'||buffer[i]=='\r')
      {
        if((buffer[i+1]=='\n'||buffer[i+1]=='\r')&&buffer[i]!=buffer[i+1])
          len++;
        break;
      }
    }
    *ptr=0;
    SetFilePointer(file,CurrPos+len+1,NULL,FILE_BEGIN);
  }
  else
  {
    buffer[0]=0;
    res=false;
  }
  return res;
}

static void Format(int len, const char *str, int indent, HANDLE fp)
{
  #define RLIMIT 76
  const char *ptr = str;
  const char *end = &str[len];
  DWORD t;

  if (indent < 0)
  {
    while (indent < 0)
    {
      WriteFile(fp," ",1,&t,NULL);
      indent++;
    }
  }
  while (ptr < end)
  {
    while ((ptr < end) && ((ptr-str) < RLIMIT))
      ptr++;
    if ((*ptr != 0x20) && (ptr < end))
    {
      while ((ptr > str) && (*ptr != 0x20))
        ptr--;
      if ((*ptr != 0x20) || (ptr == str))
      {
        ptr = ((end-str) > (RLIMIT-1) ? ptr+RLIMIT : end);
      }
    }
    char *text = (char *)malloc(ptr-str+3);
    FSF.sprintf(text,"%.*s\r\n",ptr-str,str);
    WriteFile(fp,text,lstrlen(text),&t,NULL);
    free(text);
    while ((ptr < end) && (*ptr == 0x20))
      ptr++;
    str = ptr;
  }
  if (indent > 0)
  {
    while (indent > 0)
    {
      WriteFile(fp," ",1,&t,NULL);
      indent--;
    }
  }
}

static int ShowMueller(const char *FileName, const char *Keyword, char *Error)
{
  if (!Keyword || !*Keyword)
    return 1;
  char keyword[512];
  lstrcpy(keyword,Keyword);
  if (lstrlen(keyword) < 2)
    lstrcat(keyword," ");

  HANDLE fpDic = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
  char HashFileName[MAX_PATH];
  FSF.sprintf(HashFileName,"%s.hsh",FileName);
  HANDLE fpHash = CreateFile(HashFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);

  if (fpDic==INVALID_HANDLE_VALUE)
  {
    GetMsg(MErrOpeningDic,Error);
    return 0;
  }
  if (fpHash==INVALID_HANDLE_VALUE)
  {
    fpHash = CreateFile(HashFileName,GENERIC_WRITE|GENERIC_READ,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);

    if (fpHash==INVALID_HANDLE_VALUE)
    {
      CloseHandle(fpDic);
      GetMsg(MErrOpeningHash,Error);
      return 0;
    }

    #define START "'c"
    #define PRESTART "'b"
    char DicItem[BUFFER_SIZE+1];
    DWORD pos = 0;

    const char *Msg[2];
    char tmp1[128], tmp2[128];
    Msg[0] = GetMsg(MTitle,tmp1);
    Msg[1] = GetMsg(MGeneratingHash,tmp2);
    HANDLE ss = Info.SaveScreen(0,0,-1,-1);
    Info.Message(Info.ModuleNumber,0,NULL,Msg,2,0);

    while (ReadLine(fpDic,DicItem,BUFFER_SIZE) && FSF.LStrnicmp(START,DicItem,2))
    {
      pos = SetFilePointer(fpDic,0,NULL,FILE_CURRENT);
    }
    if (pos==INVALID_SET_FILE_POINTER || !*DicItem || !*(DicItem+1))
    {
      Info.RestoreScreen(ss);
      CloseHandle(fpHash);
      CloseHandle(fpDic);
      GetMsg(MErrBadDic,Error);
      return 0;
    }
    SetFilePointer(fpDic,pos,NULL,FILE_BEGIN);
    DWORD hash;
    char HashItem[20];
    FSF.sprintf(HashItem,"%s",PRESTART);
    while (ReadLine(fpDic,DicItem,BUFFER_SIZE))
    {
      hash = pos;
      pos = SetFilePointer(fpDic,0,NULL,FILE_CURRENT);
      if (!FSF.LStrnicmp(DicItem,HashItem,2) || lstrlen(DicItem)<2/* || DicItem[1]==0x20*/)
        continue;
      FSF.sprintf(HashItem,"%.2s%d\r\n",DicItem,hash);
      WriteFile(fpHash,HashItem,lstrlen(HashItem),&hash,NULL);
    }
    SetFilePointer(fpDic,0,NULL,FILE_BEGIN);
    SetFilePointer(fpHash,0,NULL,FILE_BEGIN);
    Info.RestoreScreen(ss);
  }

  char HashItem[20];

  while (ReadLine(fpHash,HashItem,20) && FSF.LStrnicmp(keyword,HashItem,2))
    ;

  if (!*HashItem || lstrlen(HashItem) < 3)
  {
    CloseHandle(fpHash);
    CloseHandle(fpDic);
    GetMsg(MErrBadHash,Error);
    return 0;
  }

  if (SetFilePointer(fpDic,FSF.atoi(HashItem+2),NULL,FILE_BEGIN)==INVALID_SET_FILE_POINTER)
  {
    CloseHandle(fpHash);
    CloseHandle(fpDic);
    GetMsg(MErrBadX,Error);
    return 0;
  }

  char DicItem[BUFFER_SIZE+1];
  FarMenuItemEx *Menu=NULL;
  int ItemsCount=0;
  while (1)
  {
    bool found=false;
    DWORD FilePos=SetFilePointer(fpDic,0,NULL,FILE_CURRENT);
    int len;
    while (ReadLine(fpDic,DicItem,BUFFER_SIZE) && !FSF.LStrnicmp(keyword,DicItem,2))
    {
      const char *ptr = strstr((const char *)DicItem,"  ");
      len = (ptr ? ((ptr-DicItem) < 2 ? 2 : ptr-DicItem ) : 0);
      if (lstrlen(keyword) > len || len == 0)
      {
        FilePos=SetFilePointer(fpDic,0,NULL,FILE_CURRENT);
        continue;
      }
      if (!FSF.LStrnicmp(DicItem,keyword,lstrlen(keyword)))
      {
        found=true;
        break;
      }
      FilePos=SetFilePointer(fpDic,0,NULL,FILE_CURRENT);
    }
    if (found)
    {
      Menu = (struct FarMenuItemEx *)realloc(Menu,sizeof(struct FarMenuItemEx)*(++ItemsCount));
      memset(Menu+ItemsCount-1,0,sizeof(struct FarMenuItemEx));
      lstrcpyn(Menu[ItemsCount-1].Text.Text,DicItem,len+1);
      Menu[ItemsCount-1].UserData=FilePos;
    }
    else
      break;
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
        SetFilePointer(fpDic,Menu[Selected].UserData,NULL,FILE_BEGIN);
        ReadLine(fpDic,DicItem,BUFFER_SIZE);
        Show=true;
      }
    }

    if (Show)
    {
      char TempFileName[MAX_PATH];
      HANDLE fpTemp;
      if (CreateDirectory(FSF.MkTemp(TempFileName,"ACTH"),NULL))
      {
        lstrcat(TempFileName,"\\");
        lstrcat(TempFileName,"mueller.txt");
        fpTemp = CreateFile(TempFileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
        if (fpTemp==INVALID_HANDLE_VALUE)
        {
          GetMsg(MErrWritingTemp,Error);
          ret=0;
          break;
        }
      }
      else
      {
        GetMsg(MErrWritingTemp,Error);
        ret=0;
        break;
      }
      const char *str = strstr((const char *)DicItem,"  ");
      size_t len = str-DicItem;
      Format(len,DicItem,0,fpTemp);
      const char *end = &DicItem[lstrlen(DicItem)];
      const char *tmp, *ptr;
      str += 2;
      ptr = str;
      while (ptr<end)
      {
        tmp = ptr;
        if (*ptr == '_' && (*(ptr-1) == 0x20))
        {
          while (isroman(*(++ptr)))
            ;
          if (*ptr == 0x20)
          {
            Format(tmp-str,str,2,fpTemp);
            str = tmp;
            ptr++;
          }
        }
        else if (isdigit(*ptr) && (*(ptr-1) == 0x20))
        {
          while (isdigit(*(++ptr)))
            ;
          if (*ptr == '>')
          {
            Format(tmp-str,str,0,fpTemp);
            str = tmp;
            ptr++;
          }
          else if (*ptr == '.')
          {
            Format(tmp-str,str,2,fpTemp);
            str = tmp;
            ptr++;
          }
        }
        else if ((*(ptr+1) == '>') && (*(ptr-1) == 0x20))
        {
          const char *srch = tmp;
          const char *save = tmp;
          while ((srch > str) && (*srch != ';'))
            srch--;
          if (*srch == ';' && ((srch+2) != save))
          {
            tmp = srch+1;
          }
          Format(tmp-str,str,0,fpTemp);
          if (*srch == ';' && ((srch+2) != save))
          {
            tmp = save;
            Format(tmp-srch-2,srch+2,-1,fpTemp);
          }
          str = tmp;
          ptr+=2;
        }
        else
        {
          ptr++;
        }
      }
      Format(lstrlen(str),str,0,fpTemp);
      CloseHandle(fpTemp);
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
      if (Visuality==0)
        Info.Editor(TempFileName,keyword,0,0,-1,-1,(Modality==0&&!Modal?EF_NONMODAL:0)|EF_DELETEONCLOSE,0,1);
      else
        Info.Viewer(TempFileName,keyword,0,0,-1,-1,(Modality==0&&!Modal?VF_NONMODAL:0)|VF_DELETEONCLOSE);
    }

    if (!RepeatMenu || !Show || !Modality || ItemsCount==1)
      break;
  }

  if (Menu) free(Menu);
  CloseHandle(fpHash);
  CloseHandle(fpDic);

  return ret;
}

static void Config(void)
{
  static const struct InitDialogItem PreDialogItems[] =
  {
    {DI_DOUBLEBOX  ,3  ,1  ,41 ,7  ,0               ,MTitle},
    {DI_TEXT       ,5  ,2  ,0  ,0  ,0               ,MShowIn},
    {DI_COMBOBOX   ,5  ,3  ,20 ,0  ,DIF_DROPDOWNLIST,-1},
    {DI_COMBOBOX   ,23 ,3  ,38 ,0  ,DIF_DROPDOWNLIST,-1},
    {DI_CHECKBOX   ,5  ,4  ,38 ,0  ,0               ,MRepeatMenu},
    {DI_TEXT       ,4  ,5  ,0  ,0  ,DIF_SEPARATOR   ,-1},
    {DI_BUTTON     ,0  ,6  ,0  ,0  ,DIF_CENTERGROUP ,MOk},
    {DI_BUTTON     ,0  ,6  ,0  ,0  ,DIF_CENTERGROUP ,MCancel},
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
  DialogItems[4].Selected = RepeatMenu;
  DialogItems[6].DefaultButton = 1;

  if (Info.Dialog(Info.ModuleNumber,-1,-1,45,9,NULL,(struct FarDialogItem *)&DialogItems,sizeofa(DialogItems)) != 6)
    return;

  Modality = DialogItems[2].ListPos;
  Visuality = DialogItems[3].ListPos;
  RepeatMenu = DialogItems[4].Selected;

  SetRegKey("Modality",Modality);
  SetRegKey("Visuality",Visuality);
  SetRegKey("RepeatMenu",RepeatMenu);
}
