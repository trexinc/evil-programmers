/*
    CharacterMap plugin for FAR Manager
    Copyright (C) 2001-2006 Alex Yaroslavsky

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
#include "farcolor.hpp"
#include "farkeys.hpp"
#include "crt.hpp"
#include "common.hpp"
#include "memory.hpp"
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
#endif

struct PluginStartupInfo Info;
static FARSTANDARDFUNCTIONS FSF;
char PluginRootKey[80];
static const char reg[] = "EditorAutoPos\0CmdAutoPos\0CopyToClipboard\0EncodeBeforeInsertion\0LastCoordX\0LastCoordY\0Preload";

enum
{
  REG_EDITORAUTOPOS         = 0,
  REG_CMDAUTOPOS            = 14,
  REG_COPYTOCLIPBOARD       = 25,
  REG_ENCODEBEFOREINSERTION = 41,
  REG_LASTCOORDX            = 63,
  REG_LASTCOORDY            = 74,
  REG_PRELOAD               = 85,
};

#define OPEN_MANUAL 255

enum
{
  OPENF_COMMANDLINE,
  OPENF_EDITOR,
  OPENF_VIEWER,
  OPENF_MANUAL,
  OPENF_MAX,
};

int EditorAutoPos,
    CmdAutoPos,
    CopyToClipboard,
    EncodeBeforeInsertion,
    Preload;

enum
{
  MTitle,
  MDosText,
  MWinText,
  MTableSelect,
  MInfoFormat,
  MEditorAutoPos,
  MCmdAutoPos,
  MCopyToClipboard,
  MEncodeBeforeInsertion,
  MEnterFileName,
  MEnterTableInfo,
  MTableName,
  MStatusTableName,
  MRFCCharset,
  MUnavailable,
  MEditModeHex,
  MEditModeDec,
  MEditModeChr,
  MSaveToFile,
  MSureToExit,
  MMemoryError,
  MFileError,
  MOk,
  MCancel,
  MSeparator,
};

enum
{
  DMCM_REDRAW = DM_USER+1,
  DMCM_SETEDIT,
  DMCM_ENTER,
  DMCM_TABLESELECT,
  DMCM_BS,
  DMCM_DEL,
  DMCM_SHOWTABLEINFO,
  DMCM_EDITTABLE,
};

static const char s32[] = "%-32.32s";
static const char x2[] = "%02X";
static const char d3[] = "%03d";
static const char c1[] = "%01c";

struct DialogData
{
  int OpenFrom;
  unsigned char *Str;
  int *Ret;
};

struct TableStruct
{
  char RFCName[128];
  char ShortName[128];
  struct CharTableSet cts;
};

int Config(void);

BOOL ReadLine(HANDLE file, char *buffer, DWORD len)
{
  DWORD CurrPos=SetFilePointer(file,0,NULL,FILE_CURRENT), transferred;
  BOOL res=TRUE;
  if(ReadFile(file,buffer,len-1,&transferred,NULL)&&transferred)
  {
    DWORD len=0; char *ptr; size_t i;
    buffer[len-1]=0;
    ptr=buffer;
    for(i=0;i<transferred;i++,len++,ptr++)
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
    res=FALSE;
  }
  return res;
}

void Warning(int Message)
{
  const char *Msg[2];
  Msg[0] = GetMsg(MTitle);
  Msg[1] = GetMsg(Message);
  Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_MB_OK,NULL,Msg,2,0);
}

void SetVBufAttrib(CHAR_INFO *VBuf, WORD Attrib, int i)
{
  for (int j=0; j<i; j++)
    VBuf[j].Attributes = Attrib;
}

void SetVBufChars(CHAR_INFO *VBuf, unsigned char *Str, int i)
{
  for (int j=0; j<i; j++)
    VBuf[j].Char.AsciiChar = Str[j];
}

void CopyVBufSmart(CHAR_INFO *VBufOut, CHAR_INFO *VBufIn, int i)
{
  memcpy(VBufOut,VBufIn,i*sizeof(CHAR_INFO));
  for (int j=0; j<i; j++)
    if (VBufOut[j].Char.AsciiChar==0)
      VBufOut[j].Char.AsciiChar = 32;
}

int GetCharTable(int i, struct CharTableSet *cts)
{
  if (i>1)
    return Info.CharTable(i-2,(char *)cts,sizeof(struct CharTableSet));
  else
  {
    for (int j=0; j<256; j++)
      cts->DecodeTable[j] = cts->EncodeTable[j] = (unsigned char) j;
    if (i == 1)
    {
      OemToCharBuff((const CHAR *)cts->DecodeTable,(CHAR *)cts->EncodeTable,256);
      CharToOemBuff((const CHAR *)cts->DecodeTable,(CHAR *)cts->DecodeTable,256);
      lstrcpy(cts->TableName,GetMsg(MWinText));
    }
    else
      lstrcpy(cts->TableName,GetMsg(MDosText));
  }
  return i;
}

void Encode(char *Text, int TextLength, struct CharTableSet *From, int TargetNum)
{
  struct CharTableSet To;
  GetCharTable(TargetNum, &To);
  for (int i=0; i<TextLength; i++)
    Text[i] = To.EncodeTable[From->DecodeTable[Text[i]]];
}

void Decode(char *Text, int TextLength, struct CharTableSet *From)
{
  for (int i=0; i<TextLength; i++)
    Text[i] = From->DecodeTable[Text[i]];
}

void LoadFile(const char *filename, HANDLE hDlg, struct CharTableSet *newcts)
{
  HANDLE fp = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
  if (fp!=INVALID_HANDLE_VALUE)
  {
    Info.SendDlgMessage(hDlg,DM_LISTDELETE,8,(long)NULL);
    struct FarListItem li;
    struct FarList fl;
    fl.ItemsNumber=1;
    fl.Items=&li;
    memset(&li,0,sizeof(li));
    char buf[1000];
    while (ReadLine(fp,buf,1000))
    {
      lstrcpyn(li.Text,buf,128);
      Decode(li.Text,lstrlen(li.Text),newcts);
      Info.SendDlgMessage(hDlg,DM_LISTADD,8,(long)&fl);
    }
    CloseHandle(fp);
  }
}

long WINAPI MyDialog(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  static int POpenFrom;
  static unsigned char *Str;
  static int *Ret;
  static COORD coord;
  static int LastSelectedMenu;
  static struct FarMenuItem *MenuItems;
  static int TotalMenuItems;
  static int TargetTable;
  static struct CharTableSet cts;
  static struct TableStruct newtable;
  static struct FarDialogItem DlgASCII, DlgText, DlgEdit, DlgInfo;
  static CHAR_INFO VBufASCII[256];
  static const unsigned char *InfoFormat;
  static BOOL EditMode;
  static int EditInsertMode;
  static char CharTxt[4];
  static int CharTxtPos;
  static char example[MAX_PATH];
  unsigned char Temp[51];
  static int EditLen;
  static unsigned char EditText[512];
  int tmplen;
  unsigned char tmptext[512];
  char *Cmd;
  static struct EditorInfo ei;
  struct FarList *fl;
  struct EditorConvertText ect;
  struct EditorGetString egs;
  struct EditorSetString ess;
  struct EditorSetPosition esp;
  WORD Color;
  int i;

  switch (Msg)
  {
    case DN_INITDIALOG:
      EditMode = FALSE;
      EditLen = 0;
      POpenFrom = ((struct DialogData *)Param2)->OpenFrom;
      Str = ((struct DialogData *)Param2)->Str;
      Ret = ((struct DialogData *)Param2)->Ret;
      MenuItems = NULL;
      GetRegKey(&reg[REG_LASTCOORDX],(int *)&coord.X,0);
      GetRegKey(&reg[REG_LASTCOORDY],(int *)&coord.Y,0);
      for (i=0; GetCharTable(i,&cts) != -1; i++)
      {
        MenuItems=(struct FarMenuItem *)realloc(MenuItems,(i+1)*sizeof(struct FarMenuItem));
        if (!MenuItems)
        {
          Warning(MMemoryError);
          Info.SendDlgMessage(hDlg,DM_CLOSE,0,0);
          return FALSE;
        }
        lstrcpy(MenuItems[i].Text,cts.TableName);
        MenuItems[i].Checked = MenuItems[i].Selected = MenuItems[i].Separator = 0;
      }
      TotalMenuItems = i;
      MenuItems[LastSelectedMenu=0].Selected = 1;
      Info.SendDlgMessage(hDlg,DM_SETCURSORSIZE,3,MAKELONG(1,99));
      InfoFormat = (const unsigned char *) GetMsg(MInfoFormat);
      Info.SendDlgMessage(hDlg,DM_GETDLGITEM,3,(long)&DlgASCII);
      Info.SendDlgMessage(hDlg,DM_GETDLGITEM,1,(long)&DlgText);
      Info.SendDlgMessage(hDlg,DM_GETDLGITEM,5,(long)&DlgEdit);
      Info.SendDlgMessage(hDlg,DM_GETDLGITEM,6,(long)&DlgInfo);
      Color = Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_DIALOGTEXT);
      Color |= Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_DIALOGBOX);
      SetVBufAttrib(VBufASCII,Color,256);
      SetVBufAttrib(DlgInfo.VBuf,Color,32);
      {
        struct FarDialogItem DlgDemo;
        char Demo[19*17+1];
        char Space[18];
        Info.SendDlgMessage(hDlg,DM_GETDLGITEM,11,(long)&DlgDemo);
        SetVBufAttrib(DlgDemo.VBuf,Color,19*17);
        FSF.sprintf(Space,"%17s"," ");
        lstrcpy(Demo,"   0123456789ABCDEF");
        for (i=0; i<256; i+=16)
        {
          char Hex[3];
          FSF.sprintf(Hex,x2,i);
          lstrcat(Demo,Hex);
          lstrcat(Demo,Space);
        }
        Demo[19+3] = 32;
        for (i=1; i<256; i++)
        {
          Demo[19+i/16*19+i%16+3] = i;
        }
        SetVBufChars(DlgDemo.VBuf,(unsigned char *)Demo,19*17);
        Info.SendDlgMessage(hDlg,DM_SETDLGITEM,11,(long)&DlgDemo);
      }
      Info.SendDlgMessage(hDlg,DMCM_TABLESELECT,1,0); //Select DOS
      TargetTable = 0; //DOS
      if (POpenFrom == OPENF_EDITOR)
      {
        Info.EditorControl(ECTL_GETINFO,(void *)&ei);
        switch (ei.TableNum)
        {
          case -1:
            if (ei.AnsiMode == 1)
            {
              TargetTable = 1; //WIN
              Info.SendDlgMessage(hDlg,DMCM_TABLESELECT,1,1); //select WIN
            }
            break;

          default:
            TargetTable = ei.TableNum+2;
            FSF.sprintf(DlgText.Data,s32,MenuItems[ei.TableNum+2].Text);
            MenuItems[LastSelectedMenu].Selected = 0;
            MenuItems[LastSelectedMenu=(ei.TableNum+2)].Selected = 1;
            Info.CharTable(ei.TableNum,(char *)&cts,sizeof(struct CharTableSet));
            Info.SendDlgMessage(hDlg,DM_SETDLGITEM,1,(long)&DlgText);
        }
        if (EditorAutoPos)
        {
          egs.StringNumber = -1;
          Info.EditorControl(ECTL_GETSTRING,(void *)&egs);
          if (egs.StringLength > 0 && ei.CurPos < egs.StringLength)
          {
            i = egs.StringText[ei.CurPos];
            coord.X = i%32;
            coord.Y = i/32;
          }
        }
      }
      if (CmdAutoPos && POpenFrom == OPENF_COMMANDLINE)
      {
        Info.Control(INVALID_HANDLE_VALUE,FCTL_GETCMDLINEPOS,(void *)&i);
        if (i < 1024)
        {
          Cmd = (char *)malloc(1024*sizeof(char));
          if (!Cmd)
          {
            Warning(MMemoryError);
            Info.SendDlgMessage(hDlg,DM_CLOSE,0,0);
            return FALSE;
          }
          Info.Control(INVALID_HANDLE_VALUE,FCTL_GETCMDLINE,(void *)Cmd);
          i = cts.EncodeTable[Cmd[i]];
          coord.X = i%32;
          coord.Y = i/32;
          free(Cmd);
        }
      }
      if (POpenFrom == OPENF_MANUAL)
      {
        i = cts.EncodeTable[*Str];
        coord.X = i%32;
        coord.Y = i/32;
      }
      Info.SendDlgMessage(hDlg,DMCM_REDRAW,1,1);
      return TRUE;

    case DN_KEY:
    {
      long rParam2 = Param2;
      if (Param1==8)
        if (!(Param2==KEY_ESC || Param2==KEY_F10))
          return FALSE;
      if ((Param2&KEY_CTRL)&&((Param2&0xFFL)>='a')&&((Param2&0xFFL)<='z'))
        rParam2&=~0x20L;

      switch (rParam2)
      {
        case KEY_LEFT: (coord.X>0)?--coord.X:((coord.Y>0)?(coord.X=31,--coord.Y):coord.X); break;

        case KEY_RIGHT: (coord.X<31)?++coord.X:((coord.Y<7)?(coord.X=0,++coord.Y):coord.X); break;

        case KEY_UP: if (coord.Y > 0) --coord.Y; break;

        case KEY_DOWN: if (coord.Y < 7) ++coord.Y; break;

        case KEY_HOME: coord.X = 0; break;

        case KEY_END: coord.X = 31; break;

        case KEY_PGUP: coord.Y = 0; break;

        case KEY_PGDN: coord.Y = 7; break;

        case KEY_CTRLHOME: coord.Y = coord.X = 0; break;

        case KEY_CTRLEND: coord.Y = 7; coord.X = 31; break;

        case KEY_ENTER:
          if (!EditMode)
            Info.SendDlgMessage(hDlg,DMCM_ENTER,0,0);
          return TRUE;

        case KEY_SHIFTF8:
          if (!EditMode)
            Info.SendDlgMessage(hDlg,DMCM_TABLESELECT,0,0);
          return TRUE;

        case KEY_F8:
          if (!EditMode)
            Info.SendDlgMessage(hDlg,DMCM_TABLESELECT,2,0); //exchange DOS/WIN
          return TRUE;

        case KEY_SHIFTENTER:
        case KEY_INS:
        case KEY_ADD:
        case KEY_SUBTRACT:
          if (!EditMode && Info.SendDlgMessage(hDlg,DMCM_SETEDIT,0,0))
          {
            switch (Param2)
            {
              case KEY_ADD: (coord.X<31)?++coord.X:((coord.Y<7)?(coord.X=0,++coord.Y):coord.X); break;
              case KEY_SUBTRACT: (coord.X>0)?--coord.X:((coord.Y>0)?(coord.X=31,--coord.Y):coord.X); break;
            }
            break;
          }
          return TRUE;

        case KEY_DEL:
          if (!EditMode)
            Info.SendDlgMessage(hDlg,DMCM_DEL,0,0);
          return TRUE;
        case KEY_BS:
          if (!EditMode)
            Info.SendDlgMessage(hDlg,DMCM_BS,0,0);
          return TRUE;

        case KEY_CTRLINS:
        case KEY_CTRLC:
          if (!EditMode)
          {
            tmplen = EditLen;
            if (tmplen==0)
            {
              EditText[0] = coord.X+32*coord.Y;
              tmplen = 1;
            }
            EditText[tmplen] = 0;
            if (EncodeBeforeInsertion)
            {
              memcpy(tmptext,EditText,tmplen+1);
              Encode((char *)tmptext,tmplen,&cts,TargetTable);
              FSF.CopyToClipboard((const char *)tmptext);
            }
            else
              FSF.CopyToClipboard((const char *)EditText);
          }
          return TRUE;

        case KEY_ALTSHIFTF9:
          Info.SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
          Config();
          Info.SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
          return TRUE;

        case KEY_F9:
          if (!EditMode)
            Info.SendDlgMessage(hDlg,DMCM_TABLESELECT,3,1);
          break;
        case KEY_SHIFTF9:
          if (!EditMode)
            Info.SendDlgMessage(hDlg,DMCM_TABLESELECT,3,-1);
          break;

        case KEY_CTRLI:
          Info.SendDlgMessage(hDlg,DMCM_SHOWTABLEINFO,0,0);
          return TRUE;

        case KEY_CTRLN:
          if (!EditMode)
            Info.SendDlgMessage(hDlg,DMCM_EDITTABLE,1,0);
          return TRUE;

        case KEY_CTRLE:
          if (!EditMode)
          {
            Info.SendDlgMessage(hDlg,DMCM_EDITTABLE,0,0);
          }
          else
          {
            int xxx;
            const char *XXX;
            EditInsertMode = (EditInsertMode+1)%3;
            xxx=VBufASCII[coord.X+32*coord.Y].Char.AsciiChar;
            switch (EditInsertMode)
            {
              case 0: XXX = x2; break;
              case 1: XXX = d3; break;
              case 2: XXX = c1; break;
            }
            FSF.sprintf(CharTxt,XXX,xxx);
            CharTxtPos = 0;
            Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,7,(long)CharTxt);
            Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,9,(long)GetMsg(MEditModeHex+EditInsertMode));
          }
          return TRUE;

        case KEY_CTRLX:
          if (EditMode)
            Info.SendDlgMessage(hDlg,DMCM_EDITTABLE,0,1);
          return TRUE;

        case KEY_CTRLS:
          if (EditMode)
            Info.SendDlgMessage(hDlg,DMCM_EDITTABLE,1,1);
          return TRUE;

        case KEY_CTRLL:
          if (EditMode)
          {
            static struct InitDialogItem InitItems[]=
            {
              {DI_DOUBLEBOX,3,1,57,4,0,MTitle},
              {DI_TEXT,5,2,0,0,0,MEnterFileName},
              {DI_EDIT,5,3,55,0,0,-1},
              {DI_BUTTON,3,5,0,0,0,MOk},
            };
            struct FarDialogItem DialogItems[4];
            InitDialogItems(InitItems,DialogItems,4);
            DialogItems[3].DefaultButton = 1;
            if (Info.Dialog(Info.ModuleNumber,-1,-1,60,6,NULL,DialogItems,4)!=3)
              break;
            FSF.Unquote(DialogItems[2].Data);
            lstrcpy(example,DialogItems[2].Data);
            LoadFile(example,hDlg,&newtable.cts);
          }
          return TRUE;

        case KEY_CTRLU:
          if (EditMode)
          {
            *example = '\0';
            Info.SendDlgMessage(hDlg,DM_LISTDELETE,8,0);
          }
          return TRUE;

        default:
          if (EditMode)
          {
            BOOL ret=FALSE;
            if (Param2==KEY_ESC||Param2==KEY_F10)
            {
              if (Info.SendDlgMessage(hDlg,DMCM_EDITTABLE,0,1))
              {
                Info.SendDlgMessage(hDlg,DM_CLOSE,0,0);
              }
              return TRUE;
            }
            switch (EditInsertMode)
            {
              case 0: //Hex
                if((Param2>='0' && Param2<='9') || (Param2>='a' && Param2<='f') || (Param2>='A' && Param2<='F'))
                {
                  CharTxt[CharTxtPos++] = Param2;
                  CharTxt[CharTxtPos] = '\0';
                  if (CharTxtPos>1)
                    CharTxtPos=0;
                  FSF.sscanf(CharTxt,"%X",&i);
                  ret=TRUE;
                }
                break;
              case 1: //Decimal
                if((Param2>='0' && Param2<='9'))
                {
                  CharTxt[CharTxtPos++] = Param2;
                  CharTxt[CharTxtPos] = '\0';
                  FSF.sscanf(CharTxt,"%d",&i);
                  if (i>255)
                  {
                    CharTxt[CharTxtPos=0] = Param2;
                    CharTxt[++CharTxtPos] = '\0';
                  }
                  if (CharTxtPos>2)
                    CharTxtPos=0;
                  ret=TRUE;
                }
                break;
              case 2: //Char
                if((Param2>31 && Param2<256))
                {
                  CharTxt[CharTxtPos++] = Param2;
                  CharTxt[CharTxtPos] = '\0';
                  CharTxtPos=0;
                  FSF.sscanf(CharTxt,"%c",&i);
                  ret=TRUE;
                }
            }
            if (ret)
            {
              struct FarListPos lp;
              Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,7,(long)CharTxt);
              VBufASCII[coord.X+32*coord.Y].Char.AsciiChar = i;
              newtable.cts.DecodeTable[coord.X+32*coord.Y] = i;
              Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,8,(long)&lp);
              LoadFile(example,hDlg,&newtable.cts);
              Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,8,(long)&lp);
              CopyVBufSmart(DlgASCII.VBuf,VBufASCII,256);
              Info.SendDlgMessage(hDlg,DM_SETDLGITEM,3,(long)&DlgASCII);
              Info.SendDlgMessage(hDlg,DMCM_REDRAW,0,1);
              return TRUE;
            }
            return FALSE;
          }
          else
          {
            if (Param2<256 && Param2>31)
            {
              coord.X = Param2%32;
              coord.Y = Param2/32;
              break;
            }
            return FALSE;
          }
      }
      if (EditMode)
      {
        int xxx;
        const char *XXX;
        xxx=VBufASCII[coord.X+32*coord.Y].Char.AsciiChar;
        switch (EditInsertMode)
        {
          case 0: XXX = x2; break;
          case 1: XXX = d3; break;
          case 2: XXX = c1;
        }
        FSF.sprintf(CharTxt,XXX,xxx);
        CharTxtPos = 0;
        Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,7,(long)CharTxt);
      }
      Info.SendDlgMessage(hDlg,DMCM_REDRAW,0,1);
      return TRUE;
    }

    case DN_MOUSECLICK:
      switch (Param1)
      {
        case 3:
          coord = ((MOUSE_EVENT_RECORD *)Param2)->dwMousePosition;
          Info.SendDlgMessage(hDlg,DMCM_REDRAW,0,1);
          if (((MOUSE_EVENT_RECORD *)Param2)->dwButtonState == RIGHTMOST_BUTTON_PRESSED)
          {
            Info.SendDlgMessage(hDlg,DMCM_SETEDIT,0,0);
          }
          else
          {
            if (((MOUSE_EVENT_RECORD *)Param2)->dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
              if (((MOUSE_EVENT_RECORD *)Param2)->dwEventFlags & DOUBLE_CLICK)
                Info.SendDlgMessage(hDlg,DMCM_ENTER,0,0);
          }
          return TRUE;

        case 1:
          if (((MOUSE_EVENT_RECORD *)Param2)->dwButtonState == RIGHTMOST_BUTTON_PRESSED)
          {
            Info.SendDlgMessage(hDlg,DMCM_TABLESELECT,2,0);
          }
          else
          {
            if (((MOUSE_EVENT_RECORD *)Param2)->dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
            {
              Info.SendDlgMessage(hDlg,DMCM_TABLESELECT,0,0);
            }
          }
          return TRUE;

        case 5:
          if (((MOUSE_EVENT_RECORD *)Param2)->dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
          {
              Info.SendDlgMessage(hDlg,DMCM_BS,0,0);
          }
          else
          {
            if (((MOUSE_EVENT_RECORD *)Param2)->dwButtonState == RIGHTMOST_BUTTON_PRESSED)
            {
              Info.SendDlgMessage(hDlg,DMCM_DEL,0,0);
            }
          }
          return TRUE;
      }
      return FALSE;

    case DMCM_REDRAW:
      if (Param1)
      {
        SetVBufChars(VBufASCII,cts.DecodeTable,256);
        CopyVBufSmart(DlgASCII.VBuf,VBufASCII,256);
        Info.SendDlgMessage(hDlg,DM_SETDLGITEM,3,(long)&DlgASCII);
      }
      if (Param1 || Param2)
      {
        i = VBufASCII[coord.X+32*coord.Y].Char.AsciiChar;
        FSF.sprintf((char *)Temp,(const char *)InfoFormat,(unsigned char)(i==0?32:i),(unsigned char)i,(unsigned char)i,(int)(coord.X+32*coord.Y),(int)(coord.X+32*coord.Y));
        SetVBufChars(DlgInfo.VBuf,Temp,32);
        Info.SendDlgMessage(hDlg,DM_SETDLGITEM,6,(long)&DlgInfo);
        Info.SendDlgMessage(hDlg,DM_SETCURSORPOS,3,(long)&coord);
      }
      return TRUE;

    case DMCM_SETEDIT:
      if (EditLen < 511)
      {
        i = VBufASCII[coord.X+32*coord.Y].Char.AsciiChar;
        if (i == 0)
        {
          switch (POpenFrom)
          {
            case OPENF_EDITOR:
            case OPENF_VIEWER:
            case OPENF_MANUAL:
              DlgEdit.Data[EditLen] = 32;
              break;
            default:
              return FALSE;
          }
        }
        else
          DlgEdit.Data[EditLen] = (unsigned char) i;
        EditText[EditLen] = (unsigned char) (coord.X+32*coord.Y);
        DlgEdit.Data[++EditLen] = 0;
        Info.SendDlgMessage(hDlg,DM_SETDLGITEM,5,(long)&DlgEdit);
        return TRUE;
      }
      return FALSE;

    case DMCM_ENTER:
      if (EditLen==0)
      {
        i = VBufASCII[coord.X+32*coord.Y].Char.AsciiChar;
        if (i == 0 && POpenFrom == OPENF_COMMANDLINE)
        {
          //MessageBeep(0);
          return FALSE;
        }
        else
        {
          EditText[0] = (unsigned char) (coord.X+32*coord.Y);
          EditLen = 1;
        }
      }
      EditText[EditLen] = 0;
      if (EncodeBeforeInsertion)
        if (TargetTable != LastSelectedMenu)
          Encode((char *)EditText,EditLen,&cts,TargetTable);
      if (CopyToClipboard)
        FSF.CopyToClipboard((const char *)EditText);
      switch (POpenFrom)
      {
        case OPENF_COMMANDLINE:
          Info.Control(INVALID_HANDLE_VALUE,FCTL_INSERTCMDLINE,(void *)EditText);
          break;
        case OPENF_EDITOR:
          egs.StringNumber = -1;
          Info.EditorControl(ECTL_GETSTRING,(void *)&egs);
          if (ei.CurPos > egs.StringLength)
            ess.StringText = (char *)malloc((egs.StringLength+EditLen+(ei.CurPos-egs.StringLength))*sizeof(char));
          else
            ess.StringText = (char *)malloc((egs.StringLength+EditLen)*sizeof(char));
          if (!ess.StringText)
          {
            Warning(MMemoryError);
            return FALSE;
          }
          if (ei.CurPos > egs.StringLength)
          {
            for (i=egs.StringLength; i<ei.CurPos; i++)
              ess.StringText[i] = cts.EncodeTable[32];
            if (EncodeBeforeInsertion)
              Encode(&ess.StringText[egs.StringLength],ei.CurPos-egs.StringLength,&cts,TargetTable);
            memcpy(ess.StringText,egs.StringText,egs.StringLength);
            memcpy(ess.StringText+ei.CurPos,EditText,EditLen);
            ess.StringLength = EditLen + ei.CurPos;
          }
          else
          {
            memcpy(ess.StringText,egs.StringText,ei.CurPos);
            memcpy(ess.StringText+ei.CurPos,EditText,EditLen);
            memcpy(ess.StringText+ei.CurPos+EditLen,egs.StringText+ei.CurPos,egs.StringLength-ei.CurPos);
            ess.StringLength = EditLen + egs.StringLength;
          }
          ess.StringNumber = -1;
          ess.StringEOL = (char *)egs.StringEOL;
          Info.EditorControl(ECTL_SETSTRING,(void *)&ess);
          esp.CurPos = ei.CurPos + EditLen;
          esp.CurLine = esp.CurTabPos = esp.TopScreenLine = esp.LeftPos = esp.Overtype = -1;
          Info.EditorControl(ECTL_SETPOSITION,(void *)&esp);
          free(ess.StringText);
          break;
        case OPENF_VIEWER:
          break;
        case OPENF_MANUAL:
        {
          memcpy(Str,EditText,EditLen);
          *Ret=EditLen;
        }

      }
      Info.SendDlgMessage(hDlg,DM_CLOSE,0,0);
      return TRUE;

    case DMCM_TABLESELECT:
      if (EditLen > 0)
        return FALSE;
      switch (Param1)
      {
        case 0:
          i = Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT,GetMsg(MTableSelect),NULL,NULL,NULL,NULL,MenuItems,TotalMenuItems);
          break;
        case 1:
          i = Param2;
          break;
        case 2:
          i = (LastSelectedMenu == 1 ? 0 : 1);
          break;
        case 3:
          i = LastSelectedMenu + Param2;
          if (i<0)
            i = TotalMenuItems-1;
          if (i >= TotalMenuItems)
            i = 0;
          break;
      }
      if (i!=-1)
      {
        FSF.sprintf(DlgText.Data,s32,MenuItems[i].Text);
        MenuItems[LastSelectedMenu].Selected = 0;
        MenuItems[LastSelectedMenu=i].Selected = 1;
        GetCharTable(i,&cts);
        Info.SendDlgMessage(hDlg,DM_SETDLGITEM,1,(long)&DlgText);
        Info.SendDlgMessage(hDlg,DMCM_REDRAW,1,0);
        return TRUE;
      }
      return FALSE;

    case DMCM_DEL:
    case DMCM_BS:
      if (EditLen > 0)
      {
        if (Msg == DMCM_DEL)
          EditLen=1;
        DlgEdit.Data[--EditLen] = 0;
        Info.SendDlgMessage(hDlg,DM_SETDLGITEM,5,(long)&DlgEdit);
      }
      return TRUE;

    case DMCM_SHOWTABLEINFO:
      {
        struct InitDialogItem InitItems[]=
        {
          {DI_DOUBLEBOX,3,1,57,8,0,MTitle},
          {DI_TEXT,5,2,0,0,0,MTableName},
          {DI_EDIT,5,3,55,0,0,-1},
          {DI_TEXT,5,4,0,0,0,MStatusTableName},
          {DI_EDIT,5,5,55,0,0,-1},
          {DI_TEXT,5,6,0,0,0,MRFCCharset},
          {DI_EDIT,5,7,55,0,0,-1},
          {DI_BUTTON,3,9,0,0,0,MOk},
        };
        struct FarDialogItem DialogItems[8];
        if (Param1||EditMode)
          InitItems[0].Data=MEnterTableInfo;
        InitDialogItems(InitItems,DialogItems,8);
        DialogItems[7].DefaultButton = 1;
        lstrcpy(DialogItems[2].Data,EditMode?newtable.cts.TableName:(Param1?"":cts.TableName));
        if (!EditMode)
        {
          if (!Param1)
          {
            DialogItems[2].Flags|=DIF_READONLY;
            for (i=4; i<=6; i+=2)
            {
              DialogItems[i].Flags|=DIF_READONLY;
              lstrcpy(DialogItems[i].Data,GetMsg(MUnavailable));
            }
          }
        }
        else
        {
          lstrcpy(DialogItems[4].Data,newtable.ShortName);
          lstrcpy(DialogItems[6].Data,newtable.RFCName);
        }
        if (Info.Dialog(Info.ModuleNumber,-1,-1,60,10,NULL,DialogItems,8)!=7)
          return FALSE;
        if (EditMode||Param1)
        {
          lstrcpyn(newtable.cts.TableName,DialogItems[2].Data,128);
          lstrcpyn(newtable.ShortName,DialogItems[4].Data,128);
          lstrcpyn(newtable.RFCName,DialogItems[6].Data,128);
          FSF.sprintf(DlgText.Data,s32,*(newtable.cts.TableName)?newtable.cts.TableName:newtable.ShortName);
          Info.SendDlgMessage(hDlg,DM_SETDLGITEM,1,(long)&DlgText);
        }
      }
      return TRUE;

    case DMCM_EDITTABLE:
      if (Param2==0)
      {
        SMALL_RECT sr;
        COORD dcoord;
        if (Param1)
        {
          Info.SendDlgMessage(hDlg,DMCM_TABLESELECT,1,0);
          newtable.cts=cts;
          *newtable.ShortName='\0';
          *newtable.RFCName='\0';
          Info.SendDlgMessage(hDlg,DMCM_SHOWTABLEINFO,1,0);
        }
        else
        {
          newtable.cts=cts;
          *newtable.ShortName='\0';
          *newtable.RFCName='\0';
        }
        dcoord.X = 61; dcoord.Y=25;
        Info.SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(long)&dcoord);
        sr.Left = 3; sr.Top=1; sr.Right=36; sr.Bottom=23;
        Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,0,(long)&sr);
        Info.SendDlgMessage(hDlg,DM_SHOWITEM,5,0);
        Info.SendDlgMessage(hDlg,DM_SHOWITEM,8,1);
        Info.SendDlgMessage(hDlg,DM_SHOWITEM,7,1);
        Info.SendDlgMessage(hDlg,DM_SHOWITEM,9,1);
        Info.SendDlgMessage(hDlg,DM_SHOWITEM,10,1);
        Info.SendDlgMessage(hDlg,DM_SHOWITEM,11,1);
        *example = '\0';
        Info.SendDlgMessage(hDlg,DM_LISTDELETE,8,0);
        EditInsertMode = 0;
        i=VBufASCII[coord.X+32*coord.Y].Char.AsciiChar;
        FSF.sprintf(CharTxt,x2,i);
        CharTxtPos = 0;
        Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,7,(long)CharTxt);
        Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,9,(long)GetMsg(MEditModeHex+EditInsertMode));
        Info.SendDlgMessage(hDlg,DMCM_REDRAW,1,0);
        EditMode=TRUE;
      }
      else
      {
        if (Param1)
        {
          static struct InitDialogItem InitItems[]=
          {
            {DI_DOUBLEBOX,3,1,57,4,0,MTitle},
            {DI_TEXT,5,2,0,0,0,MSaveToFile},
            {DI_EDIT,5,3,55,0,0,-1},
            {DI_BUTTON,3,5,0,0,0,MOk},
          };
          struct FarDialogItem DialogItems[4];
          InitDialogItems(InitItems,DialogItems,4);
          DialogItems[3].DefaultButton = 1;
          lstrcpy(DialogItems[2].Data,*(newtable.cts.TableName)?newtable.cts.TableName:newtable.ShortName);
          lstrcat(DialogItems[2].Data,".reg");
          if (Info.Dialog(Info.ModuleNumber,-1,-1,60,6,NULL,DialogItems,4)==3)
          {
            HANDLE fp;
            DWORD transfered;
            char buf[256];
            FSF.Unquote(DialogItems[2].Data);
            fp = CreateFile(DialogItems[2].Data,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
            if (fp==INVALID_HANDLE_VALUE)
            {
              Warning(MFileError);
              return FALSE;
            }
            lstrcpy(buf,"REGEDIT4\r\n[HKEY_CURRENT_USER\\Software\\Far\\CodeTables\\");
            WriteFile(fp,buf,lstrlen(buf),&transfered,NULL);
            lstrcpy(buf,*(newtable.ShortName)?newtable.ShortName:newtable.cts.TableName);
            WriteFile(fp,buf,lstrlen(buf),&transfered,NULL);
            lstrcpy(buf,"]\r\n");
            WriteFile(fp,buf,lstrlen(buf),&transfered,NULL);
            if (*(newtable.cts.TableName)&&*(newtable.ShortName))
            {
              lstrcpy(buf,"\"TableName\"=\"");
              WriteFile(fp,buf,lstrlen(buf),&transfered,NULL);
              lstrcpy(buf,newtable.cts.TableName);
              WriteFile(fp,buf,lstrlen(buf),&transfered,NULL);
              lstrcpy(buf,"\"\r\n");
              WriteFile(fp,buf,lstrlen(buf),&transfered,NULL);
            }
            if (*(newtable.RFCName))
            {
              lstrcpy(buf,"\"RFCCharset\"=\"");
              WriteFile(fp,buf,lstrlen(buf),&transfered,NULL);
              lstrcpy(buf,newtable.RFCName);
              WriteFile(fp,buf,lstrlen(buf),&transfered,NULL);
              lstrcpy(buf,"\"\r\n");
              WriteFile(fp,buf,lstrlen(buf),&transfered,NULL);
            }
            lstrcpy(buf,"\"Mapping\"=hex:\\\r\n");
            WriteFile(fp,buf,lstrlen(buf),&transfered,NULL);
            for (i=0; i<16; i++)
            {
              int j;
              *buf = '\0';
              for (j=0; j<16; j++)
              {
                char hex[4];
                FSF.sprintf(hex,"%02X,",newtable.cts.DecodeTable[i*16+j]);
                lstrcat(buf,hex);
              }
              if ((i*16+j)<255)
                lstrcat(buf,"\\");
              else
                buf[lstrlen(buf)-1] = '\0';
              lstrcat(buf,"\r\n");
              WriteFile(fp,buf,lstrlen(buf),&transfered,NULL);
            }
            CloseHandle(fp);
          }
        }
        else
        {
          SMALL_RECT sr;
          COORD coord;
          const char *Msg[2];
          Msg[0] = GetMsg(MTitle);
          Msg[1] = GetMsg(MSureToExit);
          i = Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_MB_YESNO,NULL,Msg,2,0);
          if (i==1 || i==-1)
            return FALSE;
          Info.SendDlgMessage(hDlg,DM_LISTDELETE,8,0);
          Info.SendDlgMessage(hDlg,DM_SHOWITEM,8,0);
          Info.SendDlgMessage(hDlg,DM_SHOWITEM,9,0);
          Info.SendDlgMessage(hDlg,DM_SHOWITEM,7,0);
          Info.SendDlgMessage(hDlg,DM_SHOWITEM,10,0);
          Info.SendDlgMessage(hDlg,DM_SHOWITEM,11,0);
          Info.SendDlgMessage(hDlg,DM_SHOWITEM,5,1);
          sr.Left = 3; sr.Top=1; sr.Right=36; sr.Bottom=15;
          Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,0,(long)&sr);
          coord.X = 40; coord.Y=17;
          Info.SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(long)&coord);
          Info.SendDlgMessage(hDlg,DMCM_TABLESELECT,1,0);
          Info.SendDlgMessage(hDlg,DMCM_REDRAW,1,0);
          EditMode=FALSE;
        }
      }
      return TRUE;

    case DN_CLOSE:
      if (MenuItems)
        free(MenuItems);
      SetRegKey(&reg[REG_LASTCOORDX],coord.X);
      SetRegKey(&reg[REG_LASTCOORDY],coord.Y);
      return TRUE;

  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *psi)
{
  Info = *psi;
  FSF = *psi->FSF;
  Info.FSF = &FSF;
  FSF.sprintf(PluginRootKey,"%s\\CharacterMap",Info.RootKey);
  GetRegKey(&reg[REG_EDITORAUTOPOS],&EditorAutoPos,1);
  GetRegKey(&reg[REG_CMDAUTOPOS],&CmdAutoPos,1);
  GetRegKey(&reg[REG_COPYTOCLIPBOARD],&CopyToClipboard,0);
  GetRegKey(&reg[REG_ENCODEBEFOREINSERTION],&EncodeBeforeInsertion,1);
  GetRegKey(&reg[REG_PRELOAD],&Preload,0);
}

void WINAPI _export GetPluginInfo(struct PluginInfo *pi)
{
  static const char *PluginMenuStrings[1];

  pi->StructSize = sizeof(struct PluginInfo);
  pi->Flags = PF_EDITOR|PF_VIEWER;
  if (Preload)
    pi->Flags |= PF_PRELOAD;
  PluginMenuStrings[0] = GetMsg(MTitle);
  pi->PluginMenuStrings = PluginMenuStrings;
  pi->PluginMenuStringsNumber = 1;
  pi->PluginConfigStrings = PluginMenuStrings;
  pi->PluginConfigStringsNumber = 1;
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int item)
{

  CHAR_INFO VBufASCII[256], VBufInfo[32], VBufDemo[19*17];
  static const struct InitDialogItem PreDialogItems[] =
  {
    DI_DOUBLEBOX   ,3  ,1  ,36 ,15 ,0                  ,MTitle,
    DI_TEXT        ,4  ,2  ,0  ,0  ,DIF_SHOWAMPERSAND  ,-1,
    DI_TEXT        ,3  ,3  ,0  ,0  ,0                  ,MSeparator,
    DI_USERCONTROL ,4  ,4  ,35 ,11 ,0                  ,-1,
    DI_TEXT        ,3  ,12 ,0  ,0  ,0                  ,MSeparator,
    DI_EDIT        ,4  ,13 ,35 ,0  ,DIF_NOFOCUS        ,-1,
    DI_USERCONTROL ,4  ,14 ,35 ,14 ,DIF_NOFOCUS        ,-1,
    DI_FIXEDIT     ,4  ,13 ,6  ,0  ,DIF_NOFOCUS|DIF_HIDDEN ,-1,
    DI_LISTBOX     ,4  ,15 ,35 ,22 ,DIF_HIDDEN|DIF_LISTNOCLOSE ,-1,
    DI_TEXT        ,7  ,13 ,35 ,0  ,DIF_HIDDEN         ,-1,
    DI_DOUBLEBOX   ,37 ,3  ,57 ,21 ,DIF_HIDDEN         ,-1,
    DI_USERCONTROL ,38 ,4  ,56 ,20 ,DIF_HIDDEN|DIF_NOFOCUS ,-1,
  };
  struct FarDialogItem DialogItems[sizeof(PreDialogItems)/sizeof(PreDialogItems[0])];
  struct WindowInfo wi;
  struct DialogData dd;
  int Ret;

  wi.Pos=-1;
  Info.AdvControl(Info.ModuleNumber,ACTL_GETWINDOWINFO,&wi);

  dd.OpenFrom = -1;
  switch (OpenFrom)
  {
    case OPEN_PLUGINSMENU:
      if (wi.Type==WTYPE_PANELS)
        dd.OpenFrom = OPENF_COMMANDLINE;
      break;
    case OPEN_EDITOR:
      if (wi.Type==WTYPE_EDITOR)
        dd.OpenFrom = OPENF_EDITOR;
      break;
    case OPEN_VIEWER:
      if (wi.Type==WTYPE_VIEWER)
        dd.OpenFrom = OPENF_VIEWER;
      break;
    case OPEN_MANUAL:
      if (item)
      {
        dd.OpenFrom = OPENF_MANUAL;
        dd.Str = (unsigned char *)item;
        Ret = 0;
        dd.Ret = &Ret;
      }
      break;
  }
  if (dd.OpenFrom<0)
    return INVALID_HANDLE_VALUE;

  InitDialogItems(PreDialogItems,DialogItems,sizeof(PreDialogItems)/sizeof(PreDialogItems[0]));

  DialogItems[3].Focus = TRUE;
  DialogItems[3].VBuf = VBufASCII;
  DialogItems[5].Data[0] = 0;
  DialogItems[6].VBuf = VBufInfo;
  DialogItems[8].ListItems = NULL;
  DialogItems[11].VBuf = VBufDemo;

  Info.DialogEx(Info.ModuleNumber,-1,-1,40,17,"Contents",(struct FarDialogItem *)&DialogItems,sizeof(PreDialogItems)/sizeof(PreDialogItems[0]),0,0,MyDialog,(int)&dd);

  if (dd.OpenFrom == OPENF_MANUAL)
    return (HANDLE)Ret;
  return INVALID_HANDLE_VALUE;
}

int Config(void)
{
  static const struct InitDialogItem PreDialogItems[] =
  {
    DI_DOUBLEBOX  ,3  ,1  ,50 ,10 ,0               ,MTitle,
    DI_CHECKBOX   ,5  ,2  ,0  ,0  ,0               ,MEditorAutoPos,
    DI_CHECKBOX   ,5  ,3  ,0  ,0  ,0               ,MCmdAutoPos,
    DI_TEXT       ,4  ,4  ,0  ,0  ,DIF_SEPARATOR   ,-1,
    DI_CHECKBOX   ,5  ,5  ,0  ,0  ,0               ,MCopyToClipboard,
    DI_TEXT       ,4  ,6  ,0  ,0  ,DIF_SEPARATOR   ,-1,
    DI_CHECKBOX   ,5  ,7  ,0  ,0  ,0               ,MEncodeBeforeInsertion,
    DI_TEXT       ,4  ,8  ,0  ,0  ,DIF_SEPARATOR   ,-1,
    DI_BUTTON     ,0  ,9  ,0  ,0  ,DIF_CENTERGROUP ,MOk,
    DI_BUTTON     ,0  ,9  ,0  ,0  ,DIF_CENTERGROUP ,MCancel
  };
  struct FarDialogItem DialogItems[sizeof(PreDialogItems)/sizeof(PreDialogItems[0])];

  InitDialogItems(PreDialogItems,DialogItems,sizeof(PreDialogItems)/sizeof(PreDialogItems[0]));
  DialogItems[1].Focus = TRUE;
  DialogItems[8].DefaultButton = 1;

  DialogItems[1].Selected = EditorAutoPos;
  DialogItems[2].Selected = CmdAutoPos;
  DialogItems[4].Selected = CopyToClipboard;
  DialogItems[6].Selected = EncodeBeforeInsertion;

  if (Info.Dialog(Info.ModuleNumber,-1,-1,54,12,"Configure",(struct FarDialogItem *)&DialogItems,sizeof(PreDialogItems)/sizeof(PreDialogItems[0])) != 8)
    return FALSE;

  EditorAutoPos = DialogItems[1].Selected;
  CmdAutoPos = DialogItems[2].Selected;
  CopyToClipboard = DialogItems[4].Selected;
  EncodeBeforeInsertion = DialogItems[6].Selected;
  SetRegKey(&reg[REG_EDITORAUTOPOS],EditorAutoPos);
  SetRegKey(&reg[REG_CMDAUTOPOS],CmdAutoPos);
  SetRegKey(&reg[REG_COPYTOCLIPBOARD],CopyToClipboard);
  SetRegKey(&reg[REG_ENCODEBEFOREINSERTION],EncodeBeforeInsertion);
  return TRUE;
}

int WINAPI _export Configure(int ItemNumber)
{
  return Config();
}
