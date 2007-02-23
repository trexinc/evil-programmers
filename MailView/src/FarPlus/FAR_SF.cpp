/* $Header: /cvsroot/farplus/FARPlus/FAR_SF.cpp,v 1.5 2002/08/24 14:55:43 yole Exp $
   FAR+Plus: FAR standard functions implementation
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
*/

#include "FARPlus.h"
#include <stdio.h>
#include <ctype.h>

#define FAR_isdigit(_c) isdigit(_c)
#define FAR_isspace(_c) isspace(_c)
#define FAR_isxdigit(_c) isxdigit(_c)

namespace FarSF {

FarStandardFunctions m_FSF;

const char *GetCommaWord (const char *Src,char *Word, char Separator)
{
    if (*Src==0)
        return(NULL);

    bool SkipBrackets = false;
    int WordPos;
    for (WordPos=0; *Src!=0; Src++,WordPos++)
    {
        if (*Src=='[' && strchr(Src+1,']')!=NULL)
            SkipBrackets=true;
        if (*Src==']')
            SkipBrackets=false;
        if (*Src==Separator && !SkipBrackets)
        {
            Word[WordPos]=0;
            Src++;
            while (FAR_isspace(*Src))
                Src++;
            return(Src);
        }
        else
            Word[WordPos]=*Src;
    }
    Word[WordPos]=0;
    return(Src);
}

void ConvertNameToShort (const char *Src,char *Dest)
{
    char ShortName[NM],AnsiName[NM];
    SetFileApisToANSI();
    OemToChar(Src,AnsiName);
    if (GetShortPathName(AnsiName,ShortName,sizeof(ShortName)))
        CharToOem(ShortName,Dest);
    else
        strcpy(Dest,Src);
    SetFileApisToOEM();
}

int CheckForEsc()
{
    int ExitCode=FALSE;
    while (1)
    {
        INPUT_RECORD rec;
        HANDLE hConInp=GetStdHandle(STD_INPUT_HANDLE);
        DWORD ReadCount;
        PeekConsoleInput(hConInp,&rec,1,&ReadCount);
        if (ReadCount==0)
            break;
        ReadConsoleInput(hConInp,&rec,1,&ReadCount);
        if (rec.EventType==KEY_EVENT)
        {
            if (rec.Event.KeyEvent.wVirtualKeyCode==VK_ESCAPE && rec.Event.KeyEvent.bKeyDown)
                ExitCode=TRUE;
        }
    }
    return(ExitCode);
}

void InsertCommas(unsigned long Number,char *Dest)
{
    FarSF::sprintf(Dest,"%u",Number);
    for (int I=strlen(Dest)-4;I>=0;I-=3)
        if (Dest[I])
        {
            memmove(Dest+I+2,Dest+I+1,strlen(Dest+I));
            Dest[I+1]=',';
        }
}

////
/// Bytes2Str()
//  Convert bytes to string (M, K)

void Bytes2Str (unsigned long bytes, char *buf)
{
  unsigned long int_part, fract_part;

  if (bytes >= 1024l*1024*10)
    FarSF::sprintf (buf, "%luM", bytes / (1024l*1024));
  else if (bytes >= 1024l*1024)
  {
    int_part = bytes / (1024l*1024);
    fract_part = (bytes % (1024l*1024)) / (100*1024l);
    FarSF::sprintf (buf, "%lu.%luM", int_part, fract_part);
  }
  else if (bytes >= 10*1024)
    FarSF::sprintf (buf, "%luK", bytes / 1024);
  else if (bytes >= 1024)
  {
    int_part = bytes / 1024;
    fract_part = (bytes % 1024) / 100;
    FarSF::sprintf (buf, "%lu.%luK", int_part, fract_part);
  }
  else
    FarSF::sprintf (buf, "%lub", bytes);
}

int Execute(HANDLE hPlugin,const char *CmdStr,bool HideOutput,
            bool Silent,bool ShowTitle, int MWaitForExternalProgram,
      bool SeparateWindow)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    int ExitCode, CreateProcessCode;

    memset(&si,0,sizeof(si));
    si.cb=sizeof(si);

  HANDLE hChildStdoutRd,hChildStdoutWr;
  HANDLE StdInput=GetStdHandle(STD_INPUT_HANDLE);
  HANDLE StdOutput=GetStdHandle(STD_OUTPUT_HANDLE);
  HANDLE StdError=GetStdHandle(STD_ERROR_HANDLE);
  HANDLE hScreen=NULL;
  CONSOLE_SCREEN_BUFFER_INFO csbi;

  if (HideOutput)
  {
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 32768))
    {
      SetStdHandle(STD_OUTPUT_HANDLE,hChildStdoutWr);
      SetStdHandle(STD_ERROR_HANDLE,hChildStdoutWr);

      if (Silent)
      {
        hScreen=Far::SaveScreen (0,0,-1,0);
        Far::Text (2,0,7,MWaitForExternalProgram);
      }
      else
      {
        hScreen=Far::SaveScreen();
        FarMessage msg;
        msg.AddLine ("");
        msg.AddLine (MWaitForExternalProgram);
        msg.Show();
      }
    }
    else
      HideOutput=false;
  }
  else
  {
    GetConsoleScreenBufferInfo(StdOutput,&csbi);

    char Blank[1024];
    FillMemory (Blank, csbi.dwSize.X, ' ');
    Blank [csbi.dwSize.X] = '\0';
    for (int Y=0;Y<csbi.dwSize.Y;Y++)
      Far::Text (0, Y, 7, Blank);
    Far::FlushText();

    COORD C;
    C.X=0;
    C.Y=csbi.dwCursorPosition.Y;
    SetConsoleCursorPosition(StdOutput,C);
  }


  DWORD ConsoleMode;
  GetConsoleMode(StdInput,&ConsoleMode);
  SetConsoleMode(StdInput,ENABLE_PROCESSED_INPUT|ENABLE_LINE_INPUT|
                 ENABLE_ECHO_INPUT|ENABLE_MOUSE_INPUT);

  FarString ExpandedCmd = CmdStr;
  ExpandedCmd.Expand();

  char SaveTitle[512];
  GetConsoleTitle(SaveTitle,sizeof(SaveTitle));
  if (ShowTitle)
    SetConsoleTitle(ExpandedCmd);

  CreateProcessCode=CreateProcess(NULL,ExpandedCmd.GetBuffer(),NULL,NULL,HideOutput,0,NULL,NULL,&si,&pi);

  if (HideOutput)
  {
    SetStdHandle(STD_OUTPUT_HANDLE,StdOutput);
    SetStdHandle(STD_ERROR_HANDLE,StdError);
    CloseHandle(hChildStdoutWr);
  }

  if (CreateProcessCode)
  {
    if (SeparateWindow)
      ExitCode = 0;
    else
    {
      if (HideOutput)
      {
        WaitForSingleObject(pi.hProcess,1000);

        char PipeBuf[32768];
        DWORD Read;
        while (ReadFile(hChildStdoutRd,PipeBuf,sizeof(PipeBuf),&Read,NULL))
          ;
        CloseHandle(hChildStdoutRd);
      }
      WaitForSingleObject(pi.hProcess,INFINITE);
      GetExitCodeProcess(pi.hProcess,(LPDWORD)&ExitCode);
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
  }
  else if (HideOutput)
    CloseHandle (hChildStdoutRd);
  SetConsoleTitle(SaveTitle);
  SetConsoleMode(StdInput,ConsoleMode);
  if (!HideOutput)
  {
    SMALL_RECT src;
    COORD dest;
    CHAR_INFO fill;
    src.Left=0;
    src.Top=2;
    src.Right=csbi.dwSize.X;
    src.Bottom=csbi.dwSize.Y;
    dest.X=dest.Y=0;
    fill.Char.AsciiChar=' ';
    fill.Attributes=7;
    ScrollConsoleScreenBuffer(StdOutput,&src,NULL,dest,&fill);
    FarCtrl (hPlugin).SetUserScreen();
  }
  if (hScreen)
  {
    Far::RestoreScreen (NULL);
    Far::RestoreScreen (hScreen);
  }
  if (!CreateProcessCode) return -1;
  else return ExitCode;
}


#ifndef USE_FAR_170

int AddEndSlash(char *Path)
{
    int Length=strlen(Path);
    if (Length==0 || Path[Length-1]!='\\')
        strcat(Path,"\\");
    return TRUE;
}

char* PointToName(const char *Path)
{
    const char *NamePtr=Path;
    while (*Path)
    {
        if (*Path=='\\' || *Path=='/' || *Path==':')
            NamePtr=Path+1;
        Path++;
    }
    return (char *) NamePtr;
}

bool _FarRecursiveSearch (char *InitDir, char *Mask, FRSUSERFUNC Func,
                          DWORD Flags, void *param)
{
    char realMask [512];
    strcpy (realMask, InitDir);
    AddEndSlash (realMask);
    strcat (realMask, Mask);

    bool result = true;

    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile (realMask, &findData);
    while (hFind != INVALID_HANDLE_VALUE)
    {
        char fullName [512];
        strcpy (fullName, InitDir);
        AddEndSlash (fullName);
        strcat (fullName, findData.cFileName);

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (findData.cFileName [0] != '.' && Flags & FRS_RECUR)
                if (!_FarRecursiveSearch (fullName, Mask, Func, Flags, param))
                {
                    result = false;
                    break;
                }
        }
        else
            if (!Func (&findData, fullName, param))
            {
                result = false;
                break;
            }

            if (!FindNextFile (hFind, &findData)) break;
    }

    if (hFind != INVALID_HANDLE_VALUE)
        FindClose (hFind);

    return result;
}

void RecursiveSearch (char *InitDir, char *Mask, FRSUSERFUNC Func, DWORD Flags, void *param)
{
    _FarRecursiveSearch (InitDir, Mask, Func, Flags, param);
}

int CmpNameList (const char *MaskList, const char *Path, bool skipPath /*= false*/)
{
    const char *pMask = MaskList;
    char curMask [256];
    while ((pMask = GetCommaWord (pMask, curMask)) != NULL)
        if (Far::CmpName (curMask, Path, skipPath)) return true;

        return false;
}

char *LTrim (char *Str)
{
    if (Str)
    {
        char *p = Str;
        while (FAR_isspace ((unsigned char) *p))
            p++;
        if (p != Str)
            memmove (Str, p, strlen (p)+1);
    }
    return Str;
}

char *RTrim (char *Str)
{
    if (Str)
    {
        char *p = Str + strlen (Str)-1;
        while (p >= Str && FAR_isspace ((unsigned char) *p))
            p--;
        *++p = 0;
    }
    return Str;
}

char *Trim (char *Str)
{
    return RTrim (LTrim (Str));
}

void Unquote (char *Str)
{
    if (!Str) return;
    int len = strlen (Str);
    if (!len) return;

    // remove trailing quotes
    char *p = Str + len - 1;
    while (p >= Str && *p == '\"')
    {
        p--;
        len--;
    }
    *++p = 0;

    // remove leading quotes
    p = Str;
    while (*p == '\"')
    {
        p++;
        len--;
    }
    if (p != Str)
        memmove (Str, p, len + Str - p + 2);
}

char* QuoteSpaceOnly(char *Str)
{
    if (strchr(Str,' ')!=NULL)
    {
        unsigned l = strlen(Str);
        if(*Str != '"')
        {
            memmove (Str+1,Str,++l);
            *Str='"';
        }
        if(Str[l-1] != '"')
        {
            Str[l++] = '\"';
            Str[l] = 0;
        }
    }
    return(Str);
}


char* QuoteText(char *Str)
{
  int len = strlen (Str) + 1;
  memmove (Str+1, Str, len);
  Str [0] = '\"';
  Str [len] = '\"';
  Str [len+1] = '\0';
    return Str;
}

char* TruncStr(char *Str,int MaxLength)
{
  if(Str)
  {
    int Length;
    if (MaxLength<0)
      MaxLength=0;
    if ((Length=strlen(Str))>MaxLength)
    {
      if (MaxLength>3)
      {
        memmove (Str+3,Str+Length-MaxLength+3,MaxLength);
        memcpy (Str,"...",3);
      }
      Str[MaxLength]=0;
    }
  }
  return(Str);
}

char *TruncPathStr (char *Str, int MaxLength)
{
  if(Str)
  {
    char *Root=NULL;
    if (Str[0]!=0 && Str[1]==':' && Str[2]=='\\')
      Root=Str+3;
    else
      if (Str[0]=='\\' && Str[1]=='\\' && (Root=strchr(Str+2,'\\'))!=NULL &&
        (Root=strchr(Root+1,'\\'))!=NULL)
      Root++;
    if (Root==NULL || Root-Str+5>MaxLength)
      return(TruncStr(Str,MaxLength));
    int Length=strlen(Str);
    if (Length>MaxLength)
    {
      char *MovePos=Root+Length-MaxLength+3;
      memmove (Root+3,MovePos,strlen(MovePos)+1);
      memcpy (Root,"...",3);
    }
  }
  return(Str);
}

int GetFileOwner (const char *Computer, const char *Name, char *Owner)
{
    SECURITY_INFORMATION si = OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION;
    SECURITY_DESCRIPTOR *sd;
    char sddata[500];
    *Owner=0;
    sd=(SECURITY_DESCRIPTOR *)sddata;

    char AnsiName[NM];
    OemToChar(Name,AnsiName);
    SetFileApisToANSI();
    DWORD Needed;
    int GetCode=GetFileSecurity(AnsiName,si,sd,sizeof(sddata),&Needed);
    SetFileApisToOEM();

    if (!GetCode || (Needed>sizeof(sddata)))
        return(FALSE);
    PSID pOwner;
    BOOL OwnerDefaulted;
    if (!GetSecurityDescriptorOwner(sd,&pOwner,&OwnerDefaulted))
        return(FALSE);
    char AccountName[200],DomainName[200];
    DWORD AccountLength=sizeof(AccountName),DomainLength=sizeof(DomainName);
    SID_NAME_USE snu;
    if (!LookupAccountSid(Computer,pOwner,AccountName,&AccountLength,DomainName,&DomainLength,&snu))
        return(FALSE);
    CharToOem(AccountName,Owner);
    return(TRUE);
}

int GetNumberOfLinks (const char *Name)
{
    HANDLE hFile=CreateFile(Name,0,FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,OPEN_EXISTING,0,NULL);
    if (hFile==INVALID_HANDLE_VALUE)
        return(1);
    BY_HANDLE_FILE_INFORMATION bhfi;
    int GetCode=GetFileInformationByHandle(hFile,&bhfi);
    CloseHandle(hFile);
    return(GetCode ? bhfi.nNumberOfLinks:0);
}

int atoi (const char *string)
{
  while (FAR_isspace (*string))
    string++;

  char c = *string++;
  char sign = c;

  if (c == '-' || c == '+')
    c = *string++;

  int total = 0;

  while (FAR_isdigit(c))
  {
    total = 10 * total + (c - '0');
    c = *string++;
  }

  if (sign == '-')
    return -total;
  else
    return total;
}

char *itoa (int value, char *string, int radix)
{
  char *p = string;
  if (radix == 10 && value < 0)
  {
    *p++ = '-';
    value = -value;
  }

  char *firstdig = p;

  do {
    unsigned digval = (unsigned) (value % radix);
    value /= radix;

    if (digval > 9)
      *p++ = (char) (digval - 10 + 'a');
    else
      *p++ = (char) (digval + '0');
  } while (value > 0);

  *p-- = '\0';

  do {
    char temp = *p;
    *p-- = *firstdig;
    *firstdig++ = temp;
  } while (firstdig < p);

  return string;
}

char *MkTemp (char *Dest, const char *Prefix)
{
  if(Dest && Prefix && *Prefix)
  {
    char TempPath[NM], TempName[NM];
    int Len = GetTempPath(sizeof(TempPath),TempPath);
    TempPath[Len]=0;
    if(GetTempFileName(TempPath,Prefix,0,TempName))
    {
       strcpy(Dest,CharUpper(TempName));
       return Dest;
    }
  }
  return NULL;
}

int LIsAlpha (unsigned Ch)
{
  unsigned char c = Ch;
    OemToCharBuff ((char *) &c, (char *) &c, 1);
  return IsCharAlpha (c);
}

int LIsAlphanum (unsigned Ch)
{
  unsigned char c = Ch;
    OemToCharBuff ((char *) &c, (char *) &c, 1);
  return IsCharAlphaNumeric (c);
}

int LIsLower (unsigned Ch)
{
  unsigned char c = Ch;
    OemToCharBuff ((char *) &c, (char *) &c, 1);
  return IsCharLower (c);
}

int LIsUpper (unsigned Ch)
{
  unsigned char c = Ch;
    OemToCharBuff ((char *) &c, (char *) &c, 1);
  return IsCharUpper (c);
}

int CopyToClipboard (const char *Data)
{
  OSVERSIONINFO WinVer;
  WinVer.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
  GetVersionEx (&WinVer);

    long DataSize;
    if (Data!=NULL && (DataSize=strlen(Data))!=0)
  {
    HGLOBAL hData;
    void *GData;
    if (!OpenClipboard(NULL))
      return(FALSE);
    EmptyClipboard();
    int BufferSize=DataSize+1;
    if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize))!=NULL)
      if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,DataSize+1);
        GlobalUnlock(hData);
        SetClipboardData(CF_OEMTEXT,(HANDLE)hData);
      }
    if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize))!=NULL)
      if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,DataSize+1);
        OemToChar((LPCSTR)GData,(LPTSTR)GData);
        GlobalUnlock(hData);
        SetClipboardData(CF_TEXT,(HANDLE)hData);
      }
    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
      if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize*2))!=NULL)
        if ((GData=GlobalLock(hData))!=NULL)
        {
          MultiByteToWideChar(CP_OEMCP,0,Data,-1,(LPWSTR)GData,BufferSize);
          GlobalUnlock(hData);
          SetClipboardData(CF_UNICODETEXT,(HANDLE)hData);
        }
    CloseClipboard();
  }
  return(TRUE);
}

FarString PasteFromClipboard()
{
  OSVERSIONINFO WinVer;
  WinVer.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
  GetVersionEx (&WinVer);

  HANDLE hClipData;
  if (!OpenClipboard(NULL))
    return(NULL);
  int Unicode=FALSE;
  int Format=0;
  int ReadType=CF_OEMTEXT;
  while ((Format=EnumClipboardFormats(Format))!=0)
  {
    if (Format==CF_UNICODETEXT && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
      Unicode=TRUE;
      break;
    }
    if (Format==CF_TEXT)
    {
      ReadType=CF_TEXT;
      break;
    }
    if (Format==CF_OEMTEXT)
      break;
  }
  FarString ClipText;
  if ((hClipData=GetClipboardData(Unicode ? CF_UNICODETEXT:ReadType))!=NULL)
  {
    int BufferSize;
    char *ClipAddr=(char *)GlobalLock(hClipData);
    if (Unicode)
      BufferSize=lstrlenW((LPCWSTR)ClipAddr)+1;
    else
      BufferSize=strlen(ClipAddr)+1;

    if (Unicode)
    {
      ClipText.SetLength (BufferSize);
      WideCharToMultiByte(CP_OEMCP,0,(LPCWSTR)ClipAddr,-1,ClipText.GetBuffer(),BufferSize,NULL,FALSE);
    }
    else
    {
      ClipText.SetText (ClipAddr, BufferSize);
      if (ReadType==CF_TEXT)
        ClipText = ClipText.ToOEM();
    }
    GlobalUnlock(hClipData);
  }
  CloseClipboard();
  return ClipText;
}
#else
bool CmpNameListEx( LPCSTR Masks, LPCSTR Path, bool SkipPath )
{
  LPSTR ExcludeMasks = strchr( Masks, '|' );

  if ( ExcludeMasks )
  {
    *ExcludeMasks = '\0';
    bool Include = strlen( Masks ) > 0 ?
      FarSF::CmpNameList( Masks, Path, SkipPath ) != FALSE : true;
    *ExcludeMasks++ = '|';
    bool Exclude = FarSF::CmpNameList( ExcludeMasks, Path, SkipPath ) != FALSE;
    return Include && !Exclude;
  }

  return FarSF::CmpNameList( Masks, Path, SkipPath ) != FALSE;
}

#endif

int vsprintf( char * Buf, const char * Fmt, va_list argList )
{
  return ::vsprintf( Buf, Fmt, argList );
}

int vsnprintf( char * Buf, size_t Size, const char * Fmt, va_list argList )
{
#ifdef _MSC_VER

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000
#define FORCE_INT64     0x40000

  if ( Size == 0 )
  {
    int nMaxLen = 0;
    for ( const char * p = Fmt; *p != '\0'; p ++ )
    {
      if ( *p != '%' || *(++p) == '%' )
      {
        nMaxLen ++;
        continue;
      }

      int nItemLen = 0;

      // handle '%' character with format
      int nWidth = 0;
      for ( ; *p != '\0'; p ++ )
      {
        // check for valid flags
        if ( *p == '#')
          nMaxLen += 2;   // for '0x'
        else if ( *p == '*')
          nWidth = va_arg( argList, int );
        else if ( *p == '-' || *p == '+' || *p == '0' || *p == '\x20' )
          ;
        else // hit non-flag character
          break;
      }
      // get width and skip it
      if ( nWidth == 0 )
      {
        // width indicated by
        nWidth = FarSF::atoi( p );
        for ( ; *p != '\0' && FAR_isdigit( *p ); p ++ );
      }
      far_assert( nWidth >= 0 );

      int nPrecision = 0;
      if ( *p == '.')
      {
        // skip past '.' separator (width.precision)
        p ++;

        // get precision and skip it
        if ( *p == '*')
        {
          nPrecision = va_arg( argList, int );
          p ++;
        }
        else
        {
          nPrecision = FarSF::atoi( p );
          for (; *p != '\0' && FAR_isdigit( *p ); p ++ ) ;
        }
        far_assert( nPrecision >= 0 );
      }


    // should be on type modifier or specifier
    int nModifier = 0;
    if ( FarSF::LStrnicmp( p, "I64", 3 ) == 0 )
    {
      p += 3;
      nModifier = FORCE_INT64;
    }
    else
    {
      switch ( *p )
      {
      // modifiers that affect size
      case 'h':
        nModifier = FORCE_ANSI;
        p ++;
        break;
      case 'l':
        nModifier = FORCE_UNICODE;
        p ++;
        break;

      // modifiers that do not affect size
      case 'F':
      case 'N':
      case 'L':
        p ++;
        break;
      }
    }

    // now should be on specifier
    switch ( *p | nModifier )
    {
    // single characters
    case 'c':
      nItemLen = 1;
      va_arg( argList, char* );
      break;
    case 'C':
      nItemLen = 2;
      va_arg( argList, char* );
      break;
    case 'c'|FORCE_ANSI:
    case 'C'|FORCE_ANSI:
      nItemLen = 1;
      va_arg( argList, char );
      break;
    case 'c'|FORCE_UNICODE:
    case 'C'|FORCE_UNICODE:
      nItemLen = 2;
      va_arg( argList, WCHAR ); /*WCHAR*/
      break;

    // strings
    case 's':
      {
        LPCSTR pstrNextArg = va_arg( argList, LPCSTR );
        if ( pstrNextArg == NULL )
           nItemLen = 6;  // "(null)"
        else
        {
           nItemLen = strlen( pstrNextArg );
           nItemLen = max( 1, nItemLen );
        }
      }
      break;

    case 'S':
      {
#ifndef _UNICODE
        LPWSTR pstrNextArg = va_arg( argList, LPWSTR );
        if ( pstrNextArg == NULL )
           nItemLen = 6;  // "(null)"
        else
        {
           nItemLen = wcslen( pstrNextArg );
           nItemLen = max( 1, nItemLen );
        }
#else
        LPCSTR pstrNextArg = va_arg( argList, LPCSTR );
        if (pstrNextArg == NULL)
           nItemLen = 6; // "(null)"
        else
        {
           nItemLen = strlen( pstrNextArg );
           nItemLen = max( 1, nItemLen );
        }
#endif
      }
      break;

    case 's'|FORCE_ANSI:
    case 'S'|FORCE_ANSI:
      {
        LPCSTR pstrNextArg = va_arg( argList, LPCSTR );
        if ( pstrNextArg == NULL )
           nItemLen = 6; // "(null)"
        else
        {
           nItemLen = strlen( pstrNextArg );
           nItemLen = max( 1, nItemLen );
        }
      }
      break;

    case 's'|FORCE_UNICODE:
    case 'S'|FORCE_UNICODE:
      {
        LPWSTR pstrNextArg = va_arg( argList, LPWSTR );
        if ( pstrNextArg == NULL )
           nItemLen = 6; // "(null)"
        else
        {
           nItemLen = wcslen( pstrNextArg );
           nItemLen = max( 1, nItemLen );
        }
      }
      break;
    }

    // adjust nItemLen for strings
    if ( nItemLen != 0 )
    {
      if ( nPrecision != 0 )
        nItemLen = min( nItemLen, nPrecision );
      nItemLen = max( nItemLen, nWidth );
    }
    else
    {
      switch ( *p )
      {
      // integers
      case 'd':
      case 'i':
      case 'u':
      case 'x':
      case 'X':
      case 'o':
        if ( nModifier & FORCE_INT64 )
          va_arg( argList, __int64 );
        else
          va_arg( argList, int );
        nItemLen = 32;
        nItemLen = max( nItemLen, nWidth+nPrecision );
        break;

      case 'e':
      case 'g':
      case 'G':
        va_arg( argList, double );
        nItemLen = 128;
        nItemLen = max( nItemLen, nWidth + nPrecision );
        break;

      case 'f':
        {
          // 312 == strlen("-1+(309 zeroes).")
          // 309 zeroes == max precision of a double
          // 6 == adjustment in case precision is not specified,
          //   which means that the precision defaults to 6
          char * pszTemp = new char[ max( nWidth, 312 + nPrecision + 6 ) ];

          double f = va_arg( argList, double );
          sprintf( pszTemp, "%*.*f", nWidth, nPrecision + 6, f );
          nItemLen = strlen( pszTemp );

          delete [] pszTemp;
        }
        break;

      case 'p':
        va_arg( argList, void* );
        nItemLen = 32;
        nItemLen = max( nItemLen, nWidth+nPrecision );
        break;

      // no output
      case 'n':
        va_arg( argList, int* );
        break;

      default:
        far_assert( FALSE );  // unknown formatting option
      }
    }

    // adjust nMaxLen for output nItemLen
    nMaxLen += nItemLen;


    }
    return nMaxLen;
  }

#undef FORCE_ANSI
#undef FORCE_UNICODE
#undef FORCE_INT64

#endif

  return ::_vsnprintf( Buf, Size, Fmt, argList );
}


int sprintf( char * Buf, const char * Fmt, ...)
{
  va_list argList;
  va_start( argList, Fmt );
  int result = vsprintf( Buf, Fmt, argList );
  va_end( argList );
  return result;
}


int snprintf( char * Buf, size_t Size, const char * Fmt, ...)
{
  va_list argList;
  va_start( argList, Fmt );
  int result = vsnprintf( Buf, Size, Fmt, argList );
  va_end( argList );
  return result;
}

FarString GetEnvironmentVariable( LPCSTR Name )
{
  DWORD size = ::GetEnvironmentVariable( Name, NULL, 0 );
  if ( size == 0 )
    return FarString();
  FarString result;
  result.ReleaseBuffer( ::GetEnvironmentVariable( Name, result.GetBuffer( size ), size ) );
  return result;
}

}  // namespace
