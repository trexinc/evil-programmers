/*
    Multilingual Editor Case Converter plugin for FAR Manager
    Copyright (C) 2001-2005 Alex Yaroslavsky

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
#include "EditCase.hpp"
#include "EditCaseLng.hpp"
#include <stdio.h>
#include <io.h>
#include <string.h>
#ifdef __GNUC__
#include <limits.h>
#define MAXINT INT_MAX
#else
#include <values.h> //MAXINT
#endif
#include "memory.hpp"

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

inline char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

HKEY CreateRegKey(HKEY hRoot,const char *Key)
{
  HKEY hKey;
  DWORD Disposition;
  char FullKeyName[512];
  strcpy(FullKeyName,PluginRootKey);
  if (*Key)
  {
    strcat(FullKeyName,"\\");
    strcat(FullKeyName,Key);
  }
  RegCreateKeyEx(hRoot,FullKeyName,0,NULL,0,KEY_WRITE,NULL,
                 &hKey,&Disposition);
  return(hKey);
}

HKEY OpenRegKey(HKEY hRoot,const char *Key)
{
  HKEY hKey;
  char FullKeyName[512];
  strcpy(FullKeyName,PluginRootKey);
  if (*Key)
  {
    strcat(FullKeyName,"\\");
    strcat(FullKeyName,Key);
  }
  if (RegOpenKeyEx(hRoot,FullKeyName,0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
    return(NULL);
  return(hKey);
}

int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData,char *Default,DWORD DataSize)
{
  HKEY hKey=OpenRegKey(hRoot,Key);
  DWORD Type;
  int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(LPBYTE)ValueData,&DataSize);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    strcpy(ValueData,Default);
    return(FALSE);
  }
  return(TRUE);
}

void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData)
{
  HKEY hKey=CreateRegKey(hRoot,Key);
  RegSetValueEx(hKey,ValueName,0,REG_SZ,(CONST BYTE *)ValueData,strlen(ValueData)+1);
  RegCloseKey(hKey);
}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *psi)
{
  Info=*psi;
  FSF=*psi->FSF;
  Info.FSF=&FSF;
  strcpy(PluginRootKey,Info.RootKey);
  strcat(PluginRootKey,"\\");
  strcat(PluginRootKey,"EditCase");
  WordDivLen = Info.AdvControl(Info.ModuleNumber, ACTL_GETSYSWORDDIV, WordDiv);
  char AddWordDiv[sizeof(WordDiv)];
  GetRegKey(HKEY_CURRENT_USER,"","AddWordDiv",AddWordDiv,"#",sizeof(AddWordDiv));
  WordDivLen += strlen(AddWordDiv);
  strcat(WordDiv, AddWordDiv);
  WordDivLen += sizeof(" \n\r\t");
  strcat(WordDiv, " \n\r\t");
  int i;
  for (i=strlen(Info.ModuleName); (Info.ModuleName[i] != '\\')
        && (Info.ModuleName[i] != '/'); i--)
    ;
  strcpy(PluginRootDir,Info.ModuleName);
  PluginRootDir[i+1] = '\0';
  /*FSF.sprintf(TableFileName,s,Info.ModuleName);
  j=strlen(Info.ModuleName)-1;
  TableFileName[j--] = 't';
  TableFileName[j--] = 'a';
  TableFileName[j--] = 'd';
  FSF.sprintf(TableFileName,s,&TableFileName[i+1]);*/
  DefaultLoaded=FALSE;
  char TableFileName[260];
  GetRegKey(HKEY_CURRENT_USER,"","TableFileName",TableFileName,"",sizeof(TableFileName));
  if (!SetTables(TableFileName))
    SetDefaultTables();
}

void WINAPI _export GetPluginInfo(struct PluginInfo *pi)
{
  static char *MenuStrings[1];

  pi->StructSize=sizeof(struct PluginInfo);
  pi->Flags=PF_EDITOR|PF_DISABLEPANELS;
  // Text in Plugins menu
  MenuStrings[0]=GetMsg(MCaseConversion);
  pi->PluginMenuStrings=MenuStrings;
  pi->PluginMenuStringsNumber=1;
  pi->PluginConfigStrings=MenuStrings;
  pi->PluginConfigStringsNumber=1;
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  int i;
  struct FarMenuItem MenuItems[7], *MenuItem;
  int MenuCode;
  int Msgs[]={MCaseLower, MCaseUpper, MCaseTitle, MCaseToggle, MCaseCyclic};
  int CurLine;
  BOOL IsBlock;
  struct EditorConvertText ect;
  struct EditorSetString ess;
  char *NewString;
  struct EditorGetString egs;
  int CCType;
  struct EditorInfo ei;

  memset(MenuItems,0,sizeof(MenuItems));
  for (MenuItem=MenuItems,i=0; i < 5; i++, MenuItem++)
  {
    MenuItem->Selected=MenuItem->Checked=MenuItem->Separator=0;
    strcpy(MenuItem->Text,GetMsg(Msgs[i]));
  }
  MenuItems[5].Selected=MenuItems[5].Checked=0;
  MenuItems[5].Separator=1;
  MenuItems[6].Selected=MenuItems[6].Checked=MenuItems[6].Separator=0;
  strcpy(MenuItems[6].Text,GetMsg(MEditorLangSelect));
  MenuItems[0].Selected=1;

  MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,
                         GetMsg(MCaseConversion),NULL,"usage",NULL,NULL,
                         MenuItems,sizeof(MenuItems)/sizeof(MenuItems[0]));
  if (MenuCode == 6)
  {
    LangSelect();
    return (INVALID_HANDLE_VALUE);
  }
  switch(MenuCode)
  {
      // If menu Escaped
      case -1:
        break;

      default:
       Info.EditorControl(ECTL_GETINFO,&ei);

       // Current line number
       CurLine=ei.CurLine;
       // Is anything selected
       IsBlock=FALSE;

       // Nothing selected?
       if (ei.BlockType!=BTYPE_NONE)
       {
         IsBlock=TRUE;
         CurLine=ei.BlockStartLine;
       }

       // Type of Case Change
       CCType=MenuCode;

       // Forever :-) (Line processing loop)
       for(;;)
       {

         // Increase CurLine
         egs.StringNumber=CurLine++;

         // If can't get line
         if (!Info.EditorControl(ECTL_GETSTRING,&egs))
           break; // Exit

         // If last selected line was processed or
         // nothing selected and line is empty
         if ((IsBlock && egs.SelStart==-1) || (!IsBlock && egs.StringLength<=0))
           break; // Exit

         // If something selected, but line is empty
         if (egs.StringLength<=0)
           continue; // Get next line

         // If whole line (with EOL) is selected
         if (egs.SelEnd==-1 || egs.SelEnd>egs.StringLength)
         {
           egs.SelEnd=egs.StringLength;
           if (egs.SelEnd<egs.SelStart)
             egs.SelEnd=egs.SelStart;
         }

         // Memory allocation
         NewString=(char *)malloc(egs.StringLength+1);
         // If memory couldn't be allocated
         if(!NewString)
            break;


         // If nothing selected - finding word bounds (what'll be converted)
         if (!IsBlock)
         {
           // Making NewString
           strncpy(NewString,egs.StringText,egs.StringLength);
           NewString[egs.StringLength]=0;
           ect.Text=NewString;
           ect.TextLength=egs.StringLength;
           // Convert to OEM
           Info.EditorControl(ECTL_EDITORTOOEM,&ect);

           // Like whole line is selected
           egs.SelStart=0;
           egs.SelEnd=egs.StringLength;

           // Finding word bounds (what'll be converted)
           FindBounds(NewString, egs.StringLength, ei.CurPos, &egs.SelStart, &egs.SelEnd);
         }

         // Making NewString
         strncpy(NewString,egs.StringText,egs.StringLength);
         NewString[egs.StringLength]=0;
         ect.Text=&NewString[egs.SelStart];
         ect.TextLength=egs.SelEnd-egs.SelStart;
         // Convert to OEM
         Info.EditorControl(ECTL_EDITORTOOEM,&ect);

         // If Conversion Type is unknown or Cyclic
         if(CCType==CCCyclic)
             // Define Conversion Type
             CCType=GetNextCCType(NewString, egs.StringLength, egs.SelStart, egs.SelEnd);

         // NewString contains no words
         if(CCType!=CCCyclic)
         {
             // Do the conversion
             ChangeCase(NewString, egs.SelStart, egs.SelEnd, CCType);

             // Back to editor charset
             Info.EditorControl(ECTL_OEMTOEDITOR,&ect);

             // Put converted string to editor
             ess.StringNumber=egs.StringNumber;
             ess.StringText=NewString;
             ess.StringEOL=egs.StringEOL;
             ess.StringLength=egs.StringLength;
             Info.EditorControl(ECTL_SETSTRING,&ess);
         };

         #if 0
         if (!IsBlock)
         {
           struct EditorSelect esel;
           esel.BlockType=BTYPE_STREAM;
           esel.BlockStartLine=-1;
           esel.BlockStartPos=egs.SelStart;
           esel.BlockWidth=egs.SelEnd-egs.SelStart;
           esel.BlockHeight=1;
           Info.EditorControl(ECTL_SELECT,&esel);
         }
         #endif
         // Free memory
         free(NewString);

         // Exit if nothing was selected (single word was converted)
         if(!IsBlock)
             break;
       }
  } // switch

  return(INVALID_HANDLE_VALUE);
}

int WINAPI _export Configure(int ItemNumber)
{
  LangSelect();
  return TRUE;
}

void LangSelect(void)
{
  char filespec[260], SelectedTableFileName[260];
  char *Items[3];
  unsigned int msg_attrib;

  GetRegKey(HKEY_CURRENT_USER,"","TableFileName",SelectedTableFileName,"",sizeof(SelectedTableFileName));
  char **TableFileNames = (char **)malloc(sizeof(char *));
  TableFileNames[0] = (char *)malloc(sizeof(char));
  struct FarMenuItem *MenuItems = (struct FarMenuItem *)malloc(sizeof(struct FarMenuItem));
  TableFileNames[0][0] = '\0'; //empty file name will load eglish only table
  strcpy(MenuItems[0].Text,GetMsg(MEnglishOnly));
  MenuItems[0].Checked = MenuItems[0].Selected = MenuItems[0].Separator = 0;
  BOOL Sel = FALSE;
  if (DefaultLoaded)
  {
    MenuItems[0].Checked = '*';
    MenuItems[0].Selected = 1;
    Sel = TRUE;
  }
  strcpy(filespec,PluginRootDir);
  strcat(filespec,"*.dat");

  int t = 1;
  struct _finddata_t fileinfo;
  long l = _findfirst(filespec,&fileinfo);
  if (l > 0)
    t = 0;
  int x = 1;
  while (t == 0)
  {
    if (!(fileinfo.attrib & _A_SUBDIR))
    {
      x++;
      MenuItems=(struct FarMenuItem *)realloc(MenuItems,x*sizeof(*MenuItems));
      TableFileNames=(char **)realloc(TableFileNames,x*sizeof(char *));
      TableFileNames[x-1]=(char *)malloc(sizeof(char)*(strlen(fileinfo.name)+1));
      strcpy(TableFileNames[x-1],fileinfo.name);
      strncpy(MenuItems[x-1].Text,fileinfo.name,127);
      MenuItems[x-1].Text[127] = '\0'; //safety
      MenuItems[x-1].Checked=MenuItems[x-1].Selected=MenuItems[x-1].Separator=0;
      if (!Sel && FSF.LStricmp(SelectedTableFileName,fileinfo.name) == 0)
      {
        MenuItems[x-1].Selected = 1;
        MenuItems[x-1].Checked = '*';
        Sel = TRUE;
      }
    }
    t = _findnext(l,&fileinfo);
  }
  _findclose(l);

  int i = Info.Menu(Info.ModuleNumber,-1,-1,0,
                FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT,
                GetMsg(MLangSelect),NULL,"Configure",NULL,NULL,MenuItems,x);
  if (i >= 0)
  {
    strcpy(SelectedTableFileName,TableFileNames[i]);

    SetRegKey(HKEY_CURRENT_USER,"","TableFileName",SelectedTableFileName);
    Items[0] = GetMsg(MLangSelect);
    if (SetTables(SelectedTableFileName))
    {
      Items[1] = GetMsg(MUpdateOK);
      Info.Message(Info.ModuleNumber,FMSG_MB_OK,NULL,(const char **)Items,2,0);
    }
    else
    {
      SetDefaultTables();
      if (!i)
      {
        Items[1] = GetMsg(MLoadedDefault);
        i=2;
        msg_attrib = FMSG_MB_OK;
      }
      else
      {
        Items[1] = GetMsg(MUpdateError);
        Items[2] = GetMsg(MLoadedDefault);
        i=3;
        msg_attrib = FMSG_MB_OK|FMSG_WARNING;
      }
      Info.Message(Info.ModuleNumber,msg_attrib,NULL,(const char **)Items,i,0);
    }
  }

  for (i=0;i<x;i++)
  {
    free(TableFileNames[i]);
  }
  free(MenuItems);
  free(TableFileNames);
}

// What we consider as letter
BOOL MyIsAlpha(unsigned char c)
{
  return ( memchr(WordDiv, c, WordDivLen)==NULL ? TRUE : FALSE );
}

// Finding word bounds (what'll be converted) (Str is in OEM)
BOOL FindBounds(char *Str, int Len, int Pos, int *Start, int *End)
{
  int i = 1;
  int ret = FALSE;
  int r = MAXINT;
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

int FindStart(char *Str, int Start, int End)
{
  // Current pos in Str
  int CurPos=End-1;
  // While current character is letter
  while( CurPos>=Start && MyIsAlpha(Str[CurPos]) )
    CurPos--; // Moving to left

  return CurPos+1;
}

int FindEnd(char *Str, int Start, int End)
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
int ChangeCase(char *NewString, int Start, int End, int CCType)
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
          NewString[i]=(char)LLower(NewString[i]);
          break;

        case CCTitle:
          if(IsPrevSymbAlpha)
            NewString[i]=(char)LLower(NewString[i]);
          else
            NewString[i]=(char)LUpper(NewString[i]);
          break;

        case CCUpper:
          NewString[i]=(char)LUpper(NewString[i]);
          break;

        case CCToggle:
          if(LIsLower(NewString[i]))
            NewString[i]=(char)LUpper(NewString[i]);
          else
            NewString[i]=(char)LLower(NewString[i]);
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
int GetNextCCType(char *Str, int StrLen, int Start, int End)
{
  int SignalWordLen;
  // Default conversion is to lower case
  int CCType;
  unsigned char *WrappedWord;
  char *SignalWord;
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

  SignalWord=(char *)malloc(SignalWordLen+1);

  CCType=CCLower;
  if( SignalWord != NULL )
  {
    WrappedWord=(unsigned char *)malloc(SignalWordLen+1);

    if (WrappedWord != NULL)
    {
      strncpy(SignalWord,&Str[Start],SignalWordLen);
      SignalWord[SignalWordLen]='\0';
      strcpy((char *)WrappedWord,SignalWord);

      // if UPPER then Title
      LUpperBuf(WrappedWord, SignalWordLen);
      if (SignalWordLen == 1 && strcmp(SignalWord,(const char *)WrappedWord)==0)
        CCType=CCLower;
      else
      {
        if (SignalWordLen == 1)
          CCType=CCUpper;
        else
        {
          if (strcmp(SignalWord,(const char *)WrappedWord)==0)
            CCType=CCTitle;
          else
          {
            // if lower then UPPER
            LLowerBuf(WrappedWord, SignalWordLen);
            if(strcmp(SignalWord,(const char *)WrappedWord)==0)
              CCType=CCUpper;
            else
            {
              // if Title then lower
              WrappedWord[0]=LUpper(WrappedWord[0]);
              if(strcmp(SignalWord,(const char *)WrappedWord)==0)
                CCType=CCLower;
              else
              {
                // if upper case letters amount more than lower case letters
                // then tOGGLE
                LUpperBuf(WrappedWord, SignalWordLen);
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

unsigned char LLower(unsigned char ch)
{
  return (to_lower[ch]?to_lower[ch]:ch);
}

unsigned char LUpper(unsigned char ch)
{
  return (to_upper[ch]?to_upper[ch]:ch);
}

int LIsLower(unsigned char ch)
{
  return (to_upper[ch]); //if there is and uppert then its lower
}

void LLowerBuf(unsigned char *Buf, int Length)
{
  for (int i=0; i<Length; i++)
    Buf[i] = to_lower[Buf[i]]?to_lower[Buf[i]]:Buf[i];
}

void LUpperBuf(unsigned char *Buf, int Length)
{
  for (int i=0; i<Length; i++)
    Buf[i] = to_upper[Buf[i]]?to_upper[Buf[i]]:Buf[i];
}

//0 mean no case conversion for this char
void SetDefaultTables(void)
{
  DefaultLoaded = TRUE;
  memset(to_lower,0,256);
  memset(to_upper,0,256);
  for(int i='a',j='A';i<='z';i++,j++)
  {
    to_lower[j] = i;
    to_upper[i] = j;
  }
}

BOOL SetTables(const char *FileName)
{
  char path[260];
  strcpy(path,PluginRootDir);
  strcat(path,FileName);
  FILE *in = fopen(path,"r");
  if (in)
  {
    DefaultLoaded = FALSE;
    memset(to_lower,0,256);
    memset(to_upper,0,256);
    unsigned char tmp[4];
    while (fgets((char *)tmp,4,in))
      if (strlen((const char *)tmp) == 3 && tmp[1] == '=')
      {
        to_upper[tmp[0]]=tmp[2];
        to_lower[tmp[2]]=tmp[0];
      }
    fclose(in);
    return TRUE;
  }
  return FALSE;
}
