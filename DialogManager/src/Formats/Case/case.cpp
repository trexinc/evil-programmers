/*
    Case plugin for DialogManager
    Copyright (C) 2003 Vadim Yegorov
    Copyright (C) 2004 Vadim Yegorov and Alex Yaroslavsky

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
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "../../plugin.hpp"
#include "../../farkeys.hpp"
#include "../../dm_module.hpp"
#include "memory.hpp"

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

enum
{
  mName,
  mLower,
  mUpper,
  mTitle,
  mToggle,
  mCyclic,
  mProcessWholeLine,
};

enum
{
  CCLower,
  CCUpper,
  CCTitle,
  CCToggle,
  CCCyclic,
};

PluginStartupInfo FarInfo;
FARSTANDARDFUNCTIONS FSF;
DialogPluginStartupInfo DialogInfo;
char PluginRootKey[100];
char WordDiv[80];
int WordDivLen;
DWORD ProcessWholeLine=1;

char msg1[128];

BOOL FindBounds(unsigned char *Str, int Len, int Pos, int *Start, int *End);
int FindEnd(unsigned char *Str, int Len, int Pos);
int FindStart(unsigned char *Str, int Len, int Pos);
int GetNextCCType(unsigned char *Str, int StrLen, int Start, int End);
int ChangeCase(unsigned char *NewString, int Start, int End, int CCType);

char *GetMsg(int MsgNum,char *Str)
{
  DialogInfo.GetMsg(DialogInfo.MessageName,MsgNum,Str);
  return Str;
}

// What we consider as letter
BOOL MyIsAlpha(unsigned char c)
{
  return (memchr(WordDiv,c,WordDivLen)==NULL ? TRUE : FALSE);
}

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct DialogPluginStartupInfo *DialogInfo)
{
  ::FarInfo=*FarInfo;
  FSF=*FarInfo->FSF;
  ::DialogInfo=*DialogInfo;
  FSF.sprintf(PluginRootKey,"%s\\Case",::DialogInfo.RootKey);
  WordDivLen=::FarInfo.AdvControl(::FarInfo.ModuleNumber,ACTL_GETSYSWORDDIV,WordDiv);
  WordDivLen += sizeof(" \n\r\t");
  strcat(WordDiv, " \n\r\t");
  return 0;
}

void WINAPI _export Exit(void)
{
}

int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData)
{
  (void)OutData;
  switch(Msg)
  {
    case FMMSG_GETINFO:
      {
        GetInfoOutData *data=(GetInfoOutData *)OutData;
        GetMsg(mName,data->MenuString);
        strcpy(data->HotkeyID,"zg_case");
        data->Flags=FMMSG_MENU;
      }
      return TRUE;
    case FMMSG_MENU:
      {
        MenuInData *dlg=(MenuInData *)InData;
        FarMenuItem MenuItems[7];
        memset(MenuItems,0,sizeof(MenuItems));
        MenuItems[0].Selected=1;
        GetMsg(mLower,MenuItems[0].Text);
        GetMsg(mUpper,MenuItems[1].Text);
        GetMsg(mTitle,MenuItems[2].Text);
        GetMsg(mToggle,MenuItems[3].Text);
        GetMsg(mCyclic,MenuItems[4].Text);
        MenuItems[5].Separator=1;
        GetMsg(mProcessWholeLine,MenuItems[6].Text);
        HKEY hKey;
        if(RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
        {
          DWORD Type,DataSize=sizeof(ProcessWholeLine);
          RegQueryValueEx(hKey,"ProcessWholeLine",0,&Type,(BYTE *)&ProcessWholeLine,&DataSize);
          RegCloseKey(hKey);
        }
        MenuItems[6].Checked=(ProcessWholeLine?1:0);
        GetMsg(mName,msg1);
        int BreakKeys[2]={VK_SPACE,0};
        int MenuCode;
        while (6==(MenuCode=FarInfo.Menu(FarInfo.ModuleNumber,-1,-1,0,FMENU_WRAPMODE,msg1,NULL,NULL,BreakKeys,NULL,MenuItems,sizeof(MenuItems)/sizeof(MenuItems[0]))))
        {
          if (MenuItems[6].Checked)
          {
            ProcessWholeLine=FALSE;
            MenuItems[6].Checked=0;
          }
          else
          {
            ProcessWholeLine=TRUE;
            MenuItems[6].Checked=1;
          }
          MenuItems[0].Selected=0;
          MenuItems[6].Selected=1;
        }
        DWORD Disposition;
        if(RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition)==ERROR_SUCCESS)
        {
          RegSetValueEx(hKey,"ProcessWholeLine",0,REG_DWORD,(BYTE *)&ProcessWholeLine,sizeof(ProcessWholeLine));
          RegCloseKey(hKey);
        }
        if(MenuCode>=0)
        {
          FarDialogItem DialogItem;
          FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,dlg->ItemID,(long)&DialogItem);
          if(DialogItem.Type==DI_EDIT)
          {
            long length=FarInfo.SendDlgMessage(dlg->hDlg,DM_GETTEXTLENGTH,dlg->ItemID,0)+1;
            unsigned char *buffer=(unsigned char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,length);
            if(buffer)
            {
              EditorSelect es;
              es.BlockType=BTYPE_NONE;
              FarInfo.SendDlgMessage(dlg->hDlg,DM_GETSELECTION,dlg->ItemID,(long)&es);
              COORD Pos; FarInfo.SendDlgMessage(dlg->hDlg,DM_GETCURSORPOS,dlg->ItemID,(long)&Pos);
              FarInfo.SendDlgMessage(dlg->hDlg,DM_GETTEXTPTR,dlg->ItemID,(long)buffer);
              int Start=0,End=length-1;
              if (es.BlockType==BTYPE_NONE||es.BlockStartPos<0)
              {
                if (!ProcessWholeLine)
                  FindBounds(buffer, length-1, Pos.X, &Start, &End);
              }
              else
              {
                Start=es.BlockStartPos;
                End=es.BlockStartPos+es.BlockWidth;
              }
              if(MenuCode==CCCyclic)
              {
                //Define Conversion Type
                MenuCode=GetNextCCType(buffer, length-1, Start, End);
              }
              //Buffer contains no words
              if(MenuCode!=CCCyclic)
              {
                //Do the conversion
                ChangeCase(buffer, Start, End, MenuCode);
              }
              FarInfo.SendDlgMessage(dlg->hDlg,DM_SETTEXTPTR,dlg->ItemID,(long)buffer);
              FarInfo.SendDlgMessage(dlg->hDlg,DM_GETDLGITEM,dlg->ItemID,(long)&DialogItem);
              FarInfo.SendDlgMessage(dlg->hDlg,DN_EDITCHANGE,dlg->ItemID,(long)&DialogItem);
              FarInfo.SendDlgMessage(dlg->hDlg,DM_SETCURSORPOS,dlg->ItemID,(long)&Pos);
              FarInfo.SendDlgMessage(dlg->hDlg,DM_SETSELECTION,dlg->ItemID,(long)&es);
              HeapFree(GetProcessHeap(),0,buffer);
            }
          }
        }
      }
      return TRUE;
  }
  return FALSE;
}

// Finding word bounds (what'll be converted) (Str is in OEM)
BOOL FindBounds(unsigned char *Str, int Len, int Pos, int *Start, int *End)
{
  int i = 1;
  int ret = FALSE;
  int r = INT_MAX;
  // If line isn't empty
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
        i=INT_MAX;

      // Here r is left radius and i is right radius

      // If no letters was found
      if( min(r,i)!=INT_MAX )
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

int FindStart(unsigned char *Str, int Start, int End)
{
  // Current pos in Str
  int CurPos=End-1;
  // While current character is letter
  while( CurPos>=Start && MyIsAlpha(Str[CurPos]) )
    CurPos--; // Moving to left

  return CurPos+1;
}

int FindEnd(unsigned char *Str, int Start, int End)
{
  // Current pos in Str
  int CurPos=Start;
  // While current character is letter
  while( CurPos<End && MyIsAlpha(Str[CurPos]))
    CurPos++; // Moving to right

  return CurPos;
}

// Changes Case of NewString from position Start till End
// to CCType and returns amount of changes
int ChangeCase(unsigned char *NewString, int Start, int End, int CCType)
{
  // If previous symbol is letter, then IsPrevSymbAlpha!=0
  BOOL IsPrevSymbAlpha=FALSE;
  // Amount of changes
  int ChangeCount=0;
  // Main loop (position inside line)
  for(int i=Start; i<End; i++)
  {
    if (MyIsAlpha(NewString[i]))// && ReverseOem==NewString[i])
    {
      switch(CCType)
      {
        case CCLower:
          NewString[i]=FSF.LLower(NewString[i]);
          break;

        case CCTitle:
          if(IsPrevSymbAlpha)
            NewString[i]=FSF.LLower(NewString[i]);
          else
            NewString[i]=FSF.LUpper(NewString[i]);
          break;

        case CCUpper:
          NewString[i]=FSF.LUpper(NewString[i]);
          break;

        case CCToggle:
          if(FSF.LIsLower(NewString[i]))
            NewString[i]=FSF.LUpper(NewString[i]);
          else
            NewString[i]=FSF.LLower(NewString[i]);
          break;

      }
      // Put converted letter back to string
      IsPrevSymbAlpha=TRUE;
      ChangeCount++;
    }
    else
      IsPrevSymbAlpha=FALSE;
  }

  return ChangeCount;
}

// Return CCType by rule: lower->UPPER->Title
// If Str contains no letters, then return CCCyclic
int GetNextCCType(unsigned char *Str, int StrLen, int Start, int End)
{
  int SignalWordLen;
  // Default conversion is to lower case
  int CCType;
  unsigned char *WrappedWord;
  unsigned char *SignalWord;
  int Counter;
  int i;

  SignalWordLen=max(Start,End);
  Counter=SignalWordLen/2+1;
  SignalWordLen=min(Start,End);

  if (StrLen<SignalWordLen)
    return CCCyclic;

  // Looking for SignalWord (the 1-st word)
  if (!FindBounds(Str,StrLen,Start,&Start,&End))
    return CCCyclic;

  SignalWordLen=End-Start;

  SignalWord=(unsigned char *)malloc(SignalWordLen+1);

  CCType=CCLower;
  if( SignalWord != NULL )
  {
    WrappedWord=(unsigned char *)malloc(SignalWordLen+1);

    if (WrappedWord != NULL)
    {
      strncpy((char *)SignalWord,(const char *)&Str[Start],SignalWordLen);
      SignalWord[SignalWordLen]='\0';
      strcpy((char *)WrappedWord,(const char *)SignalWord);

      // if UPPER then Title
      FSF.LUpperBuf((char *)WrappedWord, SignalWordLen);
      if (SignalWordLen == 1 && strcmp((const char *)SignalWord,(const char *)WrappedWord)==0)
        CCType=CCLower;
      else
      {
        if (SignalWordLen == 1)
          CCType=CCUpper;
        else
        {
          if (strcmp((const char *)SignalWord,(const char *)WrappedWord)==0)
            CCType=CCTitle;
          else
          {
            // if lower then UPPER
            FSF.LLowerBuf((char *)WrappedWord, SignalWordLen);
            if(strcmp((const char *)SignalWord,(const char *)WrappedWord)==0)
              CCType=CCUpper;
            else
            {
              // if Title then lower
              WrappedWord[0]=FSF.LUpper(WrappedWord[0]);
              if(strcmp((const char *)SignalWord,(const char *)WrappedWord)==0)
                CCType=CCLower;
              else
              {
                // if upper case letters amount more than lower case letters
                // then tOGGLE
                FSF.LUpperBuf((char *)WrappedWord, SignalWordLen);
                for(i=0; i<SignalWordLen && Counter; i++)
                  if(SignalWord[i]==WrappedWord[i])
                    Counter--;
                if(!Counter)
                  CCType=CCToggle;
              }
            }
          }
        }
      }
      free(WrappedWord);
    }
    free(SignalWord);
  }
  return CCType;
}
