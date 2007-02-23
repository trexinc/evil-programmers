/* $Header: /cvsroot/farplus/FARPlus/FARPlus.h,v 1.7 2002/08/24 14:55:43 yole Exp $
   FAR+Plus: A FAR Plugin C++ class library: header file
   (C) 1998-2002 Dmitry Jemerov <yole@yole.ru>
   This file is heavily based on sources by Eugene Roshal
*/

#ifndef __FARPLUS_H
#define __FARPLUS_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include "../plugin.hpp"
#include "FARString.h"
#include "FARArray.h"
#include "FarDlgHlp.h"

// If this is defined, new features of FAR 1.70 are used.
// If this is not defined, the plugin will be compatible with FAR 1.65,
// but its code may be larger.
//#define USE_FAR_170

// -- Service functions ------------------------------------------------------

namespace FarSF {

    // FSF functions
    int AddEndSlash (char *Path);
    char* PointToName(const char *Path);
    void RecursiveSearch (const char *InitDir, const char *Mask, FRSUSERFUNC Func,
        DWORD Flags, void *param);
    int CmpNameList (const char *MaskList, const char *Path, bool skipPath = false);
  bool CmpNameListEx( const char *MaskList, const char *Path, bool skipPath = false );
    char *RTrim (char *Str);
    char *LTrim (char *Str);
    char *Trim (char *Str);
    void Unquote (char *Str);
    void qsort (void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
    void *bsearch (const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
    int atoi (const char *string);
  char *itoa (int value, char *string, int radix);
  int GetFileOwner (const char *Computer, const char *Name, char *Owner);
    int GetNumberOfLinks (const char *Name);
  char *QuoteSpaceOnly (char *Str);
  char *MkTemp (char *Dest, const char *Prefix);

  int LIsAlpha (unsigned Ch);
  int LIsAlphanum (unsigned Ch);
  int LIsLower (unsigned Ch);
  int LIsUpper (unsigned Ch);
  unsigned LUpper(unsigned LowerChar);

  char *TruncStr (char *Str, int MaxLength);
  char *TruncPathStr (char *Str, int MaxLength);

  int CopyToClipboard (const char *Data);
  FarString PasteFromClipboard();

    // Useful functions not in FSF
    char *QuoteText (char *Str);
    const char *GetCommaWord(const char *Src, char *Word, char Separator=',');
    void ConvertNameToShort(const char *Src,char *Dest);
    int CheckForEsc();
    void InsertCommas (unsigned long Number, char *Dest);
    void Bytes2Str (unsigned long bytes, char *buf);

    int Execute(HANDLE hPlugin,const char *CmdStr,bool HideOutput,
        bool Silent,bool ShowTitle,int MWaitForExternalProgram,
    bool SeparateWindow = false);

  int vsprintf( char * Buf, const char * Fmt, va_list argList );
  int sprintf( char * Buf, const char * Fmt, ...);
  int vsnprintf( char * Buf, size_t Size, const char * Fmt, va_list argList );
  int snprintf( char * Buf, size_t Size, const char * Fmt, ...);

  FarString GetEnvironmentVariable( LPCSTR Name );

#ifdef USE_FAR_170

    extern FarStandardFunctions m_FSF;

    inline int AddEndSlash (char *Path)
        { return m_FSF.AddEndSlash (Path); }
    inline char *PointToName (const char *Path)
        { return m_FSF.PointToName (Path); }
    inline void RecursiveSearch ( const char *InitDir, const char *Mask, FRSUSERFUNC Func,
        DWORD Flags, void *param)
        { m_FSF.FarRecursiveSearch (InitDir, Mask, Func, Flags, param); }
    inline int CmpNameList (const char *MaskList, const char *Path, bool skipPath )
    { return m_FSF.ProcessName (MaskList, const_cast<char *> (Path),
      PN_CMPNAMELIST | (skipPath ? PN_SKIPPATH : 0)); }
    inline char *LTrim (char *Str)
        { return m_FSF.LTrim (Str); }
    inline char *RTrim (char *Str)
        { return m_FSF.RTrim (Str); }
    inline char *Trim (char *Str)
        { return m_FSF.Trim (Str); }
    inline void Unquote (char *Str)
        { m_FSF.Unquote (Str);  }
    inline unsigned LUpper(unsigned LowerChar)
        { return m_FSF.LUpper(LowerChar); }
    inline int GetFileOwner (const char *Computer, const char *Name, char *Owner)
        { return m_FSF.GetFileOwner (Computer, Name, Owner);  }
    inline int GetNumberOfLinks (const char *Name)
        { return m_FSF.GetNumberOfLinks (Name); }
    inline char *QuoteSpaceOnly (char *Str)
        { return m_FSF.QuoteSpaceOnly (Str); }
  inline char *MkTemp (char *Dest, const char *Prefix)
    { return m_FSF.MkTemp (Dest, Prefix); }
  inline char *TruncStr (char *Str, int MaxLength)
    { return m_FSF.TruncStr (Str, MaxLength); }
  inline char *TruncPathStr (char *Str, int MaxLength)
    { return m_FSF.TruncPathStr (Str, MaxLength); }
  inline FarString TruncPathStr( const FarString& fileName, int MaxLength)
  {
    FarString result( fileName );
    m_FSF.TruncPathStr( result.GetBuffer( MaxLength ), MaxLength );
    result.ReleaseBuffer();
    return result;
  }
  inline int atoi (const char *string)
    { return m_FSF.atoi (string); }
  inline char *itoa (int value, char *string, int radix)
    { return m_FSF.itoa (value, string, radix); }
  inline int LIsAlpha (unsigned Ch)
    { return m_FSF.LIsAlpha (Ch); }
  inline int LIsAlphanum (unsigned Ch)
    { return m_FSF.LIsAlphanum (Ch); }
  inline int LIsLower (unsigned Ch)
    { return m_FSF.LIsLower (Ch); }
  inline int LIsUpper (unsigned Ch)
    { return m_FSF.LIsUpper (Ch); }

  inline int LStricmp( const char * str1, const char * str2 )
  { return m_FSF.LStricmp( str1, str2 ); }
  inline int LStrnicmp( const char * str1, const char * str2, int num )
  { return m_FSF.LStrnicmp( str1, str2, num ); }

  inline int CopyToClipboard (const char *Data)
    { return m_FSF.CopyToClipboard (Data); }
  inline int LLower( int UpperChar )
  { return m_FSF.LLower( UpperChar ); }
  inline FarString PasteFromClipboard()
  {
        char *buf = m_FSF.PasteFromClipboard();
    FarString str = buf;
    m_FSF.DeleteBuffer (buf);
    return str;
  }
  inline int InputRecordToKey( const INPUT_RECORD *r )
  { return m_FSF.FarInputRecordToKey( r ); }
#endif

    inline void qsort (void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *))
    {
#ifdef USE_FAR_170
        m_FSF.qsort (base, nelem, width, fcmp);
#else
        ::qsort (base, nelem, width, fcmp);
#endif
    }

    inline void qsort( void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *, void*), void * user )
    {
#ifdef USE_FAR_170
        m_FSF.qsortex(base, nelem, width, fcmp, user );
#else
        ::qsortex(base, nelem, width, fcmp, user );
#endif
    }

    inline void *bsearch (const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *))
    {
#ifdef USE_FAR_170
        return m_FSF.bsearch (key, base, nelem, width, fcmp);
#else
        return ::bsearch (key, base, nelem, width, fcmp);
#endif
    }
};

// ProcessEditorEvent flags not documented in FAR 1.60

#ifndef FE_READ
    #define FE_READ 0
    #define FE_SAVE 1
#endif

// -- Far --------------------------------------------------------------------

class Far
{
private:
  static bool m_Init;
protected:
    friend class FarCtrl;
    friend class FarEd;
    friend class FarDialog;
    friend class FarMessage;
    friend class FarMenuStd;
  friend class FarMenuExt;
    friend class FarDirList;

  friend class FarCustomPanelPlugin;

    static PluginStartupInfo m_Info;
    static char m_RootKey [256];
  static FarStringArray *fDiskMenuStrings;
  static FarIntArray *fDiskMenuNumbers;
  static FarStringArray *fPluginMenuStrings;
  static FarStringArray *fPluginConfigStrings;
    static DWORD  m_PluginFlags;
    static char  *m_CommandPrefix;
#ifdef USE_FAR_170
  friend struct _FARDIALOGHELPER;
public:
  typedef _FARDIALOGHELPER * HDIALOG;
  typedef long (WINAPI *DLGPROC)( HDIALOG hDlg, int Msg, int Param1, long Param2 );
  static int Dialog( int X1, int Y1, int X2, int Y2, const char * HelpTopic,
    FarDialogItem * Items, int ItemsCount, DWORD Flags = 0, DLGPROC DlgProc = NULL, void * Param = NULL )
  {
    return m_Info.DialogEx( m_Info.ModuleNumber, X1, Y1, X2, Y2,
      HelpTopic, Items, ItemsCount, 0, Flags, (FARWINDOWPROC)DlgProc, (long)Param );
  }
#else
public:
  static int Dialog( int X1, int Y1, int X2, int Y2, const char * HelpTopic, FarDialogItem * Items, int ItemsCount )
  {
    return m_Info.Dialog( m_Info.ModuleNumber, X1, Y1, X2, Y2, HelpTopic, Items, ItemsCount );
  }
#endif
public:
    // Construction
    static void Init (const PluginStartupInfo *Info, DWORD PluginFlags = 0);
    static void Done();
  static bool IsInitialized() { return m_Init; }
    static int GetVersion();
    static bool RequireVersion (int MinVersion);

    static const char *GetModuleName();
  static const char *GetRootKey();

  static PluginStartupInfo * GetPluginStartupInfo() { return &m_Info; }

    // FAR wrappers
    static const char *GetMsg(int MsgId);
    static int Editor (const char *FileName, const char *Title=NULL, int X1=0, int Y1=0,
                int X2=-1, int Y2=-1, int StartLine=0, int StartChar=1, DWORD Flags = 0);
    static int Viewer (const char *FileName, const char *Title=NULL, int X1=0, int Y1=0,
                int X2=-1, int Y2=-1, DWORD Flags = 0);
    static HANDLE SaveScreen (int X1=0, int Y1=0, int X2=-1, int Y2=-1);
    static void RestoreScreen (HANDLE hScreen);
    static void Text (int X, int Y, int Color, const char *Text);
    static void Text (int X, int Y, int Color, int LngIndex);
    static void FlushText();
    static int CmpName (const char *Pattern, const char *String, bool SkipPath);
  static int GetCharTable (int Index, CharTableSet *pTable);
  static int DetectCharTable (const char *Buffer, int BufferSize);

#ifdef USE_FAR_170
  static int AdvControl (int Command, void *Param);
#endif
  static int GetBuildNumber();

    // GetPluginInfo helpers
    static void ClearDiskMenu();
    static void ClearPluginMenu();
    static void ClearPluginConfig();
    static void AddDiskMenu (const char *Text, int Number=0);
    static void AddDiskMenu (int LngIndex, int Number=0);
    static void AddPluginMenu (const char *Text);
    static void AddPluginMenu (int LngIndex);
    static void AddPluginConfig (const char *Text);
    static void AddPluginConfig (int LngIndex);
    static void SetPluginFlags (DWORD Flags);
    static void SetCommandPrefix (const char *CommandPrefix);
    static void GetPluginInfo (struct PluginInfo *Info);

    // Miscellaneous helpers

    // Note: this function CANNOT be used when PanelItem.UserData is a class.
    // Its destructor will NOT be called.
    static void FreePanelItems (PluginPanelItem *PanelItem, int ItemsNumber);
};

// -- Inline functions -------------------------------------------------------

inline const char *Far::GetModuleName()
{
    return m_Info.ModuleName;
}

inline const char *Far::GetMsg(int MsgId)
{
    return m_Info.GetMsg (m_Info.ModuleNumber, MsgId);
}

inline int Far::Editor (const char *FileName, const char *Title, int X1, int Y1,
                       int X2, int Y2, int StartLine, int StartChar, DWORD Flags)
{
    return m_Info.Editor (FileName, Title, X1, Y1, X2, Y2, Flags, StartLine, StartChar);
}

inline int Far::Viewer (const char *FileName, const char *Title, int X1, int Y1,
                       int X2, int Y2, DWORD Flags)
{
    return m_Info.Viewer (FileName, Title, X1, Y1, X2, Y2, Flags);
}

inline HANDLE Far::SaveScreen (int X1, int Y1, int X2, int Y2)
{
    return m_Info.SaveScreen (X1, Y1, X2, Y2);
}

inline void Far::RestoreScreen (HANDLE hScreen)
{
    m_Info.RestoreScreen (hScreen);
}

inline void Far::Text (int X, int Y, int Color, const char *Text)
{
    m_Info.Text (X, Y, Color, Text);
}

inline void Far::Text (int X, int Y, int Color, int LngIndex)
{
    m_Info.Text (X, Y, Color, GetMsg (LngIndex));
}

inline void Far::FlushText()
{
    m_Info.Text (0, 0, 0, NULL);
}

inline int Far::CmpName (const char *Pattern, const char *String, bool SkipPath)
{
    return m_Info.CmpName (Pattern, String, SkipPath);
}

inline int Far::GetCharTable (int Index, CharTableSet *pTable)
{
  return m_Info.CharTable (Index, reinterpret_cast<char *> (pTable), sizeof (CharTableSet));
}

inline int Far::DetectCharTable (const char *Buffer, int BufferSize)
{
  return m_Info.CharTable (FCT_DETECT, const_cast<char *> (Buffer), BufferSize);
}

inline void Far::SetPluginFlags (DWORD Flags)
{
    m_PluginFlags = Flags;
}

#ifdef USE_FAR_170

inline int Far::AdvControl (int Command, void *Param)
{
  return m_Info.AdvControl (m_Info.ModuleNumber, Command, Param);
}

#endif

inline int Far::GetBuildNumber()
{
  if (GetVersion() >= 0x170)
    return HIWORD (m_Info.AdvControl (m_Info.ModuleNumber, ACTL_GETFARVERSION, NULL));
  return 0;
}

// -- FarCtrl ----------------------------------------------------------------

// encapsulates the Control API

class FarCtrl
{
private:
    HANDLE m_hPlugin;

  int FCTL (int Command, void *Param);

public:
    FarCtrl()
        : m_hPlugin (INVALID_HANDLE_VALUE) {}
    FarCtrl (HANDLE hPlugin)
        : m_hPlugin (hPlugin) {}
    void SetHandle (HANDLE hPlugin)
        { m_hPlugin = hPlugin; }

    int ClosePlugin (const char *DestPath);
    int GetPanelInfo (PanelInfo *pi);
  PanelInfo GetPanelInfo();
    int GetAnotherPanelInfo( PanelInfo * pi );
  PanelInfo GetAnotherPanelInfo();
  int GetAnotherPanelShortInfo(PanelInfo *pi);
  int GetPanelShortInfo(PanelInfo *pi);
  PanelInfo GetPanelShortInfo();
    int UpdatePanel (bool clearSelection = false);
    int UpdateAnotherPanel (bool clearSelection = false);
    int RedrawPanel( PanelRedrawInfo * ri = NULL );
    int RedrawAnotherPanel();
  int SetPanelDir (const char *dir);
  int SetAnotherPanelDir (const char *dir);
  int GetCmdLine (char *buf);
  FarString GetCmdLine();
  int SetCmdLine (const char *buf);
  int InsertCmdLine (const char *buf);
  int GetCmdLinePos();
  int SetCmdLinePos (int newPos);
    int SetSelection (PanelInfo *pi);
  int SetAnotherSelection (PanelInfo *pi);
    int SetUserScreen();
  int SetViewMode (int viewMode);
  int SetAnotherViewMode (int viewMode);
};

inline int FarCtrl::FCTL (int Command, void *Param)
{
  return Far::m_Info.Control (m_hPlugin, Command, Param);
}

inline int FarCtrl::ClosePlugin (const char *DestPath)
{
    return FCTL (FCTL_CLOSEPLUGIN, (void *) DestPath);
}

inline int FarCtrl::GetPanelInfo (PanelInfo *pi)
{
    return FCTL (FCTL_GETPANELINFO, pi);
}

inline PanelInfo FarCtrl::GetPanelInfo()
{
  PanelInfo pi;
  GetPanelInfo (&pi);
  return pi;
}

inline int FarCtrl::GetAnotherPanelShortInfo( PanelInfo * pi )
{
  return FCTL( FCTL_GETANOTHERPANELSHORTINFO, pi );
}

inline PanelInfo FarCtrl::GetAnotherPanelInfo()
{
  PanelInfo pi;
  GetAnotherPanelInfo (&pi);
  return pi;
}

inline int FarCtrl::GetAnotherPanelInfo (PanelInfo *pi)
{
    return FCTL (FCTL_GETANOTHERPANELINFO, pi);
}

inline int FarCtrl::GetPanelShortInfo( PanelInfo *pi )
{
    return FCTL (FCTL_GETPANELSHORTINFO, pi);
}

inline PanelInfo FarCtrl::GetPanelShortInfo()
{
  PanelInfo pi;
    GetPanelShortInfo( &pi );
  return pi;
}

inline int FarCtrl::UpdatePanel (bool clearSelection)
{
    return FCTL (FCTL_UPDATEPANEL, clearSelection ? NULL : (void *) 1);
}

inline int FarCtrl::UpdateAnotherPanel (bool clearSelection)
{
    return FCTL (FCTL_UPDATEANOTHERPANEL, clearSelection ? NULL : (void *) 1);
}

inline int FarCtrl::RedrawPanel( PanelRedrawInfo * ri )
{
    return FCTL (FCTL_REDRAWPANEL, ri);
}

inline int FarCtrl::RedrawAnotherPanel()
{
    return FCTL (FCTL_REDRAWANOTHERPANEL, NULL);
}

inline int FarCtrl::SetSelection (PanelInfo *pi)
{
    return FCTL (FCTL_SETSELECTION, pi);
}

inline int FarCtrl::SetAnotherSelection (PanelInfo *pi)
{
    return FCTL (FCTL_SETANOTHERSELECTION, pi);
}

inline int FarCtrl::SetUserScreen()
{
    return FCTL (FCTL_SETUSERSCREEN, NULL);
}

inline int FarCtrl::SetPanelDir (const char *dir)
{
  return FCTL (FCTL_SETPANELDIR, (void *) dir);
}

inline int FarCtrl::SetAnotherPanelDir (const char *dir)
{
  return FCTL (FCTL_SETANOTHERPANELDIR, (void *) dir);
}

inline int FarCtrl::GetCmdLine (char *buf)
{
  return FCTL (FCTL_GETCMDLINE, buf);
}

inline FarString FarCtrl::GetCmdLine()
{
  char buf [1024];
  GetCmdLine (buf);
  return FarString (buf);
}

inline int FarCtrl::SetCmdLine (const char *buf)
{
  return FCTL (FCTL_SETCMDLINE, (void *) buf);
}

inline int FarCtrl::InsertCmdLine (const char *buf)
{
  return FCTL (FCTL_INSERTCMDLINE, (void *) buf);
}

inline int FarCtrl::GetCmdLinePos()
{
  int pos;
    FCTL (FCTL_GETCMDLINEPOS, &pos);
  return pos;
}

inline int FarCtrl::SetCmdLinePos (int newPos)
{
  return FCTL (FCTL_SETCMDLINEPOS, &newPos);
}

inline int FarCtrl::SetViewMode (int viewMode)
{
  return FCTL (FCTL_SETVIEWMODE, &viewMode);
}

inline int FarCtrl::SetAnotherViewMode (int viewMode)
{
  return FCTL (FCTL_SETANOTHERVIEWMODE, &viewMode);
}

// -- FarMessage -------------------------------------------------------------

class FarMessage
{
protected:
    unsigned int m_Flags;
    FarString m_HelpTopic;
    char **m_Items;
    int m_ItemsNumber;
    int m_ButtonsNumber;

public:
    FarMessage (unsigned int Flags=0, const char *HelpTopic=NULL);
    ~FarMessage();
    // The first line is the message title
    void AddLine (const char *Text);
    void AddLine (int LngIndex);
    void AddFmt (const char *FmtText, ...);
    void AddFmt (int FmtLngIndex, ...);
    void AddSeparator();
    int AddButton (const char *Text);
    int AddButton (int LngIndex);
    int Show();
    int SimpleMsg (unsigned int Flags, ...);
    int ErrMsg (const char *Text, int ExtraFlags = 0);
    int ErrMsg (int LngIndex, int ExtraFlags = 0);
  static int Show( DWORD Flags, LPCSTR HelpTopic, LPCSTR const * Items, int ItemsCount, int ButtonsCount )
  {
    return Far::m_Info.Message( Far::m_Info.ModuleNumber, Flags, HelpTopic, Items, ItemsCount, ButtonsCount );
  }
};

// -- FarMenu ----------------------------------------------------------------
/*
class FarMenu
{
private:
  FarMenu (const FarMenu &rhs);
  const FarMenu &operator= (const FarMenu &rhs);

protected:
    FarString fTitle;
    unsigned int fFlags;
    FarString fHelpTopic;
    FarString fBottom;
    int *fBreakKeys;
    int fBreakCode;
    int fX, fY;
  int fMaxHeight;
    FarMenuItem *fItems;
    int fItemsNumber;
    bool fOwnsBreakKeys;
    int InternalAddItem (const char *Text, int Selected, int Checked, int Separator);

public:
    FarMenu (const char *TitleText, unsigned int Flags=FMENU_WRAPMODE,
        const char *HelpTopic=NULL);
    FarMenu (int TitleLngIndex, unsigned int Flags=FMENU_WRAPMODE,
        const char *HelpTopic=NULL);
    ~FarMenu();

    void SetLocation (int X, int Y)
    { fX = X; fY = Y; }
    void SetMaxHeight (int MaxHeight)
    { fMaxHeight = MaxHeight; }
    void SetBottomLine (const FarString &Text)
    { fBottom = Text; }
    void SetBottomLine (int LngIndex)
    { fBottom = Far::GetMsg (LngIndex); }
    void SetBreakKeys (int *BreakKeys)
    { fOwnsBreakKeys = false; fBreakKeys = BreakKeys; }
    void SetBreakKeys (int aFirstKey, ...);

    // returns index of new item
    int AddItem (const char *Text, bool Selected=false, int Checked=0);
    int AddItem (int LngIndex, bool Selected=false, int Checked=0);
    int AddSeparator();

    void ClearItems();
    void SelectItem (int index);

    // returns index of selected item
    int Show();
    int GetBreakCode() const
    { return fBreakCode; }
};
*/
// -- FarSaveScreen ----------------------------------------------------------

class FarSaveScreen
{
private:
    HANDLE m_handle;
    bool m_bFullRestore;

public:
    FarSaveScreen(bool bFullRestore=false)
        { m_handle = Far::SaveScreen(); m_bFullRestore=bFullRestore; }
    ~FarSaveScreen()
        { if (m_bFullRestore) Far::RestoreScreen (NULL); Far::RestoreScreen (m_handle); }
};

// -- FarDirList -------------------------------------------------------------

class FarDirList
{
private:
    PluginPanelItem *m_pItems;
    int m_itemsNumber;
    bool m_status;

public:
    FarDirList (const char *Dir)
        { m_status = Far::m_Info.GetDirList (Dir, &m_pItems, &m_itemsNumber) ? true : false; }
    ~FarDirList()
        { if (m_status) Far::m_Info.FreeDirList (m_pItems); }

    bool Success()
        { return m_status; }
    int Count()
        { return m_itemsNumber; }
    PluginPanelItem &operator[] (int index)
        { return m_pItems [index]; }
};

// -- FarScreen -------------------------------------------------------------

class FarScreen
{
private:
  HANDLE m_hScreen;
public:
  FarScreen( int X1 = 0, int Y1 = 0, int X2 = -1, int Y2 = -1 )
    : m_hScreen( Far::SaveScreen( X1, Y1, X2, Y2 ) )
  {
  }
  ~FarScreen()
  {
    Restore();
  }
  void Save( int X1 = 0, int Y1 = 0, int X2 = -1, int Y2 = -1 )
  {
    Restore();
    m_hScreen = Far::SaveScreen( X1, Y1, X2, Y2 );
  }
  void Restore()
  {
    if ( m_hScreen != NULL )
    {
      Far::RestoreScreen( m_hScreen );
      m_hScreen = NULL;
    }
  }
};

#endif
