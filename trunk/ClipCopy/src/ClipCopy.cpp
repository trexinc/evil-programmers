#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers

#ifdef UNICODE
#include "plugin.hpp"
#else
#include "plugin_ansi.hpp"
#endif

#include <shellapi.h>
#include <shlobj.h>
#if defined(__GNUC__)
#include <CRT/crt.hpp>
#endif
#include <malloc.h>

#ifdef UNICODE
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
        Info.Message(Info.ModuleNumber, FMSG_WARNING, szUsingThePlugin, MsgItems,
          LENGTH_OF_ARRAY(MsgItems), 1);
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
#ifdef UNICODE
  Info.AdvControl(Info.ModuleNumber, ACTL_SYNCHRO, NULL);
#else
  SetFileApisToOEM();
  Info.Control(INVALID_HANDLE_VALUE, FCTL_UPDATEPANEL, NULL);
  Info.Control(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, NULL);
#endif
}

#ifdef UNICODE
INT WINAPI ProcessSynchroEventW(INT Event, VOID *Param)
{
  if (Event == SE_COMMONSYNCHRO)
  {
    Info.Control(INVALID_HANDLE_VALUE, FCTL_UPDATEPANEL, 0, NULL);
    Info.Control(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, 0, NULL);
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
  return (Info.GetMsg(Info.ModuleNumber, MsgId));
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

HANDLE WINAPI EXP_NAME(OpenPlugin)(INT OpenFrom, INT_PTR Item)
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
  struct PanelInfo PInfo;
  DWORD effect;

  INT ExitCode = DoNothing;
  LPTSTR *SelectedFiles;
  INT SelectedFilesCount;

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
    MenuItems[DoNothing].Separator = 1;
    for (i = 0; i < DoCount; i++)
    {
#ifdef UNICODE
      MenuItems[i].Text = GetMsg(i);
#else
      lstrcpyn(MenuItems[i].Text, GetMsg(i), sizeof(MenuItems[i].Text));
#endif
    }

    ExitCode = Info.Menu(Info.ModuleNumber, -1, -1, 0, FMENU_WRAPMODE,
      GetMsg(MTitle), NULL, szUsingThePlugin, NULL, NULL, MenuItems, 5);
#ifdef UNICODE
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
          int size = Info.Control(INVALID_HANDLE_VALUE, FCTL_GETSELECTEDPANELITEM, i, 0);
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
#ifdef UNICODE
            Info.Control(INVALID_HANDLE_VALUE, FCTL_GETSELECTEDPANELITEM, i, (LONG_PTR)PPI);
            filename = PPI->FindData.lpwszFileName;
#else
            filename = PInfo.SelectedItems[i].FindData.cFileName;
#endif
          }

          if (filename)
          {
#ifdef UNICODE
            FSF.ConvertNameToReal(filename, szName, MY_MAX_PATH);
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

          size = 0;
          count = DragQueryFile((HDROP)medium.hGlobal, 0xFFFFFFFF, NULL, 0);
          if (ExitCode == DoPaste)
          {
            ci = (DWORD*)HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
              (count + 1) * (MY_MAX_PATH + 1) + sizeof(DWORD) + 2);
#ifdef UNICODE
            size += Info.Control(PANEL_ACTIVE, FCTL_GETCURRENTDIRECTORY, 
              MY_MAX_PATH, (LONG_PTR)&ci[1]) + sizeof(DWORD) + 2 - 1;
#else
            size += GetCurrentDirectory(MY_MAX_PATH, (LPTSTR)&ci[1]) + sizeof(DWORD) + 2;
#endif
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
                while (hFind = FindFirstFile(pf, &fd), hFind != INVALID_HANDLE_VALUE)
                {
                  FindClose(hFind);
                  wsprintf(&pf[j], TEXT(" (%d)%s"), ++cnt, szLnk);
                }
                hres = CreateLink(szName, pf);
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

  return INVALID_HANDLE_VALUE;
}

VOID WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
  static LPCTSTR PluginMenuStrings[1];
  PluginMenuStrings[0] = GetMsg(MTitle);
  Info->PluginMenuStrings = PluginMenuStrings;
  Info->StructSize = sizeof(*Info);
  Info->PluginMenuStringsNumber = LENGTH_OF_ARRAY(PluginMenuStrings);
  Info->CommandPrefix = TEXT("fclip");
}

VOID WINAPI EXP_NAME(SetStartupInfo)(CONST struct PluginStartupInfo *pInfo)
{
  Info = *pInfo;
  FSF = *pInfo->FSF;
  Info.FSF = &FSF;
  hHeap = GetProcessHeap();
}

#ifdef UNICODE
INT WINAPI EXP_NAME(GetMinFarVersion)(VOID)
{
#ifdef UNICODE
  return MAKEFARVERSION(2, 0, 1145);
#endif
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
