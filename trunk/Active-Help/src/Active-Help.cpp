/*
    Active-Help plugin for FAR Manager
    Copyright (C) 2002 Alex Yaroslavsky

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
#include "Active-Help.hpp"
#include "ahp_internal.hpp"
#include "ahp.hpp"
#include "memory.hpp"
#include "crt.hpp"
#include "common.hpp"
#include "registry.hpp"
#include "language.hpp"
#include "tree.hpp"
#ifdef __GNUC__
#include <limits.h>
#define MAXINT INT_MAX
#else
#include <values.h> //MAXINT
#endif

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
#endif


struct PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
char PluginRootKey[80];

static PluginManager *pm=NULL;

// This chars aren't letters
static char WordDiv[260];
static int WordDivLen;

static int ShowInPanels,
           ShowInViewer,
           ShowInDisks,
           DisksHotkey;

static PSTree *Tree=NULL;

enum
{
  CURRENT_HELP = 0,
  LAST_HELP    = 1,
  ACTIVATE     = 2,
  OPEN_HELP    = 4,
};

BOOL MyIsAlpha(unsigned char c);
BOOL FindBounds(const char *Str, int Len, int Pos, int *Start, int *End);
int FindEnd(const char *Str, int Len, int Pos);
int FindStart(const char *Str, int Len, int Pos);
int DoMain(char *FileName, char *DefKeyword, int OpenFrom, EditorInfo *ei, char *Title, char *How, char *Encoding, char *Type);

static int SayError(const char *s, const char *type=NULL)
{
  const char *err[3];
  char title[128];
  if (type)
  {
    FSF.sprintf(title,"%s: %s",GetMsg(MErrTitle),type);
    err[0]=title;
  }
  else
    err[0] = GetMsg(MErrTitle);
  err[1] = s;
  err[2] = GetMsg(MOk);
  Info.Message( Info.ModuleNumber, FMSG_WARNING, NULL, err, 3, 1 );
  return 0;
}

int GetRealState(char *ff, unsigned int DefFlags, int Flag)
{
  int state = Flag/2>=lstrlen(ff)?2:(ff[Flag/2]-'0');
  if (state==2)
    return DefFlags&Flag;
  return (ff[Flag/2]-'0');
}

int GetState(char *ff, int Flag)
{
  if (Flag/2>=lstrlen(ff))
    return 2;
  return (ff[Flag/2]-'0');
}

void SetState(char *ff, int State, int Flag)
{
  int l=lstrlen(ff);
  if (Flag/2>=l)
  {
    memset(ff+l,2+'0',Flag/2-l+1);
    ff[Flag/2+1]=0;
  }
  ff[Flag/2]=State+'0';
}

bool Encode(char *str, int len, const char *encoding)
{
  if (*encoding==0 || lstrcmp(encoding,"OEM")==0)
    return true;
  if (lstrcmp(encoding,"WIN")==0)
  {
    OemToCharBuff(str,str,len);
    return true;
  }
  /*
  for (int i=0,struct CharTableSet cts; Info.CharTable(i,(char *)cts,sizeof(cts))!=-1; i++)
  {
    if (strstr(cts.TableName,encoding))
    {
      for (int i=0; i<len; i++)
       str[i]=cts.EncodeTable[str[i]];
      return true;
    }
  }
  */
  return false;
}

bool Decode(char *str, int len, const char *encoding)
{
  if (*encoding==0 || lstrcmp(encoding,"OEM")==0)
    return true;
  if (lstrcmp(encoding,"WIN")==0)
  {
    CharToOemBuff(str,str,len);
    return true;
  }
  /*
  for (int i=0,struct CharTableSet cts; Info.CharTable(i,(char *)cts,sizeof(cts))!=-1; i++)
  {
    if (strstr(cts.TableName,encoding))
    {
      for (int i=0; i<len; i++)
       str[i]=cts.DecodeTable[str[i]];
      return true;
    }
  }
  */
  return false;
}

char *InputKeyword(const char *Keyword, const char *Title)
{
  static const char KeywordHistory[] = "Active-Help-Mueller"; //keeping old name
  #define MDLG_WIDTH 60
  #define MDLG_HEIGHT 5
  static const struct InitDialogItem PreDialogItems[] =
  {
    {DI_DOUBLEBOX   ,3  ,1  ,MDLG_WIDTH-4 ,MDLG_HEIGHT-2 ,0                              ,-1},
    {DI_EDIT        ,5  ,2  ,MDLG_WIDTH-6 ,0             ,DIF_HISTORY|DIF_USELASTHISTORY ,-1},
  };
  struct FarDialogItem DialogItems[sizeof(PreDialogItems)/sizeof(PreDialogItems[0])];

  InitDialogItems(PreDialogItems,DialogItems,sizeof(PreDialogItems)/sizeof(PreDialogItems[0]));
  DialogItems[1].Focus = TRUE;
  DialogItems[1].History = KeywordHistory;
  DialogItems[1].DefaultButton = 1;
  char title[512];
  lstrcpy(title,Title);
  FSF.TruncStr(title,MDLG_WIDTH-4-lstrlen(GetMsg(MEnterKeyword))+2);
  FSF.sprintf(DialogItems[0].Data,GetMsg(MEnterKeyword),title);
  FSF.sprintf(DialogItems[1].Data,"%.*s",512,Keyword);
  int ret = Info.Dialog(Info.ModuleNumber,-1,-1,MDLG_WIDTH,MDLG_HEIGHT,"EnterKeyword",(struct FarDialogItem *)&DialogItems,sizeof(PreDialogItems)/sizeof(PreDialogItems[0]));
  if (ret != 1)
    return NULL;
  char *temp = (char *)malloc((lstrlen(DialogItems[1].Data)+1)*sizeof(char));
  lstrcpy(temp,DialogItems[1].Data);
  return temp;
}

inline char *Process(char *String)
{
  FSF.Unquote(String);
  FSF.ExpandEnvironmentStr(String,String,512);
  return String;
}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *psi)
{
  Info=*psi;
  FSF=*psi->FSF;
  Info.FSF=&FSF;
  FSF.sprintf(PluginRootKey,"%s\\Active-Help",Info.RootKey);

  Tree = new PSTree;

  { //init plugins
    char plug_dir[MAX_PATH];
    lstrcpy(plug_dir,Info.ModuleName);
    *(FSF.PointToName(plug_dir))=0;
    lstrcat(plug_dir,"ahp\\");
    pm=new PluginManager(plug_dir);
  }

  GetRegKey("ShowInPanels",&ShowInPanels,1);
  GetRegKey("ShowInViewer",&ShowInViewer,1);
  GetRegKey("ShowInDisks",&ShowInDisks,0);
  GetRegKey("DisksHotkey",&DisksHotkey,0);
  if (DisksHotkey < 0 || DisksHotkey > 9)
    DisksHotkey = 0;

  WordDivLen = Info.AdvControl(Info.ModuleNumber, ACTL_GETSYSWORDDIV, WordDiv);
  //char AddWordDiv[sizeof(WordDiv)];
  //GetRegKey("AddWordDiv",AddWordDiv,"#",sizeof(AddWordDiv));
  //WordDivLen += lstrlen(AddWordDiv);
  //lstrcat(WordDiv, AddWordDiv);
  WordDivLen += sizeof(" \n\r\t");
  lstrcat(WordDiv, " \n\r\t");
}

void WINAPI _export GetPluginInfo(struct PluginInfo *pi)
{
  pi->StructSize=sizeof(struct PluginInfo);
  pi->Flags=PF_EDITOR;
  if (ShowInViewer)
    pi->Flags|=PF_VIEWER;
  if (!ShowInPanels)
    pi->Flags|=PF_DISABLEPANELS;
  static const char *MenuStrings[1];
  MenuStrings[0]=GetMsg(MTitle);
  pi->PluginMenuStrings=MenuStrings;
  pi->PluginMenuStringsNumber=1;
  pi->PluginConfigStrings=MenuStrings;
  pi->PluginConfigStringsNumber=1;
  if (ShowInDisks)
  {
    pi->DiskMenuStrings=MenuStrings;
    pi->DiskMenuStringsNumber=1;
    pi->DiskMenuNumbers = &DisksHotkey;
  }
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  (void) Item;

  struct EditorInfo ei;
  char FileName[512];
  if (OpenFrom == OPEN_EDITOR)
  {
    EditorSetParameter p;
    p.Type=ESPT_GETWORDDIV;
    p.Param.cParam=WordDiv;
    if (Info.EditorControl(ECTL_SETPARAM, &p))
    {
      WordDivLen = lstrlen(WordDiv);
      WordDivLen += sizeof(" \n\r\t");
      lstrcat(WordDiv," \n\r\t");
    }
    Info.EditorControl(ECTL_GETINFO,(void *)&ei);
    lstrcpy(FileName,ei.FileName);
  }
  else if (OpenFrom == OPEN_VIEWER)
  {
    struct ViewerInfo vi;
    vi.StructSize=sizeof(vi);
    Info.ViewerControl(VCTL_GETINFO,(void *)&vi);
    lstrcpy(FileName,vi.FileName);
  }
  else
    *FileName = 0;

  char Type[512];
  char DefKeyword[512];
  char Title[512];
  char Encoding[128];
  char Flags[20];
  int How;

  int ret = DoMain(FileName,DefKeyword,OpenFrom,&ei,Title,Flags,Encoding,Type);

  if (ret == -1)
    return INVALID_HANDLE_VALUE;

  char *Keyword=NULL;
  Process(FileName);

  if (ret == LAST_HELP)
  {
    int j;
    GetRegKey("LastKeywordLength",&j,0);
    Keyword = (char *)malloc((j+1)*sizeof(char));
    GetRegKey("LastKeyword",Keyword,"",j+1);
    GetRegKey("LastHelpFile",FileName,"",sizeof(FileName));
    GetRegKey("LastHelpTitle",Title,"",sizeof(Title));
    GetRegKey("LastHelpEncoding",Encoding,"",sizeof(Encoding));
    GetRegKey("LastHelpFlags",(int *)&How,0);
    if (!*FileName)
      return INVALID_HANDLE_VALUE;
  }

  if (ret != ACTIVATE)
  {
    const struct TypeInfo **data;
    int Types;
    pm->DetectType(!*Type?FileName:Type,!*Type?0:1,&data,&Types);
    if (Types>1)
    {
      struct FarMenuItem *menu = (struct FarMenuItem *)malloc(Types*sizeof(struct FarMenuItem));
      for (int i=0; i<Types; i++)
      {
        lstrcpy(menu[i].Text,data[i]->TypeString);
      }
      Types = Info.Menu(Info.ModuleNumber,-1,-1,0,
                    FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT,
                    GetMsg(MSelectType),NULL,NULL,NULL,NULL,
                    menu,Types);
      free(menu);
      if (Types==-1)
      {
        free(data);
        return INVALID_HANDLE_VALUE;
      }
    }
    else if (Types==1)
    {
      Types=0;
    }
    else
      return INVALID_HANDLE_VALUE;
    if (lstrcmp(Encoding,"AUTO")==0)
      lstrcpy(Encoding,data[Types]->Encoding);
    lstrcpy(Type,data[Types]->TypeString);
    if (ret != LAST_HELP)
    {
      How=0;
      if (GetRealState(Flags,data[Types]->HFlags,F_INPUTKEYWORD))
      How|=F_INPUTKEYWORD;
      if (GetRealState(Flags,data[Types]->HFlags,F_NOKEYWORDSEARCH))
      How|=F_NOKEYWORDSEARCH;
    }
    free(data);
  }
  else
    How = 0;


  if (ret == CURRENT_HELP && OpenFrom == OPEN_EDITOR && !(How&F_NOKEYWORDSEARCH))
  {
    struct EditorGetString egs;
    egs.StringNumber = -1;
    if (Info.EditorControl(ECTL_GETSTRING,(void *)&egs))
    {
      char *temp = (char *)malloc(egs.StringLength+1);
      memcpy(temp,egs.StringText,egs.StringLength);
      temp[egs.StringLength] = '\0';
      struct EditorConvertText ect;
      ect.Text = temp;
      ect.TextLength = egs.StringLength;
      Info.EditorControl(ECTL_EDITORTOOEM,(void *)&ect);
      int Start, End, i;
      if (egs.SelStart != -1 && egs.SelStart != egs.SelEnd && egs.SelStart != egs.StringLength)
      {
        i = 1;
        Start = egs.SelStart;
        End = (egs.SelEnd == -1 ? egs.StringLength : egs.SelEnd);
      }
      else
      {
        Start = 0;
        End = egs.StringLength;
        i = FindBounds(temp,egs.StringLength,ei.CurPos,&Start,&End);
      }
      if (i)
      {
        Keyword = (char *)malloc(End-Start+1);
        memcpy(Keyword,temp+Start,End-Start);
        Keyword[End-Start] = '\0';
      }
      if (temp)
        free(temp);
    }
    else
    {
      SayError(GetMsg(MErrEditor));
      return INVALID_HANDLE_VALUE;
    }
  }

  if (ret == OPEN_HELP || (ret == CURRENT_HELP && (How&F_NOKEYWORDSEARCH || OpenFrom!=OPEN_EDITOR)))
  {
    Keyword = (char *)malloc((lstrlen(DefKeyword)+1)*sizeof(char));
    lstrcpy(Keyword,DefKeyword);
  }

  if (!Keyword)
  {
    Keyword = (char *)malloc(1);
    Keyword[0] = '\0';
  }

  if ((ret == CURRENT_HELP && !*Keyword && !(How&F_NOKEYWORDSEARCH)) || (ret==CURRENT_HELP && OpenFrom!=OPEN_EDITOR) || How&F_INPUTKEYWORD)
  {
    char *tmp = InputKeyword(Keyword,Title);
    if (Keyword) free(Keyword);
    Keyword = tmp;
    if (!Keyword)
      return INVALID_HANDLE_VALUE;
  }

  if (ret == CURRENT_HELP || ret == LAST_HELP || ret == OPEN_HELP)
  {
    int OK = TRUE;
    char Error[128];

    if (*FileName)
    {
      SetRegKey("LastKeywordLength",lstrlen(Keyword));
      SetRegKey("LastKeyword",Keyword);
      SetRegKey("LastHelpFile",FileName);
      SetRegKey("LastHelpTitle",Title);
      SetRegKey("LastHelpEncoding",Encoding);
      SetRegKey("LastHelpFlags",How);
      Encode(Keyword,lstrlen(Keyword),Encoding);
      OK = pm->ShowHelp(Type,FileName,Keyword,How,ret,Encoding,Error);
    }
    if (Keyword)
     free(Keyword);

    if (!OK)
    {
      SayError(Error,Type);
    }
  }

  if (ret == ACTIVATE && OpenFrom == OPEN_EDITOR)
  {
    struct EditorConvertText ect;
    struct EditorGetString egs;
    int Start, End;
    egs.StringNumber=-1;
    if (Info.EditorControl(ECTL_GETSTRING,(void *)&egs))
    {
      if (egs.SelStart != -1 && egs.SelStart != egs.SelEnd && egs.SelStart != egs.StringLength)
      {
        Start = egs.SelStart;
        End = (egs.SelEnd == -1 ? egs.StringLength : egs.SelEnd);
      }
      else
      {
        if (ei.CurPos >= egs.StringLength || !egs.StringLength)
          return INVALID_HANDLE_VALUE;
        Start = ei.CurPos;
        while (egs.StringText[Start] != 9 && egs.StringText[Start] != 32 && Start >= 0)
          Start--;
        End = ei.CurPos;
        while (egs.StringText[End] != 9 && egs.StringText[End] != 32 && End < egs.StringLength)
          End++;
        if (End == Start)
          return INVALID_HANDLE_VALUE;
        Start++;
      }
      Keyword = (char *)malloc(End-Start+1);
      memcpy(Keyword,egs.StringText+Start,End-Start);
      Keyword[End-Start] = '\0';
      ect.Text = Keyword;
      ect.TextLength = End-Start;
      Info.EditorControl(ECTL_EDITORTOOEM,(void *)&ect);
      OemToCharBuff(Keyword,Keyword,End-Start);
    }
    else
      return INVALID_HANDLE_VALUE;
    ShellExecute(GetDesktopWindow(), "open", Keyword, NULL, NULL, SW_SHOWNORMAL);
    if (Keyword)
      free(Keyword);
  }
  return INVALID_HANDLE_VALUE;
}

int GetHelpBasicInfo(char *fd, char *fm, int i)
{
  char Value[50];
  FSF.sprintf(Value,"Desc%d",i);
  if (!GetRegKey(Value,fd,"",512))
    return FALSE;
  FSF.sprintf(Value,"Mask%d",i);
  GetRegKey(Value,fm,"",512);
  return TRUE;
}

int GetHelpInfo(char *fd, char *ft, char *fn, char *fk, char *fm, char *fe, char *ff, int i)
{
  char Value[50];
  FSF.sprintf(Value,"Desc%d",i);
  GetRegKey(Value,fd,"",512);
  FSF.sprintf(Value,"Type%d",i);
  GetRegKey(Value,ft,"",512);
  FSF.sprintf(Value,"File%d",i);
  GetRegKey(Value,fn,"",512);
  FSF.sprintf(Value,"Keyword%d",i);
  GetRegKey(Value,fk,"",512);
  FSF.sprintf(Value,"Mask%d",i);
  GetRegKey(Value,fm,"",512);
  FSF.sprintf(Value,"Flags%d",i);
  GetRegKey(Value,ff,"",20);
  FSF.sprintf(Value,"Encoding%d",i);
  GetRegKey(Value,fe,"AUTO",128);
  return TRUE;
}

void SetHelpInfo(char *fd, char *ft, char *fn, char *fk, char *fm, char *fe, char *ff, int i)
{
  char Value[50];
  FSF.sprintf(Value,"Desc%d",i);
  SetRegKey(Value,fd);
  FSF.sprintf(Value,"Type%d",i);
  SetRegKey(Value,ft);
  FSF.sprintf(Value,"File%d",i);
  SetRegKey(Value,fn);
  FSF.sprintf(Value,"Keyword%d",i);
  SetRegKey(Value,fk);
  FSF.sprintf(Value,"Mask%d",i);
  SetRegKey(Value,fm);
  FSF.sprintf(Value,"Flags%d",i);
  SetRegKey(Value,ff);
  FSF.sprintf(Value,"Encoding%d",i);
  SetRegKey(Value,fe);
}

void DelHelpInfo(int i)
{
  char Value[50];
  FSF.sprintf(Value,"Desc%d",i);
  DelRegValue(Value);
  FSF.sprintf(Value,"Type%d",i);
  DelRegValue(Value);
  FSF.sprintf(Value,"File%d",i);
  DelRegValue(Value);
  FSF.sprintf(Value,"Keyword%d",i);
  DelRegValue(Value);
  FSF.sprintf(Value,"Mask%d",i);
  DelRegValue(Value);
  FSF.sprintf(Value,"Flags%d",i);
  DelRegValue(Value);
  FSF.sprintf(Value,"Encoding%d",i);
  DelRegValue(Value);
}

int DoMain(char *FileName, char *DefKeyword, int OpenFrom, EditorInfo *ei, char *Title, char *How, char *Encoding, char *Type)
{
  static const char DescHistory[] = "Active-Help-Desc";
  static const char PathHistory[] = "Active-Help-Path";
  static const char KeywordHistory[] = "Active-Help-Keyword";
  static const char MaskHistory[] = "Active-Help-Mask";
  static const struct InitDialogItem PreDialogItems[] =
  {
    {DI_DOUBLEBOX   ,3  ,1  ,73 ,18 ,0                  ,MTitle},
    {DI_TEXT        ,5  ,2  ,0  ,0  ,0                  ,MDesc},
    {DI_EDIT        ,5  ,3  ,70 ,0  ,DIF_HISTORY        ,-1},
    {DI_TEXT        ,5  ,4  ,0  ,0  ,0                  ,MType},
    {DI_COMBOBOX    ,5  ,5  ,70 ,0  ,DIF_LISTWRAPMODE   ,-1},
    {DI_TEXT        ,5  ,6  ,0  ,0  ,0                  ,MFile},
    {DI_EDIT        ,5  ,7  ,70 ,0  ,DIF_HISTORY        ,-1},
    {DI_TEXT        ,5  ,8  ,0  ,0  ,0                  ,MKeyword},
    {DI_EDIT        ,5  ,9  ,70 ,0  ,DIF_HISTORY        ,-1},
    {DI_TEXT        ,5  ,10 ,0  ,0  ,0                  ,MMask},
    {DI_EDIT        ,5  ,11 ,70 ,0  ,DIF_HISTORY        ,-1},
    {DI_TEXT        ,0  ,12 ,0  ,0  ,DIF_SEPARATOR      ,-1},
    {DI_CHECKBOX    ,5  ,13 ,0  ,0  ,DIF_3STATE         ,MChk_InputKeyword},
    {DI_CHECKBOX    ,5  ,14 ,0  ,0  ,DIF_3STATE         ,MChk_NoKeywordSearch},
    {DI_TEXT        ,5  ,15 ,0  ,0  ,0                  ,MEncoding},
    {DI_COMBOBOX    ,25 ,15 ,70 ,0  ,DIF_DROPDOWNLIST|DIF_LISTWRAPMODE ,-1},
    {DI_TEXT        ,0  ,16 ,0  ,0  ,DIF_SEPARATOR      ,-1},
    {DI_BUTTON      ,0  ,17 ,0  ,0  ,DIF_CENTERGROUP    ,MOk},
    {DI_BUTTON      ,0  ,17 ,0  ,0  ,DIF_CENTERGROUP    ,MCancel},
  };
  enum
  {
    MAX_ITEMS = ('Z' - 'A' + 1),
    FIRST_INDEX = 5,
    MENU_ITEMS = (MAX_ITEMS + 1 + FIRST_INDEX),
    MENU_CURRENT_HELP = 0,
    MENU_CURRENT_HELP_BY_MASK,
    MENU_LAST_HELP,
    MENU_ACTIVATE,
    CMD_INSERT = 0,
    CMD_EDIT,
    CMD_DELETE,
    CMD_DOWN,
    CMD_UP,
    CMD_TEMPSELECT,
    CMD_SELECT,
    CMD_CONFIG,
    DLG_WIDTH = 77,
    DLG_HEIGHT = 20,
    I_DESC = 2,
    I_TXT2,
    I_TYPE,
    I_TXT3,
    I_FILE,
    I_TXT4,
    I_KWRD,
    I_TXT5,
    I_MASK,
    I_SEP1,
    I_INKEY,
    I_NOSRCH,
    I_TXT6,
    I_ENCD,
    I_TXT7,
    I_OK,
  };
  struct FarDialogItem DialogItems[sizeof(PreDialogItems)/sizeof(PreDialogItems[0])];

  InitDialogItems(PreDialogItems,DialogItems,sizeof(PreDialogItems)/sizeof(PreDialogItems[0]));
  DialogItems[I_DESC].History = DescHistory;
  DialogItems[I_FILE].History = PathHistory;
  DialogItems[I_KWRD].History = KeywordHistory;
  DialogItems[I_MASK].History = MaskHistory;
  struct FarListItem eli[3];
  struct FarList efl = {3, eli};
  memset(eli,0,sizeof(eli));
  lstrcpy(eli[0].Text,"AUTO");
  lstrcpy(eli[1].Text,"OEM");
  lstrcpy(eli[2].Text,"WIN");
  DialogItems[I_OK].DefaultButton = 1;

  struct FarMenuItem MenuItems[MENU_ITEMS];
  memset(MenuItems,0,sizeof(MenuItems));
  lstrcpy(MenuItems[MENU_CURRENT_HELP].Text,GetMsg(MCurrentHelp));
  lstrcpy(MenuItems[MENU_CURRENT_HELP_BY_MASK].Text,GetMsg(MCurrentHelpByMask));
  lstrcpy(MenuItems[MENU_LAST_HELP].Text,GetMsg(MLastHelp));
  lstrcpy(MenuItems[MENU_ACTIVATE].Text,GetMsg(MActivate));
  MenuItems[FIRST_INDEX-1].Separator=1;

  struct FarMenuItemEx MenuItems2[MAX_ITEMS];
  memset(MenuItems2,0,sizeof(MenuItems2));

  char fd[512], ft[512], fn[512], fk[512], fm[512], fe[128], ff[20];
  int i, j=-1, k=0;
  for (i=0; i<MAX_ITEMS; i++)
  {
    if (!GetHelpBasicInfo(fd,fm,i))
      break;
    FSF.sprintf(MenuItems[i+FIRST_INDEX].Text,"&%c. %.*s",i+'A',66,fd);
    if (OpenFrom == OPEN_PLUGINSMENU || OpenFrom == OPEN_DISKMENU || ((OpenFrom == OPEN_EDITOR || OpenFrom == OPEN_VIEWER) && fm[0] && FSF.ProcessName(fm,FileName,PN_CMPNAMELIST|PN_SKIPPATH)))
    {
      if (j==-1)
        j = i;
      FSF.sprintf(MenuItems2[k].Text.Text,"&%c. %.*s",k+'A',66,fd);
      MenuItems2[k++].UserData=i+FIRST_INDEX;
    }
  }
  FSF.sprintf(MenuItems[i+FIRST_INDEX].Text,"%*c",66/2,' ');

  int TotalItems = i;
  int SelectedItem = 0;
  if (j != -1)
  {
    SelectedItem = j+FIRST_INDEX;
  }
  else
  {
    if (TotalItems)
    {
      GetRegKey("SelectedItem",&SelectedItem,FIRST_INDEX);
      if (SelectedItem < FIRST_INDEX || SelectedItem > (TotalItems+FIRST_INDEX-1))
        SelectedItem = FIRST_INDEX;
    }
  }
  if (OpenFrom == OPEN_EDITOR)
  {
    AHTrackInfo ti;
    ti.EditorID = ei->EditorID;
    if (Tree->GetID(&ti) && TotalItems)
    {
      if (ti.SelectedItem >= FIRST_INDEX && ti.SelectedItem <= (TotalItems+FIRST_INDEX-1))
        SelectedItem = ti.SelectedItem;
    }
  }
  int TempSelectedItem = SelectedItem;
  int LastPos;
  GetRegKey("LastPos",&LastPos,0);
  if (LastPos < 0 || LastPos > (TotalItems+FIRST_INDEX))
    LastPos = 0;

  *Title = *FileName = *Type = *Encoding = *DefKeyword = *How = 0;

  int ret = -1;
  while (1)
  {
    if (ret != -1)
      break;
    if (TempSelectedItem)
      MenuItems[TempSelectedItem].Checked = '+';
    if (SelectedItem)
      MenuItems[SelectedItem].Checked = '*';
    MenuItems[LastPos].Selected = 1;

    int BreakCode;
    static const int BreakKeys[] =
    {
      VK_INSERT,
      VK_F4,
      VK_DELETE,
      (PKF_CONTROL<<16)|VK_DOWN,
      (PKF_CONTROL<<16)|VK_UP,
      VK_SPACE,
      (PKF_CONTROL<<16)|VK_SPACE,
      (PKF_ALT<<16)|(PKF_SHIFT<<16)|VK_F9,
      VK_RETURN,
      0
    };
    i = Info.Menu(Info.ModuleNumber,-1,-1,0,
                  FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT,
                  GetMsg(MTitle),GetMsg(MMenuCommands),"Usage",BreakKeys,&BreakCode,
                  MenuItems,TotalItems+1+FIRST_INDEX);

    if (i>=0)
    {
      MenuItems[LastPos].Selected = 0;
      LastPos = i;
      if (BreakCode == -1 && i >= FIRST_INDEX && TotalItems)
        BreakCode = CMD_TEMPSELECT;
      switch (BreakCode)
      {
        case CMD_INSERT:
        case CMD_EDIT:
        {
          eli[0].Flags=0;
          eli[1].Flags=0;
          eli[2].Flags=0;
          if (BreakCode==CMD_EDIT)
          {
            if (i>=TotalItems+FIRST_INDEX || i < FIRST_INDEX)
              break;
            i = i - FIRST_INDEX;
            GetHelpInfo(fd,ft,fn,fk,fm,fe,ff,i);
            lstrcpy(DialogItems[I_DESC].Data,fd);
            lstrcpy(DialogItems[I_TYPE].Data,ft);
            lstrcpy(DialogItems[I_FILE].Data,fn);
            lstrcpy(DialogItems[I_KWRD].Data,fk);
            lstrcpy(DialogItems[I_MASK].Data,fm);
            DialogItems[I_INKEY].Selected = GetState(ff,F_INPUTKEYWORD);
            DialogItems[I_NOSRCH].Selected = GetState(ff,F_NOKEYWORDSEARCH);
            if (lstrcmp(fe,"AUTO")==0 || *fe==0)
              eli[0].Flags=LIF_SELECTED;
            else if (lstrcmp(fe,"WIN")==0)
              eli[2].Flags=LIF_SELECTED;
            else
              eli[1].Flags=LIF_SELECTED;
          }
          else
          {
            DialogItems[I_DESC].Data[0] = 0;
            DialogItems[I_TYPE].Data[0] = 0;
            DialogItems[I_FILE].Data[0] = 0;
            DialogItems[I_KWRD].Data[0] = 0;
            DialogItems[I_MASK].Data[0] = 0;
            DialogItems[I_INKEY].Selected = 2;
            DialogItems[I_NOSRCH].Selected = 2;
            eli[0].Flags=LIF_SELECTED;
          }
          DialogItems[I_TYPE].ListItems = &(pm->TypesList);
          DialogItems[I_ENCD].ListItems = &efl;
          DialogItems[I_ENCD].Data[0] = 0;
          DialogItems[I_DESC].Focus = TRUE;
          j = Info.Dialog(Info.ModuleNumber,-1,-1,DLG_WIDTH,DLG_HEIGHT,"Settings",(struct FarDialogItem *)&DialogItems,sizeof(PreDialogItems)/sizeof(PreDialogItems[0]));
          if (j!=I_OK || !DialogItems[I_FILE].Data[0] || !DialogItems[I_DESC].Data[0] || (TotalItems >= MAX_ITEMS && BreakCode == 0))
            break;
          if (BreakCode==CMD_INSERT)
          {
            if (!TotalItems)
            {
              TempSelectedItem = SelectedItem = FIRST_INDEX;
            }
            LastPos = TotalItems+FIRST_INDEX;
            i = ++TotalItems - 1;
            MenuItems[TotalItems+FIRST_INDEX].Checked=MenuItems[TotalItems+FIRST_INDEX].Selected=MenuItems[TotalItems+FIRST_INDEX].Separator=0;
            FSF.sprintf(MenuItems[TotalItems+FIRST_INDEX].Text,"%*c",66/2,' ');
          }
          SetState(ff,DialogItems[I_INKEY].Selected,F_INPUTKEYWORD);
          SetState(ff,DialogItems[I_NOSRCH].Selected,F_NOKEYWORDSEARCH);
          memset(&MenuItems[i+FIRST_INDEX],0,sizeof(MenuItems[0]));
          FSF.sprintf(MenuItems[i+FIRST_INDEX].Text,"&%c. %.*s",i+'A',66,DialogItems[I_DESC].Data);
          SetHelpInfo(DialogItems[I_DESC].Data,DialogItems[I_TYPE].Data,DialogItems[I_FILE].Data,DialogItems[I_KWRD].Data,DialogItems[I_MASK].Data,eli[DialogItems[I_ENCD].ListPos].Text,ff,i);
          break;
        }

        case CMD_DELETE:
        {
          if (i >= TotalItems+FIRST_INDEX || i < FIRST_INDEX || !TotalItems)
            break;
          const char *msg[2];
          msg[0] = GetMsg(MTitle);
          msg[1] = GetMsg(MDeleteAsk);
          if (Info.Message(Info.ModuleNumber,FMSG_MB_YESNO,NULL,msg,2,0) != 0)
            break;
          if (i == TotalItems+FIRST_INDEX-1 && TotalItems > 1)
            LastPos--;
          if (SelectedItem >= i)
          {
            MenuItems[SelectedItem].Checked = 0;
            SelectedItem = SelectedItem==i ? FIRST_INDEX : SelectedItem-1;
            if (TotalItems == 1)
              SelectedItem = 0;
          }
          if (TempSelectedItem >= i)
          {
            MenuItems[TempSelectedItem].Checked = 0;
            TempSelectedItem = SelectedItem;
          }
          for (i=i-FIRST_INDEX; i<TotalItems-1; i++)
          {
            MenuItems[i+FIRST_INDEX] = MenuItems[i+1+FIRST_INDEX];
            GetHelpInfo(fd,ft,fn,fk,fm,fe,ff,i+1);
            SetHelpInfo(fd,ft,fn,fk,fm,fe,ff,i);
          }
          MenuItems[i+FIRST_INDEX] = MenuItems[i+1+FIRST_INDEX];
          TotalItems--;
          DelHelpInfo(TotalItems);
          for (i=0; i<TotalItems; i++)
            MenuItems[i+FIRST_INDEX].Text[1] = i + 'A';
          break;
        }

        case CMD_DOWN:
        case CMD_UP:
        {
          if (BreakCode == CMD_DOWN && (TotalItems == 1 || i >= TotalItems+FIRST_INDEX-1 || i < FIRST_INDEX))
            break;
          if (BreakCode == CMD_UP && (TotalItems == 1 || i >= TotalItems+FIRST_INDEX || i <= FIRST_INDEX))
            break;
          int x = BreakCode==CMD_UP ? -1 : 1;
          int *sel;
          for (j=0,sel=&SelectedItem; j<2; j++,sel=&TempSelectedItem)
          {
            if (*sel == i)
            {
              MenuItems[*sel].Checked = 0;
              *sel += x;
            }
            else
            {
              if (*sel == i+x)
              {
                MenuItems[*sel].Checked = 0;
                *sel -= x;
              }
            }
          }
          LastPos += x;
          i = i - FIRST_INDEX;
          struct FarMenuItem mi_tmp = MenuItems[i+x+FIRST_INDEX];
          MenuItems[i+x+FIRST_INDEX] = MenuItems[i+FIRST_INDEX];
          MenuItems[i+FIRST_INDEX] = mi_tmp;
          j = MenuItems[i+x+FIRST_INDEX].Text[1];
          MenuItems[i+x+FIRST_INDEX].Text[1] = (char)(j+x);
          MenuItems[i+FIRST_INDEX].Text[1] = (char)j;
          char fd2[512], ft2[512], fn2[512], fk2[512], fm2[512], fe2[128], ff2[20];
          GetHelpInfo(fd,ft,fn,fk,fm,fe,ff,i);
          GetHelpInfo(fd2,ft2,fn2,fk2,fm2,fe2,ff2,i+x);
          SetHelpInfo(fd2,ft2,fn2,fk2,fm2,fe2,ff2,i);
          SetHelpInfo(fd,ft,fn,fk,fm,fe,ff,i+x);
          break;
        }

        case CMD_TEMPSELECT:
        case CMD_SELECT:
        {
          if (TotalItems && i >=FIRST_INDEX && i < (TotalItems+FIRST_INDEX))
          {
            if (TempSelectedItem)
              MenuItems[TempSelectedItem].Checked = 0;
            TempSelectedItem = i;
            if (BreakCode==CMD_SELECT)
            {
              if (SelectedItem)
                MenuItems[SelectedItem].Checked = 0;
              SelectedItem = i;
            }
          }
          break;
        }

        case CMD_CONFIG:
         Configure(0);
         break;

        default:
        {
          if ((TotalItems && i < (TotalItems+FIRST_INDEX)) || i==MENU_ACTIVATE)
          {
            if (i >= FIRST_INDEX)
            {
              if (SelectedItem)
                MenuItems[SelectedItem].Checked = 0;
              if (TempSelectedItem)
                MenuItems[TempSelectedItem].Checked = 0;
              TempSelectedItem = SelectedItem = i;
            }
            if (OpenFrom == OPEN_EDITOR)
            {
              AHTrackInfo ti;
              ti.EditorID = ei->EditorID;
              ti.SelectedItem = SelectedItem;
              Tree->Insert(&ti);
            }
            if (i==MENU_CURRENT_HELP_BY_MASK && k)
            {
              int i2=0;

              if (k>1)
              {
                i2=Info.Menu(Info.ModuleNumber,-1,-1,0,
                             FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT|FMENU_USEEXT,
                             GetMsg(MTitle),NULL,NULL,NULL,NULL,
                             (const struct FarMenuItem *)MenuItems2,k);
                if (i2==-1)
                  break;
              }

              TempSelectedItem=MenuItems2[i2].UserData;
            }
            if (i!=MENU_ACTIVATE)
              GetHelpInfo(Title,Type,FileName,DefKeyword,fm,Encoding,How,TempSelectedItem-FIRST_INDEX);

            ret = i<FIRST_INDEX ? (i>0 ? --i : i) : OPEN_HELP;
          }
          break;
        }
      }
      continue;
    }
    break;
  }

  SetRegKey("SelectedItem",SelectedItem);
  SetRegKey("LastPos",LastPos);
  return ret;
}

// What we consider as letter
BOOL MyIsAlpha(unsigned char c)
{
  return (memchr(WordDiv, c, WordDivLen)==NULL?TRUE:FALSE);
}

// Finding word bounds (what'll be converted) (Str is in OEM)
BOOL FindBounds(const char *Str, int Len, int Pos, int *Start, int *End)
{
  int i = 1;
  BOOL ret = FALSE;
  int r = MAXINT;

  // If line isn't ""
  if( Len>*Start )
  {
    *End=min(*End,Len);

    // Pos between [Start, End] ?
    Pos=max(Pos,*Start);
    Pos=min(*End,Pos);

    // If current character is non letter
    if(!MyIsAlpha(Str[Pos]))
    {
      // Looking for letter on the left and counting radius
      while((*Start<=Pos-i) && (!MyIsAlpha(Str[Pos-i])))
        i++;

      // Letter was found on the left
      if(*Start<=Pos-i)
        r=i; // Storing radius

      i=1;
      // Looking for letter on the right and counting radius
      while((Pos+i<=*End) && (!MyIsAlpha(Str[Pos+i])))
        i++;

      // Letter was not found
      if(Pos+i>*End)
        i=MAXINT;

      // Here r is left radius and i is right radius

      // If no letters was found
      if( min(r,i)!=MAXINT )
      {
        // What radius is less? Left?
        if( r <= i )
        {
          *End=Pos-r+1;
          *Start=FindStart(Str, *Start, *End);
        }
        else // Right!
        {
          *Start=Pos+i;
          *End=FindEnd(Str, *Start, *End);
        }
        ret=TRUE;
      }
    }
    else // Current character is letter!
    {
      *Start=FindStart(Str, *Start, Pos);
      *End=FindEnd(Str, Pos, *End);
      ret=TRUE;
    }
  }

  if(!ret)
    *Start=*End=-1;

  return ret;
}

int FindStart(const char *Str, int Start, int End)
{
  // Current pos in Str
  int CurPos = End-1;

  // While current character is letter
  while( CurPos>=Start && MyIsAlpha(Str[CurPos]) )
    CurPos--; // Moving to left
  return CurPos+1;
}

int FindEnd(const char *Str, int CurPos, int End)
{
  // While current character is letter
  while( CurPos<End && MyIsAlpha(Str[CurPos]))
    CurPos++; // Moving to right
  return CurPos;
}

int Config(void)
{
  static const struct InitDialogItem PreDialogItems[] =
  {
    {DI_DOUBLEBOX  ,3  ,1  ,60 ,9  ,0               ,MTitle},
    {DI_CHECKBOX   ,5  ,2  ,0  ,0  ,0               ,MShowInPanels},
    {DI_CHECKBOX   ,5  ,3  ,0  ,0  ,0               ,MShowInViewer},
    {DI_TEXT       ,4  ,4  ,0  ,0  ,DIF_SEPARATOR   ,-1},
    {DI_CHECKBOX   ,5  ,5  ,0  ,0  ,0               ,MShowInDisks},
    {DI_TEXT       ,9  ,6  ,0  ,0  ,0               ,MDisksHotkey},
    {DI_FIXEDIT    ,7  ,6  ,7  ,0  ,DIF_MASKEDIT    ,-1},
    {DI_TEXT       ,4  ,7  ,0  ,0  ,DIF_SEPARATOR   ,-1},
    {DI_BUTTON     ,0  ,8  ,0  ,0  ,DIF_CENTERGROUP ,MOk},
    {DI_BUTTON     ,0  ,8  ,0  ,0  ,DIF_CENTERGROUP ,MCancel},
  };
  struct FarDialogItem DialogItems[sizeof(PreDialogItems)/sizeof(PreDialogItems[0])];
  static const char Mask[] = "9";

  InitDialogItems(PreDialogItems,DialogItems,sizeof(PreDialogItems)/sizeof(PreDialogItems[0]));
  DialogItems[1].Focus = TRUE;
  DialogItems[1].Selected = ShowInPanels;
  DialogItems[2].Selected = ShowInViewer;
  DialogItems[4].Selected = ShowInDisks;
  DialogItems[8].DefaultButton = 1;
  DialogItems[6].Mask = Mask;
  FSF.sprintf(DialogItems[6].Data,"%i",DisksHotkey);

  if (Info.Dialog(Info.ModuleNumber,-1,-1,64,11,"Configure",(struct FarDialogItem *)&DialogItems,sizeof(PreDialogItems)/sizeof(PreDialogItems[0])) != 8)
    return FALSE;

  ShowInPanels = DialogItems[1].Selected;
  ShowInViewer = DialogItems[2].Selected;
  ShowInDisks  = DialogItems[4].Selected;
  FSF.sscanf(DialogItems[6].Data,"%i",&DisksHotkey);

  SetRegKey("ShowInPanels",ShowInPanels);
  SetRegKey("ShowInViewer",ShowInViewer);
  SetRegKey("ShowInDisks",ShowInDisks);
  SetRegKey("DisksHotkey",DisksHotkey);
  return TRUE;
}

int WINAPI _export Configure(int ItemNumber)
{
  (void) ItemNumber;
  struct FarMenuItem menu[3];
  memset(menu,0,sizeof(menu));
  lstrcpy(menu[0].Text,GetMsg(MTitle));
  menu[1].Separator=1;
  lstrcpy(menu[2].Text,GetMsg(MPluginConfig));
  int MenuCode=0;
  while (MenuCode>=0)
  {
    MenuCode = Info.Menu(Info.ModuleNumber,-1,-1,0,
                  FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT,
                  GetMsg(MConfig),NULL,NULL,NULL,NULL,
                  menu,sizeof(menu)/sizeof(menu[0]));
    if (MenuCode==0)
      Config();
    else if (MenuCode==2)
      pm->ShowConfigMenu();
  }
  return TRUE;
}

int WINAPI _export ProcessEditorEvent(int Event, void *Param)
{

  if (Event==EE_CLOSE)
  {
    AHTrackInfo ti;
    ti.EditorID = *((int *)Param);
    Tree->Delete(&ti);
  }
  return 0;
}

void WINAPI _export ExitFAR()
{
  if (pm)
    delete pm;
  pm=NULL;
  if (Tree)
    delete Tree;
  Tree=NULL;
}
