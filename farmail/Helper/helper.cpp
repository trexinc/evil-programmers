/*
    Helper sub-plugin for FARMail
    Copyright (C) 2002-2004 FARMail Group

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
#include "plugin.hpp"
#include "../FARMail/fmp.hpp"
#include "helper.hpp"
#include "memory.h"

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

static char strings[4][512];
static char defstrings[4][50];

static const char *win_unknown="Windows";
static const char *win_95="Win95";
static const char *win_95osr2="Win95 OSR2";
static const char *win_98="Win98";
static const char *win_98se="Win98 SE";
static const char *win_me="WinME";
static const char *win_nt="WinNT";
static const char *win_2k="Win2K";
static const char *win_xp="WinXP";
static const char *win_32s="Win32s";
const char *NULLSTR="";

struct PluginStartupInfo FarInfo;
FARSTANDARDFUNCTIONS FSF;
struct MailPluginStartupInfo FarMailInfo;
char PluginRootKey[80];

char msg1[128];

static char last_path[MAX_PATH];
static int last_eid;

char *GetMsg(int MsgNum,char *Str)
{
  FarMailInfo.GetMsg(FarMailInfo.MessageName,MsgNum,Str);
  return Str;
}

void ReadRegistry(void)
{
  char name[10];
  for(size_t i=0;i<sizeofa(strings);i++)
  {
    FSF.sprintf(name,"value%d",i+1);
    GetRegKey2( HKEY_CURRENT_USER, PluginRootKey, NULLSTR, name, strings[i], defstrings[i], 512 );
  }
}

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct MailPluginStartupInfo *FarMailInfo)
{
  ::FarInfo=*FarInfo;
  FSF=*FarInfo->FSF;
  ::FarMailInfo=*FarMailInfo;
  FSF.sprintf(PluginRootKey,"%s\\Helper",::FarMailInfo.RootKey);
  lstrcpy(defstrings[0],"------------- system clipboard -------->8----");
  lstrcpy(defstrings[1],"----8<---- end of system clipboard ----------");
  lstrcpy(defstrings[2],"----------------- file: %s ---->8----");
  lstrcpy(defstrings[3],"----8<---- end of file: %s ----------");
  ReadRegistry();
  *last_path=0;
  last_eid=-1;
  return 0;
}

void WINAPI _export Exit(void)
{

}

static void insert_file(char *filename)
{
  HANDLE f=CreateFile(filename,GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
  if(f!=INVALID_HANDLE_VALUE)
  {
    unsigned long size=GetFileSize(f,NULL);
    if(size<0x100000)
    {
      unsigned char *buffer=(unsigned char *)malloc(size+1);
      if(buffer)
      {
        buffer[size]=0;
        unsigned long readed;
        if(ReadFile(f,buffer,size,&readed,NULL))
        {
          int table=FarInfo.CharTable(FCT_DETECT,(char *)buffer,size);
          if(table>-1)
          {
            CharTableSet cts;
            if(FarInfo.CharTable(table,(char *)&cts,sizeof(cts))==table)
            {
              for(unsigned int i=0;i<size;i++)
                buffer[i]=cts.DecodeTable[buffer[i]];
            }
          }
          int n=get_current_line();
          char separator[76]; char trunc[MAX_PATH];
          lstrcpy(trunc,filename);
          FSF.TruncPathStr(trunc,40);
          FSF.sprintf(separator,strings[2],trunc);
          insert_string(separator,n);

          unsigned char *ptr1,*ptr2,*new_ptr,*string=buffer;
          while(true)
          {
            ptr1=(unsigned char *)strchr((char *)string,'\n');
            ptr2=(unsigned char *)strchr((char *)string,'\r');
            if(!ptr1) ptr1=ptr2;
            if(!ptr2) ptr2=ptr1;
            if(ptr2<ptr1) ptr1=ptr2;
            new_ptr=ptr1;
            if(ptr1)
            {
              new_ptr++;
              if((*new_ptr!=*ptr1)&&((*new_ptr=='\n')||(*new_ptr=='\r')))
                new_ptr++;
              *ptr1=0;
            }
            n++;
            insert_string((char *)string,n);
            if(!ptr1) break;
            string=new_ptr;
          }
          n++;
          FSF.sprintf(separator,strings[3],trunc);
          insert_string(separator,n);
        }
        free(buffer);
      }
    }
    CloseHandle(f);
  }
}

struct InitDialogItem
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  int Selected;
  unsigned int Flags;
  int DefaultButton;
  const char *Data;
};

static void InitDialogItems(InitDialogItem *Init,FarDialogItem *Item,int ItemsNumber)
{
  for (int i=0;i<ItemsNumber;i++)
  {
    Item[i].Type=Init[i].Type;
    Item[i].X1=Init[i].X1;
    Item[i].Y1=Init[i].Y1;
    Item[i].X2=Init[i].X2;
    Item[i].Y2=Init[i].Y2;
    Item[i].Focus=Init[i].Focus;
    Item[i].Selected=Init[i].Selected;
    Item[i].Flags=Init[i].Flags;
    Item[i].DefaultButton=Init[i].DefaultButton;
    if((unsigned)Init[i].Data<2000)
      GetMsg((unsigned int)Init[i].Data,Item[i].Data);
    else
      lstrcpy(Item[i].Data,Init[i].Data);
  }
}

static void ConfigMain(void)
{
  //Show dialog
  /*
    0000000000111111111122222222223333333333444444444455555555556666666666777777
    0123456789012345678901234567890123456789012345678901234567890123456789012345
  00                                                                            00
  01   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»   01
  02   º Clipboard start:                                                   º   02
  03   º                                                                    º   03
  04   º Clipboard end:                                                     º   04
  05   º                                                                    º   05
  06   º File start:                                                        º   06
  07   º                                                                    º   07
  08   º File end:                                                          º   08
  09   º                                                                    º   09
  10   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   10
  11   º                 [ OK ]                [ Cancel ]                   º   11
  12   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   12
  13                                                                            13
    0000000000111111111122222222223333333333444444444455555555556666666666777777
    0123456789012345678901234567890123456789012345678901234567890123456789012345
  */
  static struct InitDialogItem InitItems[]=
  {
  /* 0*/  {DI_DOUBLEBOX,3,1,72,12,0,0,0,0,(char *)mName},
  /* 1*/  {DI_TEXT,5,2,0,0,0,0,0,0,(char *)mClipStart},
  /* 2*/  {DI_EDIT,5,3,70,0,1,0,0,0,NULLSTR},
  /* 3*/  {DI_TEXT,5,4,0,0,0,0,0,0,(char *)mClipEnd},
  /* 4*/  {DI_EDIT,5,5,70,0,1,0,0,0,NULLSTR},
  /* 5*/  {DI_TEXT,5,6,0,0,0,0,0,0,(char *)mFileStart},
  /* 6*/  {DI_EDIT,5,7,70,0,1,0,0,0,NULLSTR},
  /* 7*/  {DI_TEXT,5,8,0,0,0,0,0,0,(char *)mFileEnd},
  /* 8*/  {DI_EDIT,5,9,70,0,1,0,0,0,NULLSTR},
  /* 9*/  {DI_TEXT,-1,10,0,0,0,0,DIF_SEPARATOR,0,NULLSTR},
  /*10*/  {DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,1,(char *)mOk},
  /*11*/  {DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,0,(char *)mCancel}
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  ReadRegistry();
  for(size_t i=0;i<sizeofa(strings);i++)
    lstrcpy(DialogItems[i*2+2].Data,strings[i]);
  int DlgCode=FarInfo.DialogEx(FarInfo.ModuleNumber,-1,-1,76,14,"Config",DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,FarMailInfo.ShowHelpDlgProc,(long)FarMailInfo.ModuleName);
  if(DlgCode==10)
  {
    HKEY hRoot = HKEY_CURRENT_USER;
    char name[10];
    for(size_t i=0;i<sizeofa(strings);i++)
    {
      lstrcpy(strings[i],DialogItems[i*2+2].Data);
      FSF.sprintf(name,"value%d",i+1);
      SetRegKey2( hRoot, PluginRootKey, NULLSTR, name, strings[i] );
    }
  }
}

int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData)
{
  (void)InData;
  switch(Msg)
  {
    case FMMSG_GETINFO:
      {
        GetInfoOutData *data=(GetInfoOutData *)OutData;
        GetMsg(mName,data->MenuString);
        lstrcpy(data->HotkeyID,"generic_helper");
        data->Flags=FMMSG_MENU|FMMSG_CONFIG;
      }
      return TRUE;
    case FMMSG_MENU:
      {
        int Msgs[]={mAttach,mInsert,mPaste,mFARVer,mMailVer,mWinVer};
        char buffer[1024];
        FarMenuItem items[sizeofa(Msgs)];
        for(size_t i=0;i<sizeofa(Msgs);i++)
        {
          FSF.sprintf(items[i].Text,"%c. %s",i+'1',GetMsg(Msgs[i],msg1));
          items[i].Selected=FALSE;
          items[i].Checked=FALSE;
          items[i].Separator=FALSE;
        }
        items[0].Selected=TRUE;
        int MenuCode=FarMailInfo.ShowHelpMenu(FarMailInfo.ModuleName,FarInfo.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,GetMsg(mName,msg1),NULL,"Usage",NULL,NULL,items,sizeofa(items));
        switch(MenuCode)
        {
          case 0:
          case 1:
            {
              EditorInfo ei;
              char path[MAX_PATH],filename[MAX_PATH];
              FarInfo.EditorControl(ECTL_GETINFO,&ei);
              if(ei.EditorID==last_eid)
                lstrcpy(path,last_path);
              else
              {
                path[0]=0;
                if(GetCurrentDirectory(sizeof(path),path)) FSF.AddEndSlash(path);
              }
              if(open_file_dialog(path,filename))
              {
                last_eid=ei.EditorID;
                lstrcpy(last_path,filename);
                *(FSF.PointToName(last_path))=0;
                if(MenuCode)
                {
                  insert_file(filename);
                }
                else
                {
                  char attach[1024],x_start[101],x_end[101]; EditorInfo ei;
                  if(FarMailInfo.GetString(FMSTR_START,x_start,sizeof(x_start),NULL)&&FarMailInfo.GetString(FMSTR_END,x_end,sizeof(x_end),NULL))
                  {
                    //save position
                    FarInfo.EditorControl(ECTL_GETINFO,&ei);

                    lstrcpy(attach,"attach ");
                    lstrcat(attach,filename);

                    const char *str;
                    int curline=0,ident=0;
                    bool processed=false;
                    while(get_line(curline,&str,NULL)) //FIXME
                    {
                      lstrcpyn(buffer,str,sizeof(buffer));
                      FSF.Trim(buffer);
                      if(curline==0&&FSF.LStrnicmp(buffer,x_start,lstrlen(x_start)))
                        break;
                      else
                      {
                        if(!FSF.LStrnicmp(buffer,x_end,lstrlen(x_end)))
                        {
                          insert_string(NULLSTR,curline);
                          while(ident)
                          {
                            FarInfo.EditorControl(ECTL_INSERTTEXT,(void *)" ");
                            ident--;
                          }
                          FarInfo.EditorControl(ECTL_INSERTTEXT,attach);
                          if(curline<=ei.CurLine) ei.CurLine++;
                          processed=true; break;
                        }
                        {
                          int i=0;
                          ident=0;
                          while(str[i]==' '||str[i]=='\t') { ident++; i++; }
                        }
                      }
                      curline++;
                    }
                    { //restore position
                      EditorSetPosition esp;

                      esp.CurLine=ei.CurLine;
                      esp.CurPos=ei.CurPos;
                      esp.TopScreenLine=ei.TopScreenLine;
                      esp.LeftPos=ei.LeftPos;
                      esp.CurTabPos=-1;
                      esp.Overtype=-1;
                      FarInfo.EditorControl(ECTL_SETPOSITION,&esp);
                    }
                    if(!processed) FarInfo.EditorControl(ECTL_INSERTTEXT,attach);
                  }
                }
              }
            }
            break;
          case 2:
            {
              char *clip=FSF.PasteFromClipboard();
              if(clip)
              {
                if(strchr(clip,'\r'))
                {
                  int n=get_current_line();
                  insert_string(strings[0],n);
                  insert_string(strings[1],n+1);
                  if(clip[lstrlen(clip)-1]!='\n')
                    insert_string(NULLSTR,n+1);
                  set_current_line(n+1,0);
                }
                FSF.DeleteBuffer(clip);
              }
            }
            break;
          case 3:
            {
              DWORD ver;
              FarInfo.AdvControl(FarInfo.ModuleNumber,ACTL_GETFARVERSION,&ver);
              FSF.sprintf(buffer,"FAR %d.%d build %d",HIBYTE(ver),LOBYTE(ver),HIWORD(ver));
              FarInfo.EditorControl(ECTL_INSERTTEXT,buffer);
            }
            break;
          case 4:
            FSF.sprintf(buffer,"FARMail %d.%d build %d",FarMailInfo.GetValue(FMVAL_VERSION_MAJOR),FarMailInfo.GetValue(FMVAL_VERSION_MINOR),FarMailInfo.GetValue(FMVAL_VERSION_BUILD));
            FarInfo.EditorControl(ECTL_INSERTTEXT,buffer);
            break;
          case 5:
            {
              OSVERSIONINFO ver;
              ver.dwOSVersionInfoSize=sizeof(ver);
              if(GetVersionEx(&ver))
              {
                const char *name; DWORD build=ver.dwBuildNumber&0xFFFF;
                char format[30] = "%s %d.%d build %d";
                if (*ver.szCSDVersion)
                  lstrcat(format," %s");
                name=win_unknown;
                if(ver.dwPlatformId&VER_PLATFORM_WIN32_NT)
                {
                  switch (ver.dwMajorVersion)
                  {
                    case 3:
                    case 4:
                      name=win_nt;
                      break;
                    case 5:
                      switch (ver.dwMinorVersion)
                      {
                        case 0:
                          name=win_2k;
                          break;
                        case 1:
                          name=win_xp;
                          break;
                      }
                      break;
                  }
                  build=ver.dwBuildNumber;
                }
                else if(ver.dwPlatformId&VER_PLATFORM_WIN32_WINDOWS)
                {
                  if (ver.dwMajorVersion==4)
                  {
                    switch (ver.dwMinorVersion)
                    {
                      case 0:
                        if (!lstrcmp(ver.szCSDVersion,"C"))
                          name=win_95osr2;
                        else
                          name=win_95;
                        break;
                      case 10:
                        if (!lstrcmp(ver.szCSDVersion,"A"))
                          name=win_98se;
                        else
                          name=win_98;
                        break;
                      case 90:
                        name=win_me;
                        break;
                    }
                  }
                }
                else
                  name=win_32s;
                FSF.sprintf(buffer,format,name,ver.dwMajorVersion,ver.dwMinorVersion,build,ver.szCSDVersion);
                FarInfo.EditorControl(ECTL_INSERTTEXT,buffer);
              }
            }
            break;
        }
      }
      return TRUE;
    case FMMSG_CONFIG:
      ConfigMain();
      return TRUE;
  }
  return FALSE;
}
