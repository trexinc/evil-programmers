#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers

#if defined FAR3
#include <windows.h>
#include "plugin.v3.hpp"
#elif defined UNICODE
#include "plugin.v2.hpp"
#else
#include "plugin_ansi.hpp"
#endif

#include <shellapi.h>
#include <shlobj.h>
#if defined(__GNUC__)
#include <CRT/crt.hpp>
#endif
#include <malloc.h>

#if defined FAR3
#define EXP_NAME(p) p ## W
#elif defined UNICODE
#define EXP_NAME(p) _export p ## W
#else
#define EXP_NAME(p) _export p
#endif

#define LENGTH_OF_ARRAY(a) (sizeof(a) / sizeof(a[0]))

enum { DoCopy, DoCut, DoNothing, DoPaste, DoLink, DoCount };

static CONST TCHAR CmdParams[DoCount][6] =
{
  TEXT("copy"), TEXT("cut"), TEXT(""), TEXT("paste"), TEXT("link")
};

enum
{
  MCopy, MCut, MTitle, MPaste, MLink,
  MError, MOk,
  MErrParseDisplayNameFile, MErrSetCutAttributes,
  MErrSetClipboard, MErrLinkCreate, MErrPrepareToCopy
};

static CONST TCHAR szUsingThePlugin[] = TEXT("Use");
static CONST TCHAR szLnk           [] = TEXT(".lnk");

// {fa871763-7379-4cb4-bdb0-e4ef6fb0b524}
static const GUID ClipCopyGuid = 
{ 0xfa871763, 0x7379, 0x4cb4, { 0xbd, 0xb0, 0xe4, 0xef, 0x6f, 0xb0, 0xb5, 0x24 } };

struct PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
HANDLE hHeap;

static HRESULT CreateLink(LPCTSTR lpszPathObj, LPCTSTR lpszPathLink);
static VOID ShowErrorMsg(INT ErrorNumber, INT ErrorCode);
static VOID PanelsRedraw(VOID);
static DWORD _stdcall CopyThread(LPVOID pci);
static LPCTSTR GetMsg(INT MsgId);
static VOID parse_cmdline(LPTSTR cmdstart, LPTSTR *argv, LPTSTR args, LPUINT numargs, LPUINT numchars);

//static __inline int IsSpace(int c) { return ((c) == 0x20 || ((c) >= 0x09 && (c) <= 0x0D)); }

static VOID ShowErrorMsg(INT ErrorNumber, INT ErrorCode)
{
  LPCTSTR MsgCaption = GetMsg(MError);
  LPCTSTR MsgText = GetMsg(ErrorNumber);
  LPCTSTR MsgButtons = GetMsg(MOk);
  INT MsgTextLength = lstrlen(MsgText);

  if (MsgCaption && MsgText && MsgButtons)
  {
    LPTSTR buf = (LPTSTR)alloca((MsgTextLength + 1 + 32) * sizeof(buf[0]));
    if (buf)
    {
      if (wsprintf(buf, MsgText, ErrorCode) >= MsgTextLength)
      {
        LPCTSTR MsgItems[] = {MsgCaption, buf, MsgButtons};
        Info.Message(
#ifdef FAR3
	        &ClipCopyGuid, &ClipCopyGuid,
#else
            Info.ModuleNumber,
#endif
            FMSG_WARNING, szUsingThePlugin, MsgItems, LENGTH_OF_ARRAY(MsgItems), 1);
      }
    }
  }
}

static inline HRESULT CreateLink(LPCTSTR lpszPathObj, LPCTSTR lpszPathLink)
{
  HRESULT hres;
  IShellLink* psl;

  // Get a pointer to the IShellLink interface.
  hres = CoCreateInstance(CLSID_ShellLink, NULL,
    CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);

  if (SUCCEEDED(hres))
  {
    IPersistFile* ppf;

    // Set the path to the shortcut target, and add the
    // description.
    psl->SetPath(lpszPathObj);

    //    psl->SetDescription(lpszDesc);

    // Query IShellLink for the IPersistFile interface for saving the
    // shortcut in persistent storage.
    hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

    if (SUCCEEDED(hres))
    {
      LPCWSTR wsz = NULL;

#ifdef UNICODE
      wsz = lpszPathLink;
#else
      // Ensure that the string is ANSI.
      int len = MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, (LPWSTR)wsz, 0);
      wsz = (LPCWSTR)alloca(len * sizeof(WCHAR));
      if (wsz)
      {
        MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, (LPWSTR)wsz, len);
      }
#endif

      // Save the link by calling IPersistFile::Save.
      hres = ppf->Save(wsz, TRUE);
      ppf->Release();
    }
    psl->Release();
  }

  return hres;
}

static inline VOID PanelsRedraw(VOID)
{
#if defined FAR3
  Info.AdvControl(&ClipCopyGuid, ACTL_SYNCHRO, NULL, NULL);
      struct PanelInfo PInfo;
      Info.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, 0, &PInfo);
#elif defined UNICODE
  Info.AdvControl(Info.ModuleNumber, ACTL_SYNCHRO, NULL);
#else
  SetFileApisToOEM();
  Info.Control(INVALID_HANDLE_VALUE, FCTL_UPDATEPANEL, NULL);
  Info.Control(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, NULL);
#endif
}

#ifdef UNICODE
#ifdef FAR3
intptr_t WINAPI EXP_NAME(ProcessSynchroEvent)(const struct ProcessSynchroEventInfo *OInfo)
#else
INT WINAPI ProcessSynchroEventW(INT Event, VOID *Param)
#endif
{
#ifdef FAR3
    INT Event = OInfo->Event;
#endif
  if (Event == SE_COMMONSYNCHRO)
  {
#ifdef FAR3
    Info.PanelControl(INVALID_HANDLE_VALUE, FCTL_UPDATEPANEL, 0, NULL);
    Info.PanelControl(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, 0, NULL);
#else
    Info.Control(INVALID_HANDLE_VALUE, FCTL_UPDATEPANEL, 0, NULL);
    Info.Control(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, 0, NULL);
#endif
  }

  return 0;
}
#endif

static DWORD _stdcall CopyThread(LPVOID pci)
{
  SHFILEOPSTRUCT FOS;
#ifndef UNICODE
  DWORD dwStart = GetTickCount();
#endif

  ZeroMemory(&FOS, sizeof(FOS));
  FOS.pTo = (TCHAR*)(((DWORD*)pci) + 1);
  if (*(DWORD*)pci == DROPEFFECT_MOVE)
  {
    FOS.wFunc = FO_MOVE;
  }
  else
  {
    FOS.wFunc = FO_COPY;
  }
  FOS.pFrom = &FOS.pTo[lstrlen(FOS.pTo) + 2 * sizeof(TCHAR)];
  FOS.fFlags = FOF_ALLOWUNDO;
  SHFileOperation(&FOS);
  HeapFree(hHeap, 0, pci);
#ifndef UNICODE
  if (GetTickCount() < (dwStart + 2000))
#endif
  {
    PanelsRedraw();
  }
  return TRUE;
}

static LPCTSTR GetMsg(INT MsgId)
{
    return (Info.GetMsg(
#ifdef FAR3
        &ClipCopyGuid, (intptr_t)MsgId
#else
   	    Info.ModuleNumber, MsgId
#endif
    ));
}

#define NULCHAR    TEXT('\0')
#define SPACECHAR  TEXT(' ')
#define TABCHAR    TEXT('\t')
#define DQUOTECHAR TEXT('\"')
#define SLASHCHAR  TEXT('\\')
static VOID parse_cmdline(LPTSTR cmdstart, LPTSTR *argv, LPTSTR args, LPUINT numargs, LPUINT numchars)
{
  LPTSTR p;
  INT inquote;                    /* 1 = inside quotes */
  INT copychar;                   /* 1 = copy char to *args */
  UINT numslash;              /* num of backslashes seen */

  *numchars = 0;
  *numargs = 0;

  p = cmdstart;

  inquote = 0;

  /* loop on each argument */
  for(;;) {

    if ( *p ) {
      while (*p == SPACECHAR || *p == TABCHAR)
        ++p;
    }

    if (*p == NULCHAR)
      break;              /* end of args */

    /* scan an argument */
    if (argv)
      *argv++ = args;     /* store ptr to arg */
    ++*numargs;

    /* loop through scanning one argument */
    for (;;) {
      copychar = 1;
      /* Rules: 2N backslashes + " ==> N backslashes and begin/end quote
      2N+1 backslashes + " ==> N backslashes + literal "
      N backslashes ==> N backslashes */
      numslash = 0;
      while (*p == SLASHCHAR) { /* count number of backslashes for use below */
        ++p;
        ++numslash;
      }
      if (*p == DQUOTECHAR) {
        /* if 2N backslashes before, start/end quote, otherwise
        copy literally */
        if (numslash % 2 == 0) {
          if (inquote && p[1] == DQUOTECHAR) {
            p++;      /* Double quote inside quoted string */
          } else {    /* skip first quote char and copy second */
            copychar = 0;
            inquote = !inquote;
          }
        }
        numslash >>= 1;             /* divide numslash by two */
      }

      /* copy slashes */
      while (numslash--) {
        if (args)
          *args++ = SLASHCHAR;
        ++*numchars;
      }

      /* if at end of arg, break loop */
      if (*p == NULCHAR || (!inquote && (*p == SPACECHAR || *p == TABCHAR)))
        break;

      /* copy character into argument */
      if (copychar) {
        if (args)
          *args++ = *p;
        ++*numchars;
      }
      ++p;
    }

    /* null-terminate the argument */

    if (args)
      *args++ = NULCHAR;          /* terminate string */
    ++*numchars;
  }
}

DWORD GetCurrDir(DWORD nBufLen, LPTSTR lpBuf)
{
    DWORD size;
#if defined FAR3
    int Size=Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,0,NULL);
    FarPanelDirectory* dirInfo = (FarPanelDirectory*)new char[Size];
    dirInfo->StructSize = sizeof(FarPanelDirectory);
    Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,Size,dirInfo);
    size = lstrlen(dirInfo->Name);
    if (size <= nBufLen)
        wcscpy(lpBuf,dirInfo->Name);
    else
        size = 0;
    delete[](char *)dirInfo;
#elif defined UNICODE
    size = Info.Control(PANEL_ACTIVE, FCTL_GETCURRENTDIRECTORY, nBufLen, (LONG_PTR)lpBuf) - 1;
#else
    size = GetCurrentDirectory(nBufLen, lpBuf);
#endif
    return size;
}

#ifdef FAR3
HANDLE WINAPI EXP_NAME(Open)(const struct OpenInfo *OInfo)
#else
HANDLE WINAPI EXP_NAME(OpenPlugin)(INT OpenFrom, INT_PTR Item)
#endif
{
  static UINT CF_PREFERREDDROPEFFECT = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
  IDataObject *pData;
  IShellFolder *pDesktop = NULL, *pFolder = NULL;
  FORMATETC form = {0, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  STGMEDIUM medium;
  HRESULT hres = NOERROR;
  INT MaxSize;
  UINT size, count, i;
  LPTSTR pf;
  LPMALLOC pMalloc = NULL;
  LPITEMIDLIST *ppidl;
  LPVOID p;
#ifdef FAR3
  PanelInfo PInfo = {sizeof(PanelInfo)};
#else
  struct PanelInfo PInfo;
#endif
DWORD effect;

  INT ExitCode = DoNothing;
  LPTSTR *SelectedFiles;
  INT SelectedFilesCount;
#ifdef FAR3
  INT OpenFrom = OInfo->OpenFrom;
  INT_PTR Item = OInfo->Data;
#endif

#ifdef UNICODE
#define MY_MAX_PATH (32767)
  LPTSTR szName = (LPTSTR)HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, MY_MAX_PATH * sizeof(TCHAR));
  LPTSTR szPath = (LPTSTR)HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, MY_MAX_PATH * sizeof(TCHAR));
  LPWSTR wName, wPath;
#else
#define MY_MAX_PATH (MAX_PATH)
  TCHAR szName[MY_MAX_PATH], szPath[MY_MAX_PATH];
  WCHAR wName[MY_MAX_PATH], wPath[MY_MAX_PATH];
#endif

  if (OpenFrom == OPEN_COMMANDLINE)
  {
    parse_cmdline((LPTSTR)Item, NULL, NULL, (LPUINT)&SelectedFilesCount, &i);

    SelectedFiles = (LPTSTR*)alloca(SelectedFilesCount * sizeof(TCHAR*) + i * sizeof(TCHAR) + 1);

    parse_cmdline((LPTSTR)Item, SelectedFiles,
      ((LPTSTR)SelectedFiles) + sizeof(TCHAR*) * SelectedFilesCount,
      (LPUINT)&SelectedFilesCount, &i);

    SelectedFilesCount--;
    for (i = 0; i < DoCount; i++)
    {
      if (lstrcmpi(SelectedFiles[0], CmdParams[i]) == 0)
      {
        ExitCode = i;
      }
    }
    SelectedFiles = &SelectedFiles[1];
  }

  if (ExitCode == DoNothing)
  {
    struct FarMenuItem MenuItems[5];

    OpenFrom = OPEN_PLUGINSMENU;
    ZeroMemory(&MenuItems, sizeof(MenuItems));
#ifdef FAR3
    MenuItems[DoNothing].Flags = MIF_SEPARATOR;
#else
    MenuItems[DoNothing].Separator = 1;
#endif
    for (i = 0; i < DoCount; i++)
    {
#ifdef UNICODE
      MenuItems[i].Text = GetMsg(i);
#else
      lstrcpyn(MenuItems[i].Text, GetMsg(i), sizeof(MenuItems[i].Text));
#endif
    }

    ExitCode = Info.Menu(
#ifdef FAR3
        &ClipCopyGuid, &ClipCopyGuid,
#else
   	    Info.ModuleNumber,
#endif
        -1, -1, 0, FMENU_WRAPMODE,
    GetMsg(MTitle), NULL, szUsingThePlugin, NULL, NULL, MenuItems, 5);
#if defined FAR3
    Info.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, 0, &PInfo);
#elif defined UNICODE
    Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, 0, (LONG_PTR)&PInfo);
#else
    Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &PInfo);
#endif
    SelectedFilesCount = PInfo.SelectedItemsNumber;
  }

#ifndef UNICODE
  SetFileApisToANSI();
#endif

  switch (ExitCode)
  {
  case DoCopy:
  case DoCut:
    {
      OleSetClipboard(NULL);
      ppidl = (LPITEMIDLIST*)alloca(sizeof(LPITEMIDLIST) * SelectedFilesCount);
      if (ppidl)
      {
        OleInitialize(NULL);
        SHGetMalloc(&pMalloc);
        SHGetDesktopFolder(&pDesktop);

        MaxSize = 0;
#ifdef UNICODE
        for (i = 0; (INT)i < SelectedFilesCount; i++)
        {
#ifdef FAR3
          int size = Info.PanelControl(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM, i, 0);
#else
          int size = Info.Control(INVALID_HANDLE_VALUE, FCTL_GETSELECTEDPANELITEM, i, 0);
#endif
          if (size > MaxSize)
          {
            MaxSize = size;
          }
        }
        PluginPanelItem* PPI = (PluginPanelItem*)HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, MaxSize);
#endif

        count = 0;
        for (i = 0; (INT)i < SelectedFilesCount; i++)
        {
          LPTSTR filename = NULL;
          if (OpenFrom == OPEN_COMMANDLINE)
          {
            filename = SelectedFiles[i];
          }
          else
          {
#if defined FAR3
            FarGetPluginPanelItem FGPPI={sizeof(FarGetPluginPanelItem), MaxSize, PPI};
            size = Info.PanelControl(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM, (int)i, &FGPPI);
            filename = (LPTSTR)PPI->FileName;
#elif defined UNICODE
            Info.Control(INVALID_HANDLE_VALUE, FCTL_GETSELECTEDPANELITEM, i, (LONG_PTR)PPI);
            filename = PPI->FindData.lpwszFileName;
#else
            filename = PInfo.SelectedItems[i].FindData.cFileName;
#endif
          }

          if (filename)
          {
#ifdef UNICODE
            FSF.ConvertPath(CPM_FULL, filename, szName, MY_MAX_PATH);
#else
            lstrcpyn(szName, filename, MY_MAX_PATH);
#endif
          }

#ifndef UNICODE
          OemToChar(szName, szName);
#endif

          if (!GetFullPathName(szName, MY_MAX_PATH, szPath, &pf))
          {
            continue;
          }
          if (lstrcpy(szName, pf))
          {
            *pf = '\0';
#ifdef UNICODE
            wName = szName;
#else
            MultiByteToWideChar(CP_ACP, 0, szName, -1, wName, MY_MAX_PATH);
#endif
            if (!pFolder)
            {
#ifdef UNICODE
              wPath = szPath;
#else
              MultiByteToWideChar(CP_ACP, 0, szPath, -1, wPath, MY_MAX_PATH);
#endif
              hres = pDesktop->ParseDisplayName(0, 0, wPath, 0, ppidl, 0);
              if FAILED(hres)
              {
                break;
              }

              hres = pDesktop->BindToObject(ppidl[0], 0, IID_IShellFolder, (LPVOID*)&pFolder);
              pMalloc->Free(ppidl[0]);
              if FAILED(hres)
              {
                break;
              }
            }
            if SUCCEEDED(pFolder->ParseDisplayName(0, 0, wName, 0, &ppidl[count], 0))
            {
              count++;
            }
          }
        }

#ifdef UNICODE
        HeapFree(hHeap, 0, PPI);
#endif

        if FAILED(hres)
        {
          ShowErrorMsg(MErrPrepareToCopy, hres);
        }
        if (count != i)
        {
          ShowErrorMsg(MErrParseDisplayNameFile, i - count);
        }

        if (count)
        {
          hres = pFolder->GetUIObjectOf(0, count, (LPCITEMIDLIST*)ppidl, IID_IDataObject, 0, (LPVOID*)&pData);
          for (i = 0; i < count; i++)
          {
            pMalloc->Free(ppidl[i]);
          }
          if SUCCEEDED(hres)
          {
            if (ExitCode == DoCut)
            {
              form.cfFormat = (WORD)CF_PREFERREDDROPEFFECT;
              medium.tymed = TYMED_HGLOBAL;
              medium.pUnkForRelease = NULL;
              medium.hGlobal = GlobalAlloc(GPTR, sizeof(effect));
              if (medium.hGlobal)
              {
                *(DWORD*)medium.hGlobal = DROPEFFECT_MOVE;
                hres = pData->SetData(&form, &medium, TRUE);
                if FAILED(hres)
                {
                  ReleaseStgMedium(&medium);
                  ShowErrorMsg(MErrSetCutAttributes, hres);
                }
              }
            }
            hres = OleSetClipboard(pData);
            if SUCCEEDED(hres)
            {
              hres = OleFlushClipboard();
            }
            pData->Release();
          }
        }
        if FAILED(hres)
        {
          ShowErrorMsg(MErrSetClipboard, hres);
        }

        if (pFolder)
        {
          pFolder->Release();
        }
        if (pDesktop)
        {
          pDesktop->Release();
        }
        if (pMalloc)
        {
          pMalloc->Release();
        }
        OleUninitialize();
        PanelsRedraw();
        break;
      }
    }
  case DoPaste:
  case DoLink:
    {
      OleInitialize(NULL);
      hres = OleGetClipboard(&pData);
      if SUCCEEDED(hres)
      {
        effect = DROPEFFECT_COPY;
        form.cfFormat = (WORD)CF_PREFERREDDROPEFFECT;
        hres = pData->GetData(&form, &medium);
        if SUCCEEDED(hres)
        {
          p = GlobalLock(medium.hGlobal);
          if (p)
          {
            if ((*(LPDWORD)p & DROPEFFECT_MOVE) == DROPEFFECT_MOVE)
            {
              effect = DROPEFFECT_MOVE;
            }
            GlobalUnlock(p);
          }
          ReleaseStgMedium(&medium);
        }
        form.cfFormat = CF_HDROP;
        hres = pData->GetData(&form, &medium);
        if SUCCEEDED(hres)
        {
          LPDWORD ci;
          TCHAR tgtPath[MY_MAX_PATH];

          count = DragQueryFile((HDROP)medium.hGlobal, 0xFFFFFFFF, NULL, 0);
          if (ExitCode == DoPaste)
          {
            //*ci: (DWORD)struct_size (TCHAR[MY_MAX_PATH])to (TCHAR)0 (wchar_t?DWORD:0)0 (TCHAR[MY_MAX_PATH][count])from (TCHAR)0
            ci = (DWORD*)HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
              (count + 1) * MY_MAX_PATH + sizeof(DWORD) + 2);
            size = GetCurrDir(MY_MAX_PATH, (LPTSTR)&ci[1]) + sizeof(DWORD) + 2;
          }
          else
          {
              size = GetCurrDir(MY_MAX_PATH, (LPTSTR)&tgtPath);
              tgtPath[size++] = '\\';
          }

          for (i = 0; i < count; i++)
          {
            if (ExitCode == DoPaste)
            {
              size += DragQueryFile((HDROP)medium.hGlobal, i, &((TCHAR*)ci)[size], MY_MAX_PATH) + 1;
            }
            else
            {
              DragQueryFile((HDROP)medium.hGlobal, i, szName, MY_MAX_PATH);
              if (GetFullPathName(szName, MY_MAX_PATH, szPath, &pf) != 0)
              {
                lstrcpy((LPTSTR)&tgtPath[size],pf);
                pf = (LPTSTR)&tgtPath[size];
                int j, cnt = 1;

                for (j = lstrlen(pf) - 1; j >= 0 && pf[j] != '.'; j--)
                  ;

                if (j < 0)
                {
                  j = lstrlen(pf);
                }

                lstrcpy(&pf[j], szLnk);
                WIN32_FIND_DATA fd;
                HANDLE hFind;
                while (hFind = FindFirstFile((LPTSTR)&tgtPath, &fd), hFind != INVALID_HANDLE_VALUE)
                {
                  FindClose(hFind);
                  wsprintf(&pf[j], TEXT(" (%d)%s"), ++cnt, szLnk);
                }
                hres = CreateLink(szName, (LPTSTR)&tgtPath);
              }
              if FAILED(hres)
              {
                ShowErrorMsg(MErrLinkCreate, hres);
              }
            }
          }
          ReleaseStgMedium(&medium);
          pData->Release();
          if (ExitCode == DoPaste)
          {
            if (effect == DROPEFFECT_MOVE)
            {
              OleSetClipboard(NULL);
            }
            *ci = effect;
#ifndef UNICODE
            if( int(GetVersion()) < 0 ) // W9x
            {
              CharToOemBuff((LPTSTR)&ci[1], (LPTSTR)&ci[1], (count + 1) * (MAX_PATH + 1) + 2);
              SetFileApisToOEM();
            }
#endif
            DWORD Dummy;
            CreateThread(NULL, 0, CopyThread, ci, 0, &Dummy);
          }
          else
          {
            PanelsRedraw();
          }
        }
      }
      OleUninitialize();
      break;
    }
  }
#ifndef UNICODE
  SetFileApisToOEM();
#endif

#ifdef UNICODE
  HeapFree(hHeap, 0, szPath);
  HeapFree(hHeap, 0, szName);
#endif

#ifdef FAR3
  return NULL;
#else
  return INVALID_HANDLE_VALUE;
#endif
}

VOID WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
  static LPCTSTR PluginMenuStrings[1];
  PluginMenuStrings[0] = GetMsg(MTitle);
  Info->StructSize = sizeof(*Info);
  Info->CommandPrefix = TEXT("fclip");
#ifdef FAR3
  Info->PluginMenu.Guids = &ClipCopyGuid;
  Info->PluginMenu.Strings = PluginMenuStrings;
  Info->PluginMenu.Count = _countof(PluginMenuStrings);
  Info->PluginConfig.Guids = &ClipCopyGuid;
  Info->PluginConfig.Strings = PluginMenuStrings;
  Info->PluginConfig.Count = _countof(PluginMenuStrings);
#else
  Info->PluginMenuStrings = PluginMenuStrings;
  Info->PluginMenuStringsNumber = _countof(PluginMenuStrings);
  Info->PluginConfigStrings = PluginMenuStrings;
  Info->PluginConfigStringsNumber = _countof(PluginMenuStrings);
#endif
}

VOID WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *pInfo)
{
  Info = *pInfo;
  FSF = *pInfo->FSF;
  Info.FSF = &FSF;
  hHeap = GetProcessHeap();
}

#ifndef FAR3
INT WINAPI EXP_NAME(GetMinFarVersion)(VOID)
{
  return MAKEFARVERSION(2, 0, 1148);
}
#endif

#ifdef FAR3
VOID WINAPI EXP_NAME(GetGlobalInfo)(struct GlobalInfo *Info) {
  Info->StructSize = sizeof(struct GlobalInfo);
  Info->MinFarVersion = FARMANAGERVERSION;
  Info->Version = MAKEFARVERSION(3,0,1,0,VS_RELEASE);
  Info->Guid = ClipCopyGuid;
  Info->Title = L"Clipboard";
  Info->Description = L"Explorer like copy using clipboard";
  Info->Author = L"Andrey Budko, Stanislav Mekhanoshin";
}
#endif

/*#ifndef _DEBUG
extern "C" int WINAPI _DllMainCRTStartup(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  return TRUE;
}
#endif*/

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
void __cxa_pure_virtual(void);
#ifdef __cplusplus
};
#endif

void __cxa_pure_virtual(void)
{
}

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
